#include <catch2/catch_all.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <rainy/collections/vector.hpp>
#include <rainy/foundation/io/filesystem/streamfile.hpp>

namespace fs = std::filesystem;

using namespace rainy;
using namespace rainy::foundation;
using namespace rainy::foundation::io;
using namespace rainy::foundation::io::filesystem;
using namespace rainy::foundation::io::net;

// Test fixture for file operations
class StreamFileFixture {
protected:
    fs::path test_file_path;
    fs::path temp_dir;

    void setup() {
        temp_dir = fs::temp_directory_path() / "stream_file_test";
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
        std::ofstream file(test_file_path, std::ios::binary);
        file.write(content.data(), content.size());
        file.close();
    }

    std::string read_file_content() {
        std::ifstream file(test_file_path, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }

    std::vector<char> create_test_buffer(std::size_t size, char fill = 'A') {
        return std::vector<char>(size, fill);
    }

    io_context ctx;
};

SCENARIO_METHOD(StreamFileFixture, "stream_file construction and opening", "[stream_file][construction]") {
    GIVEN("a new stream_file without an open file") {
        stream_file file(ctx);

        THEN("the file is not open") {
            REQUIRE_FALSE(file.is_open());
        }

        AND_THEN("the offset is zero") {
            REQUIRE(file.tell() == 0);
        }

        WHEN("opening an existing file in read_only mode") {
            setup();
            create_test_file("Hello World");

            auto ec = file.open(test_file_path, open_mode::read_only);

            THEN("the open operation succeeds") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.is_open());
            }

            AND_THEN("the offset is zero for read_only mode") {
                REQUIRE(file.tell() == 0);
            }

            teardown();
        }

        WHEN("opening a non-existent file in read_only mode") {
            setup();
            auto ec = file.open(temp_dir / "nonexistent.dat", open_mode::read_only);

            THEN("the open operation fails") {
                REQUIRE(ec);
                REQUIRE_FALSE(file.is_open());
            }

            teardown();
        }
    }

    GIVEN("a stream_file constructed with a file path") {
        setup();
        create_test_file("Initial Content");

        WHEN("opening in append mode") {
            stream_file file(ctx, test_file_path, open_mode::append | open_mode::write_only);

            THEN("the file is open and offset is at end") {
                REQUIRE(file.is_open());
                REQUIRE(file.tell() == file.size());
            }
        }

        teardown();
    }
}

SCENARIO_METHOD(StreamFileFixture, "stream_file read operations", "[stream_file][read]") {
    GIVEN("a file with known content") {
        setup();
        std::string test_content = "The quick brown fox jumps over the lazy dog";
        create_test_file(test_content);
        stream_file file(ctx, test_file_path, open_mode::read_only);

        WHEN("reading from the beginning") {
            std::vector<char> buffer(10);
            mutable_buffer buf(buffer.data(), buffer.size());
            std::error_code ec;

            auto bytes_read = file.read_some(buf, ec);

            THEN("it reads the correct amount of data") {
                REQUIRE_FALSE(ec);
                REQUIRE(bytes_read == 10);
                REQUIRE(std::string(buffer.data(), bytes_read) == test_content.substr(0, 10));
            }

            AND_THEN("the offset advances correctly") {
                REQUIRE(file.tell() == 10);
            }
        }

        WHEN("reading multiple times sequentially") {
            std::vector<char> buffer1(5);
            std::vector<char> buffer2(5);
            mutable_buffer buf1(buffer1.data(), buffer1.size());
            mutable_buffer buf2(buffer2.data(), buffer2.size());
            std::error_code ec;

            auto bytes_read1 = file.read_some(buf1, ec);
            auto bytes_read2 = file.read_some(buf2, ec);

            THEN("the reads concatenate correctly") {
                REQUIRE(bytes_read1 == 5);
                REQUIRE(bytes_read2 == 5);
                REQUIRE(std::string(buffer1.data(), 5) == test_content.substr(0, 5));
                REQUIRE(std::string(buffer2.data(), 5) == test_content.substr(5, 5));
                REQUIRE(file.tell() == 10);
            }
        }

        WHEN("reading past the end of file") {
            std::vector<char> buffer(test_content.size() + 100);
            mutable_buffer buf(buffer.data(), buffer.size());
            std::error_code ec;

            auto bytes_read = file.read_some(buf, ec);

            THEN("it reads only the available data") {
                REQUIRE_FALSE(ec);
                REQUIRE(bytes_read == test_content.size());
                REQUIRE(std::string(buffer.data(), bytes_read) == test_content);
            }

            AND_THEN("the offset is at the end of file") {
                REQUIRE(file.tell() == test_content.size());
            }

            AND_THEN("subsequent reads return zero bytes") {
                std::vector<char> empty_buffer(10);
                mutable_buffer empty_buf(empty_buffer.data(), empty_buffer.size());
                auto more_bytes = file.read_some(empty_buf, ec);
                REQUIRE(more_bytes == 0);
                REQUIRE_FALSE(ec);
            }
        }

        teardown();
    }
}

