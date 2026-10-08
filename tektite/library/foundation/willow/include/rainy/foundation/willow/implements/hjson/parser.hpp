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
#ifndef RAINY_FOUNDATION_WILLOW_HJSON_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_HJSON_PARSER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/implements/hjson/config.hpp>
#include <rainy/foundation/willow/implements/hjson/exceptions.hpp>
#include <rainy/foundation/willow/implements/hjson/lexer.hpp>

namespace rainy::foundation::willow::hjson::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct hjson_parser {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;

        hjson_parser(Adapter adapter, const char_type *span_data = nullptr, std::size_t span_size = 0) :
            lexer(adapter, span_data, span_size), last_token(token_type::uninitialized) {
        }

        void parse(BasicDocument &hjson) {
            const token_type probe = lexer.probe_root();
            last_token = probe;
            if (probe == token_type::value_string && lexer.probe_sees_colon()) {
                parse_object(hjson, false);
            } else {
                token_type token = probe;
                if (probe == token_type::value_string) {
                    token = lexer.reclassify_root_value();
                    last_token = token;
                }
                if (token == token_type::begin_object) {
                    parse_object(hjson, true);
                } else if (token == token_type::begin_array) {
                    parse_value(hjson, false);
                } else if (token == token_type::end_of_input) {
                    hjson = document_type::object;
                } else if (token == token_type::value_string || token == token_type::value_integer ||
                           token == token_type::value_float || token == token_type::literal_true ||
                           token == token_type::literal_false || token == token_type::literal_null) {
                    parse_value(hjson, false);
                } else {
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected token at root");
                }
            }
            if (get_token() != token_type::end_of_input) {
                exceptions::willow::hjson::throw_hjson_parse_error("unexpected token, expect end");
            }
        }

    private:
        token_type get_token() {
            last_token = lexer.scan();
            return last_token;
        }

        token_type get_key_token() {
            lexer.set_key_mode(true);
            last_token = lexer.scan();
            lexer.set_key_mode(false);
            return last_token;
        }

        void parse_object(BasicDocument &hjson, const bool braced) {
            hjson = document_type::object;
            token_type token;
            if (braced) {
                token = get_key_token();
            } else {
                token = last_token;
            }
            if (braced && token == token_type::end_object) {
                return;
            }
            if (!braced && token == token_type::end_of_input) {
                return;
            }
            while (true) {
                if (token == token_type::value_separator) {
                    token = get_key_token();
                    if (braced && token == token_type::end_object) {
                        return;
                    }
                    if (!braced && token == token_type::end_of_input) {
                        return;
                    }
                }
                if (token != token_type::value_string) {
                    exceptions::willow::hjson::throw_hjson_parse_error("expected object key");
                }
                string_type key = lexer.token_to_string();
                if (get_token() != token_type::name_separator) {
                    exceptions::willow::hjson::throw_hjson_parse_error("expected ':' in object");
                }
                BasicDocument value;
                parse_value(value);
                auto &container = hjson.as_object();
                auto existing = container.find(key);
                if (existing != container.end()) {
                    existing->second = utility::move(value);
                } else {
                    container.emplace(utility::move(key), utility::move(value));
                }
                token = get_key_token();
                if (braced && token == token_type::end_object) {
                    return;
                }
                if (!braced && token == token_type::end_of_input) {
                    return;
                }
                if (token != token_type::value_separator && token != token_type::value_string) {
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected token in object");
                }
            }
        }

        void parse_array(BasicDocument &hjson) {
            hjson = document_type::array;
            token_type token = get_token();
            if (token == token_type::end_array) {
                return;
            }
            while (true) {
                hjson.as_array().emplace_back(BasicDocument());
                parse_value(hjson.as_array().back(), false);
                token = get_token();
                if (token == token_type::end_array) {
                    return;
                }
                if (token == token_type::value_separator) {
                    token = get_token();
                    if (token == token_type::end_array) {
                        return;
                    }
                }
            }
        }

        void parse_value(BasicDocument &hjson, const bool read_next = true) {
            token_type token = last_token;
            if (read_next) {
                token = get_token();
            }
            switch (token) {
                case token_type::literal_true:
                    hjson = true;
                    break;
                case token_type::literal_false:
                    hjson = false;
                    break;
                case token_type::literal_null:
                    hjson = document_type::null;
                    break;
                case token_type::value_string:
                    hjson = lexer.token_to_string();
                    break;
                case token_type::value_integer:
                    hjson = lexer.token_to_integer();
                    break;
                case token_type::value_float:
                    hjson = lexer.token_to_float();
                    break;
                case token_type::begin_array:
                    parse_array(hjson);
                    break;
                case token_type::begin_object:
                    parse_object(hjson, true);
                    break;
                default:
                    exceptions::willow::hjson::throw_hjson_parse_error("unexpected token");
            }
        }

        hjson_lexer<BasicDocument, Adapter> lexer;
        token_type last_token;
    };
}

#endif
