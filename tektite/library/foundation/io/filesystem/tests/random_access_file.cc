#include <catch2/catch_all.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <rainy/foundation/io/filesystem/path.hpp>
#include <rainy/foundation/io/filesystem/operations.hpp>
#include <rainy/foundation/io/filesystem/random_access_file.hpp>

using namespace rainy;
using namespace rainy::foundation;
using namespace rainy::foundation::io;
using namespace rainy::foundation::io::filesystem;

namespace fs = rainy::foundation::io::filesystem;

class RandomAccessFileFixture {
protected:
    fs::path test_file_path;
    fs::path temp_dir;

    void setup() {
        temp_dir = fs::temp_directory_path() / "random_access_file_test";
        fs::create_directories(temp_dir);
        test_file_path = temp_dir / "test_file.dat";
        std::error_code ec;
        fs::remove(test_file_path, ec);
    }

    void teardown() {
        std::error_code ec;
        fs::remove_all(temp_dir, ec);
    }

    void create_test_file(const std::string &content) {
        std::ofstream file(test_file_path.c_str(), std::ios::binary);
        file.write(content.data(), content.size());
    }

    std::string read_file_content() {
        std::ifstream file(test_file_path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>());
    }

    io_context ctx;
};

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file construction and opening",
                "[random_access_file][construction]") {

    SECTION("a default-constructed random_access_file") {
        random_access_file file(ctx);

        SECTION("the file is not open") {
            REQUIRE_FALSE(file.is_open());
        }

        SECTION("opening an existing file in read_only mode") {
            setup();
            create_test_file("Hello World");

            auto ec = file.open(test_file_path, open_mode::read_only);

            SECTION("the open succeeds") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.is_open());
            }

            teardown();
        }

        SECTION("opening a non-existent file in read_only mode") {
            setup();
            auto ec = file.open(temp_dir / "nonexistent.dat", open_mode::read_only);

            SECTION("the open fails and file remains closed") {
                REQUIRE(ec);
                REQUIRE_FALSE(file.is_open());
            }

            teardown();
        }
    }

    SECTION("a random_access_file constructed with path and mode") {
        setup();
        create_test_file("Hello World");

        SECTION("constructed with read_only mode") {
            random_access_file file(ctx, test_file_path, open_mode::read_only);

            SECTION("the file is open immediately") {
                REQUIRE(file.is_open());
            }
        }

        SECTION("constructed with write_only | create mode") {
            random_access_file file(ctx, test_file_path,
                                    open_mode::write_only | open_mode::create);

            SECTION("the file is open immediately") {
                REQUIRE(file.is_open());
            }
        }

        SECTION("simulating append by reading size first") {
            random_access_file file(ctx, test_file_path,
                                    open_mode::read_write);
            auto end_offset = file.size();

            SECTION("end_offset equals the file size") {
                REQUIRE(end_offset == std::string("Hello World").size());
            }
        }

        teardown();
    }
}

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file synchronous read operations",
                "[random_access_file][read]") {

    SECTION("a file with known content") {
        setup();
        const std::string content = "0123456789ABCDEFGHIJ";
        create_test_file(content);
        random_access_file file(ctx, test_file_path, open_mode::read_only);

        SECTION("reading from offset 0") {
            std::vector<char> buf(5);
            std::error_code ec;
            auto n = file.read_some_at(0, mutable_buffer(buf.data(), buf.size()), ec);

            SECTION("the first 5 bytes are returned") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == 5);
                REQUIRE(std::string(buf.data(), n) == content.substr(0, 5));
            }
        }

        SECTION("reading from an arbitrary middle offset") {
            std::vector<char> buf(4);
            std::error_code ec;
            auto n = file.read_some_at(5, mutable_buffer(buf.data(), buf.size()), ec);

            SECTION("it reads from the given offset") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == 4);
                REQUIRE(std::string(buf.data(), n) == content.substr(5, 4));
            }
        }

        SECTION("reading from two different offsets independently") {
            // 核心用例：体现无状态特性——两次读取互不影响
            std::vector<char> buf_a(3), buf_b(3);
            std::error_code ec;

            auto n_a = file.read_some_at(0,  mutable_buffer(buf_a.data(), buf_a.size()), ec);
            REQUIRE_FALSE(ec);
            auto n_b = file.read_some_at(10, mutable_buffer(buf_b.data(), buf_b.size()), ec);
            REQUIRE_FALSE(ec);

            SECTION("each read returns data from its own offset") {
                REQUIRE(std::string(buf_a.data(), n_a) == content.substr(0,  3));
                REQUIRE(std::string(buf_b.data(), n_b) == content.substr(10, 3));
            }

            SECTION("re-reading offset 0 still returns the original data") {
                std::vector<char> buf_c(3);
                auto n_c = file.read_some_at(0, mutable_buffer(buf_c.data(), buf_c.size()), ec);
                REQUIRE(std::string(buf_c.data(), n_c) == content.substr(0, 3));
            }
        }

        SECTION("reading past the end of file") {
            std::vector<char> buf(content.size() + 50);
            std::error_code ec;
            auto n = file.read_some_at(0, mutable_buffer(buf.data(), buf.size()), ec);

            SECTION("only available bytes are returned, no error") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == content.size());
                REQUIRE(std::string(buf.data(), n) == content);
            }
        }

        SECTION("reading from the exact last byte") {
            std::vector<char> buf(1);
            std::error_code ec;
            auto n = file.read_some_at(content.size() - 1,
                                        mutable_buffer(buf.data(), buf.size()), ec);

            SECTION("a single byte is returned") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == 1);
                REQUIRE(buf[0] == content.back());
            }
        }

        SECTION("reading from an offset equal to file size") {
            std::vector<char> buf(10);
            std::error_code ec;
            auto n = file.read_some_at(content.size(),
                                        mutable_buffer(buf.data(), buf.size()), ec);

            SECTION("zero bytes are returned without error") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == 0);
            }
        }

        teardown();
    }
}

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file synchronous write operations",
                "[random_access_file][write]") {

    SECTION("a new file opened for writing") {
        setup();
        random_access_file file(ctx, test_file_path,
                                open_mode::write_only | open_mode::create);

        SECTION("writing at offset 0") {
            std::string data = "Hello";
            std::error_code ec;
            auto n = file.write_some_at(0, const_buffer(data.data(), data.size()), ec);

            SECTION("all bytes are written") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == data.size());
            }
        }

        SECTION("writing to non-overlapping regions") {
            // 体现随机访问：先写尾部再写头部，最终文件内容正确
            std::string head = "HEAD";
            std::string tail = "TAIL";
            std::error_code ec;

            // 先扩展文件到足够大小
            file.resize(8);

            auto n1 = file.write_some_at(0, const_buffer(head.data(), head.size()), ec);
            REQUIRE_FALSE(ec);
            auto n2 = file.write_some_at(4, const_buffer(tail.data(), tail.size()), ec);
            REQUIRE_FALSE(ec);

            SECTION("each region contains the correct data") {
                REQUIRE(n1 == 4);
                REQUIRE(n2 == 4);

                file.close();
                auto content = read_file_content();
                REQUIRE(content == "HEADTAIL");
            }
        }

        SECTION("overwriting a middle region") {
            std::string initial  = "AAABBBCCC";
            std::string patch    = "XXX";
            std::error_code ec;

            file.resize(initial.size());
            file.write_some_at(0, const_buffer(initial.data(), initial.size()), ec);
            REQUIRE_FALSE(ec);

            file.write_some_at(3, const_buffer(patch.data(), patch.size()), ec);
            REQUIRE_FALSE(ec);

            SECTION("only the targeted region is modified") {
                file.close();
                auto content = read_file_content();
                REQUIRE(content == "AAAXXXCCC");
            }
        }

        SECTION("writing the same offset twice") {
            std::string first  = "First";
            std::string second = "XXXXX";
            std::error_code ec;

            file.resize(5);
            file.write_some_at(0, const_buffer(first.data(),  first.size()),  ec);
            REQUIRE_FALSE(ec);
            file.write_some_at(0, const_buffer(second.data(), second.size()), ec);
            REQUIRE_FALSE(ec);

            SECTION("the second write fully overwrites the first") {
                file.close();
                REQUIRE(read_file_content() == second);
            }
        }

        teardown();
    }
    SECTION("a file with existing content") {
        setup();
        std::string initial = "Initial\n";
        create_test_file(initial);

        random_access_file file(ctx, test_file_path, open_mode::read_write);

        SECTION("appending via explicit size() offset") {
            std::string appended = "Appended\n";
            auto end = file.size();
            std::error_code ec;
            auto n = file.write_some_at(end,
                                         const_buffer(appended.data(), appended.size()),
                                         ec);

            SECTION("data is correctly appended at end") {
                REQUIRE_FALSE(ec);
                REQUIRE(n == appended.size());

                file.close();
                REQUIRE(read_file_content() == initial + appended);
            }
        }

        teardown();
    }
}

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file async read operations",
                "[random_access_file][async][read]") {

    SECTION("a file with known content") {
        setup();
        const std::string content = "Async random access content";
        create_test_file(content);
        random_access_file file(ctx, test_file_path, open_mode::read_only);

        SECTION("performing a single async read at offset 0") {
            bool done = false;
            std::error_code async_ec;
            std::size_t transferred = 0;
            rainy::collections::vector<char> buf(10);

            file.async_read_some_at(0, io::buffer(buf),
                [&](std::error_code ec, std::size_t n) {
                    async_ec    = ec;
                    transferred = n;
                    done        = true;
                });

            ctx.run();

            SECTION("the callback receives the correct data") {
                REQUIRE(done);
                REQUIRE_FALSE(async_ec);
                REQUIRE(transferred == 10);
                REQUIRE(std::string(buf.data(), transferred) == content.substr(0, 10));
            }
        }

        SECTION("performing two async reads at different offsets") {
            struct Result { bool done{}; std::error_code ec; std::size_t n{}; };
            Result r1, r2;
            rainy::collections::vector<char> buf1(5), buf2(5);

            file.async_read_some_at(0, io::buffer(buf1),
                [&](std::error_code ec, std::size_t n) {
                    r1 = {true, ec, n};
                });

            file.async_read_some_at(10, io::buffer(buf2),
                [&](std::error_code ec, std::size_t n) {
                    r2 = {true, ec, n};
                });

            ctx.run();

            SECTION("each callback receives data from its own offset") {
                REQUIRE(r1.done);
                REQUIRE_FALSE(r1.ec);
                REQUIRE(std::string(buf1.data(), r1.n) == content.substr(0,  5));

                REQUIRE(r2.done);
                REQUIRE_FALSE(r2.ec);
                REQUIRE(std::string(buf2.data(), r2.n) == content.substr(10, 5));
            }
        }

        SECTION("performing async read past end of file") {
            bool done = false;
            std::error_code async_ec;
            std::size_t transferred = 0;
            rainy::collections::vector<char> buf(content.size() + 100);

            file.async_read_some_at(0, io::buffer(buf),
                [&](std::error_code ec, std::size_t n) {
                    async_ec    = ec;
                    transferred = n;
                    done        = true;
                });

            ctx.run();

            SECTION("only available bytes are returned without error") {
                REQUIRE(done);
                REQUIRE_FALSE(async_ec);
                REQUIRE(transferred == content.size());
            }
        }

        teardown();
    }
}

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file async write operations",
                "[random_access_file][async][write]") {

    SECTION("a new file opened for writing") {
        setup();
        random_access_file file(ctx, test_file_path,
                                open_mode::write_only | open_mode::create);

        SECTION("performing a single async write at offset 0") {
            bool done = false;
            std::error_code async_ec;
            std::size_t transferred = 0;
            core::text::string data = "Async write data";

            file.async_write_some_at(0, io::buffer(data),
                [&](std::error_code ec, std::size_t n) {
                    async_ec    = ec;
                    transferred = n;
                    done        = true;
                });

            ctx.run();

            SECTION("the callback confirms full write") {
                REQUIRE(done);
                REQUIRE_FALSE(async_ec);
                REQUIRE(transferred == data.size());

                file.close();
                REQUIRE(read_file_content() == data);
            }
        }

        SECTION("performing async writes to non-overlapping regions") {
            file.resize(8);

            struct Result { bool done{}; std::error_code ec; std::size_t n{}; };
            Result r1, r2;
            core::text::string head = "ABCD";
            core::text::string tail = "EFGH";

            file.async_write_some_at(0, io::buffer(head),
                [&](std::error_code ec, std::size_t n) { r1 = {true, ec, n}; });

            file.async_write_some_at(4, io::buffer(tail),
                [&](std::error_code ec, std::size_t n) { r2 = {true, ec, n}; });

            ctx.run();

            SECTION("both regions are written and file content is correct") {
                REQUIRE(r1.done); REQUIRE_FALSE(r1.ec); REQUIRE(r1.n == 4);
                REQUIRE(r2.done); REQUIRE_FALSE(r2.ec); REQUIRE(r2.n == 4);

                file.close();
                REQUIRE(read_file_content() == "ABCDEFGH");
            }
        }

        teardown();
    }
}

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file move semantics",
                "[random_access_file][move]") {

    SECTION("an open random_access_file") {
        setup();
        create_test_file("Move test content");
        random_access_file file1(ctx, test_file_path, open_mode::read_only);

        SECTION("move-constructing into a new object") {
            random_access_file file2(std::move(file1));

            SECTION("the moved-from file is closed") {
                REQUIRE_FALSE(file1.is_open());
            }

            SECTION("the moved-to file is open and functional") {
                REQUIRE(file2.is_open());

                std::vector<char> buf(4);
                std::error_code ec;
                auto n = file2.read_some_at(0, mutable_buffer(buf.data(), buf.size()), ec);
                REQUIRE_FALSE(ec);
                REQUIRE(n == 4);
                REQUIRE(std::string(buf.data(), n) == "Move");
            }
        }

        SECTION("move-assigning to another random_access_file") {
            random_access_file file2(ctx);
            file2 = std::move(file1);

            SECTION("the moved-from file is closed") {
                REQUIRE_FALSE(file1.is_open());
            }

            SECTION("the move-assigned file is open and functional") {
                REQUIRE(file2.is_open());

                std::vector<char> buf(4);
                std::error_code ec;
                auto n = file2.read_some_at(0, mutable_buffer(buf.data(), buf.size()), ec);
                REQUIRE_FALSE(ec);
                REQUIRE(n == 4);
            }
        }

        teardown();
    }
}

