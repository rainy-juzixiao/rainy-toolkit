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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_PARSER_HPP
#include <cstddef>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/jsonnet/ast.hpp>
#include <rainy/foundation/willow/implements/jsonnet/config.hpp>
#include <rainy/foundation/willow/implements/jsonnet/lexer.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    using namespace rainy::foundation::exceptions::willow;

    struct jsonnet_parser {
        jsonnet_parser(const char *data, const std::size_t size, ast_arena &arena) : lexer_(data, size), arena_(arena) {
            advance();
        }

        jsonnet_node *parse() {
            jsonnet_node *root = parse_expression();
            if (current_.type != token_type::end_of_input) {
                fail("unexpected token after expression");
            }
            return root;
        }

    private:
        jsonnet_node *make(const jsonnet_node_kind kind) {
            return make_node(arena_.allocator, kind, &arena_.resource);
        }

        void advance() {
            previous_ = current_;
            current_ = lexer_.scan();
        }

        token peek(const std::size_t ahead) const {
            jsonnet_lexer copy = lexer_;
            token result = current_;
            for (std::size_t i = 0; i < ahead; ++i) {
                result = copy.scan();
            }
            return result;
        }

        bool check(const token_type type) const noexcept {
            return current_.type == type;
        }

        bool match(const token_type type) {
            if (check(type)) {
                advance();
                return true;
            }
            return false;
        }

        void expect(const token_type type, const char *const what) {
            if (!check(type)) {
                fail(what);
            }
            advance();
        }

        [[noreturn]] void fail(const char *const message) const {
            exceptions::willow::jsonnet::throw_jsonnet_parse_error(message);
        }

        ast::source_location mark() const noexcept {
            return current_.span.begin;
        }

        void finish(jsonnet_node *node, const ast::source_location start) const noexcept {
            node->location = ast::source_span{start, previous_.span.end};
        }

        static int precedence(const token_type type) noexcept {
            switch (type) {
                case token_type::op_star:
                case token_type::op_slash:
                case token_type::op_percent:
                    return 3;
                case token_type::op_plus:
                case token_type::op_minus:
                    return 4;
                case token_type::op_shift_left:
                case token_type::op_shift_right:
                    return 5;
                case token_type::op_less:
                case token_type::op_less_equal:
                case token_type::op_greater:
                case token_type::op_greater_equal:
                case token_type::kw_in:
                    return 6;
                case token_type::op_equal:
                case token_type::op_not_equal:
                    return 7;
                case token_type::op_ampersand:
                    return 8;
                case token_type::op_caret:
                    return 9;
                case token_type::op_pipe:
                    return 10;
                case token_type::op_and:
                    return 11;
                case token_type::op_or:
                    return 12;
                default:
                    return 0;
            }
        }

        static bool is_unary(const token_type type) noexcept {
            return type == token_type::op_minus || type == token_type::op_plus || type == token_type::op_not ||
                   type == token_type::op_tilde;
        }

        jsonnet_node *parse_expression() {
            return parse_binary(1);
        }

        jsonnet_node *parse_binary(const int min_precedence) {
            jsonnet_node *left = parse_unary();
            while (true) {
                const int level = precedence(current_.type);
                if (level == 0 || level < min_precedence) {
                    break;
                }
                const ast::source_location start = left->location.begin;
                const token_type op = current_.type;
                advance();
                jsonnet_node *right = parse_binary(level + 1);
                jsonnet_node *node = make(jsonnet_node_kind::binary);
                node->text.assign(token_name(op));
                node->add_child(left);
                node->add_child(right);
                finish(node, start);
                left = node;
            }
            return left;
        }

        jsonnet_node *parse_unary() {
            if (is_unary(current_.type)) {
                const ast::source_location start = mark();
                const token_type op = current_.type;
                advance();
                jsonnet_node *operand = parse_unary();
                jsonnet_node *node = make(jsonnet_node_kind::unary);
                node->text.assign(token_name(op));
                node->add_child(operand);
                finish(node, start);
                return node;
            }
            return parse_postfix();
        }

        jsonnet_node *parse_postfix() {
            jsonnet_node *expr = parse_primary();
            while (true) {
                switch (current_.type) {
                    case token_type::dot: {
                        advance();
                        if (current_.type != token_type::identifier) {
                            fail("expected identifier after '.'");
                        }
                        jsonnet_node *field = make(jsonnet_node_kind::string_literal);
                        field->text = current_.text;
                        field->location = current_.span;
                        advance();
                        jsonnet_node *node = make(jsonnet_node_kind::field_access);
                        node->add_child(expr);
                        node->add_child(field);
                        node->location = ast::source_span{expr->location.begin, field->location.end};
                        expr = node;
                        break;
                    }
                    case token_type::bracket_left: {
                        expr = parse_index_or_slice(expr);
                        break;
                    }
                    case token_type::paren_left: {
                        expr = parse_call(expr);
                        break;
                    }
                    case token_type::brace_left: {
                        const ast::source_location start = expr->location.begin;
                        jsonnet_node *object = parse_object();
                        jsonnet_node *node = make(jsonnet_node_kind::binary);
                        node->text.assign("+");
                        node->add_child(expr);
                        node->add_child(object);
                        finish(node, start);
                        expr = node;
                        break;
                    }
                    default:
                        return expr;
                }
            }
        }

        jsonnet_node *parse_index_or_slice(jsonnet_node *target) {
            const ast::source_location start = target->location.begin;
            expect(token_type::bracket_left, "expected '['");

            bool is_slice = false;
            jsonnet_node *begin_expr = nullptr;
            if (!check(token_type::colon) && !check(token_type::bracket_right)) {
                begin_expr = parse_expression();
            }
            if (check(token_type::colon)) {
                is_slice = true;
            }
            if (!is_slice) {
                if (begin_expr == nullptr) {
                    fail("expected index expression");
                }
                expect(token_type::bracket_right, "expected ']'");
                jsonnet_node *node = make(jsonnet_node_kind::index);
                node->add_child(target);
                node->add_child(begin_expr);
                finish(node, start);
                return node;
            }

            jsonnet_node *node = make(jsonnet_node_kind::slice);
            node->add_child(target);
            jsonnet_node *begin_node = begin_expr != nullptr ? begin_expr : make_null();
            if (begin_expr != nullptr) {
                node->set_flag(flag_slice_begin);
            }
            node->add_child(begin_node);
            jsonnet_node *end_node = make_null();
            jsonnet_node *step_node = make_null();
            expect(token_type::colon, "expected ':'");
            if (!check(token_type::colon) && !check(token_type::bracket_right)) {
                end_node = parse_expression();
                node->set_flag(flag_slice_end);
            }
            if (match(token_type::colon)) {
                if (!check(token_type::bracket_right)) {
                    step_node = parse_expression();
                    node->set_flag(flag_slice_step);
                }
            }
            node->add_child(end_node);
            node->add_child(step_node);
            expect(token_type::bracket_right, "expected ']'");
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_call(jsonnet_node *callee) {
            const ast::source_location start = callee->location.begin;
            jsonnet_node *node = make(jsonnet_node_kind::call);
            node->add_child(callee);
            expect(token_type::paren_left, "expected '('");
            while (!check(token_type::paren_right)) {
                if (check(token_type::identifier) && peek(1).type == token_type::op_equal) {
                    jsonnet_node *arg = make(jsonnet_node_kind::named_argument);
                    arg->text = current_.text;
                    advance();
                    advance();
                    arg->add_child(parse_expression());
                    node->add_child(arg);
                } else {
                    jsonnet_node *arg = make(jsonnet_node_kind::argument);
                    arg->add_child(parse_expression());
                    node->add_child(arg);
                }
                if (!match(token_type::comma)) {
                    break;
                }
            }
            expect(token_type::paren_right, "expected ')'");
            if (match(token_type::kw_tailstrict)) {
                node->set_flag(flag_tailstrict);
            }
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_primary() {
            const ast::source_location start = mark();
            switch (current_.type) {
                case token_type::literal_null: {
                    advance();
                    jsonnet_node *node = make(jsonnet_node_kind::null_literal);
                    finish(node, start);
                    return node;
                }
                case token_type::literal_true:
                case token_type::literal_false: {
                    const bool value = current_.type == token_type::literal_true;
                    advance();
                    jsonnet_node *node = make(jsonnet_node_kind::boolean_literal);
                    node->boolean = value;
                    finish(node, start);
                    return node;
                }
                case token_type::literal_number: {
                    const double value = current_.number;
                    const bool integer = current_.is_integer;
                    advance();
                    jsonnet_node *node = make(jsonnet_node_kind::number_literal);
                    node->number = value;
                    if (integer) {
                        node->set_flag(flag_integer);
                    }
                    finish(node, start);
                    return node;
                }
                case token_type::literal_string: {
                    jsonnet_node *node = make(jsonnet_node_kind::string_literal);
                    node->text = current_.text;
                    advance();
                    finish(node, start);
                    return node;
                }
                case token_type::identifier: {
                    jsonnet_node *node = make(jsonnet_node_kind::identifier);
                    node->text = current_.text;
                    advance();
                    finish(node, start);
                    return node;
                }
                case token_type::kw_self: {
                    advance();
                    jsonnet_node *node = make(jsonnet_node_kind::self_reference);
                    finish(node, start);
                    return node;
                }
                case token_type::kw_super: {
                    advance();
                    jsonnet_node *node = make(jsonnet_node_kind::super_reference);
                    finish(node, start);
                    return node;
                }
                case token_type::dollar: {
                    advance();
                    jsonnet_node *node = make(jsonnet_node_kind::dollar);
                    finish(node, start);
                    return node;
                }
                case token_type::brace_left:
                    return parse_object();
                case token_type::bracket_left:
                    return parse_array();
                case token_type::paren_left: {
                    advance();
                    jsonnet_node *inner = parse_expression();
                    expect(token_type::paren_right, "expected ')'");
                    return inner;
                }
                case token_type::kw_local:
                    return parse_local();
                case token_type::kw_if:
                    return parse_conditional();
                case token_type::kw_function:
                    return parse_function();
                case token_type::kw_assert:
                    return parse_assert();
                case token_type::kw_error:
                    return parse_error();
                case token_type::kw_import:
                case token_type::kw_importstr:
                case token_type::kw_importbin:
                    return parse_import();
                default:
                    fail("unexpected token in expression");
            }
        }

        jsonnet_node *make_null() {
            return make(jsonnet_node_kind::null_literal);
        }

        jsonnet_node *parse_local() {
            const ast::source_location start = mark();
            jsonnet_node *node = make(jsonnet_node_kind::local);
            expect(token_type::kw_local, "expected 'local'");
            while (true) {
                jsonnet_node *bind = parse_bind();
                node->add_child(bind);
                if (!match(token_type::comma)) {
                    break;
                }
            }
            expect(token_type::semicolon, "expected ';' after local bindings");
            node->add_child(parse_expression());
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_bind() {
            const ast::source_location start = mark();
            if (current_.type != token_type::identifier) {
                fail("expected identifier in binding");
            }
            jsonnet_node *bind = make(jsonnet_node_kind::bind);
            bind->text = current_.text;
            advance();
            if (check(token_type::paren_left)) {
                bind->set_flag(flag_method);
                parse_parameters(bind);
            }
            expect(token_type::op_equal, "expected '=' in binding");
            bind->add_child(parse_expression());
            finish(bind, start);
            return bind;
        }

        void parse_parameters(jsonnet_node *owner) {
            expect(token_type::paren_left, "expected '('");
            while (!check(token_type::paren_right)) {
                if (current_.type != token_type::identifier) {
                    fail("expected parameter name");
                }
                jsonnet_node *param = make(jsonnet_node_kind::parameter);
                param->text = current_.text;
                advance();
                if (match(token_type::op_equal)) {
                    param->set_flag(flag_has_default);
                    param->add_child(parse_expression());
                }
                owner->add_child(param);
                if (!match(token_type::comma)) {
                    break;
                }
            }
            expect(token_type::paren_right, "expected ')'");
        }

        jsonnet_node *parse_conditional() {
            const ast::source_location start = mark();
            jsonnet_node *node = make(jsonnet_node_kind::conditional);
            expect(token_type::kw_if, "expected 'if'");
            node->add_child(parse_expression());
            expect(token_type::kw_then, "expected 'then'");
            node->add_child(parse_expression());
            if (match(token_type::kw_else)) {
                node->set_flag(flag_has_else);
                node->add_child(parse_expression());
            } else {
                node->add_child(make_null());
            }
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_function() {
            const ast::source_location start = mark();
            jsonnet_node *node = make(jsonnet_node_kind::function);
            expect(token_type::kw_function, "expected 'function'");
            parse_parameters(node);
            node->add_child(parse_expression());
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_assert() {
            const ast::source_location start = mark();
            jsonnet_node *node = make(jsonnet_node_kind::assert);
            expect(token_type::kw_assert, "expected 'assert'");
            node->add_child(parse_expression());
            if (match(token_type::colon)) {
                node->add_child(parse_expression());
            } else {
                node->add_child(make_null());
            }
            expect(token_type::semicolon, "expected ';' after assert condition");
            node->add_child(parse_expression());
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_error() {
            const ast::source_location start = mark();
            jsonnet_node *node = make(jsonnet_node_kind::error);
            expect(token_type::kw_error, "expected 'error'");
            node->add_child(parse_expression());
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_import() {
            const ast::source_location start = mark();
            jsonnet_node_kind kind = jsonnet_node_kind::import_expression;
            if (current_.type == token_type::kw_importstr) {
                kind = jsonnet_node_kind::import_string;
            } else if (current_.type == token_type::kw_importbin) {
                kind = jsonnet_node_kind::import_binary;
            }
            advance();
            if (current_.type != token_type::literal_string) {
                fail("expected string literal after import");
            }
            jsonnet_node *node = make(kind);
            node->text = current_.text;
            advance();
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_array() {
            const ast::source_location start = mark();
            expect(token_type::bracket_left, "expected '['");
            if (match(token_type::bracket_right)) {
                jsonnet_node *node = make(jsonnet_node_kind::array);
                finish(node, start);
                return node;
            }
            jsonnet_node *first = parse_expression();
            if (check(token_type::kw_for) || check(token_type::kw_if)) {
                jsonnet_node *node = make(jsonnet_node_kind::array_comprehension);
                node->add_child(first);
                parse_comprehension_spec(node);
                expect(token_type::bracket_right, "expected ']'");
                finish(node, start);
                return node;
            }
            jsonnet_node *node = make(jsonnet_node_kind::array);
            node->add_child(first);
            while (match(token_type::comma)) {
                if (check(token_type::bracket_right)) {
                    break;
                }
                node->add_child(parse_expression());
            }
            expect(token_type::bracket_right, "expected ']'");
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_object() {
            const ast::source_location start = mark();
            expect(token_type::brace_left, "expected '{'");
            jsonnet_node *node = make(jsonnet_node_kind::object);
            while (!check(token_type::brace_right)) {
                if (check(token_type::kw_for) || check(token_type::kw_if)) {
                    break;
                }
                if (check(token_type::kw_local)) {
                    advance();
                    jsonnet_node *bind = parse_bind();
                    node->add_child(bind);
                } else if (check(token_type::kw_assert)) {
                    advance();
                    jsonnet_node *assertion = make(jsonnet_node_kind::assert);
                    assertion->add_child(parse_expression());
                    if (match(token_type::colon)) {
                        assertion->add_child(parse_expression());
                    } else {
                        assertion->add_child(make_null());
                    }
                    node->add_child(assertion);
                } else {
                    node->add_child(parse_object_field());
                }
                if (!match(token_type::comma)) {
                    break;
                }
            }
            if (check(token_type::kw_for) || check(token_type::kw_if)) {
                jsonnet_node *comprehension = make(jsonnet_node_kind::object_comprehension);
                for (ast::ast_node *child: node->children) {
                    comprehension->add_child(child);
                }
                parse_comprehension_spec(comprehension);
                node = comprehension;
            }
            expect(token_type::brace_right, "expected '}'");
            finish(node, start);
            return node;
        }

        jsonnet_node *parse_object_field() {
            const ast::source_location start = mark();
            jsonnet_node *field = make(jsonnet_node_kind::object_field);
            jsonnet_node *name = nullptr;
            if (current_.type == token_type::identifier) {
                name = make(jsonnet_node_kind::string_literal);
                name->text = current_.text;
                advance();
            } else if (current_.type == token_type::literal_string) {
                name = make(jsonnet_node_kind::string_literal);
                name->text = current_.text;
                advance();
            } else if (current_.type == token_type::bracket_left) {
                advance();
                name = parse_expression();
                expect(token_type::bracket_right, "expected ']' after computed field name");
                field->set_flag(flag_computed_name);
            } else {
                fail("expected field name in object");
            }
            field->add_child(name);
            if (match(token_type::op_plus)) {
                field->set_flag(flag_inherits);
            }
            if (check(token_type::paren_left)) {
                field->set_flag(flag_method);
                parse_parameters(field);
            }
            switch (current_.type) {
                case token_type::colon:
                    field->visibility = field_visibility::visible;
                    break;
                case token_type::double_colon:
                    field->visibility = field_visibility::hidden;
                    break;
                case token_type::triple_colon:
                    field->visibility = field_visibility::forced_visible;
                    break;
                default:
                    fail("expected ':', '::' or ':::' after field name");
            }
            advance();
            field->add_child(parse_expression());
            finish(field, start);
            return field;
        }

        void parse_comprehension_spec(jsonnet_node *owner) {
            while (true) {
                if (match(token_type::kw_for)) {
                    if (current_.type != token_type::identifier) {
                        fail("expected identifier after 'for'");
                    }
                    jsonnet_node *clause = make(jsonnet_node_kind::comprehension_for);
                    clause->text = current_.text;
                    advance();
                    expect(token_type::kw_in, "expected 'in' in comprehension");
                    clause->add_child(parse_expression());
                    owner->add_child(clause);
                } else if (match(token_type::kw_if)) {
                    jsonnet_node *clause = make(jsonnet_node_kind::comprehension_if);
                    clause->add_child(parse_expression());
                    owner->add_child(clause);
                } else {
                    break;
                }
            }
        }

        jsonnet_lexer lexer_;
        ast_arena &arena_;
        token current_{};
        token previous_{};
    };
}

#endif

#endif
