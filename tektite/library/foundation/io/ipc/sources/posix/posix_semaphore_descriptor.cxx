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
#include <rainy/foundation/io/ipc/implements/semaphore_impl.hpp>
#include <cerrno>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

namespace rainy::foundation::io::ipc::semaphore::implements {
    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    class posix_semaphore_impl final : public semaphore_impl_base {
    public:
        posix_semaphore_impl(sem_t *sem) noexcept : sem_(sem) {
        }

        ~posix_semaphore_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return sem_ != nullptr && sem_ != SEM_FAILED;
        }

        std::error_code close() noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            if (::sem_close(sem_) == -1) {
                return posix_error();
            }
            sem_ = nullptr;
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(sem_);
        }

        std::error_code wait() noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            if (::sem_wait(sem_) == -1) {
                return posix_error();
            }
            return {};
        }

        std::error_code try_wait() noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            if (::sem_trywait(sem_) == -1) {
                return posix_error();
            }
            return {};
        }

        std::error_code timed_wait(std::chrono::milliseconds timeout) noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            struct timespec ts;
            struct timeval tv;
            ::gettimeofday(&tv, nullptr);
            auto total_ms = timeout.count();
            ts.tv_sec = tv.tv_sec + static_cast<time_t>(total_ms / 1000);
            ts.tv_nsec = static_cast<long>(tv.tv_usec) * 1000L + static_cast<long>(total_ms % 1000) * 1000000L;
            if (ts.tv_nsec >= 1000000000L) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000L;
            }
            if (::sem_timedwait(sem_, &ts) == -1) {
                return posix_error();
            }
            return {};
        }

        std::error_code post() noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            if (::sem_post(sem_) == -1) {
                return posix_error();
            }
            return {};
        }

    private:
        sem_t *sem_{nullptr};
    };

    class null_semaphore_impl final : public semaphore_impl_base {
    public:
        explicit null_semaphore_impl(executor_type) noexcept {
        }

        bool is_open() const noexcept override {
            return false;
        }

        std::error_code close() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(-1);
        }

        std::error_code wait() noexcept override {
            return {};
        }

        std::error_code try_wait() noexcept override {
            return {};
        }

        std::error_code timed_wait(std::chrono::milliseconds) noexcept override {
            return {};
        }

        std::error_code post() noexcept override {
            return {};
        }
    };

    core::memory::nebula_ptr<semaphore_impl_base>
    create_semaphore_impl(executor_type executor, const char *name, const attributes &attr, std::error_code &ec) {
        sem_t *sem = ::sem_open(name, O_CREAT | O_EXCL, 0600, attr.initial_value);
        if (sem == SEM_FAILED) {
            ec = posix_error();
            return create_null_semaphore_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_semaphore_impl>(sem);
    }

    core::memory::nebula_ptr<semaphore_impl_base>
    open_semaphore_impl(executor_type executor, const char *name, std::error_code &ec) {
        sem_t *sem = ::sem_open(name, 0);
        if (sem == SEM_FAILED) {
            ec = posix_error();
            return create_null_semaphore_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_semaphore_impl>(sem);
    }

    core::memory::nebula_ptr<semaphore_impl_base>
    open_or_create_semaphore_impl(executor_type executor, const char *name, const attributes &attr, std::error_code &ec) {
        sem_t *sem = ::sem_open(name, O_CREAT, 0600, attr.initial_value);
        if (sem == SEM_FAILED) {
            ec = posix_error();
            return create_null_semaphore_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_semaphore_impl>(sem);
    }

    core::memory::nebula_ptr<semaphore_impl_base> create_null_semaphore_impl(executor_type executor) {
        return core::memory::make_nebula<null_semaphore_impl>(executor);
    }

    std::error_code unlink_semaphore(const char *name) noexcept {
        if (::sem_unlink(name) == -1) {
            return posix_error();
        }
        return {};
    }
}