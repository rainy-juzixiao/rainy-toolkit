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
#ifndef RAINY_TOOLKIT_PRINT_HPP
#define RAINY_TOOLKIT_PRINT_HPP

#include <cstdio>
#include <rainy/core/text/format.hpp>

namespace rainy::core::text::implements {
    class file_output_iterator {
    public:
        using char_type = char;
        using difference_type = std::ptrdiff_t;

        explicit file_output_iterator(FILE *stream) noexcept : stream_(stream) {
        }

        file_output_iterator &operator=(char ch) {
            std::fputc(static_cast<unsigned char>(ch), stream_);
            return *this;
        }

        file_output_iterator &operator*() noexcept {
            return *this;
        }

        file_output_iterator &operator++() noexcept {
            return *this;
        }

        file_output_iterator &operator++(int) noexcept {
            return *this;
        }

    private:
        FILE *stream_;
    };
}

namespace rainy::core::text {
    void vprint_unicode(const string_view fmt, const format_args args);
    void vprint_unicode(FILE *stream, const string_view fmt, const format_args args);
    void vprint_unicode_locking(FILE *stream, const string_view fmt, const format_args args);
    void vprint_nonunicode(const string_view fmt, const format_args args);
    void vprint_nonunicode(FILE *stream, const string_view fmt, const format_args args);
    void vprint_nonunicode_locking(FILE *stream, const string_view fmt, const format_args args);
    void println();
    void println(FILE *stream);

    template <typename... Args>
    void print(const string_view fmt, Args &&...args) {
        print(stdout, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void print(FILE *stream, const string_view fmt, Args &&...args) {
        auto arg_store = make_format_args(args...);
        vprint_unicode(stream, fmt, format_args(arg_store));
    }

    template <typename... Args>
    void println(const string_view fmt, Args &&...args) {
        println(stdout, fmt, utility::forward<Args>(args)...);
    }

    template <typename... Args>
    void println(FILE *stream, const string_view fmt, Args &&...args) {
        print(stream, fmt, std::forward<Args>(args)...);
        std::fputc('\n', stream);
        std::fflush(stream);
    }
}

namespace rainy::text {
    using core::text::print;
    using core::text::println;
    using core::text::vprint_unicode;
    using core::text::vprint_unicode_locking;
    using core::text::vprint_nonunicode;
    using core::text::vprint_nonunicode_locking;
}

#endif