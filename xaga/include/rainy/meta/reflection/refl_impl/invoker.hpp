/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_META_REFL_IMPL_INVOKER_HPP
#define RAINY_META_REFL_IMPL_INVOKER_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/object_view.hpp>
#include <rainy/utility/any.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable: 4702 4100 4715)
#endif

namespace rainy::meta::reflection {
    class function;

    using core::method_flags;
}

namespace rainy::meta::reflection::implements {
    template <typename... Args,std::size_t... I>
    static constexpr std::size_t eval_hash_code_helper(type_traits::helper::index_sequence<I...>) {
        return (0 + ... + (foundation::ctti::typeinfo::get_type_hash<Args>() * (I + 1)));
    }

    template <typename... Args>
    static constexpr std::size_t eval_hash_code() noexcept {
        return eval_hash_code_helper<Args...>(type_traits::helper::index_sequence_for<Args...>{});
    }

    template <typename ReturnType>
    static const foundation::ctti::typeinfo &return_type_res() noexcept {
        return rainy_typeid(ReturnType);
    }

    template <typename... Args>
    static collections::array<foundation::ctti::typeinfo, sizeof...(Args)> &param_types_res() noexcept {
        static collections::array<foundation::ctti::typeinfo, sizeof...(Args)> param_types = {
            foundation::ctti::typeinfo::create<Args>()...};
        return param_types;
    }

    template <typename FunctionSignature>
    static const foundation::ctti::typeinfo &function_signature_res() noexcept {
        static const foundation::ctti::typeinfo signature = foundation::ctti::typeinfo::create<FunctionSignature>();
        return signature;
    }

    template <typename... Args>
    static std::array<foundation::ctti::typeinfo, sizeof...(Args)> &generate_param_lists() {
        static std::array<foundation::ctti::typeinfo, sizeof...(Args)> res = {
            ::rainy::foundation::ctti::typeinfo::create<Args>()...};
        return res;
    }
}

namespace rainy::meta::reflection::implements {
    template <typename Fx, typename = void>
    struct is_fnobj : type_traits::helper::false_type {};

    template <typename Fx>
    struct is_fnobj<Fx, type_traits::other_trans::void_t<decltype(&Fx::operator())>> : type_traits::helper::true_type {};

    template <typename Fx, bool IsFnObj = is_fnobj<Fx>::value> // NOLINT
    struct extract_function_traits {
        static_assert(is_fnobj<Fx>::value, "Fx must be a callable object!");
        using type = type_traits::primary_types::function_traits<type_traits::cv_modify::remove_cvref_t<decltype(&Fx::operator())>>;
    };

    template <typename Fx>
    struct extract_function_traits<Fx, false> {
        using type = type_traits::primary_types::function_traits<Fx>;
    };

