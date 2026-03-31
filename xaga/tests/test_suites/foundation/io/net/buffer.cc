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
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <numeric>
#include <rainy/foundation/io/net/buffer.hpp>

using namespace rainy;
using namespace rainy::foundation::io::net;

namespace {
    struct fake_sync_stream {
        std::vector<char> data;
        std::size_t read_pos{0};
        std::size_t write_pos{0};
        std::size_t max_read_per_call{(std::numeric_limits<std::size_t>::max)()};

        std::size_t read_some(mutable_buffer buf, std::error_code &ec) {
            ec.clear();
            if (read_pos >= data.size()) {
                ec = make_error_code(stream_errc::eof);
                return 0;
            }
            std::size_t avail = data.size() - read_pos;
            std::size_t n = (std::min)({buf.size(), avail, max_read_per_call});
            std::memcpy(buf.data(), data.data() + read_pos, n);
            read_pos += n;
            return n;
        }

        std::size_t write_some(const_buffer buf, std::error_code &ec) {
            ec.clear();
            std::size_t n = buf.size();
            const char *p = static_cast<const char *>(buf.data());
            data.insert(data.end(), p, p + n);
            write_pos += n;
            return n;
        }
    };
}

SCENARIO("stream_errc values produce correct error codes", "[buffer][stream_errc]") {
    GIVEN("stream_errc::eof and stream_errc::not_found") {
        WHEN("make_error_code is called for eof") {
            auto ec = make_error_code(stream_errc::eof);
            THEN("error code is non-zero") {
                REQUIRE(ec.value() != 0);
            }
            THEN("category is stream_category") {
                REQUIRE(ec.category() == stream_category());
            }
            THEN("message is not empty") {
                REQUIRE_FALSE(ec.message().empty());
            }
        }
        WHEN("make_error_code is called for not_found") {
            auto ec = make_error_code(stream_errc::not_found);

            THEN("error code is non-zero and differs from eof") {
                REQUIRE(ec.value() != 0);
                REQUIRE(ec.value() != make_error_code(stream_errc::eof).value());
            }
        }

        WHEN("make_error_condition is called") {
            auto cond = make_error_condition(stream_errc::eof);

            THEN("condition value matches error code value") {
                REQUIRE(cond.value() == make_error_code(stream_errc::eof).value());
            }
        }

        WHEN("std::is_error_code_enum specialisation is checked") {
            THEN("it is true") {
                REQUIRE(std::is_error_code_enum<stream_errc>::value);
            }
        }
    }
}

SCENARIO("mutable_buffer default construction yields empty buffer", "[buffer][mutable_buffer]") {
    GIVEN("a default-constructed mutable_buffer") {
        mutable_buffer b;
        THEN("data() is nullptr") {
            REQUIRE(b.data() == nullptr);
        }
        THEN("size() is 0") {
            REQUIRE(b.size() == 0);
        }
    }
}

SCENARIO("mutable_buffer constructed from pointer and size", "[buffer][mutable_buffer]") {
    GIVEN("a char array and a mutable_buffer pointing to it") {
        char arr[16]{};
        mutable_buffer b{arr, sizeof(arr)};

        THEN("data() points to the array") {
            REQUIRE(b.data() == arr);
        }
        THEN("size() equals array size") {
            REQUIRE(b.size() == 16);
        }

        WHEN("operator+= advances by 4") {
            b += 4;
            THEN("data() advances by 4 bytes") {
                REQUIRE(b.data() == arr + 4);
            }
            THEN("size() decreases by 4") {
                REQUIRE(b.size() == 12);
            }
        }

        WHEN("operator+= advances by more than size") {
            b += 100;
            THEN("size() clamps to 0") {
                REQUIRE(b.size() == 0);
            }
        }

        WHEN("operator+ (buffer + n) is used") {
            auto b2 = b + 4;
            THEN("original buffer is unchanged") {
                REQUIRE(b.size() == 16);
            }
            THEN("new buffer is advanced") {
                REQUIRE(b2.size() == 12);
            }
        }

        WHEN("operator+ (n + buffer) is used") {
            auto b2 = 4 + b;
            THEN("result equals buffer + n") {
                REQUIRE(b2.size() == 12);
            }
        }
    }
}

