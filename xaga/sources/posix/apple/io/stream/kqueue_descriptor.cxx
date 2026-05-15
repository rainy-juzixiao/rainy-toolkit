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
#include <rainy/foundation/concurrency/executor.hpp>
#include <rainy/foundation/io/executor/implements/io_context.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <sys/event.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

namespace rainy::foundation::io::stream::implements {
    using io::implements::op_result;

    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    static concurrency::executor &get_executor() noexcept {
        return concurrency::get_global_pooled_executor();
    }

    struct kqueue_stream_proxy : io_context::executor_type {
        using executor_type::post_immediate_completion;
    };

    class kqueue_descriptor_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit kqueue_descriptor_impl(executor_type executor) noexcept : executor_(executor) {
        }

        ~kqueue_descriptor_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return fd_ >= 0;
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(fd_);
        }

        std::error_code attach(native_handle_type handle) noexcept override {
            if (is_open()) {
                return posix_error(EBUSY);
            }
            fd_ = static_cast<int>(handle);
            cancel_flag_.store(false, std::memory_order_relaxed);
            return {};
        }

        native_handle_type release() noexcept override {
            int fd = fd_;
            fd_ = -1;
            wants_read_ = wants_write_ = false;
            return static_cast<native_handle_type>(fd);
        }

        std::error_code attach_from(descriptor_impl_base *other) noexcept override {
            if (!other) {
                return posix_error(EINVAL);
            }
            if (is_open()) {
                return posix_error(EBUSY);
            }
            return attach(other->release());
        }

        std::error_code close() noexcept override {
            if (fd_ < 0) {
                return posix_error(EBADF);
            }
            cancel_flag_.store(true, std::memory_order_release);
            if (fd_ == STDIN_FILENO || fd_ == STDOUT_FILENO || fd_ == STDERR_FILENO) {
                fd_ = -1;
                wants_read_ = wants_write_ = false;
                return {};
            }
            const int ret = ::close(fd_);
            fd_ = -1;
            wants_read_ = wants_write_ = false;
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code cancel() noexcept override {
            cancel_flag_.store(true, std::memory_order_release);
            return {};
        }

        std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (fd_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            const ::ssize_t n = ::read(fd_, buf, len);
            if (n < 0) {
                // EAGAIN/EWOULDBLOCK：非阻塞 fd 暂无数据，不是真正错误
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    ec.clear();
                    return 0;
                }
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (fd_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            const ::ssize_t n = ::write(fd_, buf, len);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    ec.clear();
                    return 0;
                }
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        void async_read_some(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (fd_ < 0) {
                op_result result{};
                result.user_data = op;
                result.error_code = EBADF;
                result.bytes_transferred = 0;
                op->complete(result, false);
                return;
            }
            wants_read_ = true;
            int fd = fd_;
            std::atomic<bool> *flag = &cancel_flag_;
            get_executor().submit([fd, buf, len, op, flag]() mutable {
                op_result result{};
                result.user_data = op;
                if (flag->load(std::memory_order_acquire)) {
                    result.error_code = ECANCELED;
                    result.bytes_transferred = 0;
                    op->complete(result, false);
                    return;
                }
                const ::ssize_t n = ::read(fd, buf, len);
                if (n < 0) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(n);
                }
                op->complete(result, false);
            });
        }

        void async_write_some(const void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (fd_ < 0) {
                op_result result{};
                result.user_data = op;
                result.error_code = EBADF;
                result.bytes_transferred = 0;
                op->complete(result, false);
                return;
            }
            wants_write_ = true;
            int fd = fd_;
            std::atomic<bool> *flag = &cancel_flag_;
            get_executor().submit([fd, buf, len, op, flag]() mutable {
                op_result result{};
                result.user_data = op;
                if (flag->load(std::memory_order_acquire)) {
                    result.error_code = ECANCELED;
                    result.bytes_transferred = 0;
                    op->complete(result, false);
                    return;
                }
                const ::ssize_t n = ::write(fd, buf, len);
                if (n < 0) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(n);
                }
                op->complete(result, false);
            });
        }

        bool wants_read() const noexcept override {
            return wants_read_;
        }
        bool wants_write() const noexcept override {
            return wants_write_;
        }

        void reset_operation() noexcept override {
            wants_read_ = false;
            wants_write_ = false;
            cancel_flag_.store(false, std::memory_order_release);
        }

    private:
        int fd_{-1};
        executor_type executor_;
        std::atomic<bool> cancel_flag_{false};
        bool wants_read_{false};
        bool wants_write_{false};
    };

    using kqueue_anon_pipe_impl = kqueue_descriptor_impl;

    memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl(executor_type executor) {
        return memory::make_nebula<kqueue_descriptor_impl>(executor);
    }

    memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl_from_native(executor_type executor, native_handle_type handle) {
        auto impl = memory::make_nebula<kqueue_descriptor_impl>(executor);
        static_cast<void>(impl->attach(handle));
        return impl;
    }

    memory::nebula_ptr<descriptor_impl_base> create_console_impl(executor_type executor, console_stream_kind kind) {
        int fd = -1;
        switch (kind) {
            case console_stream_kind::input:
                fd = STDIN_FILENO;
                break;
            case console_stream_kind::output:
                fd = STDOUT_FILENO;
                break;
            case console_stream_kind::error:
                fd = STDERR_FILENO;
                break;
            default:
                return create_null_impl(executor);
        }
        const int duped = ::dup(fd);
        auto impl = memory::make_nebula<kqueue_descriptor_impl>(executor);
        if (duped >= 0) {
            static_cast<void>(impl->attach(static_cast<native_handle_type>(duped)));
        }
        return impl;
    }

    utility::pair<memory::nebula_ptr<descriptor_impl_base>, memory::nebula_ptr<descriptor_impl_base>> create_pipe_impl(
        executor_type executor, std::error_code &ec) {
        int fds[2] = {-1, -1};
        if (::pipe(fds) != 0) {
            ec = posix_error();
            return {};
        }
        for (int i = 0; i < 2; ++i) {
            int flags = ::fcntl(fds[i], F_GETFD, 0);
            if (flags < 0) {
                ec = posix_error();
                ::close(fds[0]);
                ::close(fds[1]);
                return {};
            }
            if (::fcntl(fds[i], F_SETFD, flags | FD_CLOEXEC) < 0) {
                ec = posix_error();
                ::close(fds[0]);
                ::close(fds[1]);
                return {};
            }
        }
        auto read_impl = memory::make_nebula<kqueue_anon_pipe_impl>(executor);
        auto write_impl = memory::make_nebula<kqueue_anon_pipe_impl>(executor);
        static_cast<void>(read_impl->attach(static_cast<native_handle_type>(fds[0])));
        static_cast<void>(write_impl->attach(static_cast<native_handle_type>(fds[1])));
        ec.clear();
        return {std::move(read_impl), std::move(write_impl)};
    }

    memory::nebula_ptr<descriptor_impl_base> create_named_pipe_server_impl(executor_type executor, const char *name,
                                                                           pipe_direction dir, std::error_code &ec) {
        if (::mkfifo(name, 0600) != 0 && errno != EEXIST) {
            ec = posix_error();
            return create_null_impl(executor);
        }
        int flags = O_CLOEXEC;
        switch (dir) {
            case pipe_direction::in:
                flags |= O_RDONLY;
                break;
            case pipe_direction::out:
                flags |= O_WRONLY;
                break;
                // FIFO 不支持真正的双向，用 O_RDWR 在 Linux 有效但 POSIX 未定义
            case pipe_direction::inout:
                flags |= O_RDWR;
                break;
        }
        const int fd = ::open(name, flags | O_NONBLOCK);
        if (fd < 0) {
            ec = posix_error();
            return create_null_impl(executor);
        }
        {
            int fl = ::fcntl(fd, F_GETFL, 0);
            ::fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
        }
        auto impl = memory::make_nebula<kqueue_descriptor_impl>(executor);
        static_cast<void>(impl->attach(static_cast<native_handle_type>(fd)));
        ec.clear();
        return impl;
    }

    memory::nebula_ptr<descriptor_impl_base> create_named_pipe_client_impl(executor_type executor, const char *name,
                                                                           pipe_direction dir, std::error_code &ec) {
        int flags = O_CLOEXEC;
        switch (dir) {
            case pipe_direction::in:
                flags |= O_RDONLY;
                break;
            case pipe_direction::out:
                flags |= O_WRONLY;
                break;
            case pipe_direction::inout:
                flags |= O_RDWR;
                break;
        }

        const int fd = ::open(name, flags);
        if (fd < 0) {
            ec = posix_error();
            return create_null_impl(executor);
        }

        auto impl = memory::make_nebula<kqueue_descriptor_impl>(executor);
        static_cast<void>(impl->attach(static_cast<native_handle_type>(fd)));
        ec.clear();
        return impl;
    }
}
