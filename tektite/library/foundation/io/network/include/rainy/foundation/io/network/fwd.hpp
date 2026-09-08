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
#ifndef RAINY_FOUNDATION_IO_NET_FWD_HPP
#define RAINY_FOUNDATION_IO_NET_FWD_HPP
#include <rainy/foundation/io/fwd.hpp>

namespace rainy::foundation::io::net {
    template <typename Protocol>
    class basic_socket;

    template <typename Protocol>
    class basic_datagram_socket;

    template <typename Protocol>
    class basic_stream_socket;

    template <typename Protocol>
    class basic_socket_acceptor;
}

namespace rainy::foundation::io::net::ip {
    class address;
    class address_v4;
    class address_v6;

    template <typename Address>
    class basic_address_iterator;

    using address_v4_iterator = basic_address_iterator<address_v4>;
    using address_v6_iterator = basic_address_iterator<address_v6>;

    template <typename Address>
    class basic_address_range;

    using address_v4_range = basic_address_range<address_v4>;
    using address_v6_range = basic_address_range<address_v6>;
    class network_v4;
    class network_v6;

    template <typename InternetProtocol>
    class basic_endpoint;

    template <typename InternetProtocol>
    class basic_resolver_entry;

    template <typename InternetProtocol>
    class basic_resolver_results;

    template <typename InternetProtocol>
    class basic_resolver;
    
    class tcp;
    class udp;
}

#endif
