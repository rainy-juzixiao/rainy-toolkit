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

// NOLINTBEGIN, clang-format off

#include <iterator>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/decay.hpp>
#include <rainy/core/type_traits/underlying_type.hpp>
#include <rainy/core/type_traits/helper.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/limits.hpp>
#include <rainy/core/type_traits/logical.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/type_traits/meta_types.hpp>
#include <rainy/core/type_traits/modifers.hpp>
#include <rainy/core/type_traits/primary_types.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/ranges_traits.hpp>
#include <rainy/core/type_traits/templates.hpp>
#include <rainy/core/type_traits/type_list.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/core/type_traits/value_list.hpp>
#include <utility>

// NOLINTEND, clang-format on

#if RAINY_HAS_CXX20
#include <concepts>
#include <format>
#endif

namespace rainy::utility {
    /**
     * \lang english
     * @brief Placeholder type for template metaprogramming and function argument tagging.
     *         Used to indicate a position to be filled or as a default argument.
     *
     * \lang simp-chinese
     * @brief 用于模板元编程和函数参数标记的占位符类型。
     *         用于指示需要填充的位置或作为默认参数。
     */
    struct placeholder_t final {
        explicit placeholder_t() = default;
    };

    /**
     * \lang english
     * @brief Global constexpr instance of placeholder_t.
     *
     * \lang simp-chinese
     * @brief placeholder_t 的全局 constexpr 实例。
     */
    constexpr placeholder_t placeholder{};

    /**
     * \lang english
     * @brief Type-parameterized placeholder template.
     *         Allows creating placeholders that carry type information.
     *
     * @tparam Ty Type to associate with this placeholder (defaults to void)
     *
     * \lang simp-chinese
     * @brief 类型参数化的占位符模板。
     *         允许创建携带类型信息的占位符。
     *
     * @tparam Ty 与此占位符关联的类型（默认为 void）
     */
    template <typename = void>
    struct placeholder_type_t final {
        explicit placeholder_type_t() = default;
    };

    /**
     * \lang english
     * @brief Global constexpr instance of placeholder_type_t<Ty>.
     *
     * @tparam Ty Type associated with the placeholder
     *
     * \lang simp-chinese
     * @brief placeholder_type_t<Ty> 的全局 constexpr 实例。
     *
     * @tparam Ty 与占位符关联的类型
     */
    template <typename Ty>
    constexpr placeholder_type_t<Ty> placeholder_type{};

    /**
     * \lang english
     * @brief In-place construction tag with index.
     *         Used to disambiguate constructors that take an index parameter.
     *
     * @tparam Idx Index value for tag specialization
     *
     * \lang simp-chinese
     * @brief 带索引的就地构造标签。
     *         用于区分接受索引参数的构造函数。
     *
     * @tparam Idx 标签特化的索引值
     */
    template <std::size_t>
    struct in_place_index_t final {
        explicit in_place_index_t() = default;
    };

    /**
     * \lang english
     * @brief Global constexpr instance of in_place_index_t<Idx>.
     *
     * @tparam Idx Index value
     *
     * \lang simp-chinese
     * @brief in_place_index_t<Idx> 的全局 constexpr 实例。
     *
     * @tparam Idx 索引值
     */
    template <std::size_t Idx>
    constexpr in_place_index_t<Idx> in_place_index{};

    /**
     * \lang english
     * @brief Variable template to check if a type is a specialization of in_place_index_t.
     *         Primary template defaults to false.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为 in_place_index_t 特化的变量模板。
     *         主模板默认为 false。
     *
     * @tparam Ty 要检查的类型
     */
    template <class>
    constexpr bool is_in_place_index_specialization = false;

    /**
     * \lang english
     * @brief Variable template to check if a type is a specialization of in_place_index_t.
     *         Specialization for in_place_index_t types.
     *
     * @tparam Idx Index value
     *
     * \lang simp-chinese
     * @brief 检查类型是否为 in_place_index_t 特化的变量模板。
     *         in_place_index_t 类型的特化。
     *
     * @tparam Idx 索引值
     */
    template <std::size_t Idx>
    constexpr bool is_in_place_index_specialization<utility::in_place_index_t<Idx>> = true;

    /**
     * \lang english
     * @brief Alias for std::allocator_arg_t.
     *         Tag type for allocator construction disambiguation.
     *
     * \lang simp-chinese
     * @brief std::allocator_arg_t 的别名。
     *         用于分配器构造歧义消除的标签类型。
     */
    using allocator_arg_t = std::allocator_arg_t;

    /**
     * \lang english
     * @brief Global constexpr instance of allocator_arg_t.
     *
     * \lang simp-chinese
     * @brief allocator_arg_t 的全局 constexpr 实例。
     */
    inline constexpr allocator_arg_t allocator_arg{};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief RAII wrapper that invokes a callable when it goes out of scope.
     *         The invocation can be canceled via to_invalidate.
     *
     * @tparam Callable The callable type to invoke on destruction
     *
     * \lang simp-chinese
     * @brief 在离开作用域时调用可调用对象的 RAII 包装器。
     *         可以通过 to_invalidate 取消调用。
     *
     * @tparam Callable 析构时要调用的可调用对象类型
     */
    template <typename Callable>
    class finally_impl : Callable { // NOLINT
    public:
        /**
         * \lang english
         * @brief Constructs a finally_impl from a callable.
         *
         * @param callable The callable to invoke on destruction
         *
         * \lang simp-chinese
         * @brief 从可调用对象构造 finally_impl。
         *
         * @param callable 析构时要调用的可调用对象
         */
        finally_impl(Callable &&callable) noexcept : Callable(utility::forward<Callable>(callable)), invalidate_(false) { // NOLINT
        }

        /**
         * \lang english
         * @brief Invokes the wrapped callable unless the wrapper has been invalidated.
         *
         * \lang simp-chinese
         * @brief 除非包装器已被取消，否则调用被包装的可调用对象。
         */
        ~finally_impl() {
            if (!is_invalidate()) {
                invoke_now();
            }
        }

        finally_impl(const finally_impl &) = delete;

        finally_impl &operator=(const finally_impl &) = delete;
        finally_impl &operator=(finally_impl &&) = delete;

        /**
         * \lang english
         * @brief Checks whether the invocation has been invalidated.
         *
         * @return true if the invocation is canceled, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查调用是否已被取消。
         *
         * @return 如果调用已被取消则为 true，否则为 false
         */
        RAINY_NODISCARD bool is_invalidate() const noexcept {
            return invalidate_;
        }

        /**
         * \lang english
         * @brief Invalidates the wrapper so the callable is not invoked on destruction.
         *
         * \lang simp-chinese
         * @brief 使包装器失效，从而在析构时不会调用可调用对象。
         */
        void to_invalidate() noexcept {
            invalidate_ = true;
        }

        /**
         * \lang english
         * @brief Restores the wrapper to a usable state so the callable is invoked on destruction.
         *
         * \lang simp-chinese
         * @brief 将包装器恢复为可用状态，使可调用对象在析构时被调用。
         */
        void to_useable() noexcept {
            invalidate_ = false;
        }

        /**
         * \lang english
         * @brief Invokes the wrapped callable immediately.
         *
         * \lang simp-chinese
         * @brief 立即调用被包装的可调用对象。
         */
        void invoke_now() const {
            (*this)();
        }

        /**
         * \lang english
         * @brief Sets the invalidation state based on the result of a predicate.
         *
         * @tparam Pred The predicate type
         * @param pred The predicate whose result determines the invalidation state
         *
         * \lang simp-chinese
         * @brief 根据谓词的结果设置失效状态。
         *
         * @tparam Pred 谓词类型
         * @param pred 其结果决定失效状态的谓词
         */
        template <typename Pred>
        void set_condition(Pred &&pred) {
            invalidate_ = static_cast<bool>(pred());
        }

    private:
        bool invalidate_;
    };

    /**
     * \lang english
     * @brief Creates a finally_impl that invokes the callable when it goes out of scope.
     *
     * @tparam Callable The callable type
     * @param callable The callable to invoke on destruction
     * @return A finally_impl wrapping the callable
     *
     * \lang simp-chinese
     * @brief 创建一个 finally_impl，在离开作用域时调用可调用对象。
     *
     * @tparam Callable 可调用对象类型
     * @param callable 析构时要调用的可调用对象
     * @return 包装了可调用对象的 finally_impl
     */
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
     * \lang english
     * @brief Primary template for member pointer traits.
     *         Provides information about member pointer types.
     *
     * @tparam Ty The type to examine
     *
     * \lang simp-chinese
     * @brief 成员指针特性的主模板。
     *         提供关于成员指针类型的信息。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename>
    struct member_pointer_traits {
        /**
         * \lang english
         * @brief Indicates whether the type is a valid member pointer.
         *
         * \lang simp-chinese
         * @brief 指示类型是否为有效的成员指针。
         */
        static RAINY_CONSTEXPR_BOOL valid = false;

        /**
         * \lang english
         * @brief The class type that the member pointer belongs to.
         *
         * \lang simp-chinese
         * @brief 成员指针所属的类类型。
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
     * \lang english
     * @brief Specialization of member_pointer_traits for actual member pointer types.
     *         Provides the member type and class type for a valid member pointer.
     *
     * @tparam Type The type of the member
     * @tparam Class The class type that the member belongs to
     *
     * \lang simp-chinese
     * @brief member_pointer_traits 对于实际成员指针类型的特化。
     *         为有效的成员指针提供成员类型和类类型。
     *
     * @tparam Type 成员的类型
     * @tparam Class 成员所属的类类型
     */
    template <typename Type, typename Class>
    struct member_pointer_traits<Type Class::*> {
        /**
         * \lang english
         * @brief Indicates that this is a valid member pointer.
         *
         * \lang simp-chinese
         * @brief 指示这是一个有效的成员指针。
         */
        using type = Type;

        /**
         * \lang english
         * @brief The class type that the member pointer belongs to.
         *
         * \lang simp-chinese
         * @brief 成员指针所属的类类型。
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
         * \lang english
         * @brief Indicates whether this is a member function pointer
         *
         * \lang simp-chinese
         * @brief 指示是否为成员函数指针
         */
        static RAINY_CONSTEXPR_BOOL is_member_function_pointer = IsMemberFunctionPointer;

        /**
         * \lang english
         * @brief Indicates whether this is a function pointer
         *
         * \lang simp-chinese
         * @brief 指示是否为函数指针
         */
        static RAINY_CONSTEXPR_BOOL is_function_pointer = IsFunctionPointer;

        /**
         * \lang english
         * @brief Indicates whether the function is noexcept
         *
         * \lang simp-chinese
         * @brief 指示函数是否为 noexcept
         */
        static RAINY_CONSTEXPR_BOOL is_noexcept = IsNoexcept;

        /**
         * \lang english
         * @brief Indicates whether the member function is volatile-qualified
         *
         * \lang simp-chinese
         * @brief 指示成员函数是否为 volatile 限定
         */
        static RAINY_CONSTEXPR_BOOL is_volatile = IsVolatile;

        /**
         * \lang english
         * @brief Indicates whether the member function is const-qualified
         *
         * \lang simp-chinese
         * @brief 指示成员函数是否为 const 限定
         */
        static RAINY_CONSTEXPR_BOOL is_const_member_function = IsConstMemberFunction;

        /**
         * \lang english
         * @brief Indicates whether this is a valid function type
         *
         * \lang simp-chinese
         * @brief 指示是否为有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = true;

        /**
         * \lang english
         * @brief Indicates whether this is a function object
         *
         * \lang simp-chinese
         * @brief 指示是否为函数对象
         */
        static RAINY_CONSTEXPR_BOOL is_function_object = false;
    };

    template <bool IsLvalue, bool IsRvalue>
    struct member_function_traits_base {
        /**
         * \lang english
         * @brief Indicates whether the function can be invoked on lvalue objects
         *
         * \lang simp-chinese
         * @brief 指示函数是否可以在左值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = IsLvalue;

        /**
         * \lang english
         * @brief Indicates whether the function can be invoked on rvalue objects
         *
         * \lang simp-chinese
         * @brief 指示函数是否可以在右值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = IsRvalue;
    };
}

namespace rainy::type_traits::implements {
    template <typename Fx, typename = void>
    struct fx_traits_has_invoke_operator : helper::false_type {};

    template <typename Fx>
    struct fx_traits_has_invoke_operator<
        Fx, type_traits::other_trans::void_t<decltype(&type_traits::modifers::remove_cv_t<Fx>::operator())>> : helper::true_type {};

    struct empty_function_traits {
        /**
         * \lang english
         * @brief Indicates that this is not a valid function type
         *
         * \lang simp-chinese
         * @brief 指示这不是一个有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = false;
    };

    template <typename Fx, bool Enable = fx_traits_has_invoke_operator<Fx>::value>
    struct try_to_get_operator {
        using type = void;
    };

    template <typename Fx>
    struct try_to_get_operator<Fx, true> {
        using type = decltype(&type_traits::modifers::remove_cv_t<Fx>::operator());
    };

    template <typename Traits, bool Enable = Traits::valid>
    struct fn_obj_traits {
        /**
         * \lang english
         * @brief List of argument types for the function object
         *
         * \lang simp-chinese
         * @brief 函数对象的参数类型列表
         */
        using argument_list = typename Traits::argument_list;

        /**
         * \lang english
         * @brief Return type of the function object
         *
         * \lang simp-chinese
         * @brief 函数对象的返回类型
         */
        using return_type = typename Traits::return_type;

        /**
         * \lang english
         * @brief Number of arguments the function object takes
         *
         * \lang simp-chinese
         * @brief 函数对象接受的参数数量
         */
        static inline constexpr std::size_t arity = Traits::arity;

        /**
         * \lang english
         * @brief Indicates whether the function object is noexcept
         *
         * \lang simp-chinese
         * @brief 指示函数对象是否为 noexcept
         */
        static RAINY_CONSTEXPR_BOOL is_noexcept = Traits::is_noexcept;

        /**
         * \lang english
         * @brief Indicates whether the function object is volatile-qualified
         *
         * \lang simp-chinese
         * @brief 指示函数对象是否为 volatile 限定
         */
        static RAINY_CONSTEXPR_BOOL is_volatile = Traits::is_volatile;

        /**
         * \lang english
         * @brief Indicates whether the function can be invoked on lvalue objects
         *
         * \lang simp-chinese
         * @brief 指示函数是否可以在左值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_lvalue = false;

        /**
         * \lang english
         * @brief Indicates whether the function can be invoked on rvalue objects
         *
         * \lang simp-chinese
         * @brief 指示函数是否可以在右值对象上调用
         */
        static RAINY_CONSTEXPR_BOOL is_invoke_for_rvalue = false;

        /**
         * \lang english
         * @brief Indicates that this is a function object
         *
         * \lang simp-chinese
         * @brief 指示这是一个函数对象
         */
        static RAINY_CONSTEXPR_BOOL is_function_object = true;

        /**
         * \lang english
         * @brief Indicates that this is a valid function type
         *
         * \lang simp-chinese
         * @brief 指示这是一个有效的函数类型
         */
        static RAINY_CONSTEXPR_BOOL valid = true;

        /**
         * \lang english
         * @brief Indicates whether the member function is const-qualified
         *
         * \lang simp-chinese
         * @brief 指示成员函数是否为 const 限定
         */
        static RAINY_CONSTEXPR_BOOL is_const_member_function = Traits::is_const_member_function;
    };

    template <typename Traits>
    struct fn_obj_traits<Traits, false> : empty_function_traits {};
}

namespace rainy::type_traits::primary_types {
    /**
     * \lang english
     * @brief Primary template for function traits.
     *         Provides comprehensive information about function types, function pointers, and function objects.
     *
     * @tparam Ty The type to examine (function type, function pointer, or function object)
     *
     * \lang simp-chinese
     * @brief 函数特性的主模板。
     *         提供关于函数类型、函数指针和函数对象的全面信息。
     *
     * @tparam Ty 要检查的类型（函数类型、函数指针或函数对象）
     */
    template <typename Ty>
    struct function_traits
        : implements::fn_obj_traits<
              function_traits<typename implements::try_to_get_operator<type_traits::modifers::remove_cvref_t<Ty>>::type>> {};

    /**
     * \lang english
     * @brief Specialization for void type.
     *         Provides empty traits for void.
     *
     * \lang simp-chinese
     * @brief void类型的特化。
     *         为void提供空特性。
     */
    template <>
    struct function_traits<void> : implements::empty_function_traits {};

    /**
     * \lang english
     * @brief Specialization for normal function types.
     *         Provides traits for regular function types.
     *
     * @tparam Rx Return type of the function
     * @tparam Args Parameter types of the function
     *
     * \lang simp-chinese
     * @brief 普通函数类型的特化。
     *         为常规函数类型提供特性。
     *
     * @tparam Rx 函数的返回类型
     * @tparam Args 函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args...)> : implements::function_traits_base<> {
        /**
         * \lang english
         * @brief Return type of the function
         *
         * \lang simp-chinese
         * @brief 函数的返回类型
         */
        using return_type = Rx;

        /**
         * \lang english
         * @brief List of parameter types
         *
         * \lang simp-chinese
         * @brief 参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief Number of parameters
         *
         * \lang simp-chinese
         * @brief 参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * \lang english
     * @brief Specialization for variadic function types.
     *         Provides traits for C-style variadic functions.
     *
     * @tparam Rx Return type of the function
     * @tparam Args Fixed parameter types of the function
     *
     * \lang simp-chinese
     * @brief 可变参数函数类型的特化。
     *         为C风格可变参数函数提供特性。
     *
     * @tparam Rx 函数的返回类型
     * @tparam Args 函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx(Args..., ...)> : implements::function_traits_base<> {
        /**
         * \lang english
         * @brief Return type of the function
         *
         * \lang simp-chinese
         * @brief 函数的返回类型
         */
        using return_type = Rx;

        /**
         * \lang english
         * @brief List of fixed parameter types (excluding variadic part)
         *
         * \lang simp-chinese
         * @brief 固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief Number of fixed parameters
         *
         * \lang simp-chinese
         * @brief 固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    // Macro expansions for various cv-qualifier and noexcept combinations
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(false, true, volatile)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, false, noexcept)
    RAINY_DECLARE_NORMAL_FUNCTION_TRAITS(true, true, volatile noexcept)

    /**
     * \lang english
     * @brief Specialization for function pointer types.
     *         Provides traits for regular function pointers.
     *
     * @tparam Rx Return type of the function
     * @tparam Args Parameter types of the function
     *
     * \lang simp-chinese
     * @brief 函数指针类型的特化。
     *         为常规函数指针提供特性。
     *
     * @tparam Rx 函数的返回类型
     * @tparam Args 函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...)> : implements::function_traits_base<false, true> {
        /**
         * \lang english
         * @brief Return type of the function
         *
         * \lang simp-chinese
         * @brief 函数的返回类型
         */
        using return_type = Rx;

        /**
         * \lang english
         * @brief List of parameter types
         *
         * \lang simp-chinese
         * @brief 参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief Number of parameters
         *
         * \lang simp-chinese
         * @brief 参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * \lang english
     * @brief Specialization for variadic function pointer types.
     *         Provides traits for C-style variadic function pointers.
     *
     * @tparam Rx Return type of the function
     * @tparam Args Fixed parameter types of the function
     *
     * \lang simp-chinese
     * @brief 可变参数函数指针类型的特化。
     *         为C风格可变参数函数指针提供特性。
     *
     * @tparam Rx 函数的返回类型
     * @tparam Args 函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...)> : implements::function_traits_base<false, true> {
        /**
         * \lang english
         * @brief Return type of the function
         *
         * \lang simp-chinese
         * @brief 函数的返回类型
         */
        using return_type = Rx;

        /**
         * \lang english
         * @brief List of fixed parameter types (excluding variadic part)
         *
         * \lang simp-chinese
         * @brief 固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief Number of fixed parameters
         *
         * \lang simp-chinese
         * @brief 固定参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * \lang english
     * @brief Specialization for noexcept function pointer types.
     *         Provides traits for noexcept function pointers.
     *
     * @tparam Rx Return type of the function
     * @tparam Args Parameter types of the function
     *
     * \lang simp-chinese
     * @brief noexcept函数指针类型的特化。
     *         为noexcept函数指针提供特性。
     *
     * @tparam Rx 函数的返回类型
     * @tparam Args 函数的参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args...) noexcept> : implements::function_traits_base<false, true, true> {
        /**
         * \lang english
         * @brief Return type of the function
         *
         * \lang simp-chinese
         * @brief 函数的返回类型
         */
        using return_type = Rx;

        /**
         * \lang english
         * @brief List of parameter types
         *
         * \lang simp-chinese
         * @brief 参数类型列表
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief Number of parameters
         *
         * \lang simp-chinese
         * @brief 参数数量
         */
        static inline constexpr std::size_t arity = sizeof...(Args);
    };

    /**
     * \lang english
     * @brief Specialization for variadic noexcept function pointer types.
     *         Provides traits for C-style variadic noexcept function pointers.
     *
     * @tparam Rx Return type of the function
     * @tparam Args Fixed parameter types of the function
     *
     * \lang simp-chinese
     * @brief 可变参数noexcept函数指针类型的特化。
     *         为C风格可变参数noexcept函数指针提供特性。
     *
     * @tparam Rx 函数的返回类型
     * @tparam Args 函数的固定参数类型
     */
    template <typename Rx, typename... Args>
    struct function_traits<Rx (*)(Args..., ...) noexcept> : implements::function_traits_base<false, true, true> {
        /**
         * \lang english
         * @brief Return type of the function
         *
         * \lang simp-chinese
         * @brief 函数的返回类型
         */
        using return_type = Rx;

        /**
         * \lang english
         * @brief List of fixed parameter types (excluding variadic part)
         *
         * \lang simp-chinese
         * @brief 固定参数类型列表（不包括可变参数部分）
         */
        using argument_list = type_traits::other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief Number of fixed parameters
         *
         * \lang simp-chinese
         * @brief 固定参数数量
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
     * \lang english
     * @brief Alias for function return type
     *
     * @tparam Fx Function type to query
     *
     * \lang simp-chinese
     * @brief 函数返回类型的别名
     *
     * @tparam Fx 要查询的函数类型
     */
    template <typename Fx>
    using function_return_type = typename function_traits<Fx>::return_type;

    /**
     * \lang english
     * @brief Number of arguments for a function type
     *
     * @tparam Fx Function type to query
     *
     * \lang simp-chinese
     * @brief 函数类型的参数数量
     *
     * @tparam Fx 要查询的函数类型
     */
    template <typename Fx>
    static inline constexpr std::size_t arity = function_traits<Fx>::arity;

    /**
     * \lang english
     * @brief List of argument types for a function type
     *
     * @tparam Fx Function type to query
     *
     * \lang simp-chinese
     * @brief 函数类型的参数类型列表
     *
     * @tparam Fx 要查询的函数类型
     */
    template <typename Fx>
    using function_argument_list = typename function_traits<Fx>::argument_list;

    /**
     * \lang english
     * @brief Variable template for checking if a function type is variadic
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查函数类型是否为可变参数的变量模板
     *
     * @tparam Ty 要检查的类型
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
     * \lang english
     * @brief Primary template for constructing a new function type with modified parameter lists.
     *         Allows inserting type lists at the front and end of the original argument list.
     *
     * @tparam NewRx The new return type for the function
     * @tparam Fx The original function type to base the new type on
     * @tparam TypeListFront Type list to insert at the beginning of the parameter list
     * @tparam TypeListEnd Type list to insert at the end of the parameter list
     * @tparam ArgList The original argument list (deduced from Fx)
     *
     * \lang simp-chinese
     * @brief 用于构造具有修改后参数列表的新函数类型的主模板。
     *         允许在原始参数列表的前面和后面插入类型列表。
     *
     * @tparam NewRx 函数的新返回类型
     * @tparam Fx 作为新类型基础的原始函数类型
     * @tparam TypeListFront 要插入到参数列表开头的类型列表
     * @tparam TypeListEnd 要插入到参数列表末尾的类型列表
     * @tparam ArgList 原始参数列表（从Fx推导）
     */
    template <typename NewRx, typename Fx, typename TypeListFront = other_trans::type_list<>,
              typename TypeListEnd = other_trans::type_list<>, typename ArgList = function_argument_list<Fx>>
    struct make_normalfx_type_with_pl {};

    /**
     * \lang english
     * @brief Specialization that performs the actual type construction.
     *         Combines front list, original arguments, and end list into a new function type.
     *
     * @tparam NewRx The new return type
     * @tparam Fx The original function type
     * @tparam TypeListFrontArgs Types to insert at the front
     * @tparam TypeListEndArgs Types to insert at the end
     * @tparam OriginalArgs The original parameter types
     *
     * \lang simp-chinese
     * @brief 执行实际类型构造的特化。
     *         将前置列表、原始参数和后置列表组合成新的函数类型。
     *
     * @tparam NewRx 新的返回类型
     * @tparam Fx 原始函数类型
     * @tparam TypeListFrontArgs 要插入到前面的类型
     * @tparam TypeListEndArgs 要插入到后面的类型
     * @tparam OriginalArgs 原始参数类型
     */
    template <typename NewRx, typename Fx, typename... TypeListFrontArgs, typename... TypeListEndArgs, typename... OriginalArgs>
    struct make_normalfx_type_with_pl<NewRx, Fx, other_trans::type_list<TypeListFrontArgs...>,
                                      other_trans::type_list<TypeListEndArgs...>,
                                      type_traits::other_trans::type_list<OriginalArgs...>> {
        /**
         * \lang english
         * @brief Helper template for constructing the new function type.
         *         Handles both regular functions and member functions.
         *
         * @tparam UFx The function type (may be same as Fx)
         * @tparam IsMemPtr Indicates whether this is a member function pointer
         *
         * \lang simp-chinese
         * @brief 用于构造新函数类型的辅助模板。
         *         处理普通函数和成员函数。
         *
         * @tparam UFx 函数类型（可能与Fx相同）
         * @tparam IsMemPtr 指示是否为成员函数指针
         */
        template <typename UFx, bool IsMemPtr = function_traits<UFx>::is_member_function_pointer>
        struct helper {
            using fn_traits = function_traits<UFx>;

            using prototype = NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...);

            /**
             * \lang english
             * @brief The resulting function type, with appropriate noexcept specification
             *
             * \lang simp-chinese
             * @brief 最终的函数类型，带有适当的noexcept说明符
             */
            using type =
                other_trans::conditional_t<fn_traits::is_noexcept,
                                           NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept, prototype>;
        };

        /**
         * \lang english
         * @brief Specialization for member function pointers.
         *         Member function pointers have different syntax requirements.
         *
         * @tparam UFx The member function pointer type
         *
         * \lang simp-chinese
         * @brief 成员函数指针的特化。
         *         成员函数指针有不同的语法要求。
         *
         * @tparam UFx 成员函数指针类型
         */
        template <typename UFx>
        struct helper<UFx, true> {
            using fn_traits = function_traits<UFx>;

            /**
             * \lang english
             * @brief The resulting member function pointer type
             *
             * \lang simp-chinese
             * @brief 最终的成员函数指针类型
             */
            using type = other_trans::conditional_t<fn_traits::is_noexcept,
                                                    NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...) noexcept,
                                                    NewRx(TypeListFrontArgs..., OriginalArgs..., TypeListEndArgs...)>;
        };

        /**
         * \lang english
         * @brief The constructed function type
         *
         * \lang simp-chinese
         * @brief 构造完成的函数类型
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
     * \lang english
     * @brief A wrapper that stores a reference to an object or function.
     *         Provides reference semantics in contexts where copies are normally made.
     *
     * @tparam Ty The type of the referenced object or function
     *
     * \lang simp-chinese
     * @brief 存储对象或函数引用的包装器。
     *         在通常进行拷贝的上下文中提供引用语义。
     *
     * @tparam Ty 被引用对象或函数的类型
     */
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(type_traits::implements::_is_object_v<Ty> || type_traits::implements::_is_function_v<Ty>,
                      "reference_wrapper<Ty> requires Ty to be an object type or a function type.");

        /**
         * \lang english
         * @brief The type of the referenced object or function
         *
         * \lang simp-chinese
         * @brief 被引用对象或函数的类型
         */
        using type = Ty;

        /**
         * \lang english
         * @brief Constructs a reference_wrapper from a compatible type.
         *
         * @tparam Uty The type of the value to wrap
         * @param val The value to wrap a reference to
         *
         * \lang simp-chinese
         * @brief 从兼容类型构造 reference_wrapper。
         *
         * @tparam Uty 要包装的值的类型
         * @param val 要包装引用的值
         */
        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<
                                                            type_traits::logical_traits::negation<type_traits::type_relations::is_same<
                                                                type_traits::modifers::remove_cvref_t<Uty>, reference_wrapper>>,
                                                            implements::refwrap_has_ctor_from<Ty, Uty>>,
                                                        int> = 0>
        constexpr reference_wrapper(Uty &&val) noexcept(noexcept(implements::refwrap_ctor_fun<Ty>(declval<Uty>()))) {
            Ty &ref = static_cast<Uty &&>(val);
            this->reference_data = utility::addressof(ref);
        }

        reference_wrapper(const reference_wrapper &) = delete;
        reference_wrapper(reference_wrapper &&) = delete;

        /**
         * \lang english
         * @brief Conversion operator to the referenced type.
         *
         * @return Reference to the wrapped object
         *
         * \lang simp-chinese
         * @brief 到被引用类型的转换运算符。
         *
         * @return 被包装对象的引用
         */
        constexpr operator Ty &() const noexcept {
            return *reference_data;
        }

