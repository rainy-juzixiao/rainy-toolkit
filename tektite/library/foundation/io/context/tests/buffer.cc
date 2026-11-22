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
#include <rainy/foundation/io/buffer.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <numeric>

using namespace rainy;
using namespace rainy::foundation::io;

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

TEST_CASE("stream_errc values produce correct error codes", "[buffer][stream_errc]") {
    SECTION("stream_errc::eof and stream_errc::not_found") {
        SECTION("make_error_code is called for eof") {
            auto ec = make_error_code(stream_errc::eof);
            SECTION("error code is non-zero") {
                REQUIRE(ec.value() != 0);
            }
            SECTION("category is stream_category") {
                REQUIRE(ec.category() == stream_category());
            }
            SECTION("message is not empty") {
                REQUIRE_FALSE(ec.message().empty());
            }
        }
        SECTION("make_error_code is called for not_found") {
            auto ec = make_error_code(stream_errc::not_found);

            SECTION("error code is non-zero and differs from eof") {
                REQUIRE(ec.value() != 0);
                REQUIRE(ec.value() != make_error_code(stream_errc::eof).value());
            }
        }

        SECTION("make_error_condition is called") {
            auto cond = make_error_condition(stream_errc::eof);

            SECTION("condition value matches error code value") {
                REQUIRE(cond.value() == make_error_code(stream_errc::eof).value());
            }
        }

        SECTION("std::is_error_code_enum specialisation is checked") {
            SECTION("it is true") {
                REQUIRE(std::is_error_code_enum<stream_errc>::value);
            }
        }
    }
}

TEST_CASE("mutable_buffer default construction yields empty buffer", "[buffer][mutable_buffer]") {
    SECTION("a default-constructed mutable_buffer") {
        mutable_buffer b;
        SECTION("data() is nullptr") {
            REQUIRE(b.data() == nullptr);
        }
        SECTION("size() is 0") {
            REQUIRE(b.size() == 0);
        }
    }
}

TEST_CASE("mutable_buffer constructed from pointer and size", "[buffer][mutable_buffer]") {
    SECTION("a char array and a mutable_buffer pointing to it") {
        char arr[16]{};
        mutable_buffer b{arr, sizeof(arr)};

        SECTION("data() points to the array") {
            REQUIRE(b.data() == arr);
        }
        SECTION("size() equals array size") {
            REQUIRE(b.size() == 16);
        }

        SECTION("operator+= advances by 4") {
            b += 4;
            SECTION("data() advances by 4 bytes") {
                REQUIRE(b.data() == arr + 4);
            }
            SECTION("size() decreases by 4") {
                REQUIRE(b.size() == 12);
            }
        }

        SECTION("operator+= advances by more than size") {
            b += 100;
            SECTION("size() clamps to 0") {
                REQUIRE(b.size() == 0);
            }
        }

        SECTION("operator+ (buffer + n) is used") {
            auto b2 = b + 4;
            SECTION("original buffer is unchanged") {
                REQUIRE(b.size() == 16);
            }
            SECTION("new buffer is advanced") {
                REQUIRE(b2.size() == 12);
            }
        }

        SECTION("operator+ (n + buffer) is used") {
            auto b2 = 4 + b;
            SECTION("result equals buffer + n") {
                REQUIRE(b2.size() == 12);
            }
        }
    }
}

