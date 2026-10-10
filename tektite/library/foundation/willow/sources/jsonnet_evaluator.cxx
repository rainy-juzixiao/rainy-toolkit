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
#include <cmath>
#include <rainy/foundation/willow/implements/jsonnet/evaluator.hpp>
#include <rainy/foundation/willow/implements/jsonnet/parser.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    using namespace rainy::foundation::exceptions::willow;

    namespace {
        bool as_boolean(const value_ptr &v) {
            if (v->kind != value_kind::boolean) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("expected a boolean");
            }
            return v->boolean;
        }

        double as_number(const value_ptr &v) {
            if (v->kind != value_kind::number) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("expected a number");
            }
            return v->number;
        }

        std::int64_t to_int64(const value_ptr &v) {
            return static_cast<std::int64_t>(as_number(v));
        }

        int compare_values(const value_ptr &left, const value_ptr &right) {
            if (left->kind == value_kind::number && right->kind == value_kind::number) {
                return left->number < right->number ? -1 : (left->number > right->number ? 1 : 0);
            }
            if (left->kind == value_kind::string && right->kind == value_kind::string) {
                return left->str < right->str ? -1 : (left->str > right->str ? 1 : 0);
            }
            return static_cast<int>(left->kind) - static_cast<int>(right->kind);
        }

        core::text::string view_to_string(const core::text::basic_string_view<char> view) {
            return core::text::string(view.data(), view.size());
        }

        document to_document(const value_ptr &v);

        document to_document(const value_ptr &v) {
            document out;
            switch (v->kind) {
                case value_kind::null_value:
                    out = document(document_type::null);
                    break;
                case value_kind::boolean:
                    out = document(v->boolean);
                    break;
                case value_kind::number:
                    out = document(v->number);
                    break;
                case value_kind::string:
                    out = document(v->str);
                    break;
                case value_kind::array:
                    out = document(document_type::array);
                    for (const auto &element: v->elements) {
                        out.push_back(to_document(element->force()));
                    }
                    break;
                case value_kind::object:
                    out = document(document_type::object);
                    for (const auto &name: v->object->field_names(false)) {
                        std::size_t layer = 0;
                        const field_slot *slot = v->object->find(name, layer);
                        out[name] = to_document(slot->body(make_env(nullptr)));
                    }
                    break;
                case value_kind::function:
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("cannot convert a function to a document");
            }
            return out;
        }

        value_ptr from_document(const document &doc) {
            switch (doc.type()) {
                case document_type::null:
                    return make_null();
                case document_type::boolean:
                    return make_boolean(doc.as_bool());
                case document_type::number_integer:
                    return make_number(static_cast<double>(doc.as_integer()));
                case document_type::number_float:
                    return make_number(doc.as_float());
                case document_type::string:
                    return make_string(doc.as_string());
                case document_type::array: {
                    core::collections::vector<thunk_ptr> elements;
                    for (const auto &element: doc.as_array()) {
                        elements.push_back(eager_thunk(from_document(element)));
                    }
                    return make_array(utility::move(elements));
                }
                case document_type::object: {
                    object_ptr object = core::memory::make_shared<object_value>();
                    object_layer layer;
                    layer.lexical_env = make_env(nullptr);
                    for (const auto &entry: doc.as_object()) {
                        field_slot slot;
                        slot.visibility = field_visibility::visible;
                        value_ptr captured = from_document(entry.second);
                        slot.body = [captured](const env_ptr &) -> value_ptr { return captured; };
                        layer.names.push_back(entry.first);
                        layer.slots.push_back(utility::move(slot));
                    }
                    object->layers.push_back(utility::move(layer));
                    return make_object(utility::move(object));
                }
            }
            return make_null();
        }
    }

    value_ptr evaluator::evaluate(jsonnet_node *root) {
        env_ptr root_env = make_env(nullptr);
        root_env->bindings.emplace_back(core::text::string("std"), eager_thunk(make_std()));
        return eval(root, root_env);
    }

    value_ptr evaluator::eval(jsonnet_node *node, const env_ptr &env) {
        switch (node->json_kind) {
            case jsonnet_node_kind::null_literal:
                return make_null();
            case jsonnet_node_kind::boolean_literal:
                return make_boolean(node->boolean);
            case jsonnet_node_kind::number_literal:
                return make_number(node->number);
            case jsonnet_node_kind::string_literal:
                return make_string(core::text::string(node->text.data(), node->text.size()));
            case jsonnet_node_kind::identifier:
                return eval_identifier(node, env);
            case jsonnet_node_kind::self_reference:
            case jsonnet_node_kind::dollar: {
                const auto *self = env->find_self();
                if (self == nullptr) {
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("self is not available outside an object");
                }
                return *self;
            }
            case jsonnet_node_kind::super_reference: {
                const auto *super = env->find_super();
                if (super == nullptr) {
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("super is not available outside an object");
                }
                return *super;
            }
            case jsonnet_node_kind::unary:
                return eval_unary(node, env);
            case jsonnet_node_kind::binary:
                return eval_binary(node, env);
            case jsonnet_node_kind::conditional:
                return eval_conditional(node, env);
            case jsonnet_node_kind::field_access:
                return eval_field_access(node, env);
            case jsonnet_node_kind::index:
                return eval_index(node, env);
            case jsonnet_node_kind::slice:
                return eval_slice(node, env);
            case jsonnet_node_kind::call:
                return eval_call(node, env);
            case jsonnet_node_kind::function:
                return eval_function(node, env);
            case jsonnet_node_kind::object:
                return make_object(eval_object(node, env));
            case jsonnet_node_kind::object_comprehension:
                return make_object(eval_object_comprehension(node, env));
            case jsonnet_node_kind::array:
                return eval_array(node, env);
            case jsonnet_node_kind::array_comprehension:
                return eval_array_comprehension(node, env);
            case jsonnet_node_kind::local:
                return eval_local(node, env);
            case jsonnet_node_kind::assert:
                return eval_assert(node, env);
            case jsonnet_node_kind::error:
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error(value_to_string(*eval(node->at(0), env)).c_str());
            case jsonnet_node_kind::import_expression:
            case jsonnet_node_kind::import_string:
            case jsonnet_node_kind::import_binary:
                return eval_import(node);
            default:
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("unsupported expression");
        }
    }

    value_ptr evaluator::eval_identifier(jsonnet_node *node, const env_ptr &env) {
        const core::text::basic_string_view<char> name(node->text.data(), node->text.size());
        thunk_ptr *slot = env->find(name);
        if (slot == nullptr) {
            core::text::string message = "unknown variable: ";
            message.append(name.data(), name.size());
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error(message.c_str());
        }
        return (*slot)->force();
    }

    value_ptr evaluator::eval_unary(jsonnet_node *node, const env_ptr &env) {
        const value_ptr operand = eval(node->at(0), env);
        const core::text::basic_string_view<char> op(node->text.data(), node->text.size());
        if (op == "-") {
            return make_number(-as_number(operand));
        }
        if (op == "+") {
            return make_number(as_number(operand));
        }
        if (op == "!") {
            return make_boolean(!as_boolean(operand));
        }
        if (op == "~") {
            return make_number(static_cast<double>(~to_int64(operand)));
        }
        exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("unknown unary operator");
    }

    value_ptr evaluator::eval_binary(jsonnet_node *node, const env_ptr &env) {
        const core::text::basic_string_view<char> op(node->text.data(), node->text.size());
        if (op == "&&") {
            return as_boolean(eval(node->at(0), env)) ? make_boolean(as_boolean(eval(node->at(1), env))) : make_boolean(false);
        }
        if (op == "||") {
            return as_boolean(eval(node->at(0), env)) ? make_boolean(true) : make_boolean(as_boolean(eval(node->at(1), env)));
        }
        return apply_binary(op, eval(node->at(0), env), eval(node->at(1), env));
    }

    value_ptr evaluator::apply_binary(const core::text::basic_string_view<char> op, const value_ptr &left,
                                      const value_ptr &right) {
        if (op == "+") {
            if (left->kind == value_kind::number && right->kind == value_kind::number) {
                return make_number(left->number + right->number);
            }
            if (left->kind == value_kind::string || right->kind == value_kind::string) {
                core::text::string result = value_to_string(*left);
                result.append(value_to_string(*right));
                return make_string(utility::move(result));
            }
            if (left->kind == value_kind::object && right->kind == value_kind::object) {
                object_ptr merged = core::memory::make_shared<object_value>();
                merged->layers = left->object->layers;
                for (const auto &layer: right->object->layers) {
                    merged->layers.push_back(layer);
                }
                return make_object(utility::move(merged));
            }
            if (left->kind == value_kind::array && right->kind == value_kind::array) {
                auto elements = left->elements;
                for (const auto &element: right->elements) {
                    elements.push_back(element);
                }
                return make_array(utility::move(elements));
            }
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("invalid operands for '+'");
        }
        if (op == "-") {
            return make_number(as_number(left) - as_number(right));
        }
        if (op == "*") {
            return make_number(as_number(left) * as_number(right));
        }
        if (op == "/") {
            if (as_number(right) == 0.0) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("division by zero");
            }
            return make_number(as_number(left) / as_number(right));
        }
        if (op == "%") {
            if (left->kind == value_kind::string) {
                return format_string(left->str, right);
            }
            if (as_number(right) == 0.0) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("division by zero");
            }
            return make_number(willow::implements::operations::modulo(as_number(left), as_number(right)));
        }
        if (op == "<<") {
            return make_number(static_cast<double>(to_int64(left) << (to_int64(right) & 63)));
        }
        if (op == ">>") {
            return make_number(static_cast<double>(to_int64(left) >> (to_int64(right) & 63)));
        }
        if (op == "&") {
            return make_number(static_cast<double>(to_int64(left) & to_int64(right)));
        }
        if (op == "|") {
            return make_number(static_cast<double>(to_int64(left) | to_int64(right)));
        }
        if (op == "^") {
            return make_number(static_cast<double>(to_int64(left) ^ to_int64(right)));
        }
        if (op == "<") {
            return make_boolean(compare_values(left, right) < 0);
        }
        if (op == "<=") {
            return make_boolean(compare_values(left, right) <= 0);
        }
        if (op == ">") {
            return make_boolean(compare_values(left, right) > 0);
        }
        if (op == ">=") {
            return make_boolean(compare_values(left, right) >= 0);
        }
        if (op == "==") {
            return make_boolean(value_equals(*left, *right));
        }
        if (op == "!=") {
            return make_boolean(!value_equals(*left, *right));
        }
        if (op == "in") {
            if (right->kind != value_kind::object) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("'in' requires an object on the right");
            }
            std::size_t layer = 0;
            return make_boolean(right->object->find(value_to_string(*left), layer) != nullptr);
        }
        exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("unknown binary operator");
    }

    value_ptr evaluator::eval_conditional(jsonnet_node *node, const env_ptr &env) {
        if (as_boolean(eval(node->at(0), env))) {
            return eval(node->at(1), env);
        }
        return eval(node->at(2), env);
    }

    value_ptr evaluator::eval_field_access(jsonnet_node *node, const env_ptr &env) {
        return lookup_field(eval(node->at(0), env),
                            core::text::basic_string_view<char>(node->at(1)->text.data(), node->at(1)->text.size()));
    }

    value_ptr evaluator::eval_index(jsonnet_node *node, const env_ptr &env) {
        const value_ptr target = eval(node->at(0), env);
        const value_ptr index = eval(node->at(1), env);
        if (target->kind == value_kind::array) {
            const auto position = static_cast<std::int64_t>(as_number(index));
            if (position < 0 || static_cast<std::size_t>(position) >= target->elements.size()) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("array index out of bounds");
            }
            return target->elements[static_cast<std::size_t>(position)]->force();
        }
        if (target->kind == value_kind::object) {
            return lookup_field(target, value_to_string(*index));
        }
        exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("indexing requires an array or object");
    }

    value_ptr evaluator::eval_slice(jsonnet_node *node, const env_ptr &env) {
        const value_ptr target = eval(node->at(0), env);
        const std::int64_t size = target->kind == value_kind::array
                                      ? static_cast<std::int64_t>(target->elements.size())
                                      : static_cast<std::int64_t>(target->str.size());
        auto bound = [&](const std::size_t child, const std::uint16_t flag, const std::int64_t fallback) -> std::int64_t {
            if (!node->has_flag(flag)) {
                return fallback;
            }
            return static_cast<std::int64_t>(as_number(eval(node->at(child), env)));
        };
        const std::int64_t step = bound(3, flag_slice_step, 1);
        if (step == 0) {
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("slice step cannot be zero");
        }
        auto norm = [size](std::int64_t value) {
            if (value < 0) {
                value += size;
            }
            return value < 0 ? std::int64_t{0} : (value > size ? size : value);
        };
        const std::int64_t begin = norm(bound(1, flag_slice_begin, 0));
        const std::int64_t end = norm(bound(2, flag_slice_end, size));
        if (target->kind == value_kind::array) {
            core::collections::vector<thunk_ptr> result;
            if (step > 0) {
                for (std::int64_t i = begin; i < end; i += step) {
                    result.push_back(target->elements[static_cast<std::size_t>(i)]);
                }
            } else {
                for (std::int64_t i = begin; i > end; i += step) {
                    result.push_back(target->elements[static_cast<std::size_t>(i)]);
                }
            }
            return make_array(utility::move(result));
        }
        if (target->kind == value_kind::string) {
            core::text::string result;
            if (step > 0) {
                for (std::int64_t i = begin; i < end; i += step) {
                    result.push_back(target->str[static_cast<std::size_t>(i)]);
                }
            } else {
                for (std::int64_t i = begin; i > end; i += step) {
                    result.push_back(target->str[static_cast<std::size_t>(i)]);
                }
            }
            return make_string(utility::move(result));
        }
        exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("slicing requires an array or string");
    }

    value_ptr evaluator::eval_call(jsonnet_node *node, const env_ptr &env) {
        const value_ptr callee = eval(node->at(0), env);
        if (callee->kind != value_kind::function) {
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("attempt to call a non-function");
        }
        core::collections::vector<value_ptr> positional;
        core::collections::vector<core::container::pair<core::text::string, value_ptr>> named;
        for (std::size_t i = 1; i < node->child_count(); ++i) {
            jsonnet_node *arg = node->at(i);
            if (arg->json_kind == jsonnet_node_kind::named_argument) {
                named.emplace_back(core::text::string(arg->text.data(), arg->text.size()), eval(arg->at(0), env));
            } else {
                positional.push_back(eval(arg->at(0), env));
            }
        }
        return call_function(callee->function, positional, named);
    }

    value_ptr evaluator::eval_function(jsonnet_node *node, const env_ptr &env) {
        function_ptr fn = core::memory::make_shared<function_value>();
        fn->env = env;
        std::size_t index = 0;
        while (index < node->child_count() && node->at(index)->json_kind == jsonnet_node_kind::parameter) {
            jsonnet_node *param = node->at(index);
            fn->params.emplace_back(param->text.data(), param->text.size());
            fn->defaults.push_back(param->has_flag(flag_has_default) ? param->at(0) : nullptr);
            ++index;
        }
        fn->body = node->at(index);
        return make_function(utility::move(fn));
    }

    value_ptr evaluator::call_function(const function_ptr &fn, core::collections::vector<value_ptr> &positional,
                                       core::collections::vector<core::container::pair<core::text::string, value_ptr>> &named) {
        if (fn->is_native) {
            return fn->native(positional);
        }
        env_ptr call_env = make_env(fn->env);
        const auto *self = fn->env->find_self();
        if (self != nullptr) {
            call_env->self = *self;
        }
        const auto *super = fn->env->find_super();
        if (super != nullptr) {
            call_env->super = *super;
        }
        std::size_t positional_index = 0;
        for (std::size_t i = 0; i < fn->params.size(); ++i) {
            const auto &name = fn->params[i];
            value_ptr bound;
            bool has_bound = false;
            if (positional_index < positional.size()) {
                bound = positional[positional_index++];
                has_bound = true;
            } else {
                for (auto &entry: named) {
                    if (entry.first == name) {
                        bound = entry.second;
                        has_bound = true;
                        break;
                    }
                }
            }
            if (has_bound) {
                call_env->bindings.emplace_back(name, eager_thunk(bound));
            } else if (fn->defaults[i] != nullptr) {
                jsonnet_node *default_node = fn->defaults[i];
                env_ptr capture = fn->env;
                call_env->bindings.emplace_back(
                    name, make_thunk([this, default_node, capture]() { return eval(default_node, capture); }));
            } else {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("missing argument for function call");
            }
        }
        return eval(fn->body, call_env);
    }

    void evaluator::add_local_binding(const env_ptr &env, jsonnet_node *bind) {
        jsonnet_node *body = bind->at(bind->child_count() - 1);
        env_ptr capture = env;
        if (bind->has_flag(flag_method)) {
            core::collections::vector<core::text::string> params;
            core::collections::vector<jsonnet_node *> defaults;
            for (std::size_t i = 0; i + 1 < bind->child_count(); ++i) {
                jsonnet_node *param = bind->at(i);
                if (param->json_kind != jsonnet_node_kind::parameter) {
                    continue;
                }
                params.emplace_back(param->text.data(), param->text.size());
                defaults.push_back(param->has_flag(flag_has_default) ? param->at(0) : nullptr);
            }
            env->bindings.emplace_back(
                core::text::string(bind->text.data(), bind->text.size()),
                make_thunk([this, params, defaults, body, capture]() -> value_ptr {
                    function_ptr fn = core::memory::make_shared<function_value>();
                    fn->params = params;
                    fn->defaults = defaults;
                    fn->body = body;
                    fn->env = capture;
                    return make_function(utility::move(fn));
                }));
            return;
        }
        env->bindings.emplace_back(core::text::string(bind->text.data(), bind->text.size()),
                                   make_thunk([this, body, capture]() { return eval(body, capture); }));
    }

    void evaluator::add_field(object_layer &layer, jsonnet_node *field, const env_ptr &env) {
        core::text::string name;
        if (field->has_flag(flag_computed_name)) {
            name = value_to_string(*eval(field->at(0), env));
        } else {
            name = core::text::string(field->at(0)->text.data(), field->at(0)->text.size());
        }
        field_slot slot;
        slot.visibility = field->visibility;
        slot.inherits = field->has_flag(flag_inherits);
        slot.scope = env;
        jsonnet_node *body = field->at(field->child_count() - 1);
        if (field->has_flag(flag_method)) {
            slot.body = [this, field, body](const env_ptr &field_env) {
                function_ptr fn = core::memory::make_shared<function_value>();
                fn->env = field_env;
                for (std::size_t i = 1; i < field->child_count() - 1; ++i) {
                    jsonnet_node *param = field->at(i);
                    if (param->json_kind != jsonnet_node_kind::parameter) {
                        continue;
                    }
                    fn->params.emplace_back(param->text.data(), param->text.size());
                    fn->defaults.push_back(param->has_flag(flag_has_default) ? param->at(0) : nullptr);
                }
                fn->body = body;
                return make_function(utility::move(fn));
            };
        } else {
            slot.body = [this, body](const env_ptr &field_env) { return eval(body, field_env); };
        }
        layer.names.push_back(utility::move(name));
        layer.slots.push_back(utility::move(slot));
    }

    object_ptr evaluator::eval_object(jsonnet_node *node, const env_ptr &env) {
        object_ptr object = core::memory::make_shared<object_value>();
        core::collections::vector<jsonnet_node *> binds;
        core::collections::vector<jsonnet_node *> asserts;
        core::collections::vector<jsonnet_node *> fields;
        for (std::size_t i = 0; i < node->child_count(); ++i) {
            jsonnet_node *child = node->at(i);
            if (child->json_kind == jsonnet_node_kind::bind) {
                binds.push_back(child);
            } else if (child->json_kind == jsonnet_node_kind::assert) {
                asserts.push_back(child);
            } else if (child->json_kind == jsonnet_node_kind::object_field) {
                fields.push_back(child);
            }
        }
        object_layer layer;
        layer.lexical_env = binds.empty() ? env : make_env(env);
        for (jsonnet_node *bind: binds) {
            add_local_binding(layer.lexical_env, bind);
        }
        for (jsonnet_node *field: fields) {
            add_field(layer, field, layer.lexical_env);
        }
        for (jsonnet_node *assertion: asserts) {
            layer.asserts.push_back(assertion);
        }
        object->layers.push_back(utility::move(layer));
        return object;
    }

    value_ptr evaluator::lookup_field(const value_ptr &target, const core::text::basic_string_view<char> name) {
        if (target->kind != value_kind::object) {
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("field access requires an object");
        }
        std::size_t layer_index = 0;
        const field_slot *slot = target->object->find(name, layer_index);
        if (slot == nullptr) {
            core::text::string message = "field does not exist: ";
            message.append(name.data(), name.size());
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error(message.c_str());
        }
        run_asserts(target->object, target);
        return eval_field_body(target->object, name, *slot, layer_index, target);
    }

    value_ptr evaluator::eval_field_body(const object_ptr &object, const core::text::basic_string_view<char> name,
                                         const field_slot &slot, const std::size_t layer_index, const value_ptr &self_value) {
        object_ptr super_object = core::memory::make_shared<object_value>();
        for (std::size_t i = 0; i < layer_index; ++i) {
            super_object->layers.push_back(object->layers[i]);
        }
        const value_ptr super_value = make_object(super_object);
        env_ptr field_env = make_env(slot.scope != nullptr ? slot.scope : object->layers[layer_index].lexical_env);
        field_env->self = self_value;
        field_env->super = super_value;
        value_ptr base = slot.body(field_env);
        if (slot.inherits) {
            std::size_t super_layer = 0;
            const field_slot *inherited = super_object->find(name, super_layer);
            if (inherited != nullptr) {
                const value_ptr parent = eval_field_body(super_object, name, *inherited, super_layer, self_value);
                return apply_binary("+", parent, base);
            }
        }
        return base;
    }

    void evaluator::run_asserts(const object_ptr &object, const value_ptr &self_value) {
        if (object->asserts_run) {
            return;
        }
        object->asserts_run = true;
        for (const auto &layer: object->layers) {
            for (jsonnet_node *assertion: layer.asserts) {
                env_ptr assert_env = make_env(layer.lexical_env);
                assert_env->self = self_value;
                assert_env->super = make_object(core::memory::make_shared<object_value>());
                if (!as_boolean(eval(assertion->at(0), assert_env))) {
                    core::text::string message = "assertion failed";
                    if (assertion->at(1)->json_kind != jsonnet_node_kind::null_literal) {
                        message = value_to_string(*eval(assertion->at(1), assert_env));
                    }
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error(message.c_str());
                }
            }
        }
    }

    value_ptr evaluator::eval_array(jsonnet_node *node, const env_ptr &env) {
        core::collections::vector<thunk_ptr> elements;
        for (std::size_t i = 0; i < node->child_count(); ++i) {
            jsonnet_node *child = node->at(i);
            elements.push_back(make_thunk([this, child, env]() { return eval(child, env); }));
        }
        return make_array(utility::move(elements));
    }

    void evaluator::collect_comprehension_envs(const core::collections::vector<jsonnet_node *> &clauses, const env_ptr &start,
                                               core::collections::vector<env_ptr> &out) {
        core::collections::vector<env_ptr> contexts;
        contexts.push_back(start);
        for (jsonnet_node *clause: clauses) {
            core::collections::vector<env_ptr> next;
            if (clause->json_kind == jsonnet_node_kind::comprehension_for) {
                for (const auto &context: contexts) {
                    const value_ptr items = eval(clause->at(0), context);
                    if (items->kind != value_kind::array) {
                        exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("for clause requires an array");
                    }
                    for (const auto &element: items->elements) {
                        env_ptr child = make_env(context);
                        child->bindings.emplace_back(core::text::string(clause->text.data(), clause->text.size()),
                                                     eager_thunk(element->force()));
                        next.push_back(utility::move(child));
                    }
                }
            } else {
                for (const auto &context: contexts) {
                    if (as_boolean(eval(clause->at(0), context))) {
                        next.push_back(context);
                    }
                }
            }
            contexts = utility::move(next);
        }
        out = utility::move(contexts);
    }

    value_ptr evaluator::eval_array_comprehension(jsonnet_node *node, const env_ptr &env) {
        jsonnet_node *body = node->at(0);
        core::collections::vector<jsonnet_node *> clauses;
        for (std::size_t i = 1; i < node->child_count(); ++i) {
            clauses.push_back(node->at(i));
        }
        core::collections::vector<env_ptr> contexts;
        collect_comprehension_envs(clauses, env, contexts);
        core::collections::vector<thunk_ptr> elements;
        for (const auto &context: contexts) {
            elements.push_back(make_thunk([this, body, context]() { return eval(body, context); }));
        }
        return make_array(utility::move(elements));
    }

    object_ptr evaluator::eval_object_comprehension(jsonnet_node *node, const env_ptr &env) {
        object_ptr object = core::memory::make_shared<object_value>();
        core::collections::vector<jsonnet_node *> binds;
        core::collections::vector<jsonnet_node *> asserts;
        core::collections::vector<jsonnet_node *> fields;
        core::collections::vector<jsonnet_node *> clauses;
        for (std::size_t i = 0; i < node->child_count(); ++i) {
            jsonnet_node *child = node->at(i);
            switch (child->json_kind) {
                case jsonnet_node_kind::bind:
                    binds.push_back(child);
                    break;
                case jsonnet_node_kind::assert:
                    asserts.push_back(child);
                    break;
                case jsonnet_node_kind::comprehension_for:
                case jsonnet_node_kind::comprehension_if:
                    clauses.push_back(child);
                    break;
                default:
                    fields.push_back(child);
                    break;
            }
        }
        object_layer layer;
        layer.lexical_env = binds.empty() ? env : make_env(env);
        for (jsonnet_node *bind: binds) {
            add_local_binding(layer.lexical_env, bind);
        }
        for (jsonnet_node *assertion: asserts) {
            layer.asserts.push_back(assertion);
        }
        core::collections::vector<env_ptr> contexts;
        collect_comprehension_envs(clauses, layer.lexical_env, contexts);
        for (const auto &context: contexts) {
            for (jsonnet_node *field: fields) {
                if (field->has_flag(flag_computed_name)) {
                    const value_ptr name_value = eval(field->at(0), context);
                    if (name_value->kind == value_kind::null_value) {
                        continue;
                    }
                }
                add_field(layer, field, context);
            }
        }
        object->layers.push_back(utility::move(layer));
        return object;
    }

    value_ptr evaluator::eval_local(jsonnet_node *node, const env_ptr &env) {
        env_ptr local_env = make_env(env);
        for (std::size_t i = 0; i < node->child_count() - 1; ++i) {
            add_local_binding(local_env, node->at(i));
        }
        return eval(node->at(node->child_count() - 1), local_env);
    }

    value_ptr evaluator::eval_assert(jsonnet_node *node, const env_ptr &env) {
        if (!as_boolean(eval(node->at(0), env))) {
            core::text::string message = "assertion failed";
            if (node->at(1)->json_kind != jsonnet_node_kind::null_literal) {
                message = value_to_string(*eval(node->at(1), env));
            }
            exceptions::willow::jsonnet::throw_jsonnet_evaluate_error(message.c_str());
        }
        return eval(node->at(2), env);
    }

    value_ptr evaluator::eval_import(jsonnet_node *node) {
        const core::text::string path(node->text.data(), node->text.size());
        if (resolver_ == nullptr) {
            exceptions::willow::jsonnet::throw_jsonnet_import_error("no import resolver configured");
        }
        if (node->json_kind == jsonnet_node_kind::import_expression) {
            if (!resolver_->import_source) {
                exceptions::willow::jsonnet::throw_jsonnet_import_error("no import resolver configured");
            }
            const core::text::string source = resolver_->import_source(path);
            ast_arena arena;
            jsonnet_parser parser(source.data(), source.size(), arena);
            arena.root = parser.parse();
            evaluator nested{resolver_};
            return nested.evaluate(arena.root);
        }
        if (node->json_kind == jsonnet_node_kind::import_string) {
            if (!resolver_->importstr_source) {
                exceptions::willow::jsonnet::throw_jsonnet_import_error("no importstr resolver configured");
            }
            return make_string(resolver_->importstr_source(path));
        }
        if (!resolver_->importbin_source) {
            exceptions::willow::jsonnet::throw_jsonnet_import_error("no importbin resolver configured");
        }
        core::collections::vector<thunk_ptr> elements;
        for (const auto byte: resolver_->importbin_source(path)) {
            elements.push_back(eager_thunk(make_number(static_cast<double>(byte))));
        }
        return make_array(utility::move(elements));
    }

    value_ptr evaluator::format_string(const core::text::string &format, const value_ptr &args) {
        core::text::string result;
        const bool args_are_array = args->kind == value_kind::array;
        std::size_t arg_index = 0;
        for (std::size_t i = 0; i < format.size(); ++i) {
            if (format[i] != '%') {
                result.push_back(format[i]);
                continue;
            }
            ++i;
            if (i >= format.size()) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("incomplete format specifier");
            }
            const char spec = format[i];
            if (spec == '%') {
                result.push_back('%');
                continue;
            }
            value_ptr argument;
            if (args_are_array) {
                if (arg_index >= args->elements.size()) {
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("not enough arguments for format");
                }
                argument = args->elements[arg_index++]->force();
            } else {
                argument = args;
            }
            switch (spec) {
                case 'd':
                case 'i':
                    result.append(number_to_string(std::floor(as_number(argument))));
                    break;
                case 'f': {
                    char buffer[64]{};
                    const auto code = core::text::to_chars(buffer, buffer + sizeof(buffer), as_number(argument),
                                                           core::text::chars_format::fixed, 6);
                    result.append(buffer, static_cast<std::size_t>(code.ptr - buffer));
                    break;
                }
                case 's':
                    result.append(value_to_string(*argument));
                    break;
                case 'c':
                    result.push_back(static_cast<char>(static_cast<std::int64_t>(as_number(argument))));
                    break;
                default:
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("unsupported format specifier");
            }
        }
        return make_string(utility::move(result));
    }

    void evaluator::add_native(object_layer &layer, const core::text::basic_string_view<char> name,
                               functional::delegate<value_ptr(core::collections::vector<value_ptr> &)> fn) {
        field_slot slot;
        slot.visibility = field_visibility::hidden;
        auto native = utility::move(fn);
        slot.body = [native](const env_ptr &) -> value_ptr {
            function_ptr f = core::memory::make_shared<function_value>();
            f->is_native = true;
            f->native = native;
            return make_function(utility::move(f));
        };
        layer.names.push_back(view_to_string(name));
        layer.slots.push_back(utility::move(slot));
    }

    value_ptr evaluator::make_std() {
        object_ptr object = core::memory::make_shared<object_value>();
        object_layer layer;
        layer.lexical_env = make_env(nullptr);
        add_native(layer, "length", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            const auto &v = args[0];
            switch (v->kind) {
                case value_kind::string:
                    return make_number(static_cast<double>(willow::implements::operations::codepoint_length(v->str)));
                case value_kind::array:
                    return make_number(static_cast<double>(v->elements.size()));
                case value_kind::object:
                    return make_number(static_cast<double>(v->object->field_names(false).size()));
                default:
                    exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("std.length requires a string, array or object");
            }
        });
        add_native(layer, "type", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_string(view_to_string(value_type_name(*args[0])));
        });
        add_native(layer, "toString", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_string(value_to_string(*args[0]));
        });
        add_native(layer, "codepoint", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_number(static_cast<double>(willow::implements::operations::codepoint<document>(args[0]->str)));
        });
        add_native(layer, "char", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_string(willow::implements::operations::char_from_code<document>(to_int64(args[0])).as_string());
        });
        add_native(layer, "mod", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            if (as_number(args[1]) == 0.0) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("division by zero");
            }
            return make_number(willow::implements::operations::modulo(as_number(args[0]), as_number(args[1])));
        });
        add_native(layer, "floor", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_number(std::floor(as_number(args[0])));
        });
        add_native(layer, "ceil", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_number(std::ceil(as_number(args[0])));
        });
        add_native(layer, "abs", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_number(std::fabs(as_number(args[0])));
        });
        add_native(layer, "sqrt", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_number(std::sqrt(as_number(args[0])));
        });
        add_native(layer, "pow", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_number(std::pow(as_number(args[0]), as_number(args[1])));
        });
        add_native(layer, "min", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            const auto &array = args[0]->elements;
            if (array.empty()) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("std.min requires a non-empty array");
            }
            value_ptr best = array[0]->force();
            for (const auto &element: array) {
                const value_ptr candidate = element->force();
                if (compare_values(candidate, best) < 0) {
                    best = candidate;
                }
            }
            return best;
        });
        add_native(layer, "max", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            const auto &array = args[0]->elements;
            if (array.empty()) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("std.max requires a non-empty array");
            }
            value_ptr best = array[0]->force();
            for (const auto &element: array) {
                const value_ptr candidate = element->force();
                if (compare_values(candidate, best) > 0) {
                    best = candidate;
                }
            }
            return best;
        });
        add_native(layer, "primitiveEquals", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return make_boolean(value_equals(*args[0], *args[1]));
        });
        add_native(layer, "join", [](core::collections::vector<value_ptr> &args) -> value_ptr {
            return from_document(willow::implements::operations::join(to_document(args[0]), to_document(args[1])));
        });
        object->layers.push_back(utility::move(layer));
        return make_object(utility::move(object));
    }

    void evaluator::manifest_value(const value_ptr &v, document &out) {        switch (v->kind) {
            case value_kind::null_value:
                out = document(document_type::null);
                break;
            case value_kind::boolean:
                out = document(v->boolean);
                break;
            case value_kind::number:
                out = document(v->number);
                break;
            case value_kind::string:
                out = document(v->str);
                break;
            case value_kind::array:
                out = document(document_type::array);
                for (const auto &element: v->elements) {
                    document child;
                    manifest_value(element->force(), child);
                    out.push_back(utility::move(child));
                }
                break;
            case value_kind::object:
                out = document(document_type::object);
                for (const auto &name: v->object->field_names(false)) {
                    document child;
                    std::size_t layer = 0;
                    const field_slot *slot = v->object->find(name, layer);
                    manifest_value(eval_field_body(v->object, name, *slot, layer, v), child);
                    out[name] = utility::move(child);
                }
                break;
            case value_kind::function:
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("cannot manifest a function");
        }
    }

    document evaluator::manifest_document(const value_ptr &v) {
        document out;
        manifest_value(v, out);
        return out;
    }
}

#endif
