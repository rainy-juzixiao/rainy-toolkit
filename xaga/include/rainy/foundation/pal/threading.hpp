/*
 * Copyright 2025 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_FOUNDATION_PAL_THREADING_HPP
#define RAINY_FOUNDATION_PAL_THREADING_HPP
/**
 * @file threading.hpp
 * @brief 此头文件是rainy-toolkit用于实现跨平台线程API的库，若要访问内部的API，请参阅implements文件夹的tgc_layer_threading.hpp
 */
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/system/memory/nebula_ptr.hpp>
#include <rainy/foundation/system/memory/allocator.hpp>
#include <rainy/utility/any.hpp>
#include <rainy/foundation/pal/implements/tgc_layer_threading.hpp>
#include <rainy/foundation/pal/atomicinfra.hpp>

namespace rainy::foundation::pal::threading {
    class RAINY_TOOLKIT_API thread {
    public:
        class id;

        enum class policy {
            manual,
            auto_join,
            auto_detach
        };

#if RAINY_USING_LINUX
        using native_handle_type = ::pthread_t;
#else
        using native_handle_type = void *;
#endif

        thread() = default;

        thread(policy policy) : policy_(policy), thread_handle(0) {
        }

        thread(thread &&right) noexcept :
            policy_(utility::exchange(right.policy_, policy::manual)), thread_handle(utility::exchange(right.thread_handle, {})) {
        }

        template <typename Fx, typename... Args>
        RAINY_NODISCARD explicit thread(Fx &&f, Args &&...args) : policy_(policy::manual) {
            start(utility::forward<Fx>(f), utility::forward<Args>(args)...);
        }

        template <typename Fx, typename... Args>
        RAINY_NODISCARD explicit thread(policy policy, Fx &&f, Args &&...args) : policy_(policy) {
            start(utility::forward<Fx>(f), utility::forward<Args>(args)...);
        }

        ~thread();

        thread &operator=(thread &&right) noexcept;
        
        thread(const thread &) = delete;
        thread &operator=(const thread &) = delete;

        template <typename Fx, typename... Args>
        void start(Fx &&fn, Args &&...args) {
            using tuple = std::tuple<type_traits::other_trans::decay_t<Fx>, type_traits::other_trans::decay_t<Args>...>;
            if (thread_handle.handle) {
                return;
            }
            auto decay_copied =
                foundation::system::memory::make_nebula<tuple>(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
            constexpr auto invoker = get_invoke_function_addr<tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
            thread_handle = implements::create_thread(nullptr, 0, invoker, decay_copied.get(), 0, nullptr);
            if (thread_handle.handle) {
                (void) decay_copied.release();
            } else {
                throw std::system_error(errno, std::generic_category(), "Failed to create thread");
            }
        }

        void join();

        void detach();

        RAINY_NODISCARD bool joinable() const noexcept;

        static void sleep_for(const unsigned long ms) noexcept;

        RAINY_NODISCARD native_handle_type native_handle() const noexcept;

        void suspend() noexcept;

        void resume() noexcept;

        id get_id() const noexcept;

    private:
        template <typename Tuple, std::size_t... Indices>
        static unsigned int invoke_function(void *arg_list) {
            foundation::system::memory::nebula_ptr<Tuple> fn_vals(static_cast<Tuple *>(arg_list));
            Tuple &tuple = *fn_vals.get();
            utility::invoke(utility::move(std::get<Indices>(tuple))...);
            return 0;
        }

        template <typename Tuple, std::size_t... Indices>
        static constexpr auto get_invoke_function_addr(std::index_sequence<Indices...>) {
            return &invoke_function<Tuple, Indices...>;
        }

        policy policy_{};
        implements::schd_thread_t thread_handle{};
    };

    template <typename Fx, typename... Args>
    thread make_thread(Fx &&fn, Args &&...args) {
        return thread(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
    }

    template <typename Mutex, typename Fx, typename... Args>
    RAINY_INLINE decltype(auto) create_synchronized_task(Mutex &mtx, Fx &&callable, Args &&...args) noexcept(
        noexcept(utility::invoke(utility::forward<Fx>(callable), utility::forward<Args>(args)...))) {
        std::lock_guard<Mutex> guard{mtx};
        return utility::invoke(utility::forward<Fx>(callable), utility::forward<Args>(args)...);
    }
}

namespace rainy::foundation::system::this_thread::implements {
    template <typename Rep, typename Period>
    RAINY_NODISCARD auto to_absolute_time(const std::chrono::duration<Rep, Period> &rel_time) noexcept {
        constexpr auto zero = std::chrono::duration<Rep, Period>::zero();
        const auto now = std::chrono::steady_clock::now();
        decltype(now + rel_time) abs_time = now;
        if (rel_time > zero) {
            constexpr auto forever = (std::chrono::steady_clock::time_point::max)();
            if (abs_time < forever - rel_time) {
                abs_time += rel_time;
            } else {
                abs_time = forever;
            }
        }
        return abs_time;
    }

    using pal::threading::implements::thread_sleep_for;
}

namespace rainy::foundation::system::this_thread {
    template <typename Clock, typename Duration>
    void sleep_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
#if RAINY_HAS_CXX20
        static_assert(std::chrono::is_clock_v<Clock>, "Clock type required");
#endif
        for (;;) {
            const auto now = Clock::now();
            if (abs_time <= now) {
                return;
            }
            constexpr std::chrono::milliseconds clamp{std::chrono::hours{24}};
            const auto rel = abs_time - now;
            if (rel >= clamp) {
                implements::thread_sleep_for(clamp.count());
            } else {
                const auto rel_ms = std::chrono::ceil<std::chrono::milliseconds>(rel);
                implements::thread_sleep_for(static_cast<unsigned long>(rel_ms.count()));
            }
        }
    }

    template <typename Rep, typename Period>
    void sleep_for(const std::chrono::duration<Rep, Period> &rel_time) {
        sleep_until(implements::to_absolute_time(rel_time));
    }

#if RAINY_USING_LINUX
    RAINY_NODISCARD ::pthread_t get_current_thread() noexcept;
#else
    RAINY_NODISCARD void *get_current_thread() noexcept;
#endif

    RAINY_NODISCARD pal::threading::thread::id get_id() noexcept;

    void yield() noexcept;
}

namespace rainy::foundation::pal::threading {
    class thread::id {
    public:
        id() noexcept = default;

        friend class thread;

    private:
        explicit id(std::uint64_t id) noexcept : id_{id} {
        }

        friend thread::id thread::get_id() const noexcept;
        friend thread::id foundation::system::this_thread::get_id() noexcept;
        
        friend bool operator==(thread::id left, thread::id right) noexcept;

#if !RAINY_HAS_CXX20
        friend std::strong_ordering operator<=>(thread::id left, thread::id right) noexcept {
            return left.id_ <=> right.id_;
        }
#else 
        friend bool operator<(thread::id left, thread::id right) noexcept;
#endif 

        template <typename Elem, typename Traits>
        friend std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &ostream, id id) {
            ostream << id.id_;
            return ostream;
        }

        friend utility::hash<thread::id>;
        friend std::hash<thread::id>;

        std::uint64_t id_{};
    };

    RAINY_NODISCARD bool operator!=(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator<(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator<=(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator>(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator>=(thread::id left, thread::id right) noexcept;
}

namespace std {
    template <>
    struct hash<rainy::foundation::pal::threading::thread::id> {
        using argument_type = rainy::foundation::pal::threading::thread::id;
        using result_type = std::uint64_t;

        RAINY_NODISCARD static result_type operator()(const argument_type keyval) noexcept {
            return rainy::utility::implements::hash_representation(keyval.id_);
        }
    };
}

namespace rainy::utility {
    template <>
    struct hash<foundation::pal::threading::thread::id> {
        using argument_type = foundation::pal::threading::thread::id;
        using result_type = std::uint64_t;

        RAINY_NODISCARD static result_type operator()(const argument_type keyval) noexcept {
            return implements::hash_representation(keyval.id_);
        }
    };
}

namespace rainy::foundation::pal::threading {
    enum class future_errc {
        broken_promise = 1,
        future_already_retrieved,
        promise_already_satisfied,
        no_state
    };

    enum class launch {
        async = 0x1,
        deferred = 0x2
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(launch);

    enum class future_status {
        ready,
        timeout,
        deferred
    };
}

namespace rainy::foundation::pal::threading::implements {
    template <typename Ty>
    class associated_state;

    template <typename Ty>
    struct deleter_bridge {
        virtual void delete_this(associated_state<Ty> *) noexcept = 0;
        virtual ~deleter_bridge() = default;
    };

    template <typename Ty, typename Derived, typename Alloc>
    struct state_deleter : deleter_bridge<Ty> {
        state_deleter(const Alloc &al) : alloc(al) {
        }

        state_deleter(const state_deleter &) = delete;
        state_deleter &operator=(const state_deleter &) = delete;

        void delete_this(associated_state<Ty> *state) noexcept override {
        
        }

        Alloc alloc;
    };

    template <class Ty>
    union _Result_holder {
        _Result_holder() noexcept {
        }
        ~_Result_holder() noexcept {
        }

        Ty _Held_value;
    };
}

#endif