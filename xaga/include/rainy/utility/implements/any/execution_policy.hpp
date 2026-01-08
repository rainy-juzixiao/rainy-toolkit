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
#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_EXECUTION_POLICY_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_EXECUTION_POLICY_HPP

// NOLINTBEGIN

#include <rainy/core/core.hpp>
#include <rainy/utility/implements/any/cast.hpp>
#include <rainy/utility/implements/any/fwd.hpp>
#include <rainy/utility/implements/any/iteator.hpp>

// NOLINTEND

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

namespace rainy::utility::implements {
    template <typename TargetType, std::size_t Idx, bool IsReference = type_traits::composite_types::is_reference_v<TargetType>>
    struct convert_any_binding_package {
        static decltype(auto) impl(const any_binding_package &pkg) {
            using namespace type_traits;
            if constexpr (type_relations::is_same_v<TargetType, placeholder_t>) {
                return utility::placeholder;
            } else if constexpr (type_relations::is_same_v<TargetType, std::in_place_t>) {
                return std::in_place;
            } else {
                if (implements::is_as_runnable<TargetType>(*pkg.type)) {
                    return implements::as_impl<TargetType>(pkg.payload, *pkg.type);
                }
                if constexpr (is_any_convert_invocable<TargetType>) {
                    if (any_converter<TargetType>::is_convertible(*pkg.type)) {
                        return any_converter<TargetType>::basic_convert(pkg.payload, *pkg.type);
                    }
                } else if constexpr (is_any_convert_invocable<cv_modify::remove_cv<TargetType>>) {
                    if (any_converter<cv_modify::remove_cv_t<TargetType>>::is_convertible(*pkg.type)) {
                        return any_converter<cv_modify::remove_cv_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                    }
                } else if constexpr (is_any_convert_invocable<cv_modify::remove_cvref_t<TargetType>>) {
                    if (any_converter<cv_modify::remove_cvref_t<TargetType>>::is_convertible(*pkg.type)) {
                        return any_converter<cv_modify::remove_cvref_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                    }
                }
                if (foundation::ctti::is_convertible_to(*pkg.type, rainy_typeid(TargetType))) {
                    return foundation::ctti::dynamic_convert<TargetType>(pkg.payload, *pkg.type);
                }
                std::terminate();
            }
        }
    };

    template <typename TargetType, std::size_t Idx>
    struct convert_any_binding_package<TargetType, Idx, false> {
        static auto impl(const any_binding_package &pkg) {
            using namespace type_traits;
            if constexpr (type_relations::is_same_v<TargetType, placeholder_t>) {
                return placeholder;
            } else if constexpr (type_relations::is_same_v<TargetType, std::in_place_t>) {
                return std::in_place;
            } else {
                if (implements::is_as_runnable<TargetType>(*pkg.type)) {
                    return implements::as_impl<TargetType>(pkg.payload, *pkg.type);
                }
                if constexpr (is_any_convert_invocable<TargetType>) {
                    if (any_converter<TargetType>::is_convertible(*pkg.type)) {
                        return any_converter<TargetType>::basic_convert(pkg.payload, *pkg.type);
                    }
                } else if constexpr (is_any_convert_invocable<cv_modify::remove_cv<TargetType>>) {
                    if (any_converter<cv_modify::remove_cv_t<TargetType>>::is_convertible(*pkg.type)) {
                        return any_converter<cv_modify::remove_cv_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                    }
                } else if constexpr (is_any_convert_invocable<cv_modify::remove_cvref_t<TargetType>>) {
                    if (any_converter<cv_modify::remove_cvref_t<TargetType>>::is_convertible(*pkg.type)) {
                        return any_converter<cv_modify::remove_cvref_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                    }
                }
                if (foundation::ctti::is_convertible_to(*pkg.type, rainy_typeid(TargetType))) {
                    return foundation::ctti::dynamic_convert<TargetType>(pkg.payload, *pkg.type);
                }
                std::terminate();
            }
        }
    };
}

namespace rainy::utility::implements {
    template <typename Rx, typename Ty, typename Any1, typename Any2, typename Func>
    Rx any_magic_method_helper(Any1 &&left, Any2 &&right, Func &&func) {
        auto &left_operand = left.template as<Ty>();
        if (implements::is_as_runnable<Ty>(right.type().remove_cvref())) {
            return utility::invoke(utility::forward<Func>(func), left_operand, right.template as<Ty>());
        }
        if (right.template is_convertible<Ty>()) {
            return utility::invoke(utility::forward<Func>(func), left_operand, right.template convert<Ty>());
        }
        return Rx{};
    }
}

namespace rainy::utility {
    template <typename Ty, typename Any = basic_any<>, bool WithEqual = false>
    struct any_magic_method {
        using any = Any;

        static inline constexpr bool not_bool =
            !type_traits::type_relations::is_same_v<bool, type_traits::cv_modify::remove_cvref_t<Ty>>;

        any add(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_add_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::plus{});
            }
            return {};
        }

        any subtract(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::minus{});
            }
            return {};
        }

        any incr_prefix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_preinc_v<Ty> && not_bool) {
                auto &left_operand = left.template as<Ty>();
                ++left_operand;
                return any{std::in_place_type<decltype(--left_operand)>, left_operand};
            }
            return {};
        }

        any decr_prefix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_predec_v<Ty> && not_bool) {
                auto &left_operand = left.template as<Ty>();
                return any{std::in_place_type<decltype(--left_operand)>, --left_operand};
            }
            return {};
        }

        any incr_postfix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_postinc_v<Ty> && not_bool) {
                auto &left_operand = left.template as<Ty>();
                return left_operand++;
            }
            return {};
        }

        any decr_postfix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_postdec_v<Ty> && not_bool) {
                auto &left_operand = left.template as<Ty>();
                return left_operand--;
            }
            return {};
        }

        any multiply(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mul_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::multiplies{});
            }
            return {};
        }

        any divide(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_div_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::divides{});
            }
            return {};
        }

        any mod(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mod_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::modulus{});
            }
            return {};
        }

        bool compare_less(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_lt_v<Ty>) {
                return implements::any_magic_method_helper<bool, Ty>(left, right, foundation::functional::less{});
            }
            return false;
        }

        bool compare_less_equal(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_le_v<Ty>) {
                return implements::any_magic_method_helper<bool, Ty>(left, right, foundation::functional::less_equal{});
            }
            return false;
        }

        bool compare_equal(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_eq_v<Ty>) {
                return implements::any_magic_method_helper<bool, Ty>(left, right, foundation::functional::equal{});
            }
            return false;
        }

        bool compare_greater_equal(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_gt_v<Ty>) {
                return implements::any_magic_method_helper<bool, Ty>(left, right, foundation::functional::greater_equal{});
            }
            return false;
        }

        bool compare_greater(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_ge_v<Ty>) {
                return implements::any_magic_method_helper<bool, Ty>(left, right, foundation::functional::greater{});
            }
            return false;
        }
    };

    template <typename Ty, typename Any>
    struct any_magic_method<Ty, Any, true> {
        using any = Any;

        static inline constexpr bool not_bool =
            !type_traits::type_relations::is_same_v<bool, type_traits::cv_modify::remove_cvref_t<Ty>>;

        any add(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_plus_equal_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft += myright; });
            }
            return {};
        }

        any subtract(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_equal_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft -= myright; });
            }
            return {};
        }

        any multiply(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mul_equal_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft *= myright; });
            }
            return {};
        }

        any divide(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_div_equal_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft /= myright; });
            }
            return {};
        }

        any mod(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mod_equal_v<Ty> && not_bool) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft %= myright; });
            }
            return {};
        }
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_addable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL
        is_any_addable_v<Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::add)>> =
            true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL has_any_plus_equal_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL has_any_plus_equal_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::plus_equal)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_subable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_subable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::subtract)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_multable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_multable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::multiply)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_divable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_divable_v<Ty, Any, WithEqual,
                                          type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::divide)>> =
        true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_modable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL
        is_any_modable_v<Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::mod)>> =
            true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_preincable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_preincable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::incr_prefix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_predecable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_predecable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::decr_prefix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_postincable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_postincable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::incr_postfix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_postdecable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_postdecable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::decr_postfix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_less_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_less_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::compare_less)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_less_eq_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_less_eq_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::compare_less_equal)>> =
        true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_eq_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_eq_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::compare_equal)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_gt_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_gt_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::compare_greater)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_greater_eq_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_greater_eq_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_magic_method<Ty, Any, WithEqual>::compare_greater_equal)>> =
        true;
}

#define ANY_DO_BINARY_OPERATOR_OPERATION_LAYER(traits, method, operation)                                                             \
    template <typename Ty, typename Any>                                                                                              \
    struct do_binary_operator_operation_layer<Ty, Any, operation> {                                                                   \
        static constexpr rain_fn get_equal_for_ptr() -> auto {                                                                        \
            constexpr bool has_equal_for = traits<Ty, Any, true>;                                                                     \
            if constexpr (has_equal_for) {                                                                                            \
                return &any_magic_method<Ty, Any, true>::method;                                                                            \
            } else {                                                                                                                  \
                return nullptr;                                                                                                       \
            }                                                                                                                         \
        }                                                                                                                             \
        static constexpr rain_fn get_ptr() -> auto {                                                                                  \
            constexpr bool has_ptr = traits<Ty, Any, false>;                                                                          \
            if constexpr (has_ptr) {                                                                                                  \
                return &any_magic_method<Ty, Any, false>::method;                                                                     \
            } else {                                                                                                                  \
                return nullptr;                                                                                                       \
            }                                                                                                                         \
        }                                                                                                                             \
        static rain_fn invoke(void *const data) {                                                                                     \
            constexpr auto equal_for_ptr = get_equal_for_ptr();                                                                       \
            constexpr auto ptr = get_ptr();                                                                                           \
            do_binary_operator_operation_impl<ptr, equal_for_ptr, Any, Ty>(data);                                                     \
        }                                                                                                                             \
    };

namespace rainy::utility::implements {
    template <auto WhenIsFalseMemPtr, auto WhenIsTrueMemPtr, typename Any, typename RemoveCVRef>
    void do_binary_operator_operation_impl(void *const data) {
        using remove_cvref_t = RemoveCVRef;
        using any = Any;
        auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */> *>(data);
        auto &left = *std::get<0>(*res);
        auto &right = *std::get<1>(*res);
        auto &recv = *std::get<2>(*res);
        constexpr bool can_equal_for = !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<decltype(WhenIsTrueMemPtr)>, std::nullptr_t>;
        constexpr bool is_avaiable =
            !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<decltype(WhenIsFalseMemPtr)>,
                                                    std::nullptr_t>;
        if (&recv == &left) {
            if constexpr (can_equal_for) {
                static any_magic_method<remove_cvref_t, any, true> obj{};
                utility::invoke(WhenIsTrueMemPtr, obj, recv, right);
            }
        }
        if constexpr (is_avaiable) {
            static any_magic_method<remove_cvref_t, any, false> obj{};
            recv = utility::invoke(WhenIsFalseMemPtr, obj, left, right);
        }
    }

