#ifndef RAINY_META_REFL_IMPL_INVOKER_HPP
#define RAINY_META_REFL_IMPL_INVOKER_HPP
#include <rainy/meta/reflection/refl_impl/arguments.hpp>

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

namespace rainy::meta::reflection::internals {
    template <typename... Args>
    static const containers::array<foundation::rtti::typeinfo, sizeof...(Args)> &generate_param_lists() {
        static const containers::array<foundation::rtti::typeinfo, sizeof...(Args)> res = {
            ::rainy::foundation::rtti::typeinfo::create<Args>()...};
        return res;
    }

    template <typename Fx, typename... Args>
    struct invoker {
        using any = containers::any;
        using traits = type_traits::primary_types::function_traits<Fx>;

        static constexpr std::size_t arity = traits::arity;
        static constexpr bool valid = traits::valid;

        static_assert(valid, "Fx must be a function!");

        static constexpr std::size_t param_hash = internals::eval_hash_code<Args...>();

        invoker() = default;

        invoker(Fx fn) noexcept : fn(fn) {
        }

        template <typename... AnyArgs>
        containers::any oinvoke(void *object, AnyArgs &...args) const {
            if constexpr (arity == sizeof...(args)) { // 确保参数个数匹配
                return oinvoke_impl(object, std::make_index_sequence<arity>{}, args...);
            }
            return {};
        }

        template <typename... AnyArgs, std::size_t... I>
        containers::any oinvoke_impl(void *object, std::index_sequence<I...>, AnyArgs &...args) const {
            constexpr auto method_type = deduction_invoker_type();
            if constexpr (method_type == method_type::static_method || method_type == method_type::static_method_noexcept) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(
                        fn, (args.type() == rainy_typeid(instance)
                                 ? args.template as<instance>().template convert_to<std::tuple_element_t<I, std::tuple<Args...>>>()
                                 : args.template convert<std::tuple_element_t<I, std::tuple<Args...>>>())...);
                    return {};
                } else {
                    return utility::invoke(
                        fn, (args.type() == rainy_typeid(instance)
                                 ? args.template as<instance>().template convert_to<std::tuple_element_t<I, std::tuple<Args...>>>()
                                 : args.template convert<std::tuple_element_t<I, std::tuple<Args...>>>())...);
                }
            } else {
                using class_type = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(
                        fn, static_cast<class_type *>(object),
                        (args.type() == rainy_typeid(instance)
                             ? args.template as<instance>().template convert_to<std::tuple_element_t<I, std::tuple<Args...>>>()
                             : args.template convert<std::tuple_element_t<I, std::tuple<Args...>>>())...);
                    return {};
                } else {
                    return utility::invoke(
                        fn, static_cast<class_type *>(object),
                        (args.type() == rainy_typeid(instance)
                             ? args.template as<instance>().template convert_to<std::tuple_element_t<I, std::tuple<Args...>>>()
                             : args.template convert<std::tuple_element_t<I, std::tuple<Args...>>>())...);
                }
            }
        }

        any invoke(void *obj, arguments_visitor &params) const {
            utility::expects(params.arguments_count() == arity, "arguments count must be equal with sizeof...(Args)");
            std::size_t act_hash_code = params.hash_code();
            return invoke_impl(obj, params, act_hash_code != param_hash, type_traits::helper::make_index_sequence<arity>{});
        }

        template <std::size_t... I>
        any invoke_impl(void *obj, arguments_visitor &params, bool need_conv, type_traits::helper::index_sequence<I...>) const {
            constexpr auto method_type = deduction_invoker_type();
            static constexpr std::size_t temporary_buffer_size = sizeof...(Args) == 0 ? 1 : arity * sizeof(object_view);
            if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                if constexpr (method_type == method_type::static_method || method_type == method_type::static_method_noexcept) {
                    if (need_conv) {
                        core::byte_t temporary_buffer[temporary_buffer_size]{};
                        utility::invoke([this, obj](auto params) { utility::invoke(fn, params[I].template convert_to<Args>()...); },
                                        params.paramlist_view(temporary_buffer));
                    } else {
                        utility::apply([this, obj](auto &&...params) { utility::invoke(fn, params...); },
                                       params.is_view() ? static_cast<arguments_view<Args...> &>(params).get_args()
                                                        : static_cast<arguments<Args...> &>(params).get_args());
                    }
                } else {
                    using class_type = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;
                    if (need_conv) {
                        core::byte_t temporary_buffer[temporary_buffer_size]{};
                        utility::invoke(
                            [this, obj](auto params) {
                                utility::invoke(fn, static_cast<class_type *>(obj), params[I].template convert_to<Args>()...);
                            },
                            params.paramlist_view(temporary_buffer));
                    } else {
                        utility::apply(
                            [this, obj](auto &&...params) { utility::invoke(fn, static_cast<class_type *>(obj), params...); },
                            params.is_view() ? static_cast<arguments_view<Args...> &>(params).get_args()
                                             : static_cast<arguments<Args...> &>(params).get_args());
                    }
                }
            } else {
                if constexpr (method_type == method_type::static_method || method_type == method_type::static_method_noexcept) {
                    if (need_conv) {
                        core::byte_t temporary_buffer[temporary_buffer_size]{};
                        return std::invoke(
                            [this, obj](auto params) { return std::invoke(fn, params[I].template convert_to<Args>()...); },
                            params.paramlist_view(temporary_buffer));
                    } else {
                        return utility::apply([this, obj](auto &&...params) { return utility::invoke(fn, params...); },
                                              params.is_view() ? static_cast<arguments_view<Args...> &>(params).get_args()
                                                               : static_cast<arguments<Args...> &>(params).get_args());
                    }
                } else {
                    using class_type = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;
                    if (need_conv) {
                        core::byte_t temporary_buffer[temporary_buffer_size]{};
                        return utility::invoke(
                            [this, obj](auto params) {
                                return utility::invoke(fn, static_cast<class_type *>(obj), params[I].template convert_to<Args>()...);
                            },
                            params.paramlist_view(temporary_buffer));
                    } else {
                        return utility::apply(
                            [this, obj](auto &&...params) { return utility::invoke(fn, static_cast<class_type *>(obj), params...); },
                            params.is_view() ? static_cast<arguments_view<Args...> &>(params).get_args()
                                             : static_cast<arguments<Args...> &>(params).get_args());
                    }
                }
            }
            return {};
        }

        any invoke_variadic(void *obj, containers::array_view<any> param_lists) const {
            utility::expects(param_lists.size() == arity, "arguments count must be equal with sizeof...(Args)");
            return invoke_variadic_impl(obj, param_lists, std::make_index_sequence<arity>{});
        }

        template <std::size_t... I>
        any invoke_variadic_impl(void *obj, containers::array_view<any> param_lists, std::index_sequence<I...>) const noexcept {
            return oinvoke_impl(obj, std::index_sequence_for<Args...>{}, param_lists[I]...);
        };

        bool is_invocable(const std::vector<foundation::rtti::typeinfo> &paramtypes) const noexcept {
            if (arity != paramtypes.size()) {
                return false;
            }
            if constexpr (arity != 0) {
                constexpr containers::array<foundation::rtti::typeinfo, arity> typeinfo_ = {
                    foundation::rtti::typeinfo::create<Args>()...};
                return std::equal(paramtypes.begin(), paramtypes.end(), typeinfo_.begin(), typeinfo_.end(),
                                  [](const foundation::rtti::typeinfo &left, const foundation::rtti::typeinfo &right) {
                                      return left.is_compatible(right);
                                  });
            }
            return true;
        };

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
}

#endif