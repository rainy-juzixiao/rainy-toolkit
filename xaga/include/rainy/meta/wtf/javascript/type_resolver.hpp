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
#ifndef RAINY_META_WTF_JAVASCRIPT_TYPE_RESOLVER_HPP
#define RAINY_META_WTF_JAVASCRIPT_TYPE_RESOLVER_HPP
#include <napi.h>
#include <rainy/core/core.hpp>

namespace rainy::meta::wtf::javascript::node_abi {
    using value = Napi::Value;
    using number = Napi::Number;
    using callback_info = Napi::CallbackInfo;
    using script_environment = Napi::Env;
    using javascript_object = Napi::Object;
}

namespace rainy::meta::wtf::javascript {
    // NOLINTBEGIN

    template <typename Type>
    struct type_resolver {};

    // NOLINTEND

    template <>
    struct type_resolver<int> {
        node_abi::value static to_js(Napi::Env env, const int v) {
            return node_abi::number::New(env, v);
        }

        static int from_js(const Napi::Value &v) {
            return v.As<node_abi::number>().Int32Value();
        }
    };

    template <>
    struct type_resolver<double> {
        static node_abi::value to_js(Napi::Env env, const double v) {
            return Napi::Number::New(env, v);
        }

        static double from_js(const Napi::Value &v) {
            return v.As<node_abi::number>().DoubleValue();
        }
    };

    template <>
    struct type_resolver<std::string> {
        static Napi::Value to_js(Napi::Env env, const std::string &v) {
            return Napi::String::New(env, v);
        }

        static std::string from_js(const Napi::Value &v) {
            return v.As<Napi::String>().Utf8Value();
        }
    };

    template <>
    struct type_resolver<bool> {
        static Napi::Value to_js(Napi::Env env, const bool v) {
            return Napi::Boolean::New(env, v);
        }
        static bool from_js(const Napi::Value &v) {
            return v.As<Napi::Boolean>().Value();
        }
    };

    template <>
    struct type_resolver<void> {
        static Napi::Value to_js(Napi::Env env) { // NOLINT
            return env.Undefined();
        }
    };

    template <typename Ty, std::size_t Index>
    Ty extract_argument(const node_abi::callback_info &info) {
        return type_resolver<type_traits::other_trans::decay_t<Ty>>::from_js(info[Index]); // NOLINT
    }
}

#endif