    template <typename Ty, typename Any, any_execution_policy::operation Operation>
    struct do_binary_operator_operation_layer {};

    ANY_DO_BINARY_OPERATOR_OPERATION_LAYER(is_any_addable_v, add, any_execution_policy::operation::add);
    ANY_DO_BINARY_OPERATOR_OPERATION_LAYER(is_any_subable_v, subtract, any_execution_policy::operation::subtract);
    ANY_DO_BINARY_OPERATOR_OPERATION_LAYER(is_any_multable_v, multiply, any_execution_policy::operation::multiply);
    ANY_DO_BINARY_OPERATOR_OPERATION_LAYER(is_any_divable_v, divide, any_execution_policy::operation::divide);
    ANY_DO_BINARY_OPERATOR_OPERATION_LAYER(is_any_modable_v, mod, any_execution_policy::operation::mod);

    template <bool IsCompareable, auto MemPtr, typename Any, typename RemoveCVRef>
    bool do_compare_operater_operation_impl(void *const data) {
        using any = Any;
        using pack = const std::tuple<const any *, const any *, any_compare_operation> *;
        const auto *res = static_cast<pack>(data);
        rainy_const left = std::get<0>(*res);
        rainy_const right = std::get<1>(*res);
        if constexpr (IsCompareable) {
            static any_magic_method<RemoveCVRef, any> obj{};
            return utility::invoke(MemPtr, obj, *left, *right);
        } else {
            foundation::exceptions::logic::throw_any_not_implemented("Current type not support this operation");
        }
        return false;
    }

    template <typename Ty, typename Any, any_compare_operation Operation>
    bool do_compare_operater_operation(void *const data) {
        using operation = any_compare_operation;
        using remove_cvref_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        if constexpr (Operation == operation::less) {
            return do_compare_operater_operation_impl<is_any_less_compareable_v<remove_cvref_t, Any>,
                                                      &any_magic_method<remove_cvref_t, Any>::compare_less, Any, remove_cvref_t>(data);
        } else if constexpr (Operation == operation::less_eq) {
            return do_compare_operater_operation_impl<is_any_less_eq_compareable_v<remove_cvref_t, Any>,
                                                      &any_magic_method<remove_cvref_t, Any>::compare_less_equal, Any, remove_cvref_t>(
                data);
        } else if constexpr (Operation == operation::eq) {
            return do_compare_operater_operation_impl<is_any_eq_compareable_v<remove_cvref_t, Any>,
                                                      &any_magic_method<remove_cvref_t, Any>::compare_equal, Any, remove_cvref_t>(
                data);
        } else if constexpr (Operation == operation::greater_eq) {
            return do_compare_operater_operation_impl<is_any_greater_eq_compareable_v<remove_cvref_t, Any>,
                                                      &any_magic_method<remove_cvref_t, Any>::compare_greater_equal, Any,
                                                      remove_cvref_t>(data);
        } else if constexpr (Operation == operation::greater) {
            return do_compare_operater_operation_impl<is_any_gt_compareable_v<remove_cvref_t, Any>,
                                                      &any_magic_method<remove_cvref_t, Any>::compare_greater, Any, remove_cvref_t>(
                data);
        }
        return true;
    }

    template <typename Ty, typename Any>
    bool get_executer_impl_get_begin(void *const data) {
        using any = Any;
        using remove_cvref_t = Ty;
        if constexpr (type_traits::extras::iterators::has_iterator_v<remove_cvref_t>) {
            using tuple_t =
                std::tuple<bool /* is const */, typename any::reference * /* value */, typename any::iterator * /* recv */>;
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
            using const_as = type_traits::cv_modify::add_const_t<remove_ref_t>;
            using iterator = any_proxy_iterator<any, remove_cvref_t>;
            auto *res = static_cast<tuple_t *>(data);
            bool is_const = std::get<0>(*res);
            rainy_let *value = std::get<1>(*res);
            auto *recv = std::get<2>(*res);
            if constexpr (type_traits::extras::iterators::has_const_iterator_v<remove_ref_t>) {
                if (is_const || value->type().is_const()) {
                    using const_iterator = const_any_proxy_iterator<any, remove_cvref_t>;
                    if constexpr (type_traits::extras::meta_method::has_cbegin_v<remove_ref_t>) {
                        utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().cbegin());
                    } else if constexpr (type_traits::extras::meta_method::has_begin_v<const_as>) {
                        utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().begin());
                    }
                }
            }
            if constexpr (type_traits::extras::meta_method::has_begin_v<remove_ref_t>) {
                if (!is_const && !value->type().is_const()) {
                    if constexpr (!type_traits::type_properties::is_const_v<remove_ref_t>) {
                        utility::construct_at(recv, std::in_place_type<iterator>, value->template as<remove_ref_t>().begin());
                    }
                }
            }
            return !recv->empty();
        }
        return false;
    }

    template <typename Ty, typename Any>
    bool get_executer_impl_get_end(void *const data) {
        using any = Any;
        using remove_cvref_t = Ty;
        if constexpr (type_traits::extras::iterators::has_iterator_v<Ty>) {
            using tuple_t =
                std::tuple<bool /* is const */, typename any::reference * /* value */, typename any::iterator * /* recv */>;
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
            using const_as = type_traits::cv_modify::add_const_t<remove_ref_t>;
            using iterator = any_proxy_iterator<any, remove_cvref_t>;
            auto *res = static_cast<tuple_t *>(data);
            bool is_const = std::get<0>(*res);
            rainy_let *value = std::get<1>(*res);
            auto *recv = std::get<2>(*res);
            if constexpr (type_traits::extras::iterators::has_const_iterator_v<remove_ref_t>) {
                if (is_const || value->type().is_const()) {
                    using const_iterator = const_any_proxy_iterator<any, remove_cvref_t>;
                    if constexpr (type_traits::extras::meta_method::has_cend_v<remove_ref_t>) {
                        utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().cend());
                    } else if constexpr (type_traits::extras::meta_method::has_end_v<const_as>) {
                        utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().end());
                    }
                }
            }
            if constexpr (type_traits::extras::meta_method::has_end_v<remove_ref_t>) {
                if (!is_const && !value->type().is_const()) {
                    if constexpr (!type_traits::type_properties::is_const_v<remove_ref_t>) {
                        utility::construct_at(recv, std::in_place_type<iterator>, value->template as<Ty>().end());
                    }
                }
            }
            return !recv->empty();
        }
        return false;
    }
}

