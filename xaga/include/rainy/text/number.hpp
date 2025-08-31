#include <rainy/core/core.hpp>
#include <string_view>

namespace rainy::text::implements {
    using float_type = double;
    using integer_type = long long;

    static float_type pow10(int exp) {
        float_type base = 10.0;
        float_type result = 1.0;
        if (exp < 0) {
            exp = -exp;
            base = 0.1;
        }
        while (exp) {
            if (exp & 1)
                result *= base;
            base *= base;
            exp >>= 1;
        }
        return result;
    }

    template <typename CharType, typename Traits>
    bool scan_integer_digits(std::basic_string_view<CharType, Traits> text, std::size_t &pos, integer_type &value) {
        if (pos >= text.size() || !std::isdigit(text[pos])) {
            return false;
        }
        value = text[pos++] - '0';
        while (pos < text.size() && std::isdigit(text[pos])) {
            value = value * 10 + (text[pos++] - '0');
        }
        return true;
    }

    template <typename CharType, typename Traits>
    bool scan_integer(std::basic_string_view<CharType, Traits> text, std::size_t &pos, float_type &number) {
        number = text[pos++] - '0';
        while (pos < text.size() && std::isdigit(text[pos])) {
            number = number * 10 + (text[pos++] - '0');
        }
        return true;
    }
}

namespace rainy::text {
    template <typename CharType, typename Traits>
    std::pair<bool, implements::float_type> parse_float(std::basic_string_view<CharType, Traits> text) {
        using namespace implements;
        std::size_t pos = 0;
        bool is_negative = false;
        float_type number = 0.0;
        // 处理符号
        if (pos < text.size() && text[pos] == '-') {
            is_negative = true;
            ++pos;
        }
        // 整数部分，可选
        integer_type int_part = 0;
        scan_integer_digits(text, pos, int_part);
        number = static_cast<float_type>(int_part);
        // 小数部分，可选
        if (pos < text.size() && text[pos] == '.') {
            ++pos; // 跳过 '.'
            float_type fraction_factor = 0.1;
            bool has_fraction = false;
            while (pos < text.size() && std::isdigit(text[pos])) {
                number += (text[pos++] - '0') * fraction_factor;
                fraction_factor *= 0.1;
                has_fraction = true;
            }
            if (!has_fraction) {
                return {false, 0.0}; // '.' 后必须有数字
            }
        }
        // 指数部分，可选
        if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E')) {
            ++pos; // 跳过 'e' 或 'E'
            bool neg_exp = false;
            if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) {
                neg_exp = (text[pos] == '-');
                ++pos;
            }
            if (pos >= text.size() || !std::isdigit(text[pos])) {
                return {false, 0.0}; // 指数后必须有数字
            }
            int exponent = 0;
            while (pos < text.size() && std::isdigit(text[pos])) {
                exponent = exponent * 10 + (text[pos++] - '0');
            }
            number *= pow10(neg_exp ? -exponent : exponent);
        }
        constexpr float_type SCI_THRESHOLD = 1e10;
        return {true, is_negative ? -number : number};
    }

    template <typename CharType, typename Traits>
    std::pair<bool, implements::integer_type> parse_integer(std::basic_string_view<CharType, Traits> text) {
        using namespace implements;
        std::size_t pos = 0;
        bool is_negative = false;
        integer_type value = 0;
        if (pos < text.size() && text[pos] == '-') {
            is_negative = true;
            ++pos;
        }
        std::size_t start_pos = pos; // 记录整数扫描起点
        bool scanned = scan_integer_digits(text, pos, value);
        // 如果整数扫描失败或者遇到浮点标志，尝试解析浮点
        if (!scanned || (pos < text.size() && (text[pos] == '.' || text[pos] == 'e' || text[pos] == 'E'))) {
            auto [ok, fvalue] = parse_float(text);
            if (!ok) {
                return {false, 0};
            }
            return {true, static_cast<integer_type>(fvalue)};
        }
        if (pos != text.size()) {
            return {false, 0}; // 非整数多余字符
        }
        return {true, is_negative ? -value : value};
    }
}
