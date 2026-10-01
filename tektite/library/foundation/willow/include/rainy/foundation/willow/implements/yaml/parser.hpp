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
#ifndef RAINY_FOUNDATION_WILLOW_YAML_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_YAML_PARSER_HPP
#include <iomanip>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/implements/yaml/config.hpp>
#include <rainy/foundation/willow/implements/yaml/exceptions.hpp>
#include <rainy/foundation/willow/implements/yaml/lexer.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::yaml::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct yaml_parser {
        using facade_type = facade<BasicDocument>;
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;

        yaml_parser(Adapter adapter) : lexer(adapter), last_token(token_type::uninitialized) {
        }

        void parse(facade_type &yaml) {
            get_token();
            parse_document(yaml);
            if (last_token == token_type::document_end) {
                get_token();
            }
            if (last_token != token_type::end_of_input) {
                exceptions::willow::yaml::throw_yaml_parse_error("unexpected token, expect end");
            }
        }

    private:
        token_type get_token() {
            if (has_lookahead) {
                has_lookahead = false;
                last_token = lookahead_token;
                return last_token;
            }
            last_token = lexer.scan();
            return last_token;
        }

        void parse_document(facade_type &yaml) {
            parse_node(yaml, -1, /*in_flow=*/false, /*in_block=*/false);
        }

        struct depth_guard {
            explicit depth_guard(int &counter) : counter_(counter) {
                if (counter_ >= max_nesting_depth) {
                    exceptions::willow::yaml::throw_yaml_parse_error("document nesting is too deep");
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

        void parse_node(facade_type &yaml, const int parent_indent, const bool in_flow, const bool in_block) {
            const depth_guard guard(nesting_depth_);
            switch (last_token) {
                case token_type::literal_true:
                    if (!in_flow && lexer.next_is_key_separator()) {
                        parse_block_mapping(yaml, parent_indent);
                        return;
                    }
                    yaml = true;
                    get_token();
                    return;
                case token_type::literal_false:
                    if (!in_flow && lexer.next_is_key_separator()) {
                        parse_block_mapping(yaml, parent_indent);
                        return;
                    }
                    yaml = false;
                    get_token();
                    return;
                case token_type::literal_null:
                case token_type::literal_null_tilde:
                    if (!in_flow && lexer.next_is_key_separator()) {
                        parse_block_mapping(yaml, parent_indent);
                        return;
                    }
                    yaml = document_type::null;
                    get_token();
                    return;
                case token_type::value_string: {
                    if (!in_flow && lexer.next_is_key_separator()) {
                        parse_block_mapping(yaml, parent_indent);
                        return;
                    }
                    yaml = lexer.token_to_string();
                    get_token();
                    return;
                }
                case token_type::value_integer: {
                    const integer_type value = lexer.token_to_integer();
                    if (!in_flow && lexer.next_is_key_separator()) {
                        parse_block_mapping(yaml, parent_indent);
                        return;
                    }
                    yaml = value;
                    get_token();
                    return;
                }
                case token_type::value_float: {
                    const float_type value = lexer.token_to_float();
                    if (!in_flow && lexer.next_is_key_separator()) {
                        parse_block_mapping(yaml, parent_indent);
                        return;
                    }
                    yaml = value;
                    get_token();
                    return;
                }
                case token_type::anchor_indicator:
                    parse_anchor(yaml, parent_indent, in_flow, in_block);
                    return;
                case token_type::alias_indicator:
                    parse_alias(yaml);
                    return;
                case token_type::tag_indicator:
                    parse_tag(yaml, parent_indent, in_flow, in_block);
                    return;
                case token_type::block_scalar_literal:
                    parse_block_scalar(yaml, true, parent_indent);
                    return;
                case token_type::block_scalar_folded:
                    parse_block_scalar(yaml, false, parent_indent);
                    return;
                case token_type::begin_flow_sequence:
                    parse_flow_sequence(yaml);
                    return;
                case token_type::begin_flow_mapping:
                    parse_flow_mapping(yaml);
                    return;
                case token_type::block_entry_indicator:
                    parse_block_sequence(yaml, parent_indent);
                    return;
                case token_type::document_start:
                    get_token();
                    parse_node(yaml, parent_indent, in_flow, in_block);
                    return;
                case token_type::document_end:
                case token_type::end_of_input:
                    yaml = document_type::null;
                    return;
                case token_type::explicit_key:
                    parse_block_mapping(yaml, parent_indent);
                    return;
                case token_type::directive_indicator:
                    yaml.directive() = lexer.token_to_string();
                    get_token();
                    parse_node(yaml, parent_indent, in_flow, in_block);
                    return;
                default:
                    parse_implicit(yaml, parent_indent, in_flow, in_block);
                    return;
            }
        }

        void parse_implicit(facade_type &yaml, const int parent_indent, const bool in_flow, const bool in_block) {
            if (in_flow) {
                yaml = document_type::null;
                return;
            }
            parse_block_mapping(yaml, parent_indent);
        }

        void parse_anchor(facade_type &yaml, const int parent_indent, const bool in_flow, const bool in_block) {
            const string_type anchor = lexer.token_to_anchor();
            anchor_table_.erase(anchor);
            get_token();
            parse_node(yaml, parent_indent, in_flow, in_block);
            yaml.anchor() = anchor;
            anchor_table_[anchor] = yaml;
        }

        void parse_alias(facade_type &yaml) {
            const string_type alias = lexer.token_to_string();
            const auto it = anchor_table_.find(alias);
            if (it == anchor_table_.end()) {
                exceptions::willow::yaml::throw_yaml_alias_error("unknown alias");
            }
            yaml = it->second;
            yaml.alias() = alias;
            get_token();
        }

        void parse_tag(facade_type &yaml, const int parent_indent, const bool in_flow, const bool in_block) {
            const string_type tag = lexer.token_to_string();
            get_token();
            parse_node(yaml, parent_indent, in_flow, in_block);
            yaml.tag() = tag;
        }

        void parse_block_scalar(facade_type &yaml, const bool literal, const int parent_indent) {
            const string_type raw = lexer.read_block_scalar(literal);
            yaml = raw;
            yaml.set_style(literal, "literal", "folded");
            get_token();
        }

        void parse_flow_sequence(facade_type &yaml) {
            yaml = document_type::array;
            lexer.enter_flow();
            get_token();
            if (last_token == token_type::end_flow_sequence) {
                lexer.leave_flow();
                get_token();
                return;
            }
            while (true) {
                facade_type item;
                parse_node(item, -1, true, false);
                yaml.as_array().emplace_back(utility::move(item));
                if (last_token == token_type::end_flow_sequence) {
                    lexer.leave_flow();
                    get_token();
                    break;
                }
                if (last_token != token_type::value_separator) {
                    exceptions::willow::yaml::throw_yaml_parse_error("unexpected token in flow sequence");
                }
                get_token();
            }
            yaml.set_style("flow");
        }

        static string_type key_from_integer(const integer_type value) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                return std::to_string(value);
            } else {
                char buffer[32]{};
                const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), value);
                if (result.ec != std::errc{}) {
                    return string_type();
                }
                const std::size_t length = static_cast<std::size_t>(result.ptr - buffer);
                string_type key;
                for (std::size_t i = 0; i < length; ++i) {
                    key.push_back(static_cast<char_type>(buffer[i]));
                }
                return key;
            }
        }

        static string_type key_from_float(const float_type value) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                return std::to_string(value);
            } else {
                char buffer[64]{};
                const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), value);
                if (result.ec != std::errc{}) {
                    return string_type();
                }
                const std::size_t length = static_cast<std::size_t>(result.ptr - buffer);
                string_type key;
                for (std::size_t i = 0; i < length; ++i) {
                    key.push_back(static_cast<char_type>(buffer[i]));
                }
                return key;
            }
        }

        static string_type document_key(const facade_type &key_doc) {
            if (key_doc.is_string()) {
                return key_doc.as_string();
            }
            if (key_doc.is_integer()) {
                return key_from_integer(key_doc.as_integer());
            }
            if (key_doc.is_float()) {
                return key_from_float(key_doc.as_float());
            }
            if (key_doc.is_bool()) {
                if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                    return key_doc.as_bool() ? string_type("true") : string_type("false");
                } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                    return key_doc.as_bool() ? string_type(L"true") : string_type(L"false");
                } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                    return key_doc.as_bool() ? string_type(u"true") : string_type(u"false");
                } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                    return key_doc.as_bool() ? string_type(U"true") : string_type(U"false");
                } else {
#if RAINY_HAS_CXX20
                    if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                        return key_doc.as_bool() ? string_type(u8"true") : string_type(u8"false");
                    }
#endif
                    return string_type();
                }
            }
            return string_type();
        }

        void parse_flow_mapping(facade_type &yaml) {
            yaml = document_type::object;
            lexer.enter_flow();
            get_token();
            if (last_token == token_type::end_flow_mapping) {
                lexer.leave_flow();
                get_token();
                return;
            }
            while (true) {
                facade_type key_doc;
                parse_node(key_doc, -1, true, false);
                string_type key = document_key(key_doc);
                if (last_token != token_type::key_separator) {
                    exceptions::willow::yaml::throw_yaml_parse_error("expected ':' in flow mapping");
                }
                get_token();
                facade_type value;
                parse_node(value, -1, true, false);
                yaml.as_object().emplace(utility::move(key), utility::move(value));
                if (last_token == token_type::end_flow_mapping) {
                    lexer.leave_flow();
                    get_token();
                    break;
                }
                if (last_token != token_type::value_separator) {
                    exceptions::willow::yaml::throw_yaml_parse_error("unexpected token in flow mapping");
                }
                get_token();
            }
            yaml.set_style("flow");
        }

        void parse_block_sequence(facade_type &yaml, const int parent_indent) {
            yaml = document_type::array;
            yaml.set_style("block");
            const int sequence_indent = lexer.current_line_indent();
            if (sequence_indent <= parent_indent) {
                return;
            }

            while (last_token == token_type::block_entry_indicator &&
                   lexer.current_line_indent() == sequence_indent) {
                get_token();
                facade_type item;
                parse_node(item, sequence_indent, false, true);
                yaml.as_array().emplace_back(utility::move(item));
                if (last_token == token_type::end_of_input) {
                    break;
                }
            }
        }

        void parse_block_mapping(facade_type &yaml, const int parent_indent) {
            yaml = document_type::object;
            yaml.set_style("block");

            const int mapping_indent = lexer.current_line_indent();

            while (true) {
                if (last_token == token_type::end_of_input || last_token == token_type::document_end ||
                    last_token == token_type::document_start) {
                    break;
                }
                if (last_token == token_type::block_entry_indicator) {
                    break;
                }
                if (lexer.current_line_indent() < mapping_indent) {
                    break;
                }

                string_type key;
                if (last_token == token_type::explicit_key) {
                    get_token();
                    facade_type key_doc;
                    parse_node(key_doc, parent_indent, false, true);
                    key = document_key(key_doc);
                } else if (last_token == token_type::value_string) {
                    key = lexer.token_to_string();
                    get_token();
                } else if (last_token == token_type::value_integer) {
                    key = key_from_integer(lexer.token_to_integer());
                    get_token();
                } else if (last_token == token_type::value_float) {
                    key = key_from_float(lexer.token_to_float());
                    get_token();
                } else if (last_token == token_type::literal_true) {
                    if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                        key = "true";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                        key = L"true";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                        key = u"true";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                        key = U"true";
                    } else {
#if RAINY_HAS_CXX20
                        if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                            key = u8"true";
                        }
#endif
                    }
                    get_token();
                } else if (last_token == token_type::literal_false) {
                    if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                        key = "false";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                        key = L"false";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                        key = u"false";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                        key = U"false";
                    } else {
#if RAINY_HAS_CXX20
                        if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                            key = u8"false";
                        }
#endif
                    }
                    get_token();
                } else if (last_token == token_type::literal_null || last_token == token_type::literal_null_tilde) {
                    if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                        key = "null";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                        key = L"null";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                        key = u"null";
                    } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                        key = U"null";
                    } else {
#if RAINY_HAS_CXX20
                        if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                            key = u8"null";
                        }
#endif
                    }
                    get_token();
                } else {
                    exceptions::willow::yaml::throw_yaml_parse_error("expected mapping key");
                }

                if (last_token != token_type::key_separator) {
                    exceptions::willow::yaml::throw_yaml_parse_error("expected ':' after mapping key");
                }
                get_token();

                facade_type value;
                if (last_token == token_type::end_of_input || last_token == token_type::document_end ||
                    last_token == token_type::document_start) {
                    value = document_type::null;
                } else {
                    parse_node(value, parent_indent, false, true);
                }
                yaml.as_object().emplace(utility::move(key), utility::move(value));

                if (last_token == token_type::end_of_input) {
                    break;
                }
            }
        }

        static constexpr int max_nesting_depth = 256;

        yaml_lexer<BasicDocument, Adapter> lexer;
        token_type last_token;
        token_type lookahead_token{token_type::uninitialized};
        bool has_lookahead{false};
        bool parsing_explicit_key{false};
        int nesting_depth_{0};
        collections::unordered_map<string_type, facade_type> anchor_table_;
    };
}

#endif
