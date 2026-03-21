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
#ifndef RAINY_FOUNDATION_IO_NET_SOCKET_HPP
#define RAINY_FOUNDATION_IO_NET_SOCKET_HPP
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/timer.hpp>

namespace rainy::foundation::net::io {
    template <typename Protocol>
    class basic_socket;

    template <typename Protocol>
    class basic_datagram_socket;

    template <typename Protocol>
    class basic_stream_socket;

    template <typename Protocol>
    class basic_socket_acceptor;

    template <typename Protocol, typename Clock = std::chrono::steady_clock, typename WaitTraits = wait_traits<Clock>>
    class basic_socket_streambuf;

    template <typename Protocol, typename Clock = std::chrono::steady_clock, typename WaitTraits = wait_traits<Clock>>
    class basic_socket_iostream;
}

#endif