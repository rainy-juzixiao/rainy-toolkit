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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_SERIALIZER_HPP
#include <cmath>
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/implements/xml/config.hpp>
#include <rainy/foundation/willow/implements/xml/exceptions.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::xml::implements {
    using namespace rainy::foundation::exceptions::willow;

    /**
     * \lang english
     * @brief The xml serializer, writing a document as xml text.
     *
     * The serializer is the inverse of the parser's mapping: a top level object entry becomes an element, an
     * object below it becomes its child elements, an array repeats the element named by its key, and the
     * reserved `#text` key holds the character data of an element that also has children. An element whose
     * value is a scalar holds that scalar as its character data.
     *
     * Attributes, the cdata authoring flag and the root declaration are read from the node state when the
     * document type carries it, so the same code serializes a plain document without either.
     *
     * @tparam BasicDocument The document type
     *
     * \lang simp-chinese
     * @brief xml 序列化器，把文档写成 xml 文本。
     *
     * 序列化器是语法分析器映射的逆：顶层对象项变为元素，其下的对象变为子元素，数组重复其键所指名的元素，
     * 保留键 `#text` 持有同时带子元素的元素的字符数据。取值为标量的元素把该标量作为其字符数据。
     *
     * 属性、cdata 撰写标记与根声明在文档类型携带时从节点状态读取，因此同一份代码也能序列化不具备二者的普通文档。
     *
     * @tparam BasicDocument 文档类型
     */
    template <typename BasicDocument>
    struct xml_serializer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using args = serializer_args<BasicDocument>;

        static constexpr std::size_t flush_threshold = 4096;

        xml_serializer(output_adapter<char_type> adapter, const args &arg) :
            out_(utility::move(adapter)), arg_(arg), text_key_(to_string("#text")) {
            out_buffer_.reserve(flush_threshold);
        }

        void dump(const BasicDocument &xml) {
            if (!xml.is_object()) {
                exceptions::willow::xml::throw_xml_serialize_error("an xml document must be an object");
            }
            const auto &object = xml.as_object();
            if constexpr (has_xml_declaration_v<BasicDocument>) {
                if (xml.has_xml_declaration()) {
                    dump_declaration(xml.declaration());
                }
            }
            for (const auto &entry: object) {
                dump_element(entry.first, entry.second, 0);
            }
            flush_buffer();
        }

    private:
        void dump_declaration(const xml_declaration<string_type> &declaration) {
            put_literal("<?xml");
            dump_declaration_attribute("version", declaration.version);
            if (!declaration.encoding.empty()) {
                dump_declaration_attribute("encoding", declaration.encoding);
            }
            if (!declaration.standalone.empty()) {
                dump_declaration_attribute("standalone", declaration.standalone);
            }
            put_literal("?>");
            newline();
        }

        void dump_declaration_attribute(const char *const name, const string_type &value) {
            put(to_char_type(' '));
            put_literal(name);
            put_literal("=\"");
            dump_escaped_attribute(value);
            put(to_char_type('\"'));
        }

        void dump_element(const string_type &name, const BasicDocument &node, const unsigned int depth) {
            indent(depth);
            put(to_char_type('<'));
            put(name.data(), name.size());
            dump_attributes(node);
            if (node.is_null()) {
                put_literal("/>");
                newline();
                return;
            }
            if (node.is_object()) {
                dump_object_body(name, node, depth);
                return;
            }
            put(to_char_type('>'));
            dump_scalar_content(node);
            put_literal("</");
            put(name.data(), name.size());
            put(to_char_type('>'));
            newline();
        }

        void dump_object_body(const string_type &name, const BasicDocument &node, const unsigned int depth) {
            const auto &object = node.as_object();
            bool has_child_element = false;
            for (const auto &entry: object) {
                if (!(entry.first == text_key_)) {
                    has_child_element = true;
                    break;
                }
            }
            put(to_char_type('>'));
            if (has_child_element) {
                newline();
                for (const auto &entry: object) {
                    if (entry.first == text_key_) {
                        continue;
                    }
                    dump_child(entry.first, entry.second, depth + 1);
                }
                indent(depth);
                auto text = object.find(text_key_);
                if (text != object.end()) {
                    dump_text_content(text->second);
                }
            } else {
                auto text = object.find(text_key_);
                if (text != object.end()) {
                    dump_text_content(text->second);
                }
            }
            put_literal("</");
            put(name.data(), name.size());
            put(to_char_type('>'));
            newline();
        }

        void dump_child(const string_type &name, const BasicDocument &node, const unsigned int depth) {
            if (node.is_array()) {
                for (const auto &element: node.as_array()) {
                    dump_element(name, element, depth);
                }
                return;
            }
            dump_element(name, node, depth);
        }

        void dump_attributes(const BasicDocument &node) {
            if constexpr (has_xml_attributes_v<BasicDocument>) {
                for (const auto &attribute: node.attributes()) {
                    put(to_char_type(' '));
                    put(attribute.name.data(), attribute.name.size());
                    put_literal("=\"");
                    dump_escaped_attribute(attribute.value);
                    put(to_char_type('\"'));
                }
            }
        }

        void dump_scalar_content(const BasicDocument &node) {
            if constexpr (has_xml_cdata_v<BasicDocument>) {
                if (node.is_cdata()) {
                    dump_cdata(node.as_string());
                    return;
                }
            }
            dump_escaped_text(node);
        }

        void dump_text_content(const BasicDocument &node) {
            if (node.is_string()) {
                dump_scalar_content(node);
                return;
            }
            dump_escaped_text(node);
        }

        void dump_escaped_text(const BasicDocument &node) {
            switch (node.type()) {
                case document_type::string:
                    dump_escaped_string(node.as_string());
                    break;
                case document_type::boolean:
                    put_literal(node.as_bool() ? "true" : "false");
                    break;
                case document_type::number_integer:
                    dump_integer(node.as_integer());
                    break;
                case document_type::number_float:
                    dump_float(node.as_float());
                    break;
                case document_type::null:
                    break;
                default:
                    exceptions::willow::xml::throw_xml_serialize_error("an element cannot hold an array as text");
            }
        }

        void dump_escaped_attribute(const string_type &str) {
            for (const char_type ch: str) {
                switch (static_cast<std::uint32_t>(to_unit(ch))) {
                    case '&':
                        put_literal("&amp;");
                        break;
                    case '<':
                        put_literal("&lt;");
                        break;
                    case '\"':
                        put_literal("&quot;");
                        break;
                    case '\n':
                        put_literal("&#10;");
                        break;
                    case '\r':
                        put_literal("&#13;");
                        break;
                    case '\t':
                        put_literal("&#9;");
                        break;
                    default:
                        put(ch);
                        break;
                }
            }
        }

        void dump_escaped_string(const string_type &str) {
            for (const char_type ch: str) {
                switch (static_cast<std::uint32_t>(to_unit(ch))) {
                    case '&':
                        put_literal("&amp;");
                        break;
                    case '<':
                        put_literal("&lt;");
                        break;
                    case '>':
                        put_literal("&gt;");
                        break;
                    case '\r':
                        put_literal("&#13;");
                        break;
                    default:
                        put(ch);
                        break;
                }
            }
        }

        void dump_cdata(const string_type &str) {
            put_literal("<![CDATA[");
            std::size_t begin = 0;
            std::size_t cursor = 0;
            while (cursor + 2 < str.size()) {
                if (str[cursor] == char_type(']') && str[cursor + 1] == char_type(']') && str[cursor + 2] == char_type('>')) {
                    put(str.data() + begin, cursor - begin);
                    put_literal("]]]]><![CDATA[");
                    cursor += 2;
                    begin = cursor;
                    continue;
                }
                ++cursor;
            }
            put(str.data() + begin, str.size() - begin);
            put_literal("]]>");
        }

        void dump_integer(const integer_type value) {
            char buffer[32]{};
            const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), static_cast<long long>(value));
            put_narrow(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }

        void dump_float(const float_type value) {
            if (std::isnan(value) || std::isinf(value)) {
                exceptions::willow::xml::throw_xml_serialize_error("xml has no spelling for a non finite number");
            }
            char buffer[64]{};
            const auto result =
                core::text::to_chars(buffer, buffer + sizeof(buffer), value, core::text::chars_format::general, arg_.precision);
            if (result.ec != std::errc{}) {
                exceptions::willow::xml::throw_xml_serialize_error("a float could not be formatted");
            }
            put_narrow(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }

        void indent(const unsigned int depth) {
            if (arg_.indent == 0) {
                return;
            }
            put(to_char_type('\n'));
            for (unsigned int i = 0; i < depth * arg_.indent; ++i) {
                put(arg_.indent_char);
            }
        }

        void newline() {
            if (arg_.indent != 0) {
                put(to_char_type('\n'));
            }
        }

        void put_narrow(const char *const str, const std::size_t size) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                put(str, size);
            } else {
                char_type text[64];
                const std::size_t count = size < 64 ? size : 64;
                for (std::size_t i = 0; i < count; ++i) {
                    text[i] = static_cast<char_type>(str[i]);
                }
                put(text, count);
            }
        }

        void put_literal(const char *const literal) {
            std::size_t size = 0;
            while (literal[size] != '\0') {
                ++size;
            }
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                put(literal, size);
            } else {
                char_type text[16];
                const std::size_t count = size < 16 ? size : 16;
                for (std::size_t i = 0; i < count; ++i) {
                    text[i] = static_cast<char_type>(literal[i]);
                }
                put(text, count);
            }
        }

        void put(const char_type ch) {
            out_buffer_.push_back(ch);
            if (out_buffer_.size() >= flush_threshold) {
                flush_buffer();
            }
        }

        void put(const char_type *str, const std::size_t size) {
            if (size == 0) {
                return;
            }
            if (size >= flush_threshold) {
                flush_buffer();
                out_->write(str, size);
                return;
            }
            if (out_buffer_.size() + size > flush_threshold) {
                flush_buffer();
            }
            out_buffer_.insert(out_buffer_.end(), str, str + size);
        }

        void flush_buffer() {
            if (!out_buffer_.empty()) {
                out_->write(out_buffer_.data(), out_buffer_.size());
                out_buffer_.clear();
            }
        }

        static type_traits::helper::make_unsigned_t<char_type> to_unit(const char_type ch) {
            return static_cast<type_traits::helper::make_unsigned_t<char_type>>(ch);
        }

        static char_type to_char_type(const char_type ch) {
            return char_traits::to_char_type(static_cast<char_int_type>(ch));
        }

        static string_type to_string(const char *const narrow) {
            string_type result;
            for (const char *p = narrow; *p != '\0'; ++p) {
                result.push_back(static_cast<char_type>(*p));
            }
            return result;
        }

    private:
        output_adapter<char_type> out_;
        const args &arg_;
        string_type text_key_;
        core::collections::vector<char_type> out_buffer_;
    };
}

#endif
