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
#ifndef RAINY_META_WTF_JAVASCRIPT_JS_CALLABLE_HPP
#define RAINY_META_WTF_JAVASCRIPT_JS_CALLABLE_HPP
#if __has_include("napi.h")
#include <napi.h>
#endif
#include <functional>
#include <memory>
#include <rainy/core/core.hpp>
#include <rainy/meta/wtf/javascript/type_resolver.hpp>
#include <vector>

namespace rainy::meta::wtf::javascript::implements {
    template <typename... Args, std::size_t... Is>
    std::vector<napi_value> pack_args_impl(node_abi::script_environment env, type_traits::helper::index_sequence<Is...>,
                                           const std::tuple<Args...> &args) {
        return {type_resolver<std::decay_t<std::tuple_element_t<Is, std::tuple<Args...>>>>::to_js(env, std::get<Is>(args))...};
    }

    template <typename... Args>
    std::vector<napi_value> pack_args(node_abi::script_environment env, const std::tuple<Args...> &args) {
        return pack_args_impl(env, type_traits::helper::index_sequence_for<Args...>{}, args);
    }
}

namespace rainy::meta::wtf::javascript {
    class js_sync_callable {
    public:
        explicit js_sync_callable(const node_abi::callback_info &info, const std::size_t arg_index = 0) :
            ref_(Napi::Persistent(info[arg_index].As<Napi::Function>())) { // NOLINT
            ref_.SuppressDestruct();
        }

        explicit js_sync_callable(const Napi::Function &fn) : ref_(Napi::Persistent(fn)) {
            ref_.SuppressDestruct();
        }

        static js_sync_callable from_global(const node_abi::script_environment env, const char *name) {
            return js_sync_callable{env.Global().Get(name).As<Napi::Function>()};
        }

        ~js_sync_callable() { // NOLINT
            ref_.Reset();
        }

        js_sync_callable(const js_sync_callable &) = delete;
        js_sync_callable &operator=(const js_sync_callable &) = delete;
        js_sync_callable(js_sync_callable &&o) noexcept = default;

        template <typename Ret = void, typename... Args>
        Ret call(Args &&...args) const {
            node_abi::script_environment env = ref_.Env();
            auto packed = std::make_tuple(utility::forward<Args>(args)...);
            const std::vector<napi_value> js_args = implements::pack_args(env, packed);
            node_abi::value result = ref_.Value().Call(env.Global(), js_args);
            if constexpr (std::is_void_v<Ret>) {
                return;
            } else {
                return type_resolver<Ret>::from_js(result);
            }
        }

        RAINY_NODISCARD node_abi::value call_raw(const std::vector<napi_value> &js_args) const {
            const node_abi::script_environment env = ref_.Env();
            return ref_.Value().Call(env.Global(), js_args);
        }

    private:
        Napi::FunctionReference ref_;
    };

    class js_async_callable {
    public:
        explicit js_async_callable(const node_abi::callback_info &info, const std::size_t arg_index = 0) :
            js_async_callable(info[arg_index].As<Napi::Function>(), info.Env()) { // NOLINT
        }

        js_async_callable(const Napi::Function &fn, const node_abi::script_environment env) :
            tsfn_(Napi::ThreadSafeFunction::New(env, fn, "rainy_toolkit::js_async_callable", 0, 1)) {
        }

        static js_async_callable from_global(node_abi::script_environment env, const char *name) {
            return {env.Global().Get(name).As<Napi::Function>(), env};
        }

        ~js_async_callable() {
            (void) tsfn_.Release();
        }

        js_async_callable(const js_async_callable &) = delete;
        js_async_callable &operator=(const js_async_callable &) = delete;
        js_async_callable(js_async_callable &&) = default;

        template <typename Ret, typename... Args>
        void call(std::function<void(Ret)> on_complete, Args &&...args) {
            static_assert(!std::is_void_v<Ret>, "void return: use fire-and-forget overload");
            auto packed = std::make_shared<std::tuple<std::decay_t<Args>...>>(utility::forward<Args>(args)...);
            auto cb = utility::move(on_complete);
            tsfn_.NonBlockingCall([packed, cb = utility::move(cb)](node_abi::script_environment env, Napi::Function js_fn) mutable {
                auto js_args = implements::pack_args(env, *packed);
                node_abi::value result = js_fn.Call(env.Global(), js_args);
                if (cb) {
                    cb(type_resolver<Ret>::from_js(result));
                }
            });
        }

