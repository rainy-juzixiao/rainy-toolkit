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
#ifndef RAINY_FOUNDATION_CONCURRENCY_CONDITION_VARIABLE_HPP
#define RAINY_FOUNDATION_CONCURRENCY_CONDITION_VARIABLE_HPP
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/concurrency/pal.hpp>

namespace rainy::foundation::concurrency {
    enum class cv_status {
        no_timeout,
        timeout
    };
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 条件变量封装类
     *
     * 基于 PAL 层的条件变量接口封装，提供与 std::condition_variable 兼容的接口。
     * 不可复制，不可移动。
     *
     * @note 所有 wait 系列函数要求调用方持有传入的 unique_lock 所有权（即锁已锁定）。
     */
    class condition_variable {
    public:
        using native_handle_type = void *;

        /**
         * @brief 默认构造函数
         *
         * 调用 PAL 层 cnd_create 分配并初始化底层条件变量资源。
         *
         * @throws std::system_error 若底层资源分配失败
         */
        condition_variable() {
            using namespace concurrency::implements;
            if (const thrd_result result = cnd_create(&cnd_); result != thrd_result::success) {
                throw std::system_error(static_cast<int>(result), std::system_category(), "condition_variable: cnd_create failed");
            }
        }

        /**
         * @brief 析构函数
         *
         * 调用 PAL 层 cnd_destroy 释放底层资源。
         *
         * @warning 若仍有线程在此条件变量上等待，则行为未定义。
         */
        ~condition_variable() {
            implements::cnd_destroy(&cnd_);
        }

        condition_variable(const condition_variable &) = delete;
        rain_fn operator=(const condition_variable &)->condition_variable & = delete;

        /**
         * @brief 唤醒一个正在等待的线程
         *
         * 若无线程在等待，调用无效果。
         */
        rain_fn notify_one() noexcept -> void { // NOLINT
            implements::cnd_signal(&cnd_);
        }

        /**
         * @brief 唤醒所有正在等待的线程
         *
         * 若无线程在等待，调用无效果。
         */
        rain_fn notify_all() noexcept -> void { // NOLINT
            implements::cnd_broadcast(&cnd_);
        }

        /**
         * @brief 无条件等待，直到被唤醒
         *
         * 调用前 @p lock 必须处于锁定状态；函数内部会释放锁，被唤醒后重新获取。
         *
         * @param lock 持有关联 mutex 的 unique_lock
         * @throws std::system_error 若底层等待调用返回错误
         */
        rain_fn wait(unique_lock<mutex> &lock) -> void { // NOLINT
            using namespace concurrency::implements;
            mtx_t *const mtx = lock.mutex()->backend_handle();
            if (const thrd_result result = cnd_wait(&cnd_, mtx); result != thrd_result::success) {
                throw std::system_error(static_cast<int>(result), std::system_category(), "condition_variable::wait failed");
            }
        }

        /**
         * @brief 带谓词的等待，直到谓词返回 true
         *
         * 等价于 `while (!pred()) wait(lock);`，能够正确处理虚假唤醒。
         *
         * @tparam Pred 可调用类型，签名为 `bool()`
         * @param lock 持有关联 mutex 的 unique_lock
         * @param pred 等待条件谓词
         */
        template <typename Pred>
        rain_fn wait(unique_lock<mutex> &lock, Pred pred) -> void {
            while (!pred()) {
                wait(lock);
            }
        }

        /**
         * @brief 等待直到指定的绝对时间点
         *
         * @tparam Clock    时钟类型
         * @tparam Duration 时间段类型
         * @param lock     持有关联 mutex 的 unique_lock
         * @param abs_time 绝对超时时间点
         * @return
         *  - cv_status::no_timeout 被正常唤醒
         *  - cv_status::timeout    超时返回
         * @throws std::system_error 若底层调用返回非超时错误
         */
        template <typename Clock, typename Duration>
        rain_fn wait_until(unique_lock<mutex> &lock, const std::chrono::time_point<Clock, Duration> &abs_time) -> cv_status {
            using namespace concurrency::implements;
            for (;;) {
                const auto now = Clock::now();
                if (now >= abs_time) {
                    return cv_status::timeout;
                }
                const ::timespec ts = to_timespec(abs_time);
                const thrd_result r = implements::cnd_timedwait(&cnd_, lock.mutex()->backend_handle(), &ts);
                if (r == thrd_result::success) {
                    return cv_status::no_timeout;
                }
                if (r != thrd_result::timed_out) {
                    throw std::system_error(static_cast<int>(r), std::system_category(), "condition_variable_any::wait_until failed");
                }
            }
        }

