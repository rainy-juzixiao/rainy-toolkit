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

// NOLINTBEGIN
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
// NOLINTEND

#include <rainy/foundation/io/implements/io_context.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>

namespace rainy::foundation::io::net::implements {
    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    static int get_kq_from_op(io::implements::completion_op *op, io::implements::io_context_impl_base &ctx_impl, int fd) noexcept {
        ctx_impl.associate_handle(op, static_cast<std::uintptr_t>(fd), nullptr);
        if (!op->io_handle) {
            return -1;
        }
        return static_cast<int>(reinterpret_cast<std::uintptr_t>(op->io_handle));
    }

    static bool submit_kevent(int kq, int fd, short filter, io::implements::completion_op *op) noexcept {
        struct kevent ev{};
        EV_SET(&ev, static_cast<uintptr_t>(fd), filter, EV_ADD | EV_ONESHOT, 0, 0, op);
        return ::kevent(kq, &ev, 1, nullptr, 0, nullptr) == 0;
    }

    template <typename IoFunc>
    class kqueue_io_op final : public io::implements::completion_op {
    public:
        kqueue_io_op(IoFunc &&io_func, io::implements::completion_op *user_op) noexcept(std::is_nothrow_move_constructible_v<IoFunc>) :
            io::implements::completion_op(&do_complete), io_func_(std::move(io_func)), user_op_(user_op) {
        }

        kqueue_io_op(const kqueue_io_op &) = delete;
        kqueue_io_op &operator=(const kqueue_io_op &) = delete;

    private:
        static void do_complete(io::implements::completion_op *self, const io::implements::op_result & /*kevent_result*/, bool is_cancelled) noexcept {
            auto *me = static_cast<kqueue_io_op *>(self);
            IoFunc io_func = std::move(me->io_func_);
            io::implements::completion_op *user = me->user_op_;
            delete me;

            if (is_cancelled) {
                io::implements::op_result r{user, 0, ECANCELED};
                user->complete(r, true);
                return;
            }
            // 执行真正的 syscall，结果写入 r
            io::implements::op_result r{user, 0, 0};
            io_func(r);
            user->complete(r, false);
        }

        IoFunc io_func_;
        io::implements::completion_op *user_op_;
    };

    template <typename IoFunc>
    static void post_kqueue_async(int kq, int fd, short filter, io::implements::io_context_impl_base &ctx_impl, io::implements::completion_op *user_op,
                                  IoFunc &&io_func) noexcept {
        auto *io_op = new (std::nothrow) kqueue_io_op<std::decay_t<IoFunc>>(std::forward<IoFunc>(io_func), user_op);
        if (!io_op) {
            io::implements::op_result r{user_op, 0, ENOMEM};
            user_op->complete(r, false);
            return;
        }
        if (!submit_kevent(kq, fd, filter, io_op)) {
            delete io_op;
            ctx_impl.post_immediate_completion(user_op, false);
        }
    }

    class macos_socket_impl final : public socket_impl_base {
    public:
        macos_socket_impl() = default;

        ~macos_socket_impl() override {
            if (is_open()) {
                close();
            }
        }

        std::error_code open(int af, int type, int proto) noexcept override {
            if (is_open()) {
                return posix_error(EISCONN);
            }
            // macOS 无 SOCK_CLOEXEC，手动 fcntl
            fd_ = ::socket(af, type, proto);
            if (fd_ < 0) {
                return posix_error();
            }
            if (const int flags = ::fcntl(fd_, F_GETFD, 0); flags < 0 || ::fcntl(fd_, F_SETFD, flags | FD_CLOEXEC) < 0) {
                ::close(fd_);
                fd_ = -1;
                return posix_error();
            }
            af_ = af;
            type_ = type;
            proto_ = proto;
            return {};
        }

        std::error_code assign(int af, int type, int proto, native_socket_t native_sock) noexcept override {
            if (is_open()) {
                close();
            }
            fd_ = native_sock;
            af_ = af;
            type_ = type;
            proto_ = proto;
            return {};
        }