TEST_CASE("const_buffer construction and advancement", "[buffer][const_buffer]") {
    SECTION("a const_buffer default constructed") {
        const_buffer b;
        SECTION("data() is nullptr") {
            REQUIRE(b.data() == nullptr);
        }
        SECTION("size() is 0") {
            REQUIRE(b.size() == 0);
        }
    }

    SECTION("a const_buffer from pointer and size") {
        const char data[] = "hello";
        const_buffer b{data, sizeof(data)};

        SECTION("data() points correctly") {
            REQUIRE(b.data() == data);
        }
        SECTION("size() is correct") {
            REQUIRE(b.size() == sizeof(data));
        }

        SECTION("operator+= advances by 2") {
            b += 2;
            SECTION("data advances") {
                REQUIRE(b.data() == data + 2);
            }
            SECTION("size shrinks") {
                REQUIRE(b.size() == sizeof(data) - 2);
            }
        }
    }

    SECTION("a const_buffer constructed from mutable_buffer") {
        char arr[8]{};
        mutable_buffer mb{arr, sizeof(arr)};
        const_buffer cb{mb};

        SECTION("data and size match the mutable_buffer") {
            REQUIRE(cb.data() == mb.data());
            REQUIRE(cb.size() == mb.size());
        }
    }

    SECTION("const_buffer operator+ variants") {
        const char data[10]{};
        const_buffer b{data, sizeof(data)};

        SECTION("b + 3 is computed") {
            auto b2 = b + 3;
            SECTION("original unchanged") {
                REQUIRE(b.size() == 10);
            }
            SECTION("new buffer advanced") {
                REQUIRE(b2.size() == 7);
            }
        }

        SECTION("3 + b is computed") {
            auto b2 = 3 + b;
            SECTION("same result as b + 3") {
                REQUIRE(b2.size() == 7);
            }
        }
    }
}

TEST_CASE("buffer() creates correct mutable_buffer from raw pointer", "[buffer][factory]") {
    SECTION("a raw memory region") {
        char mem[32]{};

        SECTION("buffer(void*, n) is called") {
            auto b = buffer(static_cast<void *>(mem), 32);
            SECTION("data and size are correct") {
                REQUIRE(b.data() == mem);
                REQUIRE(b.size() == 32);
            }
        }
    }
}

TEST_CASE("buffer() creates correct const_buffer from const pointer", "[buffer][factory]") {
    SECTION("a const raw memory region") {
        const char mem[32]{};

        SECTION("buffer(const void*, n) is called") {
            auto b = buffer(static_cast<const void *>(mem), 32);
            SECTION("data and size are correct") {
                REQUIRE(b.data() == mem);
                REQUIRE(b.size() == 32);
            }
        }
    }
}

TEST_CASE("buffer() with size limit clamps to min(container, n)", "[buffer][factory]") {
    SECTION("a mutable_buffer of size 16") {
        char arr[16]{};
        mutable_buffer mb{arr, 16};

        SECTION("buffer(mb, 8) is called") {
            auto b = buffer(mb, 8);
            SECTION("size is clamped to 8") {
                REQUIRE(b.size() == 8);
            }
        }

        SECTION("buffer(mb, 100) is called") {
            auto b = buffer(mb, 100);
            SECTION("size is clamped to 16") {
                REQUIRE(b.size() == 16);
            }
        }
    }
}

TEST_CASE("buffer() from C array", "[buffer][factory]") {
    SECTION("a char array") {
        char arr[10]{};

        SECTION("buffer(arr) is called") {
            auto b = buffer(arr);
            SECTION("size equals array byte size") {
                REQUIRE(b.size() == 10);
            }
        }

        SECTION("buffer(arr, 5) is called") {
            auto b = buffer(arr, 5);
            SECTION("size is clamped to 5") {
                REQUIRE(b.size() == 5);
            }
        }
    }

    SECTION("a const char array") {
        const char arr[10]{};

        SECTION("buffer(arr) is called") {
            auto b = buffer(arr);
            SECTION("size equals array byte size") {
                REQUIRE(b.size() == 10);
            }
        }
    }
}

TEST_CASE("buffer() from collections::vector", "[buffer][factory]") {
    SECTION("a vector<char> with 8 elements") {
        collections::vector<char> vec(8, 'a');

        SECTION("buffer(vec) is called") {
            auto b = buffer(vec);
            SECTION("data points to vec.data()") {
                REQUIRE(b.data() == vec.data());
            }
            SECTION("size equals 8") {
                REQUIRE(b.size() == 8);
            }
        }
    }

    SECTION("a const vector<int> with 4 elements") {
        const collections::vector<int> vec{1, 2, 3, 4};

        SECTION("buffer(vec) is called") {
            auto b = buffer(vec);
            SECTION("size equals 4 * sizeof(int)") {
                REQUIRE(b.size() == 4 * sizeof(int));
            }
        }
    }
}

