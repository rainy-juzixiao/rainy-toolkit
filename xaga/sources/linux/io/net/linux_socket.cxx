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
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <liburing.h>
// NOLINTEND

#include <rainy/foundation/io/net/implements/io_context.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>

namespace rainy::foundation::io::net::implements {

    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    // 从 op->io_handle 取出 ring，填充 SQE 并提交
    // 若取不到则 fallback 到 post_immediate_completion
    static io_uring_sqe *get_sqe_from_op(completion_op        *op,
                                          io_context_impl_base &ctx_impl,
                                          int                   fd) {
        ctx_impl.associate_handle(op,
                                  static_cast<std::uintptr_t>(fd),
                                  nullptr);
        if (!op->io_handle) return nullptr;
        return ::io_uring_get_sqe(static_cast<io_uring *>(op->io_handle));
    }

    static void submit_ring(completion_op *op) noexcept {
        if (op->io_handle) {
            ::io_uring_submit(static_cast<io_uring *>(op->io_handle));
        }
    }

    class linux_socket_impl final : public socket_impl_base {
    public:
        linux_socket_impl() = default;

        ~linux_socket_impl() override {
            if (is_open()) close();
        }

        std::error_code open(int af, int type, int proto) noexcept override {
            if (is_open()) return posix_error(EISCONN);
            fd_ = ::socket(af, type | SOCK_CLOEXEC, proto);
            if (fd_ < 0) return posix_error();
            af_ = af; type_ = type; proto_ = proto;
            return {};
        }

        std::error_code assign(int af, int type, int proto,
                               native_socket_t native_sock) noexcept override {
            if (is_open()) close();
            fd_ = native_sock;
            af_ = af; type_ = type; proto_ = proto;
            return {};
        }

        native_socket_t release() noexcept override {
            native_socket_t fd = fd_;
            fd_ = -1;
            return fd;
        }

