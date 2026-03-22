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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <rainy/foundation/io/net/implements/io_context.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

namespace rainy::foundation::io::net::implements {
    struct wsa_init {
        wsa_init() {
            WSADATA wd{};
            std::ignore = ::WSAStartup(MAKEWORD(2, 2), &wd);
        }
        ~wsa_init() {
            ::WSACleanup();
        }
    };

    static wsa_init g_wsa;

    struct iocp_op : completion_op { // 必须与win_iocp中的iocp_op结构体一致
        OVERLAPPED overlapped{};
        HANDLE associated_handle{INVALID_HANDLE_VALUE};
        DWORD transferred{0};

        explicit iocp_op(fn_type f) noexcept : completion_op(f) {
        }

        static iocp_op *from_overlapped(OVERLAPPED *ov) noexcept {
            return reinterpret_cast<iocp_op *>(ov);
        }
    };

    std::error_code last_wsa_error() noexcept {
        return std::error_code{::WSAGetLastError(), std::system_category()};
    }

    class win_socket_impl final : public socket_impl_base {
    public:
        win_socket_impl() = default;

        ~win_socket_impl() override {
            if (is_open()) {
                std::error_code ec;
                close();
            }
        }

        std::error_code open(int af, int type, int proto) noexcept override {
            if (is_open()) {
                return std::error_code{WSAEISCONN, std::system_category()};
            }
            sock_ = ::WSASocketW(af, type, proto, nullptr, 0, WSA_FLAG_OVERLAPPED);
            if (sock_ == INVALID_SOCKET) {
                return last_wsa_error();
            }
            af_ = af;
            type_ = type;
            proto_ = proto;
            return {};
        }

        std::error_code assign(int af, int type, int proto, native_socket_t native_sock) noexcept override {
            if (is_open())
                close();
            sock_ = static_cast<SOCKET>(native_sock);
            af_ = af;
            type_ = type;
            proto_ = proto;
            return {};
        }

        native_socket_t release() noexcept override {
            native_socket_t s = static_cast<native_socket_t>(sock_);
            sock_ = INVALID_SOCKET;
            return s;
        }

