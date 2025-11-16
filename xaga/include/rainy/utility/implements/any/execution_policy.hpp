#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_EXECUTION_POLICY_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_EXECUTION_POLICY_HPP

// NOLINTBEGIN

#include <rainy/core/core.hpp>
#include <rainy/utility/implements/any/fwd.hpp>
#include <rainy/utility/implements/any/cast.hpp>
#include <rainy/utility/implements/any/iteator.hpp>

// NOLINTEND

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

namespace rainy::utility::implements {
    struct any_execution_policy {
        using operation = any_operation;

        using invoke_fn = bool(operation op, void *data) noexcept;

        template <typename Ty, typename BasicAnyImpl>
        static bool invoke_impl(operation op, void *data);

        invoke_fn *invoke;
    };

    template <typename Ty, typename BasicAnyImpl>
    inline const any_execution_policy any_execution_policy_object = {
        +[](const any_execution_policy::operation op, void *const data) noexcept -> bool {
            return any_execution_policy::invoke_impl<Ty, BasicAnyImpl>(op, data);
        }};

        template <typename TargetType, std::size_t Idx, bool IsReference = type_traits::composite_types::is_reference_v<TargetType>>
    struct convert_any_binding_package {
        static decltype(auto) impl(const implements::any_binding_package &pkg) {
            using namespace type_traits;
            if constexpr (type_relations::is_same_v<TargetType, utility::placeholder_t>) {
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
                std::terminate();
            }
        }
    };

    template <typename TargetType, std::size_t Idx>
    struct convert_any_binding_package<TargetType, Idx, false> {
        static auto impl(const implements::any_binding_package &pkg) {
            using namespace type_traits;
            if constexpr (type_relations::is_same_v<TargetType, utility::placeholder_t>) {
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

        any add(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_add_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::plus{});
            }
            return {};
        }

