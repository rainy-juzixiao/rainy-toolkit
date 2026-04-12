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
#ifndef RAINY_FOUNDATION_CONCURRENCY_MUTEX_HPP
#define RAINY_FOUNDATION_CONCURRENCY_MUTEX_HPP
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency {
    /**
     * @brief 互斥量。
     *
     * 提供独占所有权同步原语。
     * 行为类似 std::mutex。
     *
     * 不可复制，不可赋值。
     */
    class RAINY_TOOLKIT_API mutex {
    public:
        using native_handle_type = void *;

        /**
         * @brief 构造互斥量。
         *
         * 创建底层互斥对象。
         * 失败时调用 std::terminate()。
         */
        mutex() noexcept;

        /**
         * @brief 析构互斥量。
         *
         * 销毁底层互斥对象。
         */
        ~mutex();

        mutex(const mutex &) = delete;
        rain_fn operator=(const mutex &)->mutex & = delete;

        /**
         * @brief 锁定互斥量。
         *
         * 阻塞直到获得所有权。
         * 出错时调用 std::terminate()。
         */
        rain_fn lock() -> void;

        /**
         * @brief 尝试锁定互斥量。
         *
         * @retval true  成功获得锁
         * @retval false 已被占用
         *
         * 出错时调用 std::terminate()。
         */
        RAINY_NODISCARD rain_fn try_lock() noexcept -> bool;

        /**
         * @brief 解锁互斥量。
         *
         * 当前线程必须持有该互斥量。
         * 出错时调用 std::terminate()。
         */
        rain_fn unlock() -> void; // NOLINT

        /**
         * @brief 获取原生句柄。
         */
        rain_fn native_handle() noexcept -> native_handle_type;

        /**
         * @brief 获取 rainy-toolkit 抽象层 mtx_t 句柄指针
         *
         * 返回指向内部 mtx_t 成员的指针（即 void**），可直接传给
         * cnd_wait / cnd_timedwait 等同层 PAL 函数。
         *
         * @note 与 native_handle() 的区别：native_handle() 返回的是
         *       已解引用一层的底层 OS 句柄
         *       rainy-toolkit() 返回的是 PAL 句柄本身的地址（mtx_t*）。
         */
        rain_fn backend_handle() noexcept -> implements::mtx_t * {
            return &mtx_;
        }

    private:
        implements::mtx_t mtx_{nullptr};
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 递归互斥量。
     *
     * 允许同一线程多次获得所有权。
     * 行为类似 std::recursive_mutex。
     *
     * 不可复制，不可赋值。
     */
    class RAINY_TOOLKIT_API recursive_mutex {
    public:
        using native_handle_type = implements::mtx_t *;

        /**
         * @brief 构造递归互斥量。
         *
         * 创建支持递归锁定的底层互斥对象。
         * 失败时调用 std::terminate()。
         */
        recursive_mutex() noexcept;

        /**
         * @brief 析构递归互斥量。
         *
         * 销毁底层互斥对象。
         */
        ~recursive_mutex();

        recursive_mutex(const recursive_mutex &) = delete;
        rain_fn operator=(const recursive_mutex &)->recursive_mutex & = delete;

        /**
         * @brief 锁定互斥量。
         *
         * 阻塞直到获得所有权。
         * 出错时调用 std::terminate()。
         */
        rain_fn lock() -> void;

        /**
         * @brief 尝试锁定互斥量。
         *
         * @retval true  成功获得锁
         * @retval false 互斥量已被占用
         *
         * 出错时调用 std::terminate()。
         */
        RAINY_NODISCARD rain_fn try_lock() noexcept -> bool;

        /**
         * @brief 解锁互斥量。
         *
         * 当前线程必须持有该互斥量。
         * 出错时调用 std::terminate()。
         */
        rain_fn unlock() -> void; // NOLINT

        /**
         * @brief 获取原生句柄。
         */
        rain_fn native_handle() noexcept -> native_handle_type;

        /**
         * @brief 获取 rainy-toolkit 抽象层 mtx_t 句柄指针
         *
         * 返回指向内部 mtx_t 成员的指针（即 void**），可直接传给
         * cnd_wait / cnd_timedwait 等同层 PAL 函数。
         *
         * @note 与 native_handle() 的区别：native_handle() 返回的是
         *       已解引用一层的底层 OS 句柄
         *       rainy-toolkit() 返回的是 PAL 句柄本身的地址（mtx_t*）。
         */
        rain_fn backend_handle() noexcept -> implements::mtx_t * {
            return &mtx_;
        }

    private:
        implements::mtx_t mtx_{nullptr};
    };
}

namespace rainy::foundation::concurrency::implements {
    template <class Clock, class Duration>
    RAINY_INLINE ::timespec to_abs_timespec(const std::chrono::time_point<Clock, Duration> &tp) { // NOLINT
        if constexpr (std::is_same_v<Clock, std::chrono::system_clock>) {
            auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp).time_since_epoch();
            ::timespec ts{};
            ts.tv_sec = static_cast<time_t>(ns.count() / 1'000'000'000);
            ts.tv_nsec = static_cast<long>(ns.count() % 1'000'000'000);
            return ts;
        } else if constexpr (std::is_same_v<Clock, std::chrono::steady_clock>) {
            // 把 steady_clock 转为 system_clock
            auto now_sys = std::chrono::system_clock::now();
            auto now_steady = std::chrono::steady_clock::now();
            auto abs_sys_tp = now_sys + (tp - now_steady);
            auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(abs_sys_tp).time_since_epoch();
            ::timespec ts{};
            ts.tv_sec = static_cast<time_t>(ns.count() / 1'000'000'000);
            ts.tv_nsec = static_cast<long>(ns.count() % 1'000'000'000);
            return ts;
        } else {
            static_assert(sizeof(Clock) == 0, "Unsupported clock type for to_abs_timespec");
        }
        std::terminate();
    }
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 定时互斥量。
     *
     * 支持限时锁定操作。
     * 行为类似 std::timed_mutex。
     *
     * 不可复制，不可赋值。
     */
    class RAINY_TOOLKIT_API timed_mutex {
    public:
        using native_handle_type = implements::mtx_t *;

        /**
         * @brief 构造定时互斥量。
         *
         * 创建支持定时等待的底层互斥对象。
         * 失败时调用 std::terminate()。
         */
        timed_mutex() noexcept;

        /**
         * @brief 析构定时互斥量。
         *
         * 销毁底层互斥对象。
         */
        ~timed_mutex();

        timed_mutex(const timed_mutex &) = delete;
        rain_fn operator=(const timed_mutex &)->timed_mutex & = delete;

        /**
         * @brief 锁定互斥量。
         *
         * 阻塞直到获得所有权。
         * 出错时调用 std::terminate()。
         */
        rain_fn lock() -> void;

        /**
         * @brief 尝试锁定互斥量。
         *
         * @retval true  成功获得锁
         * @retval false 已被占用
         *
         * 出错时调用 std::terminate()。
         */
        RAINY_NODISCARD rain_fn try_lock() -> bool;

        /**
         * @brief 在给定时间段内尝试锁定。
         *
         * 若等待时间不为正，则等价于 try_lock()。
         */
        template <typename Rep, typename Period>
        rain_fn try_lock_for(const std::chrono::duration<Rep, Period> &rel_time) -> bool {
            using namespace std::chrono;
            if (rel_time <= rel_time.zero()) {
                return try_lock();
            }
            auto abs = system_clock::now() + rel_time + milliseconds(1);
            return try_lock_until(abs);
        }

        /**
         * @brief 在指定时间点前尝试锁定。
         */
        template <typename Clock, typename Duration>
        rain_fn try_lock_until(const std::chrono::time_point<Clock, Duration> &abs_time) -> bool {
            auto ts = implements::to_abs_timespec(abs_time);
#if RAINY_USING_MACOS
            if (implements::mtx_lock(&m_) != thrd_result::success) {
                std::terminate();
            }
            bool no_timeout = Clock::now() < abs_time;
            while (no_timeout && locked_) {
                auto r = implements::cnd_timedwait(&cv_, &m_, &ts);
                if (r == thrd_result::timed_out) {
                    no_timeout = false;
                } else if (r != thrd_result::success) {
                    implements::mtx_unlock(&m_);
                    std::terminate();
                }
                if (no_timeout) {
                    no_timeout = Clock::now() < abs_time;
                }
            }
            bool acquired = false;
            if (!locked_) {
                locked_   = true;
                acquired  = true;
            }
            implements::mtx_unlock(&m_);
            return acquired;
#else
            auto r = implements::mtx_timedlock(&mtx_, &ts);
            if (r == thrd_result::success) {
                return true;
            }
            if (r == thrd_result::timed_out) {
                return false;
            }
            if (r == thrd_result::busy) {
                return false;
            }
            std::terminate();
#endif
        }

        /**
         * @brief 解锁互斥量。
         *
         * 当前线程必须持有该互斥量。
         * 出错时调用 std::terminate()。
         */
        rain_fn unlock() -> void; // NOLINT

        /**
         * @brief 获取原生句柄。
         */
        rain_fn native_handle() noexcept -> native_handle_type;

        /**
         * @brief 获取 rainy-toolkit 抽象层 mtx_t 句柄指针
         *
         * 返回指向内部 mtx_t 成员的指针（即 void**），可直接传给
         * cnd_wait / cnd_timedwait 等同层 PAL 函数。
         *
         * @note 与 native_handle() 的区别：native_handle() 返回的是
         *       已解引用一层的底层 OS 句柄
         *       rainy-toolkit() 返回的是 PAL 句柄本身的地址（mtx_t*）。
         */
        rain_fn backend_handle() noexcept -> implements::mtx_t * {
#if RAINY_USING_MACOS
            return &m_;
#else
            return &mtx_;
#endif
        }

    private:
#if RAINY_USING_MACOS
        implements::mtx_t m_{nullptr};
        implements::cnd_t cv_{nullptr};
        bool locked_{false};
#else
        implements::mtx_t mtx_{nullptr};
#endif
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 递归定时互斥量。
     *
     * 允许同一线程多次获得所有权，并支持限时锁定。
     * 行为类似 std::recursive_timed_mutex。
     *
     * 不可复制，不可赋值。
     */
    class RAINY_TOOLKIT_API recursive_timed_mutex {
    public:
        using native_handle_type = implements::mtx_t *;

        /**
         * @brief 构造递归定时互斥量
         *
         * 创建支持递归和定时锁定的底层互斥对象。
         * 出错时调用 std::terminate()。
         */
        recursive_timed_mutex() noexcept;

        /**
         * @brief 析构递归定时互斥量
         *
         * 销毁底层互斥对象。
         */
        ~recursive_timed_mutex();

        recursive_timed_mutex(const recursive_timed_mutex &) = delete;
        auto operator=(const recursive_timed_mutex &) -> recursive_timed_mutex & = delete;

        /**
         * @brief 锁定互斥量
         *
         * 阻塞直到获得所有权。
         * 出错时调用 std::terminate()。
         */
        rain_fn lock() -> void;

        /**
         * @brief 尝试锁定互斥量
         *
         * @retval true  成功获得锁
         * @retval false 已被占用
         *
         * 出错时调用 std::terminate()。
         */
        RAINY_NODISCARD
        rain_fn try_lock() noexcept -> bool;

        /**
         * @brief 在给定时间段内尝试锁定
         *
         * 若等待时间不为正，则等价于 try_lock()。
         */
        template <typename Rep, typename Period>
        rain_fn try_lock_for(const std::chrono::duration<Rep, Period> &rel_time) -> bool {
            using namespace std::chrono;
            if (rel_time <= rel_time.zero()) {
                return try_lock();
            }
            // +1ms 补偿时间转换和进入循环的损耗，保证实际等待时间不短于 rel_time
            auto abs = system_clock::now() + rel_time + milliseconds(1);
            return try_lock_until(abs);
        }

        /**
         * @brief 在指定时间点前尝试锁定
         */
        template <typename Clock, typename Duration>
        rain_fn try_lock_until(const std::chrono::time_point<Clock, Duration> &abs_time) -> bool {
            auto ts = implements::to_abs_timespec(abs_time);
            auto r = implements::mtx_timedlock(&mtx_, &ts);
            if (r == thrd_result::success) {
                return true;
            }
            if (r == thrd_result::timed_out) {
                return false;
            }
            if (r == thrd_result::busy) {
                return false;
            }
            std::terminate();
        }

        /**
         * @brief 解锁互斥量
         *
         * 当前线程必须持有该互斥量。
         * 出错时调用 std::terminate()。
         */
        rain_fn unlock() -> void; // NOLINT

        /**
         * @brief 获取原生句柄
         */
        rain_fn native_handle() noexcept -> native_handle_type;

        /**
         * @brief 获取 rainy-toolkit 抽象层 mtx_t 句柄指针
         *
         * 返回指向内部 mtx_t 成员的指针（即 void**），可直接传给
         * cnd_wait / cnd_timedwait 等同层 PAL 函数。
         *
         * @note 与 native_handle() 的区别：native_handle() 返回的是
         *       已解引用一层的底层 OS 句柄
         *       rainy-toolkit() 返回的是 PAL 句柄本身的地址（mtx_t*）。
         */
        rain_fn backend_handle() noexcept -> implements::mtx_t * {
            return &mtx_;
        }

    private:
        implements::mtx_t mtx_{nullptr};
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 延迟锁定标记
     */
    struct defer_lock_t {
        explicit defer_lock_t() = default;
    };

    /**
     * @brief 尝试锁定标记
     */
    struct try_to_lock_t {
        explicit try_to_lock_t() = default;
    };

    /**
     * @brief 已锁定标记
     */
    struct adopt_lock_t {
        explicit adopt_lock_t() = default;
    };

    inline constexpr defer_lock_t defer_lock{};
    inline constexpr try_to_lock_t try_to_lock{};
    inline constexpr adopt_lock_t adopt_lock{};
}

namespace rainy::foundation::concurrency {
    /**
     * @brief RAII 互斥量锁管理器
     *
     * 构造时锁定互斥量，析构时自动解锁。
     * 行为类似 std::lock_guard。
     * 不可复制，不可赋值。
     *
     * @tparam Mutex 互斥量类型
     */
    template <typename Mutex>
    class lock_guard {
    public:
        using mutex_type = Mutex;

        /**
         * @brief 构造并锁定互斥量
         *
         * 出错时调用互斥量自身的 terminate 逻辑。
         *
         * @param mutex 互斥量引用
         */
        explicit lock_guard(mutex_type &mutex) : pm(mutex) {
            pm.lock();
        }

        /**
         * @brief 构造但假定互斥量已经锁定
         *
         * 使用 adopt_lock_t 标记。
         */
        lock_guard(mutex_type &mutex, adopt_lock_t) : pm(mutex) {
            // assume already locked
        }

        /**
         * @brief 析构时解锁互斥量
         */
        ~lock_guard() {
            pm.unlock();
        }

        lock_guard(const lock_guard &) = delete;
        auto operator=(const lock_guard &) -> lock_guard & = delete;

    private:
        mutex_type &pm;
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename... Mutexes>
    void unlock_first_n(const int n, Mutexes &...mutex) {
        int count = 0;
        // 只解锁前 n 个
        ((void) ([&]() {
             if (count < n) {
                 mutex.unlock();
                 ++count;
             }
         }()),
         ...);
    }

    template <typename... Mutexes>
    void lock_all(Mutexes &...mutex) {
        int locked_count = 0;
        auto try_lock_all = [&]() {
            locked_count = 0;
            bool all_locked = true;
            ((void) ([&]() {
                 if (all_locked) {
                     if (mutex.try_lock()) {
                         ++locked_count;
                     } else {
                         all_locked = false;
                     }
                 }
             }()),
             ...);
            return all_locked;
        };
        while (!try_lock_all()) {
            unlock_first_n(locked_count, mutex...);
            std::this_thread::yield();
        }
    }

    template <typename Tuple, size_t... I>
    void unlock_tuple(Tuple &time_point, std::index_sequence<I...>) {
        (std::get<I>(time_point).unlock(), ...);
    }
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 多互斥量 RAII 管理器
     *
     * 构造时锁定所有互斥量，析构时解锁。
     * 行为类似 std::scoped_lock。
     * 不可复制，不可赋值。
     *
     * @tparam MutexTypes 互斥量类型列表
     */
    template <typename... MutexTypes>
    class scoped_lock {
    public:
        /**
         * @brief 构造并锁定所有互斥量
         *
         * 出错时调用互斥量自身的 terminate 逻辑。
         *
         * @param mutex 多个互斥量引用
         */
        explicit scoped_lock(MutexTypes &...mutex) : pm(mutex...) {
            implements::lock_all(mutex...);
        }

        /**
         * @brief 构造但假定互斥量已经锁定
         *
         * 使用 adopt_lock_t 标记。
         */
        explicit scoped_lock(adopt_lock_t, MutexTypes &...mutex) : pm(mutex...) {
            // assume already locked
        }

        /**
         * @brief 析构时解锁所有互斥量
         */
        ~scoped_lock() {
            implements::unlock_tuple(pm, std::index_sequence_for<MutexTypes...>{});
        }

        scoped_lock(const scoped_lock &) = delete;
        auto operator=(const scoped_lock &) -> scoped_lock & = delete;

    private:
        std::tuple<MutexTypes &...> pm;
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 可拥有互斥量所有权的 RAII 管理器
     *
     * 与 lock_guard 不同，可延迟锁定、尝试锁定、释放互斥量。
     * 行为类似 std::unique_lock。
     *
     * @tparam Mutex 互斥量类型
     */
    template <typename Mutex>
    class unique_lock {
    public:
        using mutex_type = Mutex;

        /**
         * @brief 默认构造，不拥有互斥量
         */
        unique_lock() noexcept : pm(nullptr), owns(false) {
        }

        /**
         * @brief 构造并锁定互斥量
         */
        explicit unique_lock(mutex_type &mutex) : pm(&mutex), owns(false) {
            pm->lock();
            owns = true; // NOLINT
        }

        /**
         * @brief 延迟锁定构造
         */
        unique_lock(mutex_type &mutex, defer_lock_t) noexcept : pm(&mutex), owns(false) {
        }

        /**
         * @brief 尝试锁定构造
         */
        unique_lock(mutex_type &mutex, try_to_lock_t) : pm(&mutex), owns(mutex.try_lock()) {
        }

        /**
         * @brief 构造假定互斥量已经锁定
         */
        unique_lock(mutex_type &mutex, adopt_lock_t) : pm(&mutex), owns(true) {
        }

        /**
         * @brief 在指定时间点尝试锁定
         */
        template <typename Clock, typename Duration>
        unique_lock(mutex_type &mutex, const std::chrono::time_point<Clock, Duration> &time_point) : pm(&mutex) {
            owns = pm->try_lock_until(time_point); // NOLINT
        }

        /**
         * @brief 在指定时间段内尝试锁定
         */
        template <typename Rep, typename Period>
        unique_lock(mutex_type &mutex, const std::chrono::duration<Rep, Period> &duration) : pm(&mutex) {
            owns = pm->try_lock_for(duration); // NOLINT
        }

        /**
         * @brief 析构时解锁互斥量
         */
        ~unique_lock() {
            if (owns && pm) {
                pm->unlock();
            }
        }

        /**
         * @brief 移动构造
         */
        unique_lock(unique_lock &&right) noexcept : pm(right.pm), owns(right.owns) {
            right.pm = nullptr;
            right.owns = false;
        }

        /**
         * @brief 移动赋值
         */
        rain_fn operator=(unique_lock &&right)->unique_lock & { // NOLINT
            if (this != &right) {
                if (owns && pm) {
                    pm->unlock();
                }
                pm = right.pm;
                owns = right.owns;

                right.pm = nullptr;
                right.owns = false;
            }
            return *this;
        }

        unique_lock(const unique_lock &) = delete;
        rain_fn operator=(const unique_lock &)->unique_lock & = delete;

        /**
         * @brief 锁定互斥量
         */
        rain_fn lock() -> void {
            pm->lock();
            owns = true;
        }

        /**
         * @brief 尝试锁定互斥量
         */
        RAINY_NODISCARD rain_fn try_lock() -> bool {
            owns = pm->try_lock();
            return owns;
        }

        /**
         * @brief 在给定时间段内尝试锁定
         */
        template <class Rep, class Period>
        RAINY_NODISCARD rain_fn try_lock_for(const std::chrono::duration<Rep, Period> &duration) -> bool {
            owns = pm->try_lock_for(duration);
            return owns;
        }

        /**
         * @brief 在给定时间点前尝试锁定
         */
        template <class Clock, class Duration>
        RAINY_NODISCARD rain_fn try_lock_until(const std::chrono::time_point<Clock, Duration> &time_point) -> bool {
            owns = pm->try_lock_until(time_point);
            return owns;
        }

        /**
         * @brief 解锁互斥量
         */
        rain_fn unlock() -> void {
            pm->unlock();
            owns = false;
        }

        /**
         * @brief 交换两个 unique_lock
         */
        rain_fn swap(unique_lock &right) noexcept -> void {
            std::swap(pm, right.pm);
            std::swap(owns, right.owns);
        }

        /**
         * @brief 释放互斥量所有权，不解锁
         */
        RAINY_NODISCARD
        rain_fn release() noexcept -> mutex_type * {
            owns = false;
            return std::exchange(pm, nullptr);
        }

        /**
         * @brief 是否拥有锁
         */
        RAINY_NODISCARD rain_fn owns_lock() const noexcept -> bool {
            return owns;
        }

        /**
         * @brief 显式 bool 转换，等同于 owns_lock()
         */
        explicit operator bool() const noexcept {
            return owns;
        }

        /**
         * @brief 获取底层互斥量指针
         */
        RAINY_NODISCARD rain_fn mutex() const noexcept -> mutex_type * {
            return pm;
        }

    private:
        mutex_type *pm;
        bool owns;
    };

    /**
     * @brief 交换两个 unique_lock 对象
     */
    template <typename Mutex>
    rain_fn swap(unique_lock<Mutex> &left, unique_lock<Mutex> &right) noexcept -> void {
        left.swap(right);
    }
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 一次性执行标记
     *
     * 用于保证某个操作在多线程环境下只执行一次。
     */
    struct once_flag {
        once_flag() noexcept = default;

        once_flag(const once_flag &) = delete;
        auto operator=(const once_flag &) -> once_flag & = delete;

        std::atomic<bool> called{false};
        mutex mtx;
    };
}

namespace rainy::foundation::concurrency {

    /**
     * @brief 在给定互斥量保护下调用函数
     *
     * 自动锁定并解锁互斥量，返回调用结果。
     *
     * @tparam Mutex 互斥量类型
     * @tparam Fx 可调用对象类型
     * @tparam Args 参数类型列表
     * @param mtx 互斥量
     * @param callable 可调用对象
     * @param args 可调用对象参数
     * @return 调用结果
     */
    template <typename Mutex, typename Fx, typename... Args>
    rain_fn synchronized_invoke(Mutex &mtx, Fx &&callable, Args &&...args) noexcept(
        noexcept(utility::invoke(utility::forward<Fx>(callable), utility::forward<Args>(args)...))) -> decltype(auto) {

        lock_guard<Mutex> guard{mtx};
        return utility::invoke(utility::forward<Fx>(callable), utility::forward<Args>(args)...);
    }

    /**
     * @brief 保证函数在多线程环境下只调用一次
     *
     * @tparam Callable 可调用对象类型
     * @tparam Args 参数类型列表
     * @param flag once_flag 实例
     * @param func 可调用对象
     * @param args 可调用对象参数
     */
    template <class Callable, class... Args>
    rain_fn call_once(once_flag &flag, Callable &&func, Args &&...args) -> void {
        if (flag.called.load(std::memory_order_acquire)) {
            return;
        }

        lock_guard guard(flag.mtx);

        if (flag.called.load(std::memory_order_relaxed)) {
            return;
        }

        // NOLINTBEGIN
        try {
            utility::invoke(utility::forward<Callable>(func), utility::forward<Args>(args)...);
            // publish success
            flag.called.store(true, std::memory_order_release);
        } catch (...) {
            throw;
        }
        // NOLINTEND
    }
}

#endif