        /**
         * @brief 带谓词的绝对超时等待
         *
         * 等价于 `while (!pred()) { if (wait_until(lock, abs_time) == timeout) return pred(); } return true;`
         *
         * @tparam Clock    时钟类型
         * @tparam Duration 时间段类型
         * @tparam Pred     可调用类型，签名为 `bool()`
         * @param lock     持有关联 mutex 的 unique_lock
         * @param abs_time 绝对超时时间点
         * @param pred     等待条件谓词
         * @return 超时时返回 pred() 的当前值；被唤醒且 pred() 为 true 时返回 true
         */
        template <typename Clock, typename Duration, typename Pred>
        rain_fn wait_until(unique_lock<mutex> &lock, const std::chrono::time_point<Clock, Duration> &abs_time, Pred pred) -> bool {
            while (!pred()) {
                if (wait_until(lock, abs_time) == cv_status::timeout) {
                    return pred();
                }
            }
            return true;
        }

        /**
         * @brief 等待指定的相对时间段
         *
         * 内部将 rel_time 转换为绝对时间点后委托给 wait_until。
         *
         * @tparam Rep    时间量的数值类型
         * @tparam Period 时间量的周期类型
         * @param lock     持有关联 mutex 的 unique_lock
         * @param rel_time 相对等待时长
         * @return
         *  - cv_status::no_timeout 被正常唤醒
         *  - cv_status::timeout    超时返回
         */
        template <typename Rep, typename Period>
        rain_fn wait_for(unique_lock<mutex> &lock, const std::chrono::duration<Rep, Period> &rel_time) -> cv_status {
            return wait_until(lock, std::chrono::system_clock::now() + rel_time);
        }

        /**
         * @brief 带谓词的相对超时等待
         *
         * 内部将 rel_time 转换为绝对时间点后委托给带谓词的 wait_until。
         *
         * @tparam Rep    时间量的数值类型
         * @tparam Period 时间量的周期类型
         * @tparam Pred   可调用类型，签名为 `bool()`
         * @param lock     持有关联 mutex 的 unique_lock
         * @param rel_time 相对等待时长
         * @param pred     等待条件谓词
         * @return 超时时返回 pred() 的当前值；被唤醒且 pred() 为 true 时返回 true
         */
        template <typename Rep, typename Period, typename Pred>
        rain_fn wait_for(unique_lock<mutex> &lock, const std::chrono::duration<Rep, Period> &rel_time, Pred pred) -> bool {
            return wait_until(lock, std::chrono::system_clock::now() + rel_time, utility::move(pred));
        }

        /**
         * @brief 获取底层平台条件变量的原生句柄
         *
         * @return 指向底层实现的不透明指针
         */
        rain_fn native_handle() -> native_handle_type { // NOLINT
            return implements::native_cnd_handle(&cnd_);
        }

    private:
        template <typename Clock, typename Duration>
        rain_fn to_timespec(const std::chrono::time_point<Clock, Duration> &tp) noexcept -> ::timespec {
            ::timespec ts{};
            if constexpr (std::is_same_v<Clock, std::chrono::system_clock>) {
                const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(tp);
                const auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp) -
                                std::chrono::time_point_cast<std::chrono::nanoseconds>(sec);
                ts.tv_sec = std::chrono::system_clock::to_time_t(sec);
                ts.tv_nsec = static_cast<long>(ns.count());
                return ts;
            } else {
                const auto now_sys = std::chrono::system_clock::now();
                const auto now_clock = Clock::now();
                const auto delta = tp - now_clock;
                const auto sys_tp = now_sys + delta;
                const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(sys_tp);
                const auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(sys_tp) -
                                std::chrono::time_point_cast<std::chrono::nanoseconds>(sec);
                ts.tv_sec = std::chrono::system_clock::to_time_t(sec);
                ts.tv_nsec = static_cast<long>(ns.count());
                return ts;
            }
        }

        implements::cnd_t cnd_{};
    };
}

namespace rainy::foundation::concurrency {
    class condition_variable_any {
    public:
        using native_handle_type = void *;

        condition_variable_any() : mtx_(std::make_shared<mutex>()) {
            using namespace implements;
            if (const thrd_result r = cnd_create(&cnd_); r != thrd_result::success) {
                throw std::system_error(static_cast<int>(r), std::system_category(), "condition_variable_any: cnd_create failed");
            }
        }

        ~condition_variable_any() {
            implements::cnd_destroy(&cnd_);
        }

        condition_variable_any(const condition_variable_any &) = delete;
        rain_fn operator=(const condition_variable_any &)->condition_variable_any & = delete;

        /**
         * @brief 唤醒一个正在等待的线程
         */
        rain_fn notify_one() noexcept -> void { // NOLINT
            implements::cnd_signal(&cnd_);
        }