namespace rainy::utility::implements {
    template <typename Ty, typename BasicAnyImpl>
    bool any_execution_policy::invoke_impl(operation op, void *const data) {
        using namespace type_traits;
        using namespace type_traits::other_trans;
        using namespace type_traits::composite_types;
        using namespace type_traits::primary_types;
        using namespace type_traits::reference_modify;
        using any = BasicAnyImpl;
        using remove_cvref_t = cv_modify::remove_cvref_t<Ty>;
        switch (op) {
            case operation::compare: {
                using namespace foundation::exceptions::logic;
                rainy_let res = static_cast<
                    const std::tuple<const typename any::reference *, const typename any::reference *, any_compare_operation> *>(data);
                switch (std::get<2>(*res)) {
                    case any_compare_operation::less:
                        return do_compare_operater_operation<remove_cvref_t, any, any_compare_operation::less>(data);
                    case any_compare_operation::less_eq:
                        return do_compare_operater_operation<remove_cvref_t, any, any_compare_operation::less_eq>(data);
                    case any_compare_operation::eq:
                        return do_compare_operater_operation<remove_cvref_t, any, any_compare_operation::eq>(data);
                    case any_compare_operation::greater_eq:
                        return do_compare_operater_operation<remove_cvref_t, any, any_compare_operation::greater_eq>(data);
                    case any_compare_operation::greater:
                        return do_compare_operater_operation<remove_cvref_t, any, any_compare_operation::greater>(data);
                    default:
                        break;
                }
                break;
            }
            case operation::eval_hash: {
                auto *res = static_cast<std::tuple<const typename any::reference *, std::size_t *> *>(data);
                rainy_const object = std::get<0>(*res);
                std::size_t *hashcode = std::get<1>(*res);
                if constexpr (is_support_standard_hasher_available<Ty>::value) {
                    static std::hash<Ty> hasher;
                    *hashcode = hasher(object->template as<Ty>());
                    return true;
                } else if constexpr (is_support_rainytoolkit_hasher_available<Ty>::value) {
                    static utility::hash<Ty> hasher; // NOLINT
                    *hashcode = hasher(object->template as<Ty>());
                    return true;
                } else {
                    return false;
                }
            }
            case operation::query_for_is_tuple_like: {
                if constexpr (constexpr std::size_t member_count = member_count_v<type_traits::cv_modify::remove_cvref_t<Ty>>;
                              member_count == 0) {
                    return false;
                } else {
                    (*static_cast<std::size_t *>(data)) = member_count;
                }
                return true;
            }
            case operation::destructre_this_pack: {
                using implements::any_binding_package;
                if constexpr (constexpr std::size_t member_count = member_count_v<remove_cvref_t>; member_count != 0) {
                    auto *res = static_cast<std::tuple<any *, bool, collections::views::array_view<any_binding_package>> *>(data);
                    any *object = std::get<0>(*res);
                    bool use_const = std::get<1>(*res);
                    auto &view = std::get<2>(*res);
                    if (use_const) {
                        auto tuple_ptr = utility::struct_bind_tuple(std::as_const(object)->template as<remove_cvref_t>());
                        std::apply(
                            [&](auto *...elems) {
                                std::size_t idx = 0;
                                ((view[idx++] =
                                      any_binding_package{static_cast<const void *>(elems), &rainy_typeid(decltype(*elems))}),
                                 ...);
                                (void) idx;
                            },
                            tuple_ptr);
                    } else {
                        auto tuple_ptr = utility::struct_bind_tuple(object->template as<remove_cvref_t>());
                        std::apply(
                            [&](auto *...elems) {
                                std::size_t idx = 0;
                                ((view[idx++] =
                                      any_binding_package{static_cast<const void *>(elems), &rainy_typeid(decltype(*elems))}),
                                 ...);
                                (void) idx;
                            },
                            tuple_ptr);
                    }
                    return true;
                }
                break;
            }
            case operation::output_any: {
                auto *res = static_cast<std::tuple<bool /* is_char/is_wchar_t */, void * /* params */> *>(data);
                bool is_char = std::get<0>(*res);
                void *output_data = std::get<1>(*res);
                if (is_char) {
                    if constexpr (is_char_any_can_output<Ty>) {
                        auto *out =
                            static_cast<std::tuple<std::basic_ostream<char> * /* ostream */, const any * /* any */> *>(output_data);
                        (*std::get<0>(*out)) << std::get<1>(*out)->template as<Ty>();
                        return true;
                    }
                } else {
                    if constexpr (is_wchar_any_can_output<Ty>) {
                        auto *out =
                            static_cast<std::tuple<std::basic_ostream<wchar_t> * /* ostream */, const any * /* any */> *>(output_data);
                        (*std::get<0>(*out)) << std::get<1>(*out)->template as<Ty>();
                        return true;
                    }
                }
                break;
            }
            case operation::incr_prefix: {
                if constexpr (is_any_preincable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_magic_method<remove_cvref_t, any>{}.incr_prefix(left);
                    return true;
                }
                break;
            }
            case operation::decr_prefix: {
                if constexpr (is_any_predecable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_magic_method<remove_cvref_t, any>{}.decr_prefix(left);
                    return true;
                }
                break;
            }
            case operation::incr_postfix: {
                if constexpr (is_any_postincable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_magic_method<remove_cvref_t, any>{}.incr_postfix(left);
                    return true;
                }
                break;
            }
            case operation::decr_postfix: {
                if constexpr (is_any_postdecable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_magic_method<remove_cvref_t, any>{}.decr_postfix(left);
                    return true;
                }
                break;
            }
            case operation::add: {
                do_binary_operator_operation_layer<remove_cvref_t, any, operation::add>::invoke(data);
                break;
            }
            case operation::subtract: {
                do_binary_operator_operation_layer<remove_cvref_t, any, operation::subtract>::invoke(data);
                break;
            }
            case operation::multiply: {
                do_binary_operator_operation_layer<remove_cvref_t, any, operation::multiply>::invoke(data);
                break;
            }
            case operation::divide: {
                do_binary_operator_operation_layer<remove_cvref_t, any, operation::divide>::invoke(data);
                break;
            }
            case operation::mod: {
                do_binary_operator_operation_layer<remove_cvref_t, any, operation::mod>::invoke(data);
                break;
            }
            case operation::dereference: {
                using namespace type_traits::extras::meta_method;
                auto *res = static_cast<std::tuple<bool /* is_const */, any * /* left */, any * /* recv */
                                                   > *>(data);
                bool is_const = std::get<0>(*res);
                auto &value = (*std::get<1>(*res)).template as<Ty>();
                any *recv = std::get<2>(*res);
                if (is_const) {
                    if constexpr (has_operator_deref_v<type_traits::cv_modify::add_const_t<Ty>>) {
                        recv->template emplace<decltype(*value)>(*value);
                    }
                } else {
                    if constexpr (has_operator_deref_v<Ty>) {
                        recv->template emplace<decltype(*value)>(*value);
                    } else if constexpr (has_operator_deref_v<type_traits::cv_modify::add_const_t<Ty>>) {
                        recv->template emplace<decltype(*value)>(*value);
                    }
                }
                break;
            }
            case operation::access_element: {
                bool has_value{false};
                using namespace type_traits;
                using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
                using const_as = type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;
                auto *res =
                    static_cast<std::tuple<bool /* is_const */, any * /* value */, any * /* recv */, const any * /* index */> *>(data);
                bool is_const = std::get<0>(*res);
                rainy_let value = std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                auto &key = std::get<3>(*res);
                if constexpr (type_properties::is_sequential_container_v<remove_cvref_t> &&
                              extras::meta_method::has_operator_index_v<remove_ref_t>) {
                    using elem_t = decltype(utility::declval<Ty>()[0]);
                    std::size_t index{};
                    if (key->template is<std::size_t>()) {
                        index = key->template as<std::size_t>();
                    } else if (key->template is_convertible<std::size_t>()) {
                        index = key->template convert<std::size_t>();
                    }
                    if (is_const || value->type().is_const()) {
                        if constexpr (extras::meta_method::has_operator_index_v<const_as>) {
                            const auto &extract = (*std::get<1>(*res)).template as<const_as>();
                            utility::construct_in_place(recv,
                                                        utility::forward<access_elements_construct_type<elem_t>>(extract[index]));
                        }
                    } else {
                        auto &extract = (*std::get<1>(*res)).template as<Ty>();
                        utility::construct_in_place(recv, utility::forward<elem_t>(extract[index]));
                    }
                    has_value = recv.has_value();
                } else if constexpr (type_properties::is_map_like_v<remove_cvref_t>) {
                    using key_type = typename remove_cvref_t::key_type;
                    using elem_t = decltype(utility::declval<remove_cvref_t>()[utility::declval<key_type>()]);
                    key_type key_val{};
                    if (key->template is<key_type>()) {
                        key_val = key->template as<key_type>();
                    } else if (key->template is_convertible<key_type>()) {
                        key_val = key->template convert<key_type>();
                    }
                    if (is_const || value->type().is_const()) {
                        if constexpr (extras::meta_method::has_operator_index_for_key_v<const_as>) {
                            const auto &extract = (*std::get<1>(*res)).template as<const_as>();
                            utility::construct_in_place(
                                recv, utility::forward<access_elements_construct_type<elem_t>>(extract[utility::move(key_val)]));
                        }
                    } else {
                        auto &extract = (*std::get<1>(*res)).template as<Ty>();
                        constexpr bool type_is_const =
                            type_traits::type_properties::is_const_v<type_traits::reference_modify::remove_reference_t<Ty>>;
                        if constexpr (type_is_const) {
                            constexpr bool has_operator_in_const =
                                extras::meta_method::has_operator_index_for_key_v<type_traits::cv_modify::add_const_t<remove_cvref_t>>;
                            if constexpr (has_operator_in_const) {
                                utility::construct_in_place(
                                    recv, utility::forward<access_elements_construct_type<elem_t>>(extract[utility::move(key_val)]));
                            }
                        } else {
                            utility::construct_in_place(recv, utility::forward<elem_t>(extract[utility::move(key_val)]));
                        }
                    }
                    has_value = recv.has_value();
                } else if constexpr (is_index_tuple_v<Ty>) {
                    std::size_t index{0};
                    if (key->template is<std::size_t>()) {
                        index = key->template as<std::size_t>();
                    } else if (key->template is_convertible<std::size_t>()) {
                        index = key->template convert<std::size_t>();
                    }
                    constexpr auto find_fn = [](bool is_const_, const std::size_t index_, auto &&recv_var, auto &&extract) {
                        // NOLINT BEGIN
                        std::apply(
                            [&](auto &&...elems) { // NOLINT
                                std::size_t i{0};
                                (void) ((i++ == index_
                                             ? (is_const_
                                                    ? recv_var.template emplace<access_elements_construct_type<decltype(elems)>>(elems)
                                                    : recv_var.template emplace<decltype(elems)>(elems),
                                                true)
                                             : false) ||
                                        ...);
                            },
                            extract);
                        // NOLINT END
                    };
                    if (is_const || value->type().is_const()) {
                        find_fn(true, index, recv, value->template as<const_as>());
                    } else {
                        find_fn(false, index, recv, value->template as<Ty>());
                    }
                }
                return has_value;
            }
            case operation::container_begin: {
                const bool value = get_executer_impl_get_begin<remove_cvref_t, any>(data);
                return value;
            }
            case operation::container_end: {
                const bool value = get_executer_impl_get_end<remove_cvref_t, any>(data);
                return value;
            }
            case operation::assign: {
                using remove_ref_t = remove_reference_t<Ty>;
                constexpr bool can_assign = type_traits::extras::meta_method::has_operator_assign_v<Ty> &&
                                            !type_traits::type_properties::is_const_v<remove_ref_t>;
                if constexpr (can_assign) {
                    using tuple_t = std::tuple<typename any::reference *, any>;
                    auto *res = static_cast<tuple_t *>(data);
                    auto *left = std::get<0>(*res);
                    auto &right = std::get<1>(*res);
                    auto &left_operand = *static_cast<cv_modify::remove_cvref_t<Ty> *>(const_cast<void *>(left->target_as_void_ptr()));
                    if (right.template is<Ty>() || right.template is<remove_cvref_t>()) { // NOLINT
                        left_operand = right.template as<Ty>();
                    } else if (right.template is_convertible<remove_cvref_t>()) {
                        left_operand = right.template convert<remove_cvref_t>();
                    }
                    return true;
                }
                break;
            }
            case operation::get_reference: {
                using add_const = cv_modify::add_const_t<
                    conditional_t<is_reference_v<Ty>,
                                  conditional_t<is_rvalue_reference_v<Ty>, add_rvalue_reference_t<remove_cvref_t>,
                                                add_lvalue_reference_t<remove_cvref_t>>,
                                  remove_cvref_t>>;
                using tuple_t = std::tuple<bool, any *, typename any::reference *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *value = std::get<1>(*res);
                auto *reference = std::get<2>(*res);
                if (is_const) {
                    utility::construct_at(reference, value->template as<add_const>());
                } else {
                    utility::construct_at(reference, value->template as<Ty>());
                }
                return true;
            }
            case operation::get_lvalue_reference: {
                using add_const = reference_modify::add_lvalue_reference_t<cv_modify::add_const_t<Ty>>;
                using tuple_t = std::tuple<bool, any *, typename any::reference *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *value = std::get<1>(*res);
                auto *reference = std::get<2>(*res);
                if (is_const) {
                    utility::construct_at(reference, value->template as<add_const>());
                } else {
                    utility::construct_at(reference, value->template as<reference_modify::add_lvalue_reference_t<Ty>>());
                }
                return true;
            }
            case operation::get_rvalue_reference: {
                using add_const = reference_modify::add_rvalue_reference_t<cv_modify::add_const_t<Ty>>;
                using tuple_t = std::tuple<bool, any *, typename any::reference *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *value = std::get<1>(*res);
                auto *reference = std::get<2>(*res);
                if (is_const) {
                    utility::construct_at(reference, utility::move(value->template as<add_const>()));
                } else {
                    utility::construct_at(reference,
                                          utility::move(value->template as<reference_modify::add_rvalue_reference_t<Ty>>()));
                }
                return true;
            }
            case operation::construct_from: {
                using add_const = cv_modify::add_const_t<
                    conditional_t<is_reference_v<Ty>,
                                  conditional_t<is_rvalue_reference_v<Ty>, add_rvalue_reference_t<remove_cvref_t>,
                                                add_lvalue_reference_t<remove_cvref_t>>,
                                  remove_cvref_t>>;
                using tuple_t = std::tuple<bool, typename any::reference *, any *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *left = std::get<1>(*res);
                auto *value = std::get<2>(*res);
                if (is_const) {
                    value->template emplace<remove_cvref_t>(left->template as<add_const>());
                } else {
                    if (left->type().is_rvalue_reference()) {
                        value->template emplace<remove_cvref_t>(utility::move(left->template as<add_rvalue_reference_t<Ty>>()));
                    } else {
                        value->template emplace<remove_cvref_t>(left->template as<Ty>());
                    }
                }
                return value->has_value();
            }
            case operation::swap_value: {
                if constexpr (type_traits::type_properties::is_swappable_v<Ty>) {
                    using tuple_t = std::tuple<any *, any *>;
                    auto *res = static_cast<tuple_t *>(data);
                    auto *left = std::get<0>(*res);
                    if (auto *value = std::get<1>(*res); left->type().is_compatible(value->type())) {
                        using std::swap;
                        swap(left->template as<Ty>(), value->template as<Ty>());
                        return true;
                    }
                    return false;
                }
                return false;
            }
            case operation::query_inner_declaertion_type: {
                using the_type = type_traits::cv_modify::remove_cvref_t<Ty>;
                using tuple_t = std::tuple<any_inner_declaertion, foundation::ctti::typeinfo *>;
                auto *res = static_cast<tuple_t *>(data);
                any_inner_declaertion query = std::get<0>(*res);
                foundation::ctti::typeinfo &type = *std::get<1>(*res);
                switch (query) {
                    case any_inner_declaertion::key_type: {
                        if constexpr (type_traits::extras::meta_types::has_key_type_v<the_type>) {
                            type = rainy_typeid(typename the_type::key_type);
                        }
                        break;
                    }
                    case any_inner_declaertion::value_type: {
                        if constexpr (type_traits::extras::meta_types::has_value_type_v<the_type>) {
                            type = rainy_typeid(typename the_type::value_type);
                        }
                        break;
                    }
                    default:
                        break;
                }
                return false;
            }
            case operation::container_size: {
                using tuple_t = std::tuple<any *, std::size_t *>;
                using const_as = cv_modify::add_const_t<remove_cvref_t>;
                auto *res = static_cast<tuple_t *>(data);
                auto *left = std::get<0>(*res);
                std::size_t &new_size = *std::get<1>(*res);
                if constexpr (type_traits::extras::meta_method::has_size_v<const_as>) {
                    new_size = utility::size(left->template as<Ty>());
                    return true;
                }
                break;
            }
            case operation::container_resize: {
                using tuple_t = std::tuple<any *, std::size_t>;
                auto *res = static_cast<tuple_t *>(data);
                auto *left = std::get<0>(*res);
                std::size_t new_size = std::get<1>(*res);
                if constexpr (type_traits::extras::meta_method::has_resize_v<remove_cvref_t>) {
                    if (!left->type().is_const()) {
                        left->template as<remove_cvref_t>().resize(new_size);
                        return true;
                    }
                }
                break;
            }
            case operation::container_insert_seq_like: {
                using the_type = cv_modify::remove_cvref_t<Ty>;
                using const_iterator = typename any::const_iterator;
                using tuple_t = std::tuple<any *, const_iterator *, const_iterator *, const any *>;
                auto *res = static_cast<tuple_t *>(data);
                auto *this_pointer = std::get<0>(*res);
                const_iterator &iterator = *std::get<1>(*res);
                const_iterator &pos_iter = *std::get<2>(*res);
                const any &value = *std::get<2>(*res);
                if constexpr (type_traits::extras::meta_method::has_insert_for_iter_and_value_v<the_type>) {
                    using iterator_type = const_any_proxy_iterator<any, the_type>;
                    const auto& iter = pos_iter.template target_iterator<typename the_type::const_iterator>();
                    using value_type = typename the_type::value_type;
                    if (value.template is<value_type>()) {
                        utility::construct_in_place(
                            iterator, std::in_place_type<iterator_type>,
                            this_pointer->template as<the_type>().insert(iter, value.template as<value_type>()));
                    } else if (value.template is_convertible<value_type>()) {
                        utility::construct_in_place(
                            iterator, std::in_place_type<iterator_type>,
                            this_pointer->template as<the_type>().insert(iter, value.template convert<value_type>()));
                    }
                }
                break;
            }
            case operation::container_insert_map_like: {
                using the_type = cv_modify::remove_cvref_t<Ty>;
                using const_iterator = typename any::const_iterator;
                using ret_pair_t = pair<const_iterator, bool>;
                using tuple_t = std::tuple<any *, ret_pair_t *, const any *, const any *>;
                auto *res = static_cast<tuple_t *>(data);
                auto *this_pointer = std::get<0>(*res);
                ret_pair_t *pair = std::get<1>(*res);
                const any *key = std::get<2>(*res);
                const any *value = std::get<3>(*res);
                if (value == nullptr) {
                    if constexpr (extras::meta_method::has_insert_for_key_v<the_type>) {
                        using iterator = const_any_proxy_iterator<any, the_type>;
                        using key_type = typename the_type::key_type;
                        if (key->template is<key_type>()) {
                            auto [key_iterator, has_success] = this_pointer->template as<Ty>().insert(key->template as<key_type>());
                            utility::construct_in_place(pair->first, std::in_place_type<iterator>, key_iterator);
                            pair->second = has_success;
                            return true;
                        }
                        if (key->template is_convertible<key_type>()) {
                            auto [key_iterator, has_success] =
                                this_pointer->template as<Ty>().insert(key->template convert<key_type>());
                            utility::construct_in_place(pair->first, std::in_place_type<iterator>, key_iterator);
                            pair->second = has_success;
                            return true;
                        }
                        return false;
                    }
                } else {
                    rainy_assume(key && value);
                    if constexpr (extras::meta_method::has_insert_for_key_and_value_v<the_type> &&
                                  type_properties::is_associative_container_v<remove_cvref_t>) {
                        using iterator = const_any_proxy_iterator<any, the_type>;
                        using key_type = typename the_type::key_type;
                        using value_type = typename the_type::value_type;
                        if (key->template is<key_type>() && value->template is<value_type>()) {
                            auto [key_iterator, has_success] =
                                this_pointer->template as<Ty>().insert(key->template as<key_type>(), key->template as<value_type>());
                            utility::construct_in_place(pair->first, std::in_place_type<iterator>, key_iterator);
                            pair->second = has_success;
                            return true;
                        }
                        if (key->template is_convertible<key_type>() && key->template is_convertible<value_type>()) {
                            auto [key_iterator, has_success] = this_pointer->template as<Ty>().insert(
                                key->template convert<key_type>(), key->template convert<value_type>());
                            utility::construct_in_place(pair->first, std::in_place_type<iterator>, key_iterator);
                            pair->second = has_success;
                            return true;
                        }
                        return false;
                    }
                }
                break;
            }
            default:
                break;
        }
        return false;
    }
}