SCENARIO("const_buffer construction and advancement", "[buffer][const_buffer]") {
    GIVEN("a const_buffer default constructed") {
        const_buffer b;
        THEN("data() is nullptr") {
            REQUIRE(b.data() == nullptr);
        }
        THEN("size() is 0") {
            REQUIRE(b.size() == 0);
        }
    }

    GIVEN("a const_buffer from pointer and size") {
        const char data[] = "hello";
        const_buffer b{data, sizeof(data)};

        THEN("data() points correctly") {
            REQUIRE(b.data() == data);
        }
        THEN("size() is correct") {
            REQUIRE(b.size() == sizeof(data));
        }

        WHEN("operator+= advances by 2") {
            b += 2;
            THEN("data advances") {
                REQUIRE(b.data() == data + 2);
            }
            THEN("size shrinks") {
                REQUIRE(b.size() == sizeof(data) - 2);
            }
        }
    }

    GIVEN("a const_buffer constructed from mutable_buffer") {
        char arr[8]{};
        mutable_buffer mb{arr, sizeof(arr)};
        const_buffer cb{mb};

        THEN("data and size match the mutable_buffer") {
            REQUIRE(cb.data() == mb.data());
            REQUIRE(cb.size() == mb.size());
        }
    }

    GIVEN("const_buffer operator+ variants") {
        const char data[10]{};
        const_buffer b{data, sizeof(data)};

        WHEN("b + 3 is computed") {
            auto b2 = b + 3;
            THEN("original unchanged") {
                REQUIRE(b.size() == 10);
            }
            THEN("new buffer advanced") {
                REQUIRE(b2.size() == 7);
            }
        }

        WHEN("3 + b is computed") {
            auto b2 = 3 + b;
            THEN("same result as b + 3") {
                REQUIRE(b2.size() == 7);
            }
        }
    }
}

SCENARIO("buffer() creates correct mutable_buffer from raw pointer", "[buffer][factory]") {
    GIVEN("a raw memory region") {
        char mem[32]{};

        WHEN("buffer(void*, n) is called") {
            auto b = buffer(static_cast<void *>(mem), 32);
            THEN("data and size are correct") {
                REQUIRE(b.data() == mem);
                REQUIRE(b.size() == 32);
            }
        }
    }
}

SCENARIO("buffer() creates correct const_buffer from const pointer", "[buffer][factory]") {
    GIVEN("a const raw memory region") {
        const char mem[32]{};

        WHEN("buffer(const void*, n) is called") {
            auto b = buffer(static_cast<const void *>(mem), 32);
            THEN("data and size are correct") {
                REQUIRE(b.data() == mem);
                REQUIRE(b.size() == 32);
            }
        }
    }
}

SCENARIO("buffer() with size limit clamps to min(container, n)", "[buffer][factory]") {
    GIVEN("a mutable_buffer of size 16") {
        char arr[16]{};
        mutable_buffer mb{arr, 16};

        WHEN("buffer(mb, 8) is called") {
            auto b = buffer(mb, 8);
            THEN("size is clamped to 8") {
                REQUIRE(b.size() == 8);
            }
        }

        WHEN("buffer(mb, 100) is called") {
            auto b = buffer(mb, 100);
            THEN("size is clamped to 16") {
                REQUIRE(b.size() == 16);
            }
        }
    }
}

SCENARIO("buffer() from C array", "[buffer][factory]") {
    GIVEN("a char array") {
        char arr[10]{};

        WHEN("buffer(arr) is called") {
            auto b = buffer(arr);
            THEN("size equals array byte size") {
                REQUIRE(b.size() == 10);
            }
        }

        WHEN("buffer(arr, 5) is called") {
            auto b = buffer(arr, 5);
            THEN("size is clamped to 5") {
                REQUIRE(b.size() == 5);
            }
        }
    }

    GIVEN("a const char array") {
        const char arr[10]{};

        WHEN("buffer(arr) is called") {
            auto b = buffer(arr);
            THEN("size equals array byte size") {
                REQUIRE(b.size() == 10);
            }
        }
    }
}

