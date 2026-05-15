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
#include <rainy/foundation/io/executor/implements/io_context.hpp>
#include <rainy/foundation/concurrency/executor.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <liburing.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace rainy::foundation::io::stream::implements {
    using io::implements::op_result;

    static int is_same_fd(int fd1, int fd2) {
        struct stat st1, st2;
        if (fstat(fd1, &st1) == -1 || fstat(fd2, &st2) == -1) {
            return 0;
        }
        return (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino);
    }

    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    static concurrency::executor &get_executor() noexcept {
        return concurrency::get_global_pooled_executor();
    }

    struct linux_stream_proxy : io_context::executor_type {
        using executor_type::post_immediate_completion;

        io_uring_sqe *get_sqe(io::implements::completion_op *op, io_context::executor_type executor, int fd) noexcept {
            this->associate_handle(op, static_cast<std::uintptr_t>(fd), nullptr);
            if (!op->io_handle) {
                return nullptr;
            }
            return ::io_uring_get_sqe(static_cast<io_uring *>(op->io_handle));
        }
    };

    static io_uring_sqe *get_sqe(io::implements::completion_op *op, io_context::executor_type executor, int fd) noexcept {
        return linux_stream_proxy{executor}.get_sqe(op, executor, fd);
    }

    static void submit_ring(io::implements::completion_op *op) noexcept {
        if (op->io_handle) {
            ::io_uring_submit(static_cast<io_uring *>(op->io_handle));
        }
    }

    class linux_descriptor_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit linux_descriptor_impl(executor_type executor) noexcept : executor_(executor) {
        }

        ~linux_descriptor_impl() override {
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
            return {};
        }

        native_handle_type release() noexcept override {
            int fd = fd_;
            fd_ = -1;
            wants_read_ = false;
            wants_write_ = false;
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
            if (is_same_fd(fd_, STDOUT_FILENO) || is_same_fd(fd_, STDERR_FILENO) || is_same_fd(fd_, STDIN_FILENO)) {
                fd_ = -1;
                wants_read_ = wants_write_ = false;
                return {};
            }
            const int ret = ::close(fd_);
            fd_ = -1;
            wants_read_ = false;
            wants_write_ = false;
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code cancel() noexcept override {
            if (fd_ < 0) {
                return posix_error(EBADF);
            }
            return {};
        }

        std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (fd_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            const ::ssize_t n = ::read(fd_, buf, len);
            if (n < 0) {
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return n;
        }

        std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (fd_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            const ::ssize_t n = ::write(fd_, buf, len);
            if (n < 0) {
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return n;
        }

        void async_read_some(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (fd_ < 0) {
                linux_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            auto *sqe = get_sqe(op, executor, fd_);
            if (!sqe) {
                linux_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            wants_read_ = true;
            ::io_uring_prep_read(sqe, fd_, buf, static_cast<unsigned>(len), 0);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_write_some(const void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (fd_ < 0) {
                linux_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            auto *sqe = get_sqe(op, executor, fd_);
            if (!sqe) {
                linux_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            wants_write_ = true;
            if (is_same_fd(fd_, STDOUT_FILENO) || is_same_fd(fd_, STDERR_FILENO)) { // stdout,stderr需要通过线程池传递，否则回调无法触发
                int fd = fd_;
                get_executor().submit([fd, buf, len, op, this]() mutable {
                    op_result result{};
                    result.user_data = op;
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
            } else {
                ::io_uring_prep_write(sqe, fd_, buf, len, 0);
                ::io_uring_sqe_set_data(sqe, op);
                submit_ring(op);
            }
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
        }

    private:
        int fd_{-1};
        executor_type executor_;
        bool wants_read_{false};
        bool wants_write_{false};
    };

    class linux_anon_pipe_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit linux_anon_pipe_impl(executor_type executor) noexcept : executor_(executor) {
        }

        ~linux_anon_pipe_impl() override {
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
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        void async_read_some(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (fd_ < 0) {
                linux_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            auto *sqe = get_sqe(op, executor, fd_);
            if (sqe) {
                wants_read_ = true;
                ::io_uring_prep_read(sqe, fd_, buf, static_cast<unsigned>(len), 0);
                ::io_uring_sqe_set_data(sqe, op);
                submit_ring(op);
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
                linux_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            auto *sqe = get_sqe(op, executor, fd_);
            if (sqe) {
                wants_write_ = true;
                ::io_uring_prep_write(sqe, fd_, buf, static_cast<unsigned>(len), 0);
                ::io_uring_sqe_set_data(sqe, op);
                submit_ring(op);
                return;
            }
            wants_write_ = true;
            int fd = fd_;
            std::atomic<bool> *flag = &cancel_flag_;
            get_executor().submit([fd, buf, len, op, flag, this]() mutable {
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

    using platform_descriptor_impl = linux_descriptor_impl;
    using platform_anon_pipe_impl = linux_anon_pipe_impl;

    memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl(executor_type executor) {
        return memory::make_nebula<platform_descriptor_impl>(executor);
    }

    memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl_from_native(executor_type executor, native_handle_type handle) {
        auto impl = memory::make_nebula<platform_descriptor_impl>(executor);
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
        auto impl = memory::make_nebula<platform_descriptor_impl>(executor);
        if (duped >= 0) {
            static_cast<void>(impl->attach(static_cast<native_handle_type>(duped)));
        }
        return impl;
    }

    utility::pair<memory::nebula_ptr<descriptor_impl_base>, memory::nebula_ptr<descriptor_impl_base>> create_pipe_impl(
        executor_type executor, std::error_code &ec) {
        int fds[2] = {-1, -1};
        if (::pipe2(fds, O_CLOEXEC) != 0) {
            ec = posix_error();
            return {};
        }
        auto read_impl = memory::make_nebula<platform_anon_pipe_impl>(executor);
        auto write_impl = memory::make_nebula<platform_anon_pipe_impl>(executor);
        static_cast<void>(read_impl->attach(static_cast<native_handle_type>(fds[0])));
        static_cast<void>(write_impl->attach(static_cast<native_handle_type>(fds[1])));
        ec.clear();
        return {utility::move(read_impl), utility::move(write_impl)};
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
        auto impl = memory::make_nebula<platform_descriptor_impl>(executor);
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
        auto impl = memory::make_nebula<platform_descriptor_impl>(executor);
        static_cast<void>(impl->attach(static_cast<native_handle_type>(fd)));
        ec.clear();
        return impl;
    }
}
