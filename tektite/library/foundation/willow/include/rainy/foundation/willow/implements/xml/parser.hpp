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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_XML_PARSER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/implements/xml/config.hpp>
#include <rainy/foundation/willow/implements/xml/exceptions.hpp>
#include <rainy/foundation/willow/implements/xml/lexer.hpp>

namespace rainy::foundation::willow::xml::implements {
    using namespace rainy::foundation::exceptions::willow;

    /**
     * \lang english
     * @brief The xml parser, filling a document from the token stream.
     *
     * The document floor has no place for an element name, so an element becomes an object keyed by its name
     * and repeats of that name become an array. A leaf element holds its character data directly, which keeps
     * `<port>8080</port>` addressable as `doc["port"].as_string()`; an element with child elements is an object,
     * and any character data it also holds lands under the reserved `#text` key. Names, attributes and the root
     * declaration ride on the node state, which is why the parser writes them through the xml facade rather than
     * the plain document.
     *
     * Comments, processing instructions and the doctype carry nothing the floor can express and are stepped over.
     *
     * @tparam BasicDocument The untagged document type
     * @tparam Adapter The input adapter
     *
     * \lang simp-chinese
     * @brief xml 语法分析器，从记号流填充文档。
     *
     * 文档地板没有元素名的位置，因此元素变为以其名称作键的对象，同名重复则变为数组。叶元素直接持有其字符数据，
     * 这使 `<port>8080</port>` 可以按 `doc["port"].as_string()` 访问；带子元素的元素是对象，其同时持有的字符数据
     * 落在保留键 `#text` 之下。名称、属性与根声明挂在节点状态上，这也是语法分析器经由 xml facade 而非普通文档
     * 写入它们的原因。
     *
     * 注释、处理指令与 doctype 不携带地板能表达的内容，会被跳过。
     *
     * @tparam BasicDocument 未打标签的文档类型
     * @tparam Adapter 输入适配器
     */
    template <typename BasicDocument, typename Adapter>
    struct xml_parser {
        using doc_type = xml_document<BasicDocument>;
        using node_type = typename doc_type::node_type;
        using string_type = typename doc_type::string_type;
        using char_type = typename doc_type::char_type;
        using object_type = typename doc_type::object_type;
        using array_type = typename doc_type::array_type;
        using attribute_type = typename doc_type::attribute_type;
        using attribute_list = typename doc_type::attribute_list;

        explicit xml_parser(Adapter adapter) : lexer(adapter), text_key_(to_string("#text")) {
        }

        void parse(doc_type &xml) {
            advance();
            skip_prolog_noise();
            if (current_token_ == token_type::declaration) {
                declaration_ = lexer.token_to_declaration();
                has_declaration_ = true;
                advance();
                skip_prolog_noise();
            }
            if (current_token_ != token_type::element_open) {
                exceptions::willow::xml::throw_xml_parse_error("expected a root element");
            }
            node_type root = parse_element();
            while (current_token_ == token_type::comment || current_token_ == token_type::processing_instruction ||
                   is_blank_text_token()) {
                advance();
            }
            if (current_token_ != token_type::end_of_input) {
                exceptions::willow::xml::throw_xml_parse_error("unexpected content after the root element");
            }
            object_type object;
            object.emplace(root.name(), utility::move(root));
            xml = doc_type(utility::move(object));
            if (has_declaration_) {
                xml.set_declaration(declaration_);
            }
        }

    private:
        void advance() {
            current_token_ = lexer.scan();
        }

        bool is_blank_text_token() const {
            if (current_token_ != token_type::text && current_token_ != token_type::cdata) {
                return false;
            }
            for (const char_type ch: lexer.token_to_text()) {
                if (ch != char_type(' ') && ch != char_type('\t') && ch != char_type('\n') && ch != char_type('\r')) {
                    return false;
                }
            }
            return true;
        }

        void skip_prolog_noise() {
            while (current_token_ == token_type::comment || current_token_ == token_type::processing_instruction ||
                   current_token_ == token_type::doctype || is_blank_text_token()) {
                advance();
            }
        }

        node_type parse_element() {
            const string_type element_name = lexer.token_to_name();
            const bool self_closing = lexer.token_is_self_closing();
            attribute_list attributes = lexer.token_to_attributes();
            advance();

            node_type element;
            if (self_closing) {
                element = node_type(document_type::null);
            } else {
                element = parse_element_body(element_name);
            }
            element.set_name(element_name);
            attribute_list &target = element.attributes();
            for (const attribute_type &attribute: attributes) {
                target.push_back(attribute);
            }
            return element;
        }

        node_type parse_element_body(const string_type &element_name) {
            object_type children;
            string_type text;
            bool has_child_element = false;
            bool text_is_cdata = false;

            while (true) {
                if (current_token_ == token_type::end_of_input) {
                    exceptions::willow::xml::throw_xml_parse_error("the element is not closed");
                }
                if (current_token_ == token_type::element_end) {
                    if (!(lexer.token_to_name() == element_name)) {
                        exceptions::willow::xml::throw_xml_parse_error("a closing tag does not match the open element");
                    }
                    advance();
                    break;
                }
                if (current_token_ == token_type::element_open) {
                    has_child_element = true;
                    node_type child = parse_element();
                    const string_type child_name = child.name();
                    insert_child(children, child_name, utility::move(child));
                    continue;
                }
                if (current_token_ == token_type::text || current_token_ == token_type::cdata) {
                    const string_type chunk = lexer.token_to_text();
                    if (current_token_ == token_type::cdata) {
                        text_is_cdata = true;
                    }
                    text += chunk;
                    advance();
                    continue;
                }
                advance();
            }

            if (has_child_element) {
                node_type element(utility::move(children));
                if (!is_blank_text(text)) {
                    node_type text_node(text);
                    if (text_is_cdata) {
                        text_node.mark_cdata();
                    }
                    element[text_key_] = utility::move(text_node);
                }
                return element;
            }
            if (text.empty()) {
                return node_type(document_type::null);
            }
            node_type element(text);
            if (text_is_cdata) {
                element.mark_cdata();
            }
            return element;
        }

        void insert_child(object_type &children, const string_type &child_name, node_type child) {
            auto iter = children.find(child_name);
            if (iter == children.end()) {
                children.emplace(child_name, utility::move(child));
                return;
            }
            if (iter->second.is_array()) {
                iter->second.as_array().push_back(utility::move(child));
                return;
            }
            array_type repeated;
            repeated.push_back(utility::move(iter->second));
            repeated.push_back(utility::move(child));
            iter->second = node_type(utility::move(repeated));
        }

        static bool is_blank_text(const string_type &text) {
            for (const char_type ch: text) {
                if (ch != char_type(' ') && ch != char_type('\t') && ch != char_type('\n') && ch != char_type('\r')) {
                    return false;
                }
            }
            return true;
        }

        static string_type to_string(const char *const narrow) {
            string_type result;
            for (const char *p = narrow; *p != '\0'; ++p) {
                result.push_back(static_cast<char_type>(*p));
            }
            return result;
        }

    private:
        xml_lexer<BasicDocument, Adapter> lexer;
        token_type current_token_{token_type::uninitialized};
        string_type text_key_;
        typename doc_type::declaration_type declaration_{};
        bool has_declaration_{false};
    };
}

#endif