SCENARIO_METHOD(StreamFileFixture, "stream_file write operations", "[stream_file][write]") {
    GIVEN("a new file opened for writing") {
        setup();
        stream_file file(ctx, test_file_path, open_mode::write_only | open_mode::create);

        WHEN("writing data sequentially") {
            std::string data1 = "First chunk";
            std::string data2 = "Second chunk";
            const_buffer buf1(data1.data(), data1.size());
            const_buffer buf2(data2.data(), data2.size());
            std::error_code ec;

            auto bytes_written1 = file.write_some(buf1, ec);
            auto bytes_written2 = file.write_some(buf2, ec);

            THEN("all data is written correctly") {
                REQUIRE_FALSE(ec);
                REQUIRE(bytes_written1 == data1.size());
                REQUIRE(bytes_written2 == data2.size());
                REQUIRE(file.tell() == data1.size() + data2.size());
            }

            AND_THEN("the file content matches the written data") {
                file.close();
                auto content = read_file_content();
                REQUIRE(content == data1 + data2);
            }
        }

        WHEN("writing at different offsets using seek") {
            std::string initial = "Initial";
            std::string overwrite = "New";
            const_buffer initial_buf(initial.data(), initial.size());
            const_buffer overwrite_buf(overwrite.data(), overwrite.size());
            std::error_code ec;

            file.write_some(initial_buf, ec);
            REQUIRE_FALSE(ec);

            file.seek(0, seek_basis::begin, ec);
            REQUIRE_FALSE(ec);

            auto bytes_written = file.write_some(overwrite_buf, ec);

            THEN("the write overwrites from the new offset") {
                REQUIRE(bytes_written == overwrite.size());
                REQUIRE(file.tell() == overwrite.size());

                file.close();
                auto content = read_file_content();
                REQUIRE(content == "Newtial");
            }
        }

        teardown();
    }

    GIVEN("a file opened in append mode") {
        setup();
        std::string initial_content = "Initial content\n";
        create_test_file(initial_content);

        stream_file file(ctx, test_file_path, open_mode::append | open_mode::write_only);
        auto initial_size = file.size();

        WHEN("writing to the file") {
            std::string appended = "Appended content\n";
            const_buffer buf(appended.data(), appended.size());
            std::error_code ec;

            auto bytes_written = file.write_some(buf, ec);

            THEN("the data is appended at the end") {
                REQUIRE_FALSE(ec);
                REQUIRE(bytes_written == appended.size());
                REQUIRE(file.tell() == initial_size + appended.size());

                file.close();
                auto content = read_file_content();
                REQUIRE(content == initial_content + appended);
            }
        }

        teardown();
    }
}

SCENARIO_METHOD(StreamFileFixture, "stream_file seek operations", "[stream_file][seek]") {
    GIVEN("a file with content") {
        setup();
        std::string content = "0123456789ABCDEFGHIJ";
        create_test_file(content);
        stream_file file(ctx, test_file_path, open_mode::read_only);

        WHEN("seeking from beginning") {
            std::error_code ec;
            auto new_pos = file.seek(5, seek_basis::begin, ec);

            THEN("the position is set correctly") {
                REQUIRE_FALSE(ec);
                REQUIRE(new_pos == 5);
                REQUIRE(file.tell() == 5);
            }

            AND_THEN("reading from that position returns the expected data") {
                std::vector<char> buffer(5);
                mutable_buffer buf(buffer.data(), buffer.size());
                auto bytes_read = file.read_some(buf, ec);
                REQUIRE(bytes_read == 5);
                REQUIRE(std::string(buffer.data(), 5) == content.substr(5, 5));
            }
        }

        WHEN("seeking from current position") {
            std::error_code ec;
            file.seek(3, seek_basis::begin, ec);
            REQUIRE_FALSE(ec);

            auto new_pos = file.seek(2, seek_basis::current, ec);

            THEN("the position advances correctly") {
                REQUIRE_FALSE(ec);
                REQUIRE(new_pos == 5);
                REQUIRE(file.tell() == 5);
            }
        }

        WHEN("seeking from end") {
            std::error_code ec;
            auto new_pos = file.seek(-5, seek_basis::end, ec);

            THEN("the position is set from the end") {
                REQUIRE_FALSE(ec);
                REQUIRE(new_pos == content.size() - 5);
                REQUIRE(file.tell() == content.size() - 5);
            }

            AND_THEN("reading from that position returns the last 5 bytes") {
                std::vector<char> buffer(5);
                mutable_buffer buf(buffer.data(), buffer.size());
                auto bytes_read = file.read_some(buf, ec);
                REQUIRE(bytes_read == 5);
                REQUIRE(std::string(buffer.data(), 5) == content.substr(content.size() - 5));
            }
        }

        WHEN("seeking with invalid arguments") {
            std::error_code ec;

            AND_WHEN("seeking negative from beginning") {
                auto new_pos = file.seek(-1, seek_basis::begin, ec);
                THEN("it returns an error") {
                    REQUIRE(ec);
                    REQUIRE(new_pos == static_cast<std::uint64_t>(-1));
                }
            }

            AND_WHEN("seeking beyond start from current") {
                auto new_pos = file.seek(-100, seek_basis::current, ec);
                THEN("it returns an error") {
                    REQUIRE(ec);
                    REQUIRE(new_pos == static_cast<std::uint64_t>(-1));
                }
            }
        }

        teardown();
    }
}

