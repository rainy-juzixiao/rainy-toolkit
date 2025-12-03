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
#ifndef RAINY_FOUNDATION_TYPEINFO_HPP
#define RAINY_FOUNDATION_TYPEINFO_HPP
/**
 * @file typeinfo.hpp
 * @brief 类型信息类
 * @brief 此文件定义了一套类型信息类，用于获取类型信息。rainy-toolkit通过此文件提供的接口，用于实现元编程功能
 * @brief 另外，使用此ctti需要承担为信息操作而付出的代价，即生成的二进制文件可能会较大
 * @author rainy-juzixiao
 */
#include <rainy/core/core.hpp>
#include <unordered_map>
#include <string_view>

namespace rainy::foundation::ctti {
    class typeinfo;
}

namespace rainy::foundation::ctti {
    enum class traits {
        /* qualifiers */
        is_lref = 1 << 0,
        is_rref = 1 << 1,
        is_const = 1 << 2,
        is_volatile = 1 << 3,

        /* fundamental types */
        is_void = 1 << 4,
        is_nullptr_t = 1 << 5,
        is_integer = 1 << 6,
        is_floating_point = 1 << 7,
        is_unsigned = 1 << 8,
        is_arithmetic = 1 << 9,
        is_fundamental = 1 << 10,
        is_trivial = 1 << 11,

        /* pointer-related */
        is_pointer = 1 << 12,

        /* compound types */
        is_array = 1 << 13,
        is_class = 1 << 14,
        is_union = 1 << 15,
        is_enum = 1 << 16,
        is_compound = 1 << 17,

        /* function and member pointers */
        is_function = 1 << 18,
        is_function_pointer = 1 << 19,
        is_member_fnptr = 1 << 20,
        is_member_field_ptr = 1 << 21,

        /* class traits */
        is_template = 1 << 22,
        is_polymorphic = 1 << 23,
        is_abstract = 1 << 24,

        /* reflection */
        is_reflection_type = 1 << 25,
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(traits);
}

namespace rainy::foundation::ctti::implements {
    using type_name_prober = void;

    constexpr std::string_view type_name_prober_ = "void";

    template <typename Ty>
    constexpr rain_fn wrapped_type_name() -> std::string_view { // NOLINT
#if RAINY_USING_MSVC
        return __FUNCSIG__;
#elif RAINY_USING_CLANG || RAINY_USING_GCC
        return __PRETTY_FUNCTION__;
#else
        static_assert(false, "unsupported compiler");
#endif
    }

    constexpr rain_fn wrapped_type_name_prefix_length() -> std::size_t { // NOLINT
        return wrapped_type_name<type_name_prober>().find(type_name_prober_);
    }

    constexpr rain_fn wrapped_type_name_suffix_length() -> std::size_t { // NOLINT
        return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - type_name_prober_.length();
    }

    template <auto Variable>
    constexpr rain_fn wrapped_variable_name() -> std::string_view {
#if RAINY_USING_CLANG || RAINY_USING_GCC
        return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
        return __FUNCSIG__;
#else
        static_assert(false, "unsupported compiler");
#endif
        // 仅支持Clang、MSVC、GCC编译器。因为其它的编译器不在本库的支持范围内。考虑其它实现
    }
}

namespace rainy::foundation::ctti::implements {
    template <typename Ty>
    static constexpr rain_fn eval_traits_for_properties() noexcept -> traits {
        using namespace type_traits;
        traits traits_{0};
        if constexpr (primary_types::is_lvalue_reference_v<Ty>) {
            traits_ |= traits::is_lref;
        }
        if constexpr (primary_types::is_rvalue_reference_v<Ty>) {
            traits_ |= traits::is_rref;
        }
        if constexpr (type_properties::is_const_v<reference_modify::remove_reference_t<Ty>>) {
            traits_ |= traits::is_const;
        }
        if constexpr (type_properties::is_volatile_v<reference_modify::remove_reference_t<Ty>>) {
            traits_ |= traits::is_volatile;
        }
        return traits_;
    }

