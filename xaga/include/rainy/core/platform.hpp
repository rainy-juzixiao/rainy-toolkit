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
/**
 * @file platform.hpp
 * @brief
 * 这是rainy-toolkit的核心文件的最重要部分。它定义了rainy-toolkit如何与平台进行沟通。没有它，rainy-toolkit的构建系统将无法正常工作
 * @brief
 * 简单点说。它不是给用户使用的。是给库开发者使用的。它包含了一些平台相关的宏定义和函数声明以及非常基础的类型定义和模板元编程代码。它由core.hpp使用
 * @brief 它不应该被用户包含，而是考虑使用core.hpp
 * @brief 另外，包含了一系列内建函数。部分内建函数是由core.cxx负责的。直接包含则有可能导致编译失败
 * @author rainy-juzixiao
 *
 * @date 2/24/2025 1:30:49 PM 在此进行添加注释（由rainy-juzixiao添加）
 * @date 3/13/2025 1:57:45 PM 在此转移type_traits的other_trans代码以及修改注释
 */
#ifndef RAINY_CORE_PLATFORM_HPP
#define RAINY_CORE_PLATFORM_HPP
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <string_view>
#include <ctime>
#include <utility>

#ifdef __linux__
#include <csignal>
#include <linux/version.h>
#include <unistd.h>
#endif

#include <rainy/core/implements/generate/marco_gen.hpp>

#define RAINY_EXTERN_C extern "C"

/*-----------
MSVC编译器区域
-----------*/
#ifdef _MSC_VER
// 为MSVC编译器提供支持
#pragma warning(default : 4996)

#if !defined(RAINY_ENABLE_DEBUG) || !defined(RAINY_DISABLE_DEBUG)
#ifdef NDEBUG
#define RAINY_ENABLE_DEBUG 0
#else
#define RAINY_ENABLE_DEBUG 1
#endif // ifndef NDEBUG
#endif

#ifndef RAINY_INLINE
#define RAINY_INLINE __inline
#endif

#ifndef RAINY_ALWAYS_INLINE
#define RAINY_ALWAYS_INLINE __forceinline
#endif

#ifndef RAINY_CURRENT_STANDARD_VERSION
#define RAINY_CURRENT_STANDARD_VERSION _MSVC_LANG
#endif

/*----------------
clang和GNU编译器区域
----------------*/
#elif defined(__GNUC__) || defined(__clang__)
// 为GNU编译器和clang编译器提供支持

#if !defined(RAINY_ENABLE_DEBUG) || !defined(RAINY_DISABLE_DEBUG)
#ifndef NDEBUG
#define RAINY_ENABLE_DEBUG 1
#else
#define RAINY_ENABLE_DEBUG 0
#endif // ifndef NDEBUG
#endif

#ifndef RAINY_ALWAYS_INLINE
#ifdef __GNUC__
#define RAINY_ALWAYS_INLINE [[__gnu__::__always_inline__]]
#else
#define RAINY_ALWAYS_INLINE [[always_inline]]
#endif
#endif

#ifndef RAINY_INLINE
#define RAINY_INLINE inline
#endif

#ifndef RAINY_CURRENT_STANDARD_VERSION
#define RAINY_CURRENT_STANDARD_VERSION __cplusplus
#endif

#endif // #if defined __GNUC__ || defined __clang__

#if RAINY_CURRENT_STANDARD_VERSION > 201703L
#define RAINY_HAS_CXX20 true
#else
#define RAINY_HAS_CXX20 false
#endif

#if RAINY_CURRENT_STANDARD_VERSION > 202002L
#define RAINY_HAS_CXX23 true
#else
#define RAINY_HAS_CXX23 false
#endif

#if RAINY_CURRENT_STANDARD_VERSION >= 202400L
#define RAINY_HAS_CXX26 true
#else
#define RAINY_HAS_CXX26 false
#endif

#if RAINY_CURRENT_STANDARD_VERSION == 201703L
#define RAINY_IS_CXX17 true
#else
#define RAINY_IS_CXX17 false
#endif

#ifndef RAINY_NODISCARD
#define RAINY_NODISCARD [[nodiscard]]
#endif

#ifndef RAINY_NODISCARD_MSG
#define RAINY_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#endif

#if RAINY_HAS_CXX20
#define RAINY_CONSTEXPR20 constexpr
#else
#define RAINY_CONSTEXPR20
#endif

#if RAINY_HAS_CXX23
#define RAINY_CONSTEXPR23 constexpr
#else
#define RAINY_CONSTEXPR23
#endif

#ifndef RAINY_NODISCARD_CONSTEXPR20
#define RAINY_NODISCARD_CONSTEXPR20 RAINY_NODISCARD RAINY_CONSTEXPR20
#endif

#ifndef RAINY_INLINE_NODISCARD
#define RAINY_INLINE_NODISCARD RAINY_NODISCARD RAINY_INLINE
#endif

#ifndef RAINY_ALWAYS_INLINE_NODISCARD
#define RAINY_ALWASY_INLINE_NODISCARD RAINY_NODISCARD RAINY_ALWAYS_INLINE
#endif

#ifndef RAINY_NODISCARD_FRIEND
#define RAINY_NODISCARD_FRIEND RAINY_NODISCARD friend
#endif

#ifndef RAINY_USING_GCC
#ifdef __GNUC__
#define RAINY_USING_GCC 1
#else
#define RAINY_USING_GCC 0
#endif

#ifndef RAINY_USING_CLANG
#if defined(__clang__) && !defined(__GNUC__)
#define RAINY_USING_CLANG 1
#else
#define RAINY_USING_CLANG 0
#endif
#endif

#ifndef RAINY_USING_MSVC
#if defined(_MSC_VER) && !defined(__clang__)
#define RAINY_USING_MSVC 1
#else
#define RAINY_USING_MSVC 0
#endif
#endif

#ifndef RAINY_USING_MSVC_CLANG
#if defined(_MSC_VER) && defined(__clang__)
#define RAINY_USING_MSVC_CLANG 1
#else
#define RAINY_USING_MSVC_CLANG 0
#endif
#endif

#define RAINY_INLINE_CONSTEXPR inline constexpr
#define RAINY_CONSTEXPR_BOOL RAINY_INLINE_CONSTEXPR bool
#define RAINY_FALLTHROUGH [[fallthrough]]
#define RAINY_CONSTEXPR constexpr

#define RAINY_STRINGIFY_HELPER(x) #x
#define RAINY_STRINGIFY(x) RAINY_STRINGIFY_HELPER(x)

#ifndef RAINY_STRINGIZE
#define RAINY_STRINGIZE(...) #__VA_ARGS__
#endif

#ifdef _WIN32
#define RAINY_USING_WINDOWS 1
#else
#define RAINY_USING_WINDOWS 0
#endif

#ifdef __linux__
#define RAINY_USING_LINUX 1
#else
#define RAINY_USING_LINUX 0
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE

#ifdef __aarch64__

#define RAINY_USING_MACOS 1
#define RAINY_USING_MACOS_AND_IS_APPLE_SILICON 1

#else

#define RAINY_USING_MACOS 1
#define RAINY_USING_MACOS_AND_IS_APPLE_SILICON 0

#endif

#endif

#else

#define RAINY_USING_MACOS 0
#define RAINY_USING_MACOS_AND_IS_APPLE_SILICON 0

#endif

#if RAINY_USING_GCC
#define RAINY_AINLINE_NODISCARD RAINY_NODISCARD RAINY_INLINE
#else
#define RAINY_AINLINE_NODISCARD RAINY_NODISCARD RAINY_ALWAYS_INLINE
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#define RAINY_USING_64_BIT_PLATFORM 1
#define RAINY_USING_32_BIT_PLATFORM 0
#else
#define RAINY_USING_64_BIT_PLATFORM 0
#define RAINY_USING_32_BIT_PLATFORM 1
#endif

#if defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__aarch64__)
#define RAINY_IS_ARM64 1
#define RAINY_IS_X86_PLATFORM 0
#elif defined(_M_IX86) || defined(_M_X86_) || defined(__i386__) || defined(__x86_64__) || defined(_M_X64)
#define RAINY_IS_ARM64 0
#define RAINY_IS_X86_PLATFORM 1
#else
static_assert(false, "Detected invalid architecture,rainy-toolkit is not support on your compile architecture");
#endif

#if RAINY_CURRENT_STANDARD_VERSION < 201703L
static_assert(false, "We detected you are using C++14 and below, and the library only supports C++17 and above, please modify your "
                     "standard version of C++ to C++17 and above before trying to compile!");
#endif

#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
#include <immintrin.h> // NOLINT
#endif

#if RAINY_HAS_CXX20
#include <compare> // NOLINT
#endif

#ifndef RAINY_DEPRECATED
#define RAINY_DEPRECATED [[deprecated]]
#endif

#ifndef RAINY_DEPRECATED_MSG
#define RAINY_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#endif

#ifndef RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS

#ifndef RAINY_CXX17_DEPRECATED_TYPEDEF
#if !defined(RAINY_DISABLE_CXX17_DEPRECATED_TYPEDEF) || !defined(RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS)

