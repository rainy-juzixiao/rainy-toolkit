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
#ifndef RAINY_CORE_TYPE_TRAITS_HPP
#define RAINY_CORE_TYPE_TRAITS_HPP
#include <iterator>
#include <rainy/core/platform.hpp>
#include <rainy/core/implements/tuple.hpp>
#include <rainy/core/implements/basic_algorithm.hpp>
#include <rainy/core/implements/compressed_pair.hpp>
#include <rainy/core/type_traits/helper.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/type_traits/meta_types.hpp>
#include <rainy/core/type_traits/modifers.hpp>
#include <rainy/core/type_traits/ranges_traits.hpp>
#include <rainy/core/type_traits/type_list.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/core/type_traits/value_list.hpp>
#include <rainy/core/type_traits/decay.hpp>
#include <rainy/core/type_traits/primary_types.hpp>
#include <rainy/core/type_traits/logical.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <utility>
#if RAINY_USING_GCC
#include <rainy/core/gnu/typetraits.hpp>
#endif

/**
 * @def RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS
 * @brief Enables bitmask operators for enum classes.
 *        为枚举类启用位掩码操作符。
 *
 * @param EnumType The enum class type name to enable bitmask operations for
 *                 要启用位掩码操作的枚举类类型名
 *
 * @brief
 * This macro generates overloads for bitwise AND, OR, XOR, NOT, and their
 * corresponding assignment operators for the specified enum class.
 * It allows enum classes to be used as bitmasks, similar to traditional
 * C++ enum flags.
 *
 * 该宏为枚举类生成按位与、或、异或、取反以及相应的赋值操作符重载。
 * 使得枚举类可以作为位掩码使用，就像 C++ 中传统的枚举标志位一样。
 *
 * Generated operators include:
 * 生成的操作符包括：
 * - |  : Bitwise OR  / 按位或
 * - &  : Bitwise AND / 按位与
 * - ^  : Bitwise XOR / 按位异或
 * - ~  : Bitwise NOT / 按位取反
 * - |= : Bitwise OR assignment  / 按位或赋值
 * - &= : Bitwise AND assignment / 按位与赋值
 * - ^= : Bitwise XOR assignment / 按位异或赋值
 *
 * Usage example:
 * 使用示例：
 * @code
 * enum class MyFlags { A = 1 << 0, B = 1 << 1, C = 1 << 2 };
 * RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(MyFlags)
 *
 * MyFlags flags = MyFlags::A | MyFlags::B;  // Bitwise combination allowed
 *                                            // 允许按位组合
 * flags &= ~MyFlags::A;                      // Bit operations allowed
 *                                            // 允许位操作
 * @endcode
 */