SCENARIO("buffer() from collections::vector", "[buffer][factory]") {
    GIVEN("a vector<char> with 8 elements") {
        collections::vector<char> vec(8, 'a');

        WHEN("buffer(vec) is called") {
            auto b = buffer(vec);
            THEN("data points to vec.data()") {
                REQUIRE(b.data() == vec.data());
            }
            THEN("size equals 8") {
                REQUIRE(b.size() == 8);
            }
        }
    }

    GIVEN("a const vector<int> with 4 elements") {
        const collections::vector<int> vec{1, 2, 3, 4};

        WHEN("buffer(vec) is called") {
            auto b = buffer(vec);
            THEN("size equals 4 * sizeof(int)") {
                REQUIRE(b.size() == 4 * sizeof(int));
            }
        }
    }
}

SCENARIO("buffer_size returns total bytes of a buffer sequence", "[buffer][buffer_size]") {
    GIVEN("a single const_buffer of 12 bytes") {
        char arr[12]{};
        const_buffer b{arr, 12};

        THEN("buffer_size returns 12") {
            REQUIRE(buffer_size(b) == 12);
        }
    }

    GIVEN("a vector of const_buffers") {
        char a[4]{}, b_arr[8]{}, c[16]{};
        collections::vector<const_buffer> seq{{a, 4}, {b_arr, 8}, {c, 16}};

        THEN("buffer_size returns 28") {
            REQUIRE(buffer_size(seq) == 28);
        }
    }
}

SCENARIO("buffer_copy copies bytes between buffer sequences", "[buffer][buffer_copy]") {
    GIVEN("a source const_buffer with known content") {
        const char src[] = "abcdefgh";
        const_buffer source{src, 8};

        AND_GIVEN("a destination mutable_buffer of the same size") {
            char dst[8]{};
            mutable_buffer dest{dst, 8};

            WHEN("buffer_copy is called") {
                auto n = buffer_copy(dest, source);

                THEN("all 8 bytes are copied") {
                    REQUIRE(n == 8);
                    REQUIRE(std::memcmp(dst, src, 8) == 0);
                }
            }
        }

        AND_GIVEN("a destination buffer smaller than source") {
            char dst[4]{};
            mutable_buffer dest{dst, 4};

            WHEN("buffer_copy is called") {
                auto n = buffer_copy(dest, source);

                THEN("only 4 bytes are copied") {
                    REQUIRE(n == 4);
                    REQUIRE(std::memcmp(dst, src, 4) == 0);
                }
            }
        }

        AND_GIVEN("a max_size limit smaller than both buffers") {
            char dst[8]{};
            mutable_buffer dest{dst, 8};

            WHEN("buffer_copy with max_size=3 is called") {
                auto n = buffer_copy(dest, source, 3);

                THEN("only 3 bytes are copied") {
                    REQUIRE(n == 3);
                    REQUIRE(std::memcmp(dst, src, 3) == 0);
                }
            }
        }
    }
}

SCENARIO("buffer_sequence_begin/end for single buffers act as single-element range", "[buffer][sequence]") {

    GIVEN("a mutable_buffer") {
        char arr[4]{};
        mutable_buffer mb{arr, 4};

        THEN("begin+1 equals end") {
            REQUIRE(buffer_sequence_begin(mb) + 1 == buffer_sequence_end(mb));
        }
    }

    GIVEN("a const_buffer") {
        const char arr[4]{};
        const_buffer cb{arr, 4};

        THEN("begin+1 equals end") {
            REQUIRE(buffer_sequence_begin(cb) + 1 == buffer_sequence_end(cb));
        }
    }
}