        any subtract(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::minus{});
            }
            return {};
        }

        any incr_prefix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_preinc_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                ++left_operand;
                return any{std::in_place_type<decltype(--left_operand)>, left_operand};
            }
            return {};
        }

        any decr_prefix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_predec_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                return any{std::in_place_type<decltype(--left_operand)>, --left_operand};
            }
            return {};
        }

        any incr_postfix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_postinc_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                return left_operand++;
            }
            return {};
        }

        any decr_postfix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_postdec_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                return left_operand--;
            }
            return {};
        }

        any multiply(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mul_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::multiplies{});
            }
            return {};
        }

        any divide(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_div_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right, foundation::functional::divides{});
            }
            return {};
        }

        any mod(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mod_v<Ty>) {
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

        any add(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_plus_equal_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft += myright; });
            }
            return {};
        }

        any subtract(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_equal_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft -= myright; });
            }
            return {};
        }

        any multiply(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mul_equal_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft *= myright; });
            }
            return {};
        }

        any divide(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_div_equal_v<Ty>) {
                return implements::any_magic_method_helper<any, Ty>(left, right,
                                                                    [](auto &&myleft, auto &&myright) { return myleft /= myright; });
            }
            return {};
        }

        any mod(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mod_equal_v<Ty>) {
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

namespace rainy::utility::implements {
    template <typename Ty, typename BasicAnyImpl>
    bool any_execution_policy::invoke_impl(operation op, void *const data) {
        using namespace type_traits;
        using namespace type_traits::other_trans;
        using namespace type_traits::composite_types;
        using namespace type_traits::primary_types;
        using namespace type_traits::reference_modify;
        using any = BasicAnyImpl;
        using remove_cvref_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        switch (op) {
            case operation::compare: {
                using namespace foundation::exceptions::logic;
                using pack = const std::tuple<const any *, const any *, any_compare_operation> *;
                const auto *res = static_cast<pack>(data);
                const any *left = std::get<0>(*res);
                const any *right = std::get<1>(*res);
                switch (std::get<2>(*res)) {
                    case any_compare_operation::less: {
                        if constexpr (is_any_less_compareable_v<remove_cvref_t, any>) {
                            return any_magic_method<remove_cvref_t, any>{}.compare_less(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented("Current type not support this operation: less");
                        }
                        break;
                    }
                    case any_compare_operation::less_eq: {
                        if constexpr (is_any_less_eq_compareable_v<remove_cvref_t, any>) {
                            return any_magic_method<remove_cvref_t, any>{}.compare_less_equal(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented(
                                "Current type not support this operation: less_eq");
                        }
                        break;
                    }
                    case any_compare_operation::eq: {
                        if constexpr (is_any_eq_compareable_v<remove_cvref_t, any>) {
                            return any_magic_method<remove_cvref_t, any>{}.compare_equal(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented("Current type not support this operation: eq");
                        }
                        break;
                    }
                    case any_compare_operation::greater_eq: {
                        if constexpr (is_any_greater_eq_compareable_v<remove_cvref_t, any>) {
                            return any_magic_method<remove_cvref_t, any>{}.compare_greater_equal(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented(
                                "Current type not support this operation: greater_eq");
                        }
                        break;
                    }
                    case any_compare_operation::greater: {
                        if constexpr (is_any_gt_compareable_v<remove_cvref_t, any>) {
                            return any_magic_method<remove_cvref_t, any>{}.compare_greater(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented(
                                "Current type not support this operation: greater");
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case operation::eval_hash: {
                auto *res = static_cast<std::tuple<const any *, std::size_t *> *>(data);
                const any *object = std::get<0>(*res);
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
            case operation::add: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */

                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_addable_v<remove_cvref_t, any, true>) {
                        any_magic_method<remove_cvref_t, any, true>{}.add(recv, right);
                    }
                    return true;
                }
                if constexpr (is_any_addable_v<remove_cvref_t, any, false>) {
                    recv = any_magic_method<remove_cvref_t, any, false>{}.add(left, right);
                    return true;
                }
                break;
            }
            case operation::subtract: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_subable_v<remove_cvref_t, any, true>) {
                        any_magic_method<remove_cvref_t, any, true>{}.subtract(recv, right);
                    }
                    return true;
                }
                if constexpr (is_any_subable_v<remove_cvref_t, any, false>) {
                    recv = any_magic_method<remove_cvref_t, any, false>{}.subtract(left, right);
                    return true;
                }
                return true;
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
            case operation::multiply: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_multable_v<remove_cvref_t, any, true>) {
                        any_magic_method<remove_cvref_t, any, true>{}.multiply(recv, right);
                    }
                    return true;
                }
                if constexpr (is_any_multable_v<remove_cvref_t, any, false>) {
                    recv = any_magic_method<remove_cvref_t, any, false>{}.multiply(left, right);
                    return true;
                }
                break;
            }
            case operation::divide: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_divable_v<remove_cvref_t, any, true>) {
                        any_magic_method<remove_cvref_t, any, true>{}.divide(recv, right);
                    }
                    return true;
                }
                if constexpr (is_any_divable_v<remove_cvref_t, any, false>) {
                    recv = any_magic_method<remove_cvref_t, any, false>{}.divide(left, right);
                    return true;
                }
                break;
            }
            case operation::mod: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_modable_v<remove_cvref_t, any, true>) {
                        any_magic_method<remove_cvref_t, any, true>{}.mod(recv, right);
                    }
                    return true;
                }
                if constexpr (is_any_modable_v<remove_cvref_t, any, false>) {
                    recv = any_magic_method<remove_cvref_t, any, false>{}.mod(left, right);
                    return true;
                }
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
                using const_as = type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;
                auto *res = static_cast<std::tuple<bool /* is_const */, any * /* value */, typename any::reference * /* recv */,
                                                   const any * /* index */> *>(data);
                bool is_const = std::get<0>(*res);
                any *value = std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                auto &key = std::get<3>(*res);
                if constexpr (extras::meta_method::has_operator_index_v<Ty>) {
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
                            utility::construct_in_place(recv, utility::forward<access_elements_construct_type<elem_t>>(extract[index]),
                                                        value);
                        }
                    } else {
                        auto &extract = (*std::get<1>(*res)).template as<Ty>();
                        utility::construct_in_place(recv, utility::forward<elem_t>(extract[index]), value);
                    }
                    has_value = recv.has_value();
                } else if constexpr (extras::meta_method::has_operator_index_for_key_v<remove_cvref_t>) {
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
                                recv, utility::forward<access_elements_construct_type<elem_t>>(extract[utility::move(key_val)]),
                                value);
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
                                    recv, utility::forward<access_elements_construct_type<elem_t>>(extract[utility::move(key_val)]),
                                    value);
                            }
                        } else {
                            utility::construct_in_place(recv, utility::forward<elem_t>(extract[utility::move(key_val)]), value);
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
            case operation::call_begin: {
                if constexpr (type_traits::extras::iterators::has_iterator_v<Ty>) {
                    using tuple_t = std::tuple<bool /* is const */, any * /* value */, typename any::iterator * /* recv */>;
                    using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
                    using const_as = type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;
                    using iterator = any_proxy_iterator<any, remove_cvref_t>;
                    auto *res = static_cast<tuple_t *>(data);
                    bool is_const = std::get<0>(*res);
                    any *value = std::get<1>(*res);
                    auto *recv = std::get<2>(*res);
                    if constexpr (type_traits::extras::iterators::has_const_iterator_v<Ty>) {
                        if (is_const || value->type().is_const()) {
                            using const_iterator = const_any_proxy_iterator<any, remove_cvref_t>;
                            if constexpr (type_traits::extras::meta_method::has_cbegin_v<Ty>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>,
                                                      value->template as<const_as>().cbegin(), value);
                            } else if constexpr (type_traits::extras::meta_method::has_begin_v<const_as>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().begin(),
                                                      value);
                            }
                        }
                    }
                    if constexpr (type_traits::extras::meta_method::has_begin_v<Ty>) {
                        if (!is_const && !value->type().is_const()) {
                            if constexpr (!type_traits::type_properties::is_const_v<remove_ref_t>) {
                                utility::construct_at(recv, std::in_place_type<iterator>, value->template as<Ty>().begin(), value);
                            }
                        }
                    }
                    return !recv->empty();
                }
                break;
            }
            case operation::call_end: {
                if constexpr (type_traits::extras::iterators::has_iterator_v<Ty>) {
                    using tuple_t = std::tuple<bool /* is const */, any * /* value */, typename any::iterator * /* recv */>;
                    using remove_ref_t = remove_reference_t<Ty>;
                    using const_as = cv_modify::add_const_t<remove_ref_t>;
                    using iterator = any_proxy_iterator<any, remove_cvref_t>;
                    auto *res = static_cast<tuple_t *>(data);
                    bool is_const = std::get<0>(*res);
                    any *value = std::get<1>(*res);
                    auto *recv = std::get<2>(*res);
                    if constexpr (type_traits::extras::iterators::has_const_iterator_v<Ty>) {
                        if (is_const || value->type().is_const()) {
                            using const_iterator = const_any_proxy_iterator<any, remove_cvref_t>;
                            if constexpr (type_traits::extras::meta_method::has_cbegin_v<Ty>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().cend(),
                                                      value);
                            } else if constexpr (type_traits::extras::meta_method::has_begin_v<const_as>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().end(),
                                                      value);
                            }
                        }
                    }
                    if constexpr (type_traits::extras::meta_method::has_end_v<Ty>) {
                        if (!is_const && !value->type().is_const()) {
                            if constexpr (!type_traits::type_properties::is_const_v<remove_ref_t>) {
                                utility::construct_at(recv, std::in_place_type<iterator>, value->template as<Ty>().end(), value);
                            }
                        }
                    }
                    return !recv->empty();
                }
                break;
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
                    if (right.template is<Ty>()) {
                        left_operand = right.template as<Ty>();
                    } else if (right.template is_convertible<Ty>()) {
                        left_operand = right.template convert<Ty>();
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
                    utility::construct_at(reference, value->template as<add_const>(), value);
                } else {
                    utility::construct_at(reference, value->template as<Ty>(), value);
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
                    utility::construct_at(reference, value->template as<add_const>(), value);
                } else {
                    utility::construct_at(reference, value->template as<reference_modify::add_lvalue_reference_t<Ty>>(), value);
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
                    utility::construct_at(reference, utility::move(value->template as<add_const>()), value);
                } else {
                    utility::construct_at(reference, utility::move(value->template as<reference_modify::add_rvalue_reference_t<Ty>>()),
                                          value);
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
                        if (left->this_pointer) {
                            left->this_pointer->reset();
                        }
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
        }
        return false;
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif