#ifndef RAINY_BARRIER_AND_LATCH_H
#define RAINY_BARRIER_AND_LATCH_H
#include <atomic>
#include <rainy/base.hpp>
#include <rainy/meta/type_traits.hpp>

#define RAINY_NODISCARD_BARRIER_TOKEN                                                                                                   \
    RAINY_NODISCARD_MSG("The token from 'arrive()' should not be discarded; it should be passed to 'wait()'.")

#define RAINY_NODISCARD_TRY_WAIT \
    RAINY_NODISCARD_MSG("This member function returns the state of the synchronization object and does not do anything else; "                                 \
    "it is not useful to call this member function and discard the return value.")

namespace rainy::foundation::system::thread {
    struct no_complection_function {
        void operator()() const noexcept {}
    };

    template <typename complection_function = no_complection_function>
    class barrier;

    inline constexpr std::ptrdiff_t barrier_arrival_token_mask = 1;
    inline constexpr std::ptrdiff_t barrier_value_mask = ~barrier_arrival_token_mask;
    inline constexpr std::ptrdiff_t barrier_value_shift = 1;
    inline constexpr std::ptrdiff_t barrier_invalid_token = 0;
    inline constexpr std::ptrdiff_t barrier_value_step = 1 << barrier_value_shift;
    inline constexpr std::ptrdiff_t barrier_max = PTRDIFF_MAX >> barrier_value_shift;

    template <typename complection_function>
    class arrival_token {
    public:
        arrival_token(arrival_token &&other) noexcept {
            value = other.value;
            other.value = barrier_invalid_token;
        }

        arrival_token &operator=(arrival_token &&other) noexcept {
            value = other.value;
            other.value = barrier_invalid_token;
            return *this;
        }

    private:
        explicit arrival_token(const std::ptrdiff_t value_) noexcept : value(value_) {}
        friend barrier<complection_function>;

        std::ptrdiff_t value;
    };
}

#if RAINY_HAS_CXX20
namespace rainy::foundation::system::thread {
    template <typename complection_function>
    class barrier {
    public:
        static_assert(
#ifndef __cpp_noexcept_function_type
            type_traits::primary_types::is_function_v<remove_pointer_t<complection_function>> ||
#endif
                type_traits::type_properties::is_nothrow_invocable_v<complection_function &>,
            "is_nothrow_invocable_v<completion_function&> 的编译推导结果应当为真");

        using arrival_token = arrival_token<complection_function>;

        constexpr explicit barrier(const std::ptrdiff_t expected, complection_function function = complection_function()) noexcept :
            data_(utility::move(function), expected << barrier_value_shift) {
            utility::expects(expected >= 0 && expected <= (max) (), "expected >= 0 and expected <= max()");
        }

        barrier(const barrier &) = delete;
        barrier &operator=(const barrier &) = delete;

        RAINY_NODISCARD static constexpr std::ptrdiff_t(max)() noexcept {
            return barrier_max;
        }

        RAINY_NODISCARD_BARRIER_TOKEN arrival_token arrive(std::ptrdiff_t update_value = 1) noexcept /* strengthened */ {
            utility::expects(update_value > 0 && update_value <= (max) (), "update > 0");
            update_value <<= barrier_value_shift;
            // TRANSITION, GH-1133: should be memory_order_release
            std::ptrdiff_t current = data_.second.current.fetch_sub(update_value) - update_value;
            utility::ensures(current >= 0,
                                                   "update is less than or equal to the expected count for the current barrier phase");
            if ((current & barrier_value_mask) == 0) {
                // TRANSITION, GH-1133: should have this fence:
                // atomic_thread_fence(memory_order_acquire);
                completion_(current);
            }
            // 将其嵌入到令牌中，以提供额外的正确性检查，以确保令牌来自同一个屏障且未被使用。所有的位都应该匹配，因为barrier至少应该对齐到原子计数器的大小。
            return arrival_token{(current & barrier_arrival_token_mask) | reinterpret_cast<intptr_t>(this)};
        }

        void wait(arrival_token &&arrival) const noexcept {
            utility::expects((arrival.value & barrier_value_mask) == reinterpret_cast<intptr_t>(this),
                                                   "arrival is associated with the phase synchronization point for the current phase or "
                                                   "the immediately preceding phase of the same barrier object");
            const std::ptrdiff_t arrival_value = arrival.value & barrier_arrival_token_mask;
            arrival.value = barrier_invalid_token;
            for (;;) {
                // TRANSITION, GH-1133: should be memory_order_acquire
                const std::ptrdiff_t current = data_.second.current.load();
                utility::ensures(current >= 0, "Invariant counter >= 0, possibly caused by preconditions violation");
                if ((current & barrier_arrival_token_mask) != arrival_value) {
                    break;
                }
                data_.second.current.wait(current, std::memory_order_relaxed);
            }
        }

        void arrive_and_wait() noexcept /* strengthened */ {
            // TRANSITION, GH-1133: should be memory_order_acq_rel
            std::ptrdiff_t current = data_.second.current.fetch_sub(barrier_value_step) - barrier_value_step;
            const std::ptrdiff_t arrival = current & barrier_arrival_token_mask;
            utility::ensures(current >= 0,
                                                   "update is less than or equal to the expected count for the current barrier phase");
            if ((current & barrier_value_mask) == 0) {
                completion_(current);
                return;
            }
            for (;;) {
                data_.second.current.wait(current, std::memory_order_relaxed);
                current = data_.second.current.load();
                utility::ensures(current >= 0, "Invariant counter >= 0, possibly caused by preconditions violation");
                if ((current & barrier_arrival_token_mask) != arrival) {
                    break;
                }
            }
        }

        void arrive_and_drop() noexcept /* strengthened */ {
            const std::ptrdiff_t rem_count = data_.second.total.fetch_sub(barrier_value_step, std::memory_order_relaxed) - barrier_value_step;
            utility::expects(rem_count >= 0, "he expected count for the current barrier phase is greater than zero (checked initial "
                                             "expected count, which is not less than the current)");
            (void) arrive(1);
        }

    private:
        void completion_(const std::ptrdiff_t current) noexcept {
            const std::ptrdiff_t rem_count = data_.second.total.load(std::memory_order_relaxed);
            utility::expects(rem_count >= 0, "Invariant: initial expected count less than zero, "
                                                                   "possibly caused by preconditions violation ");
            data_.get_first()();
            const std::ptrdiff_t new_phase_count = rem_count | ((current + 1) & barrier_arrival_token_mask);
            // TRANSITION, GH-1133: should be memory_order_release
            data_.second.current.store(new_phase_count);
            data_.second.current.notify_all();
        }

        struct counter_t {
            constexpr explicit counter_t(std::ptrdiff_t initial) : current(initial), total(initial) {
            }
            // wait(arrival_token&&) 接受来自当前阶段或紧接前一阶段的令牌;
            // 这意味着我们可以使用1位来跟踪当前阶段，该位在每个阶段之间交替。为此，我们使用current的最低位。
            std::atomic<std::ptrdiff_t> current;
            std::atomic<std::ptrdiff_t> total;
        };

        utility::compressed_pair<complection_function, counter_t> data_;
    };

    class latch {
    public:
        explicit latch(const std::ptrdiff_t expected) noexcept : counter{expected} {
            utility::expects(expected >= 0, "expected >= 0");
        }

        latch(const latch &) = delete;
        latch &operator=(const latch &) = delete;

        RAINY_NODISCARD static constexpr std::ptrdiff_t(max)() noexcept {
            return PTRDIFF_MAX;
        }

        void count_down(const std::ptrdiff_t update = 1) noexcept {
            utility::expects(update >= 0, "update >= 0");
            // TRANSITION, GH-1133: should be memory_order_release
            const std::ptrdiff_t current = counter.fetch_sub(update) - update;
            if (current == 0) {
                counter.notify_all();
            }
            utility::ensures(current >= 0,"update <= counter");
        }

