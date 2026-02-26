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
#ifndef RAINY_FOUNDATION_CONCURRENCY_SHARED_MUTEX_HPP
#define RAINY_FOUNDATION_CONCURRENCY_SHARED_MUTEX_HPP
#include <chrono>
#include <mutex>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <rainy/foundation/concurrency/pal.hpp>
#include <system_error>

namespace rainy::foundation::concurrency {
    /**
     * @brief 共享互斥锁
     *
     * 支持独占和共享两种锁定模式。
     * 独占模式与所有锁互斥，共享模式允许多个读者同时访问。
     */
    class shared_mutex {
    public:
        using native_handle_type = void *;

        shared_mutex() {
            auto result = implements::smtx_create(&handle_);
            if (result != thrd_result::success) {
                throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again),
                                        "shared_mutex constructor failed");
            }
        }

        ~shared_mutex() {
            implements::smtx_destroy(&handle_);
        }

        shared_mutex(const shared_mutex &) = delete;
        shared_mutex &operator=(const shared_mutex &) = delete;

        // exclusive ownership
        void lock() {
            auto result = implements::smtx_lock(&handle_);
            if (result != thrd_result::success) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur), "shared_mutex::lock failed");
            }
        }

        bool try_lock() {
            auto result = implements::smtx_try_lock(&handle_);
            if (result == thrd_result::success) {
                return true;
            } else if (result == thrd_result::busy) {
                return false;
            } else {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted), "shared_mutex::try_lock failed");
            }
        }

        void unlock() {
            auto result = implements::smtx_unlock(&handle_);
            if (result != thrd_result::success) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted), "shared_mutex::unlock failed");
            }
        }

        // shared ownership
        void lock_shared() {
            auto result = implements::smtx_lock_shared(&handle_);
            if (result != thrd_result::success) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur),
                                        "shared_mutex::lock_shared failed");
            }
        }

        bool try_lock_shared() {
            auto result = implements::smtx_try_lock_shared(&handle_);
            if (result == thrd_result::success) {
                return true;
            } else if (result == thrd_result::busy) {
                return false;
            } else {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted),
                                        "shared_mutex::try_lock_shared failed");
            }
        }

        void unlock_shared() {
            auto result = implements::smtx_unlock_shared(&handle_);
            if (result != thrd_result::success) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted),
                                        "shared_mutex::unlock_shared failed");
            }
        }

        native_handle_type native_handle() {
            return implements::native_smtx_handle(&handle_);
        }

    private:
        implements::smtx_t handle_{nullptr};
    };

    /**
     * @brief 支持超时的共享互斥锁
     *
     * 使用条件变量实现超时等待机制。
     * 维护读者计数和写者状态，通过条件变量协调线程间的同步。
     */
    class shared_timed_mutex {
    public:
        shared_timed_mutex() : state_(0), exclusive_waiting_(0), shared_waiting_(0) {
        }

        ~shared_timed_mutex() = default;

        shared_timed_mutex(const shared_timed_mutex &) = delete;
        shared_timed_mutex &operator=(const shared_timed_mutex &) = delete;

        // exclusive ownership
        void lock() {
            std::unique_lock<std::mutex> lk(mtx_);
            ++exclusive_waiting_;
            // 等待没有读者和写者
            while (state_ != 0) {
                writer_queue.wait(lk);
            }
            --exclusive_waiting_;
            state_ = write_entered_;
        }

        bool try_lock() {
            std::unique_lock<std::mutex> lk(mtx_, std::try_to_lock);
            if (!lk.owns_lock()) {
                return false;
            }
            if (state_ == 0) {
                state_ = write_entered_;
                return true;
            }
            return false;
        }

        template <class Rep, class Period>
        bool try_lock_for(const std::chrono::duration<Rep, Period> &rel_time) {
            return try_lock_until(std::chrono::steady_clock::now() + rel_time);
        }

        template <class Clock, class Duration>
        bool try_lock_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
            std::unique_lock<std::mutex> lk(mtx_);
            ++exclusive_waiting_;
            // 等待没有读者和写者，或超时
            while (state_ != 0) {
                if (writer_queue.wait_until(lk, abs_time) == std::cv_status::timeout) {
                    --exclusive_waiting_;
                    return false;
                }
            }
            --exclusive_waiting_;
            state_ = write_entered_;
            return true;
        }

        void unlock() {
            {
                std::lock_guard<std::mutex> lk(mtx_);
                state_ = 0;
            }
            // 优先唤醒等待的写者，如果没有则唤醒所有读者
            if (exclusive_waiting_ > 0) {
                writer_queue.notify_one();
            } else if (shared_waiting_ > 0) {
                reader_queue.notify_all();
            }
        }

        // shared ownership
        void lock_shared() {
            std::unique_lock<std::mutex> lk(mtx_);
            ++shared_waiting_;
            // 等待没有写者且没有等待的写者
            while ((state_ & write_entered_) || exclusive_waiting_ > 0) {
                reader_queue.wait(lk);
            }
            --shared_waiting_;
            ++state_; // 增加读者计数
        }

        bool try_lock_shared() {
            std::unique_lock<std::mutex> lk(mtx_, std::try_to_lock);
            if (!lk.owns_lock()) {
                return false;
            }
            // 没有写者且没有等待的写者
            if ((state_ & write_entered_) == 0 && exclusive_waiting_ == 0) {
                ++state_; // 增加读者计数
                return true;
            }
            return false;
        }

        template <class Rep, class Period>
        bool try_lock_shared_for(const std::chrono::duration<Rep, Period> &rel_time) {
            return try_lock_shared_until(std::chrono::steady_clock::now() + rel_time);
        }

        template <class Clock, class Duration>
        bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
            std::unique_lock<std::mutex> lk(mtx_);
            ++shared_waiting_;
            // 等待没有写者且没有等待的写者，或超时
            while ((state_ & write_entered_) || exclusive_waiting_ > 0) {
                if (reader_queue.wait_until(lk, abs_time) == std::cv_status::timeout) {
                    --shared_waiting_;
                    return false;
                }
            }
            --shared_waiting_;
            ++state_; // 增加读者计数
            return true;
        }

        void unlock_shared() {
            bool notify = false;
            {
                std::lock_guard<std::mutex> lk(mtx_);
                --state_; // 减少读者计数
                // 如果是最后一个读者且有等待的写者
                if ((state_ & ~write_entered_) == 0 && exclusive_waiting_ > 0) {
                    notify = true;
                }
            }
            // 唤醒一个等待的写者
            if (notify) {
                writer_queue.notify_one();
            }
        }

    private:
        std::mutex mtx_;
        std::condition_variable writer_queue; // 写者等待队列
        std::condition_variable reader_queue; // 读者等待队列

        unsigned int state_; // 状态字：最高位表示写者，其余位表示读者数量
        unsigned int exclusive_waiting_; // 等待的写者数量
        unsigned int shared_waiting_; // 等待的读者数量

        static constexpr unsigned write_entered_ = 1U << (sizeof(unsigned) * 8 - 1);
    };

    /**
     * @brief RAII 风格的共享锁
     *
     * 自动管理共享锁的获取和释放。
     */
    template <class Mutex>
    class shared_lock {
    public:
        using mutex_type = Mutex;

        // construct/copy/destroy
        shared_lock() noexcept : pm_(nullptr), owns_(false) {
        }

        explicit shared_lock(mutex_type &m) : pm_(&m), owns_(false) {
            lock();
            owns_ = true;
        }

        shared_lock(mutex_type &m, std::defer_lock_t) noexcept : pm_(&m), owns_(false) {
        }

        shared_lock(mutex_type &m, std::try_to_lock_t) : pm_(&m), owns_(false) {
            owns_ = pm_->try_lock_shared();
        }

        shared_lock(mutex_type &m, std::adopt_lock_t) noexcept : pm_(&m), owns_(true) {
        }

        template <class Clock, class Duration>
        shared_lock(mutex_type &m, const std::chrono::time_point<Clock, Duration> &abs_time) : pm_(&m), owns_(false) {
            owns_ = pm_->try_lock_shared_until(abs_time);
        }

        template <class Rep, class Period>
        shared_lock(mutex_type &m, const std::chrono::duration<Rep, Period> &rel_time) : pm_(&m), owns_(false) {
            owns_ = pm_->try_lock_shared_for(rel_time);
        }

        ~shared_lock() {
            if (owns_) {
                pm_->unlock_shared();
            }
        }

        shared_lock(const shared_lock &) = delete;
        shared_lock &operator=(const shared_lock &) = delete;

        shared_lock(shared_lock &&u) noexcept : pm_(u.pm_), owns_(u.owns_) {
            u.pm_ = nullptr;
            u.owns_ = false;
        }

        shared_lock &operator=(shared_lock &&u) noexcept {
            if (owns_) {
                pm_->unlock_shared();
            }
            pm_ = u.pm_;
            owns_ = u.owns_;
            u.pm_ = nullptr;
            u.owns_ = false;
            return *this;
        }

        // locking
        void lock() {
            if (pm_ == nullptr) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted), "shared_lock::lock: no mutex");
            }
            if (owns_) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur),
                                        "shared_lock::lock: already locked");
            }
            pm_->lock_shared();
            owns_ = true;
        }

        bool try_lock() {
            if (pm_ == nullptr) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted), "shared_lock::try_lock: no mutex");
            }
            if (owns_) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur),
                                        "shared_lock::try_lock: already locked");
            }
            owns_ = pm_->try_lock_shared();
            return owns_;
        }

        template <class Rep, class Period>
        bool try_lock_for(const std::chrono::duration<Rep, Period> &rel_time) {
            if (pm_ == nullptr) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted),
                                        "shared_lock::try_lock_for: no mutex");
            }
            if (owns_) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur),
                                        "shared_lock::try_lock_for: already locked");
            }
            owns_ = pm_->try_lock_shared_for(rel_time);
            return owns_;
        }

        template <class Clock, class Duration>
        bool try_lock_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
            if (pm_ == nullptr) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted),
                                        "shared_lock::try_lock_until: no mutex");
            }
            if (owns_) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur),
                                        "shared_lock::try_lock_until: already locked");
            }
            owns_ = pm_->try_lock_shared_until(abs_time);
            return owns_;
        }

        void unlock() {
            if (!owns_) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted), "shared_lock::unlock: not locked");
            }
            pm_->unlock_shared();
            owns_ = false;
        }

        // modifiers
        void swap(shared_lock &u) noexcept {
            std::swap(pm_, u.pm_);
            std::swap(owns_, u.owns_);
        }

        mutex_type *release() noexcept {
            mutex_type *ret = pm_;
            pm_ = nullptr;
            owns_ = false;
            return ret;
        }

        // observers
        bool owns_lock() const noexcept {
            return owns_;
        }

        explicit operator bool() const noexcept {
            return owns_;
        }

        mutex_type *mutex() const noexcept {
            return pm_;
        }

    private:
        mutex_type *pm_;
        bool owns_;
    };

    template <class Mutex>
    void swap(shared_lock<Mutex> &left, shared_lock<Mutex> &right) noexcept {
        left.swap(right);
    }
}

#endif