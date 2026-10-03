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
#ifndef RAINY_FOUNDATION_WILLOW_JSON_WRITER_HPP
#define RAINY_FOUNDATION_WILLOW_JSON_WRITER_HPP
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::willow {
    template <typename Type>
    struct unicode_writer;
}

namespace rainy::foundation::willow::implements {
    template <typename CharType, typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_writer_impl {
        static_assert(sizeof(CharType) == 1, "unicode_writer_impl primary accepts only 1-byte character types");

        using string_type = StringTemplate<CharType, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            if (code < 0x80) {
                add_char(static_cast<char_int_type>(code));
            } else if (code <= 0x7FF) {
                add_char(static_cast<char_int_type>(0xC0 | (code >> 6)));
                add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
            } else if (code <= 0xFFFF) {
                add_char(static_cast<char_int_type>(0xE0 | (code >> 12)));
                add_char(static_cast<char_int_type>(0x80 | ((code >> 6) & 0x3F)));
                add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
            } else {
                add_char(static_cast<char_int_type>(0xF0 | (code >> 18)));
                add_char(static_cast<char_int_type>(0x80 | ((code >> 12) & 0x3F)));
                add_char(static_cast<char_int_type>(0x80 | ((code >> 6) & 0x3F)));
                add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
            }
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            add_code(implements::merge_surrogates(lead_surrogate, trail_surrogate));
        }

        string_type &buffer;
    };

    template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_writer_impl<wchar_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<wchar_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            if constexpr (sizeof(char_type) == 4) {
                add_char(static_cast<char_int_type>(code));
            } else {
                using namespace rainy::foundation::exceptions::willow;
                if (code < 0xFFFF) {
                    add_char(static_cast<char_int_type>(code));
                } else {
                    exceptions::willow::throw_willow_parse_error("invalid 16-bits unicode character");
                }
            }
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            if constexpr (sizeof(char_type) == 4) {
                add_code(implements::merge_surrogates(lead_surrogate, trail_surrogate));
            } else {
                add_code(lead_surrogate);
                add_code(trail_surrogate);
            }
        }

        string_type &buffer;
    };

    template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_writer_impl<char16_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char16_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            using namespace foundation::exceptions::willow;
            if (code < 0xFFFF) {
                add_char(static_cast<char_int_type>(code));
            } else {
                exceptions::willow::throw_willow_parse_error("Invalid 16-bits unicode character");
            }
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            add_code(lead_surrogate);
            add_code(trail_surrogate);
        }

        string_type &buffer;
    };

    template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_writer_impl<char32_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char32_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            using namespace foundation::exceptions::willow;
            add_char(code);
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            add_code(implements::merge_surrogates(lead_surrogate, trail_surrogate));
        }

        string_type &buffer;
    };
}

namespace rainy::foundation::willow {
    template <typename Traits, typename Alloc>
    struct unicode_writer<core::text::basic_string<char, Traits, Alloc>>
        : implements::unicode_writer_impl<char, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_writer_impl<char, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };

#if RAINY_HAS_CXX20
    template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<char8_t, Traits, Alloc>>
        : implements::unicode_writer_impl<char8_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_writer_impl<char8_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<core::text::basic_string<char8_t, Traits, Alloc>>
        : implements::unicode_writer_impl<char8_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_writer_impl<char8_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };
#endif

    template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<wchar_t, Traits, Alloc>>
        : implements::unicode_writer_impl<wchar_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_writer_impl<wchar_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<char, Traits, Alloc>>
        : implements::unicode_writer_impl<char, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_writer_impl<char, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<core::text::basic_string<wchar_t, Traits, Alloc>>
        : implements::unicode_writer_impl<wchar_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_writer_impl<wchar_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<char16_t, Traits, Alloc>>
        : implements::unicode_writer_impl<char16_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_writer_impl<char16_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<core::text::basic_string<char16_t, Traits, Alloc>>
        : implements::unicode_writer_impl<char16_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_writer_impl<char16_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<char32_t, Traits, Alloc>>
        : implements::unicode_writer_impl<char32_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_writer_impl<char32_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<core::text::basic_string<char32_t, Traits, Alloc>>
        : implements::unicode_writer_impl<char32_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_writer_impl<char32_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };
}

namespace rainy::foundation::willow {
    template <typename Ty>
    class output_streambuf final : public std::basic_streambuf<Ty> {
    public:
        using char_type = typename std::basic_streambuf<Ty>::char_type;
        using int_type = typename std::basic_streambuf<Ty>::int_type;
        using char_traits = core::text::char_traits<char_type>;

        explicit output_streambuf(output_adapter<char_type> adapter) : adapter(adapter) {
        }

    protected:
        int_type overflow(int_type c) override {
            if (c != EOF) {
                adapter->write(char_traits::to_char_type(c));
            }
            return c;
        }

        std::streamsize xsputn(const char_type *s, std::streamsize num) override {
            adapter->write(s, static_cast<std::size_t>(num));
            return num;
        }

    private:
        output_adapter<char_type> adapter;
    };
}


#endif