        /**
         * \lang english
         * @brief Gets the wrapped reference.
         *
         * @return Reference to the wrapped object
         *
         * \lang simp-chinese
         * @brief 获取被包装的引用。
         *
         * @return 被包装对象的引用
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get() const noexcept {
            return *reference_data;
        }

        /**
         * \lang english
         * @brief Invokes the wrapped function with the given arguments.
         *
         * @tparam Args Argument types
         * @param args Arguments to forward to the function
         * @return The result of the function call
         *
         * \lang simp-chinese
         * @brief 使用给定参数调用被包装的函数。
         *
         * @tparam Args 参数类型
         * @param args 要转发给函数的参数
         * @return 函数调用的结果
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
         * \lang english
         * @brief Function call operator to invoke the wrapped function.
         *
         * @tparam Args Argument types
         * @param args Arguments to forward to the function
         * @return The result of the function call
         *
         * \lang simp-chinese
         * @brief 调用被包装函数的函数调用运算符。
         *
         * @tparam Args 参数类型
         * @param args 要转发给函数的参数
         * @return 函数调用的结果
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
     * \lang english
     * @brief Deduction guide for reference_wrapper.
     *
     * @tparam Ty The type of the referenced object
     *
     * \lang simp-chinese
     * @brief reference_wrapper 的推导指引。
     *
     * @tparam Ty 被引用对象的类型
     */
    template <typename Ty>
    reference_wrapper(Ty &) -> reference_wrapper<Ty>;

    /**
     * \lang english
     * @brief Deduction guide for reference_wrapper with cv-qualifier removal.
     *
     * @tparam Uty The type of the referenced object
     *
     * \lang simp-chinese
     * @brief 带有 cv-限定符移除的 reference_wrapper 推导指引。
     *
     * @tparam Uty 被引用对象的类型
     */
    template <typename Uty>
    reference_wrapper(Uty &) -> reference_wrapper<type_traits::modifers::remove_cvref_t<Uty>>;

    /**
     * \lang english
     * @brief Deleted overload for rvalue references.
     *
     * @tparam Ty The type of the rvalue
     *
     * \lang simp-chinese
     * @brief 右值引用的已删除重载。
     *
     * @tparam Ty 右值的类型
     */
    template <typename Ty>
    void ref(const Ty &&) = delete;

    /**
     * \lang english
     * @brief Deleted overload for rvalue references.
     *
     * @tparam Ty The type of the rvalue
     *
     * \lang simp-chinese
     * @brief 右值引用的已删除重载。
     *
     * @tparam Ty 右值的类型
     */
    template <typename Ty>
    void cref(const Ty &&) = delete;

    /**
     * \lang english
     * @brief Creates a reference_wrapper to the given object.
     *
     * @tparam Ty The type of the referenced object
     * @param val The object to wrap a reference to
     * @return A reference_wrapper to the object
     *
     * \lang simp-chinese
     * @brief 为给定对象创建 reference_wrapper。
     *
     * @tparam Ty 被引用对象的类型
     * @param val 要包装引用的对象
     * @return 对象的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(Ty &val) noexcept {
        return reference_wrapper<Ty>(val);
    }

    /**
     * \lang english
     * @brief Returns the given reference_wrapper unchanged.
     *
     * @tparam Ty The type of the referenced object
     * @param val The reference_wrapper to return
     * @return The same reference_wrapper
     *
     * \lang simp-chinese
     * @brief 原样返回给定的 reference_wrapper。
     *
     * @tparam Ty 被引用对象的类型
     * @param val 要返回的 reference_wrapper
     * @return 相同的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(reference_wrapper<Ty> val) noexcept {
        return val;
    }

    /**
     * \lang english
     * @brief Creates a const-qualified reference_wrapper to the given object.
     *
     * @tparam Ty The type of the referenced object
     * @param val The object to wrap a const reference to
     * @return A const-qualified reference_wrapper to the object
     *
     * \lang simp-chinese
     * @brief 为给定对象创建 const 限定的 reference_wrapper。
     *
     * @tparam Ty 被引用对象的类型
     * @param val 要包装 const 引用的对象
     * @return 对象的 const 限定 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(const Ty &val) noexcept {
        return reference_wrapper<const Ty>(val);
    }

    /**
     * \lang english
     * @brief Returns the given reference_wrapper as a const-qualified wrapper.
     *
     * @tparam Ty The type of the referenced object
     * @param val The reference_wrapper to convert
     * @return A const-qualified reference_wrapper
     *
     * \lang simp-chinese
     * @brief 将给定的 reference_wrapper 作为 const 限定包装器返回。
     *
     * @tparam Ty 被引用对象的类型
     * @param val 要转换的 reference_wrapper
     * @return const 限定的 reference_wrapper
     */
    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(reference_wrapper<Ty> val) noexcept {
        return val;
    }
}

namespace rainy::type_traits::modifers {
    /**
     * \lang english
     * @brief Primary template for unwrapping reference wrappers.
     *         If Ty is not a reference wrapper, returns Ty unchanged.
     *
     * @tparam Ty The type to potentially unwrap
     *
     * \lang simp-chinese
     * @brief 解包引用包装器的主模板。
     *         如果 Ty 不是引用包装器，则原样返回 Ty。
     *
     * @tparam Ty 可能需要解包的类型
     */
    template <typename Ty>
    struct unwrap_reference {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for rainy::utility::reference_wrapper.
     *         Extracts the underlying type from a reference_wrapper.
     *
     * @tparam Ty The type wrapped by reference_wrapper
     *
     * \lang simp-chinese
     * @brief rainy::utility::reference_wrapper 的特化。
     *         从 reference_wrapper 中提取底层类型。
     *
     * @tparam Ty reference_wrapper 包装的类型
     */
    template <typename Ty>
    struct unwrap_reference<utility::reference_wrapper<Ty>> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for std::reference_wrapper.
     *         Extracts the underlying type from a std::reference_wrapper.
     *
     * @tparam Ty The type wrapped by std::reference_wrapper
     *
     * \lang simp-chinese
     * @brief std::reference_wrapper 的特化。
     *         从 std::reference_wrapper 中提取底层类型。
     *
     * @tparam Ty std::reference_wrapper 包装的类型
     */
    template <typename Ty>
    struct unwrap_reference<std::reference_wrapper<Ty>> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Alias template for unwrapping reference wrappers.
     *
     * @tparam Ty The type to potentially unwrap
     *
     * \lang simp-chinese
     * @brief 解包引用包装器的别名模板。
     *
     * @tparam Ty 可能需要解包的类型
     */
    template <typename Ty>
    using unwrap_reference_t = typename unwrap_reference<Ty>::type;

    /**
     * \lang english
     * @brief Alias template that decays and then unwraps a type.
     *
     * @tparam Ty The type to decay and unwrap
     *
     * \lang simp-chinese
     * @brief 先退化然后解包类型的别名模板。
     *
     * @tparam Ty 要退化并解包的类型
     */
    template <typename Ty>
    using unwrap_ref_decay_t = unwrap_reference_t<other_trans::decay_t<Ty>>;

    /**
     * \lang english
     * @brief Type template for decaying and unwrapping a type.
     *
     * @tparam Ty The type to decay and unwrap
     *
     * \lang simp-chinese
     * @brief 退化并解包类型的类型模板。
     *
     * @tparam Ty 要退化并解包的类型
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


    template <typename Callable, typename Ty1, typename RemoveCvref = type_traits::modifers::remove_cvref_t<Callable>,
              bool _Is_pmf = type_traits::primary_types::is_member_function_pointer_v<RemoveCvref>,
              bool _Is_pmd = type_traits::primary_types::is_member_object_pointer_v<RemoveCvref>>
    struct invoker_impl;

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, true, false>
        : type_traits::other_trans::conditional_t<
              type_traits::type_relations::is_same_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                             type_traits::modifers::remove_cvref_t<Ty1>> ||
                  std::is_base_of_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                                    type_traits::modifers::remove_cvref_t<Ty1>>,
              invoker_pmf_object,
              type_traits::other_trans::conditional_t<rainy::type_traits::primary_types::is_specialization_v<type_traits::modifers::remove_cvref_t<Ty1>,
                                                                                        std::reference_wrapper>,
                                 invoker_pmf_refwrap, invoker_pmf_pointer>> {};

    template <typename Callable, typename Ty1, typename RemoveCvref>
    struct invoker_impl<Callable, Ty1, RemoveCvref, false, true>
        : type_traits::other_trans::conditional_t<
              type_traits::type_relations::is_same_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                             type_traits::modifers::remove_cvref_t<Ty1>> ||
                  std::is_base_of_v<typename rainy::type_traits::primary_types::member_pointer_traits<RemoveCvref>::class_type,
                                    type_traits::modifers::remove_cvref_t<Ty1>>,
              invoker_pmd_object,
              type_traits::other_trans::conditional_t<rainy::type_traits::primary_types::is_specialization_v<type_traits::modifers::remove_cvref_t<Ty1>,
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
     * \lang english
     * @brief Primary template for function object invoker.
     *         Provides a unified interface for invoking various callable types.
     *
     * @tparam Callable The type of the callable object
     * @tparam Ty1 The type of the first argument (or void if not applicable)
     *
     * \lang simp-chinese
     * @brief 函数对象调用器的主模板。
     *         为调用各种可调用类型提供统一接口。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Ty1 第一个参数的类型（如果不适用则为void）
     */
    template <typename Callable, typename Ty1 = void>
    struct invoker : implements::invoker_impl<Callable, Ty1> {};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Invokes a callable object with no arguments.
     *
     * @tparam Callable The type of the callable object
     * @param obj The callable object to invoke
     * @return The result of invoking the callable object
     *
     * \lang simp-chinese
     * @brief 调用无参数的可调用对象。
     *
     * @tparam Callable 可调用对象的类型
     * @param obj 要调用的可调用对象
     * @return 调用可调用对象的结果
     */
    template <typename Callable>
    constexpr RAINY_INLINE auto invoke(Callable &&obj) noexcept(noexcept(static_cast<Callable &&>(obj)()))
        -> decltype(static_cast<Callable &&>(obj)()) {
        return static_cast<Callable &&>(obj)();
    }