        std::error_code close() noexcept override {
            if (fd_ < 0) return {};
            int ret = ::close(fd_);
            fd_ = -1;
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code cancel() noexcept override {
            return {};
        }

        bool is_open() const noexcept override { return fd_ >= 0; }

        native_socket_t native_handle() const noexcept override { return fd_; }

        std::error_code set_option(const socket_option &opt) noexcept override {
            int ret = ::setsockopt(fd_, opt.level, opt.name,
                                   opt.value,
                                   static_cast<::socklen_t>(opt.value_size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code get_option(socket_option &opt) const noexcept override {
            ::socklen_t len = static_cast<::socklen_t>(opt.value_size);
            int ret = ::getsockopt(fd_, opt.level, opt.name,
                                   const_cast<void *>(opt.value), &len);
            opt.value_size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code set_non_blocking(bool mode) noexcept override {
            non_blocking_ = mode;
            return apply_nonblock(mode);
        }

        bool non_blocking() const noexcept override { return non_blocking_; }

        std::error_code set_native_non_blocking(bool mode) noexcept override {
            native_non_blocking_ = mode;
            return apply_nonblock(mode);
        }

        bool native_non_blocking() const noexcept override {
            return native_non_blocking_;
        }

        std::error_code bind(const raw_endpoint &ep) noexcept override {
            int ret = ::bind(fd_,
                             reinterpret_cast<const ::sockaddr *>(ep.data),
                             static_cast<::socklen_t>(ep.size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code connect(const raw_endpoint &ep) noexcept override {
            int ret = ::connect(fd_,
                                reinterpret_cast<const ::sockaddr *>(ep.data),
                                static_cast<::socklen_t>(ep.size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code listen(int backlog) noexcept override {
            int ret = ::listen(fd_, backlog);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code shutdown(shutdown_how how) noexcept override {
            int sd = (how == shutdown_how::receive) ? SHUT_RD
                   : (how == shutdown_how::send)    ? SHUT_WR
                   :                                  SHUT_RDWR;
            int ret = ::shutdown(fd_, sd);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code local_endpoint(raw_endpoint &ep) const noexcept override {
            ::socklen_t len = static_cast<::socklen_t>(sizeof(ep.data));
            int ret = ::getsockname(fd_,
                                    reinterpret_cast<::sockaddr *>(ep.data),
                                    &len);
            if (ret == 0) ep.size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code remote_endpoint(raw_endpoint &ep) const noexcept override {
            ::socklen_t len = static_cast<::socklen_t>(sizeof(ep.data));
            int ret = ::getpeername(fd_,
                                    reinterpret_cast<::sockaddr *>(ep.data),
                                    &len);
            if (ret == 0) ep.size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::ptrdiff_t send(const void *buf, std::size_t len,
                            message_flags_t flags,
                            std::error_code &ec) noexcept override {
            ::ssize_t n = ::send(fd_, buf, len, flags);
            if (n < 0) { ec = posix_error(); return -1; }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len,
                               message_flags_t flags,
                               std::error_code &ec) noexcept override {
            ::ssize_t n = ::recv(fd_, buf, len, flags);
            if (n < 0) { ec = posix_error(); return -1; }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t send_to(const void *buf, std::size_t len,
                               message_flags_t flags,
                               const raw_endpoint &dest,
                               std::error_code &ec) noexcept override {
            ::ssize_t n = ::sendto(fd_, buf, len, flags,
                reinterpret_cast<const ::sockaddr *>(dest.data),
                static_cast<::socklen_t>(dest.size));
            if (n < 0) { ec = posix_error(); return -1; }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t receive_from(void *buf, std::size_t len,
                                    message_flags_t flags,
                                    raw_endpoint &sender,
                                    std::error_code &ec) noexcept override {
            ::socklen_t slen = static_cast<::socklen_t>(sizeof(sender.data));
            ::ssize_t n = ::recvfrom(fd_, buf, len, flags,
                reinterpret_cast<::sockaddr *>(sender.data), &slen);
            if (n < 0) { ec = posix_error(); return -1; }
            sender.size = static_cast<std::size_t>(slen);
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        native_socket_t accept(raw_endpoint *peer_ep,
                               std::error_code &ec) noexcept override {
            int client;
            if (peer_ep) {
                auto len = static_cast<::socklen_t>(sizeof(peer_ep->data));
                client = ::accept4(fd_,
                    reinterpret_cast<::sockaddr *>(peer_ep->data),
                    &len, SOCK_CLOEXEC);
                if (client >= 0)
                    peer_ep->size = static_cast<std::size_t>(len);
            } else {
                client = ::accept4(fd_, nullptr, nullptr, SOCK_CLOEXEC);
            }
            if (client < 0) { ec = posix_error(); return invalid_socket_value; }
            ec.clear();
            return static_cast<native_socket_t>(client);
        }

        std::size_t available(std::error_code &ec) const noexcept override {
            int avail = 0;
            int ret = ::ioctl(fd_, FIONREAD, &avail);
            if (ret < 0) { ec = posix_error(); return 0; }
            ec.clear();
            return static_cast<std::size_t>(avail);
        }

        bool at_mark(std::error_code &ec) const noexcept override {
            int ret = ::sockatmark(fd_);
            if (ret < 0) { ec = posix_error(); return false; }
            ec.clear();
            return ret != 0;
        }

        std::error_code io_control(unsigned long cmd,
                                   void *arg) noexcept override {
            int ret = ::ioctl(fd_, cmd, arg);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        // ------------------------------------------------------------------
        // 异步操作：通过 op->io_handle 桥接，不感知 io_uring_impl
        // ------------------------------------------------------------------
        void async_connect(const raw_endpoint   &ep,
                           io_context_impl_base &ctx_impl,
                           completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            ::io_uring_prep_connect(sqe, fd_,
                reinterpret_cast<const ::sockaddr *>(ep.data),
                static_cast<::socklen_t>(ep.size));
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_send(const void           *buf,
                        std::size_t           len,
                        message_flags_t       flags,
                        io_context_impl_base &ctx_impl,
                        completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            ::io_uring_prep_send(sqe, fd_, buf, len, flags);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_receive(void                *buf,
                           std::size_t          len,
                           message_flags_t      flags,
                           io_context_impl_base &ctx_impl,
                           completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            ::io_uring_prep_recv(sqe, fd_, buf, len, flags);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_send_to(const void           *buf,
                           std::size_t           len,
                           message_flags_t       flags,
                           const raw_endpoint   &dest,
                           io_context_impl_base &ctx_impl,
                           completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            static thread_local ::iovec  iov{};
            static thread_local ::msghdr msg{};
            iov.iov_base = const_cast<void *>(buf);
            iov.iov_len  = len;
            msg = {};
            msg.msg_name    = const_cast<void *>(
                static_cast<const void *>(dest.data));
            msg.msg_namelen = static_cast<::socklen_t>(dest.size);
            msg.msg_iov     = &iov;
            msg.msg_iovlen  = 1;
            ::io_uring_prep_sendmsg(sqe, fd_, &msg, flags);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_receive_from(void                *buf,
                                std::size_t          len,
                                message_flags_t      flags,
                                raw_endpoint        &sender,
                                io_context_impl_base &ctx_impl,
                                completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            static thread_local ::iovec  iov{};
            static thread_local ::msghdr msg{};
            iov.iov_base = buf;
            iov.iov_len  = len;
            msg = {};
            msg.msg_name    = sender.data;
            msg.msg_namelen = static_cast<::socklen_t>(sizeof(sender.data));
            msg.msg_iov     = &iov;
            msg.msg_iovlen  = 1;
            ::io_uring_prep_recvmsg(sqe, fd_, &msg, flags);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_accept(raw_endpoint        *peer_ep,
                          io_context_impl_base &ctx_impl,
                          completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            static thread_local ::socklen_t addrlen =
                peer_ep ? static_cast<::socklen_t>(sizeof(peer_ep->data)) : 0;
            ::io_uring_prep_accept(sqe, fd_,
                peer_ep ? reinterpret_cast<::sockaddr *>(peer_ep->data) : nullptr,
                &addrlen, SOCK_CLOEXEC);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_wait(wait_type             w,
                        io_context_impl_base &ctx_impl,
                        completion_op        *op) noexcept override {
            auto *sqe = get_sqe_from_op(op, ctx_impl, fd_);
            if (!sqe) { ctx_impl.post_immediate_completion(op, false); return; }
            unsigned poll_mask = 0;
            switch (w) {
                case wait_type::read:  poll_mask = POLLIN;  break;
                case wait_type::write: poll_mask = POLLOUT; break;
                case wait_type::error: poll_mask = POLLERR; break;
            }
            ::io_uring_prep_poll_add(sqe, fd_, poll_mask);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

    private:
        std::error_code apply_nonblock(bool mode) noexcept {
            int flags = ::fcntl(fd_, F_GETFL, 0);
            if (flags < 0) return posix_error();
            flags = mode ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
            int ret = ::fcntl(fd_, F_SETFL, flags);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        int fd_{-1};
        int af_{0};
        int type_{0};
        int proto_{0};
    };

    memory::nebula_ptr<socket_impl_base> create_socket_impl() {
        return memory::make_nebula<linux_socket_impl>();
    }

}