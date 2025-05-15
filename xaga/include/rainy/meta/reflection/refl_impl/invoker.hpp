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
#include <rainy/utility/any.hpp>
#include <rainy/meta/reflection/refl_impl/object_view.hpp>
#include <chrono>

namespace rainy::meta::reflection {
    class function;
}

namespace rainy::meta::reflection {
    enum class method_type {
        static_method,
        static_method_noexcept,
        normal_method,
        normal_method_noexcept,
        normal_method_left,
        normal_method_left_noexcept,
        normal_method_right,
        normal_method_right_noexcept,
        volatile_method,
        volatile_method_noexcept,
        volatile_method_left,
        volatile_method_right,
        volatile_method_left_noexcept,
        volatile_method_right_noexcept,
        const_method,
        const_method_noexcept,
        const_method_left,
        const_method_right,
        const_method_right_noexcept,
        const_method_left_noexcept,
        const_volatile_method,
        const_volatile_method_noexcept,
        const_volatile_method_left,
        const_volatile_method_left_noexcept,
        const_volatile_method_right,
        const_volatile_method_right_noexcept,
    };
}

namespace rainy::meta::reflection::implements {
    class arg_view {
    public:
        using view = collections::views::array_view<object_view>;
        using iterator = view::iterator;
        using const_iterator = view::const_iterator;
        using reference = object_view &;
        using const_reference = const object_view &;

        template <std::size_t N>
        arg_view(collections::array<object_view, N> &list) : args(list) {
        }

        iterator begin() noexcept {
            return args.begin();
        }

        iterator end() noexcept {
            return args.end();
        }

        reference operator[](std::size_t idx) noexcept {
            return args.at(idx);
        }

        const_reference &operator[](std::size_t idx) const noexcept {
            return args.at(idx);
        }

        reference at(std::size_t idx) noexcept {
            return args.at(idx);
        }

        const_reference at(std::size_t idx) const noexcept {
            return args.at(idx);
        }

        std::size_t size() const noexcept {
            return args.size();
        }

    private:
        view args;
    };

    template <std::size_t N>
    class arg_store {
    public:
        template <typename... Args>
        arg_store(Args &&...args_in) noexcept : args{make_object_view_helper(utility::forward<Args>(args_in))...} {
            static_assert(sizeof...(Args) == N, "Argument count mismatch with N");
        }

        arg_view to_argview() noexcept {
            return arg_view{args};
        }

        operator arg_view() && noexcept {
            return arg_view{args};
        }

    private:
        template <typename Ty>
        static object_view make_object_view_helper(Ty &&arg) noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)),
                                   foundation::rtti::typeinfo::of<other_trans::decay_t<Ty>>()};
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::rtti::typeinfo::of<Ty>()};
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return object_view{const_cast<void *>(static_cast<const void *>(arg)), foundation::rtti::typeinfo::of<Ty>()};
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return object_view{const_cast<void *>(arg.target_as_void_ptr()), arg.type(), true};
            } else {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::rtti::typeinfo::of<Ty>()};
            }
        }

        collections::array<object_view, N> args;
    };

    template <typename... Args>
    arg_store(Args...) -> arg_store<sizeof...(Args)>;
}

namespace rainy::meta::reflection::implements {
    template <typename... Args>
    static constexpr std::size_t eval_hash_code() noexcept {
        return (0 + ... + foundation::rtti::typeinfo::get_type_hash<Args>());
    }

