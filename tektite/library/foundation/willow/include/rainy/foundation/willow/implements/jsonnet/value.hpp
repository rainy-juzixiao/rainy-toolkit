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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_VALUE_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_VALUE_HPP
#include <cmath>
#include <cstdint>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/functional/delegate.hpp>
#include <rainy/core/memory/shared_ptr.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/willow/implements/common/operations.hpp>
#include <rainy/foundation/willow/implements/jsonnet/ast.hpp>
#include <rainy/foundation/willow/implements/jsonnet/exceptions.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    using namespace rainy::foundation::exceptions::willow;

    enum class value_kind {
        null_value,
        boolean,
        number,
        string,
        array,
        object,
        function,
    };

    struct value;
    struct object_value;
    struct function_value;
    struct thunk;
    struct environment;

    using value_ptr = core::memory::shared_ptr<value>;
    using object_ptr = core::memory::shared_ptr<object_value>;
    using function_ptr = core::memory::shared_ptr<function_value>;
    using thunk_ptr = core::memory::shared_ptr<thunk>;
    using env_ptr = core::memory::shared_ptr<environment>;

    struct thunk {
        functional::delegate<value_ptr()> compute{};
        value_ptr cached{};
        bool forced{false};
        bool evaluating{false};

        value_ptr force() {
            if (forced) {
                return cached;
            }
            if (evaluating) {
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("infinite recursion while forcing a value");
            }
            evaluating = true;
            cached = compute();
            forced = true;
            evaluating = false;
            return cached;
        }
    };

    struct field_slot {
        field_visibility visibility{field_visibility::visible};
        bool inherits{false};
        functional::delegate<value_ptr(const env_ptr &)> body{};
    };

    struct object_layer {
        env_ptr lexical_env{};
        core::collections::vector<core::text::string> names{};
        core::collections::vector<field_slot> slots{};
        core::collections::vector<jsonnet_node *> asserts{};
    };

    struct object_value {
        core::collections::vector<object_layer> layers{};
        bool asserts_run{false};

        const field_slot *find(const core::text::basic_string_view<char> name, std::size_t &layer_index) const noexcept {
            for (std::size_t i = layers.size(); i > 0; --i) {
                const auto &layer = layers[i - 1];
                for (std::size_t j = 0; j < layer.names.size(); ++j) {
                    if (layer.names[j] == name) {
                        layer_index = i - 1;
                        return &layer.slots[j];
                    }
                }
            }
            return nullptr;
        }

        field_visibility effective_visibility(const core::text::basic_string_view<char> name) const noexcept {
            field_visibility result = field_visibility::visible;
            bool found = false;
            for (const auto &layer: layers) {
                for (std::size_t j = 0; j < layer.names.size(); ++j) {
                    if (layer.names[j] == name) {
                        if (!found) {
                            result = layer.slots[j].visibility;
                            found = true;
                        } else if (layer.slots[j].visibility != field_visibility::visible) {
                            result = layer.slots[j].visibility;
                        }
                        break;
                    }
                }
            }
            return result;
        }

        core::collections::vector<core::text::string> field_names(const bool include_hidden) const {
            core::collections::vector<core::text::string> result;
            for (const auto &layer: layers) {
                for (const auto &name: layer.names) {
                    bool present = false;
                    for (const auto &existing: result) {
                        if (existing == name) {
                            present = true;
                            break;
                        }
                    }
                    if (present) {
                        continue;
                    }
                    const auto vis = effective_visibility(name);
                    if (include_hidden || vis != field_visibility::hidden) {
                        result.push_back(name);
                    }
                }
            }
            return result;
        }
    };

    struct function_value {
        core::collections::vector<core::text::string> params{};
        core::collections::vector<jsonnet_node *> defaults{};
        jsonnet_node *body{nullptr};
        env_ptr env{};
        functional::delegate<value_ptr(core::collections::vector<value_ptr> &)> native{};
        bool is_native{false};
    };

    struct value {
        value_kind kind{value_kind::null_value};
        bool boolean{false};
        double number{0.0};
        core::text::string str{};
        core::collections::vector<thunk_ptr> elements{};
        object_ptr object{};
        function_ptr function{};
    };

    struct environment {
        env_ptr parent{};
        core::collections::vector<core::container::pair<core::text::string, thunk_ptr>> bindings{};
        value_ptr self{};
        value_ptr super{};

        thunk_ptr *find(const core::text::basic_string_view<char> name) noexcept {
            for (auto &entry: bindings) {
                if (entry.first == name) {
                    return &entry.second;
                }
            }
            return parent != nullptr ? parent->find(name) : nullptr;
        }

        const value_ptr *find_self() const noexcept {
            if (self != nullptr) {
                return &self;
            }
            return parent != nullptr ? parent->find_self() : nullptr;
        }

        const value_ptr *find_super() const noexcept {
            if (super != nullptr) {
                return &super;
            }
            return parent != nullptr ? parent->find_super() : nullptr;
        }
    };

    inline value_ptr make_null() {
        return core::memory::make_shared<value>();
    }

    inline value_ptr make_boolean(const bool b) {
        value_ptr v = core::memory::make_shared<value>();
        v->kind = value_kind::boolean;
        v->boolean = b;
        return v;
    }

    inline value_ptr make_number(const double n) {
        value_ptr v = core::memory::make_shared<value>();
        v->kind = value_kind::number;
        v->number = n;
        return v;
    }

    inline value_ptr make_string(core::text::string s) {
        value_ptr v = core::memory::make_shared<value>();
        v->kind = value_kind::string;
        v->str = utility::move(s);
        return v;
    }

    inline value_ptr make_array(core::collections::vector<thunk_ptr> elements) {
        value_ptr v = core::memory::make_shared<value>();
        v->kind = value_kind::array;
        v->elements = utility::move(elements);
        return v;
    }

    inline value_ptr make_object(object_ptr object) {
        value_ptr v = core::memory::make_shared<value>();
        v->kind = value_kind::object;
        v->object = utility::move(object);
        return v;
    }

    inline value_ptr make_function(function_ptr function) {
        value_ptr v = core::memory::make_shared<value>();
        v->kind = value_kind::function;
        v->function = utility::move(function);
        return v;
    }

    inline thunk_ptr make_thunk(functional::delegate<value_ptr()> compute) {
        thunk_ptr t = core::memory::make_shared<thunk>();
        t->compute = utility::move(compute);
        return t;
    }

    inline thunk_ptr eager_thunk(value_ptr v) {
        thunk_ptr t = core::memory::make_shared<thunk>();
        t->cached = utility::move(v);
        t->forced = true;
        return t;
    }

    inline env_ptr make_env(env_ptr parent) {
        env_ptr env = core::memory::make_shared<environment>();
        env->parent = utility::move(parent);
        return env;
    }

    /**
     * \lang english
     * @brief Renders a number the way jsonnet manifests it.
     *
     * \lang simp-chinese
     * @brief 以 jsonnet 呈现数字的方式渲染数字。
     */
    inline core::text::string number_to_string(const double value) {
        if (value == static_cast<double>(static_cast<std::int64_t>(value)) && std::fabs(value) < 1e15) {
            char buffer[32]{};
            const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), static_cast<std::int64_t>(value));
            return core::text::string(buffer, static_cast<std::size_t>(result.ptr - buffer));
        }
        char buffer[64]{};
        const auto result = core::text::to_chars(buffer, buffer + sizeof(buffer), value, core::text::chars_format::general, 17);
        return core::text::string(buffer, static_cast<std::size_t>(result.ptr - buffer));
    }

    inline bool value_equals(const value &left, const value &right);

    inline bool array_equals(const value &left, const value &right) {
        if (left.elements.size() != right.elements.size()) {
            return false;
        }
        for (std::size_t i = 0; i < left.elements.size(); ++i) {
            if (!value_equals(*left.elements[i]->force(), *right.elements[i]->force())) {
                return false;
            }
        }
        return true;
    }

    inline bool value_equals(const value &left, const value &right) {
        if (left.kind == value_kind::number && right.kind == value_kind::number) {
            return left.number == right.number;
        }
        if (left.kind != right.kind) {
            return false;
        }
        switch (left.kind) {
            case value_kind::null_value:
                return true;
            case value_kind::boolean:
                return left.boolean == right.boolean;
            case value_kind::number:
                return left.number == right.number;
            case value_kind::string:
                return left.str == right.str;
            case value_kind::array:
                return array_equals(left, right);
            case value_kind::object:
                return left.object->field_names(false).size() == right.object->field_names(false).size();
            case value_kind::function:
                return false;
        }
        return false;
    }

    inline core::text::string value_to_string(const value &v) {
        switch (v.kind) {
            case value_kind::null_value:
                return "null";
            case value_kind::boolean:
                return v.boolean ? "true" : "false";
            case value_kind::number:
                return number_to_string(v.number);
            case value_kind::string:
                return v.str;
            case value_kind::array: {
                core::text::string result = "[";
                for (std::size_t i = 0; i < v.elements.size(); ++i) {
                    if (i != 0) {
                        result.append(", ");
                    }
                    result.append(value_to_string(*v.elements[i]->force()));
                }
                result.push_back(']');
                return result;
            }
            case value_kind::object:
                return "{ object }";
            case value_kind::function:
                exceptions::willow::jsonnet::throw_jsonnet_evaluate_error("cannot convert a function to a string");
        }
        return {};
    }

    inline core::text::basic_string_view<char> value_type_name(const value &v) noexcept {
        switch (v.kind) {
            case value_kind::null_value:
                return "null";
            case value_kind::boolean:
                return "boolean";
            case value_kind::number:
                return "number";
            case value_kind::string:
                return "string";
            case value_kind::array:
                return "array";
            case value_kind::object:
                return "object";
            case value_kind::function:
                return "function";
        }
        return "null";
    }
}

#endif

#endif