#define RAINY_CXX17_DEPRECATED_TYPEDEF                                                                                                \
    RAINY_DEPRECATED_MSG("In C++17, certain parts of the standard library design must adhere to the C++17 specifications, such "      \
                         "as deprecated typedef or using declarations. You can define the "                                           \
                         "RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS macro to suppress these and other similar warnings, or define " \
                         "the RAINY_DISABLE_CXX17_DEPRECATED_TYPEDEF macro to disable only specific deprecation warnings.")

#else
#define RAINY_CXX17_DEPRECATED_TYPEDEF
#endif // if !defined(RAINY_DISABLE_CXX17_DEPRECATED_TYPEDEF) || !defined(RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS)

#endif // ifndef RAINY_CXX17_DEPRECATED_TYPEDEF

#endif // RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS

#endif

// NOLINTBEGIN
/* 只有涉及到cast表达式时，我们才会使用这些宏 */
#define rainy_let auto
#define rainy_ref auto &
#define rainy_const const auto
#define rainy_cref const auto &
// NOLINTEND

#if RAINY_ENABLE_DEBUG

#define rainy_assume(expr) assert(expr)

#else

#if RAINY_USING_MSVC

#define rainy_assume(expr) __assume(expr)

#elif RAINY_USING_CLANG

#define rainy_assume(expr) __builtin_assume(expr)

#elif RAINY_USING_GCC

#define rainy_assume(expr)                                                                                                            \
    if (expr) {                                                                                                                       \
    } else {                                                                                                                          \
        __builtin_unreachable();                                                                                                      \
    }

#endif

#endif

#if RAINY_USING_MSVC
#define rainy_likely(x) (x)
#define rainy_unlikely(x) (x)
#else
#define rainy_likely(x) __builtin_expect(!!(x), 1)
#define rainy_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#if RAINY_USING_MSVC
#define RAINY_NORETURN __declspec(noreturn)
#else
#define RAINY_NORETURN __attribute__((noreturn))
#endif

#if RAINY_USING_MSVC
#define RAINY_NOINLINE __declspec(noinline)
#else
#define RAINY_NOINLINE __attribute__((noinline))
#endif

#define RAINY_CAT_IMPL(a, b) a##b
#define RAINY_CAT(a, b) RAINY_CAT_IMPL(a, b)

#if RAINY_USING_WINDOWS
#if RAINY_USING_DYNAMIC
#ifdef RAINY_DYNAMIC_EXPORTS
#define RAINY_TOOLKIT_API __declspec(dllexport)
#else
#define RAINY_TOOLKIT_API __declspec(dllimport)
#endif
#else
#define RAINY_TOOLKIT_API
#endif
#else
#define RAINY_TOOLKIT_API __attribute__((visibility("default")))
#endif

#if RAINY_USING_MSVC
#define RAINY_TOOLKIT_LOCAL_API
#else
#define RAINY_TOOLKIT_LOCAL_API __attribute__((visibility("hidden")))
#endif

#if RAINY_USING_MSVC
#define RAINY_CTOR_DECLARE_FUNCTION
#define RAINY_DTOR_DECLARE_FUNCTION
#elif RAINY_USING_GCC
#define RAINY_CTOR_DECLARE_FUNCTION __attribute__((constructor))
#define RAINY_DTOR_DECLARE_FUNCTION __attribute__((destructor))
#elif RAINY_USING_CLANG
#define RAINY_CTOR_DECLARE_FUNCTION __attribute__((__constructor__))
#define RAINY_DTOR_DECLARE_FUNCTION __attribute__((__destructor__))
#endif

#ifdef __EDG__
#define RAINY_USING_EDG 1
#else
#define RAINY_USING_EDG 0
#endif

#if RAINY_USING_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define RAINY_DECL_MAYBE_UNUSED_IF_WINDOWS [[maybe_unused]]
#define RAINY_DECL_MAYBE_UNUSED_PARAM_IF_WINDOWS(x) x [[maybe_unused]]
#else
#define RAINY_DECL_MAYBE_UNUSED_IF_WINDOWS
#define RAINY_DECL_MAYBE_UNUSED_PARAM_IF_WINDOWS(x) x
#endif

#if RAINY_USING_LINUX
#define RAINY_DECL_MAYBE_UNUSED_IF_LINUX [[maybe_unused]]
#define RAINY_DECL_MAYBE_UNUSED_PARAM_IF_LINUX(x) x [[maybe_unused]]
#else
#define RAINY_DECL_MAYBE_UNUSED_IF_LINUX
#define RAINY_DECL_MAYBE_UNUSED_PARAM_IF_LINUX(x) x
#endif

#if RAINY_USING_LINUX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
#define RAINY_HAS_IO_URING 1
#else
#define RAINY_HAS_IO_URING 0
#endif
#endif

#if RAINY_USING_MSVC

#define RAINY_IS_USING_MSVC_STL 1
#define RAINY_IS_USING_LIBCXX 0
#define RAINY_IS_USING_LIBSTDCXX 0

#elif RAINY_USING_CLANG
#if defined(_MSC_VER) && defined(_MSVC_STL_VERSION)

#define RAINY_IS_USING_MSVC_STL 1
#define RAINY_IS_USING_LIBCXX 0
#define RAINY_IS_USING_LIBSTDCXX 0

#elif defined(_LIBCPP_VERSION)

#define RAINY_IS_USING_MSVC_STL 0
#define RAINY_IS_USING_LIBCXX 1
#define RAINY_IS_USING_LIBSTDCXX 0

#elif defined(__GLIBCXX__)

#define RAINY_IS_USING_MSVC_STL 0
#define RAINY_IS_USING_LIBCXX 0
#define RAINY_IS_USING_LIBSTDCXX 1

#endif

#elif RAINY_USING_GCC

#define RAINY_IS_USING_MSVC_STL 0
#define RAINY_IS_USING_LIBCXX 0
#define RAINY_IS_USING_LIBSTDCXX 1

#endif

#define RAINY_STAMP4(n, x)                                                                                                            \
    x(n);                                                                                                                             \
    x(n + 1);                                                                                                                         \
    x(n + 2);                                                                                                                         \
    x(n + 3)
#define RAINY_STAMP16(n, x)                                                                                                           \
    RAINY_STAMP4(n, x);                                                                                                               \
    RAINY_STAMP4(n + 4, x);                                                                                                           \
    RAINY_STAMP4(n + 8, x);                                                                                                           \
    RAINY_STAMP4(n + 12, x)
#define RAINY_STAMP64(n, x)                                                                                                           \
    RAINY_STAMP16(n, x);                                                                                                              \
    RAINY_STAMP16(n + 16, x);                                                                                                         \
    RAINY_STAMP16(n + 32, x);                                                                                                         \
    RAINY_STAMP16(n + 48, x)

#define RAINY_STAMP(n, x) x(RAINY_STAMP##n, n)

#define rain_fn auto
#define rain_loop for (;;)

#define RAINY_TOOLKIT_VERSION                                                                                                         \
    "rainy-toolkit:" RAINY_STRINGIFY(RAINY_TOOLKIT_PROJECT_MAJOR) "." RAINY_STRINGIFY(                                                \
        RAINY_TOOLKIT_PROJECT_MINOR) "." RAINY_STRINGIFY(RAINY_TOOLKIT_PROJECT_PATCH) ".xaga"

#define RAINY_ABI_BRIDGE_CALL_HANDLER_TOTAL_COUNT 0
#define RAINY_ABI_BRIDGE_CALL_GET_VERSION 1
#define RAINY_ABI_BRIDGE_CALL_GET_FULLVERSION 2
#define RAINY_ABI_BRIDGE_CALL_GET_COMPILE_STANDARD 3
#define RAINY_ABI_BRIDGE_CALL_GET_COMPILE_IDENTIFIER 4
#define RAINY_ABI_BRIDGE_CALL_GET_VERSION_NAME 5

// 指定MuZiYan文档生成器的宏，用于适配MuZiYan的文档生成
#ifdef __MUZIYAN_IS_HERE__
#define RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON 1
#else
#define RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON 0
#endif

#if RAINY_HAS_CXX26
#include <meta>
#endif

namespace rainy::core {
    constexpr bool is_rainy_enable_debug = RAINY_ENABLE_DEBUG;

#if __cpp_exceptions
    constexpr bool is_rainy_enable_exception = true;
#else
    constexpr bool is_rainy_enable_exception = false;
#endif
}

namespace rainy::core {
    using errno_t = int;

#if RAINY_USING_WINDOWS
    using native_char = wchar_t;
#else
    using native_char = char;
#endif
}