TEST_CASE("buffer_size returns total bytes of a buffer sequence", "[buffer][buffer_size]") {
    SECTION("a single const_buffer of 12 bytes") {
        char arr[12]{};
        const_buffer b{arr, 12};

        SECTION("buffer_size returns 12") {
            REQUIRE(buffer_size(b) == 12);
        }
    }

    SECTION("a vector of const_buffers") {
        char a[4]{}, b_arr[8]{}, c[16]{};
        collections::vector<const_buffer> seq{{a, 4}, {b_arr, 8}, {c, 16}};

        SECTION("buffer_size returns 28") {
            REQUIRE(buffer_size(seq) == 28);
        }
    }
}

TEST_CASE("buffer_copy copies bytes between buffer sequences", "[buffer][buffer_copy]") {
    SECTION("a source const_buffer with known content") {
        const char src[] = "abcdefgh";
        const_buffer source{src, 8};

        SECTION("a destination mutable_buffer of the same size") {
            char dst[8]{};
            mutable_buffer dest{dst, 8};

            SECTION("buffer_copy is called") {
                auto n = buffer_copy(dest, source);

                SECTION("all 8 bytes are copied") {
                    REQUIRE(n == 8);
                    REQUIRE(std::memcmp(dst, src, 8) == 0);
                }
            }
        }

        SECTION("a destination buffer smaller than source") {
            char dst[4]{};
            mutable_buffer dest{dst, 4};

            SECTION("buffer_copy is called") {
                auto n = buffer_copy(dest, source);

                SECTION("only 4 bytes are copied") {
                    REQUIRE(n == 4);
                    REQUIRE(std::memcmp(dst, src, 4) == 0);
                }
            }
        }

        SECTION("a max_size limit smaller than both buffers") {
            char dst[8]{};
            mutable_buffer dest{dst, 8};

            SECTION("buffer_copy with max_size=3 is called") {
                auto n = buffer_copy(dest, source, 3);

                SECTION("only 3 bytes are copied") {
                    REQUIRE(n == 3);
                    REQUIRE(std::memcmp(dst, src, 3) == 0);
                }
            }
        }
    }
}

TEST_CASE("buffer_sequence_begin/end for single buffers act as single-element range", "[buffer][sequence]") {

    SECTION("a mutable_buffer") {
        char arr[4]{};
        mutable_buffer mb{arr, 4};

        SECTION("begin+1 equals end") {
            REQUIRE(buffer_sequence_begin(mb) + 1 == buffer_sequence_end(mb));
        }
    }

    SECTION("a const_buffer") {
        const char arr[4]{};
        const_buffer cb{arr, 4};

        SECTION("begin+1 equals end") {
            REQUIRE(buffer_sequence_begin(cb) + 1 == buffer_sequence_end(cb));
        }
    }
}

TEST_CASE("is_dynamic_buffer correctly identifies dynamic buffer types", "[buffer][traits]") {

    SECTION("dynamic_vector_buffer") {
        SECTION("is_dynamic_buffer_v is true") {
            REQUIRE(is_dynamic_buffer_v<dynamic_vector_buffer<char, std::allocator<char>>>);
        }
    }

    SECTION("dynamic_string_buffer") {
        SECTION("is_dynamic_buffer_v is true") {
            using str_t = core::text::basic_string<char>;
            REQUIRE(is_dynamic_buffer_v<dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>>);
        }
    }

    SECTION("plain mutable_buffer") {
        SECTION("is_dynamic_buffer_v is false") {
            REQUIRE_FALSE(is_dynamic_buffer_v<mutable_buffer>);
        }
    }
}

