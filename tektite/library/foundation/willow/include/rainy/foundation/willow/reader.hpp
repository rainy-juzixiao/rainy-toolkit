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
#ifndef RAINY_FOUNDATION_WILLOW_JSON_UTILITY_HPP
#define RAINY_FOUNDATION_WILLOW_JSON_UTILITY_HPP
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::willow {
    template <typename Type>
    struct unicode_reader;
}

namespace rainy::foundation::willow::implements {
    template <typename CharType, typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_reader_impl {
        static_assert(sizeof(CharType) == 1, "unicode_reader_impl primary accepts only 1-byte character types");

        using string_type = StringTemplate<CharType, Traits, Alloc>;

        unicode_reader_impl(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
        }

        RAINY_NODISCARD uint8_t get_byte(const std::size_t i) const {
            return static_cast<uint8_t>(val.at(i));
        }

        bool get_code(std::size_t &i, std::uint32_t &code_output) const {
            using namespace rainy::foundation::exceptions::willow;
            static constexpr std::array<std::uint8_t, 256> utf8_extra_bytes = {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
            };
            static constexpr std::array<std::uint32_t, 6> utf8_offsets = {
                0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080,
            };
            if (i >= val.size()) {
                return false;
            }
            if (!escape_unicode) {
                const auto byte = get_byte(i);
                code_output = static_cast<std::uint32_t>(byte);
                i++;
                return true;
            }
            const auto first_byte = get_byte(i);
            const auto extra_bytes_to_read = utf8_extra_bytes[first_byte];
            if (i + static_cast<std::size_t>(extra_bytes_to_read) >= val.size()) {
                exceptions::willow::throw_willow_serialize_error("string was incomplete");
            }
            std::uint32_t code = 0;
            switch (extra_bytes_to_read) {
                case 5:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 4:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 3:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 2:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 1:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 0:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    RAINY_FALLTHROUGH;
                default:
                    break;
            }
            code -= utf8_offsets[extra_bytes_to_read]; // NOLINT
            code_output = code;
            return true;
        }

        const string_type &val;
        const bool escape_unicode;
        std::uint8_t state = 0;
    };

    template <typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_reader_impl<wchar_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<wchar_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;

        unicode_reader_impl(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
        }

        bool get_code(std::size_t &i, std::uint32_t &code) const {
            using namespace foundation::exceptions::willow;
            if (i >= val.size()) {
                return false;
            }
            if constexpr (sizeof(char_type) == 4) {
                code = static_cast<std::uint32_t>(val.at(i));
                i++;
                return true;
            } else {
                code = static_cast<std::uint32_t>(static_cast<uint16_t>(val.at(i)));
                i++;
                if (!escape_unicode) {
                    return true;
                }
                if (implements::unicode_surrogate_lead_begin <= code && code <= implements::unicode_surrogate_lead_end) {
                    if (i >= val.size()) {
                        throw_willow_serialize_error("string was incomplete");
                    }
                    const std::uint32_t lead_surrogate = code;
                    const std::uint32_t trail_surrogate = static_cast<uint16_t>(val.at(i));
                    code = implements::merge_surrogates(lead_surrogate, trail_surrogate);
                    i++;
                }
                return true;
            }
        }

        const string_type &val;
        const bool escape_unicode;
    };

    template <typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_reader_impl<char16_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char16_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;

        unicode_reader_impl(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
        }

        bool get_code(std::size_t &i, std::uint32_t &code) const {
            using namespace foundation::exceptions::willow;
            if (i >= val.size()) {
                return false;
            }
            code = static_cast<std::uint32_t>(static_cast<uint16_t>(val.at(i)));
            i++;
            if (!escape_unicode) {
                return true;
            }
            if (implements::unicode_surrogate_lead_begin <= code && code <= implements::unicode_surrogate_lead_end) {
                if (i >= val.size()) {
                    throw_willow_serialize_error("string was incomplete");
                }
                const std::uint32_t lead_surrogate = code;
                const std::uint32_t trail_surrogate = static_cast<uint16_t>(val.at(i));
                code = implements::merge_surrogates(lead_surrogate, trail_surrogate);
                i++;
            }
            return true;
        }

        const string_type &val;
        const bool escape_unicode;
    };

    template <typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_reader_impl<char32_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char32_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;

        unicode_reader_impl(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
        }

        bool get_code(std::size_t &i, std::uint32_t &code) const {
            if (i >= val.size()) {
                return false;
            }
            code = static_cast<std::uint32_t>(val.at(i));
            i++;
            return true;
        }

        const string_type &val;
        const bool escape_unicode;
    };
}

namespace rainy::foundation::willow {
    template <typename Traits, typename Alloc>
    struct unicode_reader<std::basic_string<char, Traits, Alloc>>
        : implements::unicode_reader_impl<char, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_reader_impl<char, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<core::text::basic_string<char, Traits, Alloc>>
        : implements::unicode_reader_impl<char, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_reader_impl<char, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };

#if RAINY_HAS_CXX20
    template <typename Traits, typename Alloc>
    struct unicode_reader<std::basic_string<char8_t, Traits, Alloc>>
        : implements::unicode_reader_impl<char8_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_reader_impl<char8_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<core::text::basic_string<char8_t, Traits, Alloc>>
        : implements::unicode_reader_impl<char8_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_reader_impl<char8_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };
#endif

    template <typename Traits, typename Alloc>
    struct unicode_reader<std::basic_string<wchar_t, Traits, Alloc>>
        : implements::unicode_reader_impl<wchar_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_reader_impl<wchar_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<core::text::basic_string<wchar_t, Traits, Alloc>>
        : implements::unicode_reader_impl<wchar_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_reader_impl<wchar_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<std::basic_string<char16_t, Traits, Alloc>>
        : implements::unicode_reader_impl<char16_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_reader_impl<char16_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<core::text::basic_string<char16_t, Traits, Alloc>>
        : implements::unicode_reader_impl<char16_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_reader_impl<char16_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<std::basic_string<char32_t, Traits, Alloc>>
        : implements::unicode_reader_impl<char32_t, Traits, Alloc, std::basic_string> {
        using base = implements::unicode_reader_impl<char32_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_reader<core::text::basic_string<char32_t, Traits, Alloc>>
        : implements::unicode_reader_impl<char32_t, Traits, Alloc, core::text::basic_string> {
        using base = implements::unicode_reader_impl<char32_t, Traits, Alloc, core::text::basic_string>;
        using base::base;
    };
}

#endif