SCENARIO("is_dynamic_buffer correctly identifies dynamic buffer types", "[buffer][traits]") {

    GIVEN("dynamic_vector_buffer") {
        THEN("is_dynamic_buffer_v is true") {
            REQUIRE(is_dynamic_buffer_v<dynamic_vector_buffer<char, std::allocator<char>>>);
        }
    }

    GIVEN("dynamic_string_buffer") {
        THEN("is_dynamic_buffer_v is true") {
            using str_t = foundation::text::basic_string<char>;
            REQUIRE(is_dynamic_buffer_v<dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>>);
        }
    }

    GIVEN("plain mutable_buffer") {
        THEN("is_dynamic_buffer_v is false") {
            REQUIRE_FALSE(is_dynamic_buffer_v<mutable_buffer>);
        }
    }
}

SCENARIO("dynamic_vector_buffer size / max_size / capacity", "[buffer][dynamic_vector_buffer]") {

    GIVEN("a vector<char> with 4 elements and a dynamic_vector_buffer wrapping it") {
        collections::vector<char> vec(4, 'x');
        auto db = dynamic_buffer(vec);

        THEN("size() equals vector size") {
            REQUIRE(db.size() == 4);
        }
        THEN("max_size() equals vector max_size()") {
            REQUIRE(db.max_size() == vec.max_size());
        }
    }

    GIVEN("a dynamic_vector_buffer with an explicit maximum_size") {
        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 64);

        THEN("max_size() equals the specified limit") {
            REQUIRE(db.max_size() == 64);
        }
    }
}

SCENARIO("dynamic_vector_buffer prepare / commit / consume lifecycle", "[buffer][dynamic_vector_buffer]") {

    GIVEN("an empty vector and a dynamic_vector_buffer") {
        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 256);

        WHEN("prepare(8) is called") {
            auto mb = db.prepare(8);

            THEN("returned buffer has 8 bytes") {
                REQUIRE(mb.size() == 8 * sizeof(char));
            }
            THEN("size() is still 0 before commit") {
                REQUIRE(db.size() == 0);
            }

            AND_WHEN("the buffer is written and commit(8) is called") {
                std::memset(mb.data(), 'Z', 8);
                db.commit(8);

                THEN("size() is now 8") {
                    REQUIRE(db.size() == 8);
                }
                THEN("data() reflects the written content") {
                    auto cb = db.data();
                    const char *p = static_cast<const char *>(cb.data());
                    REQUIRE(std::all_of(p, p + 8, [](char c) { return c == 'Z'; }));
                }

                AND_WHEN("consume(4) is called") {
                    db.consume(4);

                    THEN("size() decreases by 4") {
                        REQUIRE(db.size() == 4);
                    }
                }
            }
        }

        WHEN("prepare exceeds max_size") {
            THEN("prepare throws std::length_error") {
                REQUIRE_THROWS_AS(db.prepare(257), std::length_error);
            }
        }
    }
}

SCENARIO("dynamic_string_buffer prepare / commit / consume lifecycle", "[buffer][dynamic_string_buffer]") {

    GIVEN("an empty basic_string and a dynamic_string_buffer") {
        foundation::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 128);

        WHEN("prepare(5) is called and data is written") {
            auto mb = db.prepare(5);
            std::memcpy(mb.data(), "hello", 5);
            db.commit(5);

            THEN("size() is 5") {
                REQUIRE(db.size() == 5);
            }
            THEN("underlying string contains the written data") {
                REQUIRE(str.substr(0, 5) == "hello");
            }

            AND_WHEN("consume(5) is called") {
                db.consume(5);
                THEN("size() is 0") {
                    REQUIRE(db.size() == 0);
                }
            }
        }
    }
}

SCENARIO("transfer_all allows unlimited transfer until error", "[buffer][transfer]") {
    GIVEN("transfer_all{}") {
        transfer_all cond;

        WHEN("called with no error and any byte count") {
            THEN("returns non-zero (continue)") {
                REQUIRE(cond(std::error_code{}, 0) > 0);
                REQUIRE(cond(std::error_code{}, 100) > 0);
            }
        }

        WHEN("called with an error") {
            THEN("returns 0 (stop)") {
                REQUIRE(cond(make_error_code(stream_errc::eof), 4) == 0);
            }
        }
    }
}