    template <typename Fx, typename... Args>
    static constexpr method_type deduction_invoker_type() {
        using traits = type_traits::primary_types::function_traits<Fx>;
        if constexpr (!type_traits::primary_types::is_member_function_pointer_v<Fx>) {
            constexpr bool noexcept_invoke = noexcept(utility::invoke(utility::declval<Fx>(), utility::declval<Args>()...));
            return noexcept_invoke ? method_type::static_method_noexcept : method_type::static_method;
        } else {
            using method_traits = type_traits::primary_types::member_pointer_traits<Fx>;
            using raw_class_type = typename method_traits::class_type;
            constexpr bool noexcept_invoke =
                noexcept(utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type *>(), utility::declval<Args>()...));
            if constexpr (traits::is_const_member_function && traits::is_volatile) {
                if constexpr (traits::is_invoke_for_lvalue) {
                    return noexcept_invoke ? method_type::const_volatile_method_left_noexcept
                                           : method_type::const_volatile_method_left;
                } else if constexpr (traits::is_invoke_for_rvalue) {
                    return noexcept_invoke ? method_type::const_volatile_method_right_noexcept
                                           : method_type::const_volatile_method_right;
                } else {
                    return noexcept_invoke ? method_type::const_volatile_method_noexcept : method_type::const_volatile_method;
                }
            } else if constexpr (traits::is_const_member_function) {
                if constexpr (traits::is_invoke_for_lvalue) {
                    return noexcept_invoke ? method_type::const_method_left_noexcept : method_type::const_method_left;
                } else if constexpr (traits::is_invoke_for_rvalue) {
                    return noexcept_invoke ? method_type::const_method_right_noexcept : method_type::const_method_right;
                } else {
                    return noexcept_invoke ? method_type::const_method_noexcept : method_type::const_method;
                }
            } else if constexpr (traits::is_volatile) {
                if constexpr (traits::is_invoke_for_lvalue) {
                    return noexcept_invoke ? method_type::volatile_method_left_noexcept : method_type::volatile_method_left;
                } else if constexpr (traits::is_invoke_for_rvalue) {
                    return noexcept_invoke ? method_type::volatile_method_right_noexcept : method_type::volatile_method_right;
                } else {
                    return noexcept_invoke ? method_type::volatile_method_noexcept : method_type::volatile_method;
                }
            } else {
                if constexpr (traits::is_invoke_for_lvalue) {
                    return noexcept_invoke ? method_type::normal_method_left_noexcept : method_type::normal_method_left;
                } else if constexpr (traits::is_invoke_for_rvalue) {
                    return noexcept_invoke ? method_type::normal_method_right_noexcept : method_type::normal_method_right;
                } else {
                    return noexcept_invoke ? method_type::normal_method_noexcept : method_type::normal_method;
                }
            }
        }
    }

    template <typename ReturnType>
    static const foundation::rtti::typeinfo &return_type_res() noexcept {
        return rainy_typeid(ReturnType);
    }

    template <typename... Args>
    static const collections::array<foundation::rtti::typeinfo, sizeof...(Args)> &param_types_res() noexcept {
        static const collections::array<foundation::rtti::typeinfo, sizeof...(Args)> param_types = {
            foundation::rtti::typeinfo::create<Args>()...};
        return param_types;
    }

    template <typename FunctionSignature>
    static const foundation::rtti::typeinfo &function_signature_res() noexcept {
        static const foundation::rtti::typeinfo signature = foundation::rtti::typeinfo::create<FunctionSignature>();
        return signature;
    }

    template <typename... Args>
    static const collections::array<foundation::rtti::typeinfo, sizeof...(Args)> &generate_param_lists() {
        static const collections::array<foundation::rtti::typeinfo, sizeof...(Args)> res = {
            ::rainy::foundation::rtti::typeinfo::create<Args>()...};
        return res;
    }
}

namespace rainy::meta::reflection::implements {
    template <typename Fx, typename... Args>
    struct invoker {
        using any = utility::any;
        using traits = type_traits::primary_types::function_traits<Fx>;

        static constexpr std::size_t arity = traits::arity;
        static constexpr bool valid = traits::valid;

        static_assert(valid, "Fx must be a function!");

        static constexpr std::size_t param_hash = implements::eval_hash_code<Args...>();

        invoker() = default;

        invoker(Fx fn) noexcept : fn(fn) {
        }

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
        bool is_compatible(arg_view *view) const {
            return is_compatible_impl(view, type_traits::helper::make_index_sequence<arity>{});
        }

        bool is_compatible(collections::views::array_view<foundation::rtti::typeinfo> paramlist) const {
            return is_compatible_impl(paramlist, type_traits::helper::make_index_sequence<arity>{});
        }

        template <std::size_t... I>
        bool is_compatible_impl(arg_view *view, type_traits::helper::index_sequence<I...>) const {
            static auto &target_paramlist = implements::param_types_res<Args...>();
            return ((view->at(I).rtti().is_compatible(target_paramlist[I])) && ...);
        }

        template <std::size_t... I>
        bool is_compatible_impl(collections::views::array_view<foundation::rtti::typeinfo> paramlist,
                                type_traits::helper::index_sequence<I...>) const {
            static auto &target_paramlist = implements::param_types_res<Args...>();
            return ((paramlist[I].is_compatible(target_paramlist[I])) && ...);
        }