        /**
         * @brief 唤醒所有正在等待的线程
         */
        rain_fn notify_all() noexcept -> void { // NOLINT
            implements::cnd_broadcast(&cnd_);
        }

        /**
         * @brief 无条件等待，直到被唤醒
         *
         * 持有内部锁期间释放外部锁并进入 cnd_wait，利用 cnd_wait 的原子
         * 语义（释放内部锁 + 挂起是原子的）保证不丢失 notify 信号。
         *
         * @tparam Lock BasicLockable 类型
         * @param lock  调用方持有的锁，必须处于锁定状态
         * @throws std::system_error 若底层等待失败
         */
        template <typename Lock>
        rain_fn wait(Lock &lock) -> void {
            // 延长内部锁生命期
            const std::shared_ptr<mutex> mtx = mtx_;

            // (1) 拿内部锁
            //     此后 notify 可以随时发出（notify 不持内部锁），
            //     但 cnd_wait 会原子地释放内部锁并挂起，
            //     保证进入等待队列和释放内部锁之间不丢信号
            unique_lock<mutex> internal_lk(*mtx);

            // (2) 释放外部锁（在持有内部锁期间）
            lock.unlock();

            // (3) 原子地释放内部锁并挂起；唤醒后重新持有内部锁
            const thrd_result r = implements::cnd_wait(&cnd_, internal_lk.mutex()->backend_handle());

            // (4) internal_lk 析构，释放内部锁

            // (5) 重拿外部锁（internal_lk 析构后，避免持有内部锁时等外部锁）
            //     注意：internal_lk 在 lock.lock() 之前析构，
            //     所以需要先手动 unlock internal_lk
            internal_lk.unlock();
            lock.lock();

            if (r != thrd_result::success) {
                throw std::system_error(static_cast<int>(r), std::system_category(), "condition_variable_any::wait failed");
            }
        }

        /**
         * @brief 带谓词的等待，处理虚假唤醒
         *
         * @tparam Lock BasicLockable 类型
         * @tparam Pred 可调用类型，签名为 `bool()`
         */
        template <typename Lock, typename Pred>
        rain_fn wait(Lock &lock, Pred pred) -> void {
            while (!pred()) {
                wait(lock);
            }
        }

        /**
         * @brief 等待直到指定的绝对时间点
         *
         * @tparam Lock     BasicLockable 类型
         * @tparam Clock    时钟类型
         * @tparam Duration 时间段类型
         * @param lock     调用方持有的锁
         * @param abs_time 绝对超时时间点
         * @return cv_status::no_timeout 或 cv_status::timeout
         * @throws std::system_error 若底层等待失败（非超时错误）
         */
        template <typename Lock, typename Clock, typename Duration>
        rain_fn wait_until(Lock &lock, const std::chrono::time_point<Clock, Duration> &abs_time) -> cv_status {
            const std::shared_ptr<mutex> mtx = mtx_;
            unique_lock<mutex> internal_lk(*mtx);
            lock.unlock();
            for (;;) {
                const auto now = Clock::now();
                if (now >= abs_time) {
                    internal_lk.unlock();
                    lock.lock();
                    return cv_status::timeout;
                }
                const ::timespec ts = to_timespec(abs_time);
                const thrd_result r = implements::cnd_timedwait(&cnd_, internal_lk.mutex()->backend_handle(), &ts);
                if (r == thrd_result::success) {
                    internal_lk.unlock();
                    lock.lock();
                    return cv_status::no_timeout;
                }

                if (r != thrd_result::timed_out) {
                    internal_lk.unlock();
                    lock.lock();
                    throw std::system_error(static_cast<int>(r), std::system_category(), "condition_variable_any::wait_until failed");
                }
            }
        }

        /**
         * @brief 带谓词的绝对超时等待
         *
         * @tparam Lock     BasicLockable 类型
         * @tparam Clock    时钟类型
         * @tparam Duration 时间段类型
         * @tparam Pred     可调用类型，签名为 `bool()`
         * @return 谓词为 true 时返回 true；超时时返回 pred() 的当前值
         */
        template <typename Lock, typename Clock, typename Duration, typename Pred>
        rain_fn wait_until(Lock &lock, const std::chrono::time_point<Clock, Duration> &abs_time, Pred pred) -> bool {
            while (!pred()) {
                if (wait_until(lock, abs_time) == cv_status::timeout) {
                    return pred();
                }
            }
            return true;
        }

        /**
         * @brief 等待指定的相对时间段
         *
         * @tparam Lock   BasicLockable 类型
         * @tparam Rep    时间量的数值类型
         * @tparam Period 时间量的周期类型
         * @return cv_status::no_timeout 或 cv_status::timeout
         */
        template <typename Lock, typename Rep, typename Period>
        rain_fn wait_for(Lock &lock, const std::chrono::duration<Rep, Period> &rel_time) -> cv_status {
            return wait_until(lock, std::chrono::system_clock::now() + rel_time);
        }

