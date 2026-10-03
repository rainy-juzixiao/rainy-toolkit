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
#ifndef RAINY_FOUNDATION_WILLOW_APAPTER_HPP
#define RAINY_FOUNDATION_WILLOW_APAPTER_HPP
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/core/poly/basic_poly.hpp>
#include <rainy/core/text/char_traits.hpp>
#include <rainy/foundation/collections/unordered_map.hpp>

namespace rainy::foundation::willow {
    template <typename Derived, typename Ty>
    struct input_adapter {
        using char_type = Ty;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        char_int_type get_char() {
            return static_cast<Derived &>(*this).get_char_impl();
        }
    };

    template <typename Ty>
    struct file_input_adapter final : input_adapter<file_input_adapter<Ty>, Ty> {
        using base = input_adapter<file_input_adapter<Ty>, Ty>;
        using char_type = typename base::char_type;
        using char_traits = typename base::char_traits;
        using char_int_type = typename base::char_int_type;

        explicit file_input_adapter(std::FILE *file) : file(file) {
        }

        char_int_type get_char_impl() {
            return static_cast<char_int_type>(std::fgetc(file));
        }

    private:
        std::FILE *file;
    };

    template <typename Ty>
    struct stream_input_adapter final : input_adapter<stream_input_adapter<Ty>, Ty> {
        using base = input_adapter<stream_input_adapter<Ty>, Ty>;
        using char_type = typename base::char_type;
        using char_traits = typename base::char_traits;
        using char_int_type = typename base::char_int_type;

        explicit stream_input_adapter(std::basic_istream<char_type> &stream) : stream(stream), streambuf(*stream.rdbuf()) {
        }

        char_int_type get_char_impl() {
            auto ch = streambuf.sbumpc();
            if (ch == EOF) {
                stream.clear(stream.rdstate() | std::ios::eofbit);
            }
            return ch;
        }

        ~stream_input_adapter() {
            stream.clear(stream.rdstate() & std::ios::eofbit);
        }

    private:
        std::basic_istream<char_type> &stream;
        std::basic_streambuf<char_type> &streambuf;
    };

    template <typename Ty>
    struct string_input_adapter final : input_adapter<string_input_adapter<Ty>, typename Ty::value_type> {
        using base = input_adapter<string_input_adapter<Ty>, typename Ty::value_type>;
        using char_type = typename base::char_type;
        using char_traits = typename base::char_traits;
        using char_int_type = typename base::char_int_type;

        explicit string_input_adapter(const Ty &str) : str(str), index(0) {
        }

        char_int_type get_char_impl() {
            if (index == str.size()) {
                return char_traits::eof();
            }
            return str[index++];
        }

        const char_type *data() const noexcept {
            return str.data();
        }

        std::size_t size() const noexcept {
            return str.size();
        }

        std::size_t position() const noexcept {
            return index;
        }

        void set_position(const std::size_t value) noexcept {
            index = value;
        }

    private:
        const Ty &str;
        typename Ty::size_type index;
    };

    template <typename Ty>
    struct string_view_input_adapter final : input_adapter<string_view_input_adapter<Ty>, typename Ty::value_type> {
        using base = input_adapter<string_view_input_adapter<Ty>, typename Ty::value_type>;
        using char_type = typename base::char_type;
        using char_traits = typename base::char_traits;
        using char_int_type = typename base::char_int_type;

        explicit string_view_input_adapter(const Ty &str) : str(str), index(0) {
        }

        char_int_type get_char_impl() {
            if (index < str.size()) {
                return str[index++];
            }
            return char_traits::eof();
        }

        const char_type *data() const noexcept {
            return str.data();
        }

        std::size_t size() const noexcept {
            return str.size();
        }

        std::size_t position() const noexcept {
            return index;
        }

        void set_position(const std::size_t value) noexcept {
            index = value;
        }

    private:
        Ty str;
        typename Ty::size_type index;
    };

    template <typename Ty>
    struct buffer_input_adapter final : input_adapter<buffer_input_adapter<Ty>, Ty> {
        using base = input_adapter<buffer_input_adapter<Ty>, Ty>;
        using char_type = typename base::char_type;
        using char_traits = typename base::char_traits;
        using char_int_type = typename base::char_int_type;

        explicit buffer_input_adapter(const Ty *str) : str(str), length(char_traits::length(str)) {
        }

        char_int_type get_char_impl() {
            if (str[index] == '\0') {
                return char_traits::eof();
            }
            return str[index++];
        }

        const char_type *data() const noexcept {
            return str;
        }

        std::size_t size() const noexcept {
            return length;
        }

        std::size_t position() const noexcept {
            return index;
        }

        void set_position(const std::size_t value) noexcept {
            index = value;
        }

    private:
        const char_type *str;
        std::size_t index{0};
        std::size_t length{0};
    };

    template <typename Ty>
    struct output_adapter_abstract {
        using char_type = Ty;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        template <typename Impl>
        using impl = type_traits::other_trans::value_list<utility::get_overloaded_func<Impl, void(Ty)>(&Impl::write),
                                                          utility::get_overloaded_func<Impl, void(const Ty *str, std::size_t size)>(
                                                              &Impl::write)>;

        template <typename Base>
        struct type : Base {
            void write(Ty ch) {
                (void) this->template invoke<0>(*this, ch);
            }

            void write(const char_type *str, std::size_t size) {
                (void) this->template invoke<1>(*this, str, size);
            }
        };
    };

    template <typename Ty>
    using output_adapter = core::basic_poly<output_adapter_abstract<Ty>>;

    template <typename Ty>
    struct string_output_adapter final {
        using char_type = typename Ty::value_type;
        using size_type = typename Ty::size_type;
        using char_traits = std::char_traits<char_type>;

        explicit string_output_adapter(Ty &str) : string(str) {
        }

        void write(const char_type ch) {
            string.push_back(ch);
        }

        void write(const char_type *str, std::size_t size) {
            this->string.append(str, static_cast<size_type>(size));
        }



    private:
        Ty &string;
    };

    template <typename Ty>
    struct stream_output_adapter final {
        using char_type = Ty;
        using size_type = std::streamsize;
        using char_traits = std::char_traits<char_type>;

        explicit stream_output_adapter(std::basic_ostream<char_type> &stream) : stream(stream) {
        }

        void write(const char_type ch) {
            stream.put(ch);
        }

        void write(const char_type *str, const std::size_t size) {
            stream.write(str, static_cast<size_type>(size));
        }

    private:
        std::basic_ostream<char_type> &stream;
    };
}

#endif