        std::error_code close() noexcept override {
            if (sock_ == INVALID_SOCKET) {
                return {};
            }
            int ret = ::closesocket(sock_);
            sock_ = INVALID_SOCKET;
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        std::error_code cancel() noexcept override {
            if (sock_ == INVALID_SOCKET) {
                return {};
            }
            if (!::CancelIoEx(reinterpret_cast<HANDLE>(sock_), nullptr)) {
                DWORD err = ::GetLastError();
                if (err != ERROR_NOT_FOUND) {
                    return std::error_code{static_cast<int>(err), std::system_category()};
                }
            }
            return {};
        }

        bool is_open() const noexcept override {
            return sock_ != INVALID_SOCKET;
        }

        native_socket_t native_handle() const noexcept override {
            return static_cast<native_socket_t>(sock_);
        }

        std::error_code set_option(const socket_option &opt) noexcept override {
            int ret = ::setsockopt(sock_, opt.level, opt.name, static_cast<const char *>(opt.value), static_cast<int>(opt.value_size));
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        std::error_code get_option(socket_option &opt) const noexcept override {
            int len = static_cast<int>(opt.value_size);
            int ret = ::getsockopt(sock_, opt.level, opt.name, static_cast<char *>(const_cast<void *>(opt.value)), &len);
            opt.value_size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        // ------------------------------------------------------------------
        std::error_code set_non_blocking(bool mode) noexcept override {
            non_blocking_ = mode;
            u_long arg = mode ? 1u : 0u;
            int ret = ::ioctlsocket(sock_, FIONBIO, &arg);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        bool non_blocking() const noexcept override {
            return non_blocking_;
        }

        std::error_code set_native_non_blocking(bool mode) noexcept override {
            native_non_blocking_ = mode;
            u_long arg = mode ? 1u : 0u;
            int ret = ::ioctlsocket(sock_, FIONBIO, &arg);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        bool native_non_blocking() const noexcept override {
            return native_non_blocking_;
        }

        // ------------------------------------------------------------------
        std::error_code bind(const raw_endpoint &ep) noexcept override {
            int ret = ::bind(sock_, reinterpret_cast<const ::sockaddr *>(ep.data), static_cast<int>(ep.size));
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        std::error_code connect(const raw_endpoint &ep) noexcept override {
            int ret = ::connect(sock_, reinterpret_cast<const ::sockaddr *>(ep.data), static_cast<int>(ep.size));
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        std::error_code listen(int backlog) noexcept override {
            int ret = ::listen(sock_, backlog);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        std::error_code shutdown(shutdown_how how) noexcept override {
            int sd = (how == shutdown_how::receive) ? SD_RECEIVE : (how == shutdown_how::send) ? SD_SEND : SD_BOTH;
            int ret = ::shutdown(sock_, sd);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        // ------------------------------------------------------------------
        std::error_code local_endpoint(raw_endpoint &ep) const noexcept override {
            int len = static_cast<int>(sizeof(ep.data));
            int ret = ::getsockname(sock_, reinterpret_cast<::sockaddr *>(ep.data), &len);
            if (ret == 0)
                ep.size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        std::error_code remote_endpoint(raw_endpoint &ep) const noexcept override {
            int len = static_cast<int>(sizeof(ep.data));
            int ret = ::getpeername(sock_, reinterpret_cast<::sockaddr *>(ep.data), &len);
            if (ret == 0)
                ep.size = static_cast<std::size_t>(len);
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        // ------------------------------------------------------------------
        std::ptrdiff_t send(const void *buf, std::size_t len, message_flags_t flags, std::error_code &ec) noexcept override {
            WSABUF wb{static_cast<ULONG>(len), const_cast<char *>(static_cast<const char *>(buf))};
            DWORD sent = 0;
            int ret = ::WSASend(sock_, &wb, 1, &sent, static_cast<DWORD>(flags), nullptr, nullptr);
            if (ret == SOCKET_ERROR) {
                ec = last_wsa_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(sent);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len, message_flags_t flags, std::error_code &ec) noexcept override {
            WSABUF wb{static_cast<ULONG>(len), static_cast<char *>(buf)};
            DWORD recvd = 0;
            DWORD f = static_cast<DWORD>(flags);
            int ret = ::WSARecv(sock_, &wb, 1, &recvd, &f, nullptr, nullptr);
            if (ret == SOCKET_ERROR) {
                ec = last_wsa_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(recvd);
        }

        std::ptrdiff_t send_to(const void *buf, std::size_t len, message_flags_t flags, const raw_endpoint &dest,
                               std::error_code &ec) noexcept override {
            WSABUF wb{static_cast<ULONG>(len), const_cast<char *>(static_cast<const char *>(buf))};
            DWORD sent = 0;
            int ret = ::WSASendTo(sock_, &wb, 1, &sent, static_cast<DWORD>(flags), reinterpret_cast<const ::sockaddr *>(dest.data),
                                  static_cast<int>(dest.size), nullptr, nullptr);
            if (ret == SOCKET_ERROR) {
                ec = last_wsa_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(sent);
        }

        std::ptrdiff_t receive_from(void *buf, std::size_t len, message_flags_t flags, raw_endpoint &sender,
                                    std::error_code &ec) noexcept override {
            WSABUF wb{static_cast<ULONG>(len), static_cast<char *>(buf)};
            DWORD recvd = 0;
            DWORD f = static_cast<DWORD>(flags);
            int from_len = static_cast<int>(sizeof(sender.data));
            int ret =
                ::WSARecvFrom(sock_, &wb, 1, &recvd, &f, reinterpret_cast<::sockaddr *>(sender.data), &from_len, nullptr, nullptr);
            if (ret == SOCKET_ERROR) {
                ec = last_wsa_error();
                return -1;
            }
            sender.size = static_cast<std::size_t>(from_len);
            ec.clear();
            return static_cast<std::ptrdiff_t>(recvd);
        }

        // ------------------------------------------------------------------
        native_socket_t accept(raw_endpoint *peer_ep, std::error_code &ec) noexcept override {
            SOCKET client;
            if (peer_ep) {
                int len = static_cast<int>(sizeof(peer_ep->data));
                client = ::accept(sock_, reinterpret_cast<::sockaddr *>(peer_ep->data), &len);
                if (client != INVALID_SOCKET)
                    peer_ep->size = static_cast<std::size_t>(len);
            } else {
                client = ::accept(sock_, nullptr, nullptr);
            }
            if (client == INVALID_SOCKET) {
                ec = last_wsa_error();
                return invalid_socket_value;
            }
            ec.clear();
            return static_cast<native_socket_t>(client);
        }

        // ------------------------------------------------------------------
        std::size_t available(std::error_code &ec) const noexcept override {
            u_long avail = 0;
            int ret = ::ioctlsocket(sock_, FIONREAD, &avail);
            if (ret == SOCKET_ERROR) {
                ec = last_wsa_error();
                return 0;
            }
            ec.clear();
            return static_cast<std::size_t>(avail);
        }

        bool at_mark(std::error_code &ec) const noexcept override {
            u_long oob = 0;
            int ret = ::ioctlsocket(sock_, SIOCATMARK, &oob);
            if (ret == SOCKET_ERROR) {
                ec = last_wsa_error();
                return false;
            }
            ec.clear();
            return oob != 0;
        }

        std::error_code io_control(unsigned long cmd, void *arg) noexcept override {
            int ret = ::ioctlsocket(sock_, static_cast<long>(cmd), static_cast<u_long *>(arg));
            return ret == 0 ? std::error_code{} : last_wsa_error();
        }

        // ------------------------------------------------------------------
        // 异步操作：借助 IOCP Overlapped I/O
        // op 里必须嵌入 OVERLAPPED（iocp_op），由上层调用方构造
        // ------------------------------------------------------------------
        void async_connect(const raw_endpoint &ep, io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            // ConnectEx 需要先 bind 到 INADDR_ANY
            ::sockaddr_storage any{};
            any.ss_family = static_cast<ADDRESS_FAMILY>(af_);
            ::bind(sock_, reinterpret_cast<::sockaddr *>(&any), static_cast<int>(sizeof(any)));

            bind_to_iocp(ctx_impl);

            auto *iop = static_cast<iocp_op *>(op);
            LPFN_CONNECTEX connect_ex = get_connect_ex();
            if (!connect_ex) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }
            DWORD bytes = 0;
            BOOL ok = connect_ex(sock_, reinterpret_cast<const ::sockaddr *>(ep.data), static_cast<int>(ep.size), nullptr, 0, &bytes,
                                 &iop->overlapped);
            if (!ok && ::WSAGetLastError() != ERROR_IO_PENDING) {
                ctx_impl.post_immediate_completion(op, false);
            }
        }

        void async_send(const void *buf, std::size_t len, message_flags_t flags, io_context_impl_base &ctx_impl,
                        completion_op *op) noexcept override {
            bind_to_iocp(ctx_impl);
            auto *iop = static_cast<iocp_op *>(op);
            WSABUF wb{static_cast<ULONG>(len), const_cast<char *>(static_cast<const char *>(buf))};
            DWORD sent = 0;
            int ret = ::WSASend(sock_, &wb, 1, &sent, static_cast<DWORD>(flags), &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                ctx_impl.post_immediate_completion(op, false);
            }
        }

        void async_receive(void *buf, std::size_t len, message_flags_t flags, io_context_impl_base &ctx_impl,
                           completion_op *op) noexcept override {
            bind_to_iocp(ctx_impl);
            auto *iop = static_cast<iocp_op *>(op);
            WSABUF wb{static_cast<ULONG>(len), static_cast<char *>(buf)};
            DWORD recvd = 0;
            DWORD f = static_cast<DWORD>(flags);
            int ret = ::WSARecv(sock_, &wb, 1, &recvd, &f, &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                ctx_impl.post_immediate_completion(op, false);
            }
        }

        void async_send_to(const void *buf, std::size_t len, message_flags_t flags, const raw_endpoint &dest,
                           io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            bind_to_iocp(ctx_impl);
            auto *iop = static_cast<iocp_op *>(op);
            WSABUF wb{static_cast<ULONG>(len), const_cast<char *>(static_cast<const char *>(buf))};
            DWORD sent = 0;
            int ret = ::WSASendTo(sock_, &wb, 1, &sent, static_cast<DWORD>(flags), reinterpret_cast<const ::sockaddr *>(dest.data),
                                  static_cast<int>(dest.size), &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                ctx_impl.post_immediate_completion(op, false);
            }
        }

        void async_receive_from(void *buf, std::size_t len, message_flags_t flags, raw_endpoint &sender,
                                io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            bind_to_iocp(ctx_impl);
            auto *iop = static_cast<iocp_op *>(op);
            WSABUF wb{static_cast<ULONG>(len), static_cast<char *>(buf)};
            DWORD recvd = 0;
            DWORD f = static_cast<DWORD>(flags);
            from_len_ = static_cast<int>(sizeof(sender.data));
            int ret = ::WSARecvFrom(sock_, &wb, 1, &recvd, &f, reinterpret_cast<::sockaddr *>(sender.data), &from_len_,
                                    &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                ctx_impl.post_immediate_completion(op, false);
            }
        }

        void async_accept(raw_endpoint *peer_ep, io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            bind_to_iocp(ctx_impl);
            auto *iop = static_cast<iocp_op *>(op);

            SOCKET client = ::WSASocketW(af_, type_, proto_, nullptr, 0, WSA_FLAG_OVERLAPPED);
            if (client == INVALID_SOCKET) {
                ctx_impl.post_immediate_completion(op, false);
                return;
            }
            iop->associated_handle = reinterpret_cast<HANDLE>(client);

            DWORD bytes_recv = 0;
            // AcceptEx 需要 addr_buf 大小 >= (sizeof(sockaddr) + 16) * 2
            static thread_local std::uint8_t addr_buf[256]{};
            BOOL ok =
                ::AcceptEx(sock_, client, addr_buf, 0, sizeof(addr_buf) / 2, sizeof(addr_buf) / 2, &bytes_recv, &iop->overlapped);
            if (!ok && ::WSAGetLastError() != ERROR_IO_PENDING) {
                ::closesocket(client);
                ctx_impl.post_immediate_completion(op, false);
            }
        }

        void async_wait(wait_type w, io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            (void) w;
            ctx_impl.post_immediate_completion(op, false);
        }

    private:
        void bind_to_iocp(io_context_impl_base &ctx_impl) noexcept {
            if (!bound_to_iocp_) {
                ctx_impl.associate_handle(nullptr, static_cast<std::uintptr_t>(sock_), nullptr);
                bound_to_iocp_ = true;
            }
        }

        LPFN_CONNECTEX get_connect_ex() noexcept {
            LPFN_CONNECTEX fn = nullptr;
            GUID guid = WSAID_CONNECTEX;
            DWORD bytes = 0;
            ::WSAIoctl(sock_, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &fn, sizeof(fn), &bytes, nullptr, nullptr);
            return fn;
        }

        SOCKET sock_{INVALID_SOCKET};
        int af_{0};
        int type_{0};
        int proto_{0};
        int from_len_{0};
        bool bound_to_iocp_{false};
    };

    memory::nebula_ptr<socket_impl_base> create_socket_impl() {
        return memory::make_nebula<win_socket_impl>();
    }
}
