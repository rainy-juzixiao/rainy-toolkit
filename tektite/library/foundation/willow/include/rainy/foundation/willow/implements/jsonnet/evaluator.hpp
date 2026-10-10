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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_EVALUATOR_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_EVALUATOR_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/jsonnet/config.hpp>
#include <rainy/foundation/willow/implements/jsonnet/resolver.hpp>
#include <rainy/foundation/willow/implements/jsonnet/value.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    /**
     * \lang english
     * @brief Evaluates a jsonnet syntax tree lazily into runtime values.
     *
     * \lang simp-chinese
     * @brief 将 jsonnet 语法树惰性求值为运行时值。
     */
    class evaluator {
    public:
        explicit evaluator(const import_resolver *resolver = nullptr) : resolver_(resolver) {
        }

        value_ptr evaluate(jsonnet_node *root);

        document manifest_document(const value_ptr &v);

        template <typename BasicDocument>
        BasicDocument manifest(const value_ptr &v) {
            document source = manifest_document(v);
            if constexpr (type_traits::type_relations::is_same_v<BasicDocument, document>) {
                return source;
            } else {
                return willow::implements::convert_document<BasicDocument>(source);
            }
        }

    private:
        value_ptr eval(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_identifier(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_unary(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_binary(jsonnet_node *node, const env_ptr &env);
        value_ptr apply_binary(core::text::basic_string_view<char> op, const value_ptr &left, const value_ptr &right);
        value_ptr eval_conditional(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_field_access(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_index(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_slice(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_call(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_function(jsonnet_node *node, const env_ptr &env);
        value_ptr call_function(const function_ptr &fn, core::collections::vector<value_ptr> &positional,
                                core::collections::vector<core::container::pair<core::text::string, value_ptr>> &named);
        object_ptr eval_object(jsonnet_node *node, const env_ptr &env);
        object_ptr eval_object_comprehension(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_array(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_array_comprehension(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_local(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_assert(jsonnet_node *node, const env_ptr &env);
        value_ptr eval_import(jsonnet_node *node);
        value_ptr eval_field_body(const object_ptr &object, const core::text::basic_string_view<char> name,
                                  const field_slot &slot, std::size_t layer_index, const value_ptr &self_value);
        value_ptr lookup_field(const value_ptr &target, const core::text::basic_string_view<char> name);
        void run_asserts(const object_ptr &object, const value_ptr &self_value);
        void add_field(object_layer &layer, jsonnet_node *field, const env_ptr &env);
        void add_local_binding(const env_ptr &env, jsonnet_node *bind);
        value_ptr format_string(const core::text::string &format, const value_ptr &args);
        value_ptr make_std();
        void add_native(object_layer &layer, core::text::basic_string_view<char> name,
                        functional::delegate<value_ptr(core::collections::vector<value_ptr> &)> fn);
        void manifest_value(const value_ptr &v, document &out);
        void collect_comprehension_envs(const core::collections::vector<jsonnet_node *> &clauses, const env_ptr &start,
                                        core::collections::vector<env_ptr> &out);

        const import_resolver *resolver_{nullptr};
    };
}

#endif

#endif
