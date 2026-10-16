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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_LEXER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_LEXER_HPP
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/simd.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/xml/config.hpp>
#include <rainy/foundation/willow/implements/xml/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::xml::implements {
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
     * @brief The xml lexer, turning a character stream into xml tokens.
     *
     * The lexer resolves every `<` form on its own: a start tag carries its name, its attributes and whether
     * it is self closing, an end tag carries its name, and character data is delivered entity decoded. Comments,
     * processing instructions and the doctype are recognised so the parser can step over them.
     *
     * Block scanning is reserved for runs that are plausibly long: character data, comment bodies and cdata
     * sections. Tag names, attribute names and attribute values stay scalar because they are short enough that
     * a block compare costs more than it saves.
     *
     * @tparam BasicDocument The document type driving the character and string types
     * @tparam Adapter The input adapter
     *
     * \lang simp-chinese
     * @brief xml 词法分析器，把字符流转换为 xml 记号。
     *
     * 词法分析器自行解析每种 `<` 形式：起始标签携带名称、属性以及是否自闭合，结束标签携带名称，字符数据在
     * 交付前完成实体解码。注释、处理指令与 doctype 都会被识别，以便语法分析器跳过。
     *
     * 块扫描只用于可能很长的串：字符数据、注释体与 cdata 段。标签名、属性名与属性值保持标量，因为它们足够短，
     * 块比较的开销大于收益。
     *
     * @tparam BasicDocument 决定字符与字符串类型的文档类型
     * @tparam Adapter 输入适配器
     */
    template <typename BasicDocument, typename Adapter>
    struct xml_lexer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using simd_char_type = core::vec<char_type>;
        using attribute_type = xml_attribute<string_type>;
        using attribute_list = core::collections::vector<attribute_type>;
        using declaration_type = xml_declaration<string_type>;

        static constexpr bool contiguous_adapter = is_contiguous_adapter<Adapter>::value;

        explicit xml_lexer(Adapter adapter) : adapter_(adapter) {
            name_buffer_.reserve(64);
            text_buffer_.reserve(256);
            attribute_buffer_.reserve(64);
            attribute_value_buffer_.reserve(64);
            read_next();
        }

        char_int_type read_next() {
            current = adapter_.get_char();
            return current;
        }

        /**
         * \lang english
         * @brief Scans the next token.
         * @return The scanned token
         *
         * \lang simp-chinese
         * @brief 扫描下一个记号。
         * @return 扫描到的记号
         */
        token_type scan() {
            if (current == char_traits::eof()) {
                return token_type::end_of_input;
            }
            if (current == '<') {
                return scan_markup();
            }
            return scan_text();
        }

        string_type token_to_name() const {
            return name_buffer_;
        }

        string_type token_to_text() const {
            return text_buffer_;
        }

        const attribute_list &token_to_attributes() const {
            return attributes_;
        }

        bool token_is_self_closing() const noexcept {
            return self_closing_;
        }

        bool token_is_cdata() const noexcept {
            return cdata_;
        }

        const declaration_type &token_to_declaration() const noexcept {
            return declaration_;
        }

        /**
         * \lang english
         * @brief Broadcasts a character into a simd vector.
         *
         * @note The character is converted to char_type before the broadcast because basic_vec only accepts
         * scalars whose conversion to its value type is value preserving.
         *
         * \lang simp-chinese
         * @brief 把字符广播进 simd 向量。
         *
         * @note 广播前先转换为 char_type，因为 basic_vec 只接受到其值类型保持值不变的标量。
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
         * The scan stays scalar until a full simd block has been consumed without a stop, so a short run never
         * pays for the block compare; only a run at least one block long switches to simd, whose scalar tail
         * covers the remainder.
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

    private:
        static constexpr bool is_space(const char_int_type ch) noexcept {
            return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
        }

        static constexpr bool is_name_start(const char_int_type ch) noexcept {
            return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_' || ch == ':' || ch >= 0x80;
        }

        static constexpr bool is_name_char(const char_int_type ch) noexcept {
            return is_name_start(ch) || (ch >= '0' && ch <= '9') || ch == '-' || ch == '.';
        }

        static int digit_value(const char_int_type ch) noexcept {
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

        void skip_spaces() {
            while (is_space(current)) {
                read_next();
            }
        }

        token_type scan_markup() {
            read_next();
            if (current == '/') {
                read_next();
                return scan_element_end();
            }
            if (current == '!') {
                read_next();
                return scan_bang();
            }
            if (current == '?') {
                read_next();
                return scan_processing();
            }
            return scan_element_open();
        }

        token_type scan_element_end() {
            if (!is_name_start(current)) {
                exceptions::willow::xml::throw_xml_parse_error("expected an element name after '</'");
            }
            read_name();
            skip_spaces();
            if (current != '>') {
                exceptions::willow::xml::throw_xml_parse_error("expected '>' at the end of an end tag");
            }
            read_next();
            return token_type::element_end;
        }

        token_type scan_bang() {
            if (current == '-') {
                read_next();
                if (current != '-') {
                    exceptions::willow::xml::throw_xml_parse_error("expected '<!--'");
                }
                read_next();
                scan_comment_body();
                return token_type::comment;
            }
            if (current == '[') {
                read_next();
                for (const char_type expected: {char_type('C'), char_type('D'), char_type('A'), char_type('T'), char_type('A'), char_type('[')}) {
                    if (char_traits::to_char_type(current) != expected) {
                        exceptions::willow::xml::throw_xml_parse_error("expected '<![CDATA['");
                    }
                    read_next();
                }
                return scan_cdata_body();
            }
            if (is_name_start(current)) {
                read_name();
                if (equals_literal(name_buffer_, "DOCTYPE")) {
                    return scan_doctype();
                }
            }
            exceptions::willow::xml::throw_xml_parse_error("unsupported markup declaration");
        }

        static bool equals_literal(const string_type &value, const char *const literal) {
            std::size_t i = 0;
            for (; literal[i] != '\0'; ++i) {
                if (i >= value.size() || value[i] != static_cast<char_type>(literal[i])) {
                    return false;
                }
            }
            return i == value.size();
        }

        token_type scan_processing() {
            read_name();
            if (equals_literal(name_buffer_, "xml") && (is_space(current) || current == '?')) {
                return scan_declaration();
            }
            skip_until_sequence(char_type('?'), char_type('>'));
            return token_type::processing_instruction;
        }

        token_type scan_declaration() {
            declaration_ = declaration_type{};
            declaration_.present = true;
            skip_spaces();
            while (current != char_traits::eof() && current != '?') {
                attribute_buffer_.clear();
                attribute_value_buffer_.clear();
                read_name_into(attribute_buffer_);
                if (attribute_buffer_.empty()) {
                    exceptions::willow::xml::throw_xml_parse_error("expected an attribute name in the xml declaration");
                }
                skip_spaces();
                if (current != '=') {
                    exceptions::willow::xml::throw_xml_parse_error("expected '=' in the xml declaration");
                }
                read_next();
                skip_spaces();
                scan_attribute_value();
                if (attribute_buffer_ == to_string("version")) {
                    declaration_.version = attribute_value_buffer_;
                } else if (attribute_buffer_ == to_string("encoding")) {
                    declaration_.encoding = attribute_value_buffer_;
                } else if (attribute_buffer_ == to_string("standalone")) {
                    declaration_.standalone = attribute_value_buffer_;
                } else {
                    exceptions::willow::xml::throw_xml_parse_error("unexpected attribute in the xml declaration");
                }
                skip_spaces();
            }
            if (current != '?') {
                exceptions::willow::xml::throw_xml_parse_error("the xml declaration is not terminated by '?>'");
            }
            read_next();
            if (current != '>') {
                exceptions::willow::xml::throw_xml_parse_error("the xml declaration is not terminated by '?>'");
            }
            read_next();
            if (declaration_.version.empty()) {
                exceptions::willow::xml::throw_xml_parse_error("the xml declaration is missing a version");
            }
            return token_type::declaration;
        }

        token_type scan_doctype() {
            std::size_t depth = 0;
            while (true) {
                if (current == char_traits::eof()) {
                    exceptions::willow::xml::throw_xml_parse_error("the doctype is not terminated");
                }
                if (current == '[') {
                    depth = 1;
                    read_next();
                    break;
                }
                if (current == '>') {
                    read_next();
                    return token_type::doctype;
                }
                read_next();
            }
            while (current != char_traits::eof()) {
                if (current == '[') {
                    ++depth;
                } else if (current == ']') {
                    --depth;
                    if (depth == 0) {
                        read_next();
                        while (is_space(current)) {
                            read_next();
                        }
                        if (current != '>') {
                            exceptions::willow::xml::throw_xml_parse_error("expected '>' after the doctype internal subset");
                        }
                        read_next();
                        return token_type::doctype;
                    }
                }
                read_next();
            }
            exceptions::willow::xml::throw_xml_parse_error("the doctype is not terminated");
        }

        token_type scan_element_open() {
            if (!is_name_start(current)) {
                exceptions::willow::xml::throw_xml_parse_error("expected an element name after '<'");
            }
            read_name();
            attributes_.clear();
            self_closing_ = false;
            while (true) {
                skip_spaces();
                if (current == char_traits::eof()) {
                    exceptions::willow::xml::throw_xml_parse_error("the start tag is not terminated");
                }
                if (current == '>') {
                    read_next();
                    return token_type::element_open;
                }
                if (current == '/') {
                    read_next();
                    if (current != '>') {
                        exceptions::willow::xml::throw_xml_parse_error("expected '/>' to close a self closing tag");
                    }
                    read_next();
                    self_closing_ = true;
                    return token_type::element_open;
                }
                scan_attribute();
            }
        }

        void scan_attribute() {
            attribute_buffer_.clear();
            attribute_value_buffer_.clear();
            if (!is_name_start(current)) {
                exceptions::willow::xml::throw_xml_parse_error("expected an attribute name");
            }
            read_name_into(attribute_buffer_);
            skip_spaces();
            if (current != '=') {
                exceptions::willow::xml::throw_xml_parse_error("expected '=' after an attribute name");
            }
            read_next();
            skip_spaces();
            scan_attribute_value();
            for (const attribute_type &existing: attributes_) {
                if (existing.name == attribute_buffer_) {
                    exceptions::willow::xml::throw_xml_parse_error("duplicate attribute");
                }
            }
            attribute_type &attribute = attributes_.emplace_back();
            attribute.name = attribute_buffer_;
            attribute.value = attribute_value_buffer_;
        }

        void scan_attribute_value() {
            if (current != '\"' && current != '\'') {
                exceptions::willow::xml::throw_xml_parse_error("an attribute value must be quoted");
            }
            const char_int_type quote = current;
            read_next();
            while (true) {
                if (current == char_traits::eof()) {
                    exceptions::willow::xml::throw_xml_parse_error("the attribute value is not terminated");
                }
                if (current == quote) {
                    read_next();
                    return;
                }
                if (current == '<') {
                    exceptions::willow::xml::throw_xml_parse_error("'<' is not allowed in an attribute value");
                }
                if (current == '&') {
                    decode_entity_into(attribute_value_buffer_);
                    continue;
                }
                if (current == '\n' || current == '\t' || current == '\r') {
                    attribute_value_buffer_.push_back(char_type(' '));
                    read_next();
                    continue;
                }
                attribute_value_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
        }

        token_type scan_text() {
            text_buffer_.clear();
            cdata_ = false;
            if constexpr (contiguous_adapter) {
                if (can_block_scan()) {
                    return scan_text_span();
                }
            }
            while (current != char_traits::eof() && current != '<') {
                if (current == '&') {
                    decode_entity_into(text_buffer_);
                    continue;
                }
                text_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
            return token_type::text;
        }

        token_type scan_text_span() {
            const char_type *const last = buffer_end();
            const char_type *cursor = buffer_begin() + buffer_position() - 1;
            while (true) {
                const char_type *const run = cursor;
                cursor = scan_until(
                    cursor,
                    [](const simd_char_type &chunk) {
                        return (chunk == simd_char(char_type('<'))) | (chunk == simd_char(char_type('&')));
                    },
                    [](const char_type ch) { return ch == char_type('<') || ch == char_type('&'); });
                if (cursor != run) {
                    text_buffer_.append(run, static_cast<std::size_t>(cursor - run));
                }
                if (cursor == last) {
                    jump_to(last);
                    return token_type::text;
                }
                if (*cursor == char_type('<')) {
                    jump_to(cursor);
                    return token_type::text;
                }
                jump_to(cursor);
                decode_entity_into(text_buffer_);
                cursor = buffer_begin() + buffer_position() - 1;
            }
        }

        void scan_comment_body() {
            if constexpr (contiguous_adapter) {
                if (can_block_scan()) {
                    scan_comment_body_span();
                    return;
                }
            }
            while (true) {
                if (current == char_traits::eof()) {
                    exceptions::willow::xml::throw_xml_parse_error("the comment is not terminated");
                }
                if (current == '-') {
                    read_next();
                    if (current == '-') {
                        read_next();
                        if (current != '>') {
                            exceptions::willow::xml::throw_xml_parse_error("'--' is not allowed inside a comment");
                        }
                        read_next();
                        return;
                    }
                    continue;
                }
                read_next();
            }
        }

        void scan_comment_body_span() {
            const char_type *const last = buffer_end();
            const char_type *cursor = buffer_begin() + buffer_position() - 1;
            while (true) {
                cursor = scan_until(
                    cursor, [](const simd_char_type &chunk) { return chunk == simd_char(char_type('-')); },
                    [](const char_type ch) { return ch == char_type('-'); });
                if (cursor == last) {
                    jump_to(last);
                    exceptions::willow::xml::throw_xml_parse_error("the comment is not terminated");
                }
                if (cursor + 2 < last && *(cursor + 1) == char_type('-') && *(cursor + 2) == char_type('>')) {
                    jump_to(cursor + 3);
                    return;
                }
                if (cursor + 1 < last && *(cursor + 1) == char_type('-')) {
                    exceptions::willow::xml::throw_xml_parse_error("'--' is not allowed inside a comment");
                }
                cursor += 1;
            }
        }

        token_type scan_cdata_body() {
            text_buffer_.clear();
            cdata_ = true;
            if constexpr (contiguous_adapter) {
                if (can_block_scan()) {
                    scan_cdata_body_span();
                    return token_type::cdata;
                }
            }
            while (true) {
                if (current == char_traits::eof()) {
                    exceptions::willow::xml::throw_xml_parse_error("the cdata section is not terminated");
                }
                if (current == ']') {
                    read_next();
                    if (current == ']') {
                        read_next();
                        if (current == '>') {
                            read_next();
                            return token_type::cdata;
                        }
                        text_buffer_.push_back(char_type(']'));
                        text_buffer_.push_back(char_type(']'));
                        continue;
                    }
                    text_buffer_.push_back(char_type(']'));
                    continue;
                }
                text_buffer_.push_back(char_traits::to_char_type(current));
                read_next();
            }
        }

        void scan_cdata_body_span() {
            const char_type *const last = buffer_end();
            const char_type *cursor = buffer_begin() + buffer_position() - 1;
            while (true) {
                const char_type *const run = cursor;
                cursor = scan_until(
                    cursor, [](const simd_char_type &chunk) { return chunk == simd_char(char_type(']')); },
                    [](const char_type ch) { return ch == char_type(']'); });
                if (cursor != run) {
                    text_buffer_.append(run, static_cast<std::size_t>(cursor - run));
                }
                if (cursor == last) {
                    jump_to(last);
                    exceptions::willow::xml::throw_xml_parse_error("the cdata section is not terminated");
                }
                if (cursor + 2 < last && *(cursor + 1) == char_type(']') && *(cursor + 2) == char_type('>')) {
                    jump_to(cursor + 3);
                    return;
                }
                text_buffer_.push_back(char_type(']'));
                cursor += 1;
            }
        }

        void skip_until_sequence(const char_type first, const char_type second) {
            while (current != char_traits::eof()) {
                if (current == first) {
                    read_next();
                    if (current == second) {
                        read_next();
                        return;
                    }
                    continue;
                }
                read_next();
            }
            exceptions::willow::xml::throw_xml_parse_error("the processing instruction is not terminated");
        }

        void read_name() {
            name_buffer_.clear();
            read_name_into(name_buffer_);
            if (name_buffer_.empty()) {
                exceptions::willow::xml::throw_xml_parse_error("expected a name");
            }
        }

        void read_name_into(string_type &target) {
            while (is_name_char(current)) {
                target.push_back(char_traits::to_char_type(current));
                read_next();
            }
        }

        string_type to_string(const char *const narrow) const {
            string_type result;
            for (const char *p = narrow; *p != '\0'; ++p) {
                result.push_back(static_cast<char_type>(*p));
            }
            return result;
        }

        void decode_entity_into(string_type &target) {
            read_next();
            if (current == '#') {
                read_next();
                bool hexadecimal = false;
                if (current == 'x' || current == 'X') {
                    hexadecimal = true;
                    read_next();
                }
                std::uint32_t code = 0;
                bool any = false;
                while (current != char_traits::eof()) {
                    const int value = digit_value(current);
                    if (value < 0 || (!hexadecimal && value > 9)) {
                        break;
                    }
                    code = code * static_cast<std::uint32_t>(hexadecimal ? 16 : 10) + static_cast<std::uint32_t>(value);
                    any = true;
                    read_next();
                }
                if (!any) {
                    exceptions::willow::xml::throw_xml_parse_error("a character reference has no digits");
                }
                if (current != ';') {
                    exceptions::willow::xml::throw_xml_parse_error("a character reference is not terminated by ';'");
                }
                read_next();
                if (code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) {
                    exceptions::willow::xml::throw_xml_parse_error("a character reference is out of range");
                }
                unicode_writer<string_type> writer(target);
                writer.add_code(code);
                return;
            }
            string_type name;
            while (is_name_char(current)) {
                name.push_back(char_traits::to_char_type(current));
                read_next();
            }
            if (current != ';') {
                exceptions::willow::xml::throw_xml_parse_error("an entity reference is not terminated by ';'");
            }
            read_next();
            if (name == to_string("amp")) {
                target.push_back(char_type('&'));
            } else if (name == to_string("lt")) {
                target.push_back(char_type('<'));
            } else if (name == to_string("gt")) {
                target.push_back(char_type('>'));
            } else if (name == to_string("quot")) {
                target.push_back(char_type('\"'));
            } else if (name == to_string("apos")) {
                target.push_back(char_type('\''));
            } else {
                exceptions::willow::xml::throw_xml_parse_error("unknown entity reference");
            }
        }

    private:
        string_type name_buffer_;
        string_type text_buffer_;
        string_type attribute_buffer_;
        string_type attribute_value_buffer_;
        attribute_list attributes_;
        declaration_type declaration_{};
        Adapter adapter_;
        char_int_type current;
        bool self_closing_{false};
        bool cdata_{false};
    };
}

#endif