        /**
         * @brief 带谓词的相对超时等待
         *
         * @tparam Lock   BasicLockable 类型
         * @tparam Rep    时间量的数值类型
         * @tparam Period 时间量的周期类型
         * @tparam Pred   可调用类型，签名为 `bool()`
         * @return 谓词为 true 时返回 true；超时时返回 pred() 的当前值
         */
        template <typename Lock, typename Rep, typename Period, typename Pred>
        rain_fn wait_for(Lock &lock, const std::chrono::duration<Rep, Period> &rel_time, Pred pred) -> bool {
            return wait_until(lock, std::chrono::system_clock::now() + rel_time, utility::move(pred));
        }

        /**
         * @brief 带中断支持的谓词等待
         *
         * @tparam Lock BasicLockable 类型
         * @tparam Pred 可调用类型，签名为 `bool()`
         * @param lock   调用方持有的锁
         * @param stoken 停止令牌
         * @param pred   等待条件谓词
         * @return pred() 为 true 时返回 true；停止请求且 pred() 为 false 时返回 false
         */
        template <typename Lock, typename Pred>
        rain_fn wait(Lock &lock, std::stop_token stoken, Pred pred) -> bool {
            std::stop_callback cb(stoken, [this] { notify_all(); });
            while (!stoken.stop_requested()) {
                if (pred()) {
                    return true;
                }
                wait(lock);
            }
            return pred();
        }

        /**
         * @brief 带中断支持的绝对超时谓词等待
         *
         * @tparam Lock     BasicLockable 类型
         * @tparam Clock    时钟类型
         * @tparam Duration 时间段类型
         * @tparam Pred     可调用类型，签名为 `bool()`
         * @return pred() 为 true 时返回 true；超时或停止请求且 pred() 为 false 时返回 false
         */
        template <typename Lock, typename Clock, typename Duration, typename Pred>
        rain_fn wait_until(Lock &lock, std::stop_token stoken, const std::chrono::time_point<Clock, Duration> &abs_time, Pred pred)
            -> bool {
            std::stop_callback cb(stoken, [this] { notify_all(); });
            while (!stoken.stop_requested()) {
                if (pred()) {
                    return true;
                }
                if (wait_until(lock, abs_time) == cv_status::timeout) {
                    return pred();
                }
            }
            return pred();
        }

        /**
         * @brief 带中断支持的相对超时谓词等待
         *
         * @tparam Lock   BasicLockable 类型
         * @tparam Rep    时间量的数值类型
         * @tparam Period 时间量的周期类型
         * @tparam Pred   可调用类型，签名为 `bool()`
         * @return pred() 为 true 时返回 true；超时或停止请求且 pred() 为 false 时返回 false
         */
        template <typename Lock, typename Rep, typename Period, typename Pred>
        rain_fn wait_for(Lock &lock, std::stop_token stoken, const std::chrono::duration<Rep, Period> &rel_time, Pred pred) -> bool {
            return wait_until(lock, utility::move(stoken), std::chrono::system_clock::now() + rel_time + std::chrono::milliseconds(1),
                              utility::move(pred));
        }

        /**
         * @brief 获取底层平台条件变量的原生句柄
         */
        rain_fn native_handle() -> native_handle_type { // NOLINT
            return implements::native_cnd_handle(&cnd_);
        }

    private:
        template <typename Clock, typename Duration>
        rain_fn to_timespec(const std::chrono::time_point<Clock, Duration> &tp) noexcept -> ::timespec {
            ::timespec ts{};
            if constexpr (std::is_same_v<Clock, std::chrono::system_clock>) {
                const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(tp);
                const auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp) -
                                std::chrono::time_point_cast<std::chrono::nanoseconds>(sec);
                ts.tv_sec = std::chrono::system_clock::to_time_t(sec);
                ts.tv_nsec = static_cast<long>(ns.count());
                return ts;
            } else {
                const auto now_sys = std::chrono::system_clock::now();
                const auto now_clock = Clock::now();
                const auto delta = tp - now_clock;
                const auto sys_tp = now_sys + delta;
                const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(sys_tp);
                const auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(sys_tp) -
                                std::chrono::time_point_cast<std::chrono::nanoseconds>(sec);
                ts.tv_sec = std::chrono::system_clock::to_time_t(sec);
                ts.tv_nsec = static_cast<long>(ns.count());
                return ts;
            }
        }

        std::shared_ptr<mutex> mtx_{};
        implements::cnd_t cnd_{};
    };
}

#endif
