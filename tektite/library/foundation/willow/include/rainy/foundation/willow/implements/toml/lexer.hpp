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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_LEXER_HPP
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/simd.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/foundation/willow/implements/toml/config.hpp>
#include <rainy/foundation/willow/implements/toml/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::toml::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename Adapter, typename = void>
    struct is_contiguous_adapter : type_traits::helper::false_type {};

    template <typename Adapter>
    struct is_contiguous_adapter<
        Adapter, type_traits::other_trans::void_t<decltype(utility::declval<const Adapter &>().data()),
                                                  decltype(utility::declval<const Adapter &>().size()),
                                                  decltype(utility::declval<const Adapter &>().position()),
                                                  decltype(utility::declval<Adapter &>().set_position(std::size_t{}))>>
        : type_traits::helper::true_type {};

    /**
     * \lang english
     * @brief The toml lexer, turning a character stream into toml tokens.
     *
     * The lexer stays context free about a bare run: `123` is a key at the start of a statement and an
     * integer after a key separator, so the parser selects the reading through scan_key and scan_value.
     * Newlines are significant and surface as their own token.
     *
     * @tparam BasicDocument The document type driving the character and number types
     * @tparam Adapter The input adapter
     *
     * \lang simp-chinese
     * @brief toml 词法分析器，把字符流转换为 toml 记号。
     *
     * 词法分析器对裸串保持上下文无关：`123` 在语句开头是键、在键分隔符之后是整数，因此由语法分析器通过 scan_key
     * 与 scan_value 选择解读方式。换行具有语义，单独成记号。
     *
     * @tparam BasicDocument 决定字符与数值类型的文档类型
     * @tparam Adapter 输入适配器
     */
    template <typename BasicDocument, typename Adapter>
    struct toml_lexer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using simd_char_type = core::vec<char_type>;

        static constexpr bool contiguous_adapter = is_contiguous_adapter<Adapter>::value;

        explicit toml_lexer(Adapter adapter) : adapter_(adapter) {
            string_buffer_.reserve(128);
            number_buffer_.reserve(64);
            read_next();
        }

        char_int_type read_next() {
            current = adapter_.get_char();
            return current;
        }

        /**
         * \lang english
         * @brief Broadcasts a character into a simd vector.
         *
         * @note The character is converted to char_type before the broadcast because basic_vec only accepts scalars
         * whose conversion to its value type is value preserving. Passing a plain char literal to a wchar_t vector
         * is rejected on MSVC, where char is signed and wchar_t is unsigned.
         *
         * \lang simp-chinese
         * @brief 把字符广播进 simd 向量。
         *
         * @note 广播前先转换为 char_type，因为 basic_vec 只接受到其值类型保持值不变的标量。在 MSVC 上把普通 char
         * 字面量传给 wchar_t 向量会被拒绝，那里 char 有符号而 wchar_t 无符号。
         */
        static constexpr simd_char_type simd_char(const char_type ch) noexcept {
            return simd_char_type{ch};
        }

        const char_type *buffer_begin() const noexcept {
            return adapter_.data();
        }

        const char_type *buffer_end() const noexcept {
            return adapter_.data() + adapter_.size();
        }

        std::size_t buffer_position() const noexcept {
            return adapter_.position();
        }

        bool can_block_scan() const noexcept {
            if constexpr (contiguous_adapter) {
                return current != char_traits::eof();
            }
            return false;
        }

        void jump_to(const char_type *target) noexcept {
            const auto offset = static_cast<std::size_t>(target - buffer_begin());
            if (offset < adapter_.size()) {
                current = char_traits::to_int_type(*target);
                adapter_.set_position(offset + 1);
            } else {
                current = char_traits::eof();
                adapter_.set_position(adapter_.size());
            }
        }

        /**
         * \lang english
         * @brief Scans forward for the first position matching a stop condition.
         *
         * The scan stays scalar until a full simd block has been consumed without a stop, so a short run
         * never pays for the block compare; only a run at least one block long switches to simd, whose
         * scalar tail covers the remainder.
         *
         * \lang simp-chinese
         * @brief 向前扫描第一个满足停止条件的位置。
         *
         * 在整整一个 simd 块都未命中停止条件之前保持标量，因此短串不会为块比较付出代价；只有长度达到一个块的串
         * 才切换到 simd，其余部分由标量尾部覆盖。
         */
        template <typename MaskFn, typename StopFn>
        const char_type *scan_until(const char_type *start, MaskFn mask_fn, StopFn stop_fn) const {
            constexpr core::simd_size_type width = simd_char_type::size();
            const char_type *const last = buffer_end();
            const char_type *cursor = start;
            if (static_cast<std::size_t>(last - start) < static_cast<std::size_t>(width)) {
                while (cursor != last && !stop_fn(*cursor)) {
                    ++cursor;
                }
                return cursor;
            }
            const char_type *const block_end = start + width;
            while (cursor != block_end && !stop_fn(*cursor)) {
                ++cursor;
            }
            if (cursor != block_end) {
                return cursor;
            }
            while (static_cast<std::size_t>(last - cursor) >= static_cast<std::size_t>(width)) {
                const auto chunk = core::unchecked_load<simd_char_type>(cursor, width);
                const auto stop = mask_fn(chunk);
                if (core::any_of(stop)) {
                    core::simd_size_type index = 0;
                    while (!stop[index]) {
                        ++index;
                    }
                    return cursor + index;
                }
                cursor += width;
            }
            while (cursor != last && !stop_fn(*cursor)) {
                ++cursor;
            }
            return cursor;
        }

        /**
         * \lang english
         * @brief Scans the next token, reading a bare run as a key.
         * @return The scanned token
         *
         * \lang simp-chinese
         * @brief 扫描下一个记号，把裸串读作键。
         * @return 扫描到的记号
         */
        token_type scan_key() {
            key_mode_ = true;
            return scan();
        }

        /**
         * \lang english
         * @brief Scans the next token, reading a bare run as a value.
         * @return The scanned token
         *
         * \lang simp-chinese
         * @brief 扫描下一个记号，把裸串读作值。
         * @return 扫描到的记号
         */
        token_type scan_value() {
            key_mode_ = false;
            return scan();
        }

        string_type token_to_string() const {
            return string_buffer_;
        }

        integer_type token_to_integer() const {
            return integer_value_;
        }

        float_type token_to_float() const {
            return float_value_;
        }

        boolean_type token_to_boolean() const {
            return boolean_value_;
        }

        datetime_fields token_to_datetime() const {
            return datetime_value_;
        }

    private:
        token_type scan() {
            skip_spaces_and_comments();
            switch (current) {
                case char_traits::eof():
                    return token_type::end_of_input;
                case '\n':
                case '\r':
                    consume_newline();
                    return token_type::newline;
                case '[':
                    read_next();
                    return token_type::left_bracket;
                case ']':
                    read_next();
                    return token_type::right_bracket;
                case '{':
                    read_next();
                    return token_type::left_brace;
                case '}':
                    read_next();
                    return token_type::right_brace;
                case '=':
                    read_next();
                    return token_type::key_separator;
                case ',':
                    read_next();
                    return token_type::value_separator;
                case '.':
                    read_next();
                    return token_type::key_dot;
                case '\"':
                    return scan_basic_string();
                case '\'':
                    return scan_literal_string();
                default:
                    break;
            }
            return scan_bare();
        }

        void skip_spaces_and_comments() {
            while (true) {
                if (current == ' ' || current == '\t') {
                    read_next();
                    continue;
                }
                if (current == '#') {
                    while (current != char_traits::eof() && !is_newline_char(current)) {
                        read_next();
                    }
                    continue;
                }
                break;
            }
        }

        void consume_newline() {
            if (current == '\r') {
                read_next();
                if (current == '\n') {
                    read_next();
                }
                return;
            }
            read_next();
        }

        static bool is_bare_key_char(const char_int_type ch) noexcept {
            return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-';
        }

        static bool is_value_bare_char(const char_int_type ch) noexcept {
            return is_bare_key_char(ch) || ch == '+' || ch == '.' || ch == ':';
        }

        static bool is_newline_char(const char_int_type ch) noexcept {
            return ch == '\n' || ch == '\r';
        }

        token_type scan_bare() {
            if (key_mode_) {
                string_buffer_.clear();
                while (current != char_traits::eof() && is_bare_key_char(current)) {
                    string_buffer_.push_back(char_traits::to_char_type(current));
                    read_next();
                }
                if (string_buffer_.empty()) {
                    exceptions::willow::toml::throw_toml_parse_error("unexpected character");
                }
                return token_type::key;
            }
            number_buffer_.clear();
            while (current != char_traits::eof() && !is_newline_char(current) &&
                   (is_value_bare_char(current) || current == ' ')) {
                number_buffer_.push_back(static_cast<char>(current));
                read_next();
            }
            while (!number_buffer_.empty() && number_buffer_.back() == ' ') {
                number_buffer_.pop_back();
            }
            if (number_buffer_.empty()) {
                exceptions::willow::toml::throw_toml_parse_error("unexpected character");
            }
            return classify_value();
        }

        token_type classify_value() {
            const char *const begin = number_buffer_.data();
            const std::size_t size = number_buffer_.size();
            if (is_literal_word(begin, size, "true") || is_literal_word(begin, size, "false")) {
                boolean_value_ = begin[0] == 't';
                return token_type::value_boolean;
            }
            if (try_scan_datetime()) {
                return token_type::value_datetime;
            }
            if (try_parse_float(begin, size)) {
                return token_type::value_float;
            }
            if (try_parse_integer(begin, size)) {
                return token_type::value_integer;
            }
            exceptions::willow::toml::throw_toml_parse_error("invalid value");
        }

        static bool is_literal_word(const char *str, const std::size_t size, const char *word) {
            std::size_t i = 0;
            for (; i < size && word[i] != '\0'; ++i) {
                if (str[i] != word[i]) {
                    return false;
                }
            }
            return i == size && word[i] == '\0';
        }

        static bool is_digit(const char ch) noexcept {
            return ch >= '0' && ch <= '9';
        }

        static int digit_value(const char ch) noexcept {
            if (ch >= '0' && ch <= '9') {
                return ch - '0';
            }
            if (ch >= 'a' && ch <= 'f') {
                return ch - 'a' + 10;
            }
            if (ch >= 'A' && ch <= 'F') {
                return ch - 'A' + 10;
            }
            return -1;
        }

        bool try_parse_float(const char *const str, const std::size_t size) {
            std::size_t i = 0;
            const bool negative = size > 0 && str[0] == '-';
            if (i < size && (str[i] == '+' || str[i] == '-')) {
                ++i;
            }
            const char *const signless = str + i;
            const std::size_t signless_size = size - i;
            if (is_literal_word(signless, signless_size, "inf") || is_literal_word(signless, signless_size, "nan")) {
                float_value_ = signless[0] == 'i' ? numeric_infinity() : numeric_nan();
                if (negative) {
                    float_value_ = -float_value_;
                }
                return true;
            }
            if (!validate_float_shape(str, size)) {
                return false;
            }
            char compact[64];
            std::size_t out = 0;
            for (std::size_t j = 0; j < size && out < sizeof(compact); ++j) {
                if (str[j] != '_') {
                    compact[out++] = str[j];
                }
            }
            const auto result = core::text::from_chars(compact, compact + out, float_value_);
            return result.ec == std::errc{} || result.ec == std::errc::result_out_of_range;
        }

        static bool validate_float_shape(const char *const str, const std::size_t size) {
            std::size_t i = 0;
            if (i < size && (str[i] == '+' || str[i] == '-')) {
                ++i;
            }
            bool int_digits = false;
            bool int_leading_zero = false;
            while (i < size && (is_digit(str[i]) || str[i] == '_')) {
                if (is_digit(str[i])) {
                    if (!int_digits && str[i] == '0') {
                        int_leading_zero = true;
                    }
                    int_digits = true;
                }
                ++i;
            }
            if (!int_digits) {
                return false;
            }
            if (int_leading_zero && i < size && is_digit(str[i])) {
                return false;
            }
            bool has_fraction_or_exponent = false;
            if (i < size && str[i] == '.') {
                has_fraction_or_exponent = true;
                ++i;
                bool frac_digits = false;
                while (i < size && (is_digit(str[i]) || str[i] == '_')) {
                    if (is_digit(str[i])) {
                        frac_digits = true;
                    }
                    ++i;
                }
                if (!frac_digits) {
                    return false;
                }
            }
            if (i < size && (str[i] == 'e' || str[i] == 'E')) {
                has_fraction_or_exponent = true;
                ++i;
                if (i < size && (str[i] == '+' || str[i] == '-')) {
                    ++i;
                }
                bool exp_digits = false;
                while (i < size && (is_digit(str[i]) || str[i] == '_')) {
                    if (is_digit(str[i])) {
                        exp_digits = true;
                    }
                    ++i;
                }
                if (!exp_digits) {
                    return false;
                }
            }
            return has_fraction_or_exponent && i == size;
        }

        bool try_parse_integer(const char *const str, const std::size_t size) {
            std::size_t i = 0;
            const bool has_sign = size > 0 && (str[0] == '+' || str[0] == '-');
            if (has_sign) {
                i = 1;
            }
            if (i >= size) {
                return false;
            }
            int base = 10;
            if (str[i] == '0' && i + 1 < size && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
                base = 16;
                i += 2;
            } else if (str[i] == '0' && i + 1 < size && (str[i + 1] == 'o' || str[i + 1] == 'O')) {
                base = 8;
                i += 2;
            } else if (str[i] == '0' && i + 1 < size && (str[i + 1] == 'b' || str[i + 1] == 'B')) {
                base = 2;
                i += 2;
            }
            if (base != 10 && has_sign) {
                return false;
            }
            const std::size_t digits_begin = i;
            bool has_digit = false;
            for (; i < size; ++i) {
                const char ch = str[i];
                if (ch == '_') {
                    continue;
                }
                const int value = digit_value(ch);
                if (value < 0 || value >= base) {
                    return false;
                }
                has_digit = true;
            }
            if (!has_digit) {
                return false;
            }
            if (base == 10) {
                std::size_t first = digits_begin;
                while (first < size && str[first] == '_') {
                    ++first;
                }
                if (first < size && str[first] == '0' && digits_count(str, digits_begin, size) > 1) {
                    exceptions::willow::toml::throw_toml_parse_error("leading zeros are not allowed");
                }
            }
            char compact[64];
            std::size_t out = 0;
            if (has_sign) {
                compact[out++] = str[0];
            }
            for (std::size_t j = digits_begin; j < size && out < sizeof(compact); ++j) {
                if (str[j] != '_') {
                    compact[out++] = str[j];
                }
            }
            integer_type value{};
            const auto result = core::text::from_chars(compact, compact + out, value, base);
            if (result.ec != std::errc{} || result.ptr != compact + out) {
                return false;
            }
            integer_value_ = value;
            return true;
        }

        static std::size_t digits_count(const char *const str, std::size_t begin, const std::size_t end) {
            std::size_t count = 0;
            for (std::size_t i = begin; i < end; ++i) {
                if (str[i] != '_') {
                    ++count;
                }
            }
            return count;
        }

        bool try_scan_datetime() {
            const char *const str = number_buffer_.data();
            const std::size_t size = number_buffer_.size();
            if (size >= 10 && is_digit(str[0]) && is_digit(str[1]) && is_digit(str[2]) && is_digit(str[3]) && str[4] == '-' &&
                is_digit(str[5]) && is_digit(str[6]) && str[7] == '-' && is_digit(str[8]) && is_digit(str[9])) {
                return parse_date_leading(str, size);
            }
            if (size >= 5 && is_digit(str[0]) && is_digit(str[1]) && str[2] == ':' && is_digit(str[3]) && is_digit(str[4])) {
                return parse_time_leading(str, size);
            }
            return false;
        }

        bool parse_date_leading(const char *const str, const std::size_t size) {
            datetime_value_ = datetime_fields{};
            datetime_value_.kind = datetime_kind::local_date;
            datetime_value_.year = decimal(str, 0, 4);
            datetime_value_.month = two_digits(str, 5);
            datetime_value_.day = two_digits(str, 8);
            std::size_t i = 10;
            if (i >= size) {
                validate_date();
                return true;
            }
            if (str[i] == 'T' || str[i] == 't' || str[i] == ' ') {
                datetime_value_.use_space_separator = str[i] == ' ';
                ++i;
            } else {
                return false;
            }
            if (!parse_time_at(str, size, i)) {
                return false;
            }
            datetime_value_.kind = datetime_value_.has_offset ? datetime_kind::offset_date_time : datetime_kind::local_date_time;
            validate_date();
            validate_time();
            return true;
        }

        bool parse_time_leading(const char *const str, const std::size_t size) {
            datetime_value_ = datetime_fields{};
            datetime_value_.kind = datetime_kind::local_time;
            if (!parse_time_at(str, size, 0)) {
                return false;
            }
            validate_time();
            return true;
        }

        bool parse_time_at(const char *const str, const std::size_t size, std::size_t i) {
            if (i + 5 > size || !is_digit(str[i]) || !is_digit(str[i + 1]) || str[i + 2] != ':' || !is_digit(str[i + 3]) ||
                !is_digit(str[i + 4])) {
                return false;
            }
            datetime_value_.hour = two_digits(str, i);
            datetime_value_.minute = two_digits(str, i + 3);
            i += 5;
            datetime_value_.second = 0;
            if (i < size && str[i] == ':') {
                if (i + 3 > size || !is_digit(str[i + 1]) || !is_digit(str[i + 2])) {
                    return false;
                }
                datetime_value_.second = two_digits(str, i + 1);
                i += 3;
            }
            if (i < size && str[i] == '.') {
                ++i;
                std::int32_t fraction = 0;
                std::int32_t scale = 100000000;
                std::size_t digits = 0;
                while (i < size && is_digit(str[i])) {
                    if (digits < 9) {
                        fraction += (str[i] - '0') * scale;
                        scale /= 10;
                    }
                    ++digits;
                    ++i;
                }
                if (digits == 0) {
                    return false;
                }
                datetime_value_.nanosecond = fraction;
            }
            if (i >= size) {
                return true;
            }
            if (str[i] == 'Z' || str[i] == 'z') {
                datetime_value_.has_offset = true;
                datetime_value_.offset_minutes = 0;
                datetime_value_.use_zulu = true;
                return i + 1 == size;
            }
            if (str[i] == '+' || str[i] == '-') {
                const bool negative = str[i] == '-';
                if (i + 6 > size || !is_digit(str[i + 1]) || !is_digit(str[i + 2]) || str[i + 3] != ':' || !is_digit(str[i + 4]) ||
                    !is_digit(str[i + 5])) {
                    return false;
                }
                const std::int32_t offset = two_digits(str, i + 1) * 60 + two_digits(str, i + 4);
                datetime_value_.has_offset = true;
                datetime_value_.offset_minutes = negative ? -offset : offset;
                return i + 6 == size;
            }
            return false;
        }

        void validate_date() const {
            if (datetime_value_.month < 1 || datetime_value_.month > 12 || datetime_value_.day < 1 || datetime_value_.day > 31) {
                exceptions::willow::toml::throw_toml_parse_error("date field out of range");
            }
        }

        void validate_time() const {
            if (datetime_value_.hour > 23 || datetime_value_.minute > 59 || datetime_value_.second > 60) {
                exceptions::willow::toml::throw_toml_parse_error("time field out of range");
            }
        }

        static std::int32_t two_digits(const char *const str, const std::size_t i) noexcept {
            return (str[i] - '0') * 10 + (str[i + 1] - '0');
        }

        static std::int32_t decimal(const char *const str, const std::size_t i, const std::size_t count) noexcept {
            std::int32_t value = 0;
            for (std::size_t j = 0; j < count; ++j) {
                value = value * 10 + (str[i + j] - '0');
            }
            return value;
        }

        static float_type numeric_infinity() noexcept {
            return (utility::numeric_limits<float_type>::infinity)();
        }

        static float_type numeric_nan() noexcept {
            return (utility::numeric_limits<float_type>::quiet_NaN)();
        }

        token_type scan_basic_string() {
            read_next();
            if (current == '\"') {
                read_next();
                if (current == '\"') {
                    read_next();
                    if (key_mode_) {
                        exceptions::willow::toml::throw_toml_parse_error("a multi line string cannot be a key");
                    }
                    return scan_multiline_string(true);
                }
                string_buffer_.clear();
                return key_mode_ ? token_type::key : token_type::value_string;
            }
            return scan_single_line_string(true);
        }

        token_type scan_literal_string() {
            read_next();
            if (current == '\'') {
                read_next();
                if (current == '\'') {
                    read_next();
                    if (key_mode_) {
                        exceptions::willow::toml::throw_toml_parse_error("a multi line string cannot be a key");
                    }
                    return scan_multiline_string(false);
                }
                string_buffer_.clear();
                return key_mode_ ? token_type::key : token_type::value_string;
            }
            return scan_single_line_string(false);
        }

        token_type scan_single_line_string(const bool escaped) {
            if constexpr (contiguous_adapter) {
                if (can_block_scan()) {
                    return scan_single_line_string_span(escaped);
                }
            }
            string_buffer_.clear();
            while (true) {
                if (current == char_traits::eof() || is_newline_char(current)) {
                    exceptions::willow::toml::throw_toml_parse_error("unterminated string");
                }
                if (escaped && current == '\"') {
                    read_next();
                    return key_mode_ ? token_type::key : token_type::value_string;
                }
                if (!escaped && current == '\'') {
                    read_next();
                    return key_mode_ ? token_type::key : token_type::value_string;
                }
                if (escaped && current == '\\') {
                    scan_escape();
                    continue;
                }
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
        }

        token_type scan_single_line_string_span(const bool escaped) {
            string_buffer_.clear();
            const char_type *const last = buffer_end();
            const char_type *cursor = buffer_begin() + buffer_position() - 1;
            const char_type quote = escaped ? char_type('\"') : char_type('\'');
            while (true) {
                const char_type *const run = cursor;
                cursor = scan_until(
                    cursor,
                    [escaped](const simd_char_type &chunk) {
                        auto stop = (chunk == simd_char('\n')) | (chunk == simd_char('\r')) | (chunk == simd_char('\''));
                        if (escaped) {
                            stop = stop | (chunk == simd_char('\"')) | (chunk == simd_char('\\'));
                        }
                        return stop;
                    },
                    [escaped](const char_type ch) {
                        if (ch == char_type('\n') || ch == char_type('\r') || ch == char_type('\'')) {
                            return true;
                        }
                        return escaped && (ch == char_type('\"') || ch == char_type('\\'));
                    });
                if (cursor != run) {
                    string_buffer_.append(run, static_cast<std::size_t>(cursor - run));
                }
                if (cursor == last) {
                    exceptions::willow::toml::throw_toml_parse_error("unterminated string");
                }
                const char_type ch = *cursor;
                if (ch == char_type('\n') || ch == char_type('\r')) {
                    exceptions::willow::toml::throw_toml_parse_error("unterminated string");
                }
                if (ch == quote) {
                    jump_to(cursor + 1);
                    return key_mode_ ? token_type::key : token_type::value_string;
                }
                if (escaped && ch == char_type('\\')) {
                    jump_to(cursor);
                    scan_escape();
                    cursor = buffer_begin() + buffer_position() - 1;
                    continue;
                }
                string_buffer_.push_back(ch);
                ++cursor;
            }
        }

        token_type scan_multiline_string(const bool escaped) {
            if constexpr (contiguous_adapter) {
                if (can_block_scan()) {
                    return scan_multiline_string_span(escaped);
                }
            }
            string_buffer_.clear();
            if (is_newline_char(current)) {
                consume_newline();
            }
            const char_type quote = escaped ? char_type('\"') : char_type('\'');
            while (true) {
                if (current == char_traits::eof()) {
                    exceptions::willow::toml::throw_toml_parse_error("unterminated multi line string");
                }
                if (current == quote) {
                    std::size_t count = 0;
                    while (current == quote) {
                        ++count;
                        read_next();
                    }
                    if (count < 3) {
                        for (std::size_t i = 0; i < count; ++i) {
                            string_buffer_.push_back(quote);
                        }
                        continue;
                    }
                    if (count > 5) {
                        exceptions::willow::toml::throw_toml_parse_error("too many quotes at the end of a multi line string");
                    }
                    for (std::size_t i = 0; i < count - 3; ++i) {
                        string_buffer_.push_back(quote);
                    }
                    return key_mode_ ? token_type::key : token_type::value_string;
                }
                if (escaped && current == '\\') {
                    if (scan_line_ending_backslash()) {
                        continue;
                    }
                    scan_escape();
                    continue;
                }
                if (is_newline_char(current)) {
                    consume_newline();
                    string_buffer_.push_back(char_type('\n'));
                    continue;
                }
                string_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
        }

        token_type scan_multiline_string_span(const bool escaped) {
            string_buffer_.clear();
            const char_type quote = escaped ? char_type('\"') : char_type('\'');
            const char_type *cursor = buffer_begin() + buffer_position() - 1;
            if (*cursor == char_type('\n') || *cursor == char_type('\r')) {
                cursor = skip_one_newline(cursor);
            }
            const char_type *const last = buffer_end();
            while (true) {
                const char_type *const run = cursor;
                cursor = scan_until(
                    cursor,
                    [escaped, quote](const simd_char_type &chunk) {
                        auto stop = (chunk == simd_char('\n')) | (chunk == simd_char('\r')) | (chunk == simd_char(quote));
                        if (escaped) {
                            stop = stop | (chunk == simd_char('\\'));
                        }
                        return stop;
                    },
                    [escaped, quote](const char_type ch) {
                        if (ch == char_type('\n') || ch == char_type('\r') || ch == quote) {
                            return true;
                        }
                        return escaped && ch == char_type('\\');
                    });
                if (cursor != run) {
                    string_buffer_.append(run, static_cast<std::size_t>(cursor - run));
                }
                if (cursor == last) {
                    exceptions::willow::toml::throw_toml_parse_error("unterminated multi line string");
                }
                const char_type ch = *cursor;
                if (ch == char_type('\n') || ch == char_type('\r')) {
                    cursor = skip_one_newline(cursor);
                    string_buffer_.push_back(char_type('\n'));
                    continue;
                }
                if (ch == quote) {
                    const char_type *after = cursor;
                    std::size_t count = 0;
                    while (after != last && *after == quote) {
                        ++count;
                        ++after;
                    }
                    if (count < 3) {
                        for (std::size_t i = 0; i < count; ++i) {
                            string_buffer_.push_back(quote);
                        }
                        cursor = after;
                        continue;
                    }
                    if (count > 5) {
                        exceptions::willow::toml::throw_toml_parse_error("too many quotes at the end of a multi line string");
                    }
                    for (std::size_t i = 0; i < count - 3; ++i) {
                        string_buffer_.push_back(quote);
                    }
                    jump_to(after);
                    return key_mode_ ? token_type::key : token_type::value_string;
                }
                jump_to(cursor);
                if (scan_line_ending_backslash()) {
                    cursor = buffer_begin() + buffer_position() - 1;
                    continue;
                }
                scan_escape();
                cursor = buffer_begin() + buffer_position() - 1;
            }
        }

        const char_type *skip_one_newline(const char_type *cursor) noexcept {
            const char_type *const last = buffer_end();
            if (*cursor == char_type('\r')) {
                ++cursor;
                if (cursor != last && *cursor == char_type('\n')) {
                    ++cursor;
                }
            } else {
                ++cursor;
            }
            return cursor;
        }

        bool scan_line_ending_backslash() {
            read_next();
            if (current == ' ' || current == '\t') {
                while (current == ' ' || current == '\t') {
                    read_next();
                }
                if (!is_newline_char(current)) {
                    return false;
                }
            } else if (!is_newline_char(current)) {
                return false;
            }
            while (is_newline_char(current) || current == ' ' || current == '\t') {
                if (is_newline_char(current)) {
                    consume_newline();
                } else {
                    read_next();
                }
            }
            return true;
        }

        void scan_escape() {
            read_next();
            switch (current) {
                case '\"':
                    string_buffer_.push_back(char_type('\"'));
                    read_next();
                    break;
                case '\\':
                    string_buffer_.push_back(char_type('\\'));
                    read_next();
                    break;
                case 'b':
                    string_buffer_.push_back(char_type('\b'));
                    read_next();
                    break;
                case 't':
                    string_buffer_.push_back(char_type('\t'));
                    read_next();
                    break;
                case 'n':
                    string_buffer_.push_back(char_type('\n'));
                    read_next();
                    break;
                case 'f':
                    string_buffer_.push_back(char_type('\f'));
                    read_next();
                    break;
                case 'r':
                    string_buffer_.push_back(char_type('\r'));
                    read_next();
                    break;
                case 'u':
                    read_next();
                    add_code(read_hex(4));
                    break;
                case 'U':
                    read_next();
                    add_code(read_hex(8));
                    break;
                default:
                    exceptions::willow::toml::throw_toml_parse_error("invalid escape sequence");
            }
        }

        std::uint32_t read_hex(const int width) {
            std::uint32_t code = 0;
            for (int i = 0; i < width; ++i) {
                const int value = digit_value(static_cast<char>(current));
                if (value < 0) {
                    exceptions::willow::toml::throw_toml_parse_error("invalid unicode escape");
                }
                code = (code << 4) | static_cast<std::uint32_t>(value);
                read_next();
            }
            if (code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) {
                exceptions::willow::toml::throw_toml_parse_error("invalid unicode code point");
            }
            return code;
        }

        void add_code(const std::uint32_t code) {
            unicode_writer<string_type> writer(string_buffer_);
            writer.add_code(code);
        }

    private:
        string_type string_buffer_;
        core::collections::vector<char> number_buffer_;
        integer_type integer_value_{};
        float_type float_value_{};
        boolean_type boolean_value_{};
        datetime_fields datetime_value_{};
        Adapter adapter_;
        char_int_type current;
        bool key_mode_{false};
    };
}

#endif
