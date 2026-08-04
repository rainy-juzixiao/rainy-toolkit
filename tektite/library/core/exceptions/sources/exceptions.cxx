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
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <rainy/core/diagnostics/exceptions.hpp>

namespace rainy::core::exceptions {
    namespace {
        class char_buffer {
        public:
            char_buffer() : data_(nullptr), size_(0) {
            }

            explicit char_buffer(std::size_t size) : data_(nullptr), size_(0) {
                if (size > 0) {
                    data_ = static_cast<char *>(std::malloc(size));
                    if (data_) {
                        size_ = size;
                        data_[0] = '\0';
                    }
                }
            }

            char_buffer(const char *str) : data_(nullptr), size_(0) {
                if (str) {
                    std::size_t len = std::strlen(str);
                    if (len > 0) {
                        data_ = static_cast<char *>(std::malloc(len + 1));
                        if (data_) {
                            size_ = len + 1;
                            std::memcpy(data_, str, len + 1);
                        }
                    }
                }
            }

            char_buffer(const char_buffer &other) : data_(nullptr), size_(0) {
                if (other.data_ && other.size_ > 0) {
                    data_ = static_cast<char *>(std::malloc(other.size_));
                    if (data_) {
                        size_ = other.size_;
                        std::memcpy(data_, other.data_, size_);
                    }
                }
            }

            char_buffer(char_buffer &&other) noexcept : data_(other.data_), size_(other.size_) {
                other.data_ = nullptr;
                other.size_ = 0;
            }

            char_buffer &operator=(const char_buffer &other) {
                if (this != &other) {
                    char_buffer temp(other);
                    swap(temp);
                }
                return *this;
            }

            char_buffer &operator=(char_buffer &&other) noexcept {
                if (this != &other) {
                    free();
                    data_ = other.data_;
                    size_ = other.size_;
                    other.data_ = nullptr;
                    other.size_ = 0;
                }
                return *this;
            }

            ~char_buffer() {
                free();
            }

            void swap(char_buffer &other) noexcept {
                std::swap(data_, other.data_);
                std::swap(size_, other.size_);
            }

            bool allocate(std::size_t size) {
                free();
                if (size > 0) {
                    data_ = static_cast<char *>(std::malloc(size));
                    if (data_) {
                        size_ = size;
                        data_[0] = '\0';
                        return true;
                    }
                }
                return false;
            }

            bool resize(std::size_t new_size) {
                if (new_size == 0) {
                    free();
                    return true;
                }

                char *new_data = static_cast<char *>(std::realloc(data_, new_size));
                if (!new_data) {
                    return false;
                }

                data_ = new_data;
                size_ = new_size;
                return true;
            }

            const char *c_str() const {
                return data_ ? data_ : "";
            }

            char *data() {
                return data_;
            }

            std::size_t size() const {
                return size_;
            }

            bool empty() const {
                return data_ == nullptr || size_ == 0;
            }

            void free() {
                if (data_) {
                    std::free(data_);
                    data_ = nullptr;
                    size_ = 0;
                }
            }

        private:
            char *data_;
            std::size_t size_;
        };
    }

    class exception::impl {
    public:
        impl() : message_() {
        }

        explicit impl(const source& location, const char* msg) {
            if (!msg) {
                msg = "";
            }

            std::size_t total_len = 0;

            const char* file = location.file_name();
            std::size_t file_len = std::strlen(file);
            total_len += file_len;

            unsigned long line_num = location.line();
            char line_buf[32];
            int line_len = 0;
            if (line_num > 0) {
                line_len = std::snprintf(line_buf, sizeof(line_buf), ":%lu", line_num);
                total_len += static_cast<std::size_t>(line_len);
            }

            unsigned long col_num = location.column();
            char col_buf[32];
            int col_len = 0;
            if (col_num > 0) {
                col_len = std::snprintf(col_buf, sizeof(col_buf), ":%lu", col_num);
                total_len += static_cast<std::size_t>(col_len);
            }

            const char* func = location.function_name();
            std::size_t func_len = std::strlen(func);
            if (func_len > 0) {
                total_len += 14;
                total_len += func_len;
                total_len += 1;
            }

            total_len += 3;
            total_len += std::strlen(msg);
            total_len += 1;

            if (!message_.allocate(total_len)) {
                const char* fallback = "Out of memory constructing exception message";
                message_ = char_buffer(fallback);
                return;
            }

            char* ptr = message_.data();

            std::memcpy(ptr, file, file_len);
            ptr += file_len;

            if (line_len > 0) {
                std::memcpy(ptr, line_buf, static_cast<std::size_t>(line_len));
                ptr += line_len;
            }

            if (col_len > 0) {
                std::memcpy(ptr, col_buf, static_cast<std::size_t>(col_len));
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

            message_.data()[total_len - 1] = '\0';
        }

        impl(const impl& other) : message_(other.message_) {
        }

        impl(impl&& other) noexcept : message_(std::move(other.message_)) {
        }

        impl& operator=(const impl& other) {
            if (this != &other) {
                message_ = other.message_;
            }
            return *this;
        }

        impl& operator=(impl&& other) noexcept {
            if (this != &other) {
                message_ = std::move(other.message_);
            }
            return *this;
        }

        ~impl() = default;

        const char* c_str() const noexcept {
            return message_.c_str();
        }

    private:
        char_buffer message_;
    };

    exception::exception(const char* message, const source& location)
        : impl_(new impl(location, message)) {
    }

    RAINY_NODISCARD const char* exception::what() const noexcept {
        return impl_ ? impl_->c_str() : "";
    }

    exception::exception(const exception& other)
        : impl_(other.impl_ ? new impl(*other.impl_) : new impl()) {
    }

    exception::exception(exception&& other) noexcept
        : impl_(other.impl_) {
        other.impl_ = nullptr;
    }

    exception& exception::operator=(const exception& other) {
        if (this != &other) {
            impl* new_impl = other.impl_ ? new impl(*other.impl_) : new impl();
            delete impl_;
            impl_ = new_impl;
        }
        return *this;
    }

    exception& exception::operator=(exception&& other) noexcept {
        if (this != &other) {
            delete impl_;
            impl_ = other.impl_;
            other.impl_ = nullptr;
        }
        return *this;
    }

    exception::~exception() {
        delete impl_;
        impl_ = nullptr;
    }

    exception::exception() : impl_(new impl()) {
    }

    void exception::build_message(const char* message, const source& location) {
        impl* new_impl = new impl(location, message);
        delete impl_;
        impl_ = new_impl;
    }
}

namespace rainy::core::exceptions {
    static exception_handler_t global_exception_handler_impl = &std::terminate;
    thread_local exception_handler_t current_thread_exception_handler_impl = &std::terminate;

    exception_handler_t global_exception_handler(exception_handler_t new_handler) noexcept {
        exception_handler_t old = global_exception_handler_impl;
        if (new_handler) {
            global_exception_handler_impl = new_handler;
        }
        return old;
    }

    exception_handler_t current_thread_exception_handler(exception_handler_t new_handler) noexcept {
        exception_handler_t old = current_thread_exception_handler_impl;
        if (new_handler) {
            current_thread_exception_handler_impl = new_handler;
        }
        return old;
    }
}

namespace rainy::core::exceptions::implements {
    void invoke_exception_handler() noexcept {
        {
            const auto invoke_address = current_thread_exception_handler();
            if (invoke_address) {
                invoke_address();
            }
        }
        {
            const auto invoke_address = global_exception_handler();
            if (invoke_address) {
                invoke_address();
            }
        }
    }
}