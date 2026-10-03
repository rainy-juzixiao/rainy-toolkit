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
#ifndef RAINY_FOUNDATION_WILLOW_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_PARSER_HPP
#include <iomanip>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/json/config.hpp>
#include <rainy/foundation/willow/implements/json/exceptions.hpp>
#include <rainy/foundation/willow/implements/json/lexer.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::json::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct json_parser {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using char_traits = core::text::char_traits<char_type>;

        json_parser(Adapter adapter, const char_type *span_data = nullptr, std::size_t span_size = 0)
            : lexer(adapter, span_data, span_size), last_token(token_type::uninitialized) {
        }

        void parse(BasicDocument &json) {
            json = parse_value();
            if (get_token() != token_type::end_of_input) {
                exceptions::willow::json::throw_json_parse_error("unexpected token, expect end");
            }
        }

    private:
        token_type get_token() {
            last_token = lexer.scan();
            return last_token;
        }

        BasicDocument parse_value(bool read_next = true) {
            token_type token = last_token;
            if (read_next) {
                token = get_token();
            }

            switch (token) {
                case token_type::literal_true:
                    return BasicDocument(true);

                case token_type::literal_false:
                    return BasicDocument(false);

                case token_type::literal_null:
                    return BasicDocument(document_type::null);

                case token_type::value_string:
                    return BasicDocument(lexer.token_to_string());

                case token_type::value_integer:
                    return BasicDocument(lexer.token_to_integer());

                case token_type::value_float:
                    return BasicDocument(lexer.token_to_float());

                case token_type::begin_array: {
                    BasicDocument result(document_type::array);
                    token = get_token();
                    if (token == token_type::end_array) {
                        return result;
                    }
                    array_type &array = result.as_array();
                    while (true) {
                        array.emplace_back(parse_value(false));

                        token = get_token();
                        if (token == token_type::end_array) {
                            break;
                        }
                        if (token != token_type::value_separator) {
                            exceptions::willow::json::throw_json_parse_error("unexpected token in array");
                        }
                        token = get_token();
                    }
                    return result;
                }
                case token_type::begin_object: {
                    BasicDocument result(document_type::object);
                    token = get_token();
                    if (token == token_type::end_object) {
                        return result;
                    }
                    object_type &object = result.as_object();
                    while (true) {
                        if (token != token_type::value_string) {
                            exceptions::willow::json::throw_json_parse_error("expected object key");
                        }
                        string_type key = lexer.token_to_string();

                        if (get_token() != token_type::name_separator) {
                            exceptions::willow::json::throw_json_parse_error("expected ':' in object");
                        }

                        object.emplace(utility::move(key), parse_value());

                        token = get_token();
                        if (token == token_type::end_object) {
                            break;
                        }
                        if (token != token_type::value_separator) {
                            exceptions::willow::json::throw_json_parse_error("unexpected token in object");
                        }
                        token = get_token();
                    }
                    return result;
                }
                default: {
                    exceptions::willow::json::throw_json_parse_error("unexpected token");
                }
            }
            return BasicDocument(document_type::null);
        }

        json_lexer<BasicDocument, Adapter> lexer;
        token_type last_token;
    };
}

#endif