        RAINY_NODISCARD_TRY_WAIT bool try_wait() const noexcept {
            // TRANSITION, GH-1133: should be memory_order_acquire
            return counter.load() == 0;
        }

        void wait() const noexcept /* strengthened */ {
            for (;;) {
                // TRANSITION, GH-1133: should be memory_order_acquire
                const std::ptrdiff_t current = counter.load();
                if (current == 0) {
                    return;
                }
                utility::ensures(current > 0, "Invariant counter >= 0, possibly caused by preconditions violation");
                counter.wait(current, std::memory_order_relaxed);
            }
        }

        void arrive_and_wait(const std::ptrdiff_t update = 1) noexcept /* strengthened */ {
            utility::expects(update >= 0, "update >= 0");
            // TRANSITION, GH-1133: should be memory_order_acq_rel
            if (const std::ptrdiff_t current = counter.fetch_sub(update) - update;current == 0) {
                counter.notify_all();
            } else {
                utility::ensures(current > 0, "update <= counter");
                counter.wait(current, std::memory_order_relaxed);
                wait();
            }
        }

    private:
        std::atomic<std::ptrdiff_t> counter;
    };
}

#else
#include <condition_variable>
/*
虽然我们提供了C++17的兼容，但是这一兼容会导致barrier并不是轻量级的，
而是一个结合了condition_variable和mutex的产物
*/

namespace rainy::foundation::system::thread {
    template <typename complection_function>
    class barrier {
    public:
        static_assert(
#ifndef __cpp_noexcept_function_type
            type_traits::primary_types::is_function_v<remove_pointer_t<complection_function>> ||
#endif
                type_traits::type_properties::is_nothrow_invocable_v<complection_function &>,
            "is_nothrow_invocable_v<completion_function&> 的编译推导结果应当为真");

        using arrival_token = arrival_token<complection_function>;

        constexpr explicit barrier(const std::ptrdiff_t expected, complection_function function = complection_function()) noexcept :
            data_(utility::move(function), expected << barrier_value_shift) {
            utility::expects(expected >= 0 && expected <= (max) ());
            // expected >= 0 and expected <= max();
        }

        barrier(const barrier &) = delete;
        barrier &operator=(const barrier &) = delete;

        RAINY_NODISCARD static constexpr std::ptrdiff_t(max)() noexcept {
            return barrier_max;
        }

        RAINY_NODISCARD_BARRIER_TOKEN arrival_token arrive(std::ptrdiff_t update_value = 1) noexcept /* strengthened */ {
            utility::expects(update_value > 0 && update_value <= (max) (), "update > 0");
            update_value <<= barrier_value_shift;
            // TRANSITION, GH-1133: should be memory_order_release
            std::ptrdiff_t current = data_.second.current.fetch_sub(update_value) - update_value;
            utility::ensures(current >= 0,
                             "update is less than or equal to the expected count for the current barrier phase");
            if ((current & barrier_value_mask) == 0) {
                // TRANSITION, GH-1133: should have this fence:
                // atomic_thread_fence(memory_order_acquire);
                completion_(current);
            }
            return arrival_token{(current & barrier_arrival_token_mask) | reinterpret_cast<intptr_t>(this)};
        }

        void wait(arrival_token &&arrival) const noexcept {
            utility::expects((arrival.value & barrier_value_mask) == reinterpret_cast<intptr_t>(this),
                             "arrival is associated with the phase synchronization point for the current phase or "
                             "the immediately preceding phase of the same barrier object");
            const std::ptrdiff_t arrival_value = arrival.value & barrier_arrival_token_mask;
            arrival.value = barrier_invalid_token;

            for (;;) {
                std::ptrdiff_t current = data_.second.current.load();
                utility::ensures(current >= 0, "Invariant counter >= 0, possibly caused by preconditions violation");
                if ((current & barrier_arrival_token_mask) != arrival_value) {
                    break;
                }

                {
                    std::unique_lock<std::mutex> lock(data_.second.mutex);
                    if ((data_.second.current.load() & barrier_arrival_token_mask) == arrival_value) {
                        data_.second.cv.wait(lock);
                    }
                }
            }
        }