    /**
     * \lang english
     * @brief Invokes a callable object with one or more arguments.
     *
     * @tparam Callable The type of the callable object
     * @tparam Ty1 The type of the first argument
     * @tparam Args The types of the remaining arguments
     * @param obj The callable object to invoke
     * @param arg1 The first argument
     * @param args The remaining arguments
     * @return The result of invoking the callable object with the given arguments
     *
     * \lang simp-chinese
     * @brief 调用带有一个或多个参数的可调用对象。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Ty1 第一个参数的类型
     * @tparam Args 剩余参数的类型
     * @param obj 要调用的可调用对象
     * @param arg1 第一个参数
     * @param args 剩余参数
     * @return 使用给定参数调用可调用对象的结果
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
     * \lang english
     * @brief Invokes a callable object with the arguments from a tuple.
     *
     * @tparam Callable The type of the callable object
     * @tparam Tuple The type of the tuple containing the arguments
     * @param obj The callable object to invoke
     * @param tuple The tuple containing the arguments to pass to the callable
     * @return The result of invoking the callable object with the unpacked tuple arguments
     *
     * \lang simp-chinese
     * @brief 使用元组中的参数调用可调用对象。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Tuple 包含参数的元组类型
     * @param obj 要调用的可调用对象
     * @param tuple 包含要传递给可调用对象的参数的元组
     * @return 使用解包后的元组参数调用可调用对象的结果
     */
    template <typename Callable, typename Tuple>
    constexpr decltype(auto) apply(Callable &&obj, Tuple &&tuple) noexcept(noexcept(implements::apply_impl(
        utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
        type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>>{}))) {
        return implements::apply_impl(
            utility::forward<Callable>(obj), utility::forward<Tuple>(tuple),
            type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>>{});
    }
}

namespace rainy::type_traits::properties::implements {
    template <typename Rx, bool NoThrow>
    struct invoke_traits_common {
        using type = Rx;
        using is_invocable = helper::true_type;
        using is_nothrow_invocable = helper::bool_constant<NoThrow>;

        template <typename Rx_>
        using is_invocable_r = helper::bool_constant<
            logical_traits::disjunction_v<primary_types::is_void<Rx>, properties::is_invoke_convertible<type, Rx_>>>;

        template <typename Rx_>
        using is_nothrow_invocable_r = helper::bool_constant<logical_traits::conjunction_v<
            is_nothrow_invocable,
            logical_traits::disjunction<primary_types::is_void<Rx>,
                                        logical_traits::conjunction<properties::is_invoke_convertible<type, Rx_>,
                                                                    properties::is_invoke_nothrow_convertible<type, Rx_>>>>>;
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

namespace rainy::type_traits::properties {
    /**
     * \lang english
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *         and the result is convertible to the specified return type.
     *
     * @tparam Rx The required return type
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用，并且结果可以转换为指定的返回类型的变量模板。
     *
     * @tparam Rx 要求的返回类型
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_r_v = implements::is_invocable_r_helper<Rx, Callable, Args...>::value;

    /**
     * \lang english
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *         and the result is convertible to the specified return type.
     *
     * @tparam Rx The required return type
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用，并且结果可以转换为指定的返回类型的类型模板。
     *
     * @tparam Rx 要求的返回类型
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_r : helper::bool_constant<is_invocable_r_v<Rx, Callable, Args...>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a callable object can be invoked with the given arguments.
     *
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用的变量模板。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_invocable_v = implements::is_invocable_helper<Callable, Args...>::value;

    /**
     * \lang english
     * @brief Type template for checking if a callable object can be invoked with the given arguments.
     *
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用的类型模板。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Callable, typename... Args>
    struct is_invocable : helper::bool_constant<is_invocable_v<Callable, Args...>> {};

    /**
     * \lang english
     * @brief Type template that yields the result type of invoking a callable object with the given arguments.
     *
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 产生使用给定参数调用可调用对象的结果类型的类型模板。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Callable, typename... Args>
    struct invoke_result {
        using type = typename implements::select_invoke_traits<Callable, Args...>::type;
    };

    /**
     * \lang english
     * @brief Alias template for invoke_result, providing direct access to the result type.
     *
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief invoke_result 的别名模板，提供对结果类型的直接访问。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Callable, typename... Args>
    using invoke_result_t = typename invoke_result<Callable, Args...>::type;

    /**
     * \lang english
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *         without throwing exceptions.
     *
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用且不抛出异常的变量模板。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_v = implements::is_nothrow_invocable_helper<Callable, Args...>::value;

    /**
     * \lang english
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *         without throwing exceptions.
     *
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用且不抛出异常的类型模板。
     *
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Callable, typename... Args>
    struct is_nothrow_invocable : helper::bool_constant<is_nothrow_invocable_v<Callable, Args...>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a callable object can be invoked with the given arguments
     *         without throwing exceptions, and the result is convertible to the specified return type.
     *
     * @tparam Rx The required return type
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用且不抛出异常，并且结果可以转换为指定的返回类型的变量模板。
     *
     * @tparam Rx 要求的返回类型
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_r_v = implements::is_nothrow_invocable_r_helper<Rx, Callable, Args...>::value;

    /**
     * \lang english
     * @brief Type template for checking if a callable object can be invoked with the given arguments
     *         without throwing exceptions, and the result is convertible to the specified return type.
     *
     * @tparam Rx The required return type
     * @tparam Callable The type of the callable object
     * @tparam Args The types of the arguments
     *
     * \lang simp-chinese
     * @brief 检查可调用对象是否可以使用给定参数调用且不抛出异常，并且结果可以转换为指定的返回类型的类型模板。
     *
     * @tparam Rx 要求的返回类型
     * @tparam Callable 可调用对象的类型
     * @tparam Args 参数的类型
     */
    template <typename Rx, typename Callable, typename... Args>
    struct is_nothrow_invocable_r : helper::bool_constant<is_nothrow_invocable_r_v<Rx, Callable, Args...>> {};
}

namespace rainy::type_traits::composite_types {
    /**
     * \lang english
     * @brief Variable template for checking if a type is arithmetic.
     *         Arithmetic types include integral and floating-point types.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为算术类型的变量模板。
     *         算术类型包括整型和浮点型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_arithmetic_v = implements::is_arithmetic_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is arithmetic.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为算术类型的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_arithmetic : helper::bool_constant<is_arithmetic_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is fundamental.
     *         Fundamental types include arithmetic types, void, and nullptr_t.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为基本类型的变量模板。
     *         基本类型包括算术类型、void 和 nullptr_t。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_fundamental_v =
        is_arithmetic_v<Ty> || primary_types::is_void_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is fundamental.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为基本类型的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_fundamental : helper::bool_constant<is_fundamental_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is an object type.
     *         Object types are cv-qualifiable and not void.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为对象类型的变量模板。
     *         对象类型可以具有 cv 限定符，且不是 void。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_object_v = properties::is_const_v<const Ty> && !primary_types::is_void_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is an object type.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为对象类型的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_object : helper::bool_constant<is_object_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a reference.
     *         References include both lvalue and rvalue references.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为引用类型的变量模板。
     *         引用类型包括左值引用和右值引用。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_reference_v = implements::_is_reference_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is a reference.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为引用类型的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_reference : helper::bool_constant<implements::_is_reference_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a pointer to member.
     *         Member pointers can point to either data members or member functions.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为成员指针的变量模板。
     *         成员指针可以指向数据成员或成员函数。
     *
     * @tparam Ty 要检查的类型
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
     * \lang english
     * @brief Type template for checking if a type is a pointer to member.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为成员指针的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_member_pointer : helper::bool_constant<is_member_pointer_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is scalar.
     *         Scalar types include arithmetic, enum, pointer, member pointer, and nullptr_t.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为标量类型的变量模板。
     *         标量类型包括算术、枚举、指针、成员指针和 nullptr_t。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_scalar_v = is_arithmetic_v<Ty> || primary_types::is_enum_v<Ty> || primary_types::is_pointer_v<Ty> ||
                                       is_member_pointer_v<Ty> || primary_types::is_null_pointer_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is scalar.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为标量类型的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_scalar : helper::bool_constant<is_scalar_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is compound.
     *         Compound types are all types that are not fundamental.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为复合类型的变量模板。
     *         复合类型是不是基本类型的所有类型。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_compound_v = !is_fundamental_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is compound.
     *
     * @tparam Ty Type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为复合类型的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_compound : helper::bool_constant<is_compound_v<Ty>> {};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Helper function to get an overloaded member function pointer with proper type deduction.
     *         Ensures the function pointer is not null and helps with overload resolution.
     *
     * @tparam Class The class type containing the member function
     * @tparam Fx The type of the member function
     * @param memfn The member function pointer
     * @return The same member function pointer (validated non-null)
     *
     * \lang simp-chinese
     * @brief 获取重载成员函数指针的辅助函数，带有正确的类型推导。
     *         确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Class 包含成员函数的类类型
     * @tparam Fx 成员函数的类型
     * @param memfn 成员函数指针
     * @return 相同的成员函数指针（已验证非空）
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
     * \lang english
     * @brief Helper function to get an overloaded function pointer with proper type deduction.
     *         Ensures the function pointer is not null and helps with overload resolution.
     *
     * @tparam Fx The type of the function
     * @param fn The function pointer
     * @return The same function pointer (validated non-null)
     *
     * \lang simp-chinese
     * @brief 获取重载函数指针的辅助函数，带有正确的类型推导。
     *         确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Fx 函数的类型
     * @param fn 函数指针
     * @return 相同的函数指针（已验证非空）
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
     * \lang english
     * @brief Helper function to get an overloaded member function pointer with proper type deduction (C++17 version).
     *         Ensures the function pointer is not null and helps with overload resolution.
     *
     * @tparam Class The class type containing the member function
     * @tparam Fx The type of the member function
     * @param memfn The member function pointer
     * @return The same member function pointer (validated non-null)
     *
     * \lang simp-chinese
     * @brief 获取重载成员函数指针的辅助函数，带有正确的类型推导（C++17版本）。
     *         确保函数指针不为空，并帮助进行重载解析。
     *
     * @tparam Class 包含成员函数的类类型
     * @tparam Fx 成员函数的类型
     * @param memfn 成员函数指针
     * @return 相同的成员函数指针（已验证非空）
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
     * \lang english
     * @brief Concept that checks if two types are exactly the same.
     *
     * @tparam Ty1 The first type to compare
     * @tparam Ty2 The second type to compare
     *
     * \lang simp-chinese
     * @brief 检查两个类型是否完全相同的概念。
     *
     * @tparam Ty1 要比较的第一个类型
     * @tparam Ty2 要比较的第二个类型
     */
    template <typename Ty1, typename Ty2>
    concept same_as = type_relations::is_same_v<Ty1, Ty2>;