    template <typename Ty>
    static constexpr rain_fn eval_traits_for_fundamental() noexcept -> traits {
        using namespace type_traits;
        traits traits_{0};
        if constexpr (primary_types::is_void_v<Ty>) {
            traits_ |= traits::is_void;
        }
        if constexpr (primary_types::is_pointer_v<cv_modify::remove_cvref_t<Ty>> || type_relations::is_same_v<Ty, std::nullptr_t>) {
            traits_ |= traits::is_pointer;
        }
        if constexpr (primary_types::is_integral_v<Ty>) {
            traits_ |= traits::is_integer;
        }
        if constexpr (type_relations::is_same_v<Ty, std::nullptr_t>) {
            traits_ |= traits::is_nullptr_t;
        }
        if constexpr (primary_types::is_floating_point_v<Ty>) {
            traits_ |= traits::is_floating_point;
        }
        if constexpr (type_properties::is_unsigned_v<Ty>) {
            traits_ |= traits::is_unsigned;
        }
        if constexpr (composite_types::is_fundamental_v<Ty>) {
            traits_ |= traits::is_fundamental;
        }
        if constexpr (type_properties::is_trivial_v<Ty>) {
            traits_ |= traits::is_trivial;
        }
        if constexpr (composite_types::is_arithmetic_v<Ty>) {
            traits_ |= traits::is_arithmetic;
        }
        return traits_;
    }

    template <typename Ty>
    static constexpr rain_fn eval_traits_for_compound() noexcept -> traits {
        using namespace type_traits;
        traits traits_{0};
        if constexpr (primary_types::is_class_v<other_trans::decay_t<Ty>>) {
            traits_ |= traits::is_class;
        }
        if constexpr (primary_types::is_template_v<other_trans::decay_t<Ty>>) {
            traits_ |= traits::is_template;
        }
        if constexpr (primary_types::is_function_v<Ty> || primary_types::is_variadic_function_v<Ty>) {
            traits_ |= traits::is_function;
        }
        if constexpr (primary_types::function_traits<Ty>::valid) {
            traits_ |= traits::is_function_pointer;
        }
        if constexpr (composite_types::is_member_pointer_v<Ty>) {
            if constexpr (primary_types::is_member_function_pointer_v<Ty>) {
                traits_ |= traits::is_member_fnptr;
            }
            if constexpr (!primary_types::is_member_function_pointer_v<Ty>) {
                traits_ |= traits::is_member_field_ptr;
            }
        }
        if constexpr (primary_types::is_array_v<Ty>) {
            traits_ |= traits::is_array;
        }
        if constexpr (primary_types::is_enum_v<Ty>) {
            traits_ |= traits::is_enum;
        }
        if constexpr (primary_types::is_union_v<Ty>) {
            traits_ |= traits::is_union;
        }
        if constexpr (composite_types::is_compound_v<Ty>) {
            traits_ |= traits::is_compound;
        }
        if constexpr (type_properties::is_polymorphic_v<Ty>) {
            traits_ |= traits::is_polymorphic;
        }
        if constexpr (type_properties::is_abstract_v<Ty>) {
            traits_ |= traits::is_abstract;
        }
        return traits_;
    }

    template <typename Ty>
    constexpr rain_fn eval_for_typeinfo() noexcept -> traits {
        traits traits{};
        traits |= implements::eval_traits_for_properties<Ty>();
        traits |= implements::eval_traits_for_fundamental<Ty>();
        traits |= implements::eval_traits_for_compound<Ty>();
        return traits;
    }