namespace rainy::type_traits::other_trans {
    /**
     * @brief Conditional type instantiation for SFINAE overload resolution.
     *        The nested typedef Condition exists (and is a synonym for true)
     *        if and only if enable_if_t<test, Ty> is Type.
     *
     *        有条件地为 SFINAE 重载决策设置类型的实例。
     *        当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     *        Condition 才存在（并且是 true 的同义词）。
     *
     * @tparam Test Value that determines whether the resulting type exists
     *         确定存在产生的类型的值
     *
     * @tparam Ty
               Type to instantiate when test is true
     *         test 为 true 时要实例化的类型
     *
     * @remark If test is true, enable_if_t<test, Ty> results in a typedef
     *         (which is a synonym for Ty). If test is false,
     *         enable_if_t<test, Ty> does not have a nested typedef named "type".
     *
     *         如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef
     *         （它是 Ty 的同义词）。如果 test 为 false，则 enable_if_t<test, Ty>
     *         结果不会拥有名为"type"的嵌套 typedef
     *
     * @note All constructors and assignment operators are deleted as this is
     *       a pure type trait struct intended for compile-time use only.
     *
     *       所有构造函数和赋值运算符都被删除，因为这是一个纯类型特性结构，
     *       仅用于编译时使用。
     */
    template <bool Test, typename Ty>
    struct enable_if {
        enable_if() = delete;
        enable_if(const enable_if &) = delete;
        enable_if(enable_if &&) = delete;
        enable_if &operator=(const enable_if &) = delete;
        enable_if &operator=(enable_if &&) = delete;
    };

    /**
     * @brief Specialization of enable_if for the true case.
     *        enable_if 在 true 情况下的特化。
     *
     * @tparam Ty Type to be aliased when Test is true
     *            当 Test 为 true 时要别名的类型
     */
    template <typename Ty>
    struct enable_if<true, Ty> {
        using type = Ty;
    };

    /**
     * @brief Convenience alias template for enable_if.
     *        Provides a direct way to access the nested type without
     *        using typename enable_if<Test, Ty>::type.
     *
     *        enable_if 的便捷别名模板。
     *        提供直接访问嵌套类型的方式，无需使用 typename enable_if<Test, Ty>::type。
     *
     * @tparam Test Value that determines whether the resulting type exists
     *              确定存在产生的类型的值
     *
     * @tparam Ty Type to instantiate when test is true (defaults to void)
     *            test 为 true 时要实例化的类型（默认为 void）
     *
     * @remark If test is true, this alias resolves to Ty. If test is false,
     *         substitution fails (SFINAE), making it useful for template constraints.
     *
     *         如果 test 为 true，此别名解析为 Ty。如果 test 为 false，
     *         替换失败（SFINAE），使其适用于模板约束。
     */
    template <bool Test, typename Ty = void>
    using enable_if_t = typename enable_if<Test, Ty>::type; // NOLINT
    /**
     * @brief Conditional type selection based on a boolean constant.
     *        If Test is true, the type is IfTrue; otherwise, it is IfFalse.
     *
     *        基于布尔常量进行条件类型选择。
     *        如果 Test 为 true，则类型为 IfTrue；否则为 IfFalse。
     *
     * @tparam Test Boolean value determining which type to select
     *               决定选择哪个类型的布尔值
     * @tparam IfTrue Type to use when Test is true
     *                Test 为 true 时使用的类型
     * @tparam IfFalse Type to use when Test is false
     *                 Test 为 false 时使用的类型
     */
    template <bool, typename IfTrue, typename>
    struct conditional {
        using type = IfTrue;
    };

    /**
     * @brief Specialization of conditional for the false case.
     *        conditional 在 false 情况下的特化。
     *
     * @tparam IfTrue Type to use when Test is true (unused in this specialization)
     *                Test 为 true 时使用的类型（在此特化中未使用）
     * @tparam IfFalse Type to use when Test is false
     *                 Test 为 false 时使用的类型
     */
    template <typename IfTrue, typename IfFalse>
    struct conditional<false, IfTrue, IfFalse> {
        using type = IfFalse;
    };

    /**
     * @brief Convenience alias template for conditional.
     *        Provides direct access to the nested type.
     *
     *        conditional 的便捷别名模板。
     *        提供对嵌套类型的直接访问。
     *
     * @tparam Test Boolean value determining which type to select
     *               决定选择哪个类型的布尔值
     * @tparam IfTrue Type to use when Test is true
     *                Test 为 true 时使用的类型
     * @tparam IfFalse Type to use when Test is false
     *                 Test 为 false 时使用的类型
     */
    template <bool Test, typename IfTrue, typename IfFalse>
    using conditional_t = typename conditional<Test, IfTrue, IfFalse>::type;

    /**
     * @brief Utility metafunction that maps any sequence of types to void.
     *        Used for SFINAE detection idioms.
     *
     *        将任意类型序列映射到 void 的工具元函数。
     *        用于 SFINAE 检测惯用法。
     *
     * @tparam ... Any types (unused)
     *             任意类型（未使用）
     */
    template <typename...>
    using void_t = void;

    /**
     * @brief Type selector based on a boolean value.
     *        Selects between two types without introducing a nested type.
     *
     *        基于布尔值的类型选择器。
     *        在两个类型之间进行选择，不引入嵌套类型。
     *
     * @tparam Test Boolean value controlling the selection
     *               控制选择的布尔值
     */
    template <bool>
    struct select {
        /**
         * @brief Applies selection by returning the first type.
         *        通过返回第一个类型来应用选择。
         *
         * @tparam Ty1 Type to return
         *             要返回的类型
         * @tparam Ty2 Type to ignore
         *             要忽略的类型
         */
        template <typename Ty1, typename>
        using apply = Ty1;
    };

    /**
     * @brief Specialization of select for the false case.
     *        select 在 false 情况下的特化。
     */
    template <>
    struct select<false> {
        /**
         * @brief Applies selection by returning the second type.
         *        通过返回第二个类型来应用选择。
         *
         * @tparam Ty1 Type to ignore
         *             要忽略的类型
         * @tparam Ty2 Type to return
         *             要返回的类型
         */
        template <typename, typename Ty2>
        using apply = Ty2;
    };

    /**
     * @brief Dummy type placeholder for template metaprogramming.
     *        用于模板元编程的哑元类型占位符。
     */
    struct dummy_t {};

    /**
     * @brief Pointer type transformation.
     *        Converts a type to a pointer to that type.
     *
     *        指针类型转换。
     *        将类型转换为指向该类型的指针。
     *
     * @tparam Ty Type to convert to pointer
     *            要转换为指针的类型
     */
    template <typename Ty>
    using pointer = Ty *;

    /**
     * @brief Conditionally adds const qualifier to a type.
     *        If IsConst is true, yields const Ty; otherwise yields Ty.
     *
     *        有条件地向类型添加 const 限定符。
     *        如果 IsConst 为 true，则生成 const Ty；否则生成 Ty。
     *
     * @tparam IsConst Boolean flag for const qualification
     *                  是否添加 const 限定的布尔标志
     * @tparam Ty Type to potentially add const to
     *            可能添加 const 的类型
     */
    template <bool IsConst, typename Ty>
    struct maybe_const {
        using type = conditional_t<IsConst, const Ty, Ty>;
    };

    /**
     * @brief Convenience alias template for maybe_const.
     *        Provides direct access to the nested type.
     *
     *        maybe_const 的便捷别名模板。
     *        提供对嵌套类型的直接访问。
     *
     * @tparam IsConst Boolean flag for const qualification
     *                  是否添加 const 限定的布尔标志
     * @tparam Ty Type to potentially add const to
     *            可能添加 const 的类型
     */
    template <bool IsConst, typename Ty>
    using maybe_const_t = typename maybe_const<IsConst, Ty>::type;
}

namespace rainy::type_traits::implements {
    template <typename Ty>
    struct remove_reference {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_reference<Ty &> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_reference<Ty &&> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_reference_t = typename remove_reference<Ty>::type;

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_lvalue_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_lvalue_reference_v<Ty &> = true;
}

namespace rainy::core::builtin {
#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
    RAINY_TOOLKIT_API std::int32_t ctz_avx2(std::uint32_t x) noexcept;
#endif
    constexpr rain_fn compare_memory(const void *mem1, const void *mem2, const std::size_t count) -> int {
        return __builtin_memcmp(mem1, mem2, count);
    }

    constexpr rain_fn compare_wmemory(const wchar_t *mem1, const wchar_t *mem2, const std::size_t count) -> int {
#if RAINY_USING_GCC
        for (std::size_t i = 0; i < count; ++i) {
            if (mem1[i] != mem2[i]) {
                return (mem1[i] > mem2[i]) ? 1 : -1;
            }
        }
        return 0;
#else
        return __builtin_wmemcmp(mem1, mem2, count);
#endif
    }

    RAINY_INLINE rain_fn copy_memory(void *dest, const void *src, std::size_t len) -> void * {
        return std::memcpy(dest, src, len);
    }