    /**
     * \lang english
     * @brief Concept that checks if a type is derived from another type.
     *         Requires both inheritance relationship and convertibility.
     *
     * @tparam base The base class type
     * @tparam derived The derived class type to check
     *
     * \lang simp-chinese
     * @brief 检查一个类型是否从另一个类型派生的概念。
     *         需要同时满足继承关系和可转换性。
     *
     * @tparam base 基类类型
     * @tparam derived 要检查的派生类类型
     */
    template <typename base, typename derived>
    concept derived_from =
        __is_base_of(base, derived) && type_relations::is_convertible_v<const volatile derived *, const volatile base *>;

    /**
     * \lang english
     * @brief Concept that checks if a type is one of the given types.
     *
     * @tparam Ty The type to check
     * @tparam Types The pack of possible types
     *
     * \lang simp-chinese
     * @brief 检查一个类型是否属于给定类型之一的概念。
     *
     * @tparam Ty 要检查的类型
     * @tparam Types 可能的类型包
     */
    template <typename Ty, typename... Types>
    concept in_types = (type_traits::type_relations::is_any_of_v<Ty, Types...>);
}

#if __has_include("format")
#include <format>

namespace rainy::type_traits::concepts {
    /**
     * \lang english
     * @brief Concept that checks if a type is formattable with the given context and formatter.
     *         Requires both const and non-const formatter operations to be valid.
     *
     * @tparam Ty The type to check for formattability
     * @tparam Context The format context type (defaults to std::format_context)
     * @tparam Formatter The formatter type for Ty (deduced from Context)
     *
     * \lang simp-chinese
     * @brief 检查类型是否可以使用给定的上下文和格式化器进行格式化的概念。
     *         要求常量和非常量格式化器操作都有效。
     *
     * @tparam Ty 要检查可格式化性的类型
     * @tparam Context 格式化上下文类型（默认为 std::format_context）
     * @tparam Formatter Ty 的格式化器类型（从 Context 推导）
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
     * \lang english
     * @brief Concept that checks if a type is formattable with non-const formatter only.
     *         Similar to formattable_with but only requires non-const formatter operations.
     *
     * @tparam Ty The type to check for formattability
     * @tparam Context The format context type (defaults to std::format_context)
     * @tparam Formatter The formatter type for Ty (deduced from Context)
     *
     * \lang simp-chinese
     * @brief 检查类型是否仅使用非常量格式化器即可进行格式化的概念。
     *         类似于 formattable_with，但仅要求非常量格式化器操作有效。
     *
     * @tparam Ty 要检查可格式化性的类型
     * @tparam Context 格式化上下文类型（默认为 std::format_context）
     * @tparam Formatter Ty 的格式化器类型（从 Context 推导）
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
     * \lang english
     * @brief Primary template for template traits.
     *         Provides information about whether a type is a template instantiation.
     *
     * @tparam Template The type to examine
     *
     * \lang simp-chinese
     * @brief 模板特性的主模板。
     *         提供关于类型是否为模板实例化的信息。
     *
     * @tparam Template 要检查的类型
     */
    template <typename Template>
    struct template_traits : helper::false_type {};

    /**
     * \lang english
     * @brief Specialization for actual template instantiations.
     *         Extracts the template template parameter and the type arguments.
     *
     * @tparam Template The template template parameter
     * @tparam Types The template arguments
     *
     * \lang simp-chinese
     * @brief 实际模板实例化的特化。
     *         提取模板模板参数和类型参数。
     *
     * @tparam Template 模板模板参数
     * @tparam Types 模板参数
     */
    template <template <typename...> typename Template, typename... Types>
    struct template_traits<Template<Types...>> : helper::true_type {
        /**
         * \lang english
         * @brief Type list containing all template arguments.
         *
         * \lang simp-chinese
         * @brief 包含所有模板参数的类型列表。
         */
        using types = other_trans::type_list<Types...>;
    };

    /**
     * \lang english
     * @brief Variable template for checking if a type is a template instantiation.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为模板实例化的变量模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_template_v = template_traits<Ty>::value;

    /**
     * \lang english
     * @brief Type template for checking if a type is a template instantiation.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为模板实例化的类型模板。
     *
     * @tparam Ty 要检查的类型
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
     * \lang english
     * @brief Deduces the method flags for a given function type and arguments at compile time.
     *         Determines whether the function is static, member function, const/volatile qualified,
     *         ref-qualified, and whether the invocation is noexcept.
     *
     * @tparam Fx The function type to analyze
     * @tparam Args The argument types that would be used to invoke the function
     * @return method_flags A bitmask of method flags indicating the function's properties
     *
     * \lang simp-chinese
     * @brief 在编译时推导给定函数类型和参数的方法标志。
     *         确定函数是静态函数、成员函数、const/volatile限定、引用限定，
     *         以及调用是否为noexcept。
     *
     * @tparam Fx 要分析的函数类型
     * @tparam Args 将用于调用函数的参数类型
     * @return method_flags 位掩码，指示函数的属性
     */
    template <typename Fx, typename... Args>
    constexpr rain_fn deduction_invoker_type() noexcept -> method_flags {
        auto flag{method_flags::none};
        using fx = type_traits::modifers::remove_cvref_t<Fx>;
        using traits = type_traits::primary_types::function_traits<fx>;
        if constexpr (!traits::valid) {
            return flag;
        }
        if constexpr (!type_traits::primary_types::is_member_function_pointer_v<fx>) {
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
                        utility::invoke(utility::declval<fx>(), utility::declval<raw_class_type &>(), utility::declval<Args>()...));
                } else {
                    noexcept_invoke = noexcept(
                        utility::invoke(utility::declval<fx>(), utility::declval<raw_class_type &&>(), utility::declval<Args>()...));
                }
            } else {
                noexcept_invoke = noexcept(
                    utility::invoke(utility::declval<fx>(), utility::declval<raw_class_type *>(), utility::declval<Args>()...));
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
        static constexpr Ty invoke(Args... args) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) {
            return Ty(utility::forward<Args>(args)...);
        }
    };

    template <typename Ty>
    struct dtor_impl {
        static RAINY_CONSTEXPR20 void invoke(const Ty *object) noexcept(type_traits::properties::is_nothrow_destructible_v<Ty>) {
            if (object) {
                object->~Ty();
            }
        }
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Constructor invoker wrapper with SFINAE constraints.
     *         Provides a unified interface for invoking constructors.
     *
     * @tparam Ty The type to construct
     * @tparam Args The constructor argument types
     *
     * \lang simp-chinese
     * @brief 带有 SFINAE 约束的构造函数调用器包装器。
     *         为调用构造函数提供统一接口。
     *
     * @tparam Ty 要构造的类型
     * @tparam Args 构造函数参数类型
     */
    template <typename Ty, typename... Args>
    struct ctor : type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...>,
                                                        implements::ctor_impl<Ty, Args...>> {};

    /**
     * \lang english
     * @brief Destructor invoker wrapper with SFINAE constraints.
     *         Provides a unified interface for invoking destructors.
     *
     * @tparam Ty The type to destroy
     *
     * \lang simp-chinese
     * @brief 带有 SFINAE 约束的析构函数调用器包装器。
     *         为调用析构函数提供统一接口。
     *
     * @tparam Ty 要销毁的类型
     */
    template <typename Ty>
    struct dtor
        : type_traits::other_trans::enable_if_t<type_traits::properties::is_destructible_v<Ty>, implements::dtor_impl<Ty>> {};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Gets a function pointer to the constructor invoker for type Ty.
     *
     * @tparam Ty The type to construct
     * @tparam Args The constructor argument types
     * @return Function pointer to ctor<Ty, Args...>::invoke
     *
     * \lang simp-chinese
     * @brief 获取指向类型 Ty 的构造函数调用器的函数指针。
     *
     * @tparam Ty 要构造的类型
     * @tparam Args 构造函数参数类型
     * @return 指向 ctor<Ty, Args...>::invoke 的函数指针
     */
    template <typename Ty, typename... Args,
              typename type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...>, int> = 0>
    constexpr rain_fn get_ctor_fn() -> auto {
        return &ctor<Ty, Args...>::invoke;
    }

    /**
     * \lang english
     * @brief Gets a function pointer to the destructor invoker for type Ty.
     *
     * @tparam Ty The type to destroy
     * @return Function pointer to dtor<Ty>::invoke
     *
     * \lang simp-chinese
     * @brief 获取指向类型 Ty 的析构函数调用器的函数指针。
     *
     * @tparam Ty 要销毁的类型
     * @return 指向 dtor<Ty>::invoke 的函数指针
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::properties::is_destructible_v<Ty>, int> = 0>
    constexpr rain_fn get_dtor_fn() -> auto {
        return &dtor<Ty>::invoke;
    }
}

namespace rainy::utility::implements {
    template <typename Ty, typename Assign>
    struct assign_impl {
        static constexpr type_traits::modifers::add_lvalue_reference_t<Ty> invoke(
            type_traits::modifers::add_lvalue_reference_t<Ty> this_,
            Assign assign) noexcept(type_traits::properties::is_nothrow_assignable_v<Ty, Assign>) {
            return (this_ = assign);
        }
    };

    template <typename Ty>
    struct copy_assign_impl {
        static constexpr type_traits::modifers::add_lvalue_reference_t<Ty> invoke(
            type_traits::modifers::add_lvalue_reference_t<Ty> this_,
            type_traits::modifers::add_rvalue_reference_t<Ty>
                &&rvalue) noexcept(type_traits::properties::is_nothrow_copy_assignable_v<Ty>) {
            return (this_ = rvalue);
        }
    };

    template <typename Ty>
    struct move_assign_impl {
        static constexpr type_traits::modifers::add_lvalue_reference_t<Ty> invoke(
            type_traits::modifers::add_lvalue_reference_t<Ty> this_,
            type_traits::modifers::add_rvalue_reference_t<Ty>
                rvalue) noexcept(type_traits::properties::is_nothrow_move_assignable_v<Ty>) {
            return (this_ = utility::move(rvalue));
        }
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Assignment operator invoker wrapper with SFINAE constraints.
     *         Provides a unified interface for invoking assignment operators.
     *
     * @tparam Ty The type being assigned to
     * @tparam Assign The type being assigned from
     *
     * \lang simp-chinese
     * @brief 带有 SFINAE 约束的赋值运算符调用器包装器。
     *         为调用赋值运算符提供统一接口。
     *
     * @tparam Ty 被赋值的类型
     * @tparam Assign 来源类型
     */
    template <typename Ty, typename Assign>
    struct assign : type_traits::other_trans::enable_if_t<type_traits::properties::is_assignable_v<Ty, Assign> &&
                                                              !type_traits::properties::is_const_v<Ty> &&
                                                              !type_traits::composite_types::is_reference_v<Ty>,
                                                          implements::assign_impl<Ty, Assign>> {};

