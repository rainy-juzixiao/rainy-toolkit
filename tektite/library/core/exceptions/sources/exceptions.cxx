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
#include <rainy/core/diagnostics/exceptions.hpp>
#include <cstdlib>  // for std::terminate

namespace rainy::core::exceptions {
    class exception::impl {
    public:
        impl() : message(nullptr), length(0) {}

        explicit impl(const source &location, const char *msg) : message(nullptr), length(0) {
            if (!msg) {
                msg = "";
            }

            std::size_t total_len = 0;

            const char *file = location.file_name();
            std::size_t file_len = std::strlen(file);

            unsigned long line_num = location.line();
            char line_buf[32];
            int line_len = 0;
            if (line_num > 0) {
                line_len = std::snprintf(line_buf, sizeof(line_buf), ":%lu", line_num);
            }

            unsigned long col_num = location.column();
            char col_buf[32];
            int col_len = 0;
            if (col_num > 0) {
                col_len = std::snprintf(col_buf, sizeof(col_buf), ":%lu", col_num);
            }

            const char *func = location.function_name();
            std::size_t func_len = std::strlen(func);

            total_len = file_len + line_len + col_len;

            if (func_len > 0) {
                total_len += 14;
                total_len += func_len;
            }

            total_len += 3;
            total_len += std::strlen(msg);

            length = total_len;
            message = new char[length + 1];

            char *ptr = message;

            std::memcpy(ptr, file, file_len);
            ptr += file_len;

            if (line_len > 0) {
                std::memcpy(ptr, line_buf, line_len);
                ptr += line_len;
            }

            if (col_len > 0) {
                std::memcpy(ptr, col_buf, col_len);
                ptr += col_len;
            }

            if (func_len > 0) {
                const char func_prefix[] = " in function '";
                std::memcpy(ptr, func_prefix, sizeof(func_prefix) - 1);
                ptr += sizeof(func_prefix) - 1;
                std::memcpy(ptr, func, func_len);
                ptr += func_len;
                *ptr++ = '\'';
            }

            std::memcpy(ptr, " : ", 3);
            ptr += 3;
            std::memcpy(ptr, msg, std::strlen(msg));

            message[length] = '\0';
        }

        impl(const impl &other) : message(nullptr), length(other.length) {
            if (other.message) {
                message = new char[length + 1];
                std::memcpy(message, other.message, length + 1);
            }
        }

        impl(impl &&other) noexcept : message(other.message), length(other.length) {
            other.message = nullptr;
            other.length = 0;
        }

        impl &operator=(const impl &other) {
            if (this != &other) {
                char *new_msg = nullptr;
                if (other.message) {
                    new_msg = new char[other.length + 1];
                    std::memcpy(new_msg, other.message, other.length + 1);
                }
                delete[] message;
                message = new_msg;
                length = other.length;
            }
            return *this;
        }

        impl &operator=(impl &&other) noexcept {
            if (this != &other) {
                delete[] message;
                message = other.message;
                length = other.length;
                other.message = nullptr;
                other.length = 0;
            }
            return *this;
        }

        ~impl() {
            delete[] message;
        }

        const char *c_str() const {
            return message ? message : "";
        }

    private:
        char *message;
        std::size_t length;
    };

    exception::exception(const char *message, const source &location)
        : impl_(new impl(location, message)) {
        if (!impl_) {
            std::terminate();
        }
    }

    RAINY_NODISCARD const char *exception::what() const noexcept {
        if (!impl_) {
            return "";
        }
        return impl_->c_str();
    }

    exception::exception(const exception &other)
        : impl_(other.impl_ ? new impl(*other.impl_) : new impl()) {
        if (!impl_) {
            std::terminate();
        }
    }

    exception::exception(exception &&other) noexcept
        : impl_(other.impl_) {
        if (!impl_) {
            std::terminate();
        }
        other.impl_ = new impl();
        if (!other.impl_) {
            std::terminate();
        }
    }

    exception &exception::operator=(const exception &other) {
        if (this != &other) {
            impl *new_impl = other.impl_ ? new impl(*other.impl_) : new impl();
            if (!new_impl) {
                std::terminate();
            }
            delete impl_;
            impl_ = new_impl;
        }
        return *this;
    }

    exception &exception::operator=(exception &&other) noexcept {
        if (this != &other) {
            delete impl_;
            impl_ = other.impl_;
            if (!impl_) {
                std::terminate();
            }
            other.impl_ = new impl();
            if (!other.impl_) {
                std::terminate();
            }
        }
        return *this;
    }

    exception::~exception() {
        if (impl_) {
            delete impl_;
            impl_ = nullptr;
        }
    }

    exception::exception() : impl_(new impl()) {
        if (!impl_) {
            std::terminate();
        }
    }

    void exception::build_message(const char *message, const source &location) {
        impl *new_impl = new impl(location, message);
        if (!new_impl) {
            std::terminate();
        }
        delete impl_;
        impl_ = new_impl;
    }
}