SCENARIO("transfer_at_least stops only once minimum bytes are transferred", "[buffer][transfer]") {
    GIVEN("transfer_at_least{10}") {
        transfer_at_least cond{10};

        WHEN("fewer than 10 bytes transferred and no error") {
            THEN("returns non-zero (continue)") {
                REQUIRE(cond(std::error_code{}, 9) > 0);
            }
        }

        WHEN("exactly 10 bytes transferred") {
            THEN("returns 0 (stop)") {
                REQUIRE(cond(std::error_code{}, 10) == 0);
            }
        }

        WHEN("more than 10 bytes transferred") {
            THEN("returns 0 (stop)") {
                REQUIRE(cond(std::error_code{}, 20) == 0);
            }
        }

        WHEN("called with an error") {
            THEN("returns 0 regardless of byte count") {
                REQUIRE(cond(make_error_code(stream_errc::eof), 0) == 0);
            }
        }
    }
}

SCENARIO("transfer_exactly stops when the exact byte count is reached", "[buffer][transfer]") {
    GIVEN("transfer_exactly{8}") {
        transfer_exactly cond{8};

        WHEN("fewer than 8 bytes transferred") {
            THEN("returns non-zero (continue) reflecting remaining bytes") {
                REQUIRE(cond(std::error_code{}, 3) > 0);
            }
        }

        WHEN("exactly 8 bytes transferred") {
            THEN("returns 0 (stop)") {
                REQUIRE(cond(std::error_code{}, 8) == 0);
            }
        }

        WHEN("called with an error") {
            THEN("returns 0") {
                REQUIRE(cond(make_error_code(stream_errc::eof), 0) == 0);
            }
        }
    }
}

SCENARIO("read() reads all bytes from a sync stream into a fixed buffer", "[buffer][read]") {

    GIVEN("a fake stream with 8 bytes of data") {
        fake_sync_stream stream;
        stream.data = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

        char out[8]{};
        mutable_buffer buf{out, 8};

        WHEN("read(stream, buf) is called") {
            auto n = read(stream, buf);

            THEN("8 bytes are read") {
                REQUIRE(n == 8);
            }
            THEN("content matches source") {
                REQUIRE(std::memcmp(out, stream.data.data(), 8) == 0);
            }
        }
    }

    GIVEN("a fake stream with 8 bytes, max 2 per read_some call") {
        fake_sync_stream stream;
        stream.data = {'1', '2', '3', '4', '5', '6', '7', '8'};
        stream.max_read_per_call = 2;

        char out[8]{};
        mutable_buffer buf{out, 8};

        WHEN("read(stream, buf) is called") {
            auto n = read(stream, buf);

            THEN("all 8 bytes are eventually read across multiple calls") {
                REQUIRE(n == 8);
            }
        }
    }

    GIVEN("a fake stream and read with transfer_at_least(4)") {
        fake_sync_stream stream;
        stream.data.resize(10, 'x');

        char out[10]{};
        mutable_buffer buf{out, 10};

        WHEN("read(stream, buf, transfer_at_least{4}) is called") {
            auto n = read(stream, buf, transfer_at_least{4});

            THEN("at least 4 bytes are read") {
                REQUIRE(n >= 4);
            }
        }
    }

    GIVEN("a fake stream and read with error_code overload") {
        fake_sync_stream stream;
        stream.data = {'X', 'Y'};

        char out[8]{};
        mutable_buffer buf{out, 8};
        std::error_code ec;

        WHEN("read(stream, buf, ec) is called") {
            auto n = read(stream, buf, ec);

            THEN("2 bytes are read") {
                REQUIRE(n == 2);
            }
            THEN("ec is set to eof") {
                REQUIRE(ec == make_error_code(stream_errc::eof));
            }
        }
    }
}

SCENARIO("read() with dynamic buffer accumulates data", "[buffer][read][dynamic]") {

    GIVEN("a fake stream with 16 bytes and a dynamic_vector_buffer") {
        fake_sync_stream stream;
        stream.data.resize(16);
        std::iota(stream.data.begin(), stream.data.end(), char(0));

        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 256);

        WHEN("read(stream, dynamic_buffer) is called") {
            std::error_code ec;
            auto n = read(stream, std::move(db), ec);

            THEN("16 bytes are read") {
                REQUIRE(n == 16);
            }
        }
    }
}