        utility::any invoke(void *object, arg_view *view) const {
            if (view->size() != arity) {
                errno = EINVAL;
                return {};
            }
            std::size_t args_hash =
                core::accumulate(view->begin(), view->end(), std::size_t{0},
                                 [](std::size_t acc, const object_view &item) { return acc + item.rtti().hash_code(); });
            if (args_hash == param_hash) {
                return invoke_impl(object, *view, type_traits::helper::make_index_sequence<arity>{});
            }
            if (is_compatible(view)) {
                return invoke_impl(object, *view, type_traits::helper::make_index_sequence<arity>{});
            }
            if constexpr ((utility::any_converter<Args>::convertible && ...)) {
                return invoke_impl_with_conv(object, *view, type_traits::helper::make_index_sequence<arity>{});
            } else {
                errno = EACCES;
                return {};
            }
        }

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

        template <std::size_t... I>
        utility::any invoke_impl(void *object, arg_view &items, type_traits::helper::index_sequence<I...>) const {
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            return access_invoke(
                object,
                utility::forward<Args>(
                    items[I].template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
        }

        template <std::size_t... I>
        utility::any invoke_impl_with_conv(void *object, arg_view &items, type_traits::helper::index_sequence<I...>) const {
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            return access_invoke(
                object, utility::forward<Args>(
                            utility::any_converter<typename type_traits::other_trans::type_at<I, tuple_t>::type>::basic_convert(
                                items[I].get_pointer(), items[I].rtti(), items[I].is_any()))...);
        }

        template <typename... UAx>
        utility::any access_invoke(void *object, UAx &&...args) const {
            constexpr auto method_type = type;
            if constexpr (method_type == method_type::static_method || method_type == method_type::static_method_noexcept) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, utility::forward<UAx>(args)...);
                    return {};
                } else {
                    return utility::invoke(fn, utility::forward<UAx>(args)...);
                }
            } else {
                using class_type = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, static_cast<class_type *>(object), utility::forward<UAx>(args)...);
                    return {};
                } else {
                    return utility::invoke(fn, static_cast<class_type *>(object), utility::forward<UAx>(args)...);
                }
            }
        }

        Fx fn;
        static constexpr method_type type = deduction_invoker_type<Fx, Args...>();
    };
}

namespace rainy::meta::reflection::implements {
    /**
     * @brief function使用的接口抽象层。用于统一访问函数对象，非库开发者不需要直接引用
    */
    struct invoker_accessor {
        virtual ~invoker_accessor() = default;
        virtual std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept = 0;
        virtual method_type type() const noexcept = 0;
        virtual utility::any invoke(object_view object) const = 0;
        virtual utility::any invoke(object_view object, arg_view arg_view) const = 0;
        virtual const foundation::rtti::typeinfo &which_belongs() const noexcept = 0;
        virtual const foundation::rtti::typeinfo &return_type() const noexcept = 0;
        virtual const foundation::rtti::typeinfo &function_signature() const noexcept = 0;
        virtual invoker_accessor *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;
        virtual const rainy::collections::views::array_view<foundation::rtti::typeinfo>& paramlists() const noexcept = 0;
        virtual bool equal_with(const invoker_accessor *impl) const noexcept = 0;
        virtual bool is_invocable(rainy::collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept = 0;
    };

    template <typename Fx, typename Class, typename ReturnType, typename... Args>
    struct invoker_accessor_impl final : implements::invoker_accessor {
        using function_signature_t = Fx;
        using storage_t = implements::invoker<Fx, Args...>;
        using typelist = type_traits::other_trans::type_list<Args...>;

        invoker_accessor_impl(Fx method) noexcept {
            utility::construct_at(&this->storage, method);
        }

        const foundation::rtti::typeinfo &return_type() const noexcept override final {
            return implements::return_type_res<ReturnType>();
        }

        const collections::views::array_view<foundation::rtti::typeinfo> &paramlists() const noexcept override final {
            static const collections::views::array_view<foundation::rtti::typeinfo> paramlist = implements::param_types_res<Args...>();
            return paramlist;
        }

        const foundation::rtti::typeinfo &function_signature() const noexcept override final {
            return implements::function_signature_res<function_signature_t>();
        }

        const foundation::rtti::typeinfo &which_belongs() const noexcept override final {
            return implements::which_belongs_res<Class>();
        }

        invoker_accessor *construct_from_this(core::byte_t *soo_buffer) const noexcept override final {
            return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), storage.fn);
        }

        method_type type() const noexcept override final {
            return storage.type;
        }