    template <typename Ty>
    constexpr rain_fn make_type_name_array() -> auto {
        constexpr auto wrapped_name = implements::wrapped_type_name<Ty>();
        constexpr auto prefix_length = implements::wrapped_type_name_prefix_length();
        constexpr auto suffix_length = implements::wrapped_type_name_suffix_length();
        constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
        collections::array<char, type_name_length + 1> arr{};
        for (std::size_t i = 0; i < type_name_length; ++i) {
            arr[i] = wrapped_name[prefix_length + i];
        }
        arr[type_name_length] = '\0';
        return arr;
    }

    template <typename Ty>
    constexpr auto type_name_array = make_type_name_array<Ty>();

    template <typename Ty>
    constexpr rain_fn generate_type_name() -> std::string_view {
        return {type_name_array<Ty>.data(), type_name_array<Ty>.size() - 1};
    }

    RAINY_INLINE static constexpr std::size_t fnv1a_hash(annotations::lifetime::in<std::string_view> val) noexcept {
        std::size_t hash = utility::implements::fnv_offset_basis;
        for (char i: val) {
            hash ^= static_cast<std::size_t>(static_cast<unsigned char>(i));
            hash *= utility::implements::fnv_prime;
        }
        return hash;
    }
}

namespace rainy::foundation::ctti::implements {
#define raw_type_id(x) typeinfo_component::typehash<x>()

    enum class type_operation {
        remove_pointer,
        remove_const,
        remove_volatile,
        remove_const_volatile,
        remove_cvref,
        remove_reference,
        decay_type
    };

    template <typename MainTypeInfo> // MainTypeInfo将被作为一个依赖倒置的参数使用，以确保上层API的工作正常。
    struct typeinfo_component {
        using is_comptaible_fnptr_t = bool (*)(const typeinfo_component &);
        using type_modifer = const typeinfo_component *(*) (type_operation);

        constexpr typeinfo_component() = default;

        template <typename TypeList>
        struct template_argument_generater {};

        template <typename... Types>
        struct template_argument_generater<type_traits::other_trans::type_list<Types...>> {
            static inline constexpr collections::array<MainTypeInfo, sizeof...(Types)> value = {
                MainTypeInfo::template create<Types>()...};
        };

        template <typename Ty>
        static rain_fn constexpr make() -> typeinfo_component {
            typeinfo_component raw;
            constexpr std::string_view name = generate_type_name<Ty>();
            constexpr std::size_t eval_hash_code = fnv1a_hash(name);
            constexpr traits traits = implements::eval_for_typeinfo<Ty>();
            raw.name = name;
            raw.hash_code = eval_hash_code;
            raw.type_traits = traits;
            raw.is_comptaible = &is_compatible_impl<Ty>;
            raw.modfier = &type_modfier_impl<Ty>;
            if constexpr (!type_traits::type_relations::is_void_v<Ty> && type_traits::type_properties::is_complete_v<Ty>) {
                raw.size_of_the_type = sizeof(Ty);
                raw.align_of_the_type = alignof(Ty);
            }
            if constexpr (type_traits::primary_types::template_traits<type_traits::cv_modify::remove_cvref_t<Ty>>::value) {
                raw.template_arguemnts = template_argument_generater<
                    typename type_traits::primary_types::template_traits<type_traits::cv_modify::remove_cvref_t<Ty>>::types>::value;
            }
            return raw;
        }

        template <typename Ty>
        static rain_fn constexpr typehash() -> std::size_t {
            constexpr std::string_view name = generate_type_name<Ty>();
            constexpr std::size_t eval_hash_code = fnv1a_hash(name);
            return eval_hash_code;
        }

        template <typename Type>
        static constexpr rain_fn is_compatible_impl(annotations::lifetime::in<typeinfo_component> type) -> bool;

        template <typename Ty>
        static constexpr rain_fn type_modfier_impl(annotations::lifetime::in<type_operation> op) -> const typeinfo_component<MainTypeInfo> *;

        static constexpr rain_fn empty_is_compatible(annotations::lifetime::in<typeinfo_component>)->bool;
        static constexpr rain_fn empty_type_modfier(annotations::lifetime::in<typeinfo_component>)->bool;

        std::string_view name{};
        std::size_t hash_code{};
        traits type_traits{};
        is_comptaible_fnptr_t is_comptaible{};
        type_modifer modfier{};
        collections::views::array_view<MainTypeInfo> template_arguemnts{};
        std::size_t size_of_the_type{0};
        std::size_t align_of_the_type{0};
    };

    template <typename MainTypeInfo>
    constexpr typeinfo_component<MainTypeInfo> empty_component;

    template <typename MainTypeInfo, typename Ty>
    static constexpr typeinfo_component<MainTypeInfo> typeinfo = typeinfo_component<MainTypeInfo>::template make<Ty>();

    template <typename MainTypeInfo>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::empty_is_compatible(annotations::lifetime::in<typeinfo_component>) -> bool {
        return false;
    }

    template <typename MainTypeInfo>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::empty_type_modfier(annotations::lifetime::in<typeinfo_component>) -> bool {
        return false;
    }

    template <typename MainTypeInfo>
        template <typename Ty>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::type_modfier_impl(annotations::lifetime::in<type_operation> op)
        -> const typeinfo_component<MainTypeInfo> * {
        using namespace type_traits;
        constexpr bool is_reference_ptr =
            type_traits::composite_types::is_reference_v<Ty> &&
            type_traits::primary_types::is_pointer_v<type_traits::reference_modify::remove_reference_t<Ty>>;
        if constexpr (!type_traits::type_relations::is_void_v<Ty>) {
            switch (op) {
                case type_operation::remove_pointer: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::reference_modify::remove_reference_t<Ty>;
                        using pointer_type = type_traits::pointer_modify::remove_pointer_t<referred_ptr>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_lvalue_reference_t<pointer_type>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_rvalue_reference_t<pointer_type>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo,type_traits::pointer_modify::remove_pointer_t<Ty>>;
                    }
                }
                case type_operation::remove_const: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::reference_modify::remove_reference_t<Ty>;
                        using pointer_type = type_traits::pointer_modify::remove_pointer_t<referred_ptr>;
                        using non_const_pointer = type_traits::cv_modify::remove_const_t<pointer_type>;
                        using non_const_ptr = type_traits::pointer_modify::add_pointer_t<non_const_pointer>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_lvalue_reference_t<non_const_ptr>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_rvalue_reference_t<non_const_ptr>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::cv_modify::remove_const_t<Ty>>;
                    }
                }
                case type_operation::remove_volatile: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::reference_modify::remove_reference_t<Ty>;
                        using pointer_type = type_traits::pointer_modify::remove_pointer_t<referred_ptr>;
                        using non_volatile_pointer = type_traits::cv_modify::remove_volatile_t<pointer_type>;
                        using non_volatile_ptr = type_traits::pointer_modify::add_pointer_t<non_volatile_pointer>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo,type_traits::reference_modify::add_lvalue_reference_t<non_volatile_ptr>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_rvalue_reference_t<non_volatile_ptr>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::cv_modify::remove_volatile_t<Ty>>;
                    }
                }
                case type_operation::remove_const_volatile: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::reference_modify::remove_reference_t<Ty>;
                        using pointer_type = type_traits::pointer_modify::remove_pointer_t<referred_ptr>;
                        using non_cv_pointer = type_traits::cv_modify::remove_cv_t<pointer_type>;
                        using non_cv_ptr = type_traits::pointer_modify::add_pointer_t<non_cv_pointer>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_lvalue_reference_t<non_cv_ptr>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::reference_modify::add_rvalue_reference_t<non_cv_ptr>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::cv_modify::remove_cv_t<Ty>>;
                    }
                }
                case type_operation::remove_cvref: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::reference_modify::remove_reference_t<Ty>;
                        using pointer_type = type_traits::pointer_modify::remove_pointer_t<referred_ptr>;
                        using non_cv_pointer = type_traits::cv_modify::remove_cv_t<pointer_type>;
                        using non_cv_ptr = type_traits::pointer_modify::add_pointer_t<non_cv_pointer>;
                        return &typeinfo<MainTypeInfo, non_cv_ptr>;
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::cv_modify::remove_cvref_t<Ty>>;
                    }
                }
                case type_operation::remove_reference: {
                    return &typeinfo<MainTypeInfo, type_traits::reference_modify::remove_reference_t<Ty>>;
                }
                case type_operation::decay_type: {
                    return &typeinfo<MainTypeInfo, type_traits::other_trans::decay_t<Ty>>;
                }
            }
        }
        return &empty_component<MainTypeInfo>;
    }

    template <typename MainTypeInfo>
        template <typename Type>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::is_compatible_impl(annotations::lifetime::in<typeinfo_component<MainTypeInfo>> type) -> bool {
        using namespace type_traits;
        using match_t = cv_modify::remove_cvref_t<Type>;
        using real_convert_type =
            other_trans::conditional_t<composite_types::is_reference_v<Type>, Type, reference_modify::add_lvalue_reference_t<Type>>;
        if constexpr (type_traits::type_relations::is_void_v<type_traits::cv_modify::remove_cvref_t<Type>>) {
            return false;
        } else {
            using ftraits = primary_types::function_traits<match_t>;
            if constexpr (ftraits::valid) {
                return raw_type_id(match_t) == type.hash_code; // 修正对类型信息检测的行为
            } else {
                switch (type.hash_code) {
                    case raw_type_id(match_t):
                        return true;
                    case raw_type_id(match_t &):
                        return type_relations::is_convertible_v<match_t &, real_convert_type>;
                    case raw_type_id(match_t &&):
                        return type_relations::is_convertible_v<match_t &&, real_convert_type>;
                    case raw_type_id(const match_t):
                        return true;
                    case raw_type_id(const match_t &):
                        return type_relations::is_convertible_v<const match_t &, real_convert_type>;
                    case raw_type_id(const match_t &&):
                        return type_relations::is_convertible_v<const match_t &&, real_convert_type>;
                    case raw_type_id(const volatile match_t):
                        return true;
                    case raw_type_id(const volatile match_t &):
                        return type_relations::is_convertible_v<const volatile match_t &, real_convert_type>;
                    case raw_type_id(const volatile match_t &&):
                        return type_relations::is_convertible_v<const volatile match_t &&, real_convert_type>;
                    default:
                        return false;
                }
            }
        }
    }
#undef raw_type_id
}

namespace rainy::foundation::ctti {
    /**
     * @brief 通过类型获取一个名称
     * @tparam Ty 要获取的类型
     * @returns 一个字符串视图，表示对编译时变量的名称
     */
    template <typename Ty>
    constexpr std::string_view type_name() {
        return implements::generate_type_name<Ty>();
    }

    /**
     * @brief 获取一个可在编译时确定的变量名
     * @tparam Variable 一个常量变量值
     * @returns 一个字符串视图，表示对编译时变量的名称
     */
    template <auto Variable>
    inline constexpr std::string_view variable_name() noexcept {
#if RAINY_USING_MSVC
        constexpr std::string_view func_name = __FUNCSIG__;
#else
        constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif
#if RAINY_USING_CLANG
        auto split = func_name.substr(0, func_name.rfind("]"));
        auto start = split.find("Variable = ") + 11;
        auto end = split.find(";", start);
        return split.substr(start, end - start);
#elif RAINY_USING_GCC
        auto split = func_name.substr(0, func_name.rfind("]") - 1);
        auto start = split.find("with auto Variable = ") + 20;
        auto end = split.find(";", start);
        return split.substr(start, end - start);
#elif RAINY_USING_MSVC
        auto split = func_name.substr(func_name.rfind("variable_name<") + 13);
        auto split_again = split.substr(split.rfind("->") + 2);
        return split_again.substr(0, split_again.rfind(">(void"));
#else
        static_assert(false, "You are using an unsupported compiler. Please use GCC, Clang or MSVC");
#endif
    }
}

#define RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(function_name, operation)                                                        \
    RAINY_NODISCARD constexpr rain_fn function_name() const noexcept -> typeinfo {                                                    \
        typeinfo ret;                                                                                                                 \
        ret.internal_type = internal_type->modfier(operation);                                                                        \
        ret.cache_name = ret.internal_type->name;                                                                                     \
        ret.cache_hash_code = ret.internal_type->hash_code;                                                                           \
        return ret;                                                                                                                   \
    }

#define RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(function_name, traits)                                                      \
    RAINY_NODISCARD constexpr rain_fn function_name() const noexcept -> bool {                                                        \
        return has_traits(traits);                                                                                                    \
    }

namespace rainy::foundation::ctti {
    class typeinfo {
    public:
        constexpr typeinfo() noexcept = default;
        constexpr typeinfo(annotations::lifetime::in<typeinfo>) noexcept = default;
        constexpr typeinfo(annotations::lifetime::move_from<typeinfo>) noexcept = default;

        constexpr typeinfo &operator=(annotations::lifetime::in<typeinfo>) noexcept = default;
        constexpr typeinfo &operator=(annotations::lifetime::move_from<typeinfo>) noexcept = default;

        /**
         * @brief 从Ty类型信息中构造typeinfo对象
         * @tparam Ty 要获取类型信息的类型
         * @return 返回Ty对应的类型信息对象
         */
        template <typename Ty>
        static constexpr rain_fn create() noexcept -> typeinfo {
            typeinfo type;
            // 我们在此处，缓存一些本地数据，以加速性能
            type.internal_type = &implements::typeinfo<foundation::ctti::typeinfo, Ty>;
            type.cache_name = type.internal_type->name;
            type.cache_hash_code = type.internal_type->hash_code;
            return type;
        }

        template <typename Ty>
        static constexpr rain_fn get_type_hash() noexcept -> std::size_t {
            return implements::fnv1a_hash(type_name<Ty>());
        }

        /**
         * @brief 获取Ty类型信息的静态实例的常量引用，通常由rainy_typeid宏使用（推荐）
         * @tparam Ty 要获取类型信息的类型
         * @return 一个指向对应类型信息的static只读生命周期对象的引用
         */
        template <typename Ty>
        static rain_fn of() noexcept -> annotations::lifetime::static_read_only<typeinfo> {
            static typeinfo instance = create<Ty>();
            return instance;
        }

        /**
         * @brief 获取类型信息中对应的名称
         */
        RAINY_NODISCARD constexpr rain_fn name() const noexcept -> std::string_view {
            return cache_name;
        }

        /**
         * @brief 获取类型信息中对应的哈希值
         */
        RAINY_NODISCARD constexpr rain_fn hash_code() const noexcept -> std::size_t {
            return cache_hash_code;
        }

        /**
         * @brief 检查两个类型信息是否相同
         * @param right 另一个类型信息对象
         * @return 如果类型信息相同，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn is_same(annotations::lifetime::in<typeinfo> right) const noexcept -> bool {
            return hash_code() == right.hash_code();
        }

        /**
         * @brief 检查两个类型信息是否为void
         * @param right 另一个类型信息对象
         * @return 如果类型信息是void，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn is_void() const noexcept -> bool {
            return hash_code() == get_type_hash<void>();
        }

        /**
         * @brief 运算符重载，检查两个类型信息是否相同。等效于is_same()
         * @param left 要比较的左边的类型信息对象
         * @param right 要比较的右边的类型信息对象
         * @return 如果类型信息相同，返回true，否则返回false
         */
        constexpr friend rain_fn operator==(annotations::lifetime::in<typeinfo> left,
                                            annotations::lifetime::in<typeinfo> right) noexcept -> bool {
            return left.is_same(right);
        }

