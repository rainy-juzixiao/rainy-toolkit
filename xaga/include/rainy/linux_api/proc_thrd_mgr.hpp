#ifndef RAINY_LINUXAPI_PROC_THRD_MGR_H
#define RAINY_LINUXAPI_PROC_THRD_MGR_H
#include <rainy/core/core.hpp>
#if RAINY_USING_LINUX
#include <cstdlib>
#include <rainy/base.hpp>
#include <rainy/meta/type_traits.hpp>
#include <rainy/system/exceptions.hpp>
#include <memory>
#include <pthread.h>
#include <string>
#include <thread>
/*
此库是为Linux所适配的
*/

namespace rainy::linux_api::proc_thrd {
    class thread_manager {
    public:
        using native_handle_type = pthread_t;

        thread_manager() : thread_({}) {
        }

        template <typename Fx,typename... Args, type_traits::other_trans::enable_if_t<
                                   !type_traits::type_relations::is_same_v<
                                       type_traits::cv_modify::remove_cvref_t<Fx>, thread_manager> &&
                                       type_traits::type_properties::is_invocable_r_v<void, Fx, Args...>,
                                   int> = 0>
        explicit thread_manager(Fx handler,Args&&... args) : thread_({}) {
            create_thread(handler,foundation::utility::forward<Args&&>(args)...);
        }

        thread_manager(thread_manager &&other) noexcept : thread_(std::exchange(other.thread_, {})) {
        }

        thread_manager(const thread_manager &) = delete;
        thread_manager &operator=(const thread_manager &) = delete;

        ~thread_manager() {
            if (joinable()) {
                join();
            }
        }

        RAINY_NODISCARD bool joinable() const noexcept {
            return thread_.thread != 0;
        }

        thread_manager &operator=(thread_manager &&right) noexcept {
            if (joinable()) {
                std::terminate();
            }
            thread_.thread = std::exchange(right.thread_.thread, {});
            return *this;
        }

        void join() {
            using foundation::system::exceptions::throw_exception;
            using foundation::system::exceptions::runtime::runtime_error;
            if (!joinable()) {
                throw_exception(runtime_error("INVALID_ARGUMENT"));
            }
            if (thread_.thread == pthread_self()) {
                throw_exception(runtime_error("RESOURCE_DEADLOCK_WOULD_OCCUR"));
            }
            if (pthread_join(thread_.thread, nullptr) != 0) {
                throw_exception(runtime_error("NO_SUCH_PROCESS"));
            }
            thread_ = {};
        }

        void detach() {
            using foundation::system::exceptions::throw_exception;
            using foundation::system::exceptions::runtime::runtime_error;
            if (!joinable()) {
                throw_exception(runtime_error("INVALID_ARGUMENT"));
            }
            // 由于我们没有与detach等效的WindowsAPI，所以我们直接置空
            thread_ = {};
        }

        RAINY_NODISCARD unsigned long get_id() const noexcept {
            return thread_.thread;
        }

        template <typename Fx, typename... Args,type_traits::other_trans::enable_if_t<
                                   !type_traits::type_relations::is_same_v<
                                       type_traits::cv_modify::remove_cvref_t<Fx>, thread_manager> &&
                                       type_traits::type_properties::is_invocable_r_v<void, Fx, Args...>,
                                   int> = 0>
        void start(Fx handler,Args&&...args) {
            create_thread(handler, std::forward<Args>(args)...);
        }

        void swap(thread_manager &right) noexcept {
            std::swap(thread_.thread, right.thread_.thread);
        }

        RAINY_NODISCARD native_handle_type native_handle() const noexcept {
            return thread_.thread;
        }

        static unsigned int hardware_concurrency() noexcept {
            return pthread_getconcurrency();
        }

    private:
        template <typename Tuple, std::size_t... Indices>
        static void *invoke(void *raw_vals) noexcept {
            const std::unique_ptr<Tuple> fnvals(static_cast<Tuple *>(raw_vals));
            Tuple &tup = *fnvals.get();
            foundation::utility::invoke(foundation::utility::move(std::get<Indices>(tup)...));
            return nullptr;
        }

        template <typename Tuple, std::size_t... Indices>
        RAINY_NODISCARD static constexpr auto get_invoke(std::index_sequence<Indices...>) noexcept {
            return &invoke<Tuple, Indices...>;
        }

        template <typename Fx,typename... Args>
        void create_thread(Fx handler, Args... args) {
            using tuple = std::tuple<std::decay_t<Fx>, std::decay_t<Args>...>;
            auto decay_copied = std::make_unique<tuple>(std::forward<Fx>(handler), std::forward<Args...>(args)...);
            constexpr auto invoker_proc = get_invoke<tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
            if (const int rc = ::pthread_create(&thread_.thread, nullptr, invoker_proc, decay_copied.get()); rc == 0) {
                (void) decay_copied.release();
            } else {
                using namespace foundation::system::exceptions;
                throw_exception(runtime::runtime_error("RESOURCE_UNAVAILABLE_TRY_AGAIN"));
            }
        }

        struct {
            // avoid unreachable code | 防止不可到达代码警告
            pthread_t thread;
        } thread_;
    };
}

namespace rainy::foundation::system::this_thread {
    namespace utils {
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
    }

    template <typename Clock, typename Duration>
    void sleep_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
#if RAINY_HAS_CXX20
        static_assert(std::chrono::is_clock_v<Clock>, "Clock type required");
#endif // RAINY_HAS_CXX20
        for (;;) {
            const auto now = Clock::now();
            if (abs_time <= now) {
                return;
            }
            const auto rel = abs_time - now;
            const auto seconds = std::chrono::ceil<std::chrono::seconds>(rel);
            const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(rel - seconds);
            timespec ts_timespec = {static_cast<std::time_t>(seconds.count()), static_cast<long>(nanoseconds.count())};
            while (nanosleep(&ts_timespec, &ts_timespec)) {
            }
        }
    }

    template <typename Rep, typename Period>
    void sleep_for(const std::chrono::duration<Rep, Period> &rel_time) {
        sleep_until(utils::to_absolute_time(rel_time));
    }

    RAINY_NODISCARD inline pthread_t get_current_thread() {
        return pthread_self();
    }
}

#endif

#endif