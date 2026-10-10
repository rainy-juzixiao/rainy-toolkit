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
#include <rainy/foundation/willow/implements/jsonnet/evaluator.hpp>
#include <rainy/foundation/willow/implements/jsonnet/parser.hpp>
#include <rainy/foundation/willow/implements/jsonnet/resolver.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    using namespace rainy::foundation::exceptions::willow;

    std::shared_ptr<value> resolver_binding::resolve_import(const core::text::string &path) const {
        if (resolver == nullptr || !resolver->import_source) {
            exceptions::willow::jsonnet::throw_jsonnet_import_error("no import resolver configured");
        }
        const core::text::string source = resolver->import_source(path);
        ast_arena arena;
        jsonnet_parser parser(source.data(), source.size(), arena);
        arena.root = parser.parse();
        evaluator nested{*resolver};
        return nested.evaluate(arena.root);
    }

    core::text::string resolver_binding::resolve_importstr(const core::text::string &path) const {
        if (resolver == nullptr || !resolver->importstr_source) {
            exceptions::willow::jsonnet::throw_jsonnet_import_error("no importstr resolver configured");
        }
        return resolver->importstr_source(path);
    }

    core::collections::vector<std::uint8_t> resolver_binding::resolve_importbin(const core::text::string &path) const {
        if (resolver == nullptr || !resolver->importbin_source) {
            exceptions::willow::jsonnet::throw_jsonnet_import_error("no importbin resolver configured");
        }
        return resolver->importbin_source(path);
    }

    value_ptr evaluator::make_std() {
        auto object = std::make_shared<object_value>();
        object_layer layer;
        layer.lexical_env = make_env(nullptr);
        add_native(layer, "length", [](core::collections::vector<value_ptr> &args) {
            return make_number(static_cast<double>(operations::length_value(args[0])));
        });
        add_native(layer, "type", [](core::collections::vector<value_ptr> &args) {
            return make_string(value_type_name(args[0]));
        });
        add_native(layer, "toString", [](core::collections::vector<value_ptr> &args) {
            return make_string(value_to_string(*args[0]));
        });
        object->layers.push_back(utility::move(layer));
        return make_object(utility::move(object));
    }
}

#endif