    RAINY_TOOLKIT_API rain_fn move_memory(void *dest, std::size_t dest_size, const void *src, std::size_t src_count) -> void *;
    RAINY_TOOLKIT_API rain_fn set_memory(void *dest, std::size_t count, int new_val) -> void *;
    RAINY_TOOLKIT_API rain_fn zero_memory(void *dest, std::size_t count) -> void *;
    RAINY_TOOLKIT_API rain_fn fill_memory(void *dest, std::size_t count, int new_val) -> void *;
    RAINY_TOOLKIT_API rain_fn fill_memory(void *dest, std::size_t count, const void *src) -> void *;
    RAINY_TOOLKIT_API rain_fn fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count) -> void *;
    RAINY_TOOLKIT_API rain_fn fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count,
                                          std::size_t src_offset) -> void *;
    RAINY_TOOLKIT_API rain_fn fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count,
                                          std::size_t src_offset, std::size_t dest_offset) -> void *;

    /**
     * @brief 使用完美转发（perfect forwarding）实现类型安全的引用转发。
     *
     * @tparam Ty 转发对象的类型。
     * @param arg 要转发的左值引用对象。
     * @return 返回类型为 `Ty&&` 的转发对象。
     *
     * @remark
     * 这个函数用于将左值引用安全地转发为相应类型的引用（可能是左值引用或右值引用），
     * 以保留传入参数的左值或右值性质。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::implements::remove_reference_t<Ty> &arg) noexcept {
        return static_cast<Ty &&>(arg);
    }

    /**
     * @brief 使用完美转发（perfect forwarding）实现类型安全的引用转发。
     *
     * @tparam Ty 转发对象的类型。
     * @param arg 要转发的右值引用对象。
     * @return 返回类型为 `Ty&&` 的转发对象。
     *
     * @remark
     * 这个函数用于将右值引用安全地转发为相应类型的引用（可能是左值引用或右值引用），
     * 以保留传入参数的左值或右值性质。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::implements::remove_reference_t<Ty> &&arg) noexcept { // NOLINT
        static_assert(!type_traits::implements::_is_lvalue_reference_v<Ty>, "bad forward call");
        return static_cast<Ty &&>(arg);
    }

    /**
     * @brief 获取变量的实际地址
     *
     * 即使变量重载了 addressof 操作符，也能获取其真实地址。
     *
     * @tparam Ty 变量类型
     * @param val 要获取地址的变量
     * @return Ty* 变量的地址
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn addressof(Ty &val) noexcept -> Ty * {
        return __builtin_addressof(val);
    }

    /**
     * @brief 禁止获取临时对象的地址（删除的重载）
     *
     * @tparam Ty 临时对象类型
     */
    template <typename Ty>
    rain_fn addressof(const Ty &&) -> const Ty * = delete;

    /**
     * @brief 在指定内存位置构造对象
     *
     * 使用提供的参数在未初始化的内存位置构造类型为 Ty 的对象。
     *
     * @tparam Ty 要构造的对象类型
     * @tparam Args 构造参数类型
     * @param location 要构造对象的内存位置指针
     * @param args 构造参数
     * @return Ty* 构造完成的对象指针，如果 location 为空则返回 nullptr
     */
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 rain_fn construct_at(Ty *location, Args &&...args) noexcept(noexcept(::new (static_cast<void *>(location))
                                                                                               Ty(builtin::forward<Args>(args)...)))
        -> Ty * {
        if (!location) {
            return nullptr;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return std::construct_at(location, builtin::forward<Args>(args)...);
        }
#endif
        return ::new (static_cast<void *>(location)) Ty(builtin::forward<Args>(args)...);
    }

    /**
     * @brief 计算 C 风格字符串的长度
     *
     * @param str 以空字符结尾的窄字符字符串
     * @return std::size_t 字符串长度（不包括结尾的空字符）
     */
    constexpr rain_fn string_length(const char *str) -> std::size_t {
        return __builtin_strlen(str);
    }

    /**
     * @brief 计算 C 风格宽字符串的长度
     *
     * @param wstr 以空字符结尾的宽字符字符串
     * @return std::size_t 字符串长度（不包括结尾的空字符）
     */
    constexpr rain_fn string_length(const wchar_t *wstr) -> std::size_t {
#if RAINY_USING_GCC
        std::size_t i = 0;
        while (wstr[i])
            ++i; // NOLINT
        return i;
#else
        return __builtin_wcslen(wstr);
#endif
    }

    /**
     * @brief 比较两个内存区域的前 count 个字节（窄字符版本）
     *
     * @param string1 指向第一个内存块的指针
     * @param string2 指向第二个内存块的指针
     * @param count 要比较的字节数
     * @return int 如果相同返回 0，如果 string1 小于 string2 返回负数，否则返回正数
     */
    constexpr rain_fn compare_string(const void *string1, const void *string2, const std::size_t count) noexcept -> int {
        return compare_memory(string1, string2, count);
    }

    /**
     * @brief 比较两个内存区域的前 count 个字节（宽字符版本）
     *
     * @param string1 指向第一个宽字符字符串的指针
     * @param string2 指向第二个宽字符字符串的指针
     * @param count 要比较的宽字符数
     * @return int 如果相同返回 0，如果 string1 小于 string2 返回负数，否则返回正数
     */
    constexpr rain_fn compare_string(const wchar_t *string1, const wchar_t *string2, const std::size_t count) noexcept -> int {
        return compare_wmemory(string1, string2, count);
    }

    template <typename Ty>
    RAINY_NORETURN RAINY_NOINLINE void throw_exception_builtin(const Ty &exception) {
        throw exception;
    }

    // NOLINTBEGIN
    template <template <typename U, U...> typename Struct, typename Ty, Ty N>
    struct make_integer_seq {
    private:
        template <typename Seq1, typename Seq2, Ty Offset>
        struct concat_impl;

        template <Ty... Is1, Ty... Is2, Ty Offset>
        struct concat_impl<Struct<Ty, Is1...>, Struct<Ty, Is2...>, Offset> {
            using type = Struct<Ty, Is1..., (Offset + Is2)...>;
        };

        template <Ty Count, typename = void>
        struct gen {
            static constexpr Ty Half = Count / 2;
            using half_seq = typename gen<Half>::type;
            using type = typename concat_impl<half_seq, half_seq, Half>::type;
        };

        template <Ty Count>
        struct gen<Count, type_traits::other_trans::enable_if_t<(Count > 1) && (Count % 2 == 1)>> {
            using prev_seq = typename gen<Count - 1>::type;
            using type = typename concat_impl<prev_seq, Struct<Ty, 0>, Count - 1>::type;
        };

        template <Ty Count>
        struct gen<Count, type_traits::other_trans::enable_if_t<Count == 0>> {
            using type = Struct<Ty>;
        };

        template <Ty Count>
        struct gen<Count, type_traits::other_trans::enable_if_t<Count == 1>> {
            using type = Struct<Ty, 0>;
        };

    public:
        using type = typename gen<N>::type;
    };
    // NOLINTEND

    template <typename Type>
    void zero_non_value_bits(Type *ptr) noexcept {
        __builtin_zero_non_value_bits(ptr);
    }
}

namespace rainy::core::builtin {
    int multibyte_to_wchar(const char *source, wchar_t *buffer, std::size_t buffer_length, std::size_t codepage);
    int wchar_to_multibyte(const wchar_t *source, char *buffer, std::size_t buffer_length, std::size_t codepage);
}

namespace rainy::core {
    enum class instruction_set {
        sse3,
        pclmulqdq,
        monitor,
        ssse3,
        fma,
        cmpxchg16b,
        sse41,
        sse42,
        movbe,
        popcnt,
        aes,
        xsave,
        osxsave,
        avx,
        f16c,
        rdrand,
        msr,
        cx8,
        sep,
        cmov,
        clflush,
        mmx,
        fxsr,
        sse,
        sse2,
        fsgsbase,
        bmi1,
        hle,
        avx2,
        bmi2,
        erms,
        invpcid,
        rtm,
        avx512f,
        rdseed,
        adx,
        avx512pf,
        avx512er,
        avx512cd,
        sha,
        prefetchwt1,
        lahf,
        lzcnt,
        abm,
        sse4a,
        xop,
        tbm,
        syscall,
        mmxext,
        rdtscp,
        _3dnowext,
        _3dnow,
        hypervisor
    };
}

namespace rainy::core::builtin {
    enum class version {
        windows11,
        windows10,
        windows8_1,
        windows8,
        windows7sp1,
        windows7,
        windows_server,
        linux_like,
        unknown
    };

    RAINY_TOOLKIT_API void cpuid(int query[4], int function_id);
    RAINY_TOOLKIT_API void cpuidex(int query[4], int function_id, int subfunction_id);
    RAINY_TOOLKIT_API bool is_hypervisor();
    RAINY_TOOLKIT_API errno_t get_machine_code(); // for future, not implemented
    RAINY_TOOLKIT_API bool has_instruction(instruction_set check);
    RAINY_TOOLKIT_API errno_t get_vendor(char *buffer, std::size_t length);
    RAINY_TOOLKIT_API errno_t get_brand(char *buffer, std::size_t length);
    RAINY_TOOLKIT_API std::size_t hardware_concurrency();
    RAINY_TOOLKIT_API version get_os_version();
    RAINY_TOOLKIT_API errno_t get_os_name(char *buffer);
    RAINY_TOOLKIT_API errno_t get_arch(char *buffer);
}

namespace rainy::core {
    /**
     * @brief Signed integer type for I/O operations.
     *        Alias of std::intptr_t for I/O size representation.
     *
     *        用于 I/O 操作的有符号整数类型。
     *        std::intptr_t 的别名，用于 I/O 大小表示。
     */
    using io_size_t = std::intptr_t;

    /**
     * @brief Constant C-style string type.
     *        Alias for const char*.
     *
     *        常量 C 风格字符串类型。
     *        const char* 的别名。
     */
    using czstring = const char *;

    /**
     * @brief Mutable C-style string type.
     *        Alias for char*.
     *
     *        可变 C 风格字符串类型。
     *        char* 的别名。
     */
    using cstring = char *;

    /**
     * @brief Native frame pointer type.
     *        Opaque pointer type for frame handling.
     *
     *        原生框架指针类型。
     *        用于框架处理的不透明指针类型。
     */
    using native_frame_ptr_t = void *;

    /**
     * @brief Constant native C-style string type.
     *        常量原生 C 风格字符串类型。
     *
     * Alias for const native_char*, representing a constant C-style string in native character encoding.
     * const native_char* 的别名，表示原生字符编码中的常量 C 风格字符串。
     */
    using native_czstring = const native_char *;

    /**
     * @brief Mutable native C-style string type.
     *        可变原生 C 风格字符串类型。
     *
     * Alias for native_char*, representing a mutable C-style string in native character encoding.
     * native_char* 的别名，表示原生字符编码中的可变 C 风格字符串。
     */
    using native_cstring = native_char *;

    /**
     * @brief Number of pointer-sized objects for small object optimization.
     *        Calculated as 6 plus space for 16 bytes divided by pointer size.
     *
     *        用于小对象优化的指针大小对象数量。
     *        计算方式为 6 加上 16 字节除以指针大小。
     */
    static inline constexpr std::size_t small_object_num_ptrs = 6 + 16 / sizeof(void *);

    /**
     * @brief Total space available for small object optimization.
     *        Derived from small_object_num_ptrs.
     *
     *        可用于小对象优化的总空间。
     *        由 small_object_num_ptrs 计算得出。
     */
    static inline constexpr std::size_t small_object_space_size = (small_object_num_ptrs - 1) * sizeof(void *);

    /**
     * @brief Byte type for raw memory operations.
     *        Unsigned char alias for byte-level access.
     *
     *        用于原始内存操作的字节类型。
     *        用于字节级访问的 unsigned char 别名。
     */
    using byte_t = unsigned char;

    /**
     * @brief Handle type for resource management.
     *        Unsigned integer type capable of holding a pointer.
     *
     *        用于资源管理的句柄类型。
     *        能够存储指针的无符号整数类型。
     */
    using handle = std::uintptr_t;

    /**
     * @brief Signed size type.
     *        Alias for std::intptr_t for signed size operations.
     *
     *        有符号大小类型。
     *        用于有符号大小操作的 std::intptr_t 别名。
     */
    using ssize_t = std::intptr_t;

    /**
     * @brief Invalid handle value constant.
     *        Used to represent an invalid or uninitialized handle.
     *
     *        无效句柄值常量。
     *        用于表示无效或未初始化的句柄。
     */
    static inline constexpr handle invalid_handle = 0;
}

namespace rainy::utility {
    /**
     * @brief Placeholder type for invalid or dummy operations.
     *        Used in template metaprogramming to represent invalid states.
     *
     *        用于无效或虚拟操作的占位符类型。
     *        在模板元编程中用于表示无效状态。
     */
    struct invalid_type {};
}

namespace rainy::type_traits::helper {
    /**
     * @brief Compile-time constant string implementation.
     *        Provides string operations that can be evaluated at compile time.
     *
     *        编译时常量字符串实现。
     *        提供可在编译时评估的字符串操作。
     *
     * @tparam CharType Character type of the string (char, wchar_t, etc.)
     *                  字符串的字符类型（char、wchar_t 等）
     * @tparam N Fixed size of the string buffer including null terminator
     *           字符串缓冲区的固定大小，包括空终止符
     */
    template <typename CharType, std::size_t N>
    struct basic_constexpr_string {
        using size_type = std::size_t;
        using value_type = CharType;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;

        /**
         * @brief Default constructor.
         *        Initializes an empty string with null terminator.
         *
         *        默认构造函数。
         *        使用空终止符初始化空字符串。
         */
        constexpr basic_constexpr_string() noexcept = default;

        /**
         * @brief Construct from std::string_view.
         *        Copies up to N-1 characters from the string view.
         *
         *        从 std::string_view 构造。
         *        从字符串视图复制最多 N-1 个字符。
         *
         * @param str_view Source string view to copy from
         *                 要复制的源字符串视图
         */
        constexpr basic_constexpr_string(std::string_view str_view) { // NOLINT
            std::size_t len_to_copy = str_view.length();
            if (len_to_copy >= N) {
                len_to_copy = N - 1;
            }
            for (std::size_t i = 0; i < len_to_copy; ++i) {
                string[i] = str_view[i]; // NOLINT
            }
            string[len_to_copy] = '\0';
        }

        /**
         * @brief Construct from C-style array.
         *        Copies the entire array including null terminator.
         *
         *        从 C 风格数组构造。
         *        复制整个数组，包括空终止符。
         *
         * @param arr Source character array
         *            源字符数组
         */
        constexpr basic_constexpr_string(const value_type (&arr)[N]) {
            for (std::size_t i = 0; i < N - 1; ++i) {
                string[i] = arr[i];
            }
            string[N - 1] = '\0';
        }

        /**
         * @brief Get the fixed size of the string buffer.
         *
         *        获取字符串缓冲区的固定大小。
         *
         * @return Fixed buffer size including null terminator
         *         包含空终止符的固定缓冲区大小
         */
        constexpr rain_fn size() const noexcept -> size_type {
            return N;
        }

        /**
         * @brief Get the actual length of the string (up to first null terminator).
         *
         *        获取字符串的实际长度（直到第一个空终止符）。
         *
         * @return Current string length
         *         当前字符串长度
         */
        constexpr rain_fn length() const noexcept -> size_type {
            size_type len = 0;
            while (len < N && string[len] != '\0') {
                len++;
            }
            return len;
        }

        /**
         * @brief Get mutable pointer to internal buffer.
         *
         *        获取指向内部缓冲区的可变指针。
         *
         * @return Pointer to first element
         *         指向第一个元素的指针
         */
        constexpr rain_fn data() noexcept -> pointer {
            return string;
        }

        /**
         * @brief Get const pointer to internal buffer.
         *
         *        获取指向内部缓冲区的常量指针。
         *
         * @return Const pointer to first element
         *         指向第一个元素的常量指针
         */
        constexpr rain_fn data() const noexcept -> const_pointer {
            return string;
        }

        /**
         * @brief Mutable element access.
         *
         *        可变元素访问。
         *
         * @param idx Index to access
         *            要访问的索引
         * @return Reference to character at specified index
         *         指定索引处字符的引用
         */
        constexpr rain_fn operator[](size_type idx) noexcept -> reference {
            return string[idx];
        }

        /**
         * @brief Const element access.
         *
         *        常量元素访问。
         *
         * @param idx Index to access
         *            要访问的索引
         * @return Const reference to character at specified index
         *         指定索引处字符的常量引用
         */
        constexpr rain_fn operator[](size_type idx) const noexcept -> const_reference {
            return string[idx];
        }

        CharType string[N]{};
    };

    /**
     * @brief Equality comparison between two constexpr strings.
     *
     *        两个 constexpr 字符串之间的相等比较。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam N1 Size of first string
     *            第一个字符串的大小
     * @tparam N2 Size of second string
     *            第二个字符串的大小
     * @param lhs Left-hand side string
     *            左侧字符串
     * @param rhs Right-hand side string
     *            右侧字符串
     * @return true if strings are equal, false otherwise
     *         如果字符串相等则为 true，否则为 false
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator==(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) ==
               std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * @brief Inequality comparison between two constexpr strings.
     *
     *        两个 constexpr 字符串之间的不相等比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator!=(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return !(lhs == rhs);
    }

    /**
     * @brief Less-than comparison between two constexpr strings.
     *
     *        两个 constexpr 字符串之间的小于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator<(const basic_constexpr_string<CharType, N1> &lhs,
                                const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) < std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * @brief Less-than-or-equal comparison between two constexpr strings.
     *
     *        两个 constexpr 字符串之间的小于等于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator<=(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return !(rhs < lhs);
    }

    /**
     * @brief Greater-than comparison between two constexpr strings.
     *
     *        两个 constexpr 字符串之间的大于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator>(const basic_constexpr_string<CharType, N1> &lhs,
                                const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return rhs < lhs;
    }

    /**
     * @brief Greater-than-or-equal comparison between two constexpr strings.
     *
     *        两个 constexpr 字符串之间的大于等于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator>=(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return !(lhs < rhs);
    }

    /**
     * @brief Equality comparison between constexpr string and string_view.
     *
     *        constexpr 字符串与 string_view 之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(const basic_constexpr_string<CharType, N> &lhs, std::basic_string_view<CharType> rhs) noexcept
        -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) == rhs;
    }

    /**
     * @brief Equality comparison between string_view and constexpr string.
     *
     *        string_view 与 constexpr 字符串之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(std::basic_string_view<CharType> lhs, const basic_constexpr_string<CharType, N> &rhs) noexcept
        -> bool {
        return lhs == std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * @brief Equality comparison between constexpr string and C-style string.
     *
     *        constexpr 字符串与 C 风格字符串之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(const basic_constexpr_string<CharType, N> &lhs, const CharType *rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) == std::basic_string_view<CharType>(rhs);
    }

    /**
     * @brief Equality comparison between C-style string and constexpr string.
     *
     *        C 风格字符串与 constexpr 字符串之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(const CharType *lhs, const basic_constexpr_string<CharType, N> &rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs) == std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * @brief Alias for char-based constexpr string.
     *
     *        基于 char 的 constexpr 字符串别名。
     */
    template <std::size_t N>
    using constexpr_string = basic_constexpr_string<char, N>;

    /**
     * @brief Factory function to create constexpr string from array.
     *
     *        从数组创建 constexpr 字符串的工厂函数。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam N Array size including null terminator
     *           包含空终止符的数组大小
     * @param str Source character array
     *            源字符数组
     * @return basic_constexpr_string instance
     *         basic_constexpr_string 实例
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn make_constexpr_string(const CharType (&str)[N]) -> auto {
        return basic_constexpr_string<CharType, N>(str);
    }

    /**
     * @brief Deduction guide for basic_constexpr_string from array.
     *
     *        从数组构造 basic_constexpr_string 的推导指引。
     */
    template <typename CharType, std::size_t N>
    basic_constexpr_string(const CharType (&)[N]) -> basic_constexpr_string<CharType, N>;

    /**
     * @brief Concatenate two constexpr strings at compile time.
     *
     *        在编译时连接两个 constexpr 字符串。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam N1 Size of first string
     *            第一个字符串的大小
     * @tparam N2 Size of second string
     *            第二个字符串的大小
     * @param lhs First string
     *            第一个字符串
     * @param rhs Second string
     *            第二个字符串
     * @return Concatenated string of size N1 + N2 - 1
     *         大小为 N1 + N2 - 1 的连接后字符串
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn concat(const basic_constexpr_string<CharType, N1> &lhs, const basic_constexpr_string<CharType, N2> &rhs)
        -> auto {
        basic_constexpr_string<CharType, N1 + N2 - 1> result{};
        std::size_t current_result_idx = 0;
        for (std::size_t i = 0; i < lhs.length(); ++i) {
            result[current_result_idx++] = lhs[i];
        }
        for (std::size_t i = 0; i < rhs.length(); ++i) {
            result[current_result_idx++] = rhs[i];
        }
        result[current_result_idx] = '\0';
        return result;
    }
}

namespace rainy::core {
    /**
     * @brief Finds the largest element in a range.
     *        查找范围中的最大元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param end Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @return Iterator to the largest element, or end if range is empty
     *         指向最大元素的迭代器，如果范围为空则返回end
     */
    template <typename Iter>
    RAINY_CONSTEXPR rain_fn max_element(Iter first, Iter end) -> Iter {
        if (first == end) {
            return end;
        }
        Iter largest = first;
        while (++first != end) {
            if (*largest < *first) {
                largest = first;
            }
        }
        return largest;
    }

    /**
     * @brief Finds the largest element in a range using a custom comparison.
     *        使用自定义比较查找范围中的最大元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Binary predicate type (bool pred(const T&, const T&))
     *              二元谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param end Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Comparison function object that returns true if the first argument is less than the second
     *             比较函数对象，如果第一个参数小于第二个参数则返回true
     * @return Iterator to the largest element, or end if range is empty
     *         指向最大元素的迭代器，如果范围为空则返回end
     */
    template <typename Iter, typename Pred>
    RAINY_CONSTEXPR rain_fn max_element(Iter first, Iter end, Pred pred) -> Iter {
        if (first == end) {
            return end;
        }
        Iter largest = first;
        while (++first != end) {
            if (pred(*largest, *first)) {
                largest = first;
            }
        }
        return largest;
    }

    /**
     * @brief Finds the smallest element in a range.
     *        查找范围中的最小元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param end Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @return Iterator to the smallest element, or end if range is empty
     *         指向最小元素的迭代器，如果范围为空则返回end
     */
    template <typename Iter>
    RAINY_CONSTEXPR rain_fn min_element(Iter first, Iter end) -> Iter {
        if (first == end) {
            return end;
        }
        Iter smallest = first;
        while (++first != end) {
            if (*first < *smallest) {
                smallest = first;
            }
        }
        return smallest;
    }

    /**
     * @brief Finds the smallest element in a range using a custom comparison.
     *        使用自定义比较查找范围中的最小元素。
     *
     * @tparam Iter Iterator type
     *              迭代器类型
     * @tparam Pred Binary predicate type (bool pred(const T&, const T&))
     *              二元谓词类型
     * @param first Iterator to the beginning of the range
     *              指向范围起始的迭代器
     * @param end Iterator to the end of the range
     *            指向范围末尾的迭代器
     * @param pred Comparison function object that returns true if the first argument is less than the second
     *             比较函数对象，如果第一个参数小于第二个参数则返回true
     * @return Iterator to the smallest element, or end if range is empty
     *         指向最小元素的迭代器，如果范围为空则返回end
     */
    template <typename Iter, typename Pred>
    RAINY_CONSTEXPR rain_fn min_element(Iter first, Iter end, Pred pred) -> Iter {
        if (first == end) {
            return end;
        }
        Iter smallest = first;
        while (++first != end) {
            if (pred(*first, *smallest)) {
                smallest = first;
            }
        }
        return smallest;
    }

    /**
     * @brief Returns the larger of two values.
     *        返回两个值中的较大者。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @param a First value
     *          第一个值
     * @param b Second value
     *          第二个值
     * @return const reference to the larger value
     *         较大值的常量引用
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(max)(const Ty &a, const Ty &b) -> const Ty & {
        return (a < b) ? b : a; // NOLINT
    }

    /**
     * @brief Returns the larger of two values using a custom comparison.
     *        使用自定义比较返回两个值中的较大者。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @tparam Pred Binary predicate type
     *              二元谓词类型
     * @param a First value
     *          第一个值
     * @param b Second value
     *          第二个值
     * @param comp Comparison function object that returns true if a is less than b
     *             比较函数对象，如果 a 小于 b 则返回 true
     * @return const reference to the larger value
     *         较大值的常量引用
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(max)(const Ty &a, const Ty &b, Pred comp) -> const Ty & {
        return (comp(a, b)) ? b : a; // NOLINT
    }

    /**
     * @brief Returns the largest value in an initializer list.
     *        返回初始化列表中的最大值。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @param ilist Initializer list of values
     *              值的初始化列表
     * @return The largest value
     *         最大值
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(max)(std::initializer_list<Ty> ilist) -> Ty {
        return *(max_element(ilist.begin(), ilist.end()));
    }

    /**
     * @brief Returns the largest value in an initializer list using a custom comparison.
     *        使用自定义比较返回初始化列表中的最大值。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @tparam Pred Binary predicate type
     *              二元谓词类型
     * @param ilist Initializer list of values
     *              值的初始化列表
     * @param pred Comparison function object
     *             比较函数对象
     * @return The largest value
     *         最大值
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(max)(std::initializer_list<Ty> ilist, Pred pred) -> Ty {
        return *(max_element(ilist.begin(), ilist.end(), pred));
    }

    /**
     * @brief Returns the smaller of two values.
     *        返回两个值中的较小者。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @param a First value
     *          第一个值
     * @param b Second value
     *          第二个值
     * @return const reference to the smaller value
     *         较小值的常量引用
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(min)(const Ty &a, const Ty &b) -> const Ty & {
        return (a < b) ? a : b;
    }

    /**
     * @brief Returns the smaller of two values using a custom comparison.
     *        使用自定义比较返回两个值中的较小者。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @tparam Pred Binary predicate type
     *              二元谓词类型
     * @param a First value
     *          第一个值
     * @param b Second value
     *          第二个值
     * @param comp Comparison function object that returns true if a is less than b
     *             比较函数对象，如果 a 小于 b 则返回 true
     * @return const reference to the smaller value
     *         较小值的常量引用
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(min)(const Ty &a, const Ty &b, Pred comp) -> const Ty & {
        return (comp(a, b)) ? a : b;
    }

    /**
     * @brief Returns the smallest value in an initializer list.
     *        返回初始化列表中的最小值。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @param ilist Initializer list of values
     *              值的初始化列表
     * @return The smallest value
     *         最小值
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(min)(std::initializer_list<Ty> ilist) -> Ty {
        return *(min_element(ilist.begin(), ilist.end()));
    }

    /**
     * @brief Returns the smallest value in an initializer list using a custom comparison.
     *        使用自定义比较返回初始化列表中的最小值。
     *
     * @tparam Ty Type of the values
     *            值的类型
     * @tparam Pred Binary predicate type
     *              二元谓词类型
     * @param ilist Initializer list of values
     *              值的初始化列表
     * @param pred Comparison function object
     *             比较函数对象
     * @return The smallest value
     *         最小值
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(min)(std::initializer_list<Ty> ilist, Pred pred) -> Ty {
        return *(min_element(ilist.begin(), ilist.end(), pred));
    }
}

namespace rainy::core::builtin {
    /**
     * @brief Compares two double values for approximate equality.
     *        比较两个 double 值是否近似相等。
     *
     * @param p1 First double value
     *           第一个 double 值
     * @param p2 Second double value
     *           第二个 double 值
     * @return true if the values are approximately equal, false otherwise
     *         如果值近似相等则返回 true，否则返回 false
     */
    static RAINY_INLINE rain_fn almost_equal(double p1, double p2) -> bool {
        return (std::abs(p1 - p2) * 1000000000000. <= (core::min) (std::abs(p1), std::abs(p2)));
    }
}

