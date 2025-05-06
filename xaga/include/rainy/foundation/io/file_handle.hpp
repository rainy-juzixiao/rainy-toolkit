/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_IO_FILE_HANDLE_HPP
#define RAINY_FOUNDATION_IO_FILE_HANDLE_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io {
    class file_handle {
    public:
        using stream_type = std::FILE *;

        enum class open_mode : unsigned int {
            no_state = 0x00,
            read = 0x01,
            write = 0x02,
            append = 0x04,
            binary = 0x08,
            truncate = 0x10,
            nocreate = 0x20,
            update = 0x40
        };

        file_handle() : stream(nullptr), has_stream_(false), mode_flags(open_mode::no_state) {
        }

        file_handle(std::nullptr_t) : stream(nullptr), has_stream_(false), mode_flags(open_mode::no_state) {
        }

        file_handle(const char *filename, const char *mode) : stream(nullptr), has_stream_(false), mode_flags(open_mode::no_state) {
            open(filename, mode);
        }

        file_handle(file_handle &&) noexcept = default;

        file_handle &operator=(file_handle &&) noexcept = default;

        file_handle(const file_handle &) = delete;

        file_handle &operator=(const file_handle &) = delete;

        ~file_handle() {
            close();
        }

        bool open(const char *filename, const char *mode) {
            if (has_stream()) {
                if (!close()) {
                    return false;
                }
            }
            if (!filename || !mode) {
                errno = EINVAL;
                return false;
            }

            stream = std::fopen(filename, mode);
            if (!stream) {
                if (errno == ENOENT && has_flag(open_mode::nocreate)) {
                    return false; // 文件不存在且设置了 nocreate 标志
                }
                return false;
            }

            has_stream_ = true;
            mode_flags = static_cast<open_mode>(parse_mode(mode));
            return true;
        }

        bool has_stream() const noexcept {
            return has_stream_;
        }

        bool close() {
            if (stream == stdout || stream == stdin || stream == stderr || !stream) {
                return true;
            }
            if (std::fclose(stream) != 0) {
                return false;
            }
            stream = nullptr;
            has_stream_ = false;
            mode_flags = open_mode::no_state;
            return true;
        }

        stream_type get_stream() noexcept {
            return stream;
        }

        const stream_type get_stream() const noexcept {
            return stream;
        }

        bool has_flag(open_mode flag) const noexcept {
            return static_cast<unsigned int>(mode_flags) & static_cast<unsigned int>(flag);
        }

        open_mode get_mode_flags() const noexcept {
            return mode_flags;
        }

        operator stream_type() noexcept {
            return stream;
        }

        explicit operator bool() const noexcept {
            return has_stream();
        }

        bool came_from_iob_func() const noexcept {
            return stream == stdin || stream == stdout || stream == stderr;
        }

        static file_handle iob_func(const std::size_t idx) {
            switch (idx) {
                case 0:
                    return file_handle(stdin);
                case 1:
                    return file_handle(stdout);
                case 2:
                    return file_handle(stderr);
                default:
                    return file_handle(nullptr);
            }
        }

    private:
        file_handle(stream_type std_handle) : stream(std_handle), mode_flags(open_mode::no_state) {
        }

        unsigned int parse_mode(const char *mode) const noexcept {
            unsigned int flags{0};
            if (std::strchr(mode, 'r')) {
                flags |= static_cast<unsigned int>(open_mode::read);
            }
            if (std::strchr(mode, 'w')) {
                flags |= static_cast<unsigned int>(open_mode::write);
            }
            if (std::strchr(mode, 'a')) {
                flags |= static_cast<unsigned int>(open_mode::append);
            }
            if (std::strchr(mode, 'b')) {
                flags |= static_cast<unsigned int>(open_mode::binary);
            }
            if (std::strchr(mode, '+')) {
                flags |= static_cast<unsigned int>(open_mode::update);
            }
            return flags;
        }

        bool has_stream_{false};
        stream_type stream;
        open_mode mode_flags;
    };

    static file_handle stdin_fhandle = file_handle::iob_func(0);
    static file_handle stdout_fhandle = file_handle::iob_func(1);
    static file_handle stderr_fhandle = file_handle::iob_func(2);
}

#endif