namespace rainy::utility::implements {
    template <bool Const, typename Fx, std::size_t N, std::size_t... Is>
    void call_handler_with_array(Fx &&handler, const collections::array<implements::any_binding_package, N> &array,
                                 type_traits::helper::index_sequence<Is...>) {
        using namespace type_traits::other_trans;
        using fn_traits = type_traits::primary_types::function_traits<Fx>;
        using type_list = typename tuple_like_to_type_list<typename fn_traits::tuple_like_type>::type;
        utility::invoke(
            utility::forward<Fx>(handler),
            utility::forward<conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                           typename type_at<Is, type_list>::type>>(
                implements::convert_any_binding_package<
                    conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                  typename type_at<Is, type_list>::type>,
                    Is>::impl(array[Is]))...);
    }

    template <bool Const, std::size_t N, typename Tuple, std::size_t... Is>
    void fill_tuple_with_array(Tuple &tuple, const collections::array<implements::any_binding_package, N> &array,
                               type_traits::helper::index_sequence<Is...>) {
        using namespace type_traits::other_trans;
        using utility::swap;
        std::destroy_at(&tuple);
        using type_list = typename tuple_like_to_type_list<type_traits::cv_modify::remove_cvref_t<Tuple>>::type;
        utility::construct_in_place(
            tuple, utility::forward<conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                                  typename type_at<Is, type_list>::type>>(
                       implements::convert_any_binding_package<
                           conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                         typename type_at<Is, type_list>::type>,
                           Is>::impl(array[Is]))...);
    }