        native_socket_t release() noexcept override {
            native_socket_t fd = fd_;
            fd_ = -1;
            return fd;
        }

        std::error_code close() noexcept override {
            if (fd_ < 0) {
                return {};
            }
            const int ret = ::close(fd_);
            fd_ = -1;
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code cancel() noexcept override {
            // kqueue 使用 EV_ONESHOT，事件触发后自动注销，无需主动取消
            return {};
        }

        bool is_open() const noexcept override {
            return fd_ >= 0;
        }

        native_socket_t native_handle() const noexcept override {
            return fd_;
        }

        std::error_code set_option(const socket_option &opt) noexcept override {
            const int ret = ::setsockopt(fd_, opt.level, opt.name, opt.value, static_cast<::socklen_t>(opt.value_size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code get_option(socket_option &opt) const noexcept override {
            auto len = static_cast<::socklen_t>(opt.value_size);
            const int ret = ::getsockopt(fd_, opt.level, opt.name, const_cast<void *>(opt.value), &len);
            opt.value_size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code set_non_blocking(bool mode) noexcept override {
            non_blocking_ = mode;
            return apply_nonblock(mode);
        }

        bool non_blocking() const noexcept override {
            return non_blocking_;
        }

        std::error_code set_native_non_blocking(bool mode) noexcept override {
            native_non_blocking_ = mode;
            return apply_nonblock(mode);
        }

        bool native_non_blocking() const noexcept override {
            return native_non_blocking_;
        }

        std::error_code bind(const raw_endpoint &ep) noexcept override {
            const int ret = ::bind(fd_, reinterpret_cast<const ::sockaddr *>(ep.data), static_cast<::socklen_t>(ep.size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code connect(const raw_endpoint &ep) noexcept override {
            if (fd_ < 0) {
                int af = ep.family;
                int type = SOCK_STREAM;
                int proto = 0;

                auto ec = open(af, type, proto);
                if (ec) {
                    return ec;
                }
            }
            const int ret = ::connect(fd_, reinterpret_cast<const ::sockaddr *>(ep.data), static_cast<::socklen_t>(ep.size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code listen(int backlog) noexcept override {
            const int ret = ::listen(fd_, backlog);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code shutdown(shutdown_how how) noexcept override {
            const int sd = (how == shutdown_how::receive) ? SHUT_RD : (how == shutdown_how::send) ? SHUT_WR : SHUT_RDWR;
            const int ret = ::shutdown(fd_, sd);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code local_endpoint(raw_endpoint &ep) const noexcept override {
            auto len = static_cast<::socklen_t>(sizeof(ep.data));
            const int ret = ::getsockname(fd_, reinterpret_cast<::sockaddr *>(ep.data), &len);
            if (ret == 0) {
                ep.size = static_cast<std::size_t>(len);
            }
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code remote_endpoint(raw_endpoint &ep) const noexcept override {
            auto len = static_cast<::socklen_t>(sizeof(ep.data));
            const int ret = ::getpeername(fd_, reinterpret_cast<::sockaddr *>(ep.data), &len);
            if (ret == 0) {
                ep.size = static_cast<std::size_t>(len);
            }
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::ptrdiff_t send(const void *buf, std::size_t len, message_flags_t flags, std::error_code &ec) noexcept override {
            const ::ssize_t n = ::send(fd_, buf, len, flags);
            if (n < 0) {
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len, message_flags_t flags, std::error_code &ec) noexcept override {
            const ::ssize_t n = ::recv(fd_, buf, len, flags);
            if (n < 0) {
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t send_to(const void *buf, std::size_t len, message_flags_t flags, const raw_endpoint &dest,
                               std::error_code &ec) noexcept override {
            const ::ssize_t n =
                ::sendto(fd_, buf, len, flags, reinterpret_cast<const ::sockaddr *>(dest.data), static_cast<::socklen_t>(dest.size));
            if (n < 0) {
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t receive_from(void *buf, std::size_t len, message_flags_t flags, raw_endpoint &sender,
                                    std::error_code &ec) noexcept override {
            auto slen = static_cast<::socklen_t>(sizeof(sender.data));
            const ::ssize_t n = ::recvfrom(fd_, buf, len, flags, reinterpret_cast<::sockaddr *>(sender.data), &slen);
            if (n < 0) {
                ec = posix_error();
                return -1;
            }
            sender.size = static_cast<std::size_t>(slen);
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        native_socket_t accept(raw_endpoint *peer_ep, std::error_code &ec) noexcept override {
            int client;
            if (peer_ep) {
                auto len = static_cast<::socklen_t>(sizeof(peer_ep->data));
                client = ::accept(fd_, reinterpret_cast<::sockaddr *>(peer_ep->data), &len);
                if (client >= 0) {
                    peer_ep->size = static_cast<std::size_t>(len);
                }
            } else {
                client = ::accept(fd_, nullptr, nullptr);
            }
            if (client < 0) {
                ec = posix_error();
                return invalid_socket_value;
            }
            if (const int flags = ::fcntl(client, F_GETFD, 0); flags >= 0) {
                ::fcntl(client, F_SETFD, flags | FD_CLOEXEC);
            }
            ec.clear();
            return static_cast<native_socket_t>(client);
        }

        std::size_t available(std::error_code &ec) const noexcept override {
            int avail = 0;
            const int ret = ::ioctl(fd_, FIONREAD, &avail);
            if (ret < 0) {
                ec = posix_error();
                return 0;
            }
            ec.clear();
            return static_cast<std::size_t>(avail);
        }

        bool at_mark(std::error_code &ec) const noexcept override {
            const int ret = ::sockatmark(fd_);
            if (ret < 0) {
                ec = posix_error();
                return false;
            }
            ec.clear();
            return ret != 0;
        }

        std::error_code io_control(unsigned long cmd, void *arg) noexcept override {
            const int ret = ::ioctl(fd_, cmd, arg);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        void async_connect(const raw_endpoint &ep, io::implements::io_context_impl_base &ctx_impl,
                           io::implements::completion_op *op) noexcept override {
            const int ret = ::connect(fd_, reinterpret_cast<const ::sockaddr *>(ep.data), static_cast<::socklen_t>(ep.size));
            if (ret == 0) {
                // 连接立即成功（loopback 常见）
                ctx_impl.post_immediate_completion(op, false);
                return;
            }
            if (errno != EINPROGRESS) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_WRITE, ctx_impl, op, [fd](io::implements::op_result &r) noexcept {
                int err = 0;
                ::socklen_t len = sizeof(err);
                if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
                    err = errno;
                }
                r.bytes_transferred = 0;
                r.error_code = err;
            });
        }

        void async_send(const void *buf, std::size_t len, message_flags_t flags, io::implements::io_context_impl_base &ctx_impl,
                        io::implements::completion_op *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_WRITE, ctx_impl, op, [fd, buf, len, flags](io::implements::op_result &r) noexcept {
                const ::ssize_t n = ::send(fd, buf, len, flags);
                if (n < 0) {
                    r.bytes_transferred = 0;
                    r.error_code = errno;
                } else {
                    r.bytes_transferred = static_cast<std::size_t>(n);
                    r.error_code = 0;
                }
            });
        }

        void async_receive(void *buf, std::size_t len, message_flags_t flags, io::implements::io_context_impl_base &ctx_impl,
                           io::implements::completion_op *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_READ, ctx_impl, op, [fd, buf, len, flags](io::implements::op_result &r) noexcept {
                const ::ssize_t n = ::recv(fd, buf, len, flags);
                if (n < 0) {
                    r.bytes_transferred = 0;
                    r.error_code = errno;
                } else {
                    r.bytes_transferred = static_cast<std::size_t>(n);
                    r.error_code = 0;
                }
            });
        }

        void async_send_to(const void *buf, std::size_t len, message_flags_t flags, const raw_endpoint &dest,
                           io::implements::io_context_impl_base &ctx_impl, io::implements::completion_op *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }
            const int fd = fd_;
            raw_endpoint dest_copy = dest;
            post_kqueue_async(kq, fd_, EVFILT_WRITE, ctx_impl, op, [fd, buf, len, flags, dest_copy](io::implements::op_result &r) noexcept {
                const ::ssize_t n = ::sendto(fd, buf, len, flags, reinterpret_cast<const ::sockaddr *>(dest_copy.data),
                                             static_cast<::socklen_t>(dest_copy.size));
                if (n < 0) {
                    r.bytes_transferred = 0;
                    r.error_code = errno;
                } else {
                    r.bytes_transferred = static_cast<std::size_t>(n);
                    r.error_code = 0;
                }
            });
        }

        void async_receive_from(void *buf, std::size_t len, message_flags_t flags, raw_endpoint &sender,
                                io::implements::io_context_impl_base &ctx_impl, io::implements::completion_op *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }
            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_READ, ctx_impl, op, [fd, buf, len, flags, &sender](io::implements::op_result &r) noexcept {
                auto slen = static_cast<::socklen_t>(sizeof(sender.data));
                const ::ssize_t n = ::recvfrom(fd, buf, len, flags, reinterpret_cast<::sockaddr *>(sender.data), &slen);
                if (n < 0) {
                    r.bytes_transferred = 0;
                    r.error_code = errno;
                } else {
                    sender.size = static_cast<std::size_t>(slen);
                    r.bytes_transferred = static_cast<std::size_t>(n);
                    r.error_code = 0;
                }
            });
        }

        void async_accept(raw_endpoint *peer_ep, io::implements::io_context_impl_base &ctx_impl, io::implements::completion_op *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_READ, ctx_impl, op, [fd, peer_ep](io::implements::op_result &r) noexcept {
                int client;
                if (peer_ep) {
                    auto len = static_cast<::socklen_t>(sizeof(peer_ep->data));
                    client = ::accept(fd, reinterpret_cast<::sockaddr *>(peer_ep->data), &len);
                    if (client >= 0) {
                        peer_ep->size = static_cast<std::size_t>(len);
                    }
                } else {
                    client = ::accept(fd, nullptr, nullptr);
                }
                if (client < 0) {
                    r.bytes_transferred = 0;
                    r.error_code = errno;
                    return;
                }
                if (const int fl = ::fcntl(client, F_GETFD, 0); fl >= 0) {
                    ::fcntl(client, F_SETFD, fl | FD_CLOEXEC);
                }
                r.bytes_transferred = static_cast<std::size_t>(client);
                r.error_code = 0;
            });
        }

        void async_wait(wait_type w, io::implements::io_context_impl_base &ctx_impl, io::implements::completion_op *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const short filter = (w == wait_type::write) ? EVFILT_WRITE : EVFILT_READ;
            if (!submit_kevent(kq, fd_, filter, op)) {
                ctx_impl.post_immediate_completion(op, false);
            }
        }

    private:
        std::error_code apply_nonblock(bool mode) noexcept {
            int flags = ::fcntl(fd_, F_GETFL, 0);
            if (flags < 0) {
                return posix_error();
            }
            flags = mode ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
            const int ret = ::fcntl(fd_, F_SETFL, flags);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        int fd_{-1};
        int af_{0};
        int type_{0};
        int proto_{0};
        bool non_blocking_{false};
        bool native_non_blocking_{false};
    };
}

namespace rainy::foundation::io::net::implements {
    memory::nebula_ptr<socket_impl_base> create_socket_impl() {
        return memory::make_nebula<macos_socket_impl>();
    }
}