namespace rainy::core {
    /**
     * @brief Buffer size for function object small object optimization.
     *        For 32-bit systems, extra space is allocated to prevent stack-overrun
     *        issues caused by memory alignment.
     *
     *        函数对象小对象优化的缓冲区大小。
     *        对于32位系统，额外分配空间以防止内存对齐导致的栈溢出问题。
     */
    static constexpr inline std::size_t fn_obj_soo_buffer_size = (small_object_num_ptrs - 1) * sizeof(void *);
}

namespace rainy::core {
    /**
     * @brief Tag type for internal constructor dispatching.
     *        用于内部构造函数分发的标签类型。
     */
    struct internal_construct_tag_t {};

    /**
     * @brief Instance of internal_construct_tag_t for constructor tagging.
     *        internal_construct_tag_t 的实例，用于构造函数标记。
     */
    RAINY_INLINE_CONSTEXPR internal_construct_tag_t internal_construct_tag{};

    /**
     * @brief Cache line size for constructive interference.
     *        Maximum size of memory that can be accessed without causing
     *        false sharing between different cores.
     *
     *        构造性干扰的缓存行大小。
     *        在不引起不同核心间错误共享的情况下，可以访问的最大内存大小。
     */
    inline constexpr std::size_t hardware_constructive_interference_size = 64;

    /**
     * @brief Cache line size for destructive interference.
     *        Minimum size of memory separation needed to avoid false sharing
     *        between different cores.
     *
     *        破坏性干扰的缓存行大小。
     *        避免不同核心间错误共享所需的最小内存间隔大小。
     */
    inline constexpr std::size_t hardware_destructive_interference_size = 64;
}

namespace rainy::core::builtin {
    /**
     * @brief Returns the positive infinity value for double.
     *        返回 double 类型的正无穷大值。
     *
     * @return Positive infinity as double
     *         double 类型的正无穷大
     */
    constexpr rain_fn huge_val() noexcept -> double {
        return __builtin_huge_val();
    }

    /**
     * @brief Returns the positive infinity value for float.
     *        返回 float 类型的正无穷大值。
     *
     * @return Positive infinity as float
     *         float 类型的正无穷大
     */
    constexpr rain_fn huge_valf() noexcept -> float {
        return __builtin_huge_valf();
    }
}

namespace rainy::utility {
    struct auto_deduce_t {
        explicit constexpr auto_deduce_t() = default;
    };

    static constexpr auto_deduce_t auto_deduce{};
}

namespace rainy::core::abi {
    using abi_bridge_call_func_t = long (*)(std::intptr_t, std::intptr_t, std::intptr_t, std::intptr_t, std::intptr_t, std::intptr_t);

    enum class bridge_version {
        major,
        minor,
        patch
    };

    struct version {
        int major;
        int minor;
        int patch;
    };

    enum class standard {
        cxx17,
        cxx20,
        cxx23,
        cxxlatest
    };

    enum class compiler_identifier {
        msvc,
        llvm_clang,
        gcc
    };

    enum class standard_library_id {
        msvc_stl,
        libcxx,
        libstdcxx
    };

    constexpr const char rainy_toolkit_version_name[] = RAINY_TOOLKIT_VERSION;
    constexpr const char build_date[] = __DATE__;
    constexpr const char msvc_stl_name[] = "MSVC-STL package";
    constexpr const char libcxx_name[] = "libc++ package";
    constexpr const char libstdcxx_name[] = "libstdc++ package";

    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_abi_bridge_call(long number, ...);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_call_handler_total_count(long *total);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_get_version(bridge_version bridge_version, int *recv);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_get_fullversion(version *recv);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_get_compile_standard(standard *recv);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_get_compile_identifier(compiler_identifier *recv);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_get_version_name(char *buffer, std::size_t buffer_length);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_export_library_context(std::uintptr_t *context);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_is_abi_compatible(std::uintptr_t *context);
}

namespace rainy::type_traits::helper {
    /**
     * @brief 从类型和值生成整型常量
     *
     * 提供一个编译期常量包装器，将指定类型的特定值封装为一个类型。
     *
     * @tparam Ty 值的类型
     * @tparam Data 常量值
     */
    template <typename Ty, Ty Data>
    struct integral_constant {
        using value_type = Ty;
        using type = integral_constant;

        constexpr explicit operator value_type() const noexcept {
            return value;
        }
        constexpr value_type operator()() const noexcept {
            return value;
        }

        static constexpr Ty value = Data;
    };

    /**
     * @brief 布尔类型的 integral_constant 别名模板
     *
     * @tparam Boolean 布尔常量值
     */
    template <bool Boolean>
    using bool_constant = integral_constant<bool, Boolean>;

    /**
     * @brief 表示 true 值的类型
     */
    using true_type = integral_constant<bool, true>;

    /**
     * @brief 表示 false 值的类型
     */
    using false_type = integral_constant<bool, false>;

    /**
     * @brief 获取字符类型的空格字符常量（主模板）
     *
     * 默认使用 char 类型的空格字符。
     *
     * @tparam CharType 字符类型
     */
    template <typename>
    struct char_space : integral_constant<char, ' '> {};

    /**
     * @brief wchar_t 类型的空格字符常量特化
     */
    template <>
    struct char_space<wchar_t> : integral_constant<wchar_t, L' '> {};

    /**
     * @brief char16_t 类型的空格字符常量特化
     */
    template <>
    struct char_space<char16_t> : integral_constant<char16_t, u' '> {};

    /**
     * @brief char32_t 类型的空格字符常量特化
     */
    template <>
    struct char_space<char32_t> : integral_constant<char32_t, U' '> {};

    /**
     * @brief 字符类型空格字符常量的变量模板
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_space_v = char_space<CharType>::value;

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    /**
     * @brief char8_t 类型的空格字符常量特化（C++20 及更高版本）
     */
    template <>
    struct char_space<char8_t> : integral_constant<char8_t, u8' '> {};
#endif

    /**
     * @brief 获取字符类型的空字符常量（主模板）
     *
     * 默认使用 char 类型的空字符。
     *
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    struct char_null : integral_constant<char, '\0'> {};

    /**
     * @brief wchar_t 类型的空字符常量特化
     */
    template <>
    struct char_null<wchar_t> : integral_constant<wchar_t, L'\0'> {};

    /**
     * @brief char16_t 类型的空字符常量特化
     */
    template <>
    struct char_null<char16_t> : integral_constant<char16_t, u'\0'> {};

    /**
     * @brief char32_t 类型的空字符常量特化
     */
    template <>
    struct char_null<char32_t> : integral_constant<char32_t, U'\0'> {};

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    /**
     * @brief char8_t 类型的空字符常量特化（C++20 及更高版本）
     */
    template <>
    struct char_null<char8_t> : integral_constant<char8_t, u8'\0'> {};
#endif

    /**
     * @brief 字符类型空字符常量的变量模板
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_null_v = char_null<CharType>::value;

    /**
     * @brief 判断字符类型是否为宽字符 wchar_t 的变量模板（主模板）
     *
     * 主模板默认为 false。
     *
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_wchar_t = false;

    /**
     * @brief wchar_t 类型的特化
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<wchar_t> = true;

    /**
     * @brief const wchar_t 类型的特化
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<const wchar_t> = true;

    /**
     * @brief volatile wchar_t 类型的特化
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<volatile wchar_t> = true;

    /**
     * @brief const volatile wchar_t 类型的特化
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<const volatile wchar_t> = true;
}

namespace rainy::type_traits::type_relations {
    /**
     * @brief 测试两个类型是否相同
     * @tparam Ty1 检索的第一个类型
     * @tparam Ty2 检索的第二个类型
     */
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_same_v<Ty, Ty> = true;

    /**
     * @brief 测试两个类型是否相同。
     * @tparam Ty1 检索的第一个类型
     * @tparam Ty2 检索的第二个类型
     */
    template <typename Ty1, typename Ty2>
    struct is_same : helper::bool_constant<is_same_v<Ty1, Ty2>> {};

    /**
     * @brief 判断类型是否为 void 的变量模板
     *
     * 检查类型 Ty 是否为 void 类型（不包括 cv 限定版本）。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = is_same_v<Ty, void>;

    /**
     * @brief 判断类型是否为 void 的类型模板
     *
     * 继承自 bool_constant，提供 ::value 成员常量表示 Ty 是否为 void。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};
}

namespace rainy::core::builtin {
    /**
     * @brief 计算两个整数相除的向上取整结果
     *
     * 对于整数除法，返回不小于 a/b 的最小整数值。
     * 正确处理正负数的向上取整。
     *
     * @tparam Ty 整数类型
     * @param a 被除数
     * @param b 除数（不能为 0）
     * @return Ty 向上取整的除法结果
     *
     * @example
     * ceil(5, 3)   // 返回 2，因为 5/3 = 1.66... 向上取整为 2
     * ceil(-5, 3)  // 返回 -1，因为 -5/3 = -1.66... 向上取整为 -1
     * ceil(5, -3)  // 返回 -1，因为 5/(-3) = -1.66... 向上取整为 -1
     */
    template <typename Ty>
    constexpr rain_fn ceil(Ty a, Ty b) -> Ty {
        return a / b + ((a % b) != 0 && ((a > 0) == (b > 0)));
    }
}

namespace rainy::core::pal {
    enum class memory_order {
        /**
         * @brief 放松的内存顺序
         *
         * 允许最大程度的重排，适用于不需要同步的原子操作。
         */
        relaxed,

        /**
         * @brief 消费者内存顺序
         *
         * 仅对消费者可见的数据产生同步约束，适用于消费者操作。
         */
        consume,

        /**
         * @brief 获取内存顺序
         *
         * 强制对获取的数据产生同步约束，适用于获取原子值时。
         */
        acquire,

        /**
         * @brief 释放内存顺序
         *
         * 强制对释放的数据产生同步约束，适用于释放原子值时。
         */
        release,

        /**
         * @brief 获取并释放内存顺序
         *
         * 结合获取和释放的内存顺序，适用于获取并释放数据的场景。
         */
        acq_rel,

        /**
         * @brief 顺序一致性内存顺序
         *
         * 强制操作按照程序顺序执行，适用于强同步需求的原子操作。
         */
        seq_cst,

        /**
         * @brief relaxed别名，放松的内存顺序
         *
         * 允许最大程度的重排，适用于不需要同步的原子操作。
         */
        memory_order_relaxed = relaxed,

        /**
         * @brief 消费者内存顺序
         *
         * 仅对消费者可见的数据产生同步约束，适用于消费者操作。
         */
        memory_order_consume = consume,

        /**
         * @brief 获取内存顺序
         *
         * 强制对获取的数据产生同步约束，适用于获取原子值时。
         */
        memory_order_acquire = acquire,

        /**
         * @brief 释放内存顺序
         *
         * 强制对释放的数据产生同步约束，适用于释放原子值时。
         */
        memory_order_release = release,

        /**
         * @brief 获取并释放内存顺序
         *
         * 结合获取和释放的内存顺序，适用于获取并释放数据的场景。
         */
        memory_order_acq_rel = acq_rel,

        /**
         * @brief 顺序一致性内存顺序
         *
         * 强制操作按照程序顺序执行，适用于强同步需求的原子操作。
         */
        memory_order_seq_cst = seq_cst
    };

    /**
     * @brief relaxed 内存顺序常量
     *
     * 表示 relaxed 内存顺序，允许最大程度的重排。
     */
    inline constexpr memory_order memory_order_relaxed = memory_order::relaxed;

    /**
     * @brief consume 内存顺序常量
     *
     * 表示 consume 内存顺序，适用于消费者操作。
     */
    inline constexpr memory_order memory_order_consume = memory_order::consume;

    /**
     * @brief acquire 内存顺序常量
     *
     * 表示 acquire 内存顺序，适用于获取原子值时的同步约束。
     */
    inline constexpr memory_order memory_order_acquire = memory_order::acquire;

    /**
     * @brief release 内存顺序常量
     *
     * 表示 release 内存顺序，适用于释放原子值时的同步约束。
     */
    inline constexpr memory_order memory_order_release = memory_order::release;

    /**
     * @brief acq_rel 内存顺序常量
     *
     * 表示 acquire-release 内存顺序，适用于获取并释放数据的场景。
     */
    inline constexpr memory_order memory_order_acq_rel = memory_order::acq_rel;

    /**
     * @brief seq_cst 内存顺序常量
     *
     * 表示顺序一致性内存顺序，适用于强同步需求的原子操作。
     */
    inline constexpr memory_order memory_order_seq_cst = memory_order::seq_cst;
}

namespace rainy::utility {
    /**
     * @brief 分段构造标签类型
     *
     * 用于指示构造函数应使用分段方式构造 pair 或 tuple 等容器。
     * 当需要分别传递参数给 pair 或 tuple 的两个元素时使用此标签。
     */
    struct piecewise_construct_t {
        explicit piecewise_construct_t() = default;
    };

    /**
     * @brief 分段构造标签常量实例
     *
     * 用于在函数调用中传递 piecewise_construct_t 类型的参数。
     */
    inline constexpr piecewise_construct_t piecewise_construct{};
}

namespace rainy::utility {
    template <class Container>
    class back_insert_iterator {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using pointer = void;
        using reference = void;
        using difference_type = void;

        using container_type = Container;

        explicit back_insert_iterator(Container &c) : container(std::addressof(c)) {
        }

        back_insert_iterator &operator=(const typename Container::value_type &value) {
            container->push_back(value);
            return *this;
        }

        back_insert_iterator &operator=(typename Container::value_type &&value) {
            container->push_back(std::move(value));
            return *this;
        }

        back_insert_iterator &operator*() noexcept {
            return *this;
        }

        back_insert_iterator &operator++() noexcept {
            return *this;
        }

        back_insert_iterator operator++(int) noexcept {
            return *this;
        }

    protected:
        Container *container;
    };

    // 辅助函数
    template <class Container>
    back_insert_iterator<Container> back_inserter(Container &c) {
        return back_insert_iterator<Container>(c);
    }
}

namespace rainy::utility {
    template <typename FloatingType>
    struct floating_type_traits;

    template <>
    struct floating_type_traits<float> {
        static constexpr std::int32_t mantissa_bits = 24;
        static constexpr std::int32_t exponent_bits = 8;
        static constexpr std::int32_t maximum_binary_exponent = 127;
        static constexpr std::int32_t minimum_binary_exponent = -126;
        static constexpr std::int32_t exponent_bias = 127;
        static constexpr std::int32_t sign_shift = 31;
        static constexpr std::int32_t exponent_shift = 23;

        using uint_type = std::uint32_t;

        static constexpr std::uint32_t exponent_mask = 0x000000FFu;
        static constexpr std::uint32_t normal_mantissa_mask = 0x00FFFFFFu;
        static constexpr std::uint32_t denormal_mantissa_mask = 0x007FFFFFu;
        static constexpr std::uint32_t special_nan_mantissa_mask = 0x00400000u;
        static constexpr std::uint32_t shifted_sign_mask = 0x80000000u;
        static constexpr std::uint32_t shifted_exponent_mask = 0x7F800000u;

        static constexpr float minimum_value = 0x1.000000p-126f;
        static constexpr float maximum_value = 0x1.FFFFFEp+127f;
    };

    template <>
    struct floating_type_traits<double> {
        static constexpr std::int32_t mantissa_bits = 53;
        static constexpr std::int32_t exponent_bits = 11;
        static constexpr std::int32_t maximum_binary_exponent = 1023;
        static constexpr std::int32_t minimum_binary_exponent = -1022;
        static constexpr std::int32_t exponent_bias = 1023;
        static constexpr std::int32_t sign_shift = 63;
        static constexpr std::int32_t exponent_shift = 52;

        using uint_type = std::uint64_t;

        static constexpr std::uint64_t exponent_mask = 0x00000000000007FFu;
        static constexpr std::uint64_t normal_mantissa_mask = 0x001FFFFFFFFFFFFFu;
        static constexpr std::uint64_t denormal_mantissa_mask = 0x000FFFFFFFFFFFFFu;
        static constexpr std::uint64_t special_nan_mantissa_mask = 0x0008000000000000u;
        static constexpr std::uint64_t shifted_sign_mask = 0x8000000000000000u;
        static constexpr std::uint64_t shifted_exponent_mask = 0x7FF0000000000000u;

        static constexpr double minimum_value = 0x1.0000000000000p-1022;
        static constexpr double maximum_value = 0x1.FFFFFFFFFFFFFp+1023;
    };

    template <>
    struct floating_type_traits<long double> : floating_type_traits<double> {};
}

namespace rainy::utility::implements {
    struct ignore_type {
        explicit ignore_type() = default;

        template <typename Ty>
        constexpr const ignore_type &operator=(const Ty &) const noexcept { // NOLINT
            return *this;
        }
    };
}

namespace rainy::utility {
    inline constexpr implements::ignore_type ignore{};
}

#endif