    template <bool Const, std::size_t N, typename Tuple, std::size_t... Is>
    void fill_structure_with_array(Tuple &so_as_tuple, const collections::array<implements::any_binding_package, N> &array,
                                   type_traits::helper::index_sequence<Is...>) {
        using namespace type_traits::other_trans;
        using utility::swap;
        Tuple tmp{so_as_tuple};
        (((*std::get<Is>(tmp)) = implements::convert_any_binding_package<
              type_traits::other_trans::conditional_t<
                  Const, type_traits::cv_modify::add_const_t<std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>>,
                  std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>>,
              Is>::impl(array[Is])),
         ...);
        swap(tmp, so_as_tuple);
    }

    template <bool Const, typename Pair, std::size_t... Is>
    void fill_pair_with_array(Pair &pair, const collections::array<implements::any_binding_package, 2> &array) {
        using implements::convert_any_binding_package;
        Pair tmp{};
        auto &[first, second] = tmp; // 从pair中解包
        using first_type = decltype(first);
        using second_type = decltype(second);
        static_assert(type_traits::type_properties::is_copy_assignable_v<first_type>,
                      "The first element of pair-like type is not assignable");
        static_assert(type_traits::type_properties::is_copy_assignable_v<second_type>,
                      "The second element of pair-like type is not assignable");
        first = convert_any_binding_package<
            type_traits::other_trans::conditional_t<Const, type_traits::cv_modify::add_const_t<first_type>, first_type>,
            0>::impl(array[0]); // NOLINT
        second = convert_any_binding_package<
            type_traits::other_trans::conditional_t<Const, type_traits::cv_modify::add_const_t<second_type>, second_type>,
            1>::impl(array[1]); // NOLINT
        std::swap(tmp, pair);
    }

