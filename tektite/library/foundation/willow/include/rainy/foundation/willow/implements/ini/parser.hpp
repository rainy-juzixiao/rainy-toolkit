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
#ifndef RAINY_FOUNDATION_WILLOW_INI_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_INI_PARSER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/adapter.hpp>
#include <rainy/foundation/willow/implements/ini/config.hpp>
#include <rainy/foundation/willow/implements/ini/exceptions.hpp>
#include <rainy/foundation/willow/implements/ini/lexer.hpp>

namespace rainy::foundation::willow::ini::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument, typename Adapter>
    struct ini_parser {
        using string_type = typename BasicDocument::string_type;
        using object_type = typename BasicDocument::object_type;

        ini_parser(Adapter adapter) : lexer(adapter), last_token(token_type::uninitialized) {
        }

        void parse(BasicDocument &ini) {
            ini = document_type::object;
            object_type &root = ini.as_object();
            object_type *current = &root;
            token_type token = get_token();
            while (token != token_type::end_of_input) {
                if (token == token_type::section) {
                    const string_type name = lexer.token_to_section();
                    const auto top = root.find(name);
                    if (top != root.end() && !top->second.is_object()) {
                        exceptions::willow::ini::throw_ini_parse_error("section name conflicts with a top-level key");
                    }
                    auto placed = root.emplace(name, BasicDocument(document_type::object));
                    current = &placed.first->second.as_object();
                    token = get_token();
                    continue;
                }
                if (token != token_type::key_value) {
                    exceptions::willow::ini::throw_ini_parse_error("unexpected token");
                }
                string_type key = lexer.token_to_key();
                string_type value = lexer.token_to_value();
                auto placed = current->emplace(utility::move(key), BasicDocument(utility::move(value)));
                if (!placed.second) {
                    placed.first->second = BasicDocument(utility::move(value));
                }
                token = get_token();
            }
        }

    private:
        token_type get_token() {
            last_token = lexer.scan();
            return last_token;
        }

        ini_lexer<BasicDocument, Adapter> lexer;
        token_type last_token;
    };
}

#endif
