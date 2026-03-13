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
#ifndef RAINY_META_WTF_NODE_BRIDGE_HPP
#define RAINY_META_WTF_NODE_BRIDGE_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/wtf/javascript/function_bridge.hpp>
#include <rainy/meta/wtf/javascript/js_callable.hpp>
#include <rainy/meta/wtf/javascript/type_resolver.hpp>

namespace rainy::meta::wtf::javascript::implements {
    template <typename Class, typename Ret, typename... Args, size_t... Is>
    node_abi::value invoke_met_impl(Class *obj, Ret (Class::*method)(Args...), const node_abi::callback_info &info,
                                    type_traits::helper::index_sequence<Is...>) {
        return type_resolver<Ret>::to_js(info.Env(), (obj->*method)(extract_argument<Args, Is>(info)...));
    }

    template <typename Class, typename... Args, size_t... Is>
    node_abi::value invoke_met_impl(Class *obj, void (Class::*method)(Args...), const node_abi::callback_info &info,
                                    type_traits::helper::index_sequence<Is...>) {
        (obj->*method)(extract_argument<Args, Is>(info)...);
        return type_resolver<void>::to_js(info.Env());
    }

    template <typename Class, typename Ret, typename... Args, size_t... Is>
    node_abi::value invoke_met_impl(Class *obj, Ret (Class::*method)(Args...) const, const node_abi::callback_info &info,
                                    type_traits::helper::index_sequence<Is...>) {
        return type_resolver<Ret>::to_js(info.Env(), (obj->*method)(extract_argument<Args, Is>(info)...));
    }

    template <typename Class, typename... Args, size_t... Is>
    node_abi::value invoke_met_impl(Class *obj, void (Class::*method)(Args...) const, const node_abi::callback_info &info,
                                    type_traits::helper::index_sequence<Is...>) {
        (obj->*method)(extract_argument<Args, Is>(info)...);
        return type_resolver<void>::to_js(info.Env());
    }
}

namespace rainy::meta::wtf::javascript {
    template <typename Class, typename Ret, typename... Args>
    node_abi::value invoke_method(Class *obj, Ret (Class::*method)(Args...), const node_abi::callback_info &info) {
        return implements::invoke_met_impl(obj, method, info, type_traits::helper::index_sequence_for<Args...>{});
    }

    template <typename Class, typename Ret, typename... Args>
    node_abi::value invoke_method(Class *obj, Ret (Class::*method)(Args...) const, const node_abi::callback_info &info) {
        return implements::invoke_met_impl(obj, method, info, type_traits::helper::index_sequence_for<Args...>{});
    }
}

#define RAINY_NODE_MODULE_IMPL(modname, mod_arg, env_arg)                                                                             \
    static rainy::meta::wtf::javascript::node_abi::javascript_object _rainy_node_init_##modname(                                      \
        Napi::Env env_arg, rainy::meta::wtf::javascript::node_abi::javascript_object mod_arg);                                        \
    NODE_API_MODULE(modname, _rainy_node_init_##modname)                                                                              \
    rainy::meta::wtf::javascript::node_abi::javascript_object _rainy_node_init_##modname(                                             \
        Napi::Env env_arg, rainy::meta::wtf::javascript::node_abi::javascript_object mod_arg)

#define RAINY_NODE_MODULE(modname, mod_arg, env_arg) RAINY_NODE_MODULE_IMPL(modname, mod_arg, env_arg)
#define RAINY_NODE_BIND_CLASS(modname, classname, exports_arg, env_arg) RAINY_NODE_MODULE_IMPL(modname, exports_arg, env_arg)

namespace rainy::meta::wtf::javascript {
    class RAINY_TOOLKIT_LOCAL_API environment {
    public:
        friend class registration;

        /**
         * @brief 设置NodeJS对当前模块的注册与环境
         * @param env
         * @param exports
         */
        static void setup(node_abi::script_environment &env, node_abi::javascript_object &exports) noexcept {
            auto o = instance();
            o->env = &env;
            o->exports = &exports;
        }

        static bool is_environment_already_setup() noexcept {
            auto o = instance();
            return o->env != nullptr && o->exports != nullptr;
        }