    template <bool UseConst, typename Ty, typename BasicAny>
    bool destructure_impl(const BasicAny *view, const any_execution_policy *executer, Ty &&receiver) {
        using implements::any_binding_package;
        using namespace type_traits;
        using namespace type_traits::primary_types;
        static_assert(!type_traits::type_properties::is_const_v<Ty>);
        constexpr std::size_t size = implements::eval_for_destructure_pack_receiver_size<Ty>();
        static_assert(size != 0, "Cannot process a invalid receiver!");
        collections::array<any_binding_package, size> array;
        std::size_t count{};
        bool ret = executer->invoke(implements::any_operation::query_for_is_tuple_like, &count);
        if (!ret || count != size) {
            return false;
        }
        std::tuple tuple{view, UseConst, collections::views::make_array_view(array)};
        ret = executer->invoke(implements::any_operation::destructre_this_pack, &tuple);
        if (!ret) {
            return false;
        }
        if constexpr (function_traits<Ty>::valid && !is_member_object_pointer_v<Ty>) {
            call_handler_with_array<UseConst>(utility::forward<Ty>(receiver), array, type_traits::helper::make_index_sequence<size>{});
            return true;
        } else if constexpr (is_pair_v<Ty>) {
            fill_pair_with_array<UseConst>(utility::forward<Ty>(receiver), array);
            return true;
        } else if constexpr (is_tuple_v<Ty>) {
            fill_tuple_with_array<UseConst>(utility::forward<Ty>(receiver), array, type_traits::helper::make_index_sequence<size>{});
            return true;
        } else if constexpr (member_count_v<cv_modify::remove_cvref_t<Ty>> != 0) {
            auto so_as_tuple = utility::struct_bind_tuple(receiver);
            fill_structure_with_array<UseConst>(so_as_tuple, array, type_traits::helper::make_index_sequence<size>{});
            return true;
        }
        return false;
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
