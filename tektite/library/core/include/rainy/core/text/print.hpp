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
    /**
     * \lang english
     * @brief Formats a format string with arguments and writes the result to stdout.
     *
     * @param fmt The format string
     * @param args The pre-built format arguments
     *
     * \lang simp-chinese
     * @brief 使用格式字符串与参数进行格式化，并将结果写入 stdout。
     *
     * @param fmt 格式字符串
     * @param args 预先构造的格式化参数
     */
    void vprint_unicode(const string_view fmt, const format_args args);
    /**
     * \lang english
     * @brief Formats a format string with arguments and writes the result to the given stream.
     *
     * @param stream The output stream
     * @param fmt The format string
     * @param args The pre-built format arguments
     *
     * \lang simp-chinese
     * @brief 使用格式字符串与参数进行格式化，并将结果写入指定流。
     *
     * @param stream 输出流
     * @param fmt 格式字符串
     * @param args 预先构造的格式化参数
     */
    void vprint_unicode(FILE *stream, const string_view fmt, const format_args args);
    /**
     * \lang english
     * @brief Thread-safe variant that locks the stream while writing the formatted output.
     *
     * @param stream The output stream
     * @param fmt The format string
     * @param args The pre-built format arguments
     *
     * \lang simp-chinese
     * @brief 线程安全版本，写入格式化结果期间会锁定输出流。
     *
     * @param stream 输出流
     * @param fmt 格式字符串
     * @param args 预先构造的格式化参数
     */
    void vprint_unicode_locking(FILE *stream, const string_view fmt, const format_args args);
    /**
     * \lang english
     * @brief Formats without Unicode-aware handling and writes the result to stdout.
     *
     * @param fmt The format string
     * @param args The pre-built format arguments
     *
     * \lang simp-chinese
     * @brief 不做 Unicode 感知处理，直接将格式化结果写入 stdout。
     *
     * @param fmt 格式字符串
     * @param args 预先构造的格式化参数
     */
    void vprint_nonunicode(const string_view fmt, const format_args args);
    /**
     * \lang english
     * @brief Formats without Unicode-aware handling and writes the result to the given stream.
     *
     * @param stream The output stream
     * @param fmt The format string
     * @param args The pre-built format arguments
     *
     * \lang simp-chinese
     * @brief 不做 Unicode 感知处理，直接将格式化结果写入指定流。
     *
     * @param stream 输出流
     * @param fmt 格式字符串
     * @param args 预先构造的格式化参数
     */
    void vprint_nonunicode(FILE *stream, const string_view fmt, const format_args args);
    /**
     * \lang english
     * @brief Thread-safe variant of vprint_nonunicode that locks the stream while writing.
     *
     * @param stream The output stream
     * @param fmt The format string
     * @param args The pre-built format arguments
     *
     * \lang simp-chinese
     * @brief vprint_nonunicode 的线程安全版本，写入期间会锁定输出流。
     *
     * @param stream 输出流
     * @param fmt 格式字符串
     * @param args 预先构造的格式化参数
     */
    void vprint_nonunicode_locking(FILE *stream, const string_view fmt, const format_args args);
    /**
     * \lang english
     * @brief Writes a newline to stdout.
     *
     * \lang simp-chinese
     * @brief 向 stdout 写入一个换行符。
     */
    void println();
    /**
     * \lang english
     * @brief Writes a newline to the given stream.
     *
     * @param stream The output stream
     *
     * \lang simp-chinese
     * @brief 向指定流写入一个换行符。
     *
     * @param stream 输出流
     */
    void println(FILE *stream);

    /**
     * \lang english
     * @brief Formats the arguments according to the format string and prints the result to stdout.
     *
     * @tparam Args The argument types
     * @param fmt The format string
     * @param args The arguments to format
     *
     * \lang simp-chinese
     * @brief 按照格式字符串格式化参数，并将结果打印到 stdout。
     *
     * @tparam Args 参数类型
     * @param fmt 格式字符串
     * @param args 待格式化的参数
     */
    template <typename... Args>
    void print(const string_view fmt, Args &&...args) {
        print(stdout, fmt, utility::forward<Args>(args)...);
    }

    /**
     * \lang english
     * @brief Formats the arguments according to the format string and prints the result to the given stream.
     *
     * @tparam Args The argument types
     * @param stream The output stream
     * @param fmt The format string
     * @param args The arguments to format
     *
     * \lang simp-chinese
     * @brief 按照格式字符串格式化参数，并将结果打印到指定流。
     *
     * @tparam Args 参数类型
     * @param stream 输出流
     * @param fmt 格式字符串
     * @param args 待格式化的参数
     */
    template <typename... Args>
    void print(FILE *stream, const string_view fmt, Args &&...args) {
        auto arg_store = make_format_args(args...);
        vprint_unicode(stream, fmt, format_args(arg_store));
    }

    /**
     * \lang english
     * @brief Formats the arguments, prints the result followed by a newline to stdout.
     *
     * @tparam Args The argument types
     * @param fmt The format string
     * @param args The arguments to format
     *
     * \lang simp-chinese
     * @brief 格式化参数后将结果及一个换行符打印到 stdout。
     *
     * @tparam Args 参数类型
     * @param fmt 格式字符串
     * @param args 待格式化的参数
     */
    template <typename... Args>
    void println(const string_view fmt, Args &&...args) {
        println(stdout, fmt, utility::forward<Args>(args)...);
    }

    /**
     * \lang english
     * @brief Formats the arguments, prints the result followed by a newline to the given stream.
     *
     * @tparam Args The argument types
     * @param stream The output stream
     * @param fmt The format string
     * @param args The arguments to format
     *
     * \lang simp-chinese
     * @brief 格式化参数后将结果及一个换行符打印到指定流。
     *
     * @tparam Args 参数类型
     * @param stream 输出流
     * @param fmt 格式字符串
     * @param args 待格式化的参数
     */
    template <typename... Args>
    void println(FILE *stream, const string_view fmt, Args &&...args) {
        print(stream, fmt, utility::forward<Args>(args)...);
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