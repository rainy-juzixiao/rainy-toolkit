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
#ifndef RAINY_FOUNDATION_IO_NET_IMPLEMENTS_SOCK_HPP
#define RAINY_FOUNDATION_IO_NET_IMPLEMENTS_SOCK_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <rainy/foundation/io/net/io_context.hpp>
#include <system_error>

namespace rainy::foundation::io::net::implements {
#if RAINY_USING_WINDOWS
    using native_socket_t = std::uintptr_t; // SOCKET
    static constexpr native_socket_t invalid_socket_value = static_cast<native_socket_t>(~0ULL); // INVALID_SOCKET
#else
    using native_socket_t = int;
    static constexpr native_socket_t invalid_socket_value = -1;
#endif

    enum class shutdown_how : int {
        receive = 0,
        send = 1,
        both = 2
    };

    enum class wait_type : int {
        read = 0,
        write = 1,
        error = 2
    };

    using message_flags_t = int;
    static constexpr message_flags_t msg_peek = 0x01;
    static constexpr message_flags_t msg_out_of_band = 0x02;
    static constexpr message_flags_t msg_do_not_route = 0x04;

    struct raw_endpoint {
        std::uint8_t data[128]{};
        std::size_t size{0};
        int family{0};
    };

    struct socket_option {
        int level{0};
        int name{0};
        const void *value{nullptr};
        std::size_t value_size{0};
    };

    class socket_impl_base {
    public:
        virtual ~socket_impl_base() = default;

        virtual std::error_code open(int address_family, int socket_type, int protocol) noexcept = 0;

        virtual std::error_code assign(int address_family, int socket_type, int protocol, native_socket_t native_sock) noexcept = 0;

        virtual native_socket_t release() noexcept = 0;

        virtual std::error_code close() noexcept = 0;

        virtual std::error_code cancel() noexcept = 0;

        virtual bool is_open() const noexcept = 0;

        virtual native_socket_t native_handle() const noexcept = 0;

        virtual std::error_code set_option(const socket_option &opt) noexcept = 0;

        virtual std::error_code get_option(socket_option &opt) const noexcept = 0;

        virtual std::error_code set_non_blocking(bool mode) noexcept = 0;

        virtual bool non_blocking() const noexcept = 0;

        virtual std::error_code set_native_non_blocking(bool mode) noexcept = 0;

        virtual bool native_non_blocking() const noexcept = 0;

        virtual std::error_code bind(const raw_endpoint &ep) noexcept = 0;

        virtual std::error_code connect(const raw_endpoint &ep) noexcept = 0;

        virtual std::error_code listen(int backlog) noexcept = 0;

        virtual std::error_code shutdown(shutdown_how how) noexcept = 0;

        virtual std::error_code local_endpoint(raw_endpoint &ep) const noexcept = 0;

        virtual std::error_code remote_endpoint(raw_endpoint &ep) const noexcept = 0;

        virtual std::ptrdiff_t send(const void *buf, std::size_t len, message_flags_t flags, std::error_code &ec) noexcept = 0;

        virtual std::ptrdiff_t receive(void *buf, std::size_t len, message_flags_t flags, std::error_code &ec) noexcept = 0;

        virtual std::ptrdiff_t send_to(const void *buf, std::size_t len, message_flags_t flags, const raw_endpoint &dest,
                                       std::error_code &ec) noexcept = 0;

        virtual std::ptrdiff_t receive_from(void *buf, std::size_t len, message_flags_t flags, raw_endpoint &sender,
                                            std::error_code &ec) noexcept = 0;

        virtual native_socket_t accept(raw_endpoint *peer_ep, std::error_code &ec) noexcept = 0;

        virtual std::size_t available(std::error_code &ec) const noexcept = 0;

        virtual bool at_mark(std::error_code &ec) const noexcept = 0;

        virtual void async_connect(const raw_endpoint &ep, io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual void async_send(const void *buf, std::size_t len, message_flags_t flags, io_context_impl_base &ctx_impl,
                                completion_op *op) noexcept = 0;

        virtual void async_receive(void *buf, std::size_t len, message_flags_t flags, io_context_impl_base &ctx_impl,
                                   completion_op *op) noexcept = 0;

        virtual void async_send_to(const void *buf, std::size_t len, message_flags_t flags, const raw_endpoint &dest,
                                   io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual void async_receive_from(void *buf, std::size_t len, message_flags_t flags, raw_endpoint &sender,
                                        io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual void async_accept(raw_endpoint *peer_ep, io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual void async_wait(wait_type w, io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual std::error_code io_control(unsigned long cmd, void *arg) noexcept = 0;

    protected:
        bool non_blocking_{false};
        bool native_non_blocking_{false};
    };

    RAINY_TOOLKIT_API memory::nebula_ptr<socket_impl_base> create_socket_impl();
}

#endif