    /**
     * \lang english
     * @brief Copy assignment operator invoker wrapper with SFINAE constraints.
     *         Provides a unified interface for invoking copy assignment operators.
     *
     * @tparam Ty The type being copy assigned
     *
     * \lang simp-chinese
     * @brief 带有 SFINAE 约束的拷贝赋值运算符调用器包装器。
     *         为调用拷贝赋值运算符提供统一接口。
     *
     * @tparam Ty 被拷贝赋值的类型
     */
    template <typename Ty>
    struct copy_assign : type_traits::other_trans::enable_if_t<type_traits::properties::is_copy_assignable_v<Ty>,
                                                               implements::copy_assign_impl<Ty>> {};

    /**
     * \lang english
     * @brief Move assignment operator invoker wrapper with fallback to copy.
     *         Provides move assignment if available, otherwise falls back to copy assignment.
     *
     * @tparam Ty The type being move assigned
     *
     * \lang simp-chinese
     * @brief 带有拷贝回退的移动赋值运算符调用器包装器。
     *         如果可用则提供移动赋值，否则回退到拷贝赋值。
     *
     * @tparam Ty 被移动赋值的类型
     */
    template <typename Ty>
    struct move_assign : type_traits::other_trans::conditional_t<
                             type_traits::properties::is_move_assignable_v<Ty>, implements::move_assign_impl<Ty>,
                             type_traits::other_trans::enable_if_t<type_traits::properties::is_copy_assignable_v<Ty>,
                                                                   implements::copy_assign_impl<Ty>>> {};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Gets a function pointer to the assignment operator invoker.
     *
     * @tparam Ty The type being assigned to
     * @tparam Assign The type being assigned from
     * @return Function pointer to assign<Ty, Assign>::invoke
     *
     * \lang simp-chinese
     * @brief 获取指向赋值运算符调用器的函数指针。
     *
     * @tparam Ty 被赋值的类型
     * @tparam Assign 来源类型
     * @return 指向 assign<Ty, Assign>::invoke 的函数指针
     */
    template <typename Ty, typename Assign,
              type_traits::other_trans::enable_if_t<type_traits::properties::is_assignable_v<Ty, Assign> &&
                                                        !type_traits::properties::is_const_v<Ty> &&
                                                        !type_traits::composite_types::is_reference_v<Ty>,
                                                    int> = 0>
    constexpr rain_fn get_assign() -> auto {
        return &assign<Ty, Assign>::invoke;
    }

    /**
     * \lang english
     * @brief Gets a function pointer to the move assignment operator invoker.
     *
     * @tparam Ty The type being move assigned
     * @return Function pointer to move_assign<Ty>::invoke
     *
     * \lang simp-chinese
     * @brief 获取指向移动赋值运算符调用器的函数指针。
     *
     * @tparam Ty 被移动赋值的类型
     * @return 指向 move_assign<Ty>::invoke 的函数指针
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::properties::is_move_assignable_v<Ty> ||
                                                                     type_traits::properties::is_nothrow_copy_assignable_v<Ty>,
                                                                 int> = 0>
    constexpr rain_fn get_move_assign() -> auto {
        return &move_assign<Ty>::invoke;
    }

    /**
     * \lang english
     * @brief Gets a function pointer to the copy assignment operator invoker.
     *
     * @tparam Ty The type being copy assigned
     * @return Function pointer to copy_assign<Ty>::invoke
     *
     * \lang simp-chinese
     * @brief 获取指向拷贝赋值运算符调用器的函数指针。
     *
     * @tparam Ty 被拷贝赋值的类型
     * @return 指向 copy_assign<Ty>::invoke 的函数指针
     */
    template <typename Ty,
              type_traits::other_trans::enable_if_t<type_traits::properties::is_nothrow_copy_assignable_v<Ty>, int> = 0>
    constexpr rain_fn get_copy_assign() -> auto {
        return &copy_assign<Ty>::invoke;
    }
}

namespace rainy::utility::cpp_methods {
    /**
     * \lang english
     * @brief String representation of operator+
     *
     * \lang simp-chinese
     * @brief operator+ 的字符串表示
     */
    static constexpr auto method_operator_add = "operator+";

    /**
     * \lang english
     * @brief String representation of operator-
     *
     * \lang simp-chinese
     * @brief operator- 的字符串表示
     */
    static constexpr auto method_operator_sub = "operator-";

    /**
     * \lang english
     * @brief String representation of operator*
     *
     * \lang simp-chinese
     * @brief operator* 的字符串表示
     */
    static constexpr auto method_operator_mul = "operator*";

    /**
     * \lang english
     * @brief String representation of operator/
     *
     * \lang simp-chinese
     * @brief operator/ 的字符串表示
     */
    static constexpr auto method_operator_div = "operator/";

    /**
     * \lang english
     * @brief String representation of operator%
     *
     * \lang simp-chinese
     * @brief operator% 的字符串表示
     */
    static constexpr auto method_operator_mod = "operator%";

    /**
     * \lang english
     * @brief String representation of operator==
     *
     * \lang simp-chinese
     * @brief operator== 的字符串表示
     */
    static constexpr auto method_operator_eq = "operator==";

    /**
     * \lang english
     * @brief String representation of operator!=
     *
     * \lang simp-chinese
     * @brief operator!= 的字符串表示
     */
    static constexpr auto method_operator_neq = "operator!=";

    /**
     * \lang english
     * @brief String representation of operator<
     *
     * \lang simp-chinese
     * @brief operator< 的字符串表示
     */
    static constexpr auto method_operator_lt = "operator<";

    /**
     * \lang english
     * @brief String representation of operator>
     *
     * \lang simp-chinese
     * @brief operator> 的字符串表示
     */
    static constexpr auto method_operator_gt = "operator>";

    /**
     * \lang english
     * @brief String representation of operator<=
     *
     * \lang simp-chinese
     * @brief operator<= 的字符串表示
     */
    static constexpr auto method_operator_le = "operator<=";

    /**
     * \lang english
     * @brief String representation of operator>=
     *
     * \lang simp-chinese
     * @brief operator>= 的字符串表示
     */
    static constexpr auto method_operator_ge = "operator>=";

    /**
     * \lang english
     * @brief String representation of operator=
     *
     * \lang simp-chinese
     * @brief operator= 的字符串表示
     */
    static constexpr auto method_operator_assign = "operator=";

    /**
     * \lang english
     * @brief String representation of operator[]
     *
     * \lang simp-chinese
     * @brief operator[] 的字符串表示
     */
    static constexpr auto method_operator_index = "operator[]";

    /**
     * \lang english
     * @brief String representation of operator()
     *
     * \lang simp-chinese
     * @brief operator() 的字符串表示
     */
    static constexpr auto method_operator_call = "operator()";

    /**
     * \lang english
     * @brief String representation of operator->
     *
     * \lang simp-chinese
     * @brief operator-> 的字符串表示
     */
    static constexpr auto method_operator_arrow = "operator->";

    /**
     * \lang english
     * @brief String representation of operator* (dereference)
     *
     * \lang simp-chinese
     * @brief operator*（解引用）的字符串表示
     */
    static constexpr auto method_operator_deref = "operator*";

    /**
     * \lang english
     * @brief String representation of operator& (address-of)
     *
     * \lang simp-chinese
     * @brief operator&（取地址）的字符串表示
     */
    static constexpr auto method_operator_addr = "operator&";

    /**
     * \lang english
     * @brief String representation of operator++ (prefix)
     *
     * \lang simp-chinese
     * @brief operator++（前缀）的字符串表示
     */
    static constexpr auto method_operator_preinc = "operator++";

    /**
     * \lang english
     * @brief String representation of operator++ (postfix)
     *
     * \lang simp-chinese
     * @brief operator++（后缀）的字符串表示
     */
    static constexpr auto method_operator_postinc = "operator++(int)";

    /**
     * \lang english
     * @brief String representation of operator-- (prefix)
     *
     * \lang simp-chinese
     * @brief operator--（前缀）的字符串表示
     */
    static constexpr auto method_operator_predec = "operator--";

    /**
     * \lang english
     * @brief String representation of operator-- (postfix)
     *
     * \lang simp-chinese
     * @brief operator--（后缀）的字符串表示
     */
    static constexpr auto method_operator_postdec = "operator--(int)";

    /**
     * \lang english
     * @brief String representation of operator||
     *
     * \lang simp-chinese
     * @brief operator|| 的字符串表示
     */
    static constexpr auto method_operator_or = "operator||";

    /**
     * \lang english
     * @brief String representation of operator&&
     *
     * \lang simp-chinese
     * @brief operator&& 的字符串表示
     */
    static constexpr auto method_operator_and = "operator&&";

    /**
     * \lang english
     * @brief String representation of operator!
     *
     * \lang simp-chinese
     * @brief operator! 的字符串表示
     */
    static constexpr auto method_operator_not = "operator!";

    /**
     * \lang english
     * @brief String representation of operator|
     *
     * \lang simp-chinese
     * @brief operator| 的字符串表示
     */
    static constexpr auto method_operator_bit_or = "operator|";

    /**
     * \lang english
     * @brief String representation of operator& (bitwise AND)
     *
     * \lang simp-chinese
     * @brief operator&（按位与）的字符串表示
     */
    static constexpr auto method_operator_bit_and = "operator&";

    /**
     * \lang english
     * @brief String representation of operator^
     *
     * \lang simp-chinese
     * @brief operator^ 的字符串表示
     */
    static constexpr auto method_operator_bit_xor = "operator^";

    /**
     * \lang english
     * @brief String representation of operator~
     *
     * \lang simp-chinese
     * @brief operator~ 的字符串表示
     */
    static constexpr auto method_operator_bit_not = "operator~";

    /**
     * \lang english
     * @brief String representation of operator<<
     *
     * \lang simp-chinese
     * @brief operator<< 的字符串表示
     */
    static constexpr auto method_operator_shift_l = "operator<<";

    /**
     * \lang english
     * @brief String representation of operator>>
     *
     * \lang simp-chinese
     * @brief operator>> 的字符串表示
     */
    static constexpr auto method_operator_shift_r = "operator>>";

    /**
     * \lang english
     * @brief String representation of begin()
     *
     * \lang simp-chinese
     * @brief begin() 的字符串表示
     */
    static constexpr auto method_begin = "begin";

    /**
     * \lang english
     * @brief String representation of end()
     *
     * \lang simp-chinese
     * @brief end() 的字符串表示
     */
    static constexpr auto method_end = "end";

    /**
     * \lang english
     * @brief String representation of cbegin()
     *
     * \lang simp-chinese
     * @brief cbegin() 的字符串表示
     */
    static constexpr auto method_cbegin = "cbegin";

    /**
     * \lang english
     * @brief String representation of cend()
     *
     * \lang simp-chinese
     * @brief cend() 的字符串表示
     */
    static constexpr auto method_cend = "cend";

    /**
     * \lang english
     * @brief String representation of rbegin()
     *
     * \lang simp-chinese
     * @brief rbegin() 的字符串表示
     */
    static constexpr auto method_rbegin = "rbegin";

    /**
     * \lang english
     * @brief String representation of rend()
     *
     * \lang simp-chinese
     * @brief rend() 的字符串表示
     */
    static constexpr auto method_rend = "rend";

    /**
     * \lang english
     * @brief String representation of size()
     *
     * \lang simp-chinese
     * @brief size() 的字符串表示
     */
    static constexpr auto method_size = "size";

    /**
     * \lang english
     * @brief String representation of empty()
     *
     * \lang simp-chinese
     * @brief empty() 的字符串表示
     */
    static constexpr auto method_empty = "empty";

    /**
     * \lang english
     * @brief String representation of clear()
     *
     * \lang simp-chinese
     * @brief clear() 的字符串表示
     */
    static constexpr auto method_clear = "clear";

    /**
     * \lang english
     * @brief String representation of push_back()
     *
     * \lang simp-chinese
     * @brief push_back() 的字符串表示
     */
    static constexpr auto method_push_back = "push_back";

    /**
     * \lang english
     * @brief String representation of pop_back()
     *
     * \lang simp-chinese
     * @brief pop_back() 的字符串表示
     */
    static constexpr auto method_pop_back = "pop_back";

    /**
     * \lang english
     * @brief String representation of length()
     *
     * \lang simp-chinese
     * @brief length() 的字符串表示
     */
    static constexpr auto method_length = "length";

    /**
     * \lang english
     * @brief String representation of insert()
     *
     * \lang simp-chinese
     * @brief insert() 的字符串表示
     */
    static constexpr auto method_insert = "insert";

    /**
     * \lang english
     * @brief String representation of erase()
     *
     * \lang simp-chinese
     * @brief erase() 的字符串表示
     */
    static constexpr auto method_erase = "erase";

    /**
     * \lang english
     * @brief String representation of find()
     *
     * \lang simp-chinese
     * @brief find() 的字符串表示
     */
    static constexpr auto method_find = "find";

    /**
     * \lang english
     * @brief String representation of resize()
     *
     * \lang simp-chinese
     * @brief resize() 的字符串表示
     */
    static constexpr auto method_resize = "resize";

    /**
     * \lang english
     * @brief String representation of swap()
     *
     * \lang simp-chinese
     * @brief swap() 的字符串表示
     */
    static constexpr auto method_swap = "swap";

    /**
     * \lang english
     * @brief String representation of at()
     *
     * \lang simp-chinese
     * @brief at() 的字符串表示
     */
    static constexpr auto method_at = "at";

    /**
     * \lang english
     * @brief String representation of front()
     *
     * \lang simp-chinese
     * @brief front() 的字符串表示
     */
    static constexpr auto method_front = "front";

    /**
     * \lang english
     * @brief String representation of back()
     *
     * \lang simp-chinese
     * @brief back() 的字符串表示
     */
    static constexpr auto method_back = "back";

    /**
     * \lang english
     * @brief String representation of append()
     *
     * \lang simp-chinese
     * @brief append() 的字符串表示
     */
    static constexpr auto method_append = "append";
}

namespace rainy::core {
    /**
     * \lang english
     * @brief Accumulates values in a range using operator+.
     *
     * @tparam InputIt Input iterator type
     * @tparam Ty Initial value and result type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param init Initial value for accumulation
     * @return The result of accumulating all elements with init
     *
     * \lang simp-chinese
     * @brief 使用 operator+ 累加范围内的值。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Ty 初始值和结果类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param init 累加的初始值
     * @return 使用 init 累加所有元素的结果
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
     * \lang english
     * @brief Accumulates values in a range using a custom binary operation.
     *
     * @tparam InputIt Input iterator type
     * @tparam Ty Initial value and result type
     * @tparam BinaryOperation Binary operation type
     * @param first Iterator to the beginning of the range
     * @param last Iterator to the end of the range
     * @param init Initial value for accumulation
     * @param op Binary operation to apply
     * @return The result of applying op to all elements with init
     *
     * \lang simp-chinese
     * @brief 使用自定义二元操作累加范围内的值。
     *
     * @tparam InputIt 输入迭代器类型
     * @tparam Ty 初始值和结果类型
     * @tparam BinaryOperation 二元操作类型
     * @param first 指向范围起始的迭代器
     * @param last 指向范围末尾的迭代器
     * @param init 累加的初始值
     * @param op 要应用的二元操作
     * @return 使用 init 对所有元素应用 op 的结果
     */
    template <typename InputIt, typename Ty, typename BinaryOperation>
    RAINY_INLINE constexpr rain_fn accumulate(InputIt first, InputIt last, Ty init, BinaryOperation op) -> Ty {
        for (; first != last; ++first) {
            init = utility::invoke(op, init, *first);
        }
        return init;
    }
}

namespace rainy::type_traits::properties {
    /**
     * \lang english
     * @brief Variable template for checking if a type is movable.
     *         A type is movable if it is an object type, move constructible,
     *         assignable from rvalue, and swappable.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否可移动的变量模板。
     *         类型如果是对象类型、可移动构造、可从右值赋值且可交换，则是可移动的。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_movable_v = composite_types::is_object_v<Ty> && properties::is_move_constructible_v<Ty> &&
                                        properties::is_assignable_v<Ty &, Ty> && properties::is_swappable_v<Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is movable.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否可移动的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_movable : helper::bool_constant<is_movable_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is copyable.
     *         A type is copyable if it is copy constructible, movable,
     *         and assignable from lvalue, const lvalue, and const rvalue.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否可拷贝的变量模板。
     *         类型如果是可拷贝构造、可移动、可从左值、常量左值和常量右值赋值，则是可拷贝的。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_copyable_v =
        properties::is_copy_constructible_v<Ty> && is_movable_v<Ty> &&
        type_traits::properties::is_assignable_v<Ty &, Ty &> && type_traits::properties::is_assignable_v<Ty &, const Ty &> &&
        type_traits::properties::is_assignable_v<Ty &, const Ty>;

    /**
     * \lang english
     * @brief Type template for checking if a type is copyable.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否可拷贝的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_copyable : helper::bool_constant<is_copyable_v<Ty>> {};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Maximum possible value for a signed difference type.
     *
     * @tparam Diff The signed difference type
     *
     * \lang simp-chinese
     * @brief 有符号差类型的最大可能值。
     *
     * @tparam Diff 有符号差类型
     */
    template <typename Diff>
    constexpr Diff max_possible_v{static_cast<type_traits::helper::make_unsigned_t<Diff>>(-1) >> 1};

    /**
     * \lang english
     * @brief Minimum possible value for a signed difference type.
     *
     * @tparam Diff The signed difference type
     *
     * \lang simp-chinese
     * @brief 有符号差类型的最小可能值。
     *
     * @tparam Diff 有符号差类型
     */
    template <typename Diff>
    constexpr Diff min_possible_v{-max_possible_v<Diff> - 1};
}

namespace rainy::type_traits::properties {
    /**
     * \lang english
     * @brief Type template for checking if a type is a sequential container.
     *         Sequential containers support push_back or are arrays.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为顺序容器的类型模板。
     *         顺序容器支持 push_back 或是数组。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_sequential_container
        : helper::bool_constant<type_traits::extras::meta_method::has_push_back_v<Ty> || primary_types::is_array_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is a sequential container.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为顺序容器的变量模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_sequential_container_v = is_sequential_container<Ty>::value;

    /**
     * \lang english
     * @brief Type template for checking if a type is an associative container.
     *         Associative containers support insert with key or key-value pairs,
     *         and do not support push_back.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为关联容器的类型模板。
     *         关联容器支持使用键或键值对的插入，不支持 push_back。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_associative_container : helper::bool_constant<type_traits::extras::meta_method::has_insert_for_key_v<Ty> &&
                                                            type_traits::extras::meta_method::has_insert_for_key_and_value_v<Ty> &&
                                                            !type_traits::extras::meta_method::has_push_back_v<Ty>> {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is an associative container.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为关联容器的变量模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_associative_container_v = is_associative_container<Ty>::value;

    /**
     * \lang english
     * @brief Type template for checking if a type is map-like (has key_type and mapped_type).
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为类似映射的类型（具有 key_type 和 mapped_type）的类型模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename, typename = void>
    struct is_map_like : helper::false_type {};

    /**
     * \lang english
     * @brief Specialization that detects key_type and mapped_type members.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检测 key_type 和 mapped_type 成员的特化。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_map_like<Ty, other_trans::void_t<typename Ty::key_type, typename Ty::mapped_type>> : helper::true_type {};

    /**
     * \lang english
     * @brief Variable template for checking if a type is map-like.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检查类型是否为类似映射的类型的变量模板。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    inline constexpr bool is_map_like_v = is_map_like<Ty>::value;
}

#endif