    template <typename Fx,typename... UAx>
    RAINY_INLINE utility::any access_invoke(Fx &&fn, void *object, UAx &&...args) {
        using traits = typename extract_function_traits<Fx>::type;
        static constexpr method_flags type = core::deduction_invoker_type<Fx, UAx...>();
        using namespace type_traits;
        if constexpr (constexpr method_flags flags = type; static_cast<bool>(flags & method_flags::static_specified)) {
            if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                utility::invoke(utility::forward<Fx>(fn), utility::forward<UAx>(args)...);
                return {};
            } else {
                return utility::invoke(utility::forward<Fx>(fn), utility::forward<UAx>(args)...);
            }
        } else {
            using instance_t = typename primary_types::member_pointer_traits<Fx>::class_type;
            if constexpr (traits::is_invoke_for_lvalue) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                    utility::forward<UAx>(args)...);
                    return {};
                } else {
                    return utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                           utility::forward<UAx>(args)...);
                }
            } else if constexpr (traits::is_invoke_for_rvalue) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                    utility::forward<UAx>(args)...);
                    return {};
                } else {
                    return utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                           utility::forward<UAx>(args)...);
                }
            } else {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t *>(object), utility::forward<UAx>(args)...);
                    return {};
                } else {
                    return utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t *>(object),
                                           utility::forward<UAx>(args)...);
                }
            }
        }
    }
    
    template <typename Type, typename UTy>
    RAINY_INLINE Type get_arg(UTy&& item) {
        if (utility::implements::is_as_runnable<Type>(item.type())) {
            return item.template as<Type>();
        }
        if constexpr (utility::is_any_convert_invocable<Type>) {
            return utility::any_converter<Type>::basic_convert(item.target_as_void_ptr(), item.type());
        }
        foundation::exceptions::cast::throw_bad_any_cast();
        std::terminate();
    }

    template <typename Fx, typename DefaultArguments, typename... Args>
    struct invoker {};

    template <typename Fx, typename... DArgs, typename... Args>
    struct invoker<Fx, default_arguments_store<DArgs...>, Args...> {
        using any = utility::any;
        using traits = typename extract_function_traits<Fx>::type;

        static constexpr std::size_t arity = traits::arity;
        static constexpr std::size_t default_arity = sizeof...(DArgs);
        static constexpr bool valid = traits::valid;

        static_assert(valid, "Fx must be a function!");
        static constexpr std::size_t param_hash = implements::eval_hash_code<Args...>();

        invoker() = default;
        ~invoker() = default;

        template <typename Functor,typename... UAx>
        explicit invoker(Functor &&fn, UAx &&...args) noexcept :
            fn(utility::forward<Functor>(fn)), arguments(utility::forward<UAx>(args)...) {
        }

        explicit invoker(const invoker &right) : fn(right.fn), arguments(right.arguments) {
        }

        explicit invoker(invoker &&right) noexcept : fn(utility::move(right.fn)), arguments(utility::move(right.arguments)) {
        }

        bool is_compatible(arg_view<> *view) const {
            const std::size_t size = view->size();
            static constexpr std::size_t least = arity - sizeof...(DArgs);
            return size >= least && size <= arity && is_compatible_impl(view, type_traits::helper::make_index_sequence<arity>{});
        }

        RAINY_NODISCARD bool is_compatible(collections::views::array_view<foundation::ctti::typeinfo> paramlist) const {
            const std::size_t size = paramlist.size();
            static constexpr std::size_t least = arity - sizeof...(DArgs);
            return size >= least && size <= arity && is_compatible_impl(paramlist, type_traits::helper::make_index_sequence<arity>{});
        }

        RAINY_NODISCARD bool is_compatible(collections::views::array_view<utility::any> paramlist) const {
            const std::size_t size = paramlist.size();
            static constexpr std::size_t least = arity - sizeof...(DArgs);
            return size >= least && size <= arity && is_compatible_impl(paramlist, type_traits::helper::make_index_sequence<arity>{});
        }

        /*---------------------*/

        template <std::size_t... I>
        bool is_compatible_impl(arg_view<> *view, type_traits::helper::index_sequence<I...>) const {
            static auto &target_paramlist = implements::param_types_res<Args...>();
            return ((I < view->size() ? view->at(I).type().is_compatible(target_paramlist[I]) : true) && ...);
        }

        template <std::size_t... I>
        bool is_compatible_impl(collections::views::array_view<foundation::ctti::typeinfo> paramlist,
                                type_traits::helper::index_sequence<I...>) const {
            static auto &target_paramlist = implements::param_types_res<Args...>();
            return ((I < paramlist.size() ? paramlist[I].is_compatible(target_paramlist[I]) : true) && ...); // NOLINT
        }

        template <std::size_t... I>
        bool is_compatible_impl(collections::views::array_view<utility::any> paramlist,
                                type_traits::helper::index_sequence<I...>) const {
            static auto &target_paramlist = implements::param_types_res<Args...>();
            return ((I < paramlist.size() ? paramlist[I].type().is_compatible(target_paramlist[I]) : true) && ...); // NOLINT
        }

        template <typename View, std::size_t... I>
        RAINY_INLINE any invoke_impl(void *object, View &items, type_traits::helper::index_sequence<I...>) {
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            return access_invoke(
                utility::forward<Fx>(fn), object,
                utility::forward<Args>(items[I].template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
        }

        template <typename View, std::size_t... I>
        RAINY_INLINE any invoke_with_conv_impl(void *object, View &items, type_traits::helper::index_sequence<I...>) {
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            return access_invoke(
                utility::forward<Fx>(fn), object,
                utility::forward<Args>(get_arg<typename type_traits::other_trans::type_at<I, tuple_t>::type>(items[I]))...);
        }

        template <typename View>
        RAINY_INLINE any invoke_with_defaults(void *object, View &items) {
            using full_list = type_traits::other_trans::type_list<Args...>;
            constexpr std::size_t N = sizeof...(Args);
            return apply_with_defaults(object, items, full_list{}, type_traits::helper::make_index_sequence<N>{});
        }

        template <typename View, typename TypeList, std::size_t... I>
        RAINY_INLINE any apply_with_defaults(void *object, View &items, TypeList, type_traits::helper::index_sequence<I...>) {
            return access_invoke(utility::forward<Fx>(fn), object, utility::forward<Args>(get_or_default<I, TypeList>(items))...);
        }

        template <std::size_t I, typename TypeList, typename Ty>
        RAINY_INLINE typename type_traits::other_trans::type_at<I, TypeList>::type get_or_default(Ty&& items) {
            using type = typename type_traits::other_trans::type_at<I, TypeList>::type;
            if (I < items.size()) {
                return get_arg<type>(items[I]);
            }
            if constexpr (I >= (arity - sizeof...(DArgs)) && I - (arity - sizeof...(DArgs)) < sizeof...(DArgs)) {
                return arguments.template get<I - (arity - sizeof...(DArgs))>();
            } else {
                throw std::runtime_error("Invalid default argument access");
            }
        }

        Fx fn;
        default_arguments_store<DArgs...> arguments;
        static constexpr method_flags type = core::deduction_invoker_type<Fx, Args...>();
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif