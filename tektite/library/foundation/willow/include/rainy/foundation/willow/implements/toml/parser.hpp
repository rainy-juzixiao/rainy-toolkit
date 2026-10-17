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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_PARSER_HPP
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/implements/toml/config.hpp>
#include <rainy/foundation/willow/implements/toml/exceptions.hpp>
#include <rainy/foundation/willow/implements/toml/lexer.hpp>

namespace rainy::foundation::willow::toml::implements {
    using namespace rainy::foundation::exceptions::willow;

    /**
     * \lang english
     * @brief The toml parser, filling a document from the token stream.
     *
     * Toml is table oriented, so the parser carries the table it is writing into. A key value line
     * assigns into the current table, a `[a.b]` header opens a table and an `[[a.b]]` header appends a
     * table to an array. The redefinition rules are enforced through the flags the document node carries:
     * a table opened by an explicit header cannot be opened again, and a table produced by an inline
     * table cannot be extended.
     *
     * @tparam BasicDocument The untagged document type
     * @tparam Adapter The input adapter
     *
     * \lang simp-chinese
     * @brief toml 语法分析器，从记号流填充文档。
     *
     * toml 以表为中心，因此语法分析器携带当前写入的表。键值行写入当前表，`[a.b]` 表头开启一张表，
     * `[[a.b]]` 表头向数组追加一张表。重复定义规则通过文档节点携带的标志执行：由显式表头开启的表不可再次开启，
     * 由内联表产生的表不可扩展。
     *
     * @tparam BasicDocument 未打标签的文档类型
     * @tparam Adapter 输入适配器
     */
    template <typename BasicDocument, typename Adapter>
    struct toml_parser {
        using doc_type = toml_document<BasicDocument>;
        using node_type = typename doc_type::node_type;
        using string_type = typename doc_type::string_type;
        using char_type = typename doc_type::char_type;
        using object_type = typename doc_type::object_type;
        using array_type = typename doc_type::array_type;

        explicit toml_parser(Adapter adapter) : lexer(adapter) {
            key_parts_.reserve(8);
        }

        void parse(doc_type &toml) {
            toml = doc_type(document_type::object);
            root_ = &toml.as_object();
            current_ = root_;
            advance_key();
            while (current_token_ != token_type::end_of_input) {
                if (current_token_ == token_type::newline) {
                    advance_key();
                    continue;
                }
                if (current_token_ == token_type::left_bracket) {
                    parse_header();
                } else if (current_token_ == token_type::key) {
                    parse_key_value();
                } else {
                    exceptions::willow::toml::throw_toml_parse_error("expected a key or a table header");
                }
                expect_statement_end();
            }
        }

    private:
        struct depth_guard {
            explicit depth_guard(int &counter) : counter_(counter) {
                if (counter_ >= static_cast<int>(willow::implements::max_document_nesting_depth)) {
                    exceptions::willow::toml::throw_toml_parse_error("document nesting is too deep");
                }
                ++counter_;
            }
            ~depth_guard() {
                --counter_;
            }
            depth_guard(const depth_guard &) = delete;
            depth_guard &operator=(const depth_guard &) = delete;

        private:
            int &counter_;
        };

        void advance_key() {
            current_token_ = lexer.scan_key();
        }

        void advance_value() {
            current_token_ = lexer.scan_value();
        }

        void expect_statement_end() {
            if (current_token_ == token_type::end_of_input) {
                return;
            }
            if (current_token_ != token_type::newline) {
                exceptions::willow::toml::throw_toml_parse_error("expected the end of the line");
            }
            advance_key();
        }

        void parse_header() {
            advance_key();
            const bool array_of_tables = current_token_ == token_type::left_bracket;
            if (array_of_tables) {
                advance_key();
            }
            parse_key_parts();
            if (current_token_ != token_type::right_bracket) {
                exceptions::willow::toml::throw_toml_parse_error("expected ']' in a table header");
            }
            advance_key();
            if (array_of_tables) {
                if (current_token_ != token_type::right_bracket) {
                    exceptions::willow::toml::throw_toml_parse_error("expected ']]' in an array of tables header");
                }
                advance_key();
                open_array_table();
            } else {
                open_table();
            }
        }

        void open_table() {
            object_type *table = root_;
            for (std::size_t i = 0; i + 1 < key_parts_.size(); ++i) {
                table = descend_or_create_object(*table, key_parts_[i]);
            }
            const string_type &name = key_parts_.back();
            auto iter = table->find(name);
            if (iter == table->end()) {
                auto placed = table->emplace(name, doc_type(document_type::object));
                placed.first->second.mark_explicit_table();
                current_ = &placed.first->second.as_object();
                return;
            }
            node_type &node = iter->second;
            if (node.is_inline_table()) {
                exceptions::willow::toml::throw_toml_parse_error("cannot redefine an inline table as a table");
            }
            if (node.is_array_table()) {
                exceptions::willow::toml::throw_toml_parse_error("a table name conflicts with an array of tables");
            }
            if (!node.is_object()) {
                exceptions::willow::toml::throw_toml_parse_error("a table name conflicts with an existing value");
            }
            if (node.is_explicit_table()) {
                exceptions::willow::toml::throw_toml_parse_error("the table was already defined");
            }
            node.mark_explicit_table();
            current_ = &node.as_object();
        }