SCENARIO_METHOD(StreamFileFixture, "stream_file async operations", "[stream_file][async]") {
    GIVEN("a file opened for reading") {
        setup();
        std::string test_content = "Async read test content";
        create_test_file(test_content);
        stream_file file(ctx, test_file_path, open_mode::read_only);

        WHEN("performing async read") {
            bool completed = false;
            std::error_code async_ec;
            std::size_t bytes_transferred = 0;
            collections::vector<char> buffer(10);

            file.async_read_some(io::buffer(buffer), [&](std::error_code ec, std::size_t transferred) {
                async_ec = ec;
                bytes_transferred = transferred;
                completed = true;
            });

            ctx.run();

            THEN("the async operation completes successfully") {
                REQUIRE(completed);
                REQUIRE_FALSE(async_ec);
                REQUIRE(bytes_transferred == 10);
                REQUIRE(std::string(buffer.data(), bytes_transferred) == test_content.substr(0, 10));
                REQUIRE(file.tell() == 10);
            }
        }

        teardown();
    }

    GIVEN("a file opened for writing") {
        setup();
        stream_file file(ctx, test_file_path, open_mode::write_only | open_mode::create);

        WHEN("performing async write") {
            bool completed = false;
            std::error_code async_ec;
            std::size_t bytes_transferred = 0;
            foundation::text::string write_data = "Async write test";

            file.async_write_some(io::buffer(write_data), [&](std::error_code ec, std::size_t transferred) {
                async_ec = ec;
                bytes_transferred = transferred;
                completed = true;
            });

            ctx.run();

            THEN("the async operation completes successfully") {
                REQUIRE(completed);
                REQUIRE_FALSE(async_ec);
                REQUIRE(bytes_transferred == write_data.size());
                REQUIRE(file.tell() == write_data.size());

                file.close();
                auto content = read_file_content();
                REQUIRE(content == write_data);
            }
        }

        teardown();
    }
}

SCENARIO_METHOD(StreamFileFixture, "stream_file move semantics", "[stream_file][move]") {
    GIVEN("an open stream_file") {
        setup();
        create_test_file("Test content");
        stream_file file1(ctx, test_file_path, open_mode::read_only);
        file1.read_some(io::buffer(std::vector<char>(5).data(), 5));
        auto offset1 = file1.tell();

        WHEN("moving to another stream_file") {
            stream_file file2(std::move(file1));

            THEN("the moved-from file is reset") {
                REQUIRE_FALSE(file1.is_open());
                REQUIRE(file1.tell() == 0);
            }

            AND_THEN("the moved-to file retains the state") {
                REQUIRE(file2.is_open());
                REQUIRE(file2.tell() == offset1);
            }
        }

        WHEN("move assigning to another stream_file") {
            stream_file file2(ctx);
            file2 = std::move(file1);

            THEN("the moved-from file is reset") {
                REQUIRE_FALSE(file1.is_open());
                REQUIRE(file1.tell() == 0);
            }

            AND_THEN("the move-assigned file retains the state") {
                REQUIRE(file2.is_open());
                REQUIRE(file2.tell() == offset1);
            }
        }

        teardown();
    }
}

SCENARIO_METHOD(StreamFileFixture, "stream_file size operations", "[stream_file][size]") {
    GIVEN("an empty file") {
        setup();
        create_test_file("");
        stream_file file(ctx, test_file_path, open_mode::read_write);

        THEN("the size is zero") {
            REQUIRE(file.size() == 0);
        }

        WHEN("resizing to a larger size") {
            auto ec = file.resize(1024);

            THEN("the resize operation succeeds") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 1024);
            }
        }

        teardown();
    }

    GIVEN("a file with content") {
        setup();
        std::string content = "Original content";
        create_test_file(content);
        stream_file file(ctx, test_file_path, open_mode::write_only | open_mode::read_write);

        WHEN("resizing to a smaller size") {
            auto ec = file.resize(5);

            THEN("the file is truncated") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 5);

                file.close();
                auto new_content = read_file_content();
                REQUIRE(new_content == content.substr(0, 5));
            }
        }

        WHEN("resizing to a larger size") {
            auto ec = file.resize(100);

            THEN("the file is extended") {
                REQUIRE_FALSE(ec);
                REQUIRE(file.size() == 100);
            }
        }

        teardown();
    }
}
