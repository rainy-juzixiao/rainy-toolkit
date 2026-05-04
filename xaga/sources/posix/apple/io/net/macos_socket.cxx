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
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
// NOLINTEND

#include <rainy/foundation/io/net/implements/io_context.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>

namespace rainy::foundation::io::net::implements {
    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    // 从 op->io_handle 取出 kq fd
    static int get_kq_from_op(completion_op        *op,
                               io_context_impl_base &ctx_impl,
                               int                   fd) noexcept {
        ctx_impl.associate_handle(op,
                                  static_cast<std::uintptr_t>(fd),
                                  nullptr);
        if (!op->io_handle) {
            return -1;
        }
        return static_cast<int>(reinterpret_cast<std::uintptr_t>(op->io_handle));
    }

    // 向 kq 提交单条 kevent，EV_ONESHOT 保证触发一次后自动注销
    static bool submit_kevent(int kq, int fd, short filter,
                              completion_op *op) noexcept {
        struct kevent ev{};
        EV_SET(&ev,
               static_cast<uintptr_t>(fd),
               filter,
               EV_ADD | EV_ONESHOT,
               0, 0,
               op);  // udata = completion_op*，harvest 中取回
        return ::kevent(kq, &ev, 1, nullptr, 0, nullptr) == 0;
    }

    // ------------------------------------------------------------------ //
    //  kqueue 版的两步式异步操作包装                                        //
    //                                                                      //
    //  每个异步操作需要：                                                   //
    //  ① 一个携带上下文的 kqueue_async_op（持有真正的 I/O 参数）           //
    //  ② 注册 kevent 就绪通知                                              //
    //  ③ harvest 回调时执行 syscall，结果写入外层 op_result                //
    //                                                                      //
    //  设计：将"就绪后执行的 syscall"封装为 lambda，存入                   //
    //  kqueue_io_op<Fn>，它本身就是一个 completion_op，可以直接作为        //
    //  kevent 的 udata。harvest 收到事件后调用 complete()，complete()       //
    //  内执行 lambda，lambda 再调用用户的原始 op->complete()。             //
    // ------------------------------------------------------------------ //

    template <typename IoFunc>
    class kqueue_io_op final : public completion_op {
    public:
        // io_func  : (op_result &result) -> void，负责执行 syscall 并填充 result
        // user_op  : 用户传入的原始 completion_op，最终把结果交给它
        kqueue_io_op(IoFunc &&io_func, completion_op *user_op) noexcept(
            std::is_nothrow_move_constructible_v<IoFunc>)
            : completion_op(&do_complete),
              io_func_(std::move(io_func)),
              user_op_(user_op) {}

        kqueue_io_op(const kqueue_io_op &) = delete;
        kqueue_io_op &operator=(const kqueue_io_op &) = delete;

    private:
        static void do_complete(completion_op        *self,
                                const op_result      & /*kevent_result*/,
                                bool                  is_cancelled) noexcept {
            auto *me = static_cast<kqueue_io_op *>(self);
            IoFunc     io_func  = std::move(me->io_func_);
            completion_op *user = me->user_op_;
            delete me;

            if (is_cancelled) {
                op_result r{user, 0, ECANCELED};
                user->complete(r, true);
                return;
            }
            // 执行真正的 syscall，结果写入 r
            op_result r{user, 0, 0};
            io_func(r);
            user->complete(r, false);
        }

        IoFunc         io_func_;
        completion_op *user_op_;
    };

    template <typename IoFunc>
    static void post_kqueue_async(int                   kq,
                                  int                   fd,
                                  short                 filter,
                                  io_context_impl_base &ctx_impl,
                                  completion_op        *user_op,
                                  IoFunc              &&io_func) noexcept {
        auto *io_op = new (std::nothrow)
            kqueue_io_op<std::decay_t<IoFunc>>(std::forward<IoFunc>(io_func),
                                               user_op);
        if (!io_op) {
            // OOM：降级为立即完成，error_code = ENOMEM
            op_result r{user_op, 0, ENOMEM};
            user_op->complete(r, false);
            return;
        }
        if (!submit_kevent(kq, fd, filter, io_op)) {
            // kevent 提交失败：降级为立即投递
            delete io_op;
            ctx_impl.post_immediate_completion(user_op, false);
        }
    }

