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
    template <typename... Args>
    static const collections::array<foundation::rtti::typeinfo, sizeof...(Args)> &generate_param_lists() {
        static const collections::array<foundation::rtti::typeinfo, sizeof...(Args)> res = {
            ::rainy::foundation::rtti::typeinfo::create<Args>()...};
        return res;
    }

    template <typename... Args>
    static constexpr std::size_t eval_hash_code() noexcept {
        return (0 + ... + foundation::rtti::typeinfo::get_type_hash<Args>());
    }

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
        template <typename... AnyArgs>
        utility::any invoke(void *object, AnyArgs &...args) const {
            if constexpr (arity != sizeof...(args)) {
                return {};
            } else {
                if constexpr (arity == 0) {
                    return invoke_impl(object, type_traits::helper::make_index_sequence<0>{});
                } else {
                    std::size_t args_hash = 0 + (args.type().hash_code() + ...);
                    if (args_hash == param_hash) {
                        return invoke_impl(object, type_traits::helper::make_index_sequence<arity>{}, args...);
                    }
                    return invoke_impl_with_conv(object, type_traits::helper::make_index_sequence<arity>{}, args...);
                }
            }
        }

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

        template <typename... AnyArgs, std::size_t... I>
        utility::any invoke_impl_with_conv(void *object, type_traits::helper::index_sequence<I...>, AnyArgs &...args) const {
            constexpr auto method_type = deduction_invoker_type();
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            if constexpr (method_type == method_type::static_method || method_type == method_type::static_method_noexcept) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, args.template convert<typename type_traits::other_trans::type_at<I, tuple_t>::type>()...);
                    return {};
                } else {
                    return utility::invoke(fn,
                                           args.template convert<typename type_traits::other_trans::type_at<I, tuple_t>::type>()...);
                }
            } else {
                using class_type = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, static_cast<class_type *>(object),
                                    args.template convert<typename type_traits::other_trans::type_at<I, tuple_t>::type>()...);
                    return {};
                } else {
                    return utility::invoke(fn, static_cast<class_type *>(object),
                                           args.template convert<typename type_traits::other_trans::type_at<I, tuple_t>::type>()...);
                }
            }
        }

        template <typename... AnyArgs, std::size_t... I>
        utility::any invoke_impl(void *object, type_traits::helper::index_sequence<I...>, AnyArgs &...args) const {
            constexpr auto method_type = deduction_invoker_type();
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            if constexpr (method_type == method_type::static_method || method_type == method_type::static_method_noexcept) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, (args.template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
                    return {};
                } else {
                    return utility::invoke(fn, (args.template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
                }
            } else {
                using class_type = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, static_cast<class_type *>(object),
                                    (args.template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
                    return {};
                } else {
                    return utility::invoke(fn, static_cast<class_type *>(object),
                                           (args.template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
                }
            }
        }

        static constexpr method_type deduction_invoker_type() {
            using traits = type_traits::primary_types::function_traits<Fx>;
            if constexpr (!type_traits::primary_types::is_member_function_pointer_v<Fx>) {
                constexpr bool noexcept_invoke = noexcept(utility::invoke(utility::declval<Fx>(), utility::declval<Args>()...));
                return noexcept_invoke ? method_type::static_method_noexcept : method_type::static_method;
            } else {
                using method_traits = type_traits::primary_types::member_pointer_traits<Fx>;
                using raw_class_type = typename method_traits::class_type;
                constexpr bool noexcept_invoke = noexcept(
                    utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type *>(), utility::declval<Args>()...));
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

        Fx fn;
        static constexpr method_type type = deduction_invoker_type();
    };

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
}

#endif