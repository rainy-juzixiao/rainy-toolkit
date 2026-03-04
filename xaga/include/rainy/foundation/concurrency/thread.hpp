/*
 * Copyright 2026 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_CONCURRENCY_THREAD_HPP
#define RAINY_FOUNDATION_CONCURRENCY_THREAD_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency {
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

        thread(policy policy) : policy_(policy), thread_handle{} { // NOLINT
        }

        thread(thread &&right) noexcept :
            policy_(utility::exchange(right.policy_, policy::manual)), thread_handle(utility::exchange(right.thread_handle, {})) {
        }

        template <typename Fx, typename... Args>
        RAINY_NODISCARD explicit thread(Fx &&f, Args &&...args) : policy_(policy::manual) {
            start(utility::forward<Fx>(f), utility::forward<Args>(args)...);
        }

        template <typename Fx, typename... Args>
        RAINY_NODISCARD explicit thread(const policy policy, Fx &&f, Args &&...args) : policy_(policy) {
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
            auto decay_copied = foundation::memory::make_nebula<tuple>(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
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

        RAINY_NODISCARD id get_id() const noexcept;

    private:
        template <typename Tuple, std::size_t... Indices>
        static unsigned int invoke_function(void *arg_list) {
            foundation::memory::nebula_ptr<Tuple> fn_vals(static_cast<Tuple *>(arg_list));
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
}

namespace rainy::foundation::system::this_thread::implements {
    template <typename Rep, typename Period>
    RAINY_NODISCARD auto to_absolute_time(const std::chrono::duration<Rep, Period> &rel_time) noexcept {
        constexpr auto zero = std::chrono::duration<Rep, Period>::zero();
        const auto now = std::chrono::steady_clock::now();
        decltype(now + rel_time) abs_time = now;
        if (rel_time > zero) {
            constexpr auto forever = (std::chrono::steady_clock::time_point::max) ();
            if (abs_time < forever - rel_time) {
                abs_time += rel_time;
            } else {
                abs_time = forever;
            }
        }
        return abs_time;
    }

    using concurrency::implements::thread_sleep_for;
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

    RAINY_NODISCARD concurrency::thread::id get_id() noexcept;

    void yield() noexcept;
}

namespace rainy::foundation::concurrency {
    class thread::id {
    public:
        id() noexcept = default;

        friend class thread;

    private:
        explicit id(const std::uint64_t id) noexcept : id_{id} {
        }

        friend id thread::get_id() const noexcept;
        friend id system::this_thread::get_id() noexcept;

        friend bool operator==(id left, id right) noexcept;

#if RAINY_HAS_CXX20
        friend std::strong_ordering operator<=>(annotations::lifetime::in<id> left, const annotations::lifetime::in<id> right) noexcept {
            return left.id_ <=> right.id_;
        }
#else
        friend bool operator<(annotations::lifetime::in<id> left, annotations::lifetime::in<id> right) noexcept;
#endif

        template <typename Elem, typename Traits>
        friend std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &ostream, annotations::lifetime::in<id> id) {
            ostream << id.id_;
            return ostream;
        }

        friend utility::hash<id>;
        friend std::hash<id>;

        std::uint64_t id_{};
    };

#if !RAINY_HAS_CXX20
    RAINY_NODISCARD bool operator!=(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator<(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator<=(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator>(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator>=(thread::id left, thread::id right) noexcept;
#endif
}

namespace std { // NOLINT
    template <>
    struct hash<rainy::foundation::concurrency::thread::id> {
        using argument_type = rainy::foundation::concurrency::thread::id;
        using result_type = std::uint64_t;

        RAINY_NODISCARD result_type operator()(const argument_type keyval) const noexcept {
            return rainy::utility::implements::hash_representation(keyval.id_);
        }
    };
}

namespace rainy::utility { // NOLINT
    template <>
    struct hash<foundation::concurrency::thread::id> {
        using argument_type = foundation::concurrency::thread::id;
        using result_type = std::uint64_t;

        RAINY_NODISCARD result_type operator()(const argument_type keyval) const noexcept {
            return implements::hash_representation(keyval.id_);
        }
    };
}

#endif