    // ------------------------------------------------------------------ //
    //                      macos_socket_impl                              //
    // ------------------------------------------------------------------ //

    class macos_socket_impl final : public socket_impl_base {
    public:
        macos_socket_impl() = default;

        ~macos_socket_impl() override {
            if (is_open()) {
                close();
            }
        }

        // -------- 基本生命周期 -------- //

        std::error_code open(int af, int type, int proto) noexcept override {
            if (is_open()) {
                return posix_error(EISCONN);
            }
            // macOS 无 SOCK_CLOEXEC，手动 fcntl
            fd_ = ::socket(af, type, proto);
            if (fd_ < 0) {
                return posix_error();
            }
            if (const int flags = ::fcntl(fd_, F_GETFD, 0);
                flags < 0 || ::fcntl(fd_, F_SETFD, flags | FD_CLOEXEC) < 0) {
                ::close(fd_);
                fd_ = -1;
                return posix_error();
                }
            af_ = af; type_ = type; proto_ = proto;
            return {};
        }

        std::error_code assign(int af, int type, int proto,
                               native_socket_t native_sock) noexcept override {
            if (is_open()) {
                close();
            }
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

        bool is_open() const noexcept override { return fd_ >= 0; }

        native_socket_t native_handle() const noexcept override { return fd_; }

        // -------- 选项 -------- //

        std::error_code set_option(const socket_option &opt) noexcept override {
            const int ret = ::setsockopt(fd_, opt.level, opt.name,
                                         opt.value,
                                         static_cast<::socklen_t>(opt.value_size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code get_option(socket_option &opt) const noexcept override {
            auto len = static_cast<::socklen_t>(opt.value_size);
            const int ret = ::getsockopt(fd_, opt.level, opt.name,
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

        // -------- 同步操作 -------- //

        std::error_code bind(const raw_endpoint &ep) noexcept override {
            const int ret = ::bind(fd_,
                                    reinterpret_cast<const ::sockaddr *>(ep.data),
                                    static_cast<::socklen_t>(ep.size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code connect(const raw_endpoint &ep) noexcept override {
            const int ret = ::connect(fd_,
                                       reinterpret_cast<const ::sockaddr *>(ep.data),
                                       static_cast<::socklen_t>(ep.size));
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code listen(int backlog) noexcept override {
            const int ret = ::listen(fd_, backlog);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code shutdown(shutdown_how how) noexcept override {
            const int sd = (how == shutdown_how::receive) ? SHUT_RD
                         : (how == shutdown_how::send)    ? SHUT_WR
                         :                                  SHUT_RDWR;
            const int ret = ::shutdown(fd_, sd);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code local_endpoint(raw_endpoint &ep) const noexcept override {
            auto len = static_cast<::socklen_t>(sizeof(ep.data));
            const int ret = ::getsockname(fd_,
                                           reinterpret_cast<::sockaddr *>(ep.data),
                                           &len);
            if (ret == 0) {
                ep.size = static_cast<std::size_t>(len);
            }
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::error_code remote_endpoint(raw_endpoint &ep) const noexcept override {
            auto len = static_cast<::socklen_t>(sizeof(ep.data));
            const int ret = ::getpeername(fd_,
                                           reinterpret_cast<::sockaddr *>(ep.data),
                                           &len);
            if (ret == 0) {
                ep.size = static_cast<std::size_t>(len);
            }
            return ret == 0 ? std::error_code{} : posix_error();
        }

        std::ptrdiff_t send(const void *buf, std::size_t len,
                            message_flags_t flags,
                            std::error_code &ec) noexcept override {
            const ::ssize_t n = ::send(fd_, buf, len, flags);
            if (n < 0) { ec = posix_error(); return -1; }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len,
                               message_flags_t flags,
                               std::error_code &ec) noexcept override {
            const ::ssize_t n = ::recv(fd_, buf, len, flags);
            if (n < 0) { ec = posix_error(); return -1; }
            ec.clear();
            return static_cast<std::ptrdiff_t>(n);
        }

        std::ptrdiff_t send_to(const void *buf, std::size_t len,
                               message_flags_t flags,
                               const raw_endpoint &dest,
                               std::error_code &ec) noexcept override {
            const ::ssize_t n = ::sendto(fd_, buf, len, flags,
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
            auto slen = static_cast<::socklen_t>(sizeof(sender.data));
            const ::ssize_t n = ::recvfrom(fd_, buf, len, flags,
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
                client = ::accept(fd_,
                    reinterpret_cast<::sockaddr *>(peer_ep->data), &len);
                if (client >= 0) {
                    peer_ep->size = static_cast<std::size_t>(len);
                }
            } else {
                client = ::accept(fd_, nullptr, nullptr);
            }
            if (client < 0) { ec = posix_error(); return invalid_socket_value; }
            // macOS 无 accept4，手动设置 FD_CLOEXEC
            if (const int flags = ::fcntl(client, F_GETFD, 0); flags >= 0) {
                ::fcntl(client, F_SETFD, flags | FD_CLOEXEC);
            }
            ec.clear();
            return static_cast<native_socket_t>(client);
        }

        std::size_t available(std::error_code &ec) const noexcept override {
            int avail = 0;
            const int ret = ::ioctl(fd_, FIONREAD, &avail);
            if (ret < 0) { ec = posix_error(); return 0; }
            ec.clear();
            return static_cast<std::size_t>(avail);
        }

        bool at_mark(std::error_code &ec) const noexcept override {
            const int ret = ::sockatmark(fd_);
            if (ret < 0) { ec = posix_error(); return false; }
            ec.clear();
            return ret != 0;
        }

        std::error_code io_control(unsigned long cmd,
                                   void *arg) noexcept override {
            const int ret = ::ioctl(fd_, cmd, arg);
            return ret == 0 ? std::error_code{} : posix_error();
        }

        // ---------------------------------------------------------------- //
        //  异步操作                                                          //
        //                                                                   //
        //  kqueue 侧流程：                                                  //
        //    async_xxx() → submit_kevent(EVFILT_READ/WRITE)                 //
        //    → harvest() 收到就绪通知 → kqueue_io_op::do_complete()         //
        //    → 执行真正 syscall → 调用用户 user_op->complete()              //
        // ---------------------------------------------------------------- //

        void async_connect(const raw_endpoint   &ep,
                           io_context_impl_base &ctx_impl,
                           completion_op        *op) noexcept override {
            // connect() 先发起非阻塞连接，EINPROGRESS 时等待 EVFILT_WRITE 就绪
            const int ret = ::connect(fd_,
                reinterpret_cast<const ::sockaddr *>(ep.data),
                static_cast<::socklen_t>(ep.size));
            if (ret == 0) {
                // 连接立即成功（loopback 常见）
                ctx_impl.post_immediate_completion(op, false);
                return;
            }
            if (errno != EINPROGRESS) {
                // 真实错误：立即完成并携带错误码
                // 借助 post_immediate_completion 保持统一回调路径；
                // 错误信息由上层 op->complete() 的回调通过 getsockopt(SO_ERROR) 取得
                ctx_impl.post_immediate_completion(op, false);
                return;
            }

            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_WRITE, ctx_impl, op,
                [fd](op_result &r) noexcept {
                    // 检查连接结果
                    int err = 0;
                    ::socklen_t len = sizeof(err);
                    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
                        err = errno;
                    }
                    r.bytes_transferred = 0;
                    r.error_code        = err;
                });
        }

        void async_send(const void           *buf,
                        std::size_t           len,
                        message_flags_t       flags,
                        io_context_impl_base &ctx_impl,
                        completion_op        *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_WRITE, ctx_impl, op,
                [fd, buf, len, flags](op_result &r) noexcept {
                    const ::ssize_t n = ::send(fd, buf, len, flags);
                    if (n < 0) {
                        r.bytes_transferred = 0;
                        r.error_code        = errno;
                    } else {
                        r.bytes_transferred = static_cast<std::size_t>(n);
                        r.error_code        = 0;
                    }
                });
        }

        void async_receive(void                *buf,
                           std::size_t          len,
                           message_flags_t      flags,
                           io_context_impl_base &ctx_impl,
                           completion_op        *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_READ, ctx_impl, op,
                [fd, buf, len, flags](op_result &r) noexcept {
                    const ::ssize_t n = ::recv(fd, buf, len, flags);
                    if (n < 0) {
                        r.bytes_transferred = 0;
                        r.error_code        = errno;
                    } else {
                        r.bytes_transferred = static_cast<std::size_t>(n);
                        r.error_code        = 0;
                    }
                });
        }

        void async_send_to(const void           *buf,
                           std::size_t           len,
                           message_flags_t       flags,
                           const raw_endpoint   &dest,
                           io_context_impl_base &ctx_impl,
                           completion_op        *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            // dest 是栈上引用，lambda 必须按值捕获其内容
            const int fd = fd_;
            // raw_endpoint 内部是 POD 数组，直接按值复制安全
            raw_endpoint dest_copy = dest;
            post_kqueue_async(kq, fd_, EVFILT_WRITE, ctx_impl, op,
                [fd, buf, len, flags, dest_copy](op_result &r) noexcept {
                    const ::ssize_t n = ::sendto(fd, buf, len, flags,
                        reinterpret_cast<const ::sockaddr *>(dest_copy.data),
                        static_cast<::socklen_t>(dest_copy.size));
                    if (n < 0) {
                        r.bytes_transferred = 0;
                        r.error_code        = errno;
                    } else {
                        r.bytes_transferred = static_cast<std::size_t>(n);
                        r.error_code        = 0;
                    }
                });
        }

        void async_receive_from(void                *buf,
                                std::size_t          len,
                                message_flags_t      flags,
                                raw_endpoint        &sender,
                                io_context_impl_base &ctx_impl,
                                completion_op        *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            // sender 是调用方持有的引用，生命周期由调用方保证在回调前有效
            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_READ, ctx_impl, op,
                [fd, buf, len, flags, &sender](op_result &r) noexcept {
                    auto slen = static_cast<::socklen_t>(sizeof(sender.data));
                    const ::ssize_t n = ::recvfrom(fd, buf, len, flags,
                        reinterpret_cast<::sockaddr *>(sender.data), &slen);
                    if (n < 0) {
                        r.bytes_transferred = 0;
                        r.error_code        = errno;
                    } else {
                        sender.size         = static_cast<std::size_t>(slen);
                        r.bytes_transferred = static_cast<std::size_t>(n);
                        r.error_code        = 0;
                    }
                });
        }

        void async_accept(raw_endpoint        *peer_ep,
                          io_context_impl_base &ctx_impl,
                          completion_op        *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            const int fd = fd_;
            post_kqueue_async(kq, fd_, EVFILT_READ, ctx_impl, op,
                [fd, peer_ep](op_result &r) noexcept {
                    int client;
                    if (peer_ep) {
                        auto len = static_cast<::socklen_t>(sizeof(peer_ep->data));
                        client = ::accept(fd,
                            reinterpret_cast<::sockaddr *>(peer_ep->data), &len);
                        if (client >= 0) {
                            peer_ep->size = static_cast<std::size_t>(len);
                        }
                    } else {
                        client = ::accept(fd, nullptr, nullptr);
                    }
                    if (client < 0) {
                        r.bytes_transferred = 0;
                        r.error_code        = errno;
                        return;
                    }
                    // macOS 无 accept4，手动补 FD_CLOEXEC
                    if (const int fl = ::fcntl(client, F_GETFD, 0); fl >= 0) {
                        ::fcntl(client, F_SETFD, fl | FD_CLOEXEC);
                    }
                    // bytes_transferred 复用为接受到的 fd 值，上层按此解释
                    r.bytes_transferred = static_cast<std::size_t>(client);
                    r.error_code        = 0;
                });
        }

        void async_wait(wait_type             w,
                        io_context_impl_base &ctx_impl,
                        completion_op        *op) noexcept override {
            const int kq = get_kq_from_op(op, ctx_impl, fd_);
            if (kq < 0) { ctx_impl.post_immediate_completion(op, false); return; }

            const short filter = (w == wait_type::write) ? EVFILT_WRITE
                                                          : EVFILT_READ;
            // async_wait 只需要就绪通知本身，无需执行额外 syscall
            // 直接把 user_op 作为 udata，harvest 收到后直接 complete()
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

        int  fd_{-1};
        int  af_{0};
        int  type_{0};
        int  proto_{0};
        bool non_blocking_{false};
        bool native_non_blocking_{false};
    };

    memory::nebula_ptr<socket_impl_base> create_socket_impl() {
        return memory::make_nebula<macos_socket_impl>();
    }
}