        void call(std::function<void()> on_complete) const {
            auto cb = utility::move(on_complete);
            tsfn_.NonBlockingCall(
                [cb = utility::move(cb)](const node_abi::script_environment env, const Napi::Function js_fn) mutable {
                    js_fn.Call(env.Global(), std::vector<napi_value>{});
                    if (cb) {
                        cb();
                    }
                });
        }

        void call_raw(std::function<void(node_abi::value)> on_complete, std::vector<napi_value> js_args_snapshot) const {
            tsfn_.NonBlockingCall([js_args = utility::move(js_args_snapshot), cb = utility::move(on_complete)](
                                      const node_abi::script_environment env, const Napi::Function js_fn) mutable {
                const node_abi::value result = js_fn.Call(env.Global(), js_args);
                if (cb) {
                    cb(result);
                }
            });
        }

        template <typename First, typename... Rest>
        void call(First &&first, Rest &&...rest) {
            auto packed = std::make_shared<std::tuple<std::decay_t<First>, std::decay_t<Rest>...>>(utility::forward<First>(first),
                                                                                                   utility::forward<Rest>(rest)...);
            tsfn_.NonBlockingCall([packed](node_abi::script_environment env, Napi::Function js_fn) {
                auto js_args = implements::pack_args(env, *packed);
                js_fn.Call(env.Global(), js_args);
            });
        }

        void call() const {
            // NOLINTBEGIN
            tsfn_.NonBlockingCall(
                [](node_abi::script_environment env, Napi::Function js_fn) { js_fn.Call(env.Global(), std::vector<napi_value>{}); });
            // NOLINTEND
        }

        void release() { // NOLINT
            (void) tsfn_.Release();
        }

    private:
        Napi::ThreadSafeFunction tsfn_;
    };

    class js_callable {
    public:
        js_callable() = default;

        static js_callable sync(const node_abi::callback_info &info, std::size_t arg_index = 0) {
            js_callable c;
            c.sync_ = std::make_unique<js_sync_callable>(info, arg_index);
            return c;
        }

        static js_callable sync(const Napi::Function &fn) {
            js_callable c;
            c.sync_ = std::make_unique<js_sync_callable>(fn);
            return c;
        }

        static js_callable sync_global(const node_abi::script_environment env, const char *name) {
            js_callable c;
            c.sync_ = std::make_unique<js_sync_callable>(js_sync_callable::from_global(env, name));
            return c;
        }

        static js_callable async(const node_abi::callback_info &info, std::size_t arg_index = 0) {
            js_callable c;
            c.async_ = std::make_unique<js_async_callable>(info, arg_index);
            return c;
        }

        static js_callable async(const Napi::Function &fn, node_abi::script_environment env) {
            js_callable c;
            c.async_ = std::make_unique<js_async_callable>(fn, env);
            return c;
        }

        static js_callable async_global(const node_abi::script_environment env, const char *name) {
            js_callable c;
            c.async_ = std::make_unique<js_async_callable>(js_async_callable::from_global(env, name));
            return c;
        }

        template <typename Ret = void, typename... Args>
        Ret call(Args &&...args) const {
            assert(sync_ && "js_callable: use call() only in sync mode");
            return sync_->template call<Ret>(utility::forward<Args>(args)...);
        }

        RAINY_NODISCARD node_abi::value call_raw(std::vector<napi_value> js_args) const {
            assert(sync_ && "js_callable: use call_raw() only in sync mode");
            return sync_->call_raw(utility::move(js_args));
        }

        template <typename Ret = void, typename... Args>
        void call_async(std::function<void(Ret)> on_complete, Args &&...args) {
            assert(async_ && "js_callable: use call_async() only in async mode");
            async_->template call<Ret>(utility::move(on_complete), utility::forward<Args>(args)...);
        }

        template <typename... Args>
        void call_async(Args &&...args) {
            assert(async_ && "js_callable: use call_async() only in async mode");
            async_->call(utility::forward<Args>(args)...);
        }

        void release_async() {
            assert(async_ && "js_callable: release_async() only valid in async mode");
            async_->release();
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return sync_ == nullptr && async_ == nullptr;
        }

        RAINY_NODISCARD bool is_sync() const noexcept {
            return sync_ != nullptr;
        }

        RAINY_NODISCARD bool is_async() const noexcept {
            return async_ != nullptr;
        }

    private:
        std::unique_ptr<js_sync_callable> sync_;
        std::unique_ptr<js_async_callable> async_;
    };
}

#endif
