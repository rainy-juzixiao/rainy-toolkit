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
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/refl_impl/object_view.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::meta::reflection {
    class function;
}

namespace rainy::meta::reflection {
    enum class method_flags : std::uint8_t {
        none = 0,
        static_qualified = 1, // static method
        noexcept_specified = 2, // noexcept
        lvalue_qualified = 4, // left qualifier (e.g. &)
        rvalue_qualified = 8, // right qualifier (e.g. &&)
        const_qualified = 16, // const
        volatile_qualified = 32, // volatile
    };

    RAINY_NODISCARD constexpr method_flags operator|(method_flags lhs, method_flags rhs) noexcept { // NOLINT
        return static_cast<method_flags>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
    }

    RAINY_NODISCARD constexpr method_flags operator&(method_flags lhs, method_flags rhs) noexcept { // NOLINT
        return static_cast<method_flags>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
    }

    RAINY_NODISCARD constexpr method_flags operator^(method_flags lhs, method_flags rhs) noexcept { // NOLINT
        return static_cast<method_flags>(static_cast<std::uint8_t>(lhs) ^ static_cast<std::uint8_t>(rhs));
    }

    constexpr method_flags &operator|=(method_flags &lhs, method_flags rhs) noexcept { // NOLINT
        lhs = lhs | rhs;
        return lhs;
    }

    constexpr method_flags &operator&=(method_flags &lhs, method_flags rhs) noexcept { // NOLINT
        lhs = lhs & rhs;
        return lhs;
    }

    constexpr method_flags &operator^=(method_flags &lhs, method_flags rhs) noexcept { // NOLINT
        lhs = lhs ^ rhs;
        return lhs;
    }

    RAINY_NODISCARD constexpr method_flags operator~(method_flags value) noexcept { // NOLINT
        return static_cast<method_flags>(~static_cast<std::uint8_t>(value));
    }
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
        arg_view(collections::array<object_view, N> &list) : args(list) { // NOLINT
        }

        iterator begin() noexcept {
            return args.begin();
        }

        iterator end() noexcept {
            return args.end();
        }

        reference operator[](const std::size_t idx) {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        const_reference &operator[](const std::size_t idx) const {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        reference at(const std::size_t idx) {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        RAINY_NODISCARD const_reference at(const std::size_t idx) const {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        RAINY_NODISCARD std::size_t size() const noexcept {
            return args.size();
        }

    private:
        view args;
    };

    template <std::size_t N>
    class arg_store {
    public:
        template <typename... Args>
        explicit arg_store([[maybe_unused]] Args &&...args_in) noexcept : args{make_object_view_helper(utility::forward<Args>(args_in))...} {
            static_assert(sizeof...(Args) == N, "Argument count mismatch with N");
        }

        arg_view to_argview() noexcept {
            return arg_view{args};
        }

        operator arg_view() && noexcept { // NOLINT
            return arg_view{args};
        }

    private:
        template <typename Ty>
        static object_view make_object_view_helper(Ty &&arg) noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)),
                                   foundation::rtti::typeinfo::of<other_trans::decay_t<Ty>>()};
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::rtti::typeinfo::of<Ty>()};
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return object_view{const_cast<void *>(static_cast<const void *>(arg)), foundation::rtti::typeinfo::of<Ty>()};
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return object_view{const_cast<void *>(arg.target_as_void_ptr()), arg.type(), true};
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, object_view>) {
                return object_view{arg};
            } else {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::rtti::typeinfo::of<Ty>()};
            }
        }

        collections::array<object_view, N> args;
    };

    template <typename... Args>
    arg_store(Args...) -> arg_store<sizeof...(Args)>;

    template <std::size_t N>
    class make_paramlist {
    public:
        template <typename... Args>
        explicit make_paramlist(Args &&...args_in) : types{make_paramlist_helper(utility::forward<Args>(args_in))...} {
        }

        RAINY_NODISCARD collections::views::array_view<foundation::rtti::typeinfo> get() const noexcept {
            return types;
        }

    private:
        template <typename Ty>
        static foundation::rtti::typeinfo make_paramlist_helper(Ty &&arg) noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return foundation::rtti::typeinfo::of<other_trans::decay_t<Ty>>();
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return foundation::rtti::typeinfo::of<Ty>();
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return foundation::rtti::typeinfo::of<Ty>();
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return arg.type();
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return arg.rtti();
            } else {
                return foundation::rtti::typeinfo::of<Ty>();
            }
        }

        collections::array<foundation::rtti::typeinfo, N> types;
    };

    template <typename... Args>
    make_paramlist(Args...) -> make_paramlist<sizeof...(Args)>;

    template <typename... Args>
    class make_nondynamic_paramlist {
    public:
        make_nondynamic_paramlist() : types{make_paramlist_helper<Args>()...} {
        }

        RAINY_NODISCARD collections::views::array_view<foundation::rtti::typeinfo> get() const noexcept {
            return types;
        }

    private:
        template <typename Ty>
        static foundation::rtti::typeinfo make_paramlist_helper() noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return foundation::rtti::typeinfo::of<other_trans::decay_t<Ty>>();
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return foundation::rtti::typeinfo::of<Ty>();
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return foundation::rtti::typeinfo::of<Ty>();
            } else {
                return foundation::rtti::typeinfo::of<Ty>();
            }
        }

        collections::array<foundation::rtti::typeinfo, sizeof...(Args)> types;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename... Args>
    static constexpr std::size_t eval_hash_code() noexcept {
        std::size_t right{1};
        return (0 + ... + (foundation::rtti::typeinfo::get_type_hash<Args>() * right++));
    }

    template <typename Fx, typename... Args>
    static constexpr method_flags deduction_invoker_type() {
        auto flag{method_flags::none};
        using traits = type_traits::primary_types::function_traits<Fx>;
        if constexpr (!type_traits::primary_types::is_member_function_pointer_v<Fx>) {
            constexpr bool noexcept_invoke = noexcept(utility::invoke(utility::declval<Fx>(), utility::declval<Args>()...));
            flag |=
                (noexcept_invoke ? method_flags::static_qualified | method_flags::noexcept_specified : method_flags::static_qualified);
        } else {
            using method_traits = type_traits::primary_types::member_pointer_traits<Fx>;
            using raw_class_type = typename method_traits::class_type;
            bool noexcept_invoke{false};
            if constexpr (traits::is_invoke_for_lvalue || traits::is_invoke_for_rvalue) {
                if constexpr (traits::is_invoke_for_lvalue) {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type &>(), utility::declval<Args>()...));
                } else {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type&&>(), utility::declval<Args>()...));
                }
            } else {
                noexcept_invoke = noexcept(
                    utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type *>(), utility::declval<Args>()...));
            }
            if constexpr (traits::is_const_member_function) {
                flag |= method_flags::const_qualified;
            }
            if constexpr (traits::is_volatile) {
                flag |= method_flags::volatile_qualified;
            }
            if constexpr (traits::is_invoke_for_lvalue) {
                flag |= method_flags::lvalue_qualified;
            } else if constexpr (traits::is_invoke_for_rvalue) {
                flag |= method_flags::rvalue_qualified;
            }
            if (noexcept_invoke) {
                flag |= method_flags::noexcept_specified;
            }
        }
        return flag;
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

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

namespace rainy::meta::reflection::implements {
    template <typename Fx, typename = void>
    struct is_fnobj : type_traits::helper::false_type {
        static RAINY_CONSTEXPR_BOOL is_lambda_without_capture = false;
    };

    template <typename Fx>
    struct is_fnobj<Fx, type_traits::other_trans::void_t<decltype(&Fx::operator())>> : type_traits::helper::true_type {
    };

    template <typename Fx, bool IsFnObj = is_fnobj<Fx>::value> // NOLINT
    struct extract_function_traits {
        static_assert(is_fnobj<Fx>::value, "Fx must be a callable object!");
        using type = type_traits::primary_types::function_traits<type_traits::cv_modify::remove_cvref_t<decltype(&Fx::operator())>>;
    };

    template <typename Fx>
    struct extract_function_traits<Fx, false> {
        using type = type_traits::primary_types::function_traits<Fx>;
    };

    template <typename Fx, typename... Args>
    struct invoker {
        using any = utility::any;
        using traits = typename extract_function_traits<Fx>::type;

        static constexpr std::size_t arity = traits::arity;
        static constexpr bool valid = traits::valid;

        static_assert(valid, "Fx must be a function!");

        static constexpr std::size_t param_hash = implements::eval_hash_code<Args...>();

        invoker() {
        }

        template <typename Functor>
        explicit invoker(Functor &&fn) noexcept : fn(utility::forward<Functor>(fn)) {
        }

        bool is_compatible(arg_view *view) const {
            return is_compatible_impl(view, type_traits::helper::make_index_sequence<arity>{});
        }

        bool is_compatible(collections::views::array_view<foundation::rtti::typeinfo> paramlist) const { // NOLINT
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

        any invoke(void *object, arg_view *view) const {
            if (view->size() != arity) {
                errno = EINVAL;
                return {};
            }
            // H(\{h_i\}) = \text{hash}\left(\sum_{i=1}^n i \times h_i \right)
            const std::size_t args_hash =
                core::accumulate(view->begin(), view->end(), std::size_t{0},
                                 [right = std::size_t{1}](const std::size_t acc, const object_view &item) mutable {
                                     return acc + static_cast<std::size_t>(item.rtti().hash_code() * right++);
                                 });
            /*
            若假设 x 的参数列表哈希如下 {1, 2 , 4 , 8};
            则 H(x) = (1 * 1) + (2 * 2) + (4 * 3) + (8 * 4) = 49
            若假设传入的参数y的哈希为 {1, 2, 8, 4}
            则 H(y) = (1 * 1) + (2 * 2) + (8 * 3) + (4 * 4) = 45
            很显然，x != y
            */
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

        template <std::size_t... I>
        any invoke_impl(void *object, arg_view &items, type_traits::helper::index_sequence<I...>) const {
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            return access_invoke(
                object,
                utility::forward<Args>(
                    items[I].template as<typename type_traits::other_trans::type_at<I, tuple_t>::type>())...);
        }

        template <std::size_t... I>
        any invoke_impl_with_conv(void *object, arg_view &items, type_traits::helper::index_sequence<I...>) const {
            using tuple_t = type_traits::other_trans::type_list<Args...>;
            return access_invoke(
                object, utility::forward<Args>(
                            utility::any_converter<typename type_traits::other_trans::type_at<I, tuple_t>::type>::basic_convert(
                                items[I].get_pointer(), items[I].rtti(), items[I].is_any()))...);
        }

        template <typename... UAx>
        any access_invoke(void *object, UAx &&...args) const {
            using namespace type_traits;
            if constexpr (constexpr method_flags flags = type; static_cast<bool>(flags & method_flags::static_qualified)) {
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, utility::forward<UAx>(args)...);
                    return {};
                } else {
                    return utility::invoke(fn, utility::forward<UAx>(args)...);
                }
            } else {
                using instance_t = typename primary_types::member_pointer_traits<Fx>::class_type;
                if constexpr (type_traits::type_relations::is_void_v<typename traits::return_type>) {
                    utility::invoke(fn, static_cast<instance_t*>(object), utility::forward<UAx>(args)...);
                    return {};
                } else {
                    if constexpr (traits::is_invoke_for_lvalue) {
                        return utility::invoke(fn, static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                               utility::forward<UAx>(args)...);
                    } else if constexpr (traits::is_invoke_for_rvalue) {
                        return utility::invoke(fn, static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                               utility::forward<UAx>(args)...);
                    } else {
                        return utility::invoke(fn, static_cast<instance_t *>(object), utility::forward<UAx>(args)...);
                    }
                }
            }
        }

        Fx fn;
        static constexpr method_flags type = deduction_invoker_type<Fx, Args...>();
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif