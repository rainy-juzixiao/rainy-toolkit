#ifndef RAINY_COMPONENT_JSON_IMPL
#define RAINY_COMPONENT_JSON_IMPL
#include <iomanip>
#include <rainy/core/core.hpp>
#include <rainy/component/willow/implements/value.hpp>
#include <rainy/component/willow/utils.hpp>
#include <rainy/component/willow/json_iter.hpp>

namespace rainy::component::willow::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename basic_json>
    struct json_lexer {
        using string_type = typename basic_json::string_type;
        using char_type = typename basic_json::char_type;
        using integer_type = typename basic_json::integer_type;
        using float_type = typename basic_json::float_type;
        using boolean_type = typename basic_json::boolean_type;
        using array_type = typename basic_json::array_type;
        using object_type = typename basic_json::object_type;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        json_lexer(adapters::input_adapter<char_type> adapter) : adapter_(adapter) {
            string_buffer_.reserve(96);
            read_next();
        }

        char_int_type read_next() {
            current = adapter_->get_char();
            return current;
        }

        void skip_spaces() {
            while (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
                read_next();
            }
        }

        token_type scan() {
            skip_spaces();
            token_type result;
            switch (current) {
                case '[':
                    result = token_type::begin_array;
                    break;
                case ']':
                    result = token_type::end_array;
                    break;
                case '{':
                    result = token_type::begin_object;
                    break;
                case '}':
                    result = token_type::end_object;
                    break;
                case ':':
                    result = token_type::name_separator;
                    break;
                case ',':
                    result = token_type::value_separator;
                    break;
                case 't':
                    return scan_literal<'t', 'r', 'u', 'e'>(token_type::literal_true);
                case 'f':
                    return scan_literal<'f', 'a', 'l', 's', 'e'>(token_type::literal_false);
                case 'n':

                    return scan_literal<'n', 'u', 'l', 'l'>(token_type::literal_null);
                case '\"':
                    return scan_string();
                case '-':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    return scan_number();
                case '\0':
                case char_traits::eof():
                    return token_type::end_of_input;
                default:
                    throw json_parse_error("unexpected character");
            }
            read_next();

            return result;
        }

        template <char_type... Ch>
        struct literal_checker {
            static void check(json_lexer &) {
            }
        };

        template <char_type This, char_type... Rest>
        struct literal_checker<This, Rest...> {
            static void check(json_lexer &s) {
                if (char_traits::to_char_type(s.current) != This) {
                    throw json_parse_error("unexpected literal");
                }
                s.read_next();
                literal_checker<Rest...>::check(s);
            }
        };

        template <char_type... chars>
        token_type scan_literal(token_type result) {
            literal_checker<chars...>::check(*this);
            return result;
        }

        token_type scan_string() {
            if (current != '\"') {
                throw json_parse_error("string must start with '\"'");
            }
            string_buffer_.clear();
            while (true) {
                switch (const auto ch = read_next(); ch) {
                    case char_traits::eof(): {
                        throw json_parse_error("unexpected end");
                    }
                    case '\"': {
                        // skip last `\"`
                        read_next();
                        return token_type::value_string;
                    }
                    case 0x00:
                    case 0x01:
                    case 0x02:
                    case 0x03:
                    case 0x04:
                    case 0x05:
                    case 0x06:
                    case 0x07:
                    case 0x08:
                    case 0x09:
                    case 0x0A:
                    case 0x0B:
                    case 0x0C:
                    case 0x0D:
                    case 0x0E:
                    case 0x0F:
                    case 0x10:
                    case 0x11:
                    case 0x12:
                    case 0x13:
                    case 0x14:
                    case 0x15:
                    case 0x16:
                    case 0x17:
                    case 0x18:
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    case 0x1F:
                        throw json_parse_error("invalid control character");
                    case '\\': {
                        switch (const uint32_t code = read_one_escaped_code(); read_next()) {
                            case '\"':
                                add_char('\"');
                                break;
                            case '\\':
                                add_char('\\');
                                break;
                            case '/':
                                add_char('/');
                                break;
                            case 'b':
                                add_char('\b');
                                break;
                            case 'f':
                                add_char('\f');
                                break;
                            case 'n':
                                add_char('\n');
                                break;
                            case 'r':
                                add_char('\r');
                                break;
                            case 't':
                                add_char('\t');
                                break;
                            case 'u': {
                                if (unicode_surrogate_lead_begin <= code && code <= unicode_surrogate_lead_end) {
                                    if (read_next() != '\\' || read_next() != 'u') {
                                        throw json_parse_error("lead surrogate must be followed by trail surrogate");
                                    }
                                    const auto lead_surrogate = code;
                                    const auto trail_surrogate = read_one_escaped_code();
                                    if (!(unicode_surrogate_lead_begin <= trail_surrogate &&
                                          trail_surrogate <= unicode_surrogate_lead_end)) {
                                        throw json_parse_error("surrogate U+D800...U+DBFF must be followed by U+DC00...U+DFFF");
                                    }
                                    utils::unicode_writer<string_type> uw(this->string_buffer_);
                                    uw.add_surrogates(lead_surrogate, trail_surrogate);
                                } else {
                                    utils::unicode_writer<string_type> uw(this->string_buffer_);
                                    uw.add_code(code);
                                }
                                break;
                            }
                            default: {
                                throw json_parse_error("invalid character");
                            }
                        }
                        break;
                    }
                    default: {
                        add_char(ch);
                    }
                }
            }
        }

        token_type scan_number() {
            is_negative_ = false;
            number_value_ = static_cast<float_type>(0.0);
            if (current == '-') {
                is_negative_ = true;
                read_next();
                return scan_integer();
            }
            if (current == '0') {
                if (read_next() == '.') {
                    return scan_float();
                } else {
                    return token_type::value_integer;
                }
            }
            return scan_integer();
        }

        token_type scan_integer() {
            if (!std::isdigit(current)) {
                throw json_parse_error("invalid integer");
            }
            number_value_ = static_cast<float_type>(current - '0');
            while (true) {
                const auto ch = read_next();
                if (ch == '.') {
                    return scan_float();
                }
                if (ch == 'e' || ch == 'E') {
                    return scan_exponent();
                }
                if (std::isdigit(ch)) {
                    number_value_ = number_value_ * 10 + (ch - '0');
                } else {
                    break;
                }
            }
            return token_type::value_integer;
        }

        token_type scan_float() {
            if (current != '.') {
                throw json_parse_error("float number must start with '.'");
            }
            read_next();
            if (!std::isdigit(current)) {
                throw json_parse_error("invalid float number");
            }
            float_type fraction = static_cast<float_type>(0.1);
            float_type decimal_part = 0.0;
            while (std::isdigit(current)) {
                decimal_part += static_cast<float_type>(current - '0') * fraction;
                fraction *= static_cast<float_type>(0.1);
                read_next();
            }
            number_value_ += decimal_part; // 累加到整数部分
            if (current == 'e' || current == 'E') {
                return scan_exponent();
            }
            return token_type::value_float;
        }

        token_type scan_exponent() {
            if (current != 'e' && current != 'E')
                throw json_parse_error("exponent number must contains 'e' or 'E'");
            read_next();
            if (const bool invalid = (std::isdigit(current) && current != '0') || (current == '-') || (current == '+'); !invalid)
                throw_exception(json_parse_error("invalid exponent number"));
            float_type base = 10;
            if (current == '+') {
                read_next();
            } else if (current == '-') {
                base = static_cast<float_type>(0.1);
                read_next();
            }
            uint32_t exponent = static_cast<uint32_t>(current - '0');
            while (std::isdigit(read_next())) {
                exponent = (exponent * 10) + static_cast<uint32_t>(current - '0');
            }
            float_type power = 1;
            for (; exponent; exponent >>= 1, base *= base) {
                if (exponent & 1) {
                    power *= base;
                }
            }
            number_value_ *= power;
            return token_type::value_float;
        }

        integer_type token_to_integer() const {
            integer_type integer = static_cast<integer_type>(number_value_);
            return is_negative_ ? -integer : integer;
        }

        float_type token_to_float() const {
            return is_negative_ ? -number_value_ : number_value_;
        }

        string_type token_to_string() const {
            return string_buffer_;
        }

        uint32_t read_one_escaped_code() {
            uint32_t code = 0;
            for (const auto factor: {12, 8, 4, 0}) {
                const auto ch = read_next();
                if (ch >= '0' && ch <= '9') {
                    code += ((ch - '0') << factor);
                } else if (ch >= 'A' && ch <= 'F') {
                    code += ((ch - 'A' + 10) << factor);
                } else if (ch >= 'a' && ch <= 'f') {
                    code += ((ch - 'a' + 10) << factor);
                } else {
                    throw json_parse_error("'\\u' must be followed by 4 hex digits");
                }
            }
            return code;
        }

        void add_char(const char_int_type ch) {
            string_buffer_.push_back(char_traits::to_char_type(ch));
        }

    private:
        bool is_negative_;
        float_type number_value_;
        string_type string_buffer_;
        adapters::input_adapter<char_type> adapter_;
        char_int_type current;
    };

    template <typename basic_json>
    struct json_parser {
        using string_type = typename basic_json::string_type;
        using char_type = typename basic_json::char_type;
        using integer_type = typename basic_json::integer_type;
        using float_type = typename basic_json::float_type;
        using boolean_type = typename basic_json::boolean_type;
        using array_type = typename basic_json::array_type;
        using object_type = typename basic_json::object_type;
        using char_traits = std::char_traits<char_type>;

        json_parser(adapters::input_adapter<char_type> adapter) : lexer(adapter), last_token(token_type::uninitialized) {
        }

        void parse(basic_json &json) {
            parse_value(json);
            if (get_token() != token_type::end_of_input) {
                throw_json_parse_error("unexpected token, expect end");
            }
        }

    private:
        token_type get_token() {
            last_token = lexer.scan();
            return last_token;
        }

        void parse_value(basic_json &json, bool read_next = true) {
            token_type token = last_token;
            if (read_next) {
                token = get_token();
            }

            switch (token) {
                case token_type::literal_true:
                    json = json_type::boolean;
                    json.value_.data.boolean = true;
                    break;

                case token_type::literal_false:
                    json = json_type::boolean;
                    json.value_.data.boolean = false;
                    break;

                case token_type::literal_null:
                    json = json_type::null;
                    break;

                case token_type::value_string:
                    json = lexer.token_to_string();
                    break;

                case token_type::value_integer:
                    json = lexer.token_to_integer();
                    break;

                case token_type::value_float:
                    json = lexer.token_to_float();
                    break;

                case token_type::begin_array:
                    json = json_type::array;
                    while (true) {
                        if (get_token() == token_type::end_array) {
                            break;
                        }
                        json.value_.data.vector->emplace_back(basic_json());
                        parse_value(json.value_.data.vector->back(), false);

                        // read ','
                        if (get_token() != token_type::value_separator)
                            break;
                    }
                    if (last_token != token_type::end_array)
                        throw json_parse_error("unexpected token in array");
                    break;

                case token_type::begin_object:
                    json = json_type::object;
                    while (true) {
                        if (get_token() != token_type::value_string)
                            break;
                        string_type key = lexer.token_to_string();

                        if (get_token() != token_type::name_separator)
                            break;

                        basic_json object;
                        parse_value(object);
                        json.value_.data.object->emplace(utility::move(key), utility::move(object));

                        // read ','
                        if (get_token() != token_type::value_separator)
                            break;
                    }
                    if (last_token != token_type::end_object)
                        throw_exception(json_parse_error("unexpected token in object"));
                    break;

                default:
                    throw_exception(json_parse_error("unexpected token"));
            }
        }

        json_lexer<basic_json> lexer;
        token_type last_token;
    };

    template <typename BasicJson>
    struct json_serializer {
        using string_type = typename BasicJson::string_type;
        using char_type = typename BasicJson::char_type;
        using integer_type = typename BasicJson::integer_type;
        using float_type = typename BasicJson::float_type;
        using boolean_type = typename BasicJson::boolean_type;
        using array_type = typename BasicJson::array_type;
        using object_type = typename BasicJson::object_type;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using args = serializer_args<BasicJson>;

        json_serializer(adapters::output_adapter<char_type> adapter, const args &args) :
            out_(utility::move(adapter)), arg_(args), indent_char_(args.indent_char) {
            string_buffer_.reserve(256);
            if (arg_.indent > 0) {
                indent_string_.assign(256, indent_char_);
            }
        }

        void dump(const BasicJson &json, const unsigned int current_indent = 0) {
            switch (json.type()) {
                case json_type::object: {
                    dump_object(json, current_indent);
                    break;
                }
                case json_type::array: {
                    dump_array(json, current_indent);
                    break;
                }
                case json_type::string: {
                    dump_string(*json.value_.data.string);
                    break;
                }
                case json_type::boolean: {
                    dump_boolean(json.value_.data.boolean);
                    break;
                }
                case json_type::number_integer: {
                    dump_integer(json.value_.data.number_integer);
                    break;
                }
                case json_type::number_float: {
                    dump_float(json.value_.data.number_float);
                    break;
                }
                case json_type::null: {
                    dump_null();
                    break;
                }
            }
        }

    private:
        void dump_object(const BasicJson &json, const unsigned int current_indent) {
            auto &object = (*json.value_.data.object);
            if (object.empty()) {
                write_literal<'{', '}'>();
                return;
            }
            const bool pretty_print = arg_.indent > 0;
            const unsigned int new_indent = current_indent + arg_.indent;
            out_->write(to_char_type('{'));
            if (pretty_print) {
                out_->write(to_char_type('\n'));
            }
            auto iter = object.cbegin();
            const auto size = object.size();
            for (std::size_t i = 0; i < size; ++i, ++iter) {
                if (pretty_print) {
                    write_indent(new_indent);
                }
                // 输出键
                out_->write(to_char_type('"'));
                dump_escaped_string(iter->first);
                out_->write(to_char_type('"'));
                out_->write(to_char_type(':'));
                if (pretty_print) {
                    out_->write(to_char_type(' '));
                }
                // 输出值
                dump(iter->second, new_indent);
                if (i != size - 1) {
                    out_->write(to_char_type(','));
                    if (pretty_print) {
                        out_->write(to_char_type('\n'));
                    }
                }
            }
            if (pretty_print) {
                out_->write(to_char_type('\n'));
                write_indent(current_indent);
            }
            out_->write(to_char_type('}'));
        }

        void dump_array(const BasicJson &json, const unsigned int current_indent) {
            auto &array = (*json.value_.data.vector);
            if (array.empty()) {
                write_literal<'[', ']'>();
                return;
            }
            const bool pretty_print = arg_.indent > 0;
            const unsigned int new_indent = current_indent + arg_.indent;
            out_->write(to_char_type('['));
            if (pretty_print) {
                out_->write(to_char_type('\n'));
            }
            const auto size = array.size();
            for (std::size_t i = 0; i < size; ++i) {
                if (pretty_print) {
                    write_indent(new_indent);
                }
                dump(array[i], new_indent);
                if (i != size - 1) {
                    out_->write(to_char_type(','));
                    if (pretty_print) {
                        out_->write(to_char_type('\n'));
                    }
                }
            }
            if (pretty_print) {
                out_->write(to_char_type('\n'));
                write_indent(current_indent);
            }
            out_->write(to_char_type(']'));
        }

        void dump_string(const string_type &str) {
            out_->write(to_char_type('"'));
            dump_escaped_string(str);
            out_->write(to_char_type('"'));
        }

        void dump_escaped_string(const string_type &str) {
            string_buffer_.clear();
            for (const auto &ch: str) {
                const auto c = static_cast<std::uint32_t>(ch);

                switch (c) {
                    case '\t':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('t'));
                        break;
                    case '\r':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('r'));
                        break;
                    case '\n':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('n'));
                        break;
                    case '\b':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('b'));
                        break;
                    case '\f':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('f'));
                        break;
                    case '"':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('"'));
                        break;
                    case '\\':
                        string_buffer_.push_back(to_char_type('\\'));
                        string_buffer_.push_back(to_char_type('\\'));
                        break;
                    default:
                        if (c <= 0x1F || (arg_.escape_unicode && c >= 0x7F)) {
                            escape_unicode(c);
                        } else {
                            string_buffer_.push_back(static_cast<char_type>(c));
                        }
                        break;
                }
                if (string_buffer_.size() > 400) {
                    out_->write(string_buffer_.data(), string_buffer_.size());
                    string_buffer_.clear();
                }
            }
            if (!string_buffer_.empty()) {
                out_->write(string_buffer_.data(), string_buffer_.size());
            }
        }

        void escape_unicode(std::uint32_t codepoint) {
            if (codepoint <= 0xFFFF) {
                // 基本多语言平面
                string_buffer_.push_back(to_char_type('\\'));
                string_buffer_.push_back(to_char_type('u'));
                append_hex(static_cast<std::uint16_t>(codepoint), 4);
            } else {
                // 需要代理对
                codepoint -= 0x10000;
                const auto high = static_cast<std::uint16_t>(0xD800 + (codepoint >> 10));
                const auto low = static_cast<std::uint16_t>(0xDC00 + (codepoint & 0x3FF));
                string_buffer_.push_back(to_char_type('\\'));
                string_buffer_.push_back(to_char_type('u'));
                append_hex(high, 4);
                string_buffer_.push_back(to_char_type('\\'));
                string_buffer_.push_back(to_char_type('u'));
                append_hex(low, 4);
            }
        }

        void append_hex(std::uint16_t value, int width) {
            static const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
            for (int i = width - 1; i >= 0; --i) {
                string_buffer_.push_back(to_char_type(hex_chars[(value >> (4 * i)) & 0xF]));
            }
        }

        void dump_boolean(boolean_type value) {
            if (value) {
                write_literal<'t', 'r', 'u', 'e'>();
            } else {
                write_literal<'f', 'a', 'l', 's', 'e'>();
            }
        }

        void dump_null() {
            write_literal<'n', 'u', 'l', 'l'>();
        }

        void dump_integer(integer_type value) {
            if (value == 0) {
                out_->write(to_char_type('0'));
                return;
            }
            bool negative = value < 0;
            auto abs_value = negative ? -static_cast<std::uint64_t>(value) : static_cast<std::uint64_t>(value);
            char_type buffer[32]{};
            std::size_t pos = sizeof(buffer) / sizeof(buffer[0]);
            buffer[--pos] = '\0';
            while (abs_value > 0) {
                buffer[--pos] = to_char_type('0' + abs_value % 10);
                abs_value /= 10;
            }
            if (negative) {
                buffer[--pos] = to_char_type('-');
            }
            out_->write(buffer + pos, sizeof(buffer) / sizeof(buffer[0]) - pos - 1);
        }

        void dump_float(float_type value) {
            if (!std::isfinite(value)) {
                dump_null();
                return;
            }

            char_type buf[32]{};
            int len = 0;

            if (value < 0) {
                buf[len++] = '-';
                value = -value;
            }

            // 科学计数法阈值
            if ((value != 0.0f && (value >= 1e9 || value < 1e-6))) {
                int exp10 = static_cast<int>(std::floor(std::log10(value)));
                float_type mantissa = value / std::pow(10.0f, exp10);
                // 输出整数部分
                int int_part = static_cast<int>(mantissa);
                buf[len++] = '0' + int_part;
                mantissa -= int_part;
                // 输出小数部分，只保留有效数字
                if (mantissa > utility::numeric_limits<float_type>::epsilon()) {
                    buf[len++] = '.';
                    constexpr int max_digits = 9;
                    for (int i = 0; i < max_digits; ++i) {
                        mantissa *= 10;
                        int digit = static_cast<int>(mantissa);
                        if (digit != 0 || len > 2) { // 避免开头多余零
                            buf[len++] = '0' + digit;
                        }
                        mantissa -= digit;
                        if (mantissa < utility::numeric_limits<float_type>::epsilon())
                            break;
                    }
                    // 去掉尾部多余零
                    while (len > 0 && buf[len - 1] == '0') {
                        --len;
                    }
                }
                // 去掉可能多余的小数点
                if (len > 0 && buf[len - 1] == '.') {
                    --len;
                }
                // 输出指数
                buf[len++] = 'e';
                if (exp10 < 0) {
                    buf[len++] = '-';
                    exp10 = -exp10;
                } else {
                    buf[len++] = '+';
                }
                if (exp10 >= 100) {
                    buf[len++] = '0' + (exp10 / 100);
                    exp10 %= 100;
                }
                buf[len++] = '0' + (exp10 / 10);
                buf[len++] = '0' + (exp10 % 10);
                out_->write(buf, len);
                return;
            }
            // 普通浮点数处理（整数+小数）
            static constexpr uint64_t pow10_table[] = {1ULL,      10ULL,      100ULL,      1000ULL,      10000ULL,
                                                       100000ULL, 1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL};
            uint64_t integer_part = static_cast<uint64_t>(value);
            float_type frac_part = value - static_cast<float_type>(integer_part);

            char_type int_buf[20]{};
            int int_len = 0;
            do {
                int_buf[int_len++] = '0' + (integer_part % 10);
                integer_part /= 10;
            } while (integer_part > 0);

            for (int i = int_len - 1; i >= 0; --i) {
                buf[len++] = int_buf[i];
            }

            if (frac_part < utility::numeric_limits<float_type>::epsilon()) {
                out_->write(buf, len);
                return;
            }

            constexpr int max_digits = 9;
            uint64_t scaled = static_cast<uint64_t>(std::round(frac_part * pow10_table[max_digits]));
            int digits_to_keep = max_digits;
            for (; digits_to_keep > 0; --digits_to_keep) {
                uint64_t divisor = pow10_table[max_digits - digits_to_keep + 1];
                uint64_t test_scaled = scaled / divisor;
                float_type test_value = static_cast<float_type>(test_scaled) / pow10_table[digits_to_keep - 1];
                float_type recomposed = std::floor(value) + test_value;
                if (std::fabs(value - recomposed) > utility::numeric_limits<float_type>::epsilon() * 10) {
                    break;
                }
            }

            digits_to_keep = (core::max)(1, digits_to_keep);
            buf[len++] = '.';
            uint64_t divisor = pow10_table[max_digits - digits_to_keep];
            scaled /= divisor;
            for (int i = digits_to_keep - 1; i >= 0; --i) {
                buf[len + i] = '0' + (scaled % 10);
                scaled /= 10;
            }
            len += digits_to_keep;
            // 去掉尾零
            while (len > 0 && buf[len - 1] == '0') {
                --len;
            }
            if (len > 0 && buf[len - 1] == '.') {
                --len;
            }
            out_->write(buf, len);
        }

        void write_indent(unsigned int indent_level) {
            if (indent_level > 0 && indent_level <= indent_string_.size()) {
                out_->write(indent_string_.data(), indent_level);
            }
        }

        template <char_type... ch>
        void write_literal() {
            static constexpr collections::array<char_type, sizeof...(ch)> literal = {ch...};
            out_->write(std::data(literal), sizeof...(ch));
        }

        static char_type to_char_type(char_type c) {
            return char_traits::to_char_type(static_cast<char_int_type>(c));
        }

    private:
        adapters::output_adapter<char_type> out_;
        const args &arg_;
        char_type indent_char_;
        string_type indent_string_;
        std::vector<char_type> string_buffer_;
    };
}

#endif