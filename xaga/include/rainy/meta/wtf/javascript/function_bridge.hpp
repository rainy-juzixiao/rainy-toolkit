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
#ifndef RAINY_META_WTF_JAVASCRIPT_FUNCTION_HPP
#define RAINY_META_WTF_JAVASCRIPT_FUNCTION_HPP
#if __has_include("napi.h")
#include <napi.h>
#endif
#include <rainy/core/core.hpp>
#include <rainy/meta/wtf/javascript/js_callable.hpp>
#include <rainy/meta/wtf/javascript/type_resolver.hpp>

namespace rainy::meta::wtf::javascript::implements {
    template <typename T>
    struct is_js_callable : std::false_type {};

    template <>
    struct is_js_callable<js_callable> : std::true_type {};

    template <typename T>
    inline constexpr bool is_js_callable_v = is_js_callable<std::decay_t<T>>::value;

    template <typename T, std::size_t Index>
    std::decay_t<T> extract_any_argument(const node_abi::callback_info &info) {
        if constexpr (is_js_callable_v<T>) {
            return js_callable::sync(info, Index);
        } else {
            return extract_argument<std::decay_t<T>, Index>(info);
        }
    }

    template <typename T, std::size_t Index>
    std::decay_t<T> extract_any_argument_async(const node_abi::callback_info &info) {
        if constexpr (is_js_callable_v<T>) {
            return js_callable::async(info, Index);
        } else {
            return extract_argument<std::decay_t<T>, Index>(info);
        }
    }

    struct sync_tag {};
    struct async_tag {};

    template <typename ModeTag, typename TypeList>
    struct callback_binder_impl {};

    template <typename ModeTag, typename... Args>
    struct callback_binder_impl<ModeTag, type_traits::other_trans::type_list<Args...>> {
        template <std::size_t... Is>
        static std::tuple<std::decay_t<Args>...> collect(const node_abi::callback_info &info,
                                                         type_traits::helper::index_sequence<Is...>) {
            if constexpr (std::is_same_v<ModeTag, sync_tag>) {
                return std::make_tuple(extract_any_argument<Args, Is>(info)...);
            } else {
                return std::make_tuple(extract_any_argument_async<Args, Is>(info)...);
            }
        }

        template <typename Fn, std::size_t... Is>
        static node_abi::value invoke_sync(std::decay_t<Fn> *stored, const node_abi::callback_info &info,
                                           type_traits::helper::index_sequence<Is...>) {
            using ret = type_traits::primary_types::function_return_type<Fn>;
            auto args_tuple = collect(info, type_traits::helper::index_sequence<Is...>{});
            if constexpr (std::is_void_v<ret>) {
                std::apply([&](auto &&...a) { (*stored)(utility::forward<decltype(a)>(a)...); },
                           utility::move(args_tuple)); // ← move，让 js_callable 以右值展开
                return type_resolver<void>::to_js(info.Env());
            } else {
                return type_resolver<ret>::to_js(info.Env(),
                                                 std::apply([&](auto &&...a) { return (*stored)(utility::forward<decltype(a)>(a)...); },
                                                            utility::move(args_tuple))); // ← 同上
            }
        }

        template <typename Fn, std::size_t... Is>
        static void invoke_async(std::decay_t<Fn> *stored, std::tuple<std::decay_t<Args>...> args_tuple,
                                 type_traits::helper::index_sequence<Is...>) {
            using ret = type_traits::primary_types::function_return_type<Fn>;
            if constexpr (std::is_void_v<ret>) {
                std::apply([&](auto &&...a) { (*stored)(utility::forward<decltype(a)>(a)...); }, utility::move(args_tuple)); // ← move
            } else {
                std::apply([&](auto &&...a) { (*stored)(utility::forward<decltype(a)>(a)...); }, utility::move(args_tuple)); // ← move
            }
        }

        template <typename Fn>
        callback_binder_impl(node_abi::javascript_object exports, node_abi::script_environment env, std::string_view name, Fn &&fn) {
            auto *stored = new std::decay_t<Fn>(utility::forward<Fn>(fn));
            auto js_fn = Napi::Function::New(
                env,
                [stored](const node_abi::callback_info &info) -> node_abi::value {
                    if constexpr (std::is_same_v<ModeTag, sync_tag>) {
                        // 同步：直接在当前线程调用
                        return invoke_sync<Fn>(stored, info, type_traits::helper::index_sequence_for<Args...>{});
                    } else {
                        // 异步：收集参数后投到后台线程
                        auto packed = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                            collect(info, type_traits::helper::index_sequence_for<Args...>{}));
                        std::thread([stored, packed]() mutable {
                            invoke_async<Fn>(stored, utility::move(*packed), type_traits::helper::index_sequence_for<Args...>{});
                        }).detach();
                        return info.Env().Undefined();
                    }
                },
                name.data(), stored);
            napi_add_finalizer(
                env, js_fn, stored, [](napi_env, void *data, void *) { delete static_cast<std::decay_t<Fn> *>(data); }, nullptr,
                nullptr);
            exports.Set(name.data(), js_fn);
        }
    };
}

namespace rainy::meta::wtf::javascript {
    template <typename Fn>
    void bind_function(node_abi::javascript_object exports, node_abi::script_environment env, const char *name, Fn &&fn) {
        using arg_list = type_traits::primary_types::function_argument_list<Fn>;
        implements::callback_binder_impl<implements::sync_tag, arg_list>(exports, env, name, utility::forward<Fn>(fn));
    }

    template <typename Fn>
    void bind_async_function(node_abi::javascript_object exports, node_abi::script_environment env, const char *name, Fn &&fn) {
        using arg_list = type_traits::primary_types::function_argument_list<Fn>;
        implements::callback_binder_impl<implements::async_tag, arg_list>(exports, env, name, utility::forward<Fn>(fn));
    }
}

#endif