        void open_array_table() {
            object_type *table = root_;
            for (std::size_t i = 0; i + 1 < key_parts_.size(); ++i) {
                table = descend_or_create_object(*table, key_parts_[i]);
            }
            const string_type &name = key_parts_.back();
            auto iter = table->find(name);
            if (iter == table->end()) {
                auto placed = table->emplace(name, doc_type(document_type::array));
                placed.first->second.mark_array_table();
                iter = placed.first;
            } else {
                node_type &node = iter->second;
                if (node.is_inline_table()) {
                    exceptions::willow::toml::throw_toml_parse_error("cannot append to an inline table");
                }
                if (!node.is_array() || !node.is_array_table()) {
                    exceptions::willow::toml::throw_toml_parse_error("an array of tables conflicts with an existing value");
                }
            }
            array_type &array = iter->second.as_array();
            array.emplace_back(document_type::object);
            current_ = &array.back().as_object();
        }

        void parse_key_value() {
            parse_key_parts();
            if (current_token_ != token_type::key_separator) {
                exceptions::willow::toml::throw_toml_parse_error("expected '=' after a key");
            }
            core::collections::vector<string_type> parts = key_parts_;
            advance_value();
            node_type value = parse_value();
            assign_into_table(*current_, parts, utility::move(value));
        }

        void assign_into_table(object_type &target, const core::collections::vector<string_type> &parts, node_type value) {
            object_type *table = &target;
            for (std::size_t i = 0; i + 1 < parts.size(); ++i) {
                table = descend_or_create_object(*table, parts[i]);
            }
            const string_type &name = parts.back();
            if (table->find(name) != table->end()) {
                exceptions::willow::toml::throw_toml_parse_error("the key was already defined");
            }
            table->emplace(name, utility::move(value));
        }

        object_type *descend_or_create_object(object_type &table, const string_type &name) {
            auto iter = table.find(name);
            if (iter != table.end()) {
                node_type &node = iter->second;
                if (node.is_inline_table()) {
                    exceptions::willow::toml::throw_toml_parse_error("cannot extend an inline table");
                }
                if (!node.is_object()) {
                    exceptions::willow::toml::throw_toml_parse_error("a key conflicts with an existing value");
                }
                return &node.as_object();
            }
            auto placed = table.emplace(name, doc_type(document_type::object));
            return &placed.first->second.as_object();
        }

        void parse_key_parts() {
            key_parts_.clear();
            if (current_token_ != token_type::key) {
                exceptions::willow::toml::throw_toml_parse_error("expected a key");
            }
            key_parts_.push_back(lexer.token_to_string());
            advance_key();
            while (current_token_ == token_type::key_dot) {
                advance_key();
                if (current_token_ != token_type::key) {
                    exceptions::willow::toml::throw_toml_parse_error("expected a key after '.'");
                }
                key_parts_.push_back(lexer.token_to_string());
                advance_key();
            }
        }

        node_type parse_value() {
            const depth_guard guard(nesting_depth_);
            switch (current_token_) {
                case token_type::value_string: {
                    node_type value(lexer.token_to_string());
                    advance_value();
                    return value;
                }
                case token_type::value_integer: {
                    node_type value(lexer.token_to_integer());
                    advance_value();
                    return value;
                }
                case token_type::value_float: {
                    node_type value(lexer.token_to_float());
                    advance_value();
                    return value;
                }
                case token_type::value_boolean: {
                    node_type value(lexer.token_to_boolean());
                    advance_value();
                    return value;
                }
                case token_type::value_datetime: {
                    node_type value(document_type::null);
                    value.set_datetime(lexer.token_to_datetime());
                    advance_value();
                    return value;
                }
                case token_type::left_bracket:
                    return parse_array();
                case token_type::left_brace:
                    return parse_inline_table();
                default:
                    exceptions::willow::toml::throw_toml_parse_error("expected a value");
            }
        }

        node_type parse_array() {
            node_type value(document_type::array);
            array_type &array = value.as_array();
            advance_value();
            while (true) {
                skip_newlines();
                if (current_token_ == token_type::right_bracket) {
                    break;
                }
                array.push_back(parse_value());
                skip_newlines();
                if (current_token_ == token_type::value_separator) {
                    advance_value();
                    continue;
                }
                if (current_token_ == token_type::right_bracket) {
                    break;
                }
                exceptions::willow::toml::throw_toml_parse_error("expected ',' or ']' in an array");
            }
            advance_value();
            return value;
        }

        node_type parse_inline_table() {
            object_type object;
            advance_key();
            if (current_token_ == token_type::right_brace) {
                advance_value();
                node_type value(object);
                value.mark_inline_table();
                return value;
            }
            while (true) {
                if (current_token_ != token_type::key) {
                    exceptions::willow::toml::throw_toml_parse_error("expected a key in an inline table");
                }
                parse_key_parts();
                if (current_token_ != token_type::key_separator) {
                    exceptions::willow::toml::throw_toml_parse_error("expected '=' in an inline table");
                }
                core::collections::vector<string_type> parts = key_parts_;
                advance_value();
                node_type member = parse_value();
                assign_into_table(object, parts, utility::move(member));
                if (current_token_ == token_type::value_separator) {
                    advance_key();
                    continue;
                }
                if (current_token_ == token_type::right_brace) {
                    break;
                }
                exceptions::willow::toml::throw_toml_parse_error("expected ',' or '}' in an inline table");
            }
            advance_value();
            node_type value(utility::move(object));
            value.mark_inline_table();
            return value;
        }

        void skip_newlines() {
            while (current_token_ == token_type::newline) {
                advance_value();
            }
        }

    private:
        toml_lexer<BasicDocument, Adapter> lexer;
        token_type current_token_{token_type::uninitialized};
        object_type *root_{nullptr};
        object_type *current_{nullptr};
        core::collections::vector<string_type> key_parts_;
        int nesting_depth_{0};
    };
}

#endif