        bool equal_with(const invoker_accessor *impl) const noexcept override final {
            if (function_signature() != impl->function_signature()) {
                return false;
            }
            if (impl->type() != type()) {
                return false;
            }
            auto cast_impl = static_cast<const invoker_accessor_impl *>(impl);
            return storage.fn == cast_impl->storage.fn;
        }

        std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept override final {
            if (fx_sign != function_signature()) {
                if (fx_sign == rainy_typeid(function *)) {
                    return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
                }
                return 0;
            }
            return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
        }

        utility::any invoke(object_view object) const override final {
#if RAINY_ENABLE_DEBUG
            utility::expects(object.rtti().is_compatible(rainy_typeid(Class)));
#endif
            if constexpr (storage_t::arity == 0) {
                return storage.access_invoke(object.get_pointer());
            } else {
                return {};
            }
        }

        utility::any invoke(object_view object, arg_view arg_view) const override final {
#if RAINY_ENABLE_DEBUG
            utility::expects(object.rtti().is_compatible(rainy_typeid(Class)));
#endif
            return storage.invoke(object.get_pointer(), &arg_view);
        }

        bool is_invocable(collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept override final {
            if (storage.arity != paramlist.size()) {
                return false;
            }
            std::size_t paramhash =
                core::accumulate(paramlist.begin(), paramlist.end(), std::size_t{0},
                                 [&paramhash](std::size_t acc, const utility::any &item) { return acc + item.type().hash_code(); });
            if (paramhash == storage.param_hash) {
                return true;
            }
            if (storage.is_compatible(paramlist)) {
                return true;
            }
            return is_invocable_helper(paramlist, type_traits::helper::make_index_sequence<storage_t::arity>{});
        }

        template <std::size_t... I>
        bool is_invocable_helper(collections::views::array_view<foundation::rtti::typeinfo> paramlist,
                                 type_traits::helper::index_sequence<I...>) const noexcept {
            return (... && utility::any_converter<typename type_traits::other_trans::type_at<I, typelist>::type>::is_convertible(
                               paramlist[I]));
        }

        storage_t storage;
    };

    template <typename Fx, typename Traits,
              typename TypeList = typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
    struct get_ia_implement_type {};

    template <typename Fx, typename Traits, typename... Args>
    struct get_ia_implement_type<Fx, Traits, type_traits::other_trans::type_list<Args...>> {
        using memptr_traits = type_traits::primary_types::member_pointer_traits<Fx>;

        template <typename FxTraits, bool IsMemptr = FxTraits::valid>
        struct decl_class {
            using type = void;
        };

        template <typename FxTraits>
        struct decl_class<FxTraits, true> {
            using type = typename FxTraits::class_type;
        };

        using type = invoker_accessor_impl<Fx, typename decl_class<memptr_traits>::type, typename Traits::return_type, Args...>;
    };

    template <typename Fx, typename = void>
    struct try_to_get_invoke_operator : type_traits::helper::false_type {
        static RAINY_CONSTEXPR_BOOL is_lambda_without_capture = false;
    };

    template <typename Fx>
    struct try_to_get_invoke_operator<Fx, type_traits::other_trans::void_t<decltype(&Fx::operator())>>
        : type_traits::helper::true_type {
        template <typename Ty, typename = void>
        struct test_is_lambda_without_capture : type_traits::helper::false_type {};

        template <typename Ty>
        struct test_is_lambda_without_capture<
            Ty, type_traits::other_trans::void_t<decltype(+utility::declval<type_traits::cv_modify::remove_cv_t<Ty>>())>>
            : type_traits::helper::bool_constant<
                  type_traits::primary_types::is_pointer_v<decltype(+utility::declval<type_traits::cv_modify::remove_cv_t<Ty>>())>> {};

        template <typename Ty, typename Traits,
                  typename TypeList =
                      typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
        struct get_fn_obj_invoke_if_default_constructible {};

        template <typename Ty, typename Traits, typename... Args>
        struct get_fn_obj_invoke_if_default_constructible<Ty, Traits, type_traits::other_trans::type_list<Args...>> {
            static decltype(auto) invoke(Args... args) noexcept(Traits::is_noexcept) {
                static Ty fn_obj{};
                return fn_obj(utility::forward<Args>(args)...);
            };
        };

        static RAINY_CONSTEXPR_BOOL is_lambda_without_capture = test_is_lambda_without_capture<Fx>::value;

        static constexpr auto method = &Fx::operator();
    };
}

#endif