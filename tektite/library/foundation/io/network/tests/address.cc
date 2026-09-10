/*
* Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain left copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/io/network/internet/address.hpp>

using namespace rainy::foundation;
using namespace rainy::foundation::io::net::ip;

TEST_CASE("address_v4 can be constructed and queried", "[net][ip][address_v4]") {

    SECTION("a default-constructed address_v4") {
        constexpr address_v4 addr{};

        SECTION("it represents the unspecified address 0.0.0.0") {
            REQUIRE(addr.is_unspecified());
            REQUIRE_FALSE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_multicast());
            REQUIRE(addr.to_uint() == 0u);
        }

        SECTION("its bytes are all zero") {
            constexpr auto bytes = addr.to_bytes();
            REQUIRE(bytes[0] == 0);
            REQUIRE(bytes[1] == 0);
            REQUIRE(bytes[2] == 0);
            REQUIRE(bytes[3] == 0);
        }
    }

    SECTION("the well-known static addresses") {
        SECTION("using address_v4::any()") {
            constexpr auto addr = address_v4::any();
            SECTION("it is the unspecified address") {
                REQUIRE(addr.is_unspecified());
                REQUIRE(addr.to_uint() == 0x00000000u);
            }
        }

        SECTION("using address_v4::loopback()") {
            constexpr auto addr = address_v4::loopback();
            SECTION("it is the loopback address 127.0.0.1") {
                REQUIRE(addr.is_loopback());
                REQUIRE_FALSE(addr.is_unspecified());
                REQUIRE_FALSE(addr.is_multicast());
                REQUIRE(addr.to_uint() == 0x7F000001u);
            }
        }

        SECTION("using address_v4::broadcast()") {
            constexpr auto addr = address_v4::broadcast();
            SECTION("it holds 255.255.255.255") {
                REQUIRE(addr.to_uint() == 0xFFFFFFFFu);
                REQUIRE_FALSE(addr.is_unspecified());
                REQUIRE_FALSE(addr.is_loopback());
            }
        }
    }

    SECTION("an address_v4 constructed from a uint value") {
        // 224.0.0.1 — first multicast address
        constexpr address_v4 addr{0xE0000001u};

        SECTION("it is identified as multicast") {
            REQUIRE(addr.is_multicast());
            REQUIRE_FALSE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_unspecified());
        }

        SECTION("to_uint round-trips correctly") {
            REQUIRE(addr.to_uint() == 0xE0000001u);
        }
    }

    SECTION("an address_v4 constructed from bytes") {
        // 192.168.1.1
        constexpr address_v4::bytes_type bytes{192, 168, 1, 1};
        constexpr address_v4 addr{bytes};

        SECTION("to_bytes returns the original bytes") {
            constexpr auto out = addr.to_bytes();
            REQUIRE(out[0] == 192);
            REQUIRE(out[1] == 168);
            REQUIRE(out[2] == 1);
            REQUIRE(out[3] == 1);
        }

        SECTION("to_uint encodes bytes in big-endian order") {
            REQUIRE(addr.to_uint() == 0xC0A80101u);
        }
    }
}

TEST_CASE("address_v4 supports value comparisons", "[net][ip][address_v4]") {

    SECTION("two identical address_v4 values") {
        constexpr address_v4 a{0xC0A80101u}; // 192.168.1.1
        constexpr address_v4 b{0xC0A80101u};

        SECTION("they compare equal") {
            REQUIRE(a == b);
            REQUIRE_FALSE(a != b);
            REQUIRE_FALSE(a < b);
            REQUIRE_FALSE(a > b);
            REQUIRE(a <= b);
            REQUIRE(a >= b);
        }
    }

    SECTION("two distinct address_v4 values") {
        constexpr address_v4 lo{0xC0A80101u}; // 192.168.1.1
        constexpr address_v4 hi{0xC0A80102u}; // 192.168.1.2

        SECTION("the lesser address compares less than the greater") {
            REQUIRE(lo < hi);
            REQUIRE(lo != hi);
            REQUIRE_FALSE(lo > hi);
            REQUIRE(lo <= hi);
            REQUIRE(hi >= lo);
        }
    }
}

TEST_CASE("address_v4 make_address_v4 factory functions", "[net][ip][address_v4]") {

    SECTION("a bytes_type representing 10.0.0.1") {
        constexpr address_v4::bytes_type bytes{10, 0, 0, 1};

        SECTION("make_address_v4 is called with those bytes") {
            constexpr auto addr = make_address_v4(bytes);

            SECTION("the resulting address encodes the correct uint") {
                REQUIRE(addr.to_uint() == 0x0A000001u);
            }
        }
    }

    SECTION("a uint value 0x0A000001 representing 10.0.0.1") {
        SECTION("make_address_v4 is called with that uint") {
            constexpr auto addr = make_address_v4(address_v4::uint_type{0x0A000001u});

            SECTION("bytes are decoded correctly") {
                constexpr auto b = addr.to_bytes();
                REQUIRE(b[0] == 10);
                REQUIRE(b[1] == 0);
                REQUIRE(b[2] == 0);
                REQUIRE(b[3] == 1);
            }
        }
    }
}

TEST_CASE("address_v6 can be constructed and queried", "[net][ip][address_v6]") {

    SECTION("a default-constructed address_v6") {
        constexpr address_v6 addr{};

        SECTION("it is the unspecified address ::") {
            REQUIRE(addr.is_unspecified());
            REQUIRE_FALSE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_multicast());
            REQUIRE(addr.scope_id() == 0u);
        }

        SECTION("all bytes are zero") {
            constexpr auto bytes = addr.to_bytes();
            for (int i = 0; i < 16; ++i) {
                REQUIRE(bytes[i] == 0);
            }
        }
    }

    SECTION("the well-known static addresses") {
        SECTION("using address_v6::any()") {
            constexpr auto addr = address_v6::any();
            SECTION("it is the unspecified address") {
                REQUIRE(addr.is_unspecified());
            }
        }

        SECTION("using address_v6::loopback()") {
            constexpr auto addr = address_v6::loopback();

            SECTION("it is the loopback address ::1") {
                REQUIRE(addr.is_loopback());
                REQUIRE_FALSE(addr.is_unspecified());
            }

            SECTION("only the last byte is 1") {
                constexpr auto bytes = addr.to_bytes();
                for (int i = 0; i < 15; ++i) {
                    REQUIRE(bytes[i] == 0);
                }
                REQUIRE(bytes[15] == 1);
            }
        }
    }

    SECTION("an address_v6 constructed from bytes representing a multicast address ff02::1") {
        // ff02::1 — all-nodes link-local multicast
        constexpr address_v6::bytes_type bytes{
            0xFF, 0x02, 0, 0, 0, 0, 0, 0,
            0,    0,    0, 0, 0, 0, 0, 1
        };
        constexpr address_v6 addr{bytes};

        SECTION("it is identified as multicast") {
            REQUIRE(addr.is_multicast());
            REQUIRE(addr.is_multicast_link_local());
            REQUIRE_FALSE(addr.is_multicast_global());
            REQUIRE_FALSE(addr.is_multicast_node_local());
            REQUIRE_FALSE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_unspecified());
        }
    }

    SECTION("an address_v6 representing a link-local address fe80::1") {
        constexpr address_v6::bytes_type bytes{
            0xFE, 0x80, 0, 0, 0, 0, 0, 0,
            0,    0,    0, 0, 0, 0, 0, 1
        };
        constexpr address_v6 addr{bytes};

        SECTION("it is identified as link-local") {
            REQUIRE(addr.is_link_local());
            REQUIRE_FALSE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_multicast());
            REQUIRE_FALSE(addr.is_site_local());
        }
    }

    SECTION("an address_v6 representing a site-local address fec0::1") {
        constexpr address_v6::bytes_type bytes{
            0xFE, 0xC0, 0, 0, 0, 0, 0, 0,
            0,    0,    0, 0, 0, 0, 0, 1
        };
        constexpr address_v6 addr{bytes};

        SECTION("it is identified as site-local") {
            REQUIRE(addr.is_site_local());
            REQUIRE_FALSE(addr.is_link_local());
        }
    }

    SECTION("a v4-mapped address ::ffff:192.168.1.1") {
        constexpr address_v6::bytes_type bytes{
            0,    0,    0,    0,    0,    0, 0, 0,
            0,    0,    0xFF, 0xFF, 192,  168, 1, 1
        };
        constexpr address_v6 addr{bytes};

        SECTION("it is identified as v4-mapped") {
            REQUIRE(addr.is_v4_mapped());
            REQUIRE_FALSE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_multicast());
        }

        SECTION("make_address_v4 with v4_mapped extracts 192.168.1.1") {
            constexpr auto v4 = make_address_v4(v4_mapped, addr);
            REQUIRE(v4.to_uint() == 0xC0A80101u);
        }
    }

    SECTION("an address_v6 with a non-zero scope_id") {
        constexpr address_v6::bytes_type bytes{
            0xFE, 0x80, 0, 0, 0, 0, 0, 0,
            0,    0,    0, 0, 0, 0, 0, 1
        };
        constexpr address_v6 addr{bytes, 3u};

        SECTION("scope_id is preserved") {
            REQUIRE(addr.scope_id() == 3u);
        }

        SECTION("scope_id can be mutated") {
            address_v6 mutable_addr{bytes, 3u};
            mutable_addr.scope_id(7u);
            REQUIRE(mutable_addr.scope_id() == 7u);
        }
    }
}

TEST_CASE("address_v6 supports value comparisons", "[net][ip][address_v6]") {

    SECTION("two identical address_v6 values") {
        constexpr auto a = address_v6::loopback();
        constexpr auto b = address_v6::loopback();

        SECTION("they compare equal") {
            REQUIRE(a == b);
            REQUIRE_FALSE(a != b);
            REQUIRE_FALSE(a < b);
            REQUIRE(a <= b);
            REQUIRE(a >= b);
        }
    }

    SECTION("the unspecified and loopback address_v6") {
        constexpr auto unspec  = address_v6::any();
        constexpr auto loopbk  = address_v6::loopback();

        SECTION("unspecified is less than loopback") {
            REQUIRE(unspec < loopbk);
            REQUIRE(unspec != loopbk);
            REQUIRE(loopbk > unspec);
        }
    }

    SECTION("two address_v6 values with different scope_ids") {
        constexpr address_v6::bytes_type bytes{
            0xFE, 0x80, 0, 0, 0, 0, 0, 0,
            0,    0,    0, 0, 0, 0, 0, 1
        };
        constexpr address_v6 low_scope{bytes, 1u};
        constexpr address_v6 high_scope{bytes, 2u};

        SECTION("scope_id participates in ordering") {
            REQUIRE(low_scope < high_scope);
            REQUIRE(low_scope != high_scope);
        }
    }
}

TEST_CASE("address_v6 make_address_v6 factory functions", "[net][ip][address_v6]") {

    SECTION("a v4-mapped address built from address_v4 192.168.0.1") {
        constexpr address_v4 v4{0xC0A80001u};

        SECTION("make_address_v6 is called with v4_mapped") {
            constexpr auto v6 = make_address_v6(v4_mapped, v4);

            SECTION("the result is v4-mapped") {
                REQUIRE(v6.is_v4_mapped());
            }

            SECTION("bytes 10-11 are 0xFF and bytes 12-15 carry the v4 address") {
                constexpr auto b = v6.to_bytes();
                REQUIRE(b[10] == 0xFF);
                REQUIRE(b[11] == 0xFF);
                REQUIRE(b[12] == 192);
                REQUIRE(b[13] == 168);
                REQUIRE(b[14] == 0);
                REQUIRE(b[15] == 1);
            }

            SECTION("round-tripping back to v4 yields the original address") {
                constexpr auto back = make_address_v4(v4_mapped, v6);
                REQUIRE(back == v4);
            }
        }
    }

    SECTION("a bytes_type and a scope_id") {
        constexpr address_v6::bytes_type bytes{
            0xFE, 0x80, 0, 0, 0, 0, 0, 0,
            0,    0,    0, 0, 0, 0, 0, 1
        };
        SECTION("make_address_v6 is called with those bytes and scope 5") {
            constexpr auto addr = make_address_v6(bytes, 5u);
            SECTION("the scope_id is preserved") {
                REQUIRE(addr.scope_id() == 5u);
            }
        }
    }
}

TEST_CASE("address can wrap either an address_v4 or address_v6", "[net][ip][address]") {

    SECTION("a default-constructed address") {
        constexpr address addr{};

        SECTION("it defaults to v4 unspecified") {
            REQUIRE(addr.is_v4());
            REQUIRE_FALSE(addr.is_v6());
            REQUIRE(addr.is_unspecified());
        }
    }

    SECTION("an address constructed from address_v4::loopback()") {
        constexpr address addr{address_v4::loopback()};

        SECTION("it is v4") {
            REQUIRE(addr.is_v4());
            REQUIRE_FALSE(addr.is_v6());
        }

        SECTION("it is loopback") {
            REQUIRE(addr.is_loopback());
            REQUIRE_FALSE(addr.is_unspecified());
            REQUIRE_FALSE(addr.is_multicast());
        }

        SECTION("to_v4 returns the original address") {
            REQUIRE(addr.to_v4() == address_v4::loopback());
        }

        SECTION("to_v6 throws bad_address_cast") {
            REQUIRE_THROWS_AS(addr.to_v6(), exceptions::io::bad_address_cast);
        }
    }

    SECTION("an address constructed from address_v6::loopback()") {
        constexpr address addr{address_v6::loopback()};

        SECTION("it is v6") {
            REQUIRE(addr.is_v6());
            REQUIRE_FALSE(addr.is_v4());
        }

        SECTION("it is loopback") {
            REQUIRE(addr.is_loopback());
        }

        SECTION("to_v6 returns the original address") {
            REQUIRE(addr.to_v6() == address_v6::loopback());
        }

        SECTION("to_v4 throws bad_address_cast") {
            REQUIRE_THROWS_AS(addr.to_v4(),
                exceptions::io::bad_address_cast);
        }
    }

    SECTION("an address assigned from address_v4") {
        address addr{};
        addr = address_v4::broadcast();

        SECTION("it is v4 and holds the broadcast address") {
            REQUIRE(addr.is_v4());
            REQUIRE(addr.to_v4() == address_v4::broadcast());
        }
    }

    SECTION("an address assigned from address_v6") {
        address addr{address_v4::loopback()};
        addr = address_v6::loopback();

        SECTION("it switches to v6") {
            REQUIRE(addr.is_v6());
            REQUIRE_FALSE(addr.is_v4());
        }
    }
}

TEST_CASE("address supports value comparisons", "[net][ip][address]") {

    SECTION("two identical v4 addresses") {
        constexpr address a{address_v4::loopback()};
        constexpr address b{address_v4::loopback()};

        SECTION("they compare equal") {
            REQUIRE(a == b);
            REQUIRE_FALSE(a != b);
        }
    }

    SECTION("a v4 address and a v6 address") {
        constexpr address v4_addr{address_v4::loopback()};
        constexpr address v6_addr{address_v6::loopback()};

        SECTION("they are not equal") {
            REQUIRE(v4_addr != v6_addr);
        }

        SECTION("v4 is ordered before v6") {
            REQUIRE(v4_addr < v6_addr);
            REQUIRE(v6_addr > v4_addr);
            REQUIRE(v4_addr <= v6_addr);
            REQUIRE(v6_addr >= v4_addr);
        }
    }

    SECTION("two identical v6 addresses") {
        constexpr address a{address_v6::loopback()};
        constexpr address b{address_v6::loopback()};

        SECTION("they compare equal") {
            REQUIRE(a == b);
            REQUIRE_FALSE(a < b);
        }
    }
}

TEST_CASE("basic_address_iterator advances through address_v4 space", "[net][ip][iterator]") {

    SECTION("an iterator at 10.0.0.1") {
        basic_address_iterator<address_v4> it{address_v4{0x0A000001u}};

        SECTION("pre-incremented once") {
            ++it;
            SECTION("it points to 10.0.0.2") {
                REQUIRE((*it).to_uint() == 0x0A000002u);
            }
        }

        SECTION("post-incremented once") {
            auto prev = it++;
            SECTION("the returned copy still points to 10.0.0.1") {
                REQUIRE((*prev).to_uint() == 0x0A000001u);
            }
            SECTION("the iterator itself points to 10.0.0.2") {
                REQUIRE((*it).to_uint() == 0x0A000002u);
            }
        }

        SECTION("pre-decremented once") {
            --it;
            SECTION("it points to 10.0.0.0") {
                REQUIRE((*it).to_uint() == 0x0A000000u);
            }
        }

        SECTION("post-decremented once") {
            auto prev = it--;
            SECTION("the returned copy still points to 10.0.0.1") {
                REQUIRE((*prev).to_uint() == 0x0A000001u);
            }
            SECTION("the iterator itself points to 10.0.0.0") {
                REQUIRE((*it).to_uint() == 0x0A000000u);
            }
        }
    }
}

TEST_CASE("basic_address_iterator advances through address_v6 space", "[net][ip][iterator]") {

    SECTION("an iterator at ::1 (loopback)") {
        basic_address_iterator<address_v6> it{address_v6::loopback()};

        SECTION("pre-incremented once") {
            ++it;
            SECTION("it points to ::2") {
                const auto bytes = (*it).to_bytes();
                REQUIRE(bytes[15] == 2);
                for (int i = 0; i < 15; ++i) REQUIRE(bytes[i] == 0);
            }
        }

        SECTION("pre-decremented once") {
            --it;
            SECTION("it points to :: (all-zeros)") {
                REQUIRE((*it).is_unspecified());
            }
        }
    }
}

TEST_CASE("basic_address_range covers a contiguous address_v4 range", "[net][ip][range]") {

    SECTION("a range from 10.0.0.1 to 10.0.0.4 (exclusive)") {
        const address_v4 first{0x0A000001u};
        const address_v4 last {0x0A000004u};
        const basic_address_range<address_v4> range{first, last};

        SECTION("it is not empty") {
            REQUIRE_FALSE(range.empty());
        }

        SECTION("its size is 3") {
            REQUIRE(range.size() == 3u);
        }

        SECTION("iteration covers exactly 10.0.0.1, 10.0.0.2, 10.0.0.3") {
            std::vector<address_v4::uint_type> seen;
            for (const auto &addr : range) {
                seen.push_back(addr.to_uint());
            }
            REQUIRE(seen.size() == 3u);
            REQUIRE(seen[0] == 0x0A000001u);
            REQUIRE(seen[1] == 0x0A000002u);
            REQUIRE(seen[2] == 0x0A000003u);
        }

        SECTION("find returns a valid iterator for an address inside the range") {
            const address_v4 target{0x0A000002u};
            const auto it = range.find(target);
            REQUIRE(it != range.end());
            REQUIRE(*it == target);
        }

        SECTION("find returns end() for an address outside the range") {
            const address_v4 outside{0x0A000005u};
            REQUIRE(range.find(outside) == range.end());
        }
    }

    SECTION("a default-constructed address_v4 range") {
        const basic_address_range<address_v4> range{};

        SECTION("it is empty") {
            REQUIRE(range.empty());
            REQUIRE(range.size() == 0u);
        }
    }

    SECTION("a range where first equals last") {
        const address_v4 point{0xC0A80101u};
        const basic_address_range<address_v4> range{point, point};

        SECTION("it is empty") {
            REQUIRE(range.empty());
            REQUIRE(range.size() == 0u);
        }
    }
}

TEST_CASE("basic_address_range covers a contiguous address_v6 range", "[net][ip][range]") {

    SECTION("a range from ::1 to ::4 (exclusive)") {
        const auto first = address_v6::loopback();   // ::1

        address_v6::bytes_type last_bytes{};
        last_bytes[15] = 4;
        const address_v6 last{last_bytes};           // ::4

        const basic_address_range<address_v6> range{first, last};

        SECTION("it is not empty") {
            REQUIRE_FALSE(range.empty());
        }

        SECTION("its size is 3") {
            REQUIRE(range.size() == 3u);
        }

        SECTION("iteration covers ::1, ::2, ::3") {
            std::vector<unsigned char> last_bytes_seen;
            for (const auto &addr : range) {
                last_bytes_seen.push_back(addr.to_bytes()[15]);
            }
            REQUIRE(last_bytes_seen.size() == 3u);
            REQUIRE(last_bytes_seen[0] == 1);
            REQUIRE(last_bytes_seen[1] == 2);
            REQUIRE(last_bytes_seen[2] == 3);
        }

        SECTION("find locates ::2 within the range") {
            address_v6::bytes_type target_bytes{};
            target_bytes[15] = 2;
            const address_v6 target{target_bytes};
            const auto it = range.find(target);
            REQUIRE(it != range.end());
            REQUIRE(*it == target);
        }
    }

    SECTION("a default-constructed address_v6 range") {
        const basic_address_range<address_v6> range{};

        SECTION("it is empty") {
            REQUIRE(range.empty());
            REQUIRE(range.size() == 0u);
        }
    }
}