TEST_CASE("dynamic_vector_buffer size / max_size / capacity", "[buffer][dynamic_vector_buffer]") {

    SECTION("a vector<char> with 4 elements and a dynamic_vector_buffer wrapping it") {
        collections::vector<char> vec(4, 'x');
        auto db = dynamic_buffer(vec);

        SECTION("size() equals vector size") {
            REQUIRE(db.size() == 4);
        }
        SECTION("max_size() equals vector max_size()") {
            REQUIRE(db.max_size() == vec.max_size());
        }
    }

    SECTION("a dynamic_vector_buffer with an explicit maximum_size") {
        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 64);

        SECTION("max_size() equals the specified limit") {
            REQUIRE(db.max_size() == 64);
        }
    }
}

TEST_CASE("dynamic_vector_buffer prepare / commit / consume lifecycle", "[buffer][dynamic_vector_buffer]") {

    SECTION("an empty vector and a dynamic_vector_buffer") {
        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 256);

        SECTION("prepare(8) is called") {
            auto mb = db.prepare(8);

            SECTION("returned buffer has 8 bytes") {
                REQUIRE(mb.size() == 8 * sizeof(char));
            }
            SECTION("size() is still 0 before commit") {
                REQUIRE(db.size() == 0);
            }

            SECTION("the buffer is written and commit(8) is called") {
                std::memset(mb.data(), 'Z', 8);
                db.commit(8);

                SECTION("size() is now 8") {
                    REQUIRE(db.size() == 8);
                }
                SECTION("data() reflects the written content") {
                    auto cb = db.data();
                    const char *p = static_cast<const char *>(cb.data());
                    REQUIRE(std::all_of(p, p + 8, [](char c) { return c == 'Z'; }));
                }

                SECTION("consume(4) is called") {
                    db.consume(4);

                    SECTION("size() decreases by 4") {
                        REQUIRE(db.size() == 4);
                    }
                }
            }
        }

        SECTION("prepare exceeds max_size") {
            SECTION("prepare throws std::length_error") {
                REQUIRE_THROWS_AS(db.prepare(257), std::length_error);
            }
        }
    }
}

TEST_CASE("dynamic_string_buffer prepare / commit / consume lifecycle", "[buffer][dynamic_string_buffer]") {

    SECTION("an empty basic_string and a dynamic_string_buffer") {
        core::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 128);

        SECTION("prepare(5) is called and data is written") {
            auto mb = db.prepare(5);
            std::memcpy(mb.data(), "hello", 5);
            db.commit(5);

            SECTION("size() is 5") {
                REQUIRE(db.size() == 5);
            }
            SECTION("underlying string contains the written data") {
                REQUIRE(str.substr(0, 5) == "hello");
            }

            SECTION("consume(5) is called") {
                db.consume(5);
                SECTION("size() is 0") {
                    REQUIRE(db.size() == 0);
                }
            }
        }
    }
}

TEST_CASE("transfer_all allows unlimited transfer until error", "[buffer][transfer]") {
    SECTION("transfer_all{}") {
        transfer_all cond;

        SECTION("called with no error and any byte count") {
            SECTION("returns non-zero (continue)") {
                REQUIRE(cond(std::error_code{}, 0) > 0);
                REQUIRE(cond(std::error_code{}, 100) > 0);
            }
        }

        SECTION("called with an error") {
            SECTION("returns 0 (stop)") {
                REQUIRE(cond(make_error_code(stream_errc::eof), 4) == 0);
            }
        }
    }
}

TEST_CASE("transfer_at_least stops only once minimum bytes are transferred", "[buffer][transfer]") {
    SECTION("transfer_at_least{10}") {
        transfer_at_least cond{10};

        SECTION("fewer than 10 bytes transferred and no error") {
            SECTION("returns non-zero (continue)") {
                REQUIRE(cond(std::error_code{}, 9) > 0);
            }
        }

        SECTION("exactly 10 bytes transferred") {
            SECTION("returns 0 (stop)") {
                REQUIRE(cond(std::error_code{}, 10) == 0);
            }
        }

        SECTION("more than 10 bytes transferred") {
            SECTION("returns 0 (stop)") {
                REQUIRE(cond(std::error_code{}, 20) == 0);
            }
        }

        SECTION("called with an error") {
            SECTION("returns 0 regardless of byte count") {
                REQUIRE(cond(make_error_code(stream_errc::eof), 0) == 0);
            }
        }
    }
}

TEST_CASE("transfer_exactly stops when the exact byte count is reached", "[buffer][transfer]") {
    SECTION("transfer_exactly{8}") {
        transfer_exactly cond{8};

        SECTION("fewer than 8 bytes transferred") {
            SECTION("returns non-zero (continue) reflecting remaining bytes") {
                REQUIRE(cond(std::error_code{}, 3) > 0);
            }
        }

        SECTION("exactly 8 bytes transferred") {
            SECTION("returns 0 (stop)") {
                REQUIRE(cond(std::error_code{}, 8) == 0);
            }
        }

        SECTION("called with an error") {
            SECTION("returns 0") {
                REQUIRE(cond(make_error_code(stream_errc::eof), 0) == 0);
            }
        }
    }
}

TEST_CASE("read() reads all bytes from a sync stream into a fixed buffer", "[buffer][read]") {

    SECTION("a fake stream with 8 bytes of data") {
        fake_sync_stream stream;
        stream.data = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

        char out[8]{};
        mutable_buffer buf{out, 8};

        SECTION("read(stream, buf) is called") {
            auto n = read(stream, buf);

            SECTION("8 bytes are read") {
                REQUIRE(n == 8);
            }
            SECTION("content matches source") {
                REQUIRE(std::memcmp(out, stream.data.data(), 8) == 0);
            }
        }
    }

    SECTION("a fake stream with 8 bytes, max 2 per read_some call") {
        fake_sync_stream stream;
        stream.data = {'1', '2', '3', '4', '5', '6', '7', '8'};
        stream.max_read_per_call = 2;

        char out[8]{};
        mutable_buffer buf{out, 8};

        SECTION("read(stream, buf) is called") {
            auto n = read(stream, buf);

            SECTION("all 8 bytes are eventually read across multiple calls") {
                REQUIRE(n == 8);
            }
        }
    }

    SECTION("a fake stream and read with transfer_at_least(4)") {
        fake_sync_stream stream;
        stream.data.resize(10, 'x');

        char out[10]{};
        mutable_buffer buf{out, 10};

        SECTION("read(stream, buf, transfer_at_least{4}) is called") {
            auto n = read(stream, buf, transfer_at_least{4});

            SECTION("at least 4 bytes are read") {
                REQUIRE(n >= 4);
            }
        }
    }

    SECTION("a fake stream and read with error_code overload") {
        fake_sync_stream stream;
        stream.data = {'X', 'Y'};

        char out[8]{};
        mutable_buffer buf{out, 8};
        std::error_code ec;

        SECTION("read(stream, buf, ec) is called") {
            auto n = read(stream, buf, ec);

            SECTION("2 bytes are read") {
                REQUIRE(n == 2);
            }
            SECTION("ec is set to eof") {
                REQUIRE(ec == make_error_code(stream_errc::eof));
            }
        }
    }
}

TEST_CASE("read() with dynamic buffer accumulates data", "[buffer][read][dynamic]") {

    SECTION("a fake stream with 16 bytes and a dynamic_vector_buffer") {
        fake_sync_stream stream;
        stream.data.resize(16);
        std::iota(stream.data.begin(), stream.data.end(), char(0));

        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 256);

        SECTION("read(stream, dynamic_buffer) is called") {
            std::error_code ec;
            auto n = read(stream, std::move(db), ec);

            SECTION("16 bytes are read") {
                REQUIRE(n == 16);
            }
        }
    }
}

TEST_CASE("write() writes all bytes from a fixed buffer to a sync stream", "[buffer][write]") {

    SECTION("a source const_buffer with known content") {
        fake_sync_stream stream;
        const char src[] = "testdata";
        const_buffer cb{src, 8};

        SECTION("write(stream, cb) is called") {
            auto n = write(stream, cb);

            SECTION("8 bytes are written") {
                REQUIRE(n == 8);
            }
            SECTION("stream data matches source") {
                REQUIRE(std::memcmp(stream.data.data(), src, 8) == 0);
            }
        }
    }

   SECTION("a dynamic_string_buffer with content to write") {
        fake_sync_stream stream;
        core::text::basic_string<char> str{"hello world"};
        std::size_t original_size = str.size();
        auto db = dynamic_buffer(str);
        SECTION("write(stream, db) is called") {
            std::error_code ec;
            auto n = write(stream, std::move(db), ec);

            SECTION("all bytes are written") {
                REQUIRE(n == original_size);
            }
            SECTION("no error") {
                REQUIRE_FALSE(ec);
            }
            SECTION("stream received the correct content") {
                REQUIRE(stream.data.size() == original_size);
            }
        }
    }
}

TEST_CASE("read_until stops at a single-character delimiter", "[buffer][read_until]") {

    SECTION("a stream containing 'hello\\nworld'") {
        fake_sync_stream stream;
        const char src[] = "hello\nworld";
        stream.data.assign(src, src + sizeof(src) - 1);

        core::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 256);

        SECTION("read_until with delimiter '\\n' is called") {
            std::error_code ec;
            auto n = read_until(stream, std::move(db), '\n', ec);

            SECTION("returns position just past the delimiter") {
                REQUIRE(n == 6); // "hello\n"
            }
            SECTION("no error") {
                REQUIRE_FALSE(ec);
            }
        }
    }

    SECTION("a stream without the delimiter") {
        fake_sync_stream stream;
        const char src[] = "nodelin";
        stream.data.assign(src, src + sizeof(src) - 1);

        core::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 16);

        SECTION("read_until with '\\n' is called on a buffer that fills up") {
            std::error_code ec;
            read_until(stream, std::move(db), '\n', ec);

            SECTION("ec is set (eof or not_found)") {
                REQUIRE(ec);
            }
        }
    }
}

TEST_CASE("read_until stops at a string_view delimiter", "[buffer][read_until]") {

    SECTION("a stream containing 'GET / HTTP/1.1\\r\\n'") {
        fake_sync_stream stream;
        const char src[] = "GET / HTTP/1.1\r\n";
        stream.data.assign(src, src + sizeof(src) - 1);

        core::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 256);

        SECTION("read_until with delimiter '\\r\\n' is called") {
            std::error_code ec;
            core::text::string_view delim{"\r\n"};
            auto n = read_until(stream, std::move(db), delim, ec);

            SECTION("returns position just past the delimiter") {
                REQUIRE(n == sizeof(src) - 1);
            }
            SECTION("no error") {
                REQUIRE_FALSE(ec);
            }
        }
    }
}

TEST_CASE("dynamic_buffer factory returns correctly initialised wrappers", "[buffer][dynamic_buffer]") {

    SECTION("a vector<unsigned char> with 3 elements") {
        collections::vector<unsigned char> vec(3, 0xAB);
        auto db = dynamic_buffer(vec);

        SECTION("size() equals 3") {
            REQUIRE(db.size() == 3);
        }
    }

    SECTION("a vector and explicit max_size") {
        collections::vector<char> vec;
        auto db = dynamic_buffer(vec, 512);

        SECTION("max_size() equals 512") {
            REQUIRE(db.max_size() == 512);
        }
    }

    SECTION("a basic_string") {
        core::text::basic_string<char> str{"ab"};
        auto db = dynamic_buffer(str);

        SECTION("size() equals 2") {
            REQUIRE(db.size() == 2);
        }
    }

    SECTION("a basic_string and explicit max_size") {
        core::text::basic_string<char> str;
        auto db = dynamic_buffer(str, 100);

        SECTION("max_size() equals 100") {
            REQUIRE(db.max_size() == 100);
        }
    }
}