SCENARIO("write() writes all bytes from a fixed buffer to a sync stream", "[buffer][write]") {

    GIVEN("a source const_buffer with known content") {
        fake_sync_stream stream;
        const char src[] = "testdata";
        const_buffer cb{src, 8};

        WHEN("write(stream, cb) is called") {
            auto n = write(stream, cb);

            THEN("8 bytes are written") {
                REQUIRE(n == 8);
            }
            THEN("stream data matches source") {
                REQUIRE(std::memcmp(stream.data.data(), src, 8) == 0);
            }
        }
    }

   GIVEN("a dynamic_string_buffer with content to write") {
        fake_sync_stream stream;
        foundation::text::basic_string<char> str{"hello world"};
        std::size_t original_size = str.size();
        auto db = dynamic_buffer(str);
        WHEN("write(stream, db) is called") {
            std::error_code ec;
            auto n = write(stream, std::move(db), ec);

            THEN("all bytes are written") {
                REQUIRE(n == original_size);
            }
            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
            THEN("stream received the correct content") {
                REQUIRE(stream.data.size() == original_size);
            }
        }
    }
}

SCENARIO("read_until stops at a single-character delimiter", "[buffer][read_until]") {

    GIVEN("a stream containing 'hello\\nworld'") {
        fake_sync_stream stream;
        const char src[] = "hello\nworld";
        stream.data.assign(src, src + sizeof(src) - 1);

        foundation::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 256);

        WHEN("read_until with delimiter '\\n' is called") {
            std::error_code ec;
            auto n = read_until(stream, std::move(db), '\n', ec);

            THEN("returns position just past the delimiter") {
                REQUIRE(n == 6); // "hello\n"
            }
            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
        }
    }

    GIVEN("a stream without the delimiter") {
        fake_sync_stream stream;
        const char src[] = "nodelin";
        stream.data.assign(src, src + sizeof(src) - 1);

        foundation::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 16);

        WHEN("read_until with '\\n' is called on a buffer that fills up") {
            std::error_code ec;
            read_until(stream, std::move(db), '\n', ec);

            THEN("ec is set (eof or not_found)") {
                REQUIRE(ec);
            }
        }
    }
}

SCENARIO("read_until stops at a string_view delimiter", "[buffer][read_until]") {

    GIVEN("a stream containing 'GET / HTTP/1.1\\r\\n'") {
        fake_sync_stream stream;
        const char src[] = "GET / HTTP/1.1\r\n";
        stream.data.assign(src, src + sizeof(src) - 1);

        foundation::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 256);

        WHEN("read_until with delimiter '\\r\\n' is called") {
            std::error_code ec;
            foundation::text::string_view delim{"\r\n"};
            auto n = read_until(stream, std::move(db), delim, ec);

            THEN("returns position just past the delimiter") {
                REQUIRE(n == sizeof(src) - 1);
            }
            THEN("no error") {
                REQUIRE_FALSE(ec);
            }
        }
    }
}

SCENARIO("dynamic_buffer factory returns correctly initialised wrappers", "[buffer][dynamic_buffer]") {

    GIVEN("a vector<unsigned char> with 3 elements") {
        collections::vector<unsigned char> vec(3, 0xAB);
        auto db = dynamic_buffer(vec);

        THEN("size() equals 3") {
            REQUIRE(db.size() == 3);
        }
    }

    GIVEN("a vector and explicit max_size") {
        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 512);

        THEN("max_size() equals 512") {
            REQUIRE(db.max_size() == 512);
        }
    }

    GIVEN("a basic_string") {
        foundation::text::basic_string<char> str{"ab"};
        auto db = dynamic_buffer(str);

        THEN("size() equals 2") {
            REQUIRE(db.size() == 2);
        }
    }

    GIVEN("a basic_string and explicit max_size") {
        foundation::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 100);

        THEN("max_size() equals 100") {
            REQUIRE(db.max_size() == 100);
        }
    }
}