TEST_CASE_METHOD(RandomAccessFileFixture,
                "random_access_file size and resize operations",
                "[random_access_file][size]") {

    SECTION("an empty file") {
        setup();
        create_test_file("");
        random_access_file file(ctx, test_file_path, open_mode::read_write);

        SECTION("initial size is zero") {
            REQUIRE(file.size() == 0);
        }

        SECTION("resizing to 1024 bytes") {
            auto ec = file.resize(1024);

            SECTION("resize succeeds and size matches") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 1024);
            }
        }

        teardown();
    }

    SECTION("a file with content") {
        setup();
        std::string content = "Original content";
        create_test_file(content);
        random_access_file file(ctx, test_file_path, open_mode::read_write);

        SECTION("truncating to 5 bytes") {
            auto ec = file.resize(5);

            SECTION("the file is truncated correctly") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 5);

                file.close();
                REQUIRE(read_file_content() == content.substr(0, 5));
            }
        }

        SECTION("extending beyond current size") {
            auto ec = file.resize(100);

            SECTION("the file is extended") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 100);
            }

            SECTION("original content at offset 0 is preserved") {
                std::vector<char> buf(content.size());
                std::error_code ec2;
                auto n = file.read_some_at(0, mutable_buffer(buf.data(), buf.size()), ec2);
                REQUIRE_FALSE(ec2);
                REQUIRE(n == content.size());
                REQUIRE(std::string(buf.data(), n) == content);
            }
        }

        SECTION("resizing to zero") {
            auto ec = file.resize(0);

            SECTION("the file becomes empty") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 0);
            }
        }

        teardown();
    }
}