        /**
         * @brief 运算符重载，检查两个类型信息是否不同。等效于!is_same()
         * @param left 要比较的左边的类型信息对象
         * @param right 要比较的右边的类型信息对象
         * @return 如果类型信息不同，返回true，否则返回false
         */
        constexpr friend rain_fn operator!=(annotations::lifetime::in<typeinfo> left,
                                            annotations::lifetime::in<typeinfo> right) noexcept -> bool {
            return !left.is_same(right);
        }

        /**
         * @brief 对当前类型执行移除const属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_const, implements::type_operation::remove_const);

        /**
         * @brief 对当前类型执行移除volatile属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_volatile, implements::type_operation::remove_volatile);

        /**
         * @brief 对当前类型执行移除const和volatile属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_cv, implements::type_operation::remove_const_volatile);

        /**
         * @brief 对当前类型执行移除reference属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_reference, implements::type_operation::remove_reference);

        /**
         * @brief 对当前类型执行移除const、volatile以及reference属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_cvref, implements::type_operation::remove_cvref);

        /**
         * @brief 对当前类型执行移除指针属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_pointer, implements::type_operation::remove_pointer);

        /**
         * @brief 对当前类型执行decay操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(decay, implements::type_operation::decay_type);

        RAINY_NODISCARD constexpr rain_fn is_compatible(annotations::lifetime::in<typeinfo> right) const noexcept -> bool {
            return internal_type->is_comptaible(*right.internal_type);
        }

        /**
         * @brief 检查此类型信息是否具有特定的traits
         * @param traits 要检查的traits，从traits枚举中获取
         * @return 如果类型信息具有traits，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn has_traits(annotations::lifetime::in<traits> traits) const noexcept -> bool {
            return static_cast<bool>(internal_type->type_traits & traits);
        }

        /**
         * @brief 获取当前类型的sizeof大小
         * @return 返回当前类型的sizeof大小
         */
        RAINY_NODISCARD constexpr rain_fn sizeof_the_type() const noexcept -> std::size_t {
            return internal_type->size_of_the_type;
        }

        /**
         * @brief 尝试获取当前类型的模板实例化参数
         * @attention 对于 template <std::size_t> 这类带有NTTP参数的模板，则无法获取，仅支持纯类型的模板
         * @return 返回模板实例化参数类型的列表视图
         */
        RAINY_NODISCARD constexpr rain_fn template_arguments()const noexcept->collections::views::array_view<foundation::ctti::typeinfo> {
            return internal_type->template_arguemnts;
        }

        operator std::size_t() const noexcept {
            return cache_hash_code;
        }

        /**
         * @brief 检查当前是否为算数类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_arithmetic, traits::is_arithmetic);
        
        /**
         * @brief 检查当前是否为浮点类型.
         * @return 如果是返回true，否则false
         */        
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_floating_point, traits::is_floating_point);
        
        /**
         * @brief 检查当前是否为整型类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_integer, traits::is_integer);
        
        /**
         * @brief 检查当前是否为空指针类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_nullptr, traits::is_nullptr_t);
        
        /**
         * @brief 检查当前是否为引用类型（包括左或右）.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_reference, traits::is_lref | traits::is_rref);
        
        /**
         * @brief 检查当前是否为指针类型.
         * @return 如果是返回true，否则false
         */        
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_pointer, traits::is_pointer);
        
        /**
         * @brief 检查当前是否为左引用类型.
         * @return 如果是返回true，否则false
         */        
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_lvalue_reference, traits::is_lref);
        
        /**
         * @brief 检查当前是否为右引用类型.
         * @return 如果是返回true，否则false
         */      
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_rvalue_reference, traits::is_rref);
        
        /**
         * @brief 检查当前是否为const类型.
         * @return 如果是返回true，否则false
         */        
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_const, traits::is_const);
        
        /**
         * @brief 检查当前是否为volatile类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_volatile, traits::is_volatile);
        
        /**
         * @brief 检查当前是否为const volatile混合的类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_const_volatile, traits::is_const & traits::is_volatile);

    private:
        std::string_view cache_name{};
        std::size_t cache_hash_code{};
        const implements::typeinfo_component<foundation::ctti::typeinfo> *internal_type{&implements::empty_component<foundation::ctti::typeinfo>};
    };
}

#undef RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER
#undef RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER

template <>
struct rainy::utility::hash<rainy::foundation::ctti::typeinfo> {
    using argument_type = foundation::ctti::typeinfo;
    using result_type = std::size_t;

    RAINY_NODISCARD std::size_t operator()(const argument_type &right) const {
        return right.hash_code();
    }

    static std::size_t hash_this_val(const argument_type &val) noexcept {
        return val.hash_code();
    }
};

// 用于获取类型信息的宏，考虑到使用传统rtti的使用习惯
#define rainy_typeid(x) ::rainy::foundation::ctti::typeinfo::of<x>()
#define rainy_typehash(x) ::rainy::foundation::ctti::typeinfo::get_type_hash<x>()

template <>
struct std::hash<rainy::foundation::ctti::typeinfo> {
    RAINY_NODISCARD std::size_t operator()(const rainy::foundation::ctti::typeinfo &val) const noexcept {
        return val.hash_code();
    }
};

namespace rainy::foundation::ctti::implements {
    using converter_func = void *(*) (void *);

    struct conversion_key {
        std::size_t source_hash;
        std::size_t target_hash;

        bool operator==(const conversion_key &other) const {
            return source_hash == other.source_hash && target_hash == other.target_hash;
        }
    };

    struct conversion_key_hash {
        std::size_t operator()(const conversion_key &k) const {
            return k.source_hash ^ (k.target_hash + 0x9e3779b9 + (k.source_hash << 6) + (k.source_hash >> 2));
        }
    };

    using conversion_map_t = std::unordered_map<conversion_key, converter_func, conversion_key_hash>;

    RAINY_INLINE conversion_map_t &get_conversion_map() {
        static conversion_map_t instance;
        return instance;
    }

    RAINY_INLINE void register_conversion(std::size_t src, std::size_t tgt, converter_func fn) {
        get_conversion_map().try_emplace(conversion_key{src, tgt}, fn);
    }

    RAINY_INLINE converter_func find_converter(std::size_t src, std::size_t tgt) {
        auto &map = get_conversion_map();
        auto it = map.find(conversion_key{src, tgt});
        if (it != map.end()) {
            return it->second;
        }
        return nullptr;
    }
}

namespace rainy::foundation::ctti {
    template <typename Derived, typename Base>
    void register_base() {
        static_assert(type_traits::type_relations::is_base_of_v<Base, Derived>, "Base must be base of Derived");
        // 派生类 -> 基类
        implements::register_conversion(typeinfo::get_type_hash<Derived>(), typeinfo::get_type_hash<Base>(),
                                        [](void *ptr) -> void * { return static_cast<Base *>(static_cast<Derived *>(ptr)); });
        // 基类 -> 派生类
        if constexpr (type_traits::type_properties::is_polymorphic_v<Base>) {
            implements::register_conversion(typeinfo::get_type_hash<Base>(), typeinfo::get_type_hash<Derived>(),
                                            [](void *ptr) -> void * { return dynamic_cast<Derived *>(static_cast<Base *>(ptr)); });
        }
    }

    RAINY_INLINE void *apply_offset(void *ptr, const typeinfo &source, const typeinfo &target) {
        if (!ptr) {
            return nullptr;
        }
        if (source.hash_code() == target.hash_code()) {
            return ptr;
        }
        auto converter = implements::find_converter(source.hash_code(), target.hash_code());
        if (converter) {
            return converter(ptr);
        }
        return nullptr;
    }
}

#define RAINY_REGISTER_BASE(Derived, Base) ::rainy::foundation::ctti::register_base<Derived, Base>()

#endif