        static js_callable sync_function(std::string_view name) noexcept {
            assert(is_environment_already_setup());
            auto o = instance();
            node_abi::value maybe_fn = o->env->Global().Get(name.data());
            if (!maybe_fn) {
                return js_callable{};
            }
            return js_callable::sync(maybe_fn.As<Napi::Function>());
        }

        static js_callable async_function(std::string_view name) noexcept {
            assert(is_environment_already_setup());
            auto o = instance();
            node_abi::value maybe_fn = o->env->Global().Get(name.data());
            if (!maybe_fn) {
                return js_callable{};
            }
            return js_callable::async(maybe_fn.As<Napi::Function>(), *o->env);
        }

    private:
        RAINY_TOOLKIT_LOCAL_API static environment *instance() noexcept {
            static environment obj;
            return &obj;
        }

        node_abi::script_environment *env;
        node_abi::javascript_object *exports;
    };

    class RAINY_TOOLKIT_LOCAL_API registration {
    public:
        template <typename Type>
        class class_ {
        public:
            class_(const char *name) : env_(*environment::instance()->env), exports_(*environment::instance()->exports), name_(name) {
                const node_abi::value symbol = Napi::Symbol::New(env_, name);
                symbol_ref_ = std::make_shared<Napi::Reference<node_abi::value>>(Napi::Reference<node_abi::value>::New(symbol, 1));
            }

            template <typename... CtorArgs>
            class_ &constructor() {
                ctor_ = [](const Napi::CallbackInfo &info) -> Type * {
                    return construct_impl<CtorArgs...>(info, type_traits::helper::index_sequence_for<CtorArgs...>{});
                };
                return *this;
            }

            template <typename Ret, typename... Args>
            class_ &method(const char *name, Ret (Type::*method)(Args...)) {
                auto symbol_ref = symbol_ref_;
                methods_.push_back({name, [method, symbol_ref](const Napi::CallbackInfo &info) -> node_abi::value {
                                        auto *self = unwrap(info.This(), symbol_ref);
                                        return invoke_method(self, method, info);
                                    }});
                return *this;
            }

            template <typename Ret, typename... Args>
            class_ &method(const char *name, Ret (Type::*method)(Args...) const) {
                auto symbol_ref = symbol_ref_;
                methods_.push_back({name, [method, symbol_ref](const Napi::CallbackInfo &info) -> node_abi::value {
                                        auto *self = unwrap(info.This(), symbol_ref);
                                        return invoke_method(self, method, info);
                                    }});
                return *this;
            }

            template <typename T>
            class_ &read_only(const char *name, T Type::*field) {
                auto symbol_ref = symbol_ref_;
                getters_.push_back({name, [field, symbol_ref](const Napi::CallbackInfo &info) -> node_abi::value {
                                        const node_abi::value this_obj = info.This();
                                        auto *self = unwrap(this_obj, symbol_ref);
                                        return type_resolver<T>::to_js(info.Env(), self->*field);
                                    }});
                return *this;
            }

            template <typename Ty>
            class_ &property(const char *name, Ty Type::*field) {
                auto symbol_ref = symbol_ref_;
                getters_.push_back({name, [field, symbol_ref](const Napi::CallbackInfo &info) -> node_abi::value {
                                        auto *self = unwrap(info.This(), symbol_ref);
                                        return type_resolver<Ty>::to_js(info.Env(), self->*field);
                                    }});
                setters_.push_back({name, [field, symbol_ref](const Napi::CallbackInfo &info) {
                                        auto *self = unwrap(info.This(), symbol_ref);
                                        self->*field = type_resolver<Ty>::from_js(info[0]); // NOLINT
                                    }});
                return *this;
            }

            ~class_() {
                auto methods = std::make_shared<std::vector<method_entry>>(utility::move(methods_));
                auto getters = std::make_shared<std::vector<method_entry>>(utility::move(getters_));
                auto setters = std::make_shared<std::vector<setter_entry>>(utility::move(setters_));
                auto ctor_fn = std::make_shared<std::function<Type *(const Napi::CallbackInfo &)>>(utility::move(ctor_));
                auto symbol_ref = symbol_ref_;
                auto js_ctor = Napi::Function::New(
                    env_,
                    [ctor_fn, symbol_ref, getters, setters](const Napi::CallbackInfo &info) -> node_abi::value {
                        Napi::Env env = info.Env();
                        if (!info.IsConstructCall()) {
                            Napi::TypeError::New(env, "Must be called with new").ThrowAsJavaScriptException();
                            return env.Undefined();
                        }
                        Type *ptr = utility::invoke(*ctor_fn, info);
                        auto ext = Napi::External<Type>::New(env, ptr, [](Napi::Env, const Type *data) { delete data; });
                        rainy_let self = info.This().As<node_abi::javascript_object>();
                        self.Set(symbol_ref->Value(), ext);
                        const node_abi::javascript_object global = env.Global();
                        rainy_let define_prop =
                            global.Get("Object").As<node_abi::javascript_object>().Get("defineProperty").As<Napi::Function>();
                        for (auto &getter_entry: *getters) {
                            std::function<void(const Napi::CallbackInfo &)> setter_fn = nullptr;
                            for (auto &s: *setters) {
                                if (s.name == getter_entry.name) {
                                    setter_fn = s.fn;
                                    break;
                                }
                            }
                            auto getter_fn = getter_entry.fn;
                            auto prop_name = getter_entry.name;
                            auto js_getter = Napi::Function::New(
                                env, [getter_fn](const Napi::CallbackInfo &cb_info) -> node_abi::value { return getter_fn(cb_info); },
                                "");
                            node_abi::javascript_object descriptor = node_abi::javascript_object::New(env);
                            descriptor.Set("enumerable", Napi::Boolean::New(env, true));
                            descriptor.Set("configurable", Napi::Boolean::New(env, true));
                            descriptor.Set("get", js_getter);
                            if (setter_fn) {
                                auto js_setter = Napi::Function::New(
                                    env,
                                    [setter_fn](const Napi::CallbackInfo &cb_info) -> node_abi::value {
                                        setter_fn(cb_info);
                                        return cb_info.Env().Undefined(); // -> void
                                    },
                                    "");
                                descriptor.Set("set", js_setter);
                            }
                            define_prop.Call({self, Napi::String::New(env, prop_name), descriptor});
                        }
                        return env.Undefined();
                    },
                    name_);
                node_abi::javascript_object proto = js_ctor.Get("prototype").template As<node_abi::javascript_object>();
                for (auto &entry: *methods) {
                    auto fn = entry.fn;
                    proto.Set(entry.name, Napi::Function::New(
                                              env_, [fn](const Napi::CallbackInfo &info) -> node_abi::value { return fn(info); },
                                              entry.name.c_str()));
                }
                exports_.Set(name_, js_ctor);
            }

        private:
            static Type *unwrap(const node_abi::value val, const std::shared_ptr<Napi::Reference<node_abi::value>> &symbol_ref) {
                return val.As<node_abi::javascript_object>().Get(symbol_ref->Value()).As<Napi::External<Type>>().Data();
            }

            template <typename... CtorArgs, size_t... Is>
            static Type *construct_impl(const Napi::CallbackInfo &info, type_traits::helper::index_sequence<Is...>) {
                return new Type(extract_argument<CtorArgs, Is>(info)...);
            }

            struct method_entry {
                std::string name;
                std::function<node_abi::value(const Napi::CallbackInfo &)> fn;
            };
            struct setter_entry {
                std::string name;
                std::function<void(const Napi::CallbackInfo &)> fn;
            };

            Napi::Env env_;
            node_abi::javascript_object exports_;
            const char *name_;
            std::shared_ptr<Napi::Reference<node_abi::value>> symbol_ref_;
            std::vector<method_entry> methods_;
            std::vector<method_entry> getters_;
            std::vector<setter_entry> setters_;
            std::function<Type *(const Napi::CallbackInfo &)> ctor_;
        };

    private:
        RAINY_TOOLKIT_LOCAL_API static registration &instance() noexcept {
            assert(environment::is_environment_already_setup() && "You need setup environment");
            static registration obj;
            return obj;
        }
    };


}

#endif