        void arrive_and_wait() noexcept /* strengthened */ {
            std::ptrdiff_t current = data_.second.current.fetch_sub(barrier_value_step) - barrier_value_step;
            const std::ptrdiff_t arrival = current & barrier_arrival_token_mask;
            utility::ensures(current >= 0,
                             "update is less than or equal to the expected count for the current barrier phase");
            if ((current & barrier_value_mask) == 0) {
                completion_(current);
                return;
            }
            for (;;) {
                {
                    std::unique_lock<std::mutex> lock(data_.second.mutex);
                    if ((data_.second.current.load() & barrier_arrival_token_mask) == arrival) {
                        data_.second.cv.wait(lock);
                    }
                }
                current = data_.second.current.load();
                utility::ensures(current >= 0, "Invariant counter >= 0, possibly caused by preconditions violation");
                if ((current & barrier_arrival_token_mask) != arrival) {
                    break;
                }
            }
        }

        void arrive_and_drop() noexcept /* strengthened */ {
            const std::ptrdiff_t rem_count = data_.second.total.fetch_sub(barrier_value_step, std::memory_order_relaxed) - barrier_value_step;
            utility::expects(rem_count >= 0, "The expected count for the current barrier phase is greater than zero");
            (void) arrive(1);
        }

    private:
        void completion_(const std::ptrdiff_t current) noexcept {
            const std::ptrdiff_t rem_count = data_.second.total.load(std::memory_order_relaxed);
            utility::expects(rem_count >= 0, "Invariant: initial expected count less than zero");
            data_.get_first()();
            const std::ptrdiff_t new_phase_count = rem_count | ((current + 1) & barrier_arrival_token_mask);
            data_.second.current.store(new_phase_count);
            {
                std::lock_guard<std::mutex> lock(data_.second.mutex);
                data_.second.cv.notify_all();
            }
        }

        struct counter_t {
            constexpr explicit counter_t(std::ptrdiff_t initial) : current(initial), total(initial) {}
            std::atomic<std::ptrdiff_t> current;
            std::atomic<std::ptrdiff_t> total;
            mutable std::mutex mutex;
            mutable std::condition_variable cv;
        };

        utility::compressed_pair<complection_function, counter_t> data_;
    };

    class latch {
    public:
        explicit latch(const std::ptrdiff_t expected) noexcept : counter{expected} {
            utility::expects(expected >= 0, "expected >= 0");
        }

        latch(const latch &) = delete;
        latch &operator=(const latch &) = delete;

        RAINY_NODISCARD static constexpr std::ptrdiff_t (max)() noexcept {
            return PTRDIFF_MAX;
        }

        void count_down(const std::ptrdiff_t update = 1) noexcept {
            utility::expects(update >= 0, "update >= 0");
            const std::ptrdiff_t current = counter.fetch_sub(update) - update;
            if (current == 0) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.notify_all();
            }
            utility::ensures(current >= 0, "update <= counter");
        }

        RAINY_NODISCARD_TRY_WAIT bool try_wait() const noexcept {
            return counter.load() == 0;
        }

        void wait() const noexcept {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return counter.load() == 0; });
        }

        void arrive_and_wait(const std::ptrdiff_t update = 1) noexcept {
            utility::expects(update >= 0, "update >= 0");
            const std::ptrdiff_t current = counter.fetch_sub(update) - update;
            if (current == 0) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.notify_all();
            } else {
                utility::ensures(current > 0, "update <= counter");
                wait();
            }
        }

    private:
        std::atomic<std::ptrdiff_t> counter;
        mutable std::mutex mtx;
        mutable std::condition_variable cv;
    };
}
#endif

#endif