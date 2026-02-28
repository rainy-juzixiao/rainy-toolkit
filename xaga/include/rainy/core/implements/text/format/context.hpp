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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_CONTEXT_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_CONTEXT_HPP
#include <array>
#include <bitset>
#include <iterator>
#include <locale>
#include <optional>
#include <rainy/core/implements/text/format/format_arg.hpp>
#include <rainy/core/implements/text/string.hpp>
#include <rainy/core/implements/text/string_view.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace rainy::foundation::text {
    template <class Context>
    class basic_format_args;

    template <class Context>
    class basic_format_arg;

    template <typename T, typename CharT, typename = void>
    struct formatter {
        static_assert(sizeof(T) == 0, "formatter must be specialized for this type");
    };

    template <typename CharT>
    class basic_format_parse_context {
    public:
        using char_type = CharT;
        using const_iterator = typename basic_string_view<CharT>::const_iterator;
        using iterator = const_iterator;

        constexpr explicit basic_format_parse_context(basic_string_view<CharT> fmt, size_t num_args = 0) noexcept :
            fmt_(fmt), num_args_(num_args), next_arg_id_(0), manual_indexing_(false) {
        }

        basic_format_parse_context(const basic_format_parse_context &) = delete;
        basic_format_parse_context &operator=(const basic_format_parse_context &) = delete;

        RAINY_NODISCARD constexpr const_iterator begin() const noexcept {
            return fmt_.begin();
        }

        RAINY_NODISCARD constexpr const_iterator end() const noexcept {
            return fmt_.end();
        }

        constexpr void advance_to(const_iterator it) {
            fmt_ = basic_string_view<CharT>(it, fmt_.end());
        }

        RAINY_NODISCARD constexpr size_t next_arg_id() {
            if (manual_indexing_) {
                throw std::format_error("cannot switch from manual to automatic argument indexing");
            }
            return next_arg_id_++;
        }

        constexpr void check_arg_id(size_t id) {
            if (!manual_indexing_ && next_arg_id_ > 0) {
                throw std::format_error("cannot switch from automatic to manual argument indexing");
            }
            if (id >= num_args_) {
                throw std::format_error("argument index out of range");
            }
            manual_indexing_ = true;
        }

        template <typename... Args>
        constexpr void check_dynamic_spec_integral(size_t id) const {
            if (id >= num_args_) {
                throw std::format_error("dynamic spec argument index out of range");
            }
        }

        template <typename... Args>
        constexpr void check_dynamic_spec_string(size_t id) const {
            if (id >= num_args_) {
                throw std::format_error("dynamic spec string argument index out of range");
            }
        }

    private:
        basic_string_view<CharT> fmt_;
        size_t num_args_;
        size_t next_arg_id_;
        bool manual_indexing_;
    };

    class format_error : public std::runtime_error {
    public:
        explicit format_error(const std::string &what_arg) : std::runtime_error(what_arg) {
        }

        explicit format_error(const char *what_arg) : std::runtime_error(what_arg) {
        }
    };

    template <class Out, class CharT>
    class basic_format_context {
    public:
        using iterator = Out;
        using char_type = CharT;

        template <class T>
        using formatter_type = formatter<T, CharT>;

    private:
        basic_format_args<basic_format_context> args_;
        Out out_;
        std::optional<std::locale> loc_; // 可选的 locale

        basic_format_context(const basic_format_context &) = delete;
        basic_format_context &operator=(const basic_format_context &) = delete;

    public:
        basic_format_context(Out out, basic_format_args<basic_format_context> args) :
            args_(args), out_(std::move(out)), loc_(std::nullopt) {
        }

        basic_format_context(Out out, basic_format_args<basic_format_context> args, const std::locale &loc) :
            args_(args), out_(std::move(out)), loc_(loc) {
        }

        basic_format_arg<basic_format_context> arg(size_t id) const noexcept {
            return args_.get(id);
        }

        std::locale locale() const {
            return loc_.value_or(std::locale());
        }

        iterator out() {
            return std::move(out_);
        }

        void advance_to(iterator it) {
            out_ = std::move(it);
        }
    };

    template <typename CharT>
    using format_buffer_iterator = utility::back_insert_iterator<basic_string<CharT>>;

    using format_context = basic_format_context<format_buffer_iterator<char>, char>;
    using wformat_context = basic_format_context<format_buffer_iterator<wchar_t>, wchar_t>;

    using format_parse_context = basic_format_parse_context<char>;
    using wformat_parse_context = basic_format_parse_context<wchar_t>;

    using format_args = basic_format_args<format_context>;
    using wformat_args = basic_format_args<wformat_context>;

    template <class Context, class... Args>
    constexpr auto make_format_args(const Args &...args) noexcept {
        return __format_arg_store<Context, Args...>(std::index_sequence_for<Args...>{}, args...);
    }

    template <class... Args>
    constexpr auto make_format_args(const Args &...args) noexcept {
        return make_format_args<format_context>(args...);
    }

    template <class... Args>
    constexpr auto make_wformat_args(const Args &...args) noexcept {
        return make_format_args<wformat_context>(args...);
    }
}

#endif