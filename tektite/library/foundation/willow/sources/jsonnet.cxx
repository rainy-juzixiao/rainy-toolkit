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
#include <rainy/foundation/willow/implements/jsonnet/ast.hpp>
#include <rainy/foundation/willow/implements/jsonnet/lexer.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    const char *token_name(const token_type type) noexcept {
        switch (type) {
            case token_type::uninitialized:
                return "uninitialized";
            case token_type::end_of_input:
                return "end_of_input";
            case token_type::identifier:
                return "identifier";
            case token_type::literal_null:
                return "null";
            case token_type::literal_true:
                return "true";
            case token_type::literal_false:
                return "false";
            case token_type::literal_number:
                return "number";
            case token_type::literal_string:
                return "string";
            case token_type::kw_assert:
                return "assert";
            case token_type::kw_else:
                return "else";
            case token_type::kw_error:
                return "error";
            case token_type::kw_for:
                return "for";
            case token_type::kw_function:
                return "function";
            case token_type::kw_if:
                return "if";
            case token_type::kw_import:
                return "import";
            case token_type::kw_importstr:
                return "importstr";
            case token_type::kw_importbin:
                return "importbin";
            case token_type::kw_in:
                return "in";
            case token_type::kw_local:
                return "local";
            case token_type::kw_tailstrict:
                return "tailstrict";
            case token_type::kw_then:
                return "then";
            case token_type::kw_self:
                return "self";
            case token_type::kw_super:
                return "super";
            case token_type::brace_left:
                return "{";
            case token_type::brace_right:
                return "}";
            case token_type::bracket_left:
                return "[";
            case token_type::bracket_right:
                return "]";
            case token_type::paren_left:
                return "(";
            case token_type::paren_right:
                return ")";
            case token_type::comma:
                return ",";
            case token_type::dot:
                return ".";
            case token_type::semicolon:
                return ";";
            case token_type::dollar:
                return "$";
            case token_type::colon:
                return ":";
            case token_type::double_colon:
                return "::";
            case token_type::triple_colon:
                return ":::";
            case token_type::op_plus:
                return "+";
            case token_type::op_minus:
                return "-";
            case token_type::op_star:
                return "*";
            case token_type::op_slash:
                return "/";
            case token_type::op_percent:
                return "%";
            case token_type::op_shift_left:
                return "<<";
            case token_type::op_shift_right:
                return ">>";
            case token_type::op_less:
                return "<";
            case token_type::op_less_equal:
                return "<=";
            case token_type::op_greater:
                return ">";
            case token_type::op_greater_equal:
                return ">=";
            case token_type::op_equal:
                return "==";
            case token_type::op_not_equal:
                return "!=";
            case token_type::op_ampersand:
                return "&";
            case token_type::op_caret:
                return "^";
            case token_type::op_pipe:
                return "|";
            case token_type::op_and:
                return "&&";
            case token_type::op_or:
                return "||";
            case token_type::op_not:
                return "!";
            case token_type::op_tilde:
                return "~";
        }
        return "unknown";
    }
}

namespace rainy::foundation::willow::jsonnet {
    const char *jsonnet_kind_name(const ast::node_kind kind) noexcept {
        switch (jsonnet_kind_of(kind)) {
            case jsonnet_node_kind::null_literal:
                return "null";
            case jsonnet_node_kind::boolean_literal:
                return "bool";
            case jsonnet_node_kind::number_literal:
                return "number";
            case jsonnet_node_kind::string_literal:
                return "string";
            case jsonnet_node_kind::identifier:
                return "identifier";
            case jsonnet_node_kind::self_reference:
                return "self";
            case jsonnet_node_kind::super_reference:
                return "super";
            case jsonnet_node_kind::dollar:
                return "dollar";
            case jsonnet_node_kind::unary:
                return "unary";
            case jsonnet_node_kind::binary:
                return "binary";
            case jsonnet_node_kind::conditional:
                return "conditional";
            case jsonnet_node_kind::field_access:
                return "field_access";
            case jsonnet_node_kind::index:
                return "index";
            case jsonnet_node_kind::slice:
                return "slice";
            case jsonnet_node_kind::call:
                return "call";
            case jsonnet_node_kind::argument:
                return "argument";
            case jsonnet_node_kind::named_argument:
                return "named_argument";
            case jsonnet_node_kind::function:
                return "function";
            case jsonnet_node_kind::parameter:
                return "parameter";
            case jsonnet_node_kind::object:
                return "object";
            case jsonnet_node_kind::object_field:
                return "field";
            case jsonnet_node_kind::object_comprehension:
                return "object_comprehension";
            case jsonnet_node_kind::array:
                return "array";
            case jsonnet_node_kind::array_comprehension:
                return "array_comprehension";
            case jsonnet_node_kind::local:
                return "local";
            case jsonnet_node_kind::bind:
                return "bind";
            case jsonnet_node_kind::assert:
                return "assert";
            case jsonnet_node_kind::error:
                return "error";
            case jsonnet_node_kind::import_expression:
                return "import";
            case jsonnet_node_kind::import_string:
                return "importstr";
            case jsonnet_node_kind::import_binary:
                return "importbin";
            case jsonnet_node_kind::comprehension_for:
                return "for";
            case jsonnet_node_kind::comprehension_if:
                return "if";
            default:
                return "unknown";
        }
    }
}

#endif
