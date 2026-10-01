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
#ifndef RAINY_FOUNDATION_WILLOW_JSON5_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_JSON5_PARSER_HPP
#include <iomanip>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/json5/config.hpp>
#include <rainy/foundation/willow/implements/json5/exceptions.hpp>
#include <rainy/foundation/willow/implements/json5/lexer.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::json5::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct json5_parser {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;

        json5_parser(Adapter adapter) : lexer(adapter), last_token(token_type::uninitialized) {
        }

        void parse(BasicDocument &json) {
            parse_value(json);
            if (get_token() != token_type::end_of_input) {
                exceptions::willow::json5::throw_json5_parse_error("unexpected token, expect end");
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

        void parse_value(BasicDocument &json, bool read_next = true) {
            token_type token = last_token;
            if (read_next) {
                token = get_token();
            }

            switch (token) {
                case token_type::literal_true:
                    json = true;
                    break;

                case token_type::literal_false:
                    json = false;
                    break;

                case token_type::literal_null:
                    json = document_type::null;
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

                case token_type::begin_array: {
                    json = document_type::array;
                    token_type token = get_token();
                    if (token == token_type::end_array) {
                        break;
                    }
                    while (true) {
                        json.as_array().emplace_back(BasicDocument());
                        parse_value(json.as_array().back(), false);

                        token = get_token();
                        if (token == token_type::end_array) {
                            break;
                        }
                        if (token != token_type::value_separator) {
                            exceptions::willow::json5::throw_json5_parse_error("unexpected token in array");
                        }
                        token = get_token();
                        if (token == token_type::end_array) {
                            break;
                        }
                    }
                    break;
                }
                case token_type::begin_object: {
                    json = document_type::object;
                    token_type token = get_key_token();
                    if (token == token_type::end_object) {
                        break;
                    }
                    while (true) {
                        if (token != token_type::value_string) {
                            exceptions::willow::json5::throw_json5_parse_error("expected object key");
                        }
                        string_type key = lexer.token_to_string();

                        if (get_token() != token_type::name_separator) {
                            exceptions::willow::json5::throw_json5_parse_error("expected ':' in object");
                        }

                        BasicDocument object;
                        parse_value(object);
                        json.as_object().emplace(utility::move(key), utility::move(object));

                        token = get_token();
                        if (token == token_type::end_object) {
                            break;
                        }
                        if (token != token_type::value_separator) {
                            exceptions::willow::json5::throw_json5_parse_error("unexpected token in object");
                        }
                        token = get_key_token();
                        if (token == token_type::end_object) {
                            break;
                        }
                    }
                    break;
                }
                default: {
                    exceptions::willow::json5::throw_json5_parse_error("unexpected token");
                }
            }
        }

        json5_lexer<BasicDocument, Adapter> lexer;
        token_type last_token;
    };
}

#endif
