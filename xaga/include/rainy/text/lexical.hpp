#pragma once
#include <map>
#include <string>
#include <format>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

#include <rainy/text/lexical/char.hpp>
//#include "lexical/wchar_t.hpp"

namespace rainy::text::lexical {
    template <typename Ty, typename CharType>
    Ty lexical_cast(std::basic_string_view<CharType> input) {
    }

    template <typename Ty, typename chars>
    std::basic_string<chars> lexical_to_string(const Ty &value);

    template <typename Ty>
    Ty lexical_cast(std::string_view input) {
        return lexical_caster<char, Ty>{}.convert(input);
    }

    template <typename Ty>
    Ty lexical_cast(std::wstring_view input) {
        return lexical_caster<wchar_t, Ty>{}.convert(input);
    }

    template <typename Ty>
    Ty lexical_cast(std::u16string_view input) {
        return lexical_caster<char16_t, Ty>{}.convert(input);
    }

    template <typename Ty>
    Ty lexical_cast(std::u32string_view input) {
        return lexical_caster<char32_t, Ty>{}.convert(input);
    }

    template <typename Ty>
    std::string lexical_to_string(const Ty &value) {
        return lexical_caster<char, Ty>{}.to_string(value);
    }
}