#define RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(EnumType)                                                                           \
    inline constexpr EnumType operator|(EnumType left, EnumType right) {                                                              \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(left) | static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator&(EnumType left, EnumType right) {                                                              \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(left) & static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator^(EnumType left, EnumType right) {                                                              \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(static_cast<type>(left) ^ static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator~(EnumType val) {                                                                               \
        using type = ::rainy::type_traits::other_trans::underlying_type_t<EnumType>;                                                  \
        return static_cast<EnumType>(~static_cast<type>(val));                                                                        \
    }                                                                                                                                 \
    inline constexpr EnumType &operator|=(EnumType &left, EnumType right) {                                                           \
        left = left | right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline constexpr EnumType &operator&=(EnumType &left, EnumType right) {                                                           \
        left = left & right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline constexpr EnumType &operator^=(EnumType &left, EnumType right) {                                                           \
        left = left ^ right;                                                                                                          \
        return left;                                                                                                                  \
    }

namespace rainy::utility {
    /**
     * @brief Placeholder type for template metaprogramming and function argument tagging.
     *        Used to indicate a position to be filled or as a default argument.
     *
     *        用于模板元编程和函数参数标记的占位符类型。
     *        用于指示需要填充的位置或作为默认参数。
     */
    struct placeholder_t final {
        explicit placeholder_t() = default;
    };

    /**
     * @brief Global constexpr instance of placeholder_t.
     *        placeholder_t 的全局 constexpr 实例。
     */
    constexpr placeholder_t placeholder{};

    /**
     * @brief Type-parameterized placeholder template.
     *        Allows creating placeholders that carry type information.
     *
     *        类型参数化的占位符模板。
     *        允许创建携带类型信息的占位符。
     *
     * @tparam Ty Type to associate with this placeholder (defaults to void)
     *            与此占位符关联的类型（默认为 void）
     */
    template <typename = void>
    struct placeholder_type_t final {
        explicit placeholder_type_t() = default;
    };

    /**
     * @brief Global constexpr instance of placeholder_type_t<Ty>.
     *        placeholder_type_t<Ty> 的全局 constexpr 实例。
     *
     * @tparam Ty Type associated with the placeholder
     *            与占位符关联的类型
     */
    template <typename Ty>
    constexpr placeholder_type_t<Ty> placeholder_type{};

    /**
     * @brief In-place construction tag with index.
     *        Used to disambiguate constructors that take an index parameter.
     *
     *        带索引的就地构造标签。
     *        用于区分接受索引参数的构造函数。
     *
     * @tparam Idx Index value for tag specialization
     *             标签特化的索引值
     */
    template <std::size_t>
    struct in_place_index_t final {
        explicit in_place_index_t() = default;
    };

    /**
     * @brief Global constexpr instance of in_place_index_t<Idx>.
     *        in_place_index_t<Idx> 的全局 constexpr 实例。
     *
     * @tparam Idx Index value
     *             索引值
     */
    template <std::size_t Idx>
    constexpr in_place_index_t<Idx> in_place_index{};

    /**
     * @brief Variable template to check if a type is a specialization of in_place_index_t.
     *        Primary template defaults to false.
     *
     *        检查类型是否为 in_place_index_t 特化的变量模板。
     *        主模板默认为 false。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <class>
    constexpr bool is_in_place_index_specialization = false;

    /**
     * @brief Variable template to check if a type is a specialization of in_place_index_t.
     *        Specialization for in_place_index_t types.
     *
     *        检查类型是否为 in_place_index_t 特化的变量模板。
     *        in_place_index_t 类型的特化。
     *
     * @tparam Idx Index value
     *             索引值
     */
    template <std::size_t Idx>
    constexpr bool is_in_place_index_specialization<utility::in_place_index_t<Idx>> = true;

    /**
     * @brief Alias for std::allocator_arg_t.
     *        Tag type for allocator construction disambiguation.
     *
     *        std::allocator_arg_t 的别名。
     *        用于分配器构造歧义消除的标签类型。
     */
    using allocator_arg_t = std::allocator_arg_t;

    /**
     * @brief Global constexpr instance of allocator_arg_t.
     *        allocator_arg_t 的全局 constexpr 实例。
     */
    inline constexpr allocator_arg_t allocator_arg{};
}

namespace rainy::utility {
    template <typename Callable>
    class finally_impl : Callable { // NOLINT
    public:
        finally_impl(Callable &&callable) noexcept : Callable(utility::forward<Callable>(callable)), invalidate_(false) { // NOLINT
        }

        ~finally_impl() {
            if (!is_invalidate()) {
                invoke_now();
            }
        }

        finally_impl(const finally_impl &) = delete;

        finally_impl &operator=(const finally_impl &) = delete;
        finally_impl &operator=(finally_impl &&) = delete;

        RAINY_NODISCARD bool is_invalidate() const noexcept {
            return invalidate_;
        }

        void to_invalidate() noexcept {
            invalidate_ = true;
        }

        void to_useable() noexcept {
            invalidate_ = false;
        }

        void invoke_now() const {
            (*this)();
        }

        template <typename Pred>
        void set_condition(Pred &&pred) {
            invalidate_ = static_cast<bool>(pred());
        }

    private:
        bool invalidate_;
    };

    template <typename Callable>
    auto make_finally(Callable &&callable) -> finally_impl<Callable> {
        return finally_impl<Callable>(utility::forward<Callable>(callable));
    }
}

// NOLINTBEGIN (bugprone-marco-parentheses)
#define RAINY_MEMBER_POINTER_TRAITS_SPEC(SPEC)                                                                                        \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct member_pointer_traits<Rx (Class::*)(Args...) SPEC> {                                                                       \
        static constexpr bool valid = true;                                                                                           \
        using class_type = Class;                                                                                                     \
        using return_type = Rx;                                                                                                       \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct member_pointer_traits<Rx (Class::*)(Args..., ...) SPEC> {                                                                  \
        using class_type = Class;                                                                                                     \
        using return_type = Rx;                                                                                                       \
        static constexpr bool valid = true;                                                                                           \
    };
// NOLINTEND (bugprone-marco-parenthese)

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4003)
#endif

namespace rainy::type_traits::primary_types {
    /**
     * @brief Primary template for member pointer traits.
     *        Provides information about member pointer types.
     *
     *        成员指针特性的主模板。
     *        提供关于成员指针类型的信息。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename>
    struct member_pointer_traits {
        /**
         * @brief Indicates whether the type is a valid member pointer.
         *        指示类型是否为有效的成员指针。
         */
        static RAINY_CONSTEXPR_BOOL valid = false;

        /**
         * @brief The class type that the member pointer belongs to.
         *        成员指针所属的类类型。
         */
        using class_type = void;
    };

    // Specializations for various member pointer cv-qualifier and ref-qualifier combinations
    // 各种成员指针 cv-限定符和引用限定符组合的特化
    RAINY_MEMBER_POINTER_TRAITS_SPEC()
    RAINY_MEMBER_POINTER_TRAITS_SPEC(&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(&&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const &)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const &&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(& noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(&& noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const & noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const && noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile &)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile &&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile & noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(volatile && noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile &)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile &&)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile & noexcept)
    RAINY_MEMBER_POINTER_TRAITS_SPEC(const volatile && noexcept)

    /**
     * @brief Specialization of member_pointer_traits for actual member pointer types.
     *        Provides the member type and class type for a valid member pointer.
     *
     *        member_pointer_traits 对于实际成员指针类型的特化。
     *        为有效的成员指针提供成员类型和类类型。
     *
     * @tparam Type The type of the member
     *              成员的类型
     * @tparam Class The class type that the member belongs to
     *               成员所属的类类型
     */
    template <typename Type, typename Class>
    struct member_pointer_traits<Type Class::*> {
        /**
         * @brief Indicates that this is a valid member pointer.
         *        指示这是一个有效的成员指针。
         */
        using type = Type;

        /**
         * @brief The class type that the member pointer belongs to.
         *        成员指针所属的类类型。
         */
        using class_type = Class;
    };
}

#undef RAINY_MEMBER_POINTER_TRAITS_SPEC

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

// NOLINTBEGIN(bugprone-macro-parentheses)
#define RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, SPEC)                                                    \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args...) SPEC>                                                                                          \
        : implements::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> {                                     \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename... Args>                                                                                          \
    struct function_traits<Rx(Args..., ...) SPEC>                                                                                     \
        : implements::function_traits_base<false, false, IsNothrowInvocable, IsVolatile, false> {                                     \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };

#define RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer, IsLValue, IsRValue, SPEC)  \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args...) SPEC>                                                                               \
        : implements::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                \
          implements::member_function_traits_base<IsLValue, IsRValue> {                                                               \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };                                                                                                                                \
    template <typename Rx, typename Class, typename... Args>                                                                          \
    struct function_traits<Rx (Class::*)(Args..., ...) SPEC>                                                                          \
        : implements::function_traits_base<true, false, IsNothrowInvocable, IsVolatile, IsConstMemberFunctionPointer>,                \
          implements::member_function_traits_base<IsLValue, IsRValue> {                                                               \
        using return_type = Rx;                                                                                                       \
        using argument_list = type_traits::other_trans::type_list<Args...>;                                                           \
        static inline constexpr std::size_t arity = sizeof...(Args);                                                                  \
    };
// NOLINTEND(bugprone-macro-parentheses)

namespace rainy::type_traits::implements {
    template <bool IsMemberFunctionPointer = false, bool IsFunctionPointer = false, bool IsNoexcept = false, bool IsVolatile = false,
              bool IsConstMemberFunction = false>
    struct function_traits_base {
        /**
         * @brief Indicates whether this is a member function pointer
         *        指示是否为成员函数指针
         */
        static RAINY_CONSTEXPR_BOOL is_member_function_pointer = IsMemberFunctionPointer;

        /**
         * @brief Indicates whether this is a function pointer
         *        指示是否为函数指针
         */
        static RAINY_CONSTEXPR_BOOL is_function_pointer = IsFunctionPointer;

        /**
         * @brief Indicates whether the function is noexcept
         *        指示函数是否为 noexcept
         */
        static RAINY_CONSTEXPR_BOOL is_noexcept = IsNoexcept;

        /**
         * @brief Indicates whether the member function is volatile-qualified
         *        指示成员函数是否为 volatile 限定
         */
        static RAINY_CONSTEXPR_BOOL is_volatile = IsVolatile;

        /**
         * @brief Indicates whether the member function is const-qualified
         *        指示成员函数是否为 const 限定
         */
        static RAINY_CONSTEXPR_BOOL is_const_member_function = IsConstMemberFunction;

        /**
         * @brief Indicates whether this is a valid function type
         *        指示是否为有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = true;

        /**
         * @brief Indicates whether this is a function object
         *        指示是否为函数对象
         */
        static RAINY_CONSTEXPR_BOOL is_function_object = false;
    };

    template <bool IsLvalue, bool IsRvalue>
    struct member_function_traits_base {
        /**
         * @brief Indicates whether the function can be invoked on lvalue objects
         *        指示函数是否可以在左值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = IsLvalue;

        /**
         * @brief Indicates whether the function can be invoked on rvalue objects
         *        指示函数是否可以在右值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = IsRvalue;
    };
}

namespace rainy::type_traits::implements {
    template <typename Fx, typename = void>
    struct fx_traits_has_invoke_operator : helper::false_type {};

    template <typename Fx>
    struct fx_traits_has_invoke_operator<
        Fx, type_traits::other_trans::void_t<decltype(&type_traits::cv_modify::remove_cv_t<Fx>::operator())>> : helper::true_type {};

    struct empty_function_traits {
        /**
         * @brief Indicates that this is not a valid function type
         *        指示这不是一个有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = false;
    };

    template <typename Fx, bool Enable = fx_traits_has_invoke_operator<Fx>::value>
    struct try_to_get_operator {
        using type = void;
    };

    template <typename Fx>
    struct try_to_get_operator<Fx, true> {
        using type = decltype(&type_traits::cv_modify::remove_cv_t<Fx>::operator());
    };

    template <typename Traits, bool Enable = Traits::valid>
    struct fn_obj_traits {
        /**
         * @brief List of argument types for the function object
         *        函数对象的参数类型列表
         */
        using argument_list = typename Traits::argument_list;

        /**
         * @brief Return type of the function object
         *        函数对象的返回类型
         */
        using return_type = typename Traits::return_type;

        /**
         * @brief Number of arguments the function object takes
         *        函数对象接受的参数数量
         */
        static inline constexpr std::size_t arity = Traits::arity;

        /**
         * @brief Indicates whether the function object is noexcept
         *        指示函数对象是否为 noexcept
         */
        static RAINY_CONSTEXPR_BOOL is_noexcept = Traits::is_noexcept;

        /**
         * @brief Indicates whether the function object is volatile-qualified
         *        指示函数对象是否为 volatile 限定
         */
        static RAINY_CONSTEXPR_BOOL is_volatile = Traits::is_volatile;

        /**
         * @brief Indicates whether the function can be invoked on lvalue objects
         *        指示函数是否可以在左值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = false;

        /**
         * @brief Indicates whether the function can be invoked on rvalue objects
         *        指示函数是否可以在右值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = false;

        /**
         * @brief Indicates that this is a function object
         *        指示这是一个函数对象
         */
        static RAINY_CONSTEXPR_BOOL is_function_object = true;

        /**
         * @brief Indicates that this is a valid function type
         *        指示这是一个有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = true;

        /**
         * @brief Indicates whether the member function is const-qualified
         *        指示成员函数是否为 const 限定
         */
        static RAINY_CONSTEXPR_BOOL is_const_member_function = Traits::is_const_member_function;
    };

    template <typename Traits>
    struct fn_obj_traits<Traits, false> : empty_function_traits {};
}

namespace rainy::type_traits::primary_types {
    /**
     * @brief Primary template for function traits.
     *        Provides comprehensive information about function types, function pointers, and function objects.
     *
     *        函数特性的主模板。
     *        提供关于函数类型、函数指针和函数对象的全面信息。
     *
     * @tparam Ty The type to examine (function type, function pointer, or function object)
     *            要检查的类型（函数类型、函数指针或函数对象）
     */
    template <typename Ty>
    struct function_traits
        : implements::fn_obj_traits<
              function_traits<typename implements::try_to_get_operator<type_traits::cv_modify::remove_cvref_t<Ty>>::type>> {};

    /**
     * @brief Specialization for void type.
     *        Provides empty traits for void.
     *
     *        void类型的特化。
     *        为void提供空特性。
     */
    template <>
    struct function_traits<void> : implements::empty_function_traits {};

    /**
     * @brief Specialization for normal function types.
     *        Provides traits for regular function types.
     *
     *        普通函数类型的特化。
     *        为常规函数类型提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Parameter types of the function
     *              函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args...)> : implements::function_traits_base<> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of parameter types
         *        参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of parameters
         *        参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for variadic function types.
     *        Provides traits for C-style variadic functions.
     *
     *        可变参数函数类型的特化。
     *        为C风格可变参数函数提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Fixed parameter types of the function
     *              函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args..., ...)> : implements::function_traits_base<> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of fixed parameter types (excluding variadic part)
         *        固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of fixed parameters
         *        固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    // Macro expansions for various cv-qualifier and noexcept combinations
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(false, true, volatile)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, false, noexcept)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, true, volatile noexcept)

    /**
     * @brief Specialization for function pointer types.
     *        Provides traits for regular function pointers.
     *
     *        函数指针类型的特化。
     *        为常规函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Parameter types of the function
     *              函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...)> : implements::function_traits_base<false, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of parameter types
         *        参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of parameters
         *        参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for variadic function pointer types.
     *        Provides traits for C-style variadic function pointers.
     *
     *        可变参数函数指针类型的特化。
     *        为C风格可变参数函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Fixed parameter types of the function
     *              函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...)> : implements::function_traits_base<false, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of fixed parameter types (excluding variadic part)
         *        固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = std::tuple<Args...>;

        /**
         * @brief Number of fixed parameters
         *        固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for noexcept function pointer types.
     *        Provides traits for noexcept function pointers.
     *
     *        noexcept函数指针类型的特化。
     *        为noexcept函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Parameter types of the function
     *              函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...) noexcept> : implements::function_traits_base<false, true, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of parameter types
         *        参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of parameters
         *        参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * @brief Specialization for variadic noexcept function pointer types.
     *        Provides traits for C-style variadic noexcept function pointers.
     *
     *        可变参数noexcept函数指针类型的特化。
     *        为C风格可变参数noexcept函数指针提供特性。
     *
     * @tparam Rx Return type of the function
     *            函数的返回类型
     * @tparam Args Fixed parameter types of the function
     *              函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...) noexcept> : implements::function_traits_base<false, true, true> {
        /**
         * @brief Return type of the function
         *        函数的返回类型
         */
        using return_type = Rx;

        /**
         * @brief List of fixed parameter types (excluding variadic part)
         *        固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * @brief Number of fixed parameters
         *        固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /*------------------
    [normal]
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, false, false, )
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, true, false, &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, false, false, true, &&)
    /*------------------
    (const)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, true, false, false, const)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, true, true, false, const &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, false, true, false, true, const &&)
    /*------------------
    (const noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, true, false, false, const noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, true, true, false, const & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, true, false, true, const && noexcept)
    /*------------------
    (const volatile)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, true, false, false, const volatile)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, true, true, false, const volatile &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, true, false, true, const volatile &&)
    /*------------------
    (const volatile noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, true, false, false, const volatile noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, true, true, false, const volatile & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, true, false, true, const volatile && noexcept)
    /*------------------
    (noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, false, false, false, noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, false, true, false, & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, false, false, false, true, && noexcept)
    /*------------------
    (volatile noexcept)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, false, false, false, volatile noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, false, true, false, volatile & noexcept)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(true, true, false, false, true, volatile && noexcept)
    /*------------------
    (volatile)
    ------------------*/
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, false, false, false, volatile)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, false, true, false, volatile &)
    RAINY_DECLARE_MEMBER_FUNCTION_TRAITS(false, true, false, false, true, volatile &&)

    /**
     * @brief Alias for function return type
     *        函数返回类型的别名
     *
     * @tparam Fx Function type to query
     *            要查询的函数类型
     */
    template <typename Fx>
    using function_return_type = typename function_traits<Fx>::return_type;

    /**
     * @brief Number of arguments for a function type
     *        函数类型的参数数量
     *
     * @tparam Fx Function type to query
     *            要查询的函数类型
     */
    template <typename Fx>
    static inline constexpr std::size_t arity = function_traits<Fx>::arity;

    /**
     * @brief List of argument types for a function type
     *        函数类型的参数类型列表
     *
     * @tparam Fx Function type to query
     *            要查询的函数类型
     */
    template <typename Fx>
    using function_argument_list = typename function_traits<Fx>::argument_list;

    /**
     * @brief Variable template for checking if a function type is variadic
     *        检查函数类型是否为可变参数的变量模板
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v = false;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx (*)(Args..., ...)> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...)> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...) volatile> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...) noexcept> = true;

    template <typename Rx, typename... Args>
    RAINY_CONSTEXPR_BOOL is_variadic_function_v<Rx(Args..., ...) volatile noexcept> = true;

    /**
     * @brief Primary template for constructing a new function type with modified parameter lists.
     *        Allows inserting type lists at the front and end of the original argument list.
     *
     *        用于构造具有修改后参数列表的新函数类型的主模板。
     *        允许在原始参数列表的前面和后面插入类型列表。
     *
     * @tparam NewRx The new return type for the function
     *               函数的新返回类型
     * @tparam Fx The original function type to base the new type on
     *            作为新类型基础的原始函数类型
     * @tparam TypeListFront Type list to insert at the beginning of the parameter list
     *                       要插入到参数列表开头的类型列表
     * @tparam TypeListEnd Type list to insert at the end of the parameter list
     *                     要插入到参数列表末尾的类型列表
     * @tparam ArgList The original argument list (deduced from Fx)
     *                 原始参数列表（从Fx推导）
     */
    template <typename NewRx, typename Fx, typename TypeListFront = other_trans::type_list<>,
              typename TypeListEnd = other_trans::type_list<>, typename ArgList = function_argument_list<Fx>>
    struct make_normalfx_type_with_pl {};

    /**
     * @brief Specialization that performs the actual type construction.
     *        Combines front list, original arguments, and end list into a new function type.
     *
     *        执行实际类型构造的特化。
     *        将前置列表、原始参数和后置列表组合成新的函数类型。
     *
     * @tparam NewRx The new return type
     *               新的返回类型
     * @tparam Fx The original function type
     *            原始函数类型
     * @tparam TypeListFrontArgs Types to insert at the front
     *                           要插入到前面的类型
     * @tparam TypeListEndArgs Types to insert at the end
     *                         要插入到后面的类型
     * @tparam OriginalArgs The original parameter types
     *                      原始参数类型
     */
    template <typename NewRx, typename Fx, typename... TypeListFrontArgs, typename... TypeListEndArgs, typename... OriginalArgs>
    struct make_normalfx_type_with_pl<NewRx, Fx, other_trans::type_list<TypeListFrontArgs...>,
                                      other_trans::type_list<TypeListEndArgs...>,
                                      type_traits::other_trans::type_list<OriginalArgs...>> {
        /**
         * @brief Helper template for constructing the new function type.
         *        Handles both regular functions and member functions.
         *
         *        用于构造新函数类型的辅助模板。
         *        处理普通函数和成员函数。
         *
         * @tparam UFx The function type (may be same as Fx)
         *             函数类型（可能与Fx相同）
         * @tparam IsMemPtr Indicates whether this is a member function pointer
         *                  指示是否为成员函数指针
         */
        template <typename UFx, bool IsMemPtr = function_traits<UFx>::is_member_function_pointer>
        struct helper {
            using fn_traits = function_traits<UFx>;

            using prototype = NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...);

            /**
             * @brief The resulting function type, with appropriate noexcept specification
             *        最终的函数类型，带有适当的noexcept说明符
             */
            using type =
                other_trans::conditional_t<fn_traits::is_noexcept,
                                           NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept, prototype>;
        };

        /**
         * @brief Specialization for member function pointers.
         *        Member function pointers have different syntax requirements.
         *
         *        成员函数指针的特化。
         *        成员函数指针有不同的语法要求。
         *
         * @tparam UFx The member function pointer type
         *             成员函数指针类型
         */
        template <typename UFx>
        struct helper<UFx, true> {
            using fn_traits = function_traits<UFx>;

            /**
             * @brief The resulting member function pointer type
             *        最终的成员函数指针类型
             */
            using type = other_trans::conditional_t<fn_traits::is_noexcept,
                                                    NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept,
                                                    NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...)>;
        };

        /**
         * @brief The constructed function type
         *        构造完成的函数类型
         */
        using type = typename helper<Fx>::type;
    };
}

namespace rainy::utility::implements {
    template <typename Ty>
    void refwrap_ctor_fun(type_traits::helper::identity_t<Ty &>) noexcept {
    }

    template <typename Ty>
    void refwrap_ctor_fun(type_traits::helper::identity_t<Ty &&>) = delete;

    template <typename Ty, typename Uty, typename = void>
    struct refwrap_has_ctor_from : type_traits::helper::false_type {};

    template <typename Ty, typename Uty>
    struct refwrap_has_ctor_from<Ty, Uty, type_traits::other_trans::void_t<decltype(refwrap_ctor_fun<Ty>(declval<Uty>()))>>
        : type_traits::helper::true_type {};

    template <typename Fx, typename... Args>
    struct test_refwrap_nothrow_invoke {
        static auto test() {
            if constexpr (std::is_invocable_v<Fx, Args...>) {
                if constexpr (type_traits::primary_types::function_traits<Fx>::is_nothrow_invocable) {
                    return type_traits::helper::true_type{};
                } else {
                    return type_traits::helper::false_type{};
                }
            } else {
                return type_traits::helper::false_type{};
            }
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test())::value;
    };
}

namespace rainy::utility {
    /**
     * @brief A wrapper that stores a reference to an object or function.
     *        Provides reference semantics in contexts where copies are normally made.
     *
     *        存储对象或函数引用的包装器。
     *        在通常进行拷贝的上下文中提供引用语义。
     *
     * @tparam Ty The type of the referenced object or function
     *            被引用对象或函数的类型
     */
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(type_traits::implements::_is_object_v<Ty> || type_traits::implements::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        /**
         * @brief The type of the referenced object or function
         *        被引用对象或函数的类型
         */
        using type = Ty;

        /**
         * @brief Constructs a reference_wrapper from a compatible type.
         *        从兼容类型构造 reference_wrapper。
         *
         * @tparam Uty The type of the value to wrap
         *             要包装的值的类型
         * @param val The value to wrap a reference to
         *            要包装引用的值
         */
        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<
                                                            type_traits::logical_traits::negation<type_traits::type_relations::is_same<
                                                                type_traits::cv_modify::remove_cvref_t<Uty>, reference_wrapper>>,
                                                            implements::refwrap_has_ctor_from<Ty, Uty>>,
                                                        int> = 0>
        constexpr reference_wrapper(Uty &&val) noexcept(noexcept(implements::refwrap_ctor_fun<Ty>(declval<Uty>()))) {
            Ty &ref = static_cast<Uty &&>(val);
            this->reference_data = utility::addressof(ref);
        }

        reference_wrapper(const reference_wrapper &) = delete;
        reference_wrapper(reference_wrapper &&) = delete;

        /**
         * @brief Conversion operator to the referenced type.
         *        到被引用类型的转换运算符。
         *
         * @return Reference to the wrapped object
         *         被包装对象的引用
         */
        constexpr operator Ty &() const noexcept {
            return *reference_data;
        }

        /**
         * @brief Gets the wrapped reference.
         *        获取被包装的引用。
         *
         * @return Reference to the wrapped object
         *         被包装对象的引用
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get() const noexcept {
            return *reference_data;
        }

        /**
         * @brief Invokes the wrapped function with the given arguments.
         *        使用给定参数调用被包装的函数。
         *
         * @tparam Args Argument types
         *              参数类型
         * @param args Arguments to forward to the function
         *             要转发给函数的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <typename... Args, type_traits::other_trans::enable_if_t<std::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) try_to_invoke_as_function(Args &&...args) const
            noexcept(implements::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            using f_traits = type_traits::primary_types::function_traits<Ty>;
            if constexpr (f_traits::valid) {
                using return_type = typename f_traits::return_type;
                if constexpr (std::is_invocable_r_v<return_type, Ty, Args...>) {
                    if constexpr (type_traits::primary_types::is_void_v<return_type>) {
                        get(utility::forward<Args>(args)...);
                    } else {
                        return get()(utility::forward<Args>(args)...);
                    }
                }
            }
        }

        /**
         * @brief Function call operator to invoke the wrapped function.
         *        调用被包装函数的函数调用运算符。
         *
         * @tparam Args Argument types
         *              参数类型
         * @param args Arguments to forward to the function
         *             要转发给函数的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <typename... Args, type_traits::other_trans::enable_if_t<std::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) operator()(Args &&...args) const
            noexcept(implements::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            return try_to_invoke_as_function(utility::forward<Args>(args)...);
        }

    private:
        Ty *reference_data{nullptr};
    };

    /**
     * @brief Deduction guide for reference_wrapper.
     *        reference_wrapper 的推导指引。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     */
    template <class Ty>
    reference_wrapper(Ty &) -> reference_wrapper<Ty>;

    /**
     * @brief Deduction guide for reference_wrapper with cv-qualifier removal.
     *        带有 cv-限定符移除的 reference_wrapper 推导指引。
     *
     * @tparam Uty The type of the referenced object
     *             被引用对象的类型
     */
    template <typename Uty>
    reference_wrapper(Uty &) -> reference_wrapper<type_traits::cv_modify::remove_cvref_t<Uty>>;

    /**
     * @brief Deleted overload for rvalue references.
     *        右值引用的已删除重载。
     *
     * @tparam Ty The type of the rvalue
     *            右值的类型
     */
    template <typename Ty>
    void ref(const Ty &&) = delete;

    /**
     * @brief Deleted overload for rvalue references.
     *        右值引用的已删除重载。
     *
     * @tparam Ty The type of the rvalue
     *            右值的类型
     */
    template <typename Ty>
    void cref(const Ty &&) = delete;

    /**
     * @brief Creates a reference_wrapper to the given object.
     *        为给定对象创建 reference_wrapper。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The object to wrap a reference to
     *            要包装引用的对象
     * @return A reference_wrapper to the object
     *         对象的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(Ty &val) noexcept {
        return reference_wrapper<Ty>(val);
    }

    /**
     * @brief Returns the given reference_wrapper unchanged.
     *        原样返回给定的 reference_wrapper。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The reference_wrapper to return
     *            要返回的 reference_wrapper
     * @return The same reference_wrapper
     *         相同的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(reference_wrapper<Ty> val) noexcept {
        return val;
    }

    /**
     * @brief Creates a const-qualified reference_wrapper to the given object.
     *        为给定对象创建 const 限定的 reference_wrapper。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The object to wrap a const reference to
     *            要包装 const 引用的对象
     * @return A const-qualified reference_wrapper to the object
     *         对象的 const 限定 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(const Ty &val) noexcept {
        return reference_wrapper<const Ty>(val);
    }

    /**
     * @brief Returns the given reference_wrapper as a const-qualified wrapper.
     *        将给定的 reference_wrapper 作为 const 限定包装器返回。
     *
     * @tparam Ty The type of the referenced object
     *            被引用对象的类型
     * @param val The reference_wrapper to convert
     *            要转换的 reference_wrapper
     * @return A const-qualified reference_wrapper
     *         const 限定的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(reference_wrapper<Ty> val) noexcept {
        return val;
    }
}

namespace rainy::type_traits::cv_modify {
    /**
     * @brief Primary template for unwrapping reference wrappers.
     *        If Ty is not a reference wrapper, returns Ty unchanged.
     *
     *        解包引用包装器的主模板。
     *        如果 Ty 不是引用包装器，则原样返回 Ty。
     *
     * @tparam Ty The type to potentially unwrap
     *            可能需要解包的类型
     */
    template <typename Ty>
    struct unwrap_reference {
        using type = Ty;
    };

    /**
     * @brief Specialization for rainy::utility::reference_wrapper.
     *        Extracts the underlying type from a reference_wrapper.
     *
     *        rainy::utility::reference_wrapper 的特化。
     *        从 reference_wrapper 中提取底层类型。
     *
     * @tparam Ty The type wrapped by reference_wrapper
     *            reference_wrapper 包装的类型
     */
    template <typename Ty>
    struct unwrap_reference<utility::reference_wrapper<Ty>> {
        using type = Ty;
    };

    /**
     * @brief Specialization for std::reference_wrapper.
     *        Extracts the underlying type from a std::reference_wrapper.
     *
     *        std::reference_wrapper 的特化。
     *        从 std::reference_wrapper 中提取底层类型。
     *
     * @tparam Ty The type wrapped by std::reference_wrapper
     *            std::reference_wrapper 包装的类型
     */
    template <typename Ty>
    struct unwrap_reference<std::reference_wrapper<Ty>> {
        using type = Ty;
    };

    /**
     * @brief Alias template for unwrapping reference wrappers.
     *        解包引用包装器的别名模板。
     *
     * @tparam Ty The type to potentially unwrap
     *            可能需要解包的类型
     */
    template <typename Ty>
    using unwrap_reference_t = typename unwrap_reference<Ty>::type;

    /**
     * @brief Alias template that decays and then unwraps a type.
     *        先退化然后解包类型的别名模板。
     *
     * @tparam Ty The type to decay and unwrap
     *            要退化并解包的类型
     */
    template <typename Ty>
    using unwrap_ref_decay_t = unwrap_reference_t<other_trans::decay_t<Ty>>;

    /**
     * @brief Type template for decaying and unwrapping a type.
     *        退化并解包类型的类型模板。
     *
     * @tparam Ty The type to decay and unwrap
     *            要退化并解包的类型
     */
    template <typename Ty>
    struct unwrap_ref_decay {
        using type = unwrap_ref_decay_t<Ty>;
    };
}

namespace rainy::utility {
    enum class invoker_strategy : std::uint8_t {
        functor, // 仿函数或函数类型
        pmf_object, // 类成员函数——对象调用
        pmf_refwrap, // 类成员函数——对象引用包装器调用
        pmf_pointer, // 类成员函数——对象指针调用
        pmd_object, // 类成员变量——对象调用
        pmd_refwrap, // 类成员变量——对象引用包装器调用
        pmd_pointer // 类成员变量——对象指针调用
    };
}

namespace rainy::utility::implements {
    struct invoker_functor {
        static constexpr auto strategy = invoker_strategy::functor;

        template <typename Callable, typename... Args>
        static constexpr auto invoke(Callable &&object, Args &&...args) noexcept(
            noexcept(static_cast<Callable &&>(object)(static_cast<Args &&>(args)...))) //
            -> decltype(static_cast<Callable &&>(object)(static_cast<Args &&>(args)...)) {
            return static_cast<Callable &&>(object)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_object {
        static constexpr auto strategy = invoker_strategy::pmf_object;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept((static_cast<Ty &&>(args1).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)) {
            return (static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_refwrap {
        static constexpr auto strategy = invoker_strategy::pmf_refwrap;

        template <typename Decayed, typename Refwrap, typename... Args>
        static constexpr auto invoke(Decayed _Pmf, Refwrap _Rw,
                                     Args &&...args) noexcept(noexcept((_Rw.get().*_Pmf)(static_cast<Args &&>(args)...))) //
            -> decltype((_Rw.get().*_Pmf)(static_cast<Args &&>(args)...)) {
            return (_Rw.get().*_Pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmf_pointer {
        static constexpr auto strategy = invoker_strategy::pmf_pointer;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept(((*static_cast<Ty &&>(args1)).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)) {
            return ((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    struct invoker_pmd_object {
        static constexpr auto strategy = invoker_strategy::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept -> decltype(static_cast<Ty &&>(args).*pmd) {
            return static_cast<Ty &&>(args).*pmd;
        }
    };

    struct invoker_pmd_refwrap {
        static constexpr auto strategy = invoker_strategy::pmd_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty>
        static constexpr auto invoke(Decayed pmd, RefWrap<Ty> ref_wrap) noexcept -> decltype(ref_wrap.get().*pmd) {
            return ref_wrap.get().*pmd;
        }
    };

    struct invoker_pmd_pointer {
        static constexpr auto strategy = invoker_strategy::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept(noexcept((*static_cast<Ty &&>(args)).*pmd))
            -> decltype((*static_cast<Ty &&>(args)).*pmd) {
            return (*static_cast<Ty &&>(args)).*pmd;
        }
    };


    template <typename Callable, typename Ty1, typename RemoveCvref = type_traits::cv_modify::remove_cvref_t<Callable>,
              bool _Is_pmf = type_traits::primary_types::is_member_function_pointer_v<RemoveCvref>,
              bool _Is_pmd = type_traits::primary_types::is_member_object_pointer_v<RemoveCvref>>
    struct invoker_impl;

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, true, false>
        : std::conditional_t<
              std::is_same_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                             type_traits::cv_modify::remove_cvref_t<Ty1>> ||
                  std::is_base_of_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                                    type_traits::cv_modify::remove_cvref_t<Ty1>>,
              invoker_pmf_object,
              std::conditional_t<rainy::type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                                        std::reference_wrapper>,
                                 invoker_pmf_refwrap, invoker_pmf_pointer>> {};

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, false, true>
        : std::conditional_t<
              std::is_same_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                             type_traits::cv_modify::remove_cvref_t<Ty1>> ||
                  std::is_base_of_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                                    type_traits::cv_modify::remove_cvref_t<Ty1>>,
              invoker_pmd_object,
              std::conditional_t<rainy::type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                                        std::reference_wrapper>,
                                 invoker_pmd_refwrap, invoker_pmd_pointer>> {};

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, false, false> : invoker_functor {};

    template <typename Callable, typename Ty1>
    struct select_invoker {
        static constexpr auto value = invoker_impl<Callable, Ty1>::strategy;
    };
}

namespace rainy::utility {
    /**
     * @brief Primary template for function object invoker.
     *        Provides a unified interface for invoking various callable types.
     *
     *        函数对象调用器的主模板。
     *        为调用各种可调用类型提供统一接口。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Ty1 The type of the first argument (or void if not applicable)
     *             第一个参数的类型（如果不适用则为void）
     */
    template <typename Callable, typename Ty1 = void>
    struct invoker : implements::invoker_impl<Callable, Ty1> {};
}

namespace rainy::utility {
    /**
     * @brief Invokes a callable object with no arguments.
     *        调用无参数的可调用对象。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @param obj The callable object to invoke
     *            要调用的可调用对象
     * @return The result of invoking the callable object
     *         调用可调用对象的结果
     */
    template <typename Callable>
    constexpr RAINY_INLINE auto invoke(Callable &&obj) noexcept(noexcept(static_cast<Callable &&>(obj)()))
        -> decltype(static_cast<Callable &&>(obj)()) {
        return static_cast<Callable &&>(obj)();
    }

    /**
     * @brief Invokes a callable object with one or more arguments.
     *        调用带有一个或多个参数的可调用对象。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Ty1 The type of the first argument
     *             第一个参数的类型
     * @tparam Args The types of the remaining arguments
     *              剩余参数的类型
     * @param obj The callable object to invoke
     *            要调用的可调用对象
     * @param arg1 The first argument
     *             第一个参数
     * @param args The remaining arguments
     *             剩余参数
     * @return The result of invoking the callable object with the given arguments
     *         使用给定参数调用可调用对象的结果
     */
    template <typename Callable, typename Ty1, typename... Args>
    constexpr RAINY_INLINE auto invoke(Callable &&obj, Ty1 &&arg1, Args &&...args) noexcept(noexcept(
        invoker<Callable, Ty1>::invoke(static_cast<Callable &&>(obj), static_cast<Ty1 &&>(arg1), static_cast<Args &&>(args)...)))
        -> decltype(invoker<Callable, Ty1>::invoke(static_cast<Callable &&>(obj), static_cast<Ty1 &&>(arg1),
                                                   static_cast<Args &&>(args)...)) {
        return invoker<Callable, Ty1>::invoke(utility::forward<Callable>(obj), utility::forward<Ty1>(arg1),
                                              utility::forward<Args>(args)...);
    }
}

namespace rainy::utility::implements {
    template <typename Callable, typename Tuple, std::size_t... Indices>
    constexpr decltype(auto) apply_impl(Callable &&obj, Tuple &&tuple, type_traits::helper::index_sequence<Indices...>) noexcept(
        noexcept(utility::invoke(utility::forward<Callable>(obj), std::get<Indices>(utility::forward<Tuple>(tuple))...))) {
        return utility::invoke(utility::forward<Callable>(obj), std::get<Indices>(utility::forward<Tuple>(tuple))...);
    }
}

namespace rainy::utility {
    /**
     * @brief Invokes a callable object with the arguments from a tuple.
     *        使用元组中的参数调用可调用对象。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Tuple The type of the tuple containing the arguments
     *               包含参数的元组类型
     * @param obj The callable object to invoke
     *            要调用的可调用对象
     * @param tuple The tuple containing the arguments to pass to the callable
     *              包含要传递给可调用对象的参数的元组
     * @return The result of invoking the callable object with the unpacked tuple arguments
     *         使用解包后的元组参数调用可调用对象的结果
     */
    template <typename Callable, typename Tuple>
    constexpr decltype(auto) apply(Callable &&obj, Tuple &&tuple) noexcept(noexcept(implements::apply_impl(
        utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
        type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::reference_modify::remove_reference_t<Tuple>>>{}))) {
        return implements::apply_impl(
            utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
            type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::reference_modify::remove_reference_t<Tuple>>>{});
    }
}

namespace rainy::type_traits::implements {
    template <typename Rx, bool NoThrow>
    struct invoke_traits_common {
        using type = Rx;
        using is_invocable = helper::true_type;
        using is_nothrow_invocable = helper::bool_constant<NoThrow>;

        template <typename Rx_>
        using is_invocable_r = helper::bool_constant<
            logical_traits::disjunction_v<primary_types::is_void<Rx>, type_properties::is_invoke_convertible<type, Rx_>>>;

        template <typename Rx_>
        using is_nothrow_invocable_r = helper::bool_constant<logical_traits::conjunction_v<
            is_nothrow_invocable,
            logical_traits::disjunction<primary_types::is_void<Rx>,
                                        logical_traits::conjunction<type_properties::is_invoke_convertible<type, Rx_>,
                                                                    type_properties::is_invoke_nothrow_convertible<type, Rx_>>>>>;
    };

    template <typename Void, typename Callable>
    struct invoke_traits_zero {
        using is_invocable = helper::false_type;
        using is_nothrow_invocable = helper::false_type;
        template <typename Rx>
        using is_invocable_r = helper::false_type;
        template <typename Rx>
        using is_nothrow_invocable_r = helper::false_type;
        using is_void_ = primary_types::is_void<Void>;
        using Callable_type_ = Callable;
    };

    template <typename Callable>
    using decltype_invoke_zero = decltype(utility::declval<Callable>()());

    template <typename Callable>
    struct invoke_traits_zero<other_trans::void_t<decltype_invoke_zero<Callable>>, Callable>
        : invoke_traits_common<decltype_invoke_zero<Callable>, noexcept(utility::declval<Callable>()())> {};

    template <typename Void, typename... Args>
    struct invoke_traits_nonzero {
        using is_invocable = helper::false_type;
        using is_nothrow_invocable = helper::false_type;

        template <typename Rx>
        using is_invocable_r = helper::false_type;

        template <typename Rx>
        using is_nothrow_invocable_r = helper::false_type;

        using is_void_ = primary_types::is_void<Void>;
    };

    template <typename Callable, typename Ty1, typename... Args>
    using decltype_invoke_nonzero = decltype(utility::invoker<Callable, Ty1>::invoke(
        utility::declval<Callable>(), utility::declval<Ty1>(), utility::declval<Args>()...));

    template <typename Callable, typename Ty1, typename... Args>
    struct invoke_traits_nonzero<other_trans::void_t<decltype_invoke_nonzero<Callable, Ty1, Args...>>, Callable, Ty1, Args...>
        : invoke_traits_common<decltype_invoke_nonzero<Callable, Ty1, Args...>,
                               noexcept(utility::invoker<Callable, Ty1>::invoke(utility::declval<Callable>(), utility::declval<Ty1>(),
                                                                                utility::declval<Args>()...))> {};

    template <typename Callable, typename... Args>
    using select_invoke_traits = other_trans::conditional_t<sizeof...(Args) == 0, invoke_traits_zero<void, Callable>,
                                                            invoke_traits_nonzero<void, Callable, Args...>>;

    template <typename Rx, typename Callable, typename... Args>
    using is_invocable_r_helper = typename select_invoke_traits<Callable, Args...>::template is_invocable_r<Rx>;

    template <typename Callable, typename... Args>
    using is_invocable_helper = typename select_invoke_traits<Callable, Args...>::is_invocable;

    template <typename Callable, typename... Args>
    using is_nothrow_invocable_helper = typename select_invoke_traits<Callable, Args...>::is_nothrow_invocable;

    template <typename Rx, typename Callable, typename... Args>
    using is_nothrow_invocable_r_helper = typename select_invoke_traits<Callable, Args...>::template is_nothrow_invocable_r<Rx>;
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *        and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用，并且结果可以转换为指定的返回类型的变量模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_r_v = implements::is_invocable_r_helper<Rx, Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *        and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用，并且结果可以转换为指定的返回类型的类型模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_r : helper::bool_constant<is_invocable_r_v<Rx, Callable, Args...>> {};

    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments.
     *
     *        检查可调用对象是否可以使用给定参数调用的变量模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_v = implements::is_invocable_helper<Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments.
     *
     *        检查可调用对象是否可以使用给定参数调用的类型模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    struct is_invocable : helper::bool_constant<is_invocable_v<Callable, Args...>> {};

    /**
     * @brief Type template that yields the result type of invoking a callable object with the given arguments.
     *
     *        产生使用给定参数调用可调用对象的结果类型的类型模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    struct invoke_result {
        using type = typename implements::select_invoke_traits<Callable, Args...>::type;
    };

    /**
     * @brief Alias template for invoke_result, providing direct access to the result type.
     *
     *        invoke_result 的别名模板，提供对结果类型的直接访问。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    using invoke_result_t = typename invoke_result<Callable, Args...>::type;

    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常的变量模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_v = implements::is_nothrow_invocable_helper<Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常的类型模板。
     *
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Callable, typename... Args>
    struct is_nothrow_invocable : helper::bool_constant<is_nothrow_invocable_v<Callable, Args...>> {};

    /**
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions, and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常，并且结果可以转换为指定的返回类型的变量模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_r_v = implements::is_nothrow_invocable_r_helper<Rx, Callable, Args...>::value;

    /**
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *        without throwing exceptions, and the result is convertible to the specified return type.
     *
     *        检查可调用对象是否可以使用给定参数调用且不抛出异常，并且结果可以转换为指定的返回类型的类型模板。
     *
     * @tparam Rx The required return type
     *            要求的返回类型
     * @tparam Callable The type of the callable object
     *                  可调用对象的类型
     * @tparam Args The types of the arguments
     *              参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    struct is_nothrow_invocable_r : helper::bool_constant<is_nothrow_invocable_r_v<Rx, Callable, Args...>> {};
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Variable template for checking if a type is const-qualified.
     *        检查类型是否为 const 限定的变量模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_v = false;

    /**
     * @brief Specialization for const-qualified types.
     *        const 限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_v<const Ty> = true;

    /**
     * @brief Type template for checking if a type is const-qualified.
     *        检查类型是否为 const 限定的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_const : helper::bool_constant<is_const_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is volatile-qualified.
     *        检查类型是否为 volatile 限定的变量模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_volatile_v = false;

    /**
     * @brief Specialization for volatile-qualified types.
     *        volatile 限定类型的特化。
     *
     * @tparam Ty The underlying type
     *            底层类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_volatile_v<volatile Ty> = true;

    /**
     * @brief Type template for checking if a type is volatile-qualified.
     *        检查类型是否为 volatile 限定的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct is_volatile : helper::bool_constant<is_volatile_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is trivial.
     *        A trivial type has a trivial default constructor, copy/move constructors,
     *        copy/move assignment operators, and destructor.
     *
     *        检查类型是否为平凡类型的变量模板。
     *        平凡类型具有平凡的默认构造函数、复制/移动构造函数、
     *        复制/移动赋值运算符和析构函数。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
#if RAINY_USING_CLANG || RAINY_USING_MSVC
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivial_v = __is_trivially_constructible(Ty) && __is_trivially_copyable(Ty);
#else
    /**
     * @brief Variable template for checking if a type is trivial (GCC implementation).
     *        If type Ty is a trivial type, the instance is true; otherwise false.
     *        Trivial types are scalar types, trivially copyable class types,
     *        arrays of these types, and cv-qualified versions of these types.
     *
     *        检查类型是否为平凡类型的变量模板（GCC实现）。
     *        如果类型 Ty 是平凡类型，则实例为 true；否则为 false。
     *        平凡类型是标量类型、完全可复制类类型、这些类型的数组
     *        以及这些类型的 cv 限定版本。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_trivial_v = __is_trivial(Ty);
#endif

    /**
     * @brief Type template for checking if a type is trivial.
     *        检查类型是否为平凡类型的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     * @remark If type Ty is a trivial type, the instance is true; otherwise false.
     *         Trivial types are scalar types, trivially copyable class types,
     *         arrays of these types, and cv-qualified versions of these types.
     *         如果类型 Ty 是平凡类型，则实例为 true；否则为 false。
     *         平凡类型是标量类型、完全可复制类类型、这些类型的数组
     *         以及这些类型的 cv 限定版本。
     */
    template <typename Ty>
    struct is_trivial : helper::bool_constant<is_trivial_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has unique object representations.
     *        Indicates whether every object of type Ty has a unique representation
     *        (i.e., no padding bits).
     *
     *        检查类型是否具有唯一对象表示的变量模板。
     *        指示类型 Ty 的每个对象是否具有唯一的表示形式（即没有填充位）。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_unique_object_representations_v = __has_unique_object_representations(Ty);

    /**
     * @brief Type template for checking if a type has unique object representations.
     *        检查类型是否具有唯一对象表示的类型模板。
     *
     * @tparam Ty Type to query
     *            要查询的类型
     */
    template <typename Ty>
    struct has_unique_object_representations : helper::bool_constant<has_unique_object_representations_v<Ty>> {};
}

namespace rainy::type_traits::extras::templates {
    /**
     * @brief Primary template for getting the first template parameter of a template instantiation.
     *        获取模板实例化的第一个模板参数的主模板。
     *
     * @tparam Ty The template instantiation type
     *            模板实例化类型
     */
    template <typename Ty>
    struct get_first_parameter;

    /**
     * @brief Specialization that extracts the first template parameter.
     *        提取第一个模板参数的特化。
     *
     * @tparam Ty The template template parameter
     *            模板模板参数
     * @tparam First The first template parameter type
     *               第一个模板参数类型
     * @tparam Rest The remaining template parameter types
     *              剩余的模板参数类型
     */
    template <template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct get_first_parameter<Ty<First, Rest...>> {
        using type = First;
    };

    /**
     * @brief Primary template for replacing the first template parameter of a template instantiation.
     *        替换模板实例化的第一个模板参数的主模板。
     *
     * @tparam newfirst The new type for the first parameter
     *                  第一个参数的新类型
     * @tparam Ty The template instantiation type
     *            模板实例化类型
     */
    template <typename newfirst, typename Ty>
    struct replace_first_parameter;

    /**
     * @brief Specialization that performs the replacement.
     *        执行替换操作的特化。
     *
     * @tparam NewFirst The new type for the first parameter
     *                  第一个参数的新类型
     * @tparam Ty The template template parameter
     *            模板模板参数
     * @tparam First The original first parameter type (to be replaced)
     *               原始的第一个参数类型（将被替换）
     * @tparam Rest The remaining template parameter types (preserved)
     *              剩余的模板参数类型（保持不变）
     */
    template <typename NewFirst, template <typename, typename...> typename Ty, typename First, typename... Rest>
    struct replace_first_parameter<NewFirst, Ty<First, Rest...>> {
        using type = Ty<NewFirst, Rest...>;
    };

    /**
     * @brief Primary template for getting the pointer difference type.
     *        Defaults to ptrdiff_t.
     *
     *        获取指针差类型的主模板。
     *        默认为 ptrdiff_t。
     *
     * @tparam Ty The type to query for difference_type
     *            要查询 difference_type 的类型
     */
    template <typename, typename = void>
    struct get_ptr_difference_type {
        using type = ptrdiff_t;
    };

    /**
     * @brief Specialization for types that provide a difference_type member.
     *        为提供 difference_type 成员的类型提供的特化。
     *
     * @tparam Ty The type that provides difference_type
     *            提供 difference_type 的类型
     */
    template <typename Ty>
    struct get_ptr_difference_type<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    /**
     * @brief Primary template for getting the rebound alias of an allocator or similar template.
     *        Uses replace_first_parameter as fallback.
     *
     *        获取分配器或类似模板的重绑定别名的主模板。
     *        使用 replace_first_parameter 作为回退。
     *
     * @tparam Ty The template type to rebind
     *            要重绑定的模板类型
     * @tparam Other The new type to bind to
     *               要绑定到的新类型
     */
    template <typename Ty, typename Other, typename = void>
    struct get_rebind_alias {
        using type = typename replace_first_parameter<Other, Ty>::type;
    };

    /**
     * @brief Specialization for types that provide a rebind member template.
     *        为提供 rebind 成员模板的类型提供的特化。
     *
     * @tparam Ty The type that provides rebind
     *            提供 rebind 的类型
     * @tparam Other The new type to bind to
     *               要绑定到的新类型
     */
    template <typename Ty, typename Other>
    struct get_rebind_alias<Ty, Other, type_traits::other_trans::void_t<typename Ty::template rebind<Other>>> {
        using type = typename Ty::template rebind<Other>;
    };
}

namespace rainy::foundation::memory::implements {
    template <typename Ty, typename Elem>
    struct pointer_traits_base {
        using pointer = Ty;
        using element_type = Elem;
        using difference_type = typename type_traits::extras::templates::get_ptr_difference_type<Ty>::type;

        template <typename other>
        using rebind = typename type_traits::extras::templates::get_rebind_alias<Ty, other>::type;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Elem>, char, Elem> &;

        /**
         * @brief Creates a pointer to the given reference using the type's pointer_to function.
         *        使用类型的 pointer_to 函数创建指向给定引用的指针。
         *
         * @param val The reference to create a pointer to
         *            要创建指针的引用
         * @return A pointer created by Ty::pointer_to(val)
         *         由 Ty::pointer_to(val) 创建的指针
         */
        RAINY_NODISCARD static RAINY_CONSTEXPR20 rain_fn pointer_to(ref_type val) noexcept(noexcept(Ty::pointer_to(val))) -> pointer {
            return Ty::pointer_to(val);
        }
    };

    template <typename, typename = void, typename = void>
    struct ptr_traits_sfinae_layer {};

    template <typename Ty, typename Uty>
    struct ptr_traits_sfinae_layer<
        Ty, Uty, type_traits::other_trans::void_t<typename type_traits::extras::templates::get_first_parameter<Ty>::type>>
        : implements::pointer_traits_base<Ty, typename type_traits::extras::templates::get_first_parameter<Ty>::type> {};

    template <typename Ty>
    struct ptr_traits_sfinae_layer<Ty, type_traits::other_trans::void_t<typename Ty::element_type>, void>
        : implements::pointer_traits_base<Ty, typename Ty::element_type> {};
}

namespace rainy::foundation::memory {
    template <typename Ty>
    struct pointer_traits : implements::ptr_traits_sfinae_layer<Ty> {};

    template <typename Ty>
    struct pointer_traits<Ty *> {
        using pointer = Ty *;
        using elemen_type = Ty;
        using difference_type = ptrdiff_t;

        template <typename other>
        using rebind = other *;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Ty>, char, Ty> &;

        /**
         * @brief Creates a pointer to the given reference using addressof.
         *        使用 addressof 创建指向给定引用的指针。
         *
         * @param val The reference to create a pointer to
         *            要创建指针的引用
         * @return A pointer to the referenced object
         *         指向被引用对象的指针
         */
        RAINY_NODISCARD static constexpr rain_fn pointer_to(ref_type val) noexcept -> pointer {
            return utility::addressof(val);
        }
    };
}

namespace rainy::utility {
    using foundation::memory::pointer_traits;
}

namespace rainy::utility::implements {
    /**
     * @brief Variable template for detecting if pointer_traits<Pointer> has a to_address member function.
     *        检测 pointer_traits<Pointer> 是否具有 to_address 成员函数的变量模板。
     *
     * @tparam Ty The pointer type to check
     *            要检查的指针类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_to_address = false;

    /**
     * @brief Specialization that detects the presence of pointer_traits<Pointer>::to_address.
     *        检测 pointer_traits<Pointer>::to_address 是否存在的特化。
     *
     * @tparam Ty The pointer type that provides to_address
     *            提供 to_address 的指针类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_to_address<Ty, type_traits::other_trans::void_t<decltype(foundation::memory::pointer_traits<Ty>::to_address(
                               utility::declval<const Ty &>()))>> = true;
}

namespace rainy::utility {
    /**
     * @brief Converts a raw pointer to an address (identity function).
     *        将原始指针转换为地址（恒等函数）。
     *
     * @tparam Ty The type pointed to
     *            指向的类型
     * @param val The raw pointer
     *            原始指针
     * @return The same pointer value
     *         相同的指针值
     */
    template <typename Ty>
    constexpr rain_fn to_address(Ty *const val) noexcept -> Ty * {
        static_assert(!type_traits::primary_types::is_function_v<Ty>, "Ty cannot be a function type.");
        return val;
    }

    /**
     * @brief Converts any fancy pointer to a raw address.
     *        将任何花哨指针转换为原始地址。
     *
     * @tparam Pointer The fancy pointer type
     *                 花哨指针类型
     * @param val The fancy pointer to convert
     *            要转换的花哨指针
     * @return The raw address obtained either from pointer_traits or operator->
     *         从 pointer_traits 或 operator-> 获取的原始地址
     */
    template <typename Pointer>
    RAINY_NODISCARD constexpr rain_fn to_address(const Pointer &val) noexcept -> auto {
        if constexpr (implements::has_to_address<Pointer>) {
            return pointer_traits<Pointer>::to_address(val);
        } else {
            return utility::to_address(val.operator->());
        }
    }
}

namespace rainy::type_traits::composite_types {
    /**
     * @brief Variable template for checking if a type is arithmetic.
     *        Arithmetic types include integral and floating-point types.
     *
     *        检查类型是否为算术类型的变量模板。
     *        算术类型包括整型和浮点型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = implements::is_arithmetic_v<Ty>;

    /**
     * @brief Type template for checking if a type is arithmetic.
     *        检查类型是否为算术类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_arithmetic : helper::bool_constant<is_arithmetic_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is fundamental.
     *        Fundamental types include arithmetic types, void, and nullptr_t.
     *
     *        检查类型是否为基本类型的变量模板。
     *        基本类型包括算术类型、void 和 nullptr_t。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_fundamental_v =
        is_arithmetic_v<Ty> || primary_types::is_void_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    /**
     * @brief Type template for checking if a type is fundamental.
     *        检查类型是否为基本类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_fundamental : helper::bool_constant<is_fundamental_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is an object type.
     *        Object types are cv-qualifiable and not void.
     *
     *        检查类型是否为对象类型的变量模板。
     *        对象类型可以具有 cv 限定符，且不是 void。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_object_v = type_properties::is_const_v<const Ty> && !primary_types::is_void_v<Ty>;

    /**
     * @brief Type template for checking if a type is an object type.
     *        检查类型是否为对象类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_object : helper::bool_constant<is_object_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a reference.
     *        References include both lvalue and rvalue references.
     *
     *        检查类型是否为引用类型的变量模板。
     *        引用类型包括左值引用和右值引用。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_reference_v = implements::_is_reference_v<Ty>;

    /**
     * @brief Type template for checking if a type is a reference.
     *        检查类型是否为引用类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_reference : helper::bool_constant<implements::_is_reference_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is a pointer to member.
     *        Member pointers can point to either data members or member functions.
     *
     *        检查类型是否为成员指针的变量模板。
     *        成员指针可以指向数据成员或成员函数。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
#if RAINY_USING_CLANG
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_pointer_v = __is_member_pointer(Ty);
#else
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_member_pointer_v =
        primary_types::is_member_object_pointer_v<Ty> || primary_types::is_member_function_pointer_v<Ty>;
#endif

    /**
     * @brief Type template for checking if a type is a pointer to member.
     *        检查类型是否为成员指针的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_member_pointer : helper::bool_constant<is_member_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is scalar.
     *        Scalar types include arithmetic, enum, pointer, member pointer, and nullptr_t.
     *
     *        检查类型是否为标量类型的变量模板。
     *        标量类型包括算术、枚举、指针、成员指针和 nullptr_t。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_scalar_v = is_arithmetic_v<Ty> || primary_types::is_enum_v<Ty> || primary_types::is_pointer_v<Ty> ||
                                       is_member_pointer_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    /**
     * @brief Type template for checking if a type is scalar.
     *        检查类型是否为标量类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_scalar : helper::bool_constant<is_scalar_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is compound.
     *        Compound types are all types that are not fundamental.
     *
     *        检查类型是否为复合类型的变量模板。
     *        复合类型是不是基本类型的所有类型。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_compound_v = !is_fundamental_v<Ty>;

    /**
     * @brief Type template for checking if a type is compound.
     *        检查类型是否为复合类型的类型模板。
     *
     * @tparam Ty Type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_compound : helper::bool_constant<is_compound_v<Ty>> {};
}

namespace rainy::utility {
    /**
     * @brief Helper function to get an overloaded member function pointer with proper type deduction.
     *        Ensures the function pointer is not null and helps with overload resolution.
     *
     *        获取重载成员函数指针的辅助函数，带有正确的类型推导。
     *        确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Class The class type containing the member function
     *               包含成员函数的类类型
     * @tparam Fx The type of the member function
     *            成员函数的类型
     * @param memfn The member function pointer
     *              成员函数指针
     * @return The same member function pointer (validated non-null)
     *         相同的成员函数指针（已验证非空）
     */
#if RAINY_HAS_CXX20
    template <typename Class, typename Fx>
        requires(type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                 !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>)
    constexpr rain_fn get_overloaded_func(Fx Class::*memfn) -> auto {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }

    /**
     * @brief Helper function to get an overloaded function pointer with proper type deduction.
     *        Ensures the function pointer is not null and helps with overload resolution.
     *
     *        获取重载函数指针的辅助函数，带有正确的类型推导。
     *        确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Fx The type of the function
     *            函数的类型
     * @param fn The function pointer
     *           函数指针
     * @return The same function pointer (validated non-null)
     *         相同的函数指针（已验证非空）
     */
    template <typename Fx>
        requires(!type_traits::primary_types::is_member_function_pointer_v<Fx *> &&
                 !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>)
    constexpr rain_fn get_overloaded_func(Fx fn) -> auto {
        assert(fn != nullptr);
        return fn;
    }
#else
    /**
     * @brief Helper function to get an overloaded member function pointer with proper type deduction (C++17 version).
     *        Ensures the function pointer is not null and helps with overload resolution.
     *
     *        获取重载成员函数指针的辅助函数，带有正确的类型推导（C++17版本）。
     *        确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Class The class type containing the member function
     *               包含成员函数的类类型
     * @tparam Fx The type of the member function
     *            成员函数的类型
     * @param memfn The member function pointer
     *              成员函数指针
     * @return The same member function pointer (validated non-null)
     *         相同的成员函数指针（已验证非空）
     */
    template <typename Class, typename Fx,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_member_function_pointer_v<Fx Class::*> &&
                                                        !type_traits::type_relations::is_same_v<Fx, std::nullptr_t>,
                                                    int> = 0>
    constexpr rain_fn get_overloaded_func(Fx Class::*memfn) -> auto {
        assert(memfn != nullptr && "memfn cannot be nullptr");
        return memfn;
    }
#endif
}

#if RAINY_HAS_CXX20

namespace rainy::type_traits::concepts {
    /**
     * @brief Concept that checks if two types are exactly the same.
     *        检查两个类型是否完全相同的概念。
     *
     * @tparam Ty1 The first type to compare
     *             要比较的第一个类型
     * @tparam Ty2 The second type to compare
     *             要比较的第二个类型
     */
    template <typename Ty1, typename Ty2>
    concept same_as = implements::is_same_v<Ty1, Ty2>;

    /**
     * @brief Concept that checks if a type is derived from another type.
     *        Requires both inheritance relationship and convertibility.
     *
     *        检查一个类型是否从另一个类型派生的概念。
     *        需要同时满足继承关系和可转换性。
     *
     * @tparam base The base class type
     *              基类类型
     * @tparam derived The derived class type to check
     *                 要检查的派生类类型
     */
    template <typename base, typename derived>
    concept derived_from =
        __is_base_of(base, derived) && type_relations::is_convertible_v<const volatile derived *, const volatile base *>;

    /**
     * @brief Concept that checks if a type is one of the given types.
     *        检查一个类型是否属于给定类型之一的概念。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     * @tparam Types The pack of possible types
     *               可能的类型包
     */
    template <typename Ty, typename... Types>
    concept in_types = (type_traits::type_relations::is_any_of_v<Ty, Types...>);
}

#if __has_include("format")
#include <format>

namespace rainy::type_traits::concepts {
    /**
     * @brief Concept that checks if a type is formattable with the given context and formatter.
     *        Requires both const and non-const formatter operations to be valid.
     *
     *        检查类型是否可以使用给定的上下文和格式化器进行格式化的概念。
     *        要求常量和非常量格式化器操作都有效。
     *
     * @tparam Ty The type to check for formattability
     *            要检查可格式化性的类型
     * @tparam Context The format context type (defaults to std::format_context)
     *                 格式化上下文类型（默认为 std::format_context）
     * @tparam Formatter The formatter type for Ty (deduced from Context)
     *                   Ty 的格式化器类型（从 Context 推导）
     */
    template <typename Ty, typename Context = std::format_context,
              typename Formatter = typename Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with = std::semiregular<Formatter> &&
                               requires(Formatter &formatter, const Formatter &const_formatter, Ty &&type, Context format_context,
                                        std::basic_format_parse_context<typename Context::char_type> parse_context) {
                                   { formatter.parse(parse_context) } -> same_as<typename decltype(parse_context)::iterator>;
                                   { const_formatter.format(type, format_context) } -> same_as<typename Context::iterator>;
                               };

    /**
     * @brief Concept that checks if a type is formattable with non-const formatter only.
     *        Similar to formattable_with but only requires non-const formatter operations.
     *
     *        检查类型是否仅使用非常量格式化器即可进行格式化的概念。
     *        类似于 formattable_with，但仅要求非常量格式化器操作有效。
     *
     * @tparam Ty The type to check for formattability
     *            要检查可格式化性的类型
     * @tparam Context The format context type (defaults to std::format_context)
     *                 格式化上下文类型（默认为 std::format_context）
     * @tparam Formatter The formatter type for Ty (deduced from Context)
     *                   Ty 的格式化器类型（从 Context 推导）
     */
    template <typename Ty, typename Context = std::format_context,
              typename Formatter = typename Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with_non_const =
        std::semiregular<Formatter> && requires(Formatter &formatter, Ty &&type, Context format_context,
                                                std::basic_format_parse_context<typename Context::char_type> parse_context) {
            { formatter.parse(format_context) } -> std::same_as<typename decltype(parse_context)::iterator>;
            { formatter.format(type, format_context) } -> std::same_as<typename Context::iterator>;
        };
}

#endif

#endif

namespace rainy::type_traits::extras::templates {
    /**
     * @brief Primary template for template traits.
     *        Provides information about whether a type is a template instantiation.
     *
     *        模板特性的主模板。
     *        提供关于类型是否为模板实例化的信息。
     *
     * @tparam Template The type to examine
     *                  要检查的类型
     */
    template <typename Template>
    struct template_traits : helper::false_type {};

    /**
     * @brief Specialization for actual template instantiations.
     *        Extracts the template template parameter and the type arguments.
     *
     *        实际模板实例化的特化。
     *        提取模板模板参数和类型参数。
     *
     * @tparam Template The template template parameter
     *                  模板模板参数
     * @tparam Types The template arguments
     *               模板参数
     */
    template <template <typename...> typename Template, typename... Types>
    struct template_traits<Template<Types...>> : helper::true_type {
        /**
         * @brief Type list containing all template arguments.
         *        包含所有模板参数的类型列表。
         */
        using types = other_trans::type_list<Types...>;
    };

    /**
     * @brief Variable template for checking if a type is a template instantiation.
     *        检查类型是否为模板实例化的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_template_v = template_traits<Ty>::value;

    /**
     * @brief Type template for checking if a type is a template instantiation.
     *        检查类型是否为模板实例化的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_template : helper::bool_constant<is_template_v<Ty>> {};
}

namespace rainy::type_traits::primary_types {
    using extras::templates::is_template;
    using extras::templates::template_traits;

    using extras::templates::is_template_v;
}

namespace rainy::core {
    enum class method_flags : std::uint8_t {
        none = 0,
        static_specified = 1, // static method
        memfn_specified = 2, // member method
        noexcept_specified = 4, // noexcept
        lvalue_qualified = 8, // left qualifier (e.g. &)
        rvalue_qualified = 16, // right qualifier (e.g. &&)
        const_qualified = 32, // const
        volatile_qualified = 64, // volatile
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(method_flags);

    /**
     * @brief Deduces the method flags for a given function type and arguments at compile time.
     *        Determines whether the function is static, member function, const/volatile qualified,
     *        ref-qualified, and whether the invocation is noexcept.
     *
     *        在编译时推导给定函数类型和参数的方法标志。
     *        确定函数是静态函数、成员函数、const/volatile限定、引用限定，
     *        以及调用是否为noexcept。
     *
     * @tparam Fx The function type to analyze
     *            要分析的函数类型
     * @tparam Args The argument types that would be used to invoke the function
     *              将用于调用函数的参数类型
     * @return method_flags A bitmask of method flags indicating the function's properties
     *                      method_flags 位掩码，指示函数的属性
     */
    template <typename Fx, typename... Args>
    constexpr rain_fn deduction_invoker_type() noexcept -> method_flags {
        auto flag{method_flags::none};
        using traits = type_traits::primary_types::function_traits<Fx>;
        if constexpr (!traits::valid) {
            return flag;
        }
        if constexpr (!type_traits::primary_types::is_member_function_pointer_v<Fx>) {
            constexpr bool noexcept_invoke = noexcept(utility::invoke(utility::declval<Fx>(), utility::declval<Args>()...));
            flag |=
                (noexcept_invoke ? method_flags::static_specified | method_flags::noexcept_specified : method_flags::static_specified);
        } else {
            flag |= method_flags::memfn_specified;
            using method_traits = type_traits::primary_types::member_pointer_traits<Fx>;
            using raw_class_type = typename method_traits::class_type;
            bool noexcept_invoke{false};
            if constexpr (traits::is_invoke_for_lvalue || traits::is_invoke_for_rvalue) {
                if constexpr (traits::is_invoke_for_lvalue) {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type &>(), utility::declval<Args>()...));
                } else {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<Fx>(), utility::declval<raw_class_type &&>(), utility::declval<Args>()...));
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
}

namespace rainy::utility::implements {
    template <typename Ty, typename... Args>
    struct ctor_impl {
        static constexpr Ty invoke(Args... args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) {
            return Ty(utility::forward<Args>(args)...);
        }
    };

    template <typename Ty>
    struct dtor_impl {
        static RAINY_CONSTEXPR20 void invoke(const Ty *object) noexcept(std::is_nothrow_destructible_v<Ty>) {
            if (object) {
                object->~Ty();
            }
        }
    };
}

namespace rainy::utility {
    /**
     * @brief Constructor invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking constructors.
     *
     *        带有 SFINAE 约束的构造函数调用器包装器。
     *        为调用构造函数提供统一接口。
     *
     * @tparam Ty The type to construct
     *            要构造的类型
     * @tparam Args The constructor argument types
     *              构造函数参数类型
     */
    template <typename Ty, typename... Args>
    struct ctor : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>,
                                                        implements::ctor_impl<Ty, Args...>> {};

    /**
     * @brief Destructor invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking destructors.
     *
     *        带有 SFINAE 约束的析构函数调用器包装器。
     *        为调用析构函数提供统一接口。
     *
     * @tparam Ty The type to destroy
     *            要销毁的类型
     */
    template <typename Ty>
    struct dtor
        : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, implements::dtor_impl<Ty>> {};
}

namespace rainy::utility {
    /**
     * @brief Gets a function pointer to the constructor invoker for type Ty.
     *        获取指向类型 Ty 的构造函数调用器的函数指针。
     *
     * @tparam Ty The type to construct
     *            要构造的类型
     * @tparam Args The constructor argument types
     *              构造函数参数类型
     * @return Function pointer to ctor<Ty, Args...>::invoke
     *         指向 ctor<Ty, Args...>::invoke 的函数指针
     */
    template <typename Ty, typename... Args,
              typename type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
    constexpr rain_fn get_ctor_fn() -> auto {
        return &ctor<Ty, Args...>::invoke;
    }

    /**
     * @brief Gets a function pointer to the destructor invoker for type Ty.
     *        获取指向类型 Ty 的析构函数调用器的函数指针。
     *
     * @tparam Ty The type to destroy
     *            要销毁的类型
     * @return Function pointer to dtor<Ty>::invoke
     *         指向 dtor<Ty>::invoke 的函数指针
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, int> = 0>
    constexpr rain_fn get_dtor_fn() -> auto {
        return &dtor<Ty>::invoke;
    }
}

namespace rainy::utility::implements {
    template <typename Ty, typename Assign>
    struct assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            Assign assign) noexcept(type_traits::type_properties::is_nothrow_assignable_v<Ty, Assign>) {
            return (this_ = assign);
        }
    };

    template <typename Ty>
    struct copy_assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            type_traits::reference_modify::add_rvalue_reference_t<Ty>
                &&rvalue) noexcept(type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>) {
            return (this_ = rvalue);
        }
    };

    template <typename Ty>
    struct move_assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            type_traits::reference_modify::add_rvalue_reference_t<Ty>
                rvalue) noexcept(type_traits::type_properties::is_nothrow_move_assignable_v<Ty>) {
            return (this_ = utility::move(rvalue));
        }
    };
}

namespace rainy::utility {
    /**
     * @brief Assignment operator invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking assignment operators.
     *
     *        带有 SFINAE 约束的赋值运算符调用器包装器。
     *        为调用赋值运算符提供统一接口。
     *
     * @tparam Ty The type being assigned to
     *            被赋值的类型
     * @tparam Assign The type being assigned from
     *                来源类型
     */
    template <typename Ty, typename Assign>
    struct assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                              !type_traits::type_properties::is_const_v<Ty> &&
                                                              !type_traits::composite_types::is_reference_v<Ty>,
                                                          implements::assign_impl<Ty, Assign>> {};

    /**
     * @brief Copy assignment operator invoker wrapper with SFINAE constraints.
     *        Provides a unified interface for invoking copy assignment operators.
     *
     *        带有 SFINAE 约束的拷贝赋值运算符调用器包装器。
     *        为调用拷贝赋值运算符提供统一接口。
     *
     * @tparam Ty The type being copy assigned
     *            被拷贝赋值的类型
     */
    template <typename Ty>
    struct copy_assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                               implements::copy_assign_impl<Ty>> {};

    /**
     * @brief Move assignment operator invoker wrapper with fallback to copy.
     *        Provides move assignment if available, otherwise falls back to copy assignment.
     *
     *        带有拷贝回退的移动赋值运算符调用器包装器。
     *        如果可用则提供移动赋值，否则回退到拷贝赋值。
     *
     * @tparam Ty The type being move assigned
     *            被移动赋值的类型
     */
    template <typename Ty>
    struct move_assign : type_traits::other_trans::conditional_t<
                             type_traits::type_properties::is_move_assignable_v<Ty>, implements::move_assign_impl<Ty>,
                             type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                                   implements::copy_assign_impl<Ty>>> {};
}

namespace rainy::utility {
    /**
     * @brief Gets a function pointer to the assignment operator invoker.
     *        获取指向赋值运算符调用器的函数指针。
     *
     * @tparam Ty The type being assigned to
     *            被赋值的类型
     * @tparam Assign The type being assigned from
     *                来源类型
     * @return Function pointer to assign<Ty, Assign>::invoke
     *         指向 assign<Ty, Assign>::invoke 的函数指针
     */
    template <typename Ty, typename Assign,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                        !type_traits::type_properties::is_const_v<Ty> &&
                                                        !type_traits::composite_types::is_reference_v<Ty>,
                                                    int> = 0>
    constexpr rain_fn get_assign() -> auto {
        return &assign<Ty, Assign>::invoke;
    }

    /**
     * @brief Gets a function pointer to the move assignment operator invoker.
     *        获取指向移动赋值运算符调用器的函数指针。
     *
     * @tparam Ty The type being move assigned
     *            被移动赋值的类型
     * @return Function pointer to move_assign<Ty>::invoke
     *         指向 move_assign<Ty>::invoke 的函数指针
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty> ||
                                                                     type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>,
                                                                 int> = 0>
    constexpr rain_fn get_move_assign() -> auto {
        return &move_assign<Ty>::invoke;
    }

    /**
     * @brief Gets a function pointer to the copy assignment operator invoker.
     *        获取指向拷贝赋值运算符调用器的函数指针。
     *
     * @tparam Ty The type being copy assigned
     *            被拷贝赋值的类型
     * @return Function pointer to copy_assign<Ty>::invoke
     *         指向 copy_assign<Ty>::invoke 的函数指针
     */
    template <typename Ty,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>, int> = 0>
    constexpr rain_fn get_copy_assign() -> auto {
        return &copy_assign<Ty>::invoke;
    }
}

namespace rainy::utility::cpp_methods {
    /**
     * @brief String representation of operator+
     *        operator+ 的字符串表示
     */
    static constexpr std::string_view method_operator_add = "operator+";

    /**
     * @brief String representation of operator-
     *        operator- 的字符串表示
     */
    static constexpr std::string_view method_operator_sub = "operator-";

    /**
     * @brief String representation of operator*
     *        operator* 的字符串表示
     */
    static constexpr std::string_view method_operator_mul = "operator*";

    /**
     * @brief String representation of operator/
     *        operator/ 的字符串表示
     */
    static constexpr std::string_view method_operator_div = "operator/";

    /**
     * @brief String representation of operator%
     *        operator% 的字符串表示
     */
    static constexpr std::string_view method_operator_mod = "operator%";

    /**
     * @brief String representation of operator==
     *        operator== 的字符串表示
     */
    static constexpr std::string_view method_operator_eq = "operator==";

    /**
     * @brief String representation of operator!=
     *        operator!= 的字符串表示
     */
    static constexpr std::string_view method_operator_neq = "operator!=";

    /**
     * @brief String representation of operator<
     *        operator< 的字符串表示
     */
    static constexpr std::string_view method_operator_lt = "operator<";

    /**
     * @brief String representation of operator>
     *        operator> 的字符串表示
     */
    static constexpr std::string_view method_operator_gt = "operator>";

    /**
     * @brief String representation of operator<=
     *        operator<= 的字符串表示
     */
    static constexpr std::string_view method_operator_le = "operator<=";

    /**
     * @brief String representation of operator>=
     *        operator>= 的字符串表示
     */
    static constexpr std::string_view method_operator_ge = "operator>=";

    /**
     * @brief String representation of operator=
     *        operator= 的字符串表示
     */
    static constexpr std::string_view method_operator_assign = "operator=";

    /**
     * @brief String representation of operator[]
     *        operator[] 的字符串表示
     */
    static constexpr std::string_view method_operator_index = "operator[]";

    /**
     * @brief String representation of operator()
     *        operator() 的字符串表示
     */
    static constexpr std::string_view method_operator_call = "operator()";

    /**
     * @brief String representation of operator->
     *        operator-> 的字符串表示
     */
    static constexpr std::string_view method_operator_arrow = "operator->";

    /**
     * @brief String representation of operator* (dereference)
     *        operator*（解引用）的字符串表示
     */
    static constexpr std::string_view method_operator_deref = "operator*";

    /**
     * @brief String representation of operator& (address-of)
     *        operator&（取地址）的字符串表示
     */
    static constexpr std::string_view method_operator_addr = "operator&";

    /**
     * @brief String representation of operator++ (prefix)
     *        operator++（前缀）的字符串表示
     */
    static constexpr std::string_view method_operator_preinc = "operator++";

    /**
     * @brief String representation of operator++ (postfix)
     *        operator++（后缀）的字符串表示
     */
    static constexpr std::string_view method_operator_postinc = "operator++(int)";

    /**
     * @brief String representation of operator-- (prefix)
     *        operator--（前缀）的字符串表示
     */
    static constexpr std::string_view method_operator_predec = "operator--";

    /**
     * @brief String representation of operator-- (postfix)
     *        operator--（后缀）的字符串表示
     */
    static constexpr std::string_view method_operator_postdec = "operator--(int)";

    /**
     * @brief String representation of operator||
     *        operator|| 的字符串表示
     */
    static constexpr std::string_view method_operator_or = "operator||";

    /**
     * @brief String representation of operator&&
     *        operator&& 的字符串表示
     */
    static constexpr std::string_view method_operator_and = "operator&&";

    /**
     * @brief String representation of operator!
     *        operator! 的字符串表示
     */
    static constexpr std::string_view method_operator_not = "operator!";

    /**
     * @brief String representation of operator|
     *        operator| 的字符串表示
     */
    static constexpr std::string_view method_operator_bit_or = "operator|";

    /**
     * @brief String representation of operator& (bitwise AND)
     *        operator&（按位与）的字符串表示
     */
    static constexpr std::string_view method_operator_bit_and = "operator&";

    /**
     * @brief String representation of operator^
     *        operator^ 的字符串表示
     */
    static constexpr std::string_view method_operator_bit_xor = "operator^";

    /**
     * @brief String representation of operator~
     *        operator~ 的字符串表示
     */
    static constexpr std::string_view method_operator_bit_not = "operator~";

    /**
     * @brief String representation of operator<<
     *        operator<< 的字符串表示
     */
    static constexpr std::string_view method_operator_shift_l = "operator<<";

    /**
     * @brief String representation of operator>>
     *        operator>> 的字符串表示
     */
    static constexpr std::string_view method_operator_shift_r = "operator>>";

    /**
     * @brief String representation of begin()
     *        begin() 的字符串表示
     */
    static constexpr std::string_view method_begin = "begin";

    /**
     * @brief String representation of end()
     *        end() 的字符串表示
     */
    static constexpr std::string_view method_end = "end";

    /**
     * @brief String representation of cbegin()
     *        cbegin() 的字符串表示
     */
    static constexpr std::string_view method_cbegin = "cbegin";

    /**
     * @brief String representation of cend()
     *        cend() 的字符串表示
     */
    static constexpr std::string_view method_cend = "cend";

    /**
     * @brief String representation of rbegin()
     *        rbegin() 的字符串表示
     */
    static constexpr std::string_view method_rbegin = "rbegin";

    /**
     * @brief String representation of rend()
     *        rend() 的字符串表示
     */
    static constexpr std::string_view method_rend = "rend";

    /**
     * @brief String representation of size()
     *        size() 的字符串表示
     */
    static constexpr std::string_view method_size = "size";

    /**
     * @brief String representation of empty()
     *        empty() 的字符串表示
     */
    static constexpr std::string_view method_empty = "empty";

    /**
     * @brief String representation of clear()
     *        clear() 的字符串表示
     */
    static constexpr std::string_view method_clear = "clear";

    /**
     * @brief String representation of push_back()
     *        push_back() 的字符串表示
     */
    static constexpr std::string_view method_push_back = "push_back";

    /**
     * @brief String representation of pop_back()
     *        pop_back() 的字符串表示
     */
    static constexpr std::string_view method_pop_back = "pop_back";

    /**
     * @brief String representation of length()
     *        length() 的字符串表示
     */
    static constexpr std::string_view method_length = "length";

    /**
     * @brief String representation of insert()
     *        insert() 的字符串表示
     */
    static constexpr std::string_view method_insert = "insert";

    /**
     * @brief String representation of erase()
     *        erase() 的字符串表示
     */
    static constexpr std::string_view method_erase = "erase";

    /**
     * @brief String representation of find()
     *        find() 的字符串表示
     */
    static constexpr std::string_view method_find = "find";

    /**
     * @brief String representation of resize()
     *        resize() 的字符串表示
     */
    static constexpr std::string_view method_resize = "resize";

    /**
     * @brief String representation of swap()
     *        swap() 的字符串表示
     */
    static constexpr std::string_view method_swap = "swap";

    /**
     * @brief String representation of at()
     *        at() 的字符串表示
     */
    static constexpr std::string_view method_at = "at";

    /**
     * @brief String representation of front()
     *        front() 的字符串表示
     */
    static constexpr std::string_view method_front = "front";

    /**
     * @brief String representation of back()
     *        back() 的字符串表示
     */
    static constexpr std::string_view method_back = "back";

    /**
     * @brief String representation of append()
     *        append() 的字符串表示
     */
    static constexpr std::string_view method_append = "append";
}

namespace rainy::core {
    /**
     * @brief Accumulates values in a range using operator+.
     *        使用 operator+ 累加范围内的值。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam Ty Initial value and result type
     *            初始值和结果类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param init Initial value for accumulation
     *             累加的初始值
     * @return The result of accumulating all elements with init
     *         使用 init 累加所有元素的结果
     */
    template <typename InputIt, typename Ty>
    RAINY_INLINE constexpr rain_fn accumulate(InputIt first, InputIt last, Ty init) -> Ty {
        for (; first != last; ++first) {
#if RAINY_HAS_CXX20
            init = utility::move(init) + *first;
#else
            init += *first;
#endif
        }
        return init;
    }

    /**
     * @brief Accumulates values in a range using a custom binary operation.
     *        使用自定义二元操作累加范围内的值。
     *
     * @tparam InputIt Input iterator type
     *                 输入迭代器类型
     * @tparam Ty Initial value and result type
     *            初始值和结果类型
     * @tparam BinaryOperation Binary operation type
     *                         二元操作类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @param init Initial value for accumulation
     *             累加的初始值
     * @param op Binary operation to apply
     *           要应用的二元操作
     * @return The result of applying op to all elements with init
     *         使用 init 对所有元素应用 op 的结果
     */
    template <typename InputIt, typename Ty, typename BinaryOperation>
    RAINY_INLINE constexpr rain_fn accumulate(InputIt first, InputIt last, Ty init, BinaryOperation op) -> Ty {
        for (; first != last; ++first) {
            init = utility::invoke(op, init, *first);
        }
        return init;
    }
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Variable template for checking if a type is an input iterator.
     *        Input iterators support dereference and pre-increment.
     *
     *        检查类型是否为输入迭代器的变量模板。
     *        输入迭代器支持解引用和前自增。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_input_iterator_v = meta_method::has_operator_deref_v<It> && meta_method::has_operator_preinc_v<It>;

    /**
     * @brief Variable template for checking if a type is an output iterator (primary template).
     *        Output iterators support dereference as lvalue.
     *
     *        检查类型是否为输出迭代器的变量模板（主模板）。
     *        输出迭代器支持解引用作为左值。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_output_iterator_v = false;

    /**
     * @brief Specialization that checks for lvalue reference from dereference.
     *        检查解引用是否产生左值引用的特化。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL
        is_output_iterator_v<It, other_trans::enable_if_t<meta_method::has_operator_deref_v<It> &&
                                                          primary_types::is_lvalue_reference_v<decltype(*utility::declval<It &>())>>> =
            false;

    /**
     * @brief Variable template for checking if a type is a forward iterator.
     *        Forward iterators are input iterators that are copyable, default constructible,
     *        and support multiple passes.
     *
     *        检查类型是否为前向迭代器的变量模板。
     *        前向迭代器是可拷贝、可默认构造的输入迭代器，支持多次遍历。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_forward_iterator_v =
        is_input_iterator_v<It> && type_properties::is_copy_constructible_v<It> && type_properties::is_copy_assignable_v<It> &&
        type_properties::is_default_constructible_v<It>;

    /**
     * @brief Variable template for checking if a type is a bidirectional iterator.
     *        Bidirectional iterators support decrement in addition to forward iterator operations.
     *
     *        检查类型是否为双向迭代器的变量模板。
     *        双向迭代器在前向迭代器操作的基础上支持自减。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_bidirectional_iterator_v = is_forward_iterator_v<It> && meta_method::has_operator_predec_v<It>;

    /**
     * @brief Variable template for checking if a type is a random access iterator.
     *        Random access iterators support addition, indexing, and comparison operations.
     *
     *        检查类型是否为随机访问迭代器的变量模板。
     *        随机访问迭代器支持加法、索引和比较操作。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_random_access_iterator_v = is_bidirectional_iterator_v<It> && meta_method::has_operator_addition_v<It> &&
                                                       meta_method::has_operator_index_v<It> && meta_method::has_operator_lt_v<It>;

    /**
     * @brief Variable template for checking if a type is a contiguous iterator (primary template).
     *        Contiguous iterators store elements in contiguous memory.
     *
     *        检查类型是否为连续迭代器的变量模板（主模板）。
     *        连续迭代器将元素存储在连续内存中。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v = false;

    /**
     * @brief Specialization that checks if the iterator can be converted to a pointer.
     *        检查迭代器是否可以转换为指针的特化。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v<
        It,
        type_traits::other_trans::enable_if_t<is_random_access_iterator_v<It> &&
                                              primary_types::is_pointer_v<decltype(utility::to_address(utility::declval<It>()))>>> =
        true;
}

namespace rainy::utility {
    /**
     * @brief Verifies that a pointer range is valid (first <= last).
     *        验证指针范围是否有效（first <= last）。
     *
     * @tparam Ty The element type
     *            元素类型
     * @param first Pointer to the beginning of the range
     *              指向范围起始的指针
     * @param last Pointer to the end of the range
     *             指向范围末尾的指针
     */
    template <typename Ty>
    constexpr rain_fn verify_range(const Ty *const first, const Ty *const last) noexcept -> void {
        expects(first <= last, "transposed pointer range");
    }

    /**
     * @brief Variable template for determining if unwrapping should be allowed.
     *        Defaults to true unless prevent_inheriting_unwrap is defined.
     *
     *        确定是否允许解包的变量模板。
     *        默认为true，除非定义了 prevent_inheriting_unwrap。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter, typename = void>
    constexpr bool allow_inheriting_unwrap_v = true;

    /**
     * @brief Specialization that checks prevent_inheriting_unwrap.
     *        检查 prevent_inheriting_unwrap 的特化。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    constexpr bool allow_inheriting_unwrap_v<Iter, type_traits::other_trans::void_t<typename Iter::prevent_inheriting_unwrap>> =
        type_traits::implements::is_same_v<Iter, typename Iter::prevent_inheriting_unwrap>;

    /**
     * @brief Variable template for checking if a range can be verified (primary template).
     *        检查范围是否可验证的变量模板（主模板）。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     * @tparam Sentinel The sentinel type
     *                  哨兵类型
     */
    template <typename Iter, typename Sentinel = Iter, typename = void>
    constexpr bool range_verifiable_v = false;

    /**
     * @brief Specialization that detects if verify_range is callable.
     *        检测 verify_range 是否可调用的特化。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     * @tparam Sentinel The sentinel type
     *                  哨兵类型
     */
    template <typename Iter, typename Sentinel>
    constexpr bool range_verifiable_v<
        Iter, Sentinel,
        type_traits::implements::void_t<decltype(verify_range(declval<const Iter &>(), declval<const Sentinel &>()))>> =
        allow_inheriting_unwrap_v<Iter>;

    /**
     * @brief ADL-enabled range verification function.
     *        Verifies that [first, last) forms a valid iterator range.
     *
     *        启用ADL的范围验证函数。
     *        验证 [first, last) 是否构成有效的迭代器范围。
     *
     * @tparam iter The iterator type
     *              迭代器类型
     * @tparam sentinel The sentinel type
     *                  哨兵类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Sentinel to the end of the range
     *             指向范围末尾的哨兵
     */
    template <typename iter, typename sentinel>
    constexpr rain_fn adl_verify_range(const iter &first, const sentinel &last) -> void {
        if constexpr (type_traits::implements::_is_pointer_v<iter> && type_traits::implements::_is_pointer_v<sentinel>) {
            expects(first <= last, "transposed pointer range");
        } else if constexpr (range_verifiable_v<iter, sentinel>) {
            verify_range(first, last);
        }
    }
}

namespace rainy::utility {
    /**
     * @brief Computes the distance between two iterators.
     *        计算两个迭代器之间的距离。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param last Iterator to the end of the range
     *             指向范围末尾的迭代器
     * @return The number of elements between first and last
     *         first 和 last 之间的元素数量
     */
    template <typename Iter>
    RAINY_NODISCARD constexpr rain_fn distance(Iter first, Iter last) -> std::ptrdiff_t {
        if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
            return last - first; // assume the iterator will do debug checking
        } else {
            adl_verify_range(first, last);
            std::ptrdiff_t off = 0;
            for (; first != last; ++first) {
                ++off;
            }
            return off;
        }
    }
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Variable template for checking if a type is movable.
     *        A type is movable if it is an object type, move constructible,
     *        assignable from rvalue, and swappable.
     *
     *        检查类型是否可移动的变量模板。
     *        类型如果是对象类型、可移动构造、可从右值赋值且可交换，则是可移动的。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_movable_v = composite_types::is_object_v<Ty> && type_properties::is_move_constructible_v<Ty> &&
                                        type_properties::is_assignable_v<Ty &, Ty> && type_properties::is_swappable_v<Ty>;

    /**
     * @brief Type template for checking if a type is movable.
     *        检查类型是否可移动的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_movable : helper::bool_constant<is_movable_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type is copyable.
     *        A type is copyable if it is copy constructible, movable,
     *        and assignable from lvalue, const lvalue, and const rvalue.
     *
     *        检查类型是否可拷贝的变量模板。
     *        类型如果是可拷贝构造、可移动、可从左值、常量左值和常量右值赋值，则是可拷贝的。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copyable_v =
        type_properties::is_copy_constructible_v<Ty> && is_movable_v<Ty> &&
        type_traits::type_properties::is_assignable_v<Ty &, Ty &> && type_traits::type_properties::is_assignable_v<Ty &, const Ty &> &&
        type_traits::type_properties::is_assignable_v<Ty &, const Ty>;

    /**
     * @brief Type template for checking if a type is copyable.
     *        检查类型是否可拷贝的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_copyable : helper::bool_constant<is_copyable_v<Ty>> {};
}

namespace rainy::utility {
    /**
     * @brief Maximum possible value for a signed difference type.
     *        有符号差类型的最大可能值。
     *
     * @tparam Diff The signed difference type
     *              有符号差类型
     */
    template <typename Diff>
    constexpr Diff max_possible_v{static_cast<type_traits::helper::make_unsigned_t<Diff>>(-1) >> 1};

    /**
     * @brief Minimum possible value for a signed difference type.
     *        有符号差类型的最小可能值。
     *
     * @tparam Diff The signed difference type
     *              有符号差类型
     */
    template <typename Diff>
    constexpr Diff min_possible_v{-max_possible_v<Diff> - 1};
}

namespace rainy::type_traits::type_properties {
    /**
     * @brief Type template for checking if a type is a sequential container.
     *        Sequential containers support push_back or are arrays.
     *
     *        检查类型是否为顺序容器的类型模板。
     *        顺序容器支持 push_back 或是数组。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    struct is_sequential_container
        : helper::bool_constant<type_traits::extras::meta_method::has_push_back_v<T> || primary_types::is_array_v<T>> {};

    /**
     * @brief Variable template for checking if a type is a sequential container.
     *        检查类型是否为顺序容器的变量模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    RAINY_CONSTEXPR_BOOL is_sequential_container_v = is_sequential_container<T>::value;

    /**
     * @brief Type template for checking if a type is an associative container.
     *        Associative containers support insert with key or key-value pairs,
     *        and do not support push_back.
     *
     *        检查类型是否为关联容器的类型模板。
     *        关联容器支持使用键或键值对的插入，不支持 push_back。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    struct is_associative_container : helper::bool_constant<type_traits::extras::meta_method::has_insert_for_key_v<T> &&
                                                            type_traits::extras::meta_method::has_insert_for_key_and_value_v<T> &&
                                                            !type_traits::extras::meta_method::has_push_back_v<T>> {};

    /**
     * @brief Variable template for checking if a type is an associative container.
     *        检查类型是否为关联容器的变量模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    RAINY_CONSTEXPR_BOOL is_associative_container_v = is_associative_container<T>::value;

    /**
     * @brief Type template for checking if a type is map-like (has key_type and mapped_type).
     *        检查类型是否为类似映射的类型（具有 key_type 和 mapped_type）的类型模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename, typename = void>
    struct is_map_like : helper::false_type {};

    /**
     * @brief Specialization that detects key_type and mapped_type members.
     *        检测 key_type 和 mapped_type 成员的特化。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    struct is_map_like<T, other_trans::void_t<typename T::key_type, typename T::mapped_type>> : helper::true_type {};

    /**
     * @brief Variable template for checking if a type is map-like.
     *        检查类型是否为类似映射的类型的变量模板。
     *
     * @tparam T The type to check
     *           要检查的类型
     */
    template <typename T>
    inline constexpr bool is_map_like_v = is_map_like<T>::value;
}

#endif
