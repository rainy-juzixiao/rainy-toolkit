/*
 * Copyright 2026 rainy-juzixiao
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
 * @module Core
 *
 * \lang english
 * @brief
 * This is the most critical part of the rainy-toolkit core. It defines how
 * rainy-toolkit communicates with the platform. Without it, the build system
 * will not function properly.
 * @brief
 * It is not intended for end users but for library developers. It contains
 * platform-related macro definitions, function declarations, fundamental type
 * definitions, and template metaprogramming code. It is used by core.hpp.
 * @brief It should not be included directly by users; use core.hpp instead.
 * @brief Also includes a series of builtin functions. Some are implemented in
 *        core.cxx. Direct inclusion may cause compilation failures.
 * @author rainy-juzixiao
 * @date 2/24/2025 1:30:49 PM Initial annotation added (by rainy-juzixiao)
 * @date 3/13/2025 1:57:45 PM Migrated type_traits::other_trans code and revised annotations
 *
 * \lang simp-chinese
 * @brief
 * 这是rainy-toolkit的核心文件的最重要部分。它定义了rainy-toolkit如何与平台进行沟通。没有它，rainy-toolkit的构建系统将无法正常工作
 * @brief
 * 简单点说。它不是给用户使用的。是给库开发者使用的。它包含了一些平台相关的宏定义和函数声明以及非常基础的类型定义和模板元编程代码。它由core.hpp使用
 * @brief 它不应该被用户包含，而是考虑使用core.hpp
 * @brief 另外，包含了一系列内建函数。部分内建函数是由core.cxx负责的。直接包含则有可能导致编译失败
 * @author rainy-juzixiao
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
#include <ctime>
#include <exception>
#include <initializer_list>
#include <memory>
#include <string_view>
#include <utility>

#ifdef __linux__
#include <csignal>
#include <linux/version.h>
#include <unistd.h>
#endif

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

#ifndef RAINY_NODISCARD_RAW_PTR_ALLOC
#define RAINY_NODISCARD_RAW_PTR_ALLOC                                                                                                 \
    RAINY_NODISCARD_MSG("This function allocates memory and returns a raw pointer. "                                                  \
                        "Discarding the return value will cause a memory leak.")
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

#if RAINY_HAS_CXX26
#define RAINY_CONSTEXPR26 constexpr
#else
#define RAINY_CONSTEXPR26
#endif

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
#define RAINY_STATIC_REFLECTION_CONSTEXPR26 constexpr
#else
#define RAINY_STATIC_REFLECTION_CONSTEXPR26
#endif

#if RAINY_HAS_CXX20
#define RAINY_CONSTEVAL consteval
#else
#define RAINY_CONSTEVAL constexpr
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

#ifndef RAINY_USING_LLVM_GCC
#if defined(__clang__) && defined(__GNUC__)
#define RAINY_USING_LLVM_GCC 1
#else
#define RAINY_USING_LLVM_GCC 0
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

// @NODOCBEGIN
// NOLINTBEGIN
/* 只有涉及到cast表达式时，我们才会使用这些宏 */
#define rainy_let auto
#define rainy_ref auto &
#define rainy_const const auto
#define rainy_cref const auto &
// NOLINTEND
// @NODOCEND

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

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
#include <meta>
#endif

#if RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core {

    /**
     * \lang english
     * @brief Indicates whether Rainy debug mode is enabled.
     *        The value is determined by the RAINY_ENABLE_DEBUG configuration macro.
     *
     * \lang simp-chinese
     * @brief 指示 Rainy 调试模式是否启用。
     *        该值由 RAINY_ENABLE_DEBUG 配置宏决定。
     */
    constexpr bool is_rainy_enable_debug = RAINY_UNSPECVALUE;

    /**
     * \lang english
     * @brief Indicates whether exception support is enabled.
     *        The value depends on whether the compiler supports C++ exceptions.
     *
     * \lang simp-chinese
     * @brief 指示异常支持是否启用。
     *        该值取决于编译器是否支持 C++ 异常机制。
     */
    constexpr bool is_rainy_enable_exception = RAINY_UNSPECVALUE;
}

#else

namespace rainy::core {
    constexpr bool is_rainy_enable_debug = RAINY_ENABLE_DEBUG;

#if __cpp_exceptions
    constexpr bool is_rainy_enable_exception = true;
#else
    constexpr bool is_rainy_enable_exception = false;
#endif
}

#endif

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
     * \lang english
     * @brief Conditional type instantiation for SFINAE overload resolution.
     *        The nested typedef Condition exists (and is a synonym for true)
     *        if and only if enable_if_t<test, Ty> is Type.
     * @tparam Test Value that determines whether the resulting type exists
     * @tparam Ty Type to instantiate when test is true
     * @remark If test is true, enable_if_t<test, Ty> results in a typedef
     *         (which is a synonym for Ty). If test is false,
     *         enable_if_t<test, Ty> does not have a nested typedef named "type".
     * @note All constructors and assignment operators are deleted as this is
     *       a pure type trait struct intended for compile-time use only.
     *
     * \lang simp-chinese
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。
     *        当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     *        Condition 才存在（并且是 true 的同义词）。
     * @tparam Test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef
     *         （它是 Ty 的同义词）。如果 test 为 false，则 enable_if_t<test, Ty>
     *         结果不会拥有名为"type"的嵌套 typedef
     * @note 所有构造函数和赋值运算符都被删除，因为这是一个纯类型特性结构，
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

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief Specialization of enable_if for the true case.
     * @tparam Ty Type to be aliased when Test is true
     *
     * \lang simp-chinese
     * @brief enable_if 在 true 情况下的特化。
     * @tparam Ty 当 Test 为 true 时要别名的类型
     */
    template <typename Ty>
    struct enable_if<true, Ty> {
        using type = Ty;
    };
#endif

    /**
     * \lang english
     * @brief Convenience alias template for enable_if.
     *        Provides a direct way to access the nested type without
     *        using typename enable_if<Test, Ty>::type.
     * @tparam Test Value that determines whether the resulting type exists
     * @tparam Ty Type to instantiate when test is true (defaults to void)
     * @remark If test is true, this alias resolves to Ty. If test is false,
     *         substitution fails (SFINAE), making it useful for template constraints.
     *
     * \lang simp-chinese
     * @brief enable_if 的便捷别名模板。
     *        提供直接访问嵌套类型的方式，无需使用 typename enable_if<Test, Ty>::type。
     * @tparam Test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型（默认为 void）
     * @remark 如果 test 为 true，此别名解析为 Ty。如果 test 为 false，
     *         替换失败（SFINAE），使其适用于模板约束。
     */
    template <bool Test, typename Ty = void>
    using enable_if_t = typename enable_if<Test, Ty>::type; // NOLINT

    /**
     * \lang english
     * @brief Conditional type selection based on a boolean constant.
     *        If Test is true, the type is IfTrue; otherwise, it is IfFalse.
     * @tparam Test Boolean value determining which type to select
     * @tparam IfTrue Type to use when Test is true
     * @tparam IfFalse Type to use when Test is false
     *
     * \lang simp-chinese
     * @brief 基于布尔常量进行条件类型选择。
     *        如果 Test 为 true，则类型为 IfTrue；否则为 IfFalse。
     * @tparam Test 决定选择哪个类型的布尔值
     * @tparam IfTrue Test 为 true 时使用的类型
     * @tparam IfFalse Test 为 false 时使用的类型
     */
    template <bool Test, typename IfTrue, typename IfFalse>
    struct conditional {
        using type = IfTrue;
    };

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief Specialization of conditional for the false case.
     * @tparam IfTrue Type to use when Test is true (unused in this specialization)
     * @tparam IfFalse Type to use when Test is false
     *
     * \lang simp-chinese
     * @brief conditional 在 false 情况下的特化。
     * @tparam IfTrue Test 为 true 时使用的类型（在此特化中未使用）
     * @tparam IfFalse Test 为 false 时使用的类型
     */
    template <typename IfTrue, typename IfFalse>
    struct conditional<false, IfTrue, IfFalse> {
        using type = IfFalse;
    };
#endif

    /**
     * \lang english
     * @brief Convenience alias template for conditional.
     *        Provides direct access to the nested type.
     * @tparam Test Boolean value determining which type to select
     * @tparam IfTrue Type to use when Test is true
     * @tparam IfFalse Type to use when Test is false
     *
     * \lang simp-chinese
     * @brief conditional 的便捷别名模板。
     *        提供对嵌套类型的直接访问。
     * @tparam Test 决定选择哪个类型的布尔值
     * @tparam IfTrue Test 为 true 时使用的类型
     * @tparam IfFalse Test 为 false 时使用的类型
     */
    template <bool Test, typename IfTrue, typename IfFalse>
    using conditional_t = typename conditional<Test, IfTrue, IfFalse>::type;

    /**
     * \lang english
     * @brief Utility metafunction that maps any sequence of types to void.
     *        Used for SFINAE detection idioms.
     * @tparam Types ... Any types (unused)
     *
     * \lang simp-chinese
     * @brief 将任意类型序列映射到 void 的工具元函数。
     *        用于 SFINAE 检测惯用法。
     * @tparam Types ... 任意类型（未使用）
     */
    template <typename... Types>
    using void_t = void;

    /**
     * \lang english
     * @brief Type selector based on a boolean value.
     *        Selects between two types without introducing a nested type.
     * @tparam Test Boolean value controlling the selection
     *
     * \lang simp-chinese
     * @brief 基于布尔值的类型选择器。
     *        在两个类型之间进行选择，不引入嵌套类型。
     * @tparam Test 控制选择的布尔值
     */
    template <bool Test>
    struct select {
        /**
         * \lang english
         * @brief Applies selection by returning the first type.
         * @tparam Ty1 Type to return
         * @tparam Ty2 Type to ignore
         *
         * \lang simp-chinese
         * @brief 通过返回第一个类型来应用选择。
         * @tparam Ty1 要返回的类型
         * @tparam Ty2 要忽略的类型
         */
        template <typename Ty1, typename Ty2>
        using apply = Ty1;
    };

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief Specialization of select for the false case.
     *
     * \lang simp-chinese
     * @brief select 在 false 情况下的特化。
     */
    template <>
    struct select<false> {
        /**
         * \lang english
         * @brief Applies selection by returning the second type.
         * @tparam Ty1 Type to ignore
         * @tparam Ty2 Type to return
         *
         * \lang simp-chinese
         * @brief 通过返回第二个类型来应用选择。
         * @tparam Ty1 要忽略的类型
         * @tparam Ty2 要返回的类型
         */
        template <typename Ty1, typename Ty2>
        using apply = Ty2;
    };
#endif

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
     * \lang english
     * @brief Conditionally adds const qualifier to a type.
     *        If IsConst is true, yields const Ty; otherwise yields Ty.
     * @tparam IsConst Boolean flag for const qualification
     * @tparam Ty Type to potentially add const to
     *
     * \lang simp-chinese
     * @brief 有条件地向类型添加 const 限定符。
     *        如果 IsConst 为 true，则生成 const Ty；否则生成 Ty。
     * @tparam IsConst 是否添加 const 限定的布尔标志
     * @tparam Ty 可能添加 const 的类型
     */
    template <bool IsConst, typename Ty>
    struct maybe_const {
        using type = conditional_t<IsConst, const Ty, Ty>;
    };

    /**
     * \lang english
     * @brief Convenience alias template for maybe_const.
     *        Provides direct access to the nested type.
     * @tparam IsConst Boolean flag for const qualification
     * @tparam Ty Type to potentially add const to
     *
     * \lang simp-chinese
     * @brief maybe_const 的便捷别名模板。
     *        提供对嵌套类型的直接访问。
     * @tparam IsConst 是否添加 const 限定的布尔标志
     * @tparam Ty 可能添加 const 的类型
     */
    template <bool IsConst, typename Ty>
    using maybe_const_t = typename maybe_const<IsConst, Ty>::type;
}

namespace rainy::type_traits::modifers {
    /**
     * \lang english
     * @brief Removes reference qualification from a type.
     *        If Ty is an lvalue reference or rvalue reference, provides the referred type.
     *        Otherwise, provides Ty itself.
     * @tparam Ty Type to remove reference qualification from
     *
     * \lang simp-chinese
     * @brief 移除类型中的引用限定。
     *        如果 Ty 是左值引用或右值引用，则提供其引用所指向的类型。
     *        否则，直接提供 Ty 类型本身。
     * @tparam Ty 需要移除引用限定的类型
     */
    template <typename Ty>
    struct remove_reference {
        using type = Ty;
    };

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief Specialization for lvalue reference types.
     *        Removes the lvalue reference qualifier and provides the underlying type.
     * @tparam Ty Type referenced by the lvalue reference
     *
     * \lang simp-chinese
     * @brief 左值引用类型的特化版本。
     *        移除左值引用限定，并提供其引用所对应的基础类型。
     * @tparam Ty 左值引用所引用的类型
     */
    template <typename Ty>
    struct remove_reference<Ty &> {
        using type = Ty;
    };

    /**
     * \lang english
     * @brief Specialization for rvalue reference types.
     *        Removes the rvalue reference qualifier and provides the underlying type.
     * @tparam Ty Type referenced by the rvalue reference
     *
     * \lang simp-chinese
     * @brief 右值引用类型的特化版本。
     *        移除右值引用限定，并提供其引用所对应的基础类型。
     * @tparam Ty 右值引用所引用的类型
     */
    template <typename Ty>
    struct remove_reference<Ty &&> {
        using type = Ty;
    };
#endif

    /**
     * \lang english
     * @brief Convenience alias template for remove_reference.
     *        Provides direct access to the nested type.
     * @tparam Ty Type to remove reference qualification from
     *
     * \lang simp-chinese
     * @brief remove_reference 的便捷别名模板。
     *        提供对嵌套类型的直接访问。
     * @tparam Ty 需要移除引用限定的类型
     */
    template <typename Ty>
    using remove_reference_t = typename remove_reference<Ty>::type;
}

namespace rainy::type_traits::properties {
    /**
     * \lang english
     * @brief Variable template that checks whether a type is an lvalue reference.
     *        Provides true if Ty is an lvalue reference type, otherwise false.
     * @tparam Ty Type to check for lvalue reference qualification
     *
     * \lang simp-chinese
     * @brief 检查类型是否为左值引用的变量模板。
     *        如果 Ty 是左值引用类型，则提供 true，否则提供 false。
     * @tparam Ty 需要检查是否具有左值引用限定的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v = false;

    /**
     * \lang english
     * @brief Specialization for lvalue reference types.
     *        Indicates that the given type is an lvalue reference.
     * @tparam Ty Type referenced by the lvalue reference
     *
     * \lang simp-chinese
     * @brief 左值引用类型的特化版本。
     *        表示给定类型是一个左值引用类型。
     * @tparam Ty 左值引用所引用的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_lvalue_reference_v<Ty &> = true;
}

namespace rainy::core::builtin {
#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
    /**
     * \lang english
     * @brief Counts the number of consecutive zero bits from the least significant bit.
     *        Uses AVX2 instructions to accelerate the calculation on x86 platforms.
     *        Returns 32 when the input value is zero.
     * @param x Unsigned 32-bit integer value to inspect
     * @return Number of trailing zero bits in x
     *
     * \lang simp-chinese
     * @brief 计算从最低有效位开始连续零位的数量。
     *        在 x86 平台上使用 AVX2 指令加速计算。
     *        当输入值为零时返回 32。
     * @param x 需要检查的无符号 32 位整数值
     * @return x 中末尾连续零位的数量
     */
    RAINY_INLINE std::int32_t ctz_avx2(std::uint32_t x) noexcept {
        if (x == 0) {
            return 32;
        }
        const __m256i v = _mm256_set1_epi32(static_cast<int>(x));
        __m256i bits = _mm256_set1_epi32(1);
        int i = 0;
        while (_mm256_testz_si256(v, bits)) {
            bits = _mm256_slli_epi32(bits, 1);
            ++i;
        }
        return i;
    }

    /**
     * \lang english
     * @brief Counts the number of trailing zero bits in an integer.
     *        Uses compiler-specific intrinsics for efficient implementation.
     * @param mask Integer value to inspect
     * @return Number of consecutive zero bits from the least significant bit
     *
     * \lang simp-chinese
     * @brief 计算整数末尾连续零位的数量。
     *        使用编译器相关内建函数实现高效计算。
     * @param mask 需要检查的整数值
     * @return 从最低有效位开始连续零位的数量
     */
    RAINY_INLINE int ctz(const int mask) {
#if RAINY_COMPILER_MSVC
        unsigned long index;
        _BitScanForward(&index, static_cast<unsigned long>(mask));
        return static_cast<int>(index);
#else
        return __builtin_ctz(static_cast<unsigned int>(mask));
#endif
    }

#endif

    /**
     * \lang english
     * @brief Compares the contents of two memory blocks.
     *        Returns a value indicating the lexicographical relationship between them.
     * @param mem1 Pointer to the first memory block
     * @param mem2 Pointer to the second memory block
     * @param count Number of bytes to compare
     * @return Negative value if mem1 is less than mem2,
     *         zero if they are equal,
     *         positive value if mem1 is greater than mem2
     *
     * \lang simp-chinese
     * @brief 比较两个内存区域中的内容。
     *        返回表示二者字典序关系的结果。
     * @param mem1 指向第一个内存区域的指针
     * @param mem2 指向第二个内存区域的指针
     * @param count 需要比较的字节数量
     * @return 如果 mem1 小于 mem2，则返回负值；
     *         如果二者相等，则返回 0；
     *         如果 mem1 大于 mem2，则返回正值
     */
    constexpr rain_fn compare_memory(const void *mem1, const void *mem2, const std::size_t count) -> int {
        return __builtin_memcmp(mem1, mem2, count);
    }

    /**
     * \lang english
     * @brief Compares the contents of two wide-character memory blocks.
     *        Performs lexicographical comparison of wide-character sequences.
     * @param mem1 Pointer to the first wide-character memory block
     * @param mem2 Pointer to the second wide-character memory block
     * @param count Number of wide characters to compare
     * @return Negative value if mem1 is less than mem2,
     *         zero if they are equal,
     *         positive value if mem1 is greater than mem2
     *
     * \lang simp-chinese
     * @brief 比较两个宽字符内存区域中的内容。
     *        对宽字符序列执行字典序比较。
     * @param mem1 指向第一个宽字符内存区域的指针
     * @param mem2 指向第二个宽字符内存区域的指针
     * @param count 需要比较的宽字符数量
     * @return 如果 mem1 小于 mem2，则返回负值；
     *         如果二者相等，则返回 0；
     *         如果 mem1 大于 mem2，则返回正值
     */
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

    /**
     * \lang english
     * @brief Copies a block of memory from source to destination.
     * @param dest Pointer to the destination memory block.
     * @param src Pointer to the source memory block.
     * @param len Number of bytes to copy.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function copies `len` bytes from `src` to `dest`.
     * The source and destination memory regions must not overlap.
     *
     * \lang simp-chinese
     * @brief 从源内存块复制数据到目标内存块。
     * @param dest 目标内存块的指针。
     * @param src 源内存块的指针。
     * @param len 要复制的字节数。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数将 `len` 字节从 `src` 复制到 `dest`。
     * 源和目标内存区域不得重叠。
     */
    RAINY_INLINE rain_fn copy_memory(void *dest, const void *src, std::size_t len) -> void * {
        return std::memcpy(dest, src, len);
    }

    /**
     * \lang english
     * @brief Moves a block of memory with handling of overlapping regions.
     * @param dest Pointer to the destination memory block.
     * @param dest_size Size of the destination buffer.
     * @param src Pointer to the source memory block.
     * @param src_count Number of bytes to move.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function safely copies `src_count` bytes from `src` to `dest`,
     * even when the source and destination memory regions overlap.
     *
     * \lang simp-chinese
     * @brief 移动内存块，处理重叠区域。
     * @param dest 目标内存块的指针。
     * @param dest_size 目标缓冲区的大小。
     * @param src 源内存块的指针。
     * @param src_count 要移动的字节数。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数安全地将 `src_count` 字节从 `src` 复制到 `dest`，
     * 即使源和目标内存区域重叠也能正确处理。
     */
    RAINY_INLINE rain_fn move_memory(void *dest, std::size_t dest_size, const void *src, std::size_t src_count) -> void * {
        return std::memmove(dest, src, src_count);
    }

    /**
     * \lang english
     * @brief Sets a block of memory to a specified value.
     * @param dest Pointer to the memory block.
     * @param count Number of bytes to set.
     * @param new_val Value to fill the memory with.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function fills `count` bytes of the memory block starting at `dest`
     * with the specified `new_val` value.
     *
     * \lang simp-chinese
     * @brief 将内存块设置为指定值。
     * @param dest 内存块的指针。
     * @param count 要设置的字节数。
     * @param new_val 要填充的值。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数将从 `dest` 开始的 `count` 字节内存块填充为指定的 `new_val` 值。
     */
    RAINY_INLINE rain_fn set_memory(void *dest, std::size_t count, int new_val) -> void * {
        return std::memset(dest, new_val, count);
    }

    /**
     * \lang english
     * @brief Zeroes out a block of memory.
     * @param dest Pointer to the memory block.
     * @param count Number of bytes to zero.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function sets `count` bytes of the memory block starting at `dest` to zero.
     *
     * \lang simp-chinese
     * @brief 将内存块置零。
     * @param dest 内存块的指针。
     * @param count 要置零的字节数。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数将从 `dest` 开始的 `count` 字节内存块设置为零。
     */
    RAINY_INLINE rain_fn zero_memory(void *dest, std::size_t count) -> void * {
        return std::memset(dest, 0, count);
    }

    /**
     * \lang english
     * @brief Fills a memory block with a specified value.
     * @param dest Pointer to the memory block.
     * @param count Number of bytes to fill.
     * @param new_val Value to fill the memory with.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function fills `count` bytes of the memory block starting at `dest`
     * with the specified `new_val` value.
     *
     * \lang simp-chinese
     * @brief 用指定值填充内存块。
     * @param dest 内存块的指针。
     * @param count 要填充的字节数。
     * @param new_val 要填充的值。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数将从 `dest` 开始的 `count` 字节内存块填充为指定的 `new_val` 值。
     */
    RAINY_INLINE rain_fn fill_memory(void *dest, std::size_t count, int new_val) -> void * {
        return std::memset(dest, new_val, count);
    }

    /**
     * \lang english
     * @brief Copies a block of memory from source to destination.
     * @param dest Pointer to the destination memory block.
     * @param count Number of bytes to copy.
     * @param src Pointer to the source memory block.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function copies `count` bytes from `src` to `dest`.
     * The source and destination memory regions must not overlap.
     *
     * \lang simp-chinese
     * @brief 从源内存块复制数据到目标内存块。
     * @param dest 目标内存块的指针。
     * @param count 要复制的字节数。
     * @param src 源内存块的指针。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数将 `count` 字节从 `src` 复制到 `dest`。
     * 源和目标内存区域不得重叠。
     */
    RAINY_INLINE rain_fn fill_memory(void *dest, std::size_t count, const void *src) -> void * {
        return std::memcpy(dest, src, count);
    }

    /**
     * \lang english
     * @brief Copies a limited block of memory from source to destination.
     * @param dest Pointer to the destination memory block.
     * @param count Number of bytes to fill in destination.
     * @param src Pointer to the source memory block.
     * @param src_count Number of bytes available in source.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function copies up to `src_count` bytes from `src` to `dest`,
     * but no more than `count` bytes.
     *
     * \lang simp-chinese
     * @brief 从源内存块复制有限的数据到目标内存块。
     * @param dest 目标内存块的指针。
     * @param count 目标区域要填充的字节数。
     * @param src 源内存块的指针。
     * @param src_count 源区域可用的字节数。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数从 `src` 复制最多 `src_count` 字节到 `dest`，
     * 但不超过 `count` 字节。
     */
    RAINY_INLINE rain_fn fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count) -> void * {
        return std::memcpy(dest, src, std::min(count, src_count));
    }

    /**
     * \lang english
     * @brief Copies a memory block with source offset.
     * @param dest Pointer to the destination memory block.
     * @param count Number of bytes to fill in destination.
     * @param src Pointer to the source memory block.
     * @param src_count Number of bytes available in source.
     * @param src_offset Offset into the source memory block.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function copies up to `src_count` bytes from `src` starting at
     * `src_offset` to `dest`, but no more than `count` bytes.
     *
     * \lang simp-chinese
     * @brief 从源内存块的偏移位置复制数据。
     * @param dest 目标内存块的指针。
     * @param count 目标区域要填充的字节数。
     * @param src 源内存块的指针。
     * @param src_count 源区域可用的字节数。
     * @param src_offset 源内存块的偏移量。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数从 `src` 偏移 `src_offset` 位置开始复制最多 `src_count` 字节到 `dest`，
     * 但不超过 `count` 字节。
     */
    RAINY_INLINE rain_fn fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count, std::size_t src_offset)
        -> void * {
        return std::memcpy(dest, static_cast<const char *>(src) + src_offset, std::min(count, src_count));
    }

    /**
     * \lang english
     * @brief Copies a memory block with both source and destination offsets.
     * @param dest Pointer to the destination memory block.
     * @param count Number of bytes to fill in destination.
     * @param src Pointer to the source memory block.
     * @param src_count Number of bytes available in source.
     * @param src_offset Offset into the source memory block.
     * @param dest_offset Offset into the destination memory block.
     * @return Returns a pointer to `dest`.
     * @remark
     * This function copies up to `src_count` bytes from `src` starting at
     * `src_offset` to `dest` starting at `dest_offset`, but no more than `count` bytes.
     *
     * \lang simp-chinese
     * @brief 从源内存块的偏移位置复制数据到目标内存块的偏移位置。
     * @param dest 目标内存块的指针。
     * @param count 目标区域要填充的字节数。
     * @param src 源内存块的指针。
     * @param src_count 源区域可用的字节数。
     * @param src_offset 源内存块的偏移量。
     * @param dest_offset 目标内存块的偏移量。
     * @return 返回指向 `dest` 的指针。
     * @remark
     * 此函数从 `src` 偏移 `src_offset` 位置开始复制最多 `src_count` 字节到 `dest` 偏移 `dest_offset` 位置，
     * 但不超过 `count` 字节。
     */
    RAINY_INLINE rain_fn fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count, std::size_t src_offset,
                                     std::size_t dest_offset) -> void * {
        return std::memcpy(static_cast<char *>(dest) + dest_offset, static_cast<const char *>(src) + src_offset,
                           std::min(count, src_count));
    }

    /**
     * \lang english
     * @brief Type-safe reference forwarding using perfect forwarding.
     * @tparam Ty The type of the object being forwarded.
     * @param arg The lvalue reference object to forward.
     * @return Returns a `Ty&&` forwarded reference.
     * @remark
     * This function safely forwards an lvalue reference to the corresponding type
     * reference (either lvalue or rvalue), preserving the lvalue/rvalue nature
     * of the passed argument.
     *
     * \lang simp-chinese
     * @brief 使用完美转发（perfect forwarding）实现类型安全的引用转发。
     * @tparam Ty 转发对象的类型。
     * @param arg 要转发的左值引用对象。
     * @return 返回类型为 `Ty&&` 的转发对象。
     * @remark
     * 这个函数用于将左值引用安全地转发为相应类型的引用（可能是左值引用或右值引用），
     * 以保留传入参数的左值或右值性质。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::modifers::remove_reference_t<Ty> &arg) noexcept {
        return static_cast<Ty &&>(arg);
    }

    /**
     * \lang english
     * @brief Type-safe reference forwarding using perfect forwarding (rvalue overload).
     * @tparam Ty The type of the object being forwarded.
     * @param arg The rvalue reference object to forward.
     * @return Returns a `Ty&&` forwarded reference.
     * @remark
     * This function safely forwards an rvalue reference to the corresponding type
     * reference, preserving the lvalue/rvalue nature of the passed argument.
     *
     * \lang simp-chinese
     * @brief 使用完美转发（perfect forwarding）实现类型安全的引用转发。
     * @tparam Ty 转发对象的类型。
     * @param arg 要转发的右值引用对象。
     * @return 返回类型为 `Ty&&` 的转发对象。
     * @remark
     * 这个函数用于将右值引用安全地转发为相应类型的引用（可能是左值引用或右值引用），
     * 以保留传入参数的左值或右值性质。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr Ty &&forward(type_traits::modifers::remove_reference_t<Ty> &&arg) noexcept { // NOLINT
        static_assert(!type_traits::properties::is_lvalue_reference_v<Ty>, "bad forward call");
        return static_cast<Ty &&>(arg);
    }

    /**
     * \lang english
     * @brief Obtains the actual address of a variable.
     *
     * Retrieves the real address even if the variable has overloaded
     * the addressof operator.
     * @tparam Ty The type of the variable
     * @param val The variable whose address to obtain
     * @return Ty* The address of the variable
     *
     * \lang simp-chinese
     * @brief 获取变量的实际地址
     * @tparam Ty 变量类型
     * @param val 要获取地址的变量
     * @return Ty* 变量的地址
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr rain_fn addressof(Ty &val) noexcept -> Ty * {
        return __builtin_addressof(val);
    }

    /**
     * \lang english
     * @brief Prevents taking the address of a temporary object (deleted overload).
     * @tparam Ty The type of the temporary
     *
     * \lang simp-chinese
     * @brief 禁止获取临时对象的地址（删除的重载）
     * @tparam Ty 临时对象类型
     */
    template <typename Ty>
    rain_fn addressof(const Ty &&) -> const Ty * = delete;

    /**
     * \lang english
     * @brief Constructs an object at the given memory location.
     *
     * Constructs an object of type Ty at the uninitialized memory location
     * using the provided arguments.
     * @tparam Ty The type of object to construct
     * @tparam Args The constructor argument types
     * @param location Pointer to the memory location
     * @param args Constructor arguments
     * @return Pointer to the constructed object, or nullptr if location is null
     *
     * \lang simp-chinese
     * @brief 在指定内存位置构造对象
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
     * \lang english
     * @brief Computes the length of a C-style narrow string.
     * @param str Null-terminated narrow character string
     * @return std::size_t Length of the string (excluding the null terminator)
     *
     * \lang simp-chinese
     * @brief 计算 C 风格窄字符串的长度
     * @param str 以空字符结尾的窄字符字符串
     * @return std::size_t 字符串长度（不包括结尾的空字符）
     */
    constexpr rain_fn string_length(const char *str) -> std::size_t {
        return __builtin_strlen(str);
    }

    /**
     * \lang english
     * @brief Computes the length of a C-style wide string.
     * @param wstr Null-terminated wide character string
     * @return std::size_t Length of the string (excluding the null terminator)
     *
     * \lang simp-chinese
     * @brief 计算 C 风格宽字符串的长度
     * @param wstr 以空字符结尾的宽字符字符串
     * @return std::size_t 字符串长度（不包括结尾的空字符）
     */
    constexpr rain_fn string_length(const wchar_t *wstr) -> std::size_t {
#if RAINY_USING_GCC
        std::size_t i = 0;
        while (wstr[i]) {
            ++i; // NOLINT
        }
        return i;
#else
        return __builtin_wcslen(wstr);
#endif
    }

    /**
     * \lang english
     * @brief Compares the first count bytes of two memory regions (narrow char).
     * @param string1 Pointer to the first memory block
     * @param string2 Pointer to the second memory block
     * @param count Number of bytes to compare
     * @return int 0 if equal, negative if string1 < string2, positive otherwise
     *
     * \lang simp-chinese
     * @brief 比较两个内存区域的前 count 个字节（窄字符版本）
     * @param string1 指向第一个内存块的指针
     * @param string2 指向第二个内存块的指针
     * @param count 要比较的字节数
     * @return int 如果相同返回 0，如果 string1 小于 string2 返回负数，否则返回正数
     */
    constexpr rain_fn compare_string(const void *string1, const void *string2, const std::size_t count) noexcept -> int {
        return compare_memory(string1, string2, count);
    }

    /**
     * \lang english
     * @brief Compares the first count bytes of two memory regions (wide char).
     * @param string1 Pointer to the first wide character string
     * @param string2 Pointer to the second wide character string
     * @param count Number of wide characters to compare
     * @return int 0 if equal, negative if string1 < string2, positive otherwise
     *
     * \lang simp-chinese
     * @brief 比较两个内存区域的前 count 个字节（宽字符版本）
     * @param string1 指向第一个宽字符字符串的指针
     * @param string2 指向第二个宽字符字符串的指针
     * @param count 要比较的宽字符数
     * @return int 如果相同返回 0，如果 string1 小于 string2 返回负数，否则返回正数
     */
    constexpr rain_fn compare_string(const wchar_t *string1, const wchar_t *string2, const std::size_t count) noexcept -> int {
        return compare_wmemory(string1, string2, count);
    }

    /**
     * \lang english
     * @brief Built-in exception throw helper.
     * @tparam Ty The exception type
     * @param exception The exception object to throw
     *
     * \lang simp-chinese
     * @brief 内建异常抛出辅助函数。
     * @tparam Ty 异常类型
     * @param exception 要抛出的异常对象
     */
    template <typename Ty>
    RAINY_NORETURN RAINY_NOINLINE void throw_exception_builtin(const Ty &exception) {
        throw exception;
    }

#if RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    // NOLINTBEGIN
    /**
     * \lang english
     * @brief Compile-time integer sequence generator.
     *        Generates compile-time integer sequences such as std::integer_sequence,
     *        used for parameter pack expansion in template metaprogramming.
     * @tparam Struct The sequence template, e.g. template <typename U, U...> typename Seq
     * @tparam Ty The element type of the sequence
     * @tparam N The length of the sequence
     *
     * \lang simp-chinese
     * @brief 编译期整数序列生成器。
     *        用于生成如 std::integer_sequence 这样的编译期整数序列，
     *        适用于模板元编程中需要展开参数包的场景。
     * @tparam Struct 序列模板，如 template <typename U, U...> typename Seq
     * @tparam Ty 序列中元素的类型
     * @tparam N 序列长度
     */
    template <template <typename U, U...> typename Struct, typename Ty, Ty N>
    struct make_integer_seq {
        using type = Struct<Ty>;
    };
#else
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
#endif
    // NOLINTEND

    /**
     * \lang english
     * @brief Zeroes the non-value bits of a given object.
     * @tparam Type The type of the object pointer
     * @param ptr Pointer to the object to clear non-value bits for
     *
     * \lang simp-chinese
     * @brief 将给定对象的非值位清零。
     * @tparam Type 对象指针的类型
     * @param ptr 要清除非值位的对象指针
     */
    template <typename Type>
    void zero_non_value_bits(Type *ptr) noexcept {
        __builtin_zero_non_value_bits(ptr);
    }
}

namespace rainy::utility {
    using core::builtin::addressof;
    using core::builtin::construct_at;
    using core::builtin::forward;
}

namespace rainy::core::layer {
    /* memory io */

    /**
     * \lang english
     * @brief Checks if a pointer is aligned to the specified alignment.
     * @param ptr The pointer to check
     * @param alignment The alignment requirement
     * @return true if aligned, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查指针是否按指定对齐方式对齐。
     * @param ptr 要检查的指针
     * @param alignment 对齐要求
     * @return 如果对齐则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn is_aligned(void *ptr, std::size_t alignment) -> bool;

    /**
     * \lang english
     * @brief Allocates memory of the specified size.
     * @param size The size to allocate in bytes
     * @return Pointer to the allocated memory, or nullptr on failure
     *
     * \lang simp-chinese
     * @brief 分配指定大小的内存。
     * @param size 要分配的字节数
     * @return 指向已分配内存的指针，失败时返回nullptr
     */
    RAINY_TOOLKIT_API rain_fn allocate(std::size_t size) noexcept -> void *;

    /**
     * \lang english
     * @brief Allocates aligned memory of the specified size.
     * @param size The size to allocate in bytes
     * @param alignment The alignment requirement
     * @return Pointer to the allocated memory, or nullptr on failure
     *
     * \lang simp-chinese
     * @brief 分配指定大小的对齐内存。
     * @param size 要分配的字节数
     * @param alignment 对齐要求
     * @return 指向已分配内存的指针，失败时返回nullptr
     */
    RAINY_TOOLKIT_API rain_fn allocate(std::size_t size, std::size_t alignment) noexcept -> void *;

    /**
     * \lang english
     * @brief Deallocates memory previously allocated with allocate().
     * @param block Pointer to the memory to deallocate
     *
     * \lang simp-chinese
     * @brief 释放之前使用allocate()分配的内存。
     * @param block 要释放的内存指针
     */
    RAINY_TOOLKIT_API rain_fn deallocate(void *block) -> void;

    /**
     * \lang english
     * @brief Deallocates aligned memory.
     * @param block Pointer to the memory to deallocate
     * @param alignment The alignment that was used for allocation
     *
     * \lang simp-chinese
     * @brief 释放对齐的内存。
     * @param block 要释放的内存指针
     * @param alignment 分配时使用的对齐方式
     */
    RAINY_TOOLKIT_API rain_fn deallocate(void *block, std::size_t alignment) -> void;

    /**
     * \lang english
     * @brief Deallocates memory with full allocation parameters.
     * @param ptr Pointer to the memory to deallocate
     * @param size The size that was allocated
     * @param alignment The alignment that was used
     *
     * \lang simp-chinese
     * @brief 使用完整的分配参数释放内存。
     * @param ptr 要释放的内存指针
     * @param size 分配的大小
     * @param alignment 使用的对齐方式
     */
    RAINY_TOOLKIT_API rain_fn deallocate(void *ptr, std::size_t size, std::size_t alignment) -> void;
}

namespace rainy::core {
    /**
     * \lang english
     * @brief CPU instruction set feature flags.
     *        Enumerates supported x86/x64 instruction set extensions that can
     *        be queried at runtime through has_instruction().
     *
     * \lang simp-chinese
     * @brief CPU 指令集特性标志。
     *        枚举支持查询的 x86/x64 指令集扩展，
     *        可通过 has_instruction() 在运行时检测。
     */
    enum class instruction_set {

        /**
         * \lang english
         * @brief Streaming SIMD Extensions 3.
         *
         * \lang simp-chinese
         * @brief 流式 SIMD 扩展 3。
         */
        sse3,

        /**
         * \lang english
         * @brief Carry-less multiplication instruction support.
         *
         * \lang simp-chinese
         * @brief 无进位乘法指令支持。
         */
        pclmulqdq,

        /**
         * \lang english
         * @brief Monitor and wait instruction support.
         *
         * \lang simp-chinese
         * @brief Monitor/Wait 监控等待指令支持。
         */
        monitor,

        /**
         * \lang english
         * @brief Supplemental Streaming SIMD Extensions 3.
         *
         * \lang simp-chinese
         * @brief 补充流式 SIMD 扩展 3。
         */
        ssse3,

        /**
         * \lang english
         * @brief Fused multiply-add instruction support.
         *
         * \lang simp-chinese
         * @brief 融合乘加指令支持。
         */
        fma,

        /**
         * \lang english
         * @brief Compare and exchange 16-byte instruction support.
         *
         * \lang simp-chinese
         * @brief 16 字节比较交换指令支持。
         */
        cmpxchg16b,

        /**
         * \lang english
         * @brief Streaming SIMD Extensions 4.1.
         *
         * \lang simp-chinese
         * @brief 流式 SIMD 扩展 4.1。
         */
        sse41,

        /**
         * \lang english
         * @brief Streaming SIMD Extensions 4.2.
         *
         * \lang simp-chinese
         * @brief 流式 SIMD 扩展 4.2。
         */
        sse42,

        /**
         * \lang english
         * @brief Move byte swap instruction support.
         *
         * \lang simp-chinese
         * @brief 字节交换移动指令支持。
         */
        movbe,

        /**
         * \lang english
         * @brief Population count instruction support.
         *
         * \lang simp-chinese
         * @brief 位计数指令支持。
         */
        popcnt,

        /**
         * \lang english
         * @brief AES instruction support.
         *
         * \lang simp-chinese
         * @brief AES 加密指令支持。
         */
        aes,

        /**
         * \lang english
         * @brief XSAVE processor state management support.
         *
         * \lang simp-chinese
         * @brief XSAVE 处理器状态保存支持。
         */
        xsave,

        /**
         * \lang english
         * @brief Operating system XSAVE support.
         *
         * \lang simp-chinese
         * @brief 操作系统 XSAVE 支持。
         */
        osxsave,

        /**
         * \lang english
         * @brief Advanced Vector Extensions support.
         *
         * \lang simp-chinese
         * @brief 高级向量扩展支持。
         */
        avx,

        /**
         * \lang english
         * @brief Half-precision floating-point conversion support.
         *
         * \lang simp-chinese
         * @brief 半精度浮点转换支持。
         */
        f16c,

        /**
         * \lang english
         * @brief Hardware random number generation support.
         *
         * \lang simp-chinese
         * @brief 硬件随机数生成支持。
         */
        rdrand,

        /**
         * \lang english
         * @brief Model-specific register support.
         *
         * \lang simp-chinese
         * @brief 模型专用寄存器支持。
         */
        msr,

        /**
         * \lang english
         * @brief CMPXCHG8B instruction support.
         *
         * \lang simp-chinese
         * @brief CMPXCHG8B 指令支持。
         */
        cx8,

        /**
         * \lang english
         * @brief Fast system call support.
         *
         * \lang simp-chinese
         * @brief 快速系统调用支持。
         */
        sep,

        /**
         * \lang english
         * @brief Conditional move instruction support.
         *
         * \lang simp-chinese
         * @brief 条件移动指令支持。
         */
        cmov,

        /**
         * \lang english
         * @brief Cache line flush instruction support.
         *
         * \lang simp-chinese
         * @brief 缓存行刷新指令支持。
         */
        clflush,

        /**
         * \lang english
         * @brief MMX instruction support.
         *
         * \lang simp-chinese
         * @brief MMX 指令支持。
         */
        mmx,

        /**
         * \lang english
         * @brief Floating-point state save/restore support.
         *
         * \lang simp-chinese
         * @brief 浮点状态保存与恢复支持。
         */
        fxsr,

        /**
         * \lang english
         * @brief Streaming SIMD Extensions.
         *
         * \lang simp-chinese
         * @brief 流式 SIMD 扩展。
         */
        sse,

        /**
         * \lang english
         * @brief Streaming SIMD Extensions 2.
         *
         * \lang simp-chinese
         * @brief 流式 SIMD 扩展 2。
         */
        sse2,

        /**
         * \lang english
         * @brief FSGSBASE instruction support.
         *
         * \lang simp-chinese
         * @brief FSGSBASE 指令支持。
         */
        fsgsbase,

        /**
         * \lang english
         * @brief Bit Manipulation Instruction Set 1.
         *
         * \lang simp-chinese
         * @brief 位操作指令集 1。
         */
        bmi1,

        /**
         * \lang english
         * @brief Hardware Lock Elision support.
         *
         * \lang simp-chinese
         * @brief 硬件锁省略支持。
         */
        hle,

        /**
         * \lang english
         * @brief Advanced Vector Extensions 2 support.
         *
         * \lang simp-chinese
         * @brief 高级向量扩展 2 支持。
         */
        avx2,

        /**
         * \lang english
         * @brief Bit Manipulation Instruction Set 2.
         *
         * \lang simp-chinese
         * @brief 位操作指令集 2。
         */
        bmi2,

        /**
         * \lang english
         * @brief Enhanced REP MOV/STOS support.
         *
         * \lang simp-chinese
         * @brief 增强 REP MOV/STOS 指令支持。
         */
        erms,

        /**
         * \lang english
         * @brief INVPCID instruction support.
         *
         * \lang simp-chinese
         * @brief INVPCID 指令支持。
         */
        invpcid,

        /**
         * \lang english
         * @brief Restricted Transactional Memory support.
         *
         * \lang simp-chinese
         * @brief 限制事务内存支持。
         */
        rtm,

        /**
         * \lang english
         * @brief AVX-512 foundation support.
         *
         * \lang simp-chinese
         * @brief AVX-512 基础支持。
         */
        avx512f,

        /**
         * \lang english
         * @brief RDSEED instruction support.
         *
         * \lang simp-chinese
         * @brief RDSEED 指令支持。
         */
        rdseed,

        /**
         * \lang english
         * @brief ADX instruction support.
         *
         * \lang simp-chinese
         * @brief ADX 指令支持。
         */
        adx,

        /**
         * \lang english
         * @brief AVX-512 prefetch support.
         *
         * \lang simp-chinese
         * @brief AVX-512 预取支持。
         */
        avx512pf,

        /**
         * \lang english
         * @brief AVX-512 exponential and reciprocal support.
         *
         * \lang simp-chinese
         * @brief AVX-512 指数与倒数运算支持。
         */
        avx512er,

        /**
         * \lang english
         * @brief AVX-512 conflict detection support.
         *
         * \lang simp-chinese
         * @brief AVX-512 冲突检测支持。
         */
        avx512cd,

        /**
         * \lang english
         * @brief SHA instruction support.
         *
         * \lang simp-chinese
         * @brief SHA 指令支持。
         */
        sha,

        /**
         * \lang english
         * @brief PREFETCHWT1 instruction support.
         *
         * \lang simp-chinese
         * @brief PREFETCHWT1 指令支持。
         */
        prefetchwt1,

        /**
         * \lang english
         * @brief LAHF/SAHF instruction support in 64-bit mode.
         *
         * \lang simp-chinese
         * @brief 64 位模式下 LAHF/SAHF 指令支持。
         */
        lahf,

        /**
         * \lang english
         * @brief Leading zero count support.
         *
         * \lang simp-chinese
         * @brief 前导零计数支持。
         */
        lzcnt,

        /**
         * \lang english
         * @brief Advanced bit manipulation support.
         *
         * \lang simp-chinese
         * @brief 高级位操作支持。
         */
        abm,

        /**
         * \lang english
         * @brief SSE4a support.
         *
         * \lang simp-chinese
         * @brief SSE4a 支持。
         */
        sse4a,

        /**
         * \lang english
         * @brief Extended operation support.
         *
         * \lang simp-chinese
         * @brief 扩展操作支持。
         */
        xop,

        /**
         * \lang english
         * @brief Trailing bit manipulation support.
         *
         * \lang simp-chinese
         * @brief 尾部位操作支持。
         */
        tbm,

        /**
         * \lang english
         * @brief SYSCALL instruction support.
         *
         * \lang simp-chinese
         * @brief SYSCALL 指令支持。
         */
        syscall,

        /**
         * \lang english
         * @brief Extended MMX support.
         *
         * \lang simp-chinese
         * @brief 扩展 MMX 支持。
         */
        mmxext,

        /**
         * \lang english
         * @brief RDTSCP instruction support.
         *
         * \lang simp-chinese
         * @brief RDTSCP 指令支持。
         */
        rdtscp,

        /**
         * \lang english
         * @brief 3DNow! extension support.
         *
         * \lang simp-chinese
         * @brief 3DNow! 扩展支持。
         */
        _3dnowext,

        /**
         * \lang english
         * @brief 3DNow! instruction support.
         *
         * \lang simp-chinese
         * @brief 3DNow! 指令支持。
         */
        _3dnow,

        /**
         * \lang english
         * @brief Indicates execution under a hypervisor environment.
         *
         * \lang simp-chinese
         * @brief 表示当前运行于虚拟机监控器环境。
         */
        hypervisor
    };
}

namespace rainy::core::builtin {
    /**
     * \lang english
     * @brief Operating system version identifiers.
     *        Enumerates supported operating system versions that can be
     *        reported by get_os_version().
     *
     * \lang simp-chinese
     * @brief 操作系统版本标识符。
     *        枚举可由 get_os_version() 报告的受支持操作系统版本。
     */
    enum class version {

        /**
         * \lang english
         * @brief Microsoft Windows 11.
         *
         * \lang simp-chinese
         * @brief Microsoft Windows 11。
         */
        windows11,

        /**
         * \lang english
         * @brief Microsoft Windows 10.
         *
         * \lang simp-chinese
         * @brief Microsoft Windows 10。
         */
        windows10,

        /**
         * \lang english
         * @brief Microsoft Windows 8.1.
         *
         * \lang simp-chinese
         * @brief Microsoft Windows 8.1。
         */
        windows8_1,

        /**
         * \lang english
         * @brief Microsoft Windows 8.
         *
         * \lang simp-chinese
         * @brief Microsoft Windows 8。
         */
        windows8,

        /**
         * \lang english
         * @brief Microsoft Windows 7 Service Pack 1.
         *
         * \lang simp-chinese
         * @brief Microsoft Windows 7 Service Pack 1。
         */
        windows7sp1,

        /**
         * \lang english
         * @brief Microsoft Windows 7.
         *
         * \lang simp-chinese
         * @brief Microsoft Windows 7。
         */
        windows7,

        /**
         * \lang english
         * @brief Windows Server operating system versions.
         *
         * \lang simp-chinese
         * @brief Windows Server 操作系统版本。
         */
        windows_server,

        /**
         * \lang english
         * @brief Linux-like operating systems.
         *
         * \lang simp-chinese
         * @brief Linux 类操作系统。
         */
        linux_like,

        /**
         * \lang english
         * @brief macOS Catalina.
         *
         * \lang simp-chinese
         * @brief macOS Catalina。
         */
        macos_catalina,

        /**
         * \lang english
         * @brief macOS Big Sur.
         *
         * \lang simp-chinese
         * @brief macOS Big Sur。
         */
        macos_big_sur,

        /**
         * \lang english
         * @brief macOS Monterey.
         *
         * \lang simp-chinese
         * @brief macOS Monterey。
         */
        macos_monterey,

        /**
         * \lang english
         * @brief macOS Ventura.
         *
         * \lang simp-chinese
         * @brief macOS Ventura。
         */
        macos_ventura,

        /**
         * \lang english
         * @brief macOS Sonoma.
         *
         * \lang simp-chinese
         * @brief macOS Sonoma。
         */
        macos_sonoma,

        /**
         * \lang english
         * @brief macOS Sequoia.
         *
         * \lang simp-chinese
         * @brief macOS Sequoia。
         */
        macos_sequoia,

        /**
         * \lang english
         * @brief macOS Tahoe.
         *
         * \lang simp-chinese
         * @brief macOS Tahoe。
         */
        macos_tahoe,

        /**
         * \lang english
         * @brief Unknown or unsupported operating system version.
         *
         * \lang simp-chinese
         * @brief 未知或不受支持的操作系统版本。
         */
        unknown
    };

    /**
     * \lang english
     * @brief Executes the CPUID instruction to query processor information.
     * @param query[4] Output array receiving the EAX, EBX, ECX, EDX values
     * @param function_id The CPUID leaf function to execute
     *
     * \lang simp-chinese
     * @brief 执行 CPUID 指令以查询处理器信息。
     * @param query[4] 输出数组，接收 EAX、EBX、ECX、EDX 值
     * @param function_id 要执行的 CPUID 叶函数
     */
    RAINY_TOOLKIT_API void cpuid(int query[4], int function_id);

    /**
     * \lang english
     * @brief Executes the CPUID instruction with a subfunction.
     * @param query[4] Output array receiving the EAX, EBX, ECX, EDX values
     * @param function_id The CPUID leaf function to execute
     * @param subfunction_id The CPUID subfunction (ECX input) to execute
     *
     * \lang simp-chinese
     * @brief 使用子功能执行 CPUID 指令。
     * @param query[4] 输出数组，接收 EAX、EBX、ECX、EDX 值
     * @param function_id 要执行的 CPUID 叶函数
     * @param subfunction_id 要执行的 CPUID 子功能（ECX 输入）
     */
    RAINY_TOOLKIT_API void cpuidex(int query[4], int function_id, int subfunction_id);

    /**
     * \lang english
     * @brief Detects if running inside a virtual machine / hypervisor.
     * @return true if a hypervisor is detected, false otherwise
     *
     * \lang simp-chinese
     * @brief 检测是否在虚拟机/虚拟机监控程序中运行。
     * @return 如果检测到虚拟机监控程序则为 true，否则为 false
     */
    RAINY_TOOLKIT_API bool is_hypervisor();

    /**
     * \lang english
     * @brief Gets a machine-specific identification code.
     * @return errno_t Error code (reserved for future use, not yet implemented)
     *
     * \lang simp-chinese
     * @brief 获取机器特定的标识码。
     * @return errno_t 错误码（保留供将来使用，尚未实现）
     */
    RAINY_TOOLKIT_API errno_t get_machine_code();

    /**
     * \lang english
     * @brief Checks if a specific CPU instruction set extension is supported.
     * @param check The instruction set feature to query
     * @return true if the instruction set is available on this CPU
     *
     * \lang simp-chinese
     * @brief 检查是否支持特定的 CPU 指令集扩展。
     * @param check 要查询的指令集特性
     * @return 如果此 CPU 支持该指令集则返回 true
     */
    RAINY_TOOLKIT_API bool has_instruction(instruction_set check);

    /**
     * \lang english
     * @brief Gets the CPU vendor string (e.g. "GenuineIntel", "AuthenticAMD").
     * @param buffer Output buffer to store the vendor string
     * @param length Size of the output buffer
     * @return errno_t 0 on success, error code otherwise
     *
     * \lang simp-chinese
     * @brief 获取 CPU 供应商字符串（如"GenuineIntel"、"AuthenticAMD"）。
     * @param buffer 用于存储供应商字符串的输出缓冲区
     * @param length 输出缓冲区的大小
     * @return errno_t 成功时返回 0，否则返回错误码
     */
    RAINY_TOOLKIT_API errno_t get_vendor(char *buffer, std::size_t length);

    /**
     * \lang english
     * @brief Gets the CPU brand string.
     * @param buffer Output buffer to store the brand string
     * @param length Size of the output buffer
     * @return errno_t 0 on success, error code otherwise
     *
     * \lang simp-chinese
     * @brief 获取 CPU 品牌字符串。
     * @param buffer 用于存储品牌字符串的输出缓冲区
     * @param length 输出缓冲区的大小
     * @return errno_t 成功时返回 0，否则返回错误码
     */
    RAINY_TOOLKIT_API errno_t get_brand(char *buffer, std::size_t length);

    /**
     * \lang english
     * @brief Returns the number of hardware threads available.
     * @return std::size_t Number of concurrent threads supported by the hardware
     *
     * \lang simp-chinese
     * @brief 返回可用的硬件线程数。
     * @return std::size_t 硬件支持的并发线程数
     */
    RAINY_TOOLKIT_API std::size_t hardware_concurrency();

    /**
     * \lang english
     * @brief Gets the current operating system version.
     * @return version Enum value identifying the OS version
     *
     * \lang simp-chinese
     * @brief 获取当前操作系统版本。
     * @return version 标识操作系统版本的枚举值
     */
    RAINY_TOOLKIT_API version get_os_version();

    /**
     * \lang english
     * @brief Gets the operating system name as a string.
     * @param buffer Output buffer to store the OS name
     * @return errno_t 0 on success, error code otherwise
     *
     * \lang simp-chinese
     * @brief 获取操作系统名称字符串。
     * @param buffer 用于存储操作系统名称的输出缓冲区
     * @return errno_t 成功时返回 0，否则返回错误码
     */
    RAINY_TOOLKIT_API errno_t get_os_name(char *buffer);

    /**
     * \lang english
     * @brief Gets the CPU architecture string (e.g. "x86_64", "aarch64").
     * @param buffer Output buffer to store the architecture string
     * @return errno_t 0 on success, error code otherwise
     *
     * \lang simp-chinese
     * @brief 获取 CPU 架构字符串（如"x86_64"、"aarch64"）。
     * @param buffer 用于存储架构字符串的输出缓冲区
     * @return errno_t 成功时返回 0，否则返回错误码
     */
    RAINY_TOOLKIT_API errno_t get_arch(char *buffer);
}

namespace rainy::core {
    /**
     * \lang english
     * @brief Signed integer type for I/O operations.
     *        Alias of std::intptr_t for I/O size representation.
     *
     * \lang simp-chinese
     * @brief 用于 I/O 操作的有符号整数类型。
     *        std::intptr_t 的别名，用于 I/O 大小表示。
     */
    using io_size_t = std::intptr_t;

    /**
     * \lang english
     * @brief Constant C-style string type.
     *        Alias for const char*.
     *
     * \lang simp-chinese
     * @brief 常量 C 风格字符串类型。
     *        const char* 的别名。
     */
    using czstring = const char *;

    /**
     * \lang english
     * @brief Mutable C-style string type.
     *        Alias for char*.
     *
     * \lang simp-chinese
     * @brief 可变 C 风格字符串类型。
     *        char* 的别名。
     */
    using cstring = char *;

    /**
     * \lang english
     * @brief Native frame pointer type.
     *        Opaque pointer type for frame handling.
     *
     * \lang simp-chinese
     * @brief 原生框架指针类型。
     *        用于框架处理的不透明指针类型。
     */
    using native_frame_ptr_t = void *;

    /**
     * \lang english
     * @brief Constant native C-style string type.
     *        Alias for const native_char*, representing a constant C-style string
     *        in native character encoding.
     *
     * \lang simp-chinese
     * @brief 常量原生 C 风格字符串类型。
     *        const native_char* 的别名，表示原生字符编码中的常量 C 风格字符串。
     */
    using native_czstring = const native_char *;

    /**
     * \lang english
     * @brief Mutable native C-style string type.
     *        Alias for native_char*, representing a mutable C-style string
     *        in native character encoding.
     *
     * \lang simp-chinese
     * @brief 可变原生 C 风格字符串类型。
     *        native_char* 的别名，表示原生字符编码中的可变 C 风格字符串。
     */
    using native_cstring = native_char *;

    /**
     * \lang english
     * @brief Number of pointer-sized objects for small object optimization.
     *        Calculated as 6 plus space for 16 bytes divided by pointer size.
     *
     * \lang simp-chinese
     * @brief 用于小对象优化的指针大小对象数量。
     *        计算方式为 6 加上 16 字节除以指针大小。
     */
    static inline constexpr std::size_t small_object_num_ptrs = 6 + 16 / sizeof(void *);

    /**
     * \lang english
     * @brief Total space available for small object optimization.
     *        Derived from small_object_num_ptrs.
     *
     * \lang simp-chinese
     * @brief 可用于小对象优化的总空间。
     *        由 small_object_num_ptrs 计算得出。
     */
    static inline constexpr std::size_t small_object_space_size = (small_object_num_ptrs - 1) * sizeof(void *);

    /**
     * \lang english
     * @brief Byte type for raw memory operations.
     *        Unsigned char alias for byte-level access.
     *
     * \lang simp-chinese
     * @brief 用于原始内存操作的字节类型。
     *        用于字节级访问的 unsigned char 别名。
     */
    using byte_t = unsigned char;

    /**
     * \lang english
     * @brief Handle type for resource management.
     *        Unsigned integer type capable of holding a pointer.
     *
     * \lang simp-chinese
     * @brief 用于资源管理的句柄类型。
     *        能够存储指针的无符号整数类型。
     */
    using handle = std::uintptr_t;

    /**
     * \lang english
     * @brief Signed size type.
     *        Alias for std::intptr_t for signed size operations.
     *
     * \lang simp-chinese
     * @brief 有符号大小类型。
     *        用于有符号大小操作的 std::intptr_t 别名。
     */
    using ssize_t = std::intptr_t;

    /**
     * \lang english
     * @brief Invalid handle value constant.
     *        Used to represent an invalid or uninitialized handle.
     *
     * \lang simp-chinese
     * @brief 无效句柄值常量。
     *        用于表示无效或未初始化的句柄。
     */
    static inline constexpr handle invalid_handle = 0;
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Placeholder type for invalid or dummy operations.
     *        Used in template metaprogramming to represent invalid states.
     *
     * \lang simp-chinese
     * @brief 用于无效或虚拟操作的占位符类型。
     *        在模板元编程中用于表示无效状态。
     */
    struct invalid_type {};
}

namespace rainy::type_traits::helper {
    /**
     * \lang english
     * @brief Compile-time constant string implementation.
     *        Provides string operations that can be evaluated at compile time.
     * @tparam CharType Character type of the string (char, wchar_t, etc.)
     * @tparam N Fixed size of the string buffer including null terminator
     *
     * \lang simp-chinese
     * @brief 编译时常量字符串实现。
     *        提供可在编译时评估的字符串操作。
     * @tparam CharType 字符串的字符类型（char、wchar_t 等）
     * @tparam N 字符串缓冲区的固定大小，包括空终止符
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
         * \lang english
         * @brief Default constructor.
         *        Initializes an empty string with null terminator.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         *        使用空终止符初始化空字符串。
         */
        constexpr basic_constexpr_string() noexcept = default;

        /**
         * \lang english
         * @brief Construct from std::string_view.
         *        Copies up to N-1 characters from the string view.
         * @param str_view Source string view to copy from
         *
         * \lang simp-chinese
         * @brief 从 std::string_view 构造。
         *        从字符串视图复制最多 N-1 个字符。
         * @param str_view 要复制的源字符串视图
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
         * \lang english
         * @brief Construct from C-style array.
         *        Copies the entire array including null terminator.
         * @param arr Source character array
         *
         * \lang simp-chinese
         * @brief 从 C 风格数组构造。
         *        复制整个数组，包括空终止符。
         * @param arr 源字符数组
         */
        constexpr basic_constexpr_string(const value_type (&arr)[N]) {
            for (std::size_t i = 0; i < N - 1; ++i) {
                string[i] = arr[i];
            }
            string[N - 1] = '\0';
        }

        /**
         * \lang english
         * @brief Get the fixed size of the string buffer.
         * @return Fixed buffer size including null terminator
         *
         * \lang simp-chinese
         * @brief 获取字符串缓冲区的固定大小。
         * @return 包含空终止符的固定缓冲区大小
         */
        constexpr rain_fn size() const noexcept -> size_type {
            return N;
        }

        /**
         * \lang english
         * @brief Get the actual length of the string (up to first null terminator).
         * @return Current string length
         *
         * \lang simp-chinese
         * @brief 获取字符串的实际长度（直到第一个空终止符）。
         * @return 当前字符串长度
         */
        constexpr rain_fn length() const noexcept -> size_type {
            size_type len = 0;
            while (len < N && string[len] != '\0') {
                len++;
            }
            return len;
        }

        /**
         * \lang english
         * @brief Get mutable pointer to internal buffer.
         * @return Pointer to first element
         *
         * \lang simp-chinese
         * @brief 获取指向内部缓冲区的可变指针。
         * @return 指向第一个元素的指针
         */
        constexpr rain_fn data() noexcept -> pointer {
            return string;
        }

        /**
         * \lang english
         * @brief Get const pointer to internal buffer.
         * @return Const pointer to first element
         *
         * \lang simp-chinese
         * @brief 获取指向内部缓冲区的常量指针。
         * @return 指向第一个元素的常量指针
         */
        constexpr rain_fn data() const noexcept -> const_pointer {
            return string;
        }

        /**
         * \lang english
         * @brief Mutable element access.
         * @param idx Index to access
         * @return Reference to character at specified index
         *
         * \lang simp-chinese
         * @brief 可变元素访问。
         * @param idx 要访问的索引
         * @return 指定索引处字符的引用
         */
        constexpr rain_fn operator[](size_type idx) noexcept -> reference {
            return string[idx];
        }

        /**
         * \lang english
         * @brief Const element access.
         * @param idx Index to access
         * @return Const reference to character at specified index
         *
         * \lang simp-chinese
         * @brief 常量元素访问。
         * @param idx 要访问的索引
         * @return 指定索引处字符的常量引用
         */
        constexpr rain_fn operator[](size_type idx) const noexcept -> const_reference {
            return string[idx];
        }

        CharType string[N]{};
    };

    /**
     * \lang english
     * @brief Equality comparison between two constexpr strings.
     * @tparam CharType Character type
     * @tparam N1 Size of first string
     * @tparam N2 Size of second string
     * @param lhs Left-hand side string
     * @param rhs Right-hand side string
     * @return true if strings are equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 constexpr 字符串之间的相等比较。
     * @tparam CharType 字符类型
     * @tparam N1 第一个字符串的大小
     * @tparam N2 第二个字符串的大小
     * @param lhs 左侧字符串
     * @param rhs 右侧字符串
     * @return 如果字符串相等则为 true，否则为 false
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator==(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) ==
               std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * \lang english
     * @brief Inequality comparison between two constexpr strings.
     *
     * \lang simp-chinese
     * @brief 两个 constexpr 字符串之间的不相等比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator!=(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return !(lhs == rhs);
    }

    /**
     * \lang english
     * @brief Less-than comparison between two constexpr strings.
     *
     * \lang simp-chinese
     * @brief 两个 constexpr 字符串之间的小于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator<(const basic_constexpr_string<CharType, N1> &lhs,
                                const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) < std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * \lang english
     * @brief Less-than-or-equal comparison between two constexpr strings.
     *
     * \lang simp-chinese
     * @brief 两个 constexpr 字符串之间的小于等于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator<=(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return !(rhs < lhs);
    }

    /**
     * \lang english
     * @brief Greater-than comparison between two constexpr strings.
     *
     * \lang simp-chinese
     * @brief 两个 constexpr 字符串之间的大于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator>(const basic_constexpr_string<CharType, N1> &lhs,
                                const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return rhs < lhs;
    }

    /**
     * \lang english
     * @brief Greater-than-or-equal comparison between two constexpr strings.
     *
     * \lang simp-chinese
     * @brief 两个 constexpr 字符串之间的大于等于比较。
     */
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr rain_fn operator>=(const basic_constexpr_string<CharType, N1> &lhs,
                                 const basic_constexpr_string<CharType, N2> &rhs) noexcept -> bool {
        return !(lhs < rhs);
    }

    /**
     * \lang english
     * @brief Equality comparison between constexpr string and string_view.
     *
     * \lang simp-chinese
     * @brief constexpr 字符串与 string_view 之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(const basic_constexpr_string<CharType, N> &lhs, std::basic_string_view<CharType> rhs) noexcept
        -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) == rhs;
    }

    /**
     * \lang english
     * @brief Equality comparison between string_view and constexpr string.
     *
     * \lang simp-chinese
     * @brief string_view 与 constexpr 字符串之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(std::basic_string_view<CharType> lhs, const basic_constexpr_string<CharType, N> &rhs) noexcept
        -> bool {
        return lhs == std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * \lang english
     * @brief Equality comparison between constexpr string and C-style string.
     *
     * \lang simp-chinese
     * @brief constexpr 字符串与 C 风格字符串之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(const basic_constexpr_string<CharType, N> &lhs, const CharType *rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) == std::basic_string_view<CharType>(rhs);
    }

    /**
     * \lang english
     * @brief Equality comparison between C-style string and constexpr string.
     *
     * \lang simp-chinese
     * @brief C 风格字符串与 constexpr 字符串之间的相等比较。
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn operator==(const CharType *lhs, const basic_constexpr_string<CharType, N> &rhs) noexcept -> bool {
        return std::basic_string_view<CharType>(lhs) == std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    /**
     * \lang english
     * @brief Alias for char-based constexpr string.
     *
     * \lang simp-chinese
     * @brief 基于 char 的 constexpr 字符串别名。
     */
    template <std::size_t N>
    using constexpr_string = basic_constexpr_string<char, N>;

    /**
     * \lang english
     * @brief Factory function to create constexpr string from array.
     * @tparam CharType Character type
     * @tparam N Array size including null terminator
     * @param str Source character array
     * @return basic_constexpr_string instance
     *
     * \lang simp-chinese
     * @brief 从数组创建 constexpr 字符串的工厂函数。
     * @tparam CharType 字符类型
     * @tparam N 包含空终止符的数组大小
     * @param str 源字符数组
     * @return basic_constexpr_string 实例
     */
    template <typename CharType, std::size_t N>
    constexpr rain_fn make_constexpr_string(const CharType (&str)[N]) -> auto {
        return basic_constexpr_string<CharType, N>(str);
    }

    /**
     * \lang english
     * @brief Deduction guide for basic_constexpr_string from array.
     *
     * \lang simp-chinese
     * @brief 从数组构造 basic_constexpr_string 的推导指引。
     */
    template <typename CharType, std::size_t N>
    basic_constexpr_string(const CharType (&)[N]) -> basic_constexpr_string<CharType, N>;

    /**
     * \lang english
     * @brief Concatenate two constexpr strings at compile time.
     * @tparam CharType Character type
     * @tparam N1 Size of first string
     * @tparam N2 Size of second string
     * @param lhs First string
     * @param rhs Second string
     * @return Concatenated string of size N1 + N2 - 1
     *
     * \lang simp-chinese
     * @brief 在编译时连接两个 constexpr 字符串。
     * @tparam CharType 字符类型
     * @tparam N1 第一个字符串的大小
     * @tparam N2 第二个字符串的大小
     * @param lhs 第一个字符串
     * @param rhs 第二个字符串
     * @return 大小为 N1 + N2 - 1 的连接后字符串
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
     * \lang english
     * @brief Finds the largest element in a range.
     * @tparam Iter Iterator type
     * @param first Iterator to the beginning of the range
     * @param end Iterator to the end of the range
     * @return Iterator to the largest element, or end if range is empty
     *
     * \lang simp-chinese
     * @brief 查找范围中的最大元素。
     * @tparam Iter 迭代器类型
     * @param first 指向范围起始的迭代器
     * @param end 指向范围末尾的迭代器
     * @return 指向最大元素的迭代器，如果范围为空则返回end
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
     * \lang english
     * @brief Finds the largest element in a range using a custom comparison.
     * @tparam Iter Iterator type
     * @tparam Pred Binary predicate type (bool pred(const T&, const T&))
     * @param first Iterator to the beginning of the range
     * @param end Iterator to the end of the range
     * @param pred Comparison function object that returns true if the first argument is less than the second
     * @return Iterator to the largest element, or end if range is empty
     *
     * \lang simp-chinese
     * @brief 使用自定义比较查找范围中的最大元素。
     * @tparam Iter 迭代器类型
     * @tparam Pred 二元谓词类型
     * @param first 指向范围起始的迭代器
     * @param end 指向范围末尾的迭代器
     * @param pred 比较函数对象，如果第一个参数小于第二个参数则返回true
     * @return 指向最大元素的迭代器，如果范围为空则返回end
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
     * \lang english
     * @brief Finds the smallest element in a range.
     * @tparam Iter Iterator type
     * @param first Iterator to the beginning of the range
     * @param end Iterator to the end of the range
     * @return Iterator to the smallest element, or end if range is empty
     *
     * \lang simp-chinese
     * @brief 查找范围中的最小元素。
     * @tparam Iter 迭代器类型
     * @param first 指向范围起始的迭代器
     * @param end 指向范围末尾的迭代器
     * @return 指向最小元素的迭代器，如果范围为空则返回end
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
     * \lang english
     * @brief Finds the smallest element in a range using a custom comparison.
     * @tparam Iter Iterator type
     * @tparam Pred Binary predicate type (bool pred(const T&, const T&))
     * @param first Iterator to the beginning of the range
     * @param end Iterator to the end of the range
     * @param pred Comparison function object that returns true if the first argument is less than the second
     * @return Iterator to the smallest element, or end if range is empty
     *
     * \lang simp-chinese
     * @brief 使用自定义比较查找范围中的最小元素。
     * @tparam Iter 迭代器类型
     * @tparam Pred 二元谓词类型
     * @param first 指向范围起始的迭代器
     * @param end 指向范围末尾的迭代器
     * @param pred 比较函数对象，如果第一个参数小于第二个参数则返回true
     * @return 指向最小元素的迭代器，如果范围为空则返回end
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
     * \lang english
     * @brief Returns the larger of two values.
     * @tparam Ty Type of the values
     * @param a First value
     * @param b Second value
     * @return const reference to the larger value
     *
     * \lang simp-chinese
     * @brief 返回两个值中的较大者。
     * @tparam Ty 值的类型
     * @param a 第一个值
     * @param b 第二个值
     * @return 较大值的常量引用
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(max)(const Ty &a, const Ty &b) -> const Ty & {
        return (a < b) ? b : a; // NOLINT
    }

    /**
     * \lang english
     * @brief Returns the larger of two values using a custom comparison.
     * @tparam Ty Type of the values
     * @tparam Pred Binary predicate type
     * @param a First value
     * @param b Second value
     * @param comp Comparison function object that returns true if a is less than b
     * @return const reference to the larger value
     *
     * \lang simp-chinese
     * @brief 使用自定义比较返回两个值中的较大者。
     * @tparam Ty 值的类型
     * @tparam Pred 二元谓词类型
     * @param a 第一个值
     * @param b 第二个值
     * @param comp 比较函数对象，如果 a 小于 b 则返回 true
     * @return 较大值的常量引用
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(max)(const Ty &a, const Ty &b, Pred comp) -> const Ty & {
        return (comp(a, b)) ? b : a; // NOLINT
    }

    /**
     * \lang english
     * @brief Returns the largest value in an initializer list.
     * @tparam Ty Type of the values
     * @param ilist Initializer list of values
     * @return The largest value
     *
     * \lang simp-chinese
     * @brief 返回初始化列表中的最大值。
     * @tparam Ty 值的类型
     * @param ilist 值的初始化列表
     * @return 最大值
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(max)(std::initializer_list<Ty> ilist) -> Ty {
        return *(max_element(ilist.begin(), ilist.end()));
    }

    /**
     * \lang english
     * @brief Returns the largest value in an initializer list using a custom comparison.
     * @tparam Ty Type of the values
     * @tparam Pred Binary predicate type
     * @param ilist Initializer list of values
     * @param pred Comparison function object
     * @return The largest value
     *
     * \lang simp-chinese
     * @brief 使用自定义比较返回初始化列表中的最大值。
     * @tparam Ty 值的类型
     * @tparam Pred 二元谓词类型
     * @param ilist 值的初始化列表
     * @param pred 比较函数对象
     * @return 最大值
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(max)(std::initializer_list<Ty> ilist, Pred pred) -> Ty {
        return *(max_element(ilist.begin(), ilist.end(), pred));
    }

    /**
     * \lang english
     * @brief Returns the smaller of two values.
     * @tparam Ty Type of the values
     * @param a First value
     * @param b Second value
     * @return const reference to the smaller value
     *
     * \lang simp-chinese
     * @brief 返回两个值中的较小者。
     * @tparam Ty 值的类型
     * @param a 第一个值
     * @param b 第二个值
     * @return 较小值的常量引用
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(min)(const Ty &a, const Ty &b) -> const Ty & {
        return (a < b) ? a : b;
    }

    /**
     * \lang english
     * @brief Returns the smaller of two values using a custom comparison.
     * @tparam Ty Type of the values
     * @tparam Pred Binary predicate type
     * @param a First value
     * @param b Second value
     * @param comp Comparison function object that returns true if a is less than b
     * @return const reference to the smaller value
     *
     * \lang simp-chinese
     * @brief 使用自定义比较返回两个值中的较小者。
     * @tparam Ty 值的类型
     * @tparam Pred 二元谓词类型
     * @param a 第一个值
     * @param b 第二个值
     * @param comp 比较函数对象，如果 a 小于 b 则返回 true
     * @return 较小值的常量引用
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(min)(const Ty &a, const Ty &b, Pred comp) -> const Ty & {
        return (comp(a, b)) ? a : b;
    }

    /**
     * \lang english
     * @brief Returns the smallest value in an initializer list.
     * @tparam Ty Type of the values
     * @param ilist Initializer list of values
     * @return The smallest value
     *
     * \lang simp-chinese
     * @brief 返回初始化列表中的最小值。
     * @tparam Ty 值的类型
     * @param ilist 值的初始化列表
     * @return 最小值
     */
    template <typename Ty>
    RAINY_CONSTEXPR rain_fn(min)(std::initializer_list<Ty> ilist) -> Ty {
        return *(min_element(ilist.begin(), ilist.end()));
    }

    /**
     * \lang english
     * @brief Returns the smallest value in an initializer list using a custom comparison.
     * @tparam Ty Type of the values
     * @tparam Pred Binary predicate type
     * @param ilist Initializer list of values
     * @param pred Comparison function object
     * @return The smallest value
     *
     * \lang simp-chinese
     * @brief 使用自定义比较返回初始化列表中的最小值。
     * @tparam Ty 值的类型
     * @tparam Pred 二元谓词类型
     * @param ilist 值的初始化列表
     * @param pred 比较函数对象
     * @return 最小值
     */
    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR rain_fn(min)(std::initializer_list<Ty> ilist, Pred pred) -> Ty {
        return *(min_element(ilist.begin(), ilist.end(), pred));
    }
}

namespace rainy::core::builtin {
    /**
     * \lang english
     * @brief Compares two double values for approximate equality.
     * @param p1 First double value
     * @param p2 Second double value
     * @return true if the values are approximately equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较两个 double 值是否近似相等。
     * @param p1 第一个 double 值
     * @param p2 第二个 double 值
     * @return 如果值近似相等则返回 true，否则返回 false
     */
    static RAINY_INLINE rain_fn almost_equal(double p1, double p2) -> bool {
        return (std::abs(p1 - p2) * 1000000000000. <= (core::min) (std::abs(p1), std::abs(p2)));
    }
}

namespace rainy::core {
    /**
     * \lang english
     * @brief Buffer size for function object small object optimization.
     *        For 32-bit systems, extra space is allocated to prevent stack-overrun
     *        issues caused by memory alignment.
     *
     * \lang simp-chinese
     * @brief 函数对象小对象优化的缓冲区大小。
     *        对于32位系统，额外分配空间以防止内存对齐导致的栈溢出问题。
     */
    static constexpr inline std::size_t fn_obj_soo_buffer_size = (small_object_num_ptrs - 1) * sizeof(void *);
}

namespace rainy::core {
    /**
     * \lang english
     * @brief Tag type for internal constructor dispatching.
     *
     * \lang simp-chinese
     * @brief 用于内部构造函数分发的标签类型。
     */
    struct internal_construct_tag_t {};

    /**
     * \lang english
     * @brief Instance of internal_construct_tag_t for constructor tagging.
     *
     * \lang simp-chinese
     * @brief internal_construct_tag_t 的实例，用于构造函数标记。
     */
    RAINY_INLINE_CONSTEXPR internal_construct_tag_t internal_construct_tag{};

    /**
     * \lang english
     * @brief Cache line size for constructive interference.
     *        Maximum size of memory that can be accessed without causing
     *        false sharing between different cores.
     *
     * \lang simp-chinese
     * @brief 构造性干扰的缓存行大小。
     *        在不引起不同核心间错误共享的情况下，可以访问的最大内存大小。
     */
    inline constexpr std::size_t hardware_constructive_interference_size = 64;

    /**
     * \lang english
     * @brief Cache line size for destructive interference.
     *        Minimum size of memory separation needed to avoid false sharing
     *        between different cores.
     *
     * \lang simp-chinese
     * @brief 破坏性干扰的缓存行大小。
     *        避免不同核心间错误共享所需的最小内存间隔大小。
     */
    inline constexpr std::size_t hardware_destructive_interference_size = 64;
}

namespace rainy::core::builtin {
    /**
     * \lang english
     * @brief Returns the positive infinity value for double.
     * @return Positive infinity as double
     *
     * \lang simp-chinese
     * @brief 返回 double 类型的正无穷大值。
     * @return double 类型的正无穷大
     */
    constexpr rain_fn huge_val() noexcept -> double {
        return __builtin_huge_val();
    }

    /**
     * \lang english
     * @brief Returns the positive infinity value for float.
     * @return Positive infinity as float
     *
     * \lang simp-chinese
     * @brief 返回 float 类型的正无穷大值。
     * @return float 类型的正无穷大
     */
    constexpr rain_fn huge_valf() noexcept -> float {
        return __builtin_huge_valf();
    }
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Tag type used for automatic type deduction.
     *        Provides a lightweight compile-time marker object that can be
     *        passed to APIs requiring automatic deduction behavior.
     *
     * \lang simp-chinese
     * @brief 用于自动类型推导的标签类型。
     *        提供一个轻量级的编译期标记对象，
     *        可用于需要自动推导行为的接口。
     */
    struct auto_deduce_t {

        /**
         * \lang english
         * @brief Default constexpr constructor.
         *
         * \lang simp-chinese
         * @brief 默认 constexpr 构造函数。
         */
        explicit constexpr auto_deduce_t() = default;
    };


    /**
     * \lang english
     * @brief Global tag object for requesting automatic type deduction.
     *        Used as a convenient value of auto_deduce_t.
     *
     * \lang simp-chinese
     * @brief 用于请求自动类型推导的全局标签对象。
     *        作为 auto_deduce_t 类型的便捷实例使用。
     */
    static constexpr auto_deduce_t auto_deduce{};
}

namespace rainy::type_traits::helper {
    /**
     * \lang english
     * @brief Compile-time integral constant wrapper.
     *        Wraps a constant value of a specified type as a type for
     *        template metaprogramming and compile-time computations.
     * @tparam Ty Type of the stored constant value
     * @tparam Data Constant value to store
     *
     * \lang simp-chinese
     * @brief 编译期整型常量包装器。
     *        将指定类型的常量值封装为类型，
     *        用于模板元编程以及编译期计算。
     * @tparam Ty 存储常量值的类型
     * @tparam Data 需要存储的常量值
     */
    template <typename Ty, Ty Data>
    struct integral_constant {

        /**
         * \lang english
         * @brief Type of the stored constant value.
         *
         * \lang simp-chinese
         * @brief 存储常量值的类型。
         */
        using value_type = Ty;


        /**
         * \lang english
         * @brief Self type alias.
         *
         * \lang simp-chinese
         * @brief 当前类型的别名。
         */
        using type = integral_constant;


        /**
         * \lang english
         * @brief Converts the wrapper to the stored value type.
         * @return Stored constant value.
         *
         * \lang simp-chinese
         * @brief 将包装器转换为存储值类型。
         * @return 存储的常量值。
         */
        constexpr explicit operator value_type() const noexcept;


        /**
         * \lang english
         * @brief Function call operator returning the stored value.
         * @return Stored constant value.
         *
         * \lang simp-chinese
         * @brief 函数调用运算符，返回存储的常量值。
         * @return 存储的常量值。
         */
        constexpr value_type operator()() const noexcept;


        /**
         * \lang english
         * @brief Stored compile-time constant value.
         *
         * \lang simp-chinese
         * @brief 存储的编译期常量值。
         */
        static constexpr Ty value = Data;
    };

    /**
     * \lang english
     * @brief Alias template for bool-based integral_constant.
     * @tparam Boolean The boolean constant value
     *
     * \lang simp-chinese
     * @brief 布尔类型的 integral_constant 别名模板。
     * @tparam Boolean 布尔常量值
     */
    template <bool Boolean>
    using bool_constant = integral_constant<bool, Boolean>;

    /**
     * \lang english
     * @brief Type representing the true value.
     *
     * \lang simp-chinese
     * @brief 表示 true 值的类型。
     */
    using true_type = integral_constant<bool, true>;

    /**
     * \lang english
     * @brief Type representing the false value.
     *
     * \lang simp-chinese
     * @brief 表示 false 值的类型。
     */
    using false_type = integral_constant<bool, false>;

    /**
     * \lang english
     * @brief Gets the space character constant for a given character type (primary template).
     *        Defaults to char with value ' '.
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 获取字符类型的空格字符常量（主模板）。
     *        默认使用 char 类型的空格字符 ' '。
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    struct char_space : integral_constant<char, ' '> {};

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief wchar_t specialization of space character constant.
     *
     * \lang simp-chinese
     * @brief wchar_t 类型的空格字符常量特化。
     */
    template <>
    struct char_space<wchar_t> : integral_constant<wchar_t, L' '> {};

    /**
     * \lang english
     * @brief char16_t specialization of space character constant.
     *
     * \lang simp-chinese
     * @brief char16_t 类型的空格字符常量特化。
     */
    template <>
    struct char_space<char16_t> : integral_constant<char16_t, u' '> {};

    /**
     * \lang english
     * @brief char32_t specialization of space character constant.
     *
     * \lang simp-chinese
     * @brief char32_t 类型的空格字符常量特化。
     */
    template <>
    struct char_space<char32_t> : integral_constant<char32_t, U' '> {};
#endif

    /**
     * \lang english
     * @brief Variable template for the space character of a given character type.
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 字符类型空格字符常量的变量模板。
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_space_v = char_space<CharType>::value;

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t) && !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief char8_t specialization of space character constant (C++20 and later).
     *
     * \lang simp-chinese
     * @brief char8_t 类型的空格字符常量特化（C++20 及更高版本）。
     */
    template <>
    struct char_space<char8_t> : integral_constant<char8_t, u8' '> {};
#endif

    /**
     * \lang english
     * @brief Gets the null character constant for a given character type (primary template).
     *        Defaults to char with value '\\0'.
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 获取字符类型的空字符常量（主模板）。
     *        默认使用 char 类型的空字符 '\\0'。
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    struct char_null : integral_constant<char, '\0'> {};

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief wchar_t specialization of null character constant.
     *
     * \lang simp-chinese
     * @brief wchar_t 类型的空字符常量特化。
     */
    template <>
    struct char_null<wchar_t> : integral_constant<wchar_t, L'\0'> {};

    /**
     * \lang english
     * @brief char16_t specialization of null character constant.
     *
     * \lang simp-chinese
     * @brief char16_t 类型的空字符常量特化。
     */
    template <>
    struct char_null<char16_t> : integral_constant<char16_t, u'\0'> {};

    /**
     * \lang english
     * @brief char32_t specialization of null character constant.
     *
     * \lang simp-chinese
     * @brief char32_t 类型的空字符常量特化。
     */
    template <>
    struct char_null<char32_t> : integral_constant<char32_t, U'\0'> {};
#endif

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t) && !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief char8_t specialization of null character constant (C++20 and later).
     *
     * \lang simp-chinese
     * @brief char8_t 类型的空字符常量特化（C++20 及更高版本）。
     */
    template <>
    struct char_null<char8_t> : integral_constant<char8_t, u8'\0'> {};
#endif

    /**
     * \lang english
     * @brief Variable template for the null character of a given character type.
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 字符类型空字符常量的变量模板。
     * @tparam CharType 字符类型
     */
    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_null_v = char_null<CharType>::value;

    /**
     * \lang english
     * @brief Variable template to check if a type is wchar_t (primary template, false).
     * @tparam CharType The character type to check
     *
     * \lang simp-chinese
     * @brief 判断字符类型是否为 wchar_t 的变量模板（主模板，false）。
     * @tparam CharType 要检查的字符类型
     */
    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_wchar_t = false;

    /**
     * \lang english
     * @brief Specialization for wchar_t (true).
     *
     * \lang simp-chinese
     * @brief wchar_t 类型的特化（true）。
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<wchar_t> = true;

    /**
     * \lang english
     * @brief Specialization for const wchar_t (true).
     *
     * \lang simp-chinese
     * @brief const wchar_t 类型的特化（true）。
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<const wchar_t> = true;

    /**
     * \lang english
     * @brief Specialization for volatile wchar_t (true).
     *
     * \lang simp-chinese
     * @brief volatile wchar_t 类型的特化（true）。
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<volatile wchar_t> = true;

    /**
     * \lang english
     * @brief Specialization for const volatile wchar_t (true).
     *
     * \lang simp-chinese
     * @brief const volatile wchar_t 类型的特化（true）。
     */
    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<const volatile wchar_t> = true;
}

namespace rainy::type_traits::type_relations {
    /**
     * \lang english
     * @brief Variable template checking whether two types are identical.
     * @tparam Ty1 The first type to compare
     * @tparam Ty2 The second type to compare
     *
     * \lang simp-chinese
     * @brief 判断两个类型是否相同的变量模板。
     * @tparam Ty1 要比较的第一个类型
     * @tparam Ty2 要比较的第二个类型
     */
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_same_v<Ty, Ty> = true;

    /**
     * \lang english
     * @brief Type trait checking whether two types are identical.
     * @tparam Ty1 The first type to compare
     * @tparam Ty2 The second type to compare
     *
     * \lang simp-chinese
     * @brief 判断两个类型是否相同的类型模板。
     * @tparam Ty1 要比较的第一个类型
     * @tparam Ty2 要比较的第二个类型
     */
    template <typename Ty1, typename Ty2>
    struct is_same : helper::bool_constant<is_same_v<Ty1, Ty2>> {};

    /**
     * \lang english
     * @brief Variable template checking whether a type is void.
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为 void 的变量模板。
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = is_same_v<Ty, void>;

    /**
     * \lang english
     * @brief Type trait checking whether a type is void.
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 判断类型是否为 void 的类型模板。
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};
}

namespace rainy::core::builtin {
    /**
     * \lang english
     * @brief Computes the ceiling division of two integers.
     *        Returns the smallest integer value not less than a/b for integer division.
     *        Properly handles ceiling for both positive and negative values.
     * @tparam Ty The integer type
     * @param a The dividend
     * @param b The divisor (must not be 0)
     * @return Ty The ceiling division result
     *
     * @example
     * ceil(5, 3)   // returns 2
     * ceil(-5, 3)  // returns -1
     * ceil(5, -3)  // returns -1
     *
     * \lang simp-chinese
     * @brief 计算两个整数相除的向上取整结果。
     *        返回不小于 a/b 的最小整数值。
     *        正确处理正负数的向上取整。
     * @tparam Ty 整数类型
     * @param a 被除数
     * @param b 除数（不能为 0）
     * @return Ty 向上取整的除法结果
     *
     * @example
     * ceil(5, 3)   // 返回 2
     * ceil(-5, 3)  // 返回 -1
     * ceil(5, -3)  // 返回 -1
     */
    template <typename Ty>
    constexpr rain_fn ceil(Ty a, Ty b) -> Ty {
        return a / b + ((a % b) != 0 && ((a > 0) == (b > 0)));
    }
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Memory ordering enumeration for atomic operations.
     *        Defines the available memory order constraints that control how
     *        operations on atomic variables are ordered around other memory accesses.
     *
     * \lang simp-chinese
     * @brief 原子操作的内存顺序枚举。
     *        定义了可用的内存顺序约束，控制原子变量操作如何与其他内存访问排序。
     */
    enum class memory_order {
        /**
         * \lang english
         * @brief Relaxed memory ordering.
         *        Permits maximum reordering, suitable for atomic operations
         *        that do not require synchronization.
         *
         * \lang simp-chinese
         * @brief 放松的内存顺序。
         *        允许最大程度的重排，适用于不需要同步的原子操作。
         */
        relaxed,

        /**
         * \lang english
         * @brief Consume memory ordering.
         *
         * \lang simp-chinese
         * @brief 消费者内存顺序。
         */
        consume,

        /**
         * \lang english
         * @brief Acquire memory ordering.
         *
         * \lang simp-chinese
         * @brief 获取内存顺序。
         */
        acquire,

        /**
         * \lang english
         * @brief Release memory ordering.
         *
         * \lang simp-chinese
         * @brief 释放内存顺序。
         */
        release,

        /**
         * \lang english
         * @brief Acquire-release memory ordering.
         *
         * \lang simp-chinese
         * @brief 获取并释放内存顺序。
         */
        acq_rel,

        /**
         * \lang english
         * @brief Sequentially consistent memory ordering.
         *
         * \lang simp-chinese
         * @brief 顺序一致性内存顺序。
         */
        seq_cst,

        memory_order_relaxed = relaxed,
        memory_order_consume = consume,
        memory_order_acquire = acquire,
        memory_order_release = release,
        memory_order_acq_rel = acq_rel,
        memory_order_seq_cst = seq_cst
    };

    /**
     * \lang english
     * @brief relaxed memory order constant.
     *
     * \lang simp-chinese
     * @brief relaxed 内存顺序常量。
     */
    inline constexpr memory_order memory_order_relaxed = memory_order::relaxed;

    /**
     * \lang english
     * @brief consume memory order constant.
     *
     * \lang simp-chinese
     * @brief consume 内存顺序常量。
     */
    inline constexpr memory_order memory_order_consume = memory_order::consume;

    /**
     * \lang english
     * @brief acquire memory order constant.
     *
     * \lang simp-chinese
     * @brief acquire 内存顺序常量。
     */
    inline constexpr memory_order memory_order_acquire = memory_order::acquire;

    /**
     * \lang english
     * @brief release memory order constant.
     *
     * \lang simp-chinese
     * @brief release 内存顺序常量。
     */
    inline constexpr memory_order memory_order_release = memory_order::release;

    /**
     * \lang english
     * @brief acq_rel memory order constant.
     *
     * \lang simp-chinese
     * @brief acq_rel 内存顺序常量。
     */
    inline constexpr memory_order memory_order_acq_rel = memory_order::acq_rel;

    /**
     * \lang english
     * @brief seq_cst memory order constant.
     *
     * \lang simp-chinese
     * @brief seq_cst 内存顺序常量。
     */
    inline constexpr memory_order memory_order_seq_cst = memory_order::seq_cst;
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Piecewise construction tag type.
     *        Used to indicate that a constructor should use piecewise construction
     *        for pair or tuple containers.
     *
     * \lang simp-chinese
     * @brief 分段构造标签类型。
     *        用于指示构造函数应使用分段方式构造 pair 或 tuple 等容器。
     */
    struct piecewise_construct_t {
#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
        explicit piecewise_construct_t() = default;
#endif
    };

    /**
     * \lang english
     * @brief Piecewise construction tag constant.
     *
     * \lang simp-chinese
     * @brief 分段构造标签常量实例。
     */
    inline constexpr piecewise_construct_t piecewise_construct{};
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Output iterator that inserts elements at the end of a container.
     * @tparam Container The container type (must support push_back())
     *
     * \lang simp-chinese
     * @brief 在容器末尾插入元素的输出迭代器。
     * @tparam Container 容器类型（必须支持 push_back()）
     */
    template <class Container>
    class back_insert_iterator {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using pointer = void;
        using reference = void;
        using difference_type = void;

        using container_type = Container;

        /**
         * \lang english
         * @brief Constructs a back_insert_iterator for the given container.
         * @param c The container to insert into
         *
         * \lang simp-chinese
         * @brief 为给定容器构造 back_insert_iterator。
         * @param c 要插入元素的容器
         */
        explicit back_insert_iterator(Container &c) : container(std::addressof(c)) {
        }

        /**
         * \lang english
         * @brief Copy-assigns a value by calling push_back.
         *
         * \lang simp-chinese
         * @brief 通过 push_back 复制赋值一个值。
         */
        back_insert_iterator &operator=(const typename Container::value_type &value) {
            container->push_back(value);
            return *this;
        }

        /**
         * \lang english
         * @brief Move-assigns a value by calling push_back.
         *
         * \lang simp-chinese
         * @brief 通过 push_back 移动赋值一个值。
         */
        back_insert_iterator &operator=(typename Container::value_type &&value) {
            container->push_back(std::move(value));
            return *this;
        }

        /** @brief Dereference returns itself (output iterator pattern).  解引用返回自身（输出迭代器模式）。 */
        back_insert_iterator &operator*() noexcept {
            return *this;
        }

        /** @brief Pre-increment returns itself.  前置自增返回自身。 */
        back_insert_iterator &operator++() noexcept {
            return *this;
        }

        /** @brief Post-increment returns a copy.  后置自增返回副本。 */
        back_insert_iterator operator++(int) noexcept {
            return *this;
        }

    protected:
        Container *container;
    };

    /**
     * \lang english
     * @brief Helper function to create a back_insert_iterator.
     * @tparam Container The container type
     * @param c The container to insert into
     * @return back_insert_iterator<Container>
     *
     * \lang simp-chinese
     * @brief 创建 back_insert_iterator 的辅助函数。
     * @tparam Container 容器类型
     * @param c 要插入元素的容器
     * @return back_insert_iterator<Container>
     */
    template <class Container>
    back_insert_iterator<Container> back_inserter(Container &c) {
        return back_insert_iterator<Container>(c);
    }
}
namespace rainy::utility {
#if RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    /**
     * \lang english
     * @brief Traits providing compile-time floating-point type properties.
     *        Provides information about the binary representation,
     *        exponent layout, bit masks, and value limits of floating-point types.
     * @tparam FloatingType Floating-point type to inspect
     *
     * \lang simp-chinese
     * @brief 提供编译期浮点类型属性信息的 traits。
     *        提供浮点类型的二进制表示、指数布局、
     *        位掩码以及取值范围等信息。
     * @tparam FloatingType 需要检查的浮点类型
     */
    template <>
    struct floating_type_traits {
        /**
         * \lang english
         * @brief Number of bits used by the mantissa.
         *
         * \lang simp-chinese
         * @brief 尾数部分占用的位数。
         */
        static constexpr std::int32_t mantissa_bits = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Number of bits used by the exponent.
         *
         * \lang simp-chinese
         * @brief 指数部分占用的位数。
         */
        static constexpr std::int32_t exponent_bits = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Maximum unbiased binary exponent.
         *
         * \lang simp-chinese
         * @brief 最大无偏二进制指数。
         */
        static constexpr std::int32_t maximum_binary_exponent = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Minimum unbiased binary exponent.
         *
         * \lang simp-chinese
         * @brief 最小无偏二进制指数。
         */
        static constexpr std::int32_t minimum_binary_exponent = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Exponent bias value used by the IEEE 754 representation.
         *
         * \lang simp-chinese
         * @brief IEEE 754 表示中使用的指数偏移量。
         */
        static constexpr std::int32_t exponent_bias = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Bit position of the sign bit.
         *
         * \lang simp-chinese
         * @brief 符号位所在的位位置。
         */
        static constexpr std::int32_t sign_shift = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Bit position where the exponent field begins.
         *
         * \lang simp-chinese
         * @brief 指数字段开始的位置。
         */
        static constexpr std::int32_t exponent_shift = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Unsigned integer type used for bit manipulation.
         *
         * \lang simp-chinese
         * @brief 用于位操作的无符号整数类型。
         */
        using uint_type = std::uint32_t;

        /**
         * \lang english
         * @brief Mask for extracting the exponent field.
         *
         * \lang simp-chinese
         * @brief 用于提取指数部分的掩码。
         */
        static constexpr std::uint32_t exponent_mask = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Mask for extracting the normalized mantissa including the implicit bit.
         *
         * \lang simp-chinese
         * @brief 用于提取包含隐藏位的规格化尾数掩码。
         */
        static constexpr std::uint32_t normal_mantissa_mask = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Mask for extracting the denormalized mantissa.
         *
         * \lang simp-chinese
         * @brief 用于提取非规格化尾数的掩码。
         */
        static constexpr std::uint32_t denormal_mantissa_mask = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Mask identifying the quiet NaN mantissa bit.
         *
         * \lang simp-chinese
         * @brief 用于识别静默 NaN 尾数位的掩码。
         */
        static constexpr std::uint32_t special_nan_mantissa_mask = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Mask for the shifted sign bit.
         *
         * \lang simp-chinese
         * @brief 移位后的符号位掩码。
         */
        static constexpr std::uint32_t shifted_sign_mask = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Mask for the shifted exponent field.
         *
         * \lang simp-chinese
         * @brief 移位后的指数部分掩码。
         */
        static constexpr std::uint32_t shifted_exponent_mask = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Minimum positive normalized floating-point value.
         *
         * \lang simp-chinese
         * @brief 最小正规格化浮点值。
         */
        static constexpr float minimum_value = RAINY_UNSPECVALUE;

        /**
         * \lang english
         * @brief Maximum finite floating-point value.
         *
         * \lang simp-chinese
         * @brief 最大有限浮点值。
         */
        static constexpr float maximum_value = RAINY_UNSPECVALUE;
    };
#else
    /**
     * \lang english
     * @brief Traits providing compile-time floating-point type properties.
     *        Defines information about the binary representation, masks,
     *        exponent range, and limits of floating-point types.
     * @tparam FloatingType The floating-point type to inspect
     *
     * \lang simp-chinese
     * @brief 提供编译期浮点类型特性的 traits。
     *        定义浮点类型的二进制表示、掩码、指数范围以及取值限制等信息。
     * @tparam FloatingType 需要检查的浮点类型
     */
    template <typename FloatingType>
    struct floating_type_traits;

    /**
     * \lang english
     * @brief Traits specialization for the float type.
     *        Provides IEEE 754 single-precision floating-point layout information.
     *
     * \lang simp-chinese
     * @brief float 类型的 traits 特化版本。
     *        提供 IEEE 754 单精度浮点数布局相关信息。
     */
    template <>
    struct floating_type_traits<float> {
        /**
         * \lang english
         * @brief Number of bits used by the mantissa.
         *
         * \lang simp-chinese
         * @brief 尾数部分占用的位数。
         */
        static constexpr std::int32_t mantissa_bits = 24;

        /**
         * \lang english
         * @brief Number of bits used by the exponent.
         *
         * \lang simp-chinese
         * @brief 指数部分占用的位数。
         */
        static constexpr std::int32_t exponent_bits = 8;

        /**
         * \lang english
         * @brief Maximum unbiased binary exponent.
         *
         * \lang simp-chinese
         * @brief 最大无偏二进制指数。
         */
        static constexpr std::int32_t maximum_binary_exponent = 127;

        /**
         * \lang english
         * @brief Minimum unbiased binary exponent.
         *
         * \lang simp-chinese
         * @brief 最小无偏二进制指数。
         */
        static constexpr std::int32_t minimum_binary_exponent = -126;

        /**
         * \lang english
         * @brief Exponent bias value used by the IEEE 754 representation.
         *
         * \lang simp-chinese
         * @brief IEEE 754 表示中使用的指数偏移量。
         */
        static constexpr std::int32_t exponent_bias = 127;

        /**
         * \lang english
         * @brief Bit position of the sign bit.
         *
         * \lang simp-chinese
         * @brief 符号位所在的位位置。
         */
        static constexpr std::int32_t sign_shift = 31;

        /**
         * \lang english
         * @brief Bit position where the exponent field begins.
         *
         * \lang simp-chinese
         * @brief 指数字段开始的位置。
         */
        static constexpr std::int32_t exponent_shift = 23;

        /**
         * \lang english
         * @brief Unsigned integer type used for bit manipulation.
         *
         * \lang simp-chinese
         * @brief 用于位操作的无符号整数类型。
         */
        using uint_type = std::uint32_t;

        /**
         * \lang english
         * @brief Mask for extracting the exponent field.
         *
         * \lang simp-chinese
         * @brief 用于提取指数部分的掩码。
         */
        static constexpr std::uint32_t exponent_mask = 0x000000FFu;

        /**
         * \lang english
         * @brief Mask for extracting the normalized mantissa including the implicit bit.
         *
         * \lang simp-chinese
         * @brief 用于提取包含隐藏位的规格化尾数掩码。
         */
        static constexpr std::uint32_t normal_mantissa_mask = 0x00FFFFFFu;

        /**
         * \lang english
         * @brief Mask for extracting the denormalized mantissa.
         *
         * \lang simp-chinese
         * @brief 用于提取非规格化尾数的掩码。
         */
        static constexpr std::uint32_t denormal_mantissa_mask = 0x007FFFFFu;

        /**
         * \lang english
         * @brief Mask identifying the quiet NaN mantissa bit.
         *
         * \lang simp-chinese
         * @brief 用于识别静默 NaN 尾数位的掩码。
         */
        static constexpr std::uint32_t special_nan_mantissa_mask = 0x00400000u;

        /**
         * \lang english
         * @brief Mask for the shifted sign bit.
         *
         * \lang simp-chinese
         * @brief 移位后的符号位掩码。
         */
        static constexpr std::uint32_t shifted_sign_mask = 0x80000000u;

        /**
         * \lang english
         * @brief Mask for the shifted exponent field.
         *
         * \lang simp-chinese
         * @brief 移位后的指数部分掩码。
         */
        static constexpr std::uint32_t shifted_exponent_mask = 0x7F800000u;

        /**
         * \lang english
         * @brief Minimum positive normalized floating-point value.
         *
         * \lang simp-chinese
         * @brief 最小正规格化浮点值。
         */
        static constexpr float minimum_value = 0x1.000000p-126f;

        /**
         * \lang english
         * @brief Maximum finite floating-point value.
         *
         * \lang simp-chinese
         * @brief 最大有限浮点值。
         */
        static constexpr float maximum_value = 0x1.FFFFFEp+127f;
    };

    /**
     * \lang english
     * @brief Traits specialization for the double type.
     *        Provides IEEE 754 double-precision floating-point layout information.
     *
     * \lang simp-chinese
     * @brief double 类型的 traits 特化版本。
     *        提供 IEEE 754 双精度浮点数布局相关信息。
     */
    template <>
    struct floating_type_traits<double> {
        /**
         * \lang english
         * @brief Number of bits used by the mantissa.
         *
         * \lang simp-chinese
         * @brief 尾数部分占用的位数。
         */
        static constexpr std::int32_t mantissa_bits = 53;

        /**
         * \lang english
         * @brief Number of bits used by the exponent.
         *
         * \lang simp-chinese
         * @brief 指数部分占用的位数。
         */
        static constexpr std::int32_t exponent_bits = 11;

        /**
         * \lang english
         * @brief Maximum unbiased binary exponent.
         *
         * \lang simp-chinese
         * @brief 最大无偏二进制指数。
         */
        static constexpr std::int32_t maximum_binary_exponent = 1023;

        /**
         * \lang english
         * @brief Minimum unbiased binary exponent.
         *
         * \lang simp-chinese
         * @brief 最小无偏二进制指数。
         */
        static constexpr std::int32_t minimum_binary_exponent = -1022;

        /**
         * \lang english
         * @brief Exponent bias value used by the IEEE 754 representation.
         *
         * \lang simp-chinese
         * @brief IEEE 754 表示中使用的指数偏移量。
         */
        static constexpr std::int32_t exponent_bias = 1023;

        /**
         * \lang english
         * @brief Bit position of the sign bit.
         *
         * \lang simp-chinese
         * @brief 符号位所在的位位置。
         */
        static constexpr std::int32_t sign_shift = 63;

        /**
         * \lang english
         * @brief Bit position where the exponent field begins.
         *
         * \lang simp-chinese
         * @brief 指数字段开始的位置。
         */
        static constexpr std::int32_t exponent_shift = 52;

        /**
         * \lang english
         * @brief Unsigned integer type used for bit manipulation.
         *
         * \lang simp-chinese
         * @brief 用于位操作的无符号整数类型。
         */
        using uint_type = std::uint64_t;

        /**
         * \lang english
         * @brief Mask for extracting the exponent field.
         *
         * \lang simp-chinese
         * @brief 用于提取指数部分的掩码。
         */
        static constexpr std::uint64_t exponent_mask = 0x00000000000007FFu;

        /**
         * \lang english
         * @brief Mask for extracting the normalized mantissa including the implicit bit.
         *
         * \lang simp-chinese
         * @brief 用于提取包含隐藏位的规格化尾数掩码。
         */
        static constexpr std::uint64_t normal_mantissa_mask = 0x001FFFFFFFFFFFFFu;

        /**
         * \lang english
         * @brief Mask for extracting the denormalized mantissa.
         *
         * \lang simp-chinese
         * @brief 用于提取非规格化尾数的掩码。
         */
        static constexpr std::uint64_t denormal_mantissa_mask = 0x000FFFFFFFFFFFFFu;

        /**
         * \lang english
         * @brief Mask identifying the quiet NaN mantissa bit.
         *
         * \lang simp-chinese
         * @brief 用于识别静默 NaN 尾数位的掩码。
         */
        static constexpr std::uint64_t special_nan_mantissa_mask = 0x0008000000000000u;

        /**
         * \lang english
         * @brief Mask for the shifted sign bit.
         *
         * \lang simp-chinese
         * @brief 移位后的符号位掩码。
         */
        static constexpr std::uint64_t shifted_sign_mask = 0x8000000000000000u;

        /**
         * \lang english
         * @brief Mask for the shifted exponent field.
         *
         * \lang simp-chinese
         * @brief 移位后的指数部分掩码。
         */
        static constexpr std::uint64_t shifted_exponent_mask = 0x7FF0000000000000u;

        /**
         * \lang english
         * @brief Minimum finite value represented by the floating-point type.
         *
         * \lang simp-chinese
         * @brief 浮点类型可表示的最小有限值。
         */
        static constexpr double minimum_value = 0x1.0000000000000p-1022;


        /**
         * \lang english
         * @brief Maximum finite value represented by the floating-point type.
         *
         * \lang simp-chinese
         * @brief 浮点类型可表示的最大有限值。
         */
        static constexpr double maximum_value = 0x1.FFFFFFFFFFFFFp+1023;
    };

    /**
     * \lang english
     * @brief Traits specialization for long double.
     *        Uses double precision layout information as a fallback.
     *
     * \lang simp-chinese
     * @brief long double 类型的 traits 特化版本。
     *        使用 double 精度布局信息作为兼容实现。
     */
    template <>
    struct floating_type_traits<long double> : floating_type_traits<double> {};
#endif
}

namespace rainy::utility::implements {
    /**
     * \lang english
     * @brief Discard / ignore type for unused variables.
     *        Any value assigned to an ignore_type instance is silently discarded.
     *
     * \lang simp-chinese
     * @brief 用于未使用变量的丢弃/忽略类型。
     *        赋值给 ignore_type 实例的任何值都会被静默丢弃。
     */
    struct ignore_type {
        explicit ignore_type() = default;
#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
        template <typename Ty>
        constexpr const ignore_type &operator=(const Ty &) const noexcept { // NOLINT
            return *this;
        }
#endif
    };
}

namespace rainy::utility {
    /**
     * \lang english
     * @brief Global ignore object used to explicitly discard values.
     *        Provides a convenient instance of implements::ignore_type.
     *
     * \lang simp-chinese
     * @brief 用于显式忽略值的全局 ignore 对象。
     *        提供 implements::ignore_type 的便捷实例。
     */
    inline constexpr implements::ignore_type ignore{};
}

#if RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::annotations::implements {
    using info_handle = void *;
}

namespace rainy::annotations {
    /**
     * \lang english
     * @brief Stores runtime member annotation information for C++26 static reflection.
     *        Provides a lightweight representation of annotations attached to
     *        reflected members.
     *
     * \lang simp-chinese
     * @brief 存储 C++26 静态反射中的运行时成员注释信息。
     *        提供反射成员所附加注释的轻量级表示。
     */
    struct member_anno {
        /**
         * \lang english
         * @brief Pointer to the annotation information array.
         *
         * \lang simp-chinese
         * @brief 指向注释信息数组的指针。
         */
        const implements::info_handle *attns;

        /**
         * \lang english
         * @brief Number of stored annotations.
         *
         * \lang simp-chinese
         * @brief 存储的注释数量。
         */
        std::size_t num_attns;
    };

    /**
     * \lang english
     * @brief Creates a member annotation query object from a reflected member.
     * @param member Reflected member information handle
     * @return Member annotation information
     *
     * \lang simp-chinese
     * @brief 根据反射成员创建成员注释查询对象。
     * @param member 反射成员信息句柄
     * @return 成员注释信息
     */
    consteval auto make_member_anno(implements::info_handle member) -> member_anno;
}

namespace rainy::annotations {
    /**
     * \lang english
     * @brief Stores runtime type annotation information for C++26 static reflection.
     *        Provides a lightweight representation of annotations attached to types.
     *
     * \lang simp-chinese
     * @brief 存储 C++26 静态反射中的运行时类型注释信息。
     *        提供类型所附加注释的轻量级表示。
     */
    struct type_anno {
        /**
         * \lang english
         * @brief Pointer to the annotation information array.
         *
         * \lang simp-chinese
         * @brief 指向注释信息数组的指针。
         */
        const implements::info_handle *attns;

        /**
         * \lang english
         * @brief Number of stored annotations.
         *
         * \lang simp-chinese
         * @brief 存储的注释数量。
         */
        std::size_t num_attns;
    };

    /**
     * \lang english
     * @brief Creates a type annotation query object for a type.
     * @tparam Ty Type to inspect for annotations
     * @return Type annotation information
     *
     * \lang simp-chinese
     * @brief 为指定类型创建类型注释查询对象。
     * @tparam Ty 需要检查注释的类型
     * @return 类型注释信息
     */
    template <typename Ty>
    consteval auto make_type_anno() -> type_anno;
}

#elif RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

namespace rainy::annotations {
    struct member_anno {
        template <typename Ty>
        RAINY_NODISCARD consteval auto has() const noexcept -> bool {
            using namespace std::meta;
            for (auto attn: std::span{attns, num_attns}) {
                if (remove_const(type_of(attn)) == ^^Ty) {
                    return true;
                }
            }
            return false;
        }

        template <typename Ty>
        RAINY_NODISCARD consteval auto fetch() const -> Ty {
            using namespace std::meta;
            for (auto attn: std::span{attns, num_attns}) {
                if (remove_const(type_of(attn)) == ^^Ty) {
                    return extract<Ty>(attn);
                }
            }
            std::unreachable();
        }

        template <typename Ty>
        RAINY_NODISCARD consteval auto get_or(Ty default_val) const noexcept -> Ty {
            if (has<Ty>()) {
                return fetch<Ty>();
            }
            return default_val;
        }

        const std::meta::info *attns{nullptr};
        std::size_t num_attns{0};
    };

    consteval auto make_member_anno(std::meta::info member) -> member_anno {
        auto attns = std::meta::annotations_of(member);
        auto span = std::define_static_array(attns);
        return member_anno{span.data(), span.size()};
    }
}

namespace rainy::annotations {
    struct type_anno {
        template <typename Ty>
        RAINY_NODISCARD consteval auto has() const noexcept -> bool {
            using namespace std::meta;
            for (auto attn: std::span{attns, num_attns}) {
                if (remove_const(type_of(attn)) == ^^Ty) {
                    return true;
                }
            }
            return false;
        }

        template <typename Ty>
        RAINY_NODISCARD consteval auto fetch() const -> Ty {
            using namespace std::meta;
            for (auto attn: std::span{attns, num_attns}) {
                if (remove_const(type_of(attn)) == ^^Ty) {
                    return extract<Ty>(attn);
                }
            }
            std::unreachable();
        }

        template <typename Ty>
        RAINY_NODISCARD consteval auto get_or(Ty default_val) const noexcept -> Ty {
            if (has<Ty>()) {
                return fetch<Ty>();
            }
            return default_val;
        }

        const std::meta::info *attns{nullptr};
        std::size_t num_attns{0};
    };

    template <typename Ty>
    consteval auto make_type_anno() -> type_anno {
        auto attns = std::meta::annotations_of(^^Ty);
        auto span = std::define_static_array(attns);
        return type_anno{span.data(), span.size()};
    }
}

#endif

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::implements {
    constexpr bool is_pow_2(const std::size_t val) noexcept {
        return val != 0 && (val & (val - 1)) == 0;
    }

    template <typename Ty>
    constexpr std::size_t get_size_of_n(const std::size_t count) noexcept {
        constexpr std::size_t type_size = sizeof(Ty);
        if constexpr (constexpr bool overflow_is_possible = type_size > 1; overflow_is_possible) {
            if (constexpr std::size_t max_possible = static_cast<std::size_t>(-1) / type_size; count > max_possible) {
                std::terminate(); // multiply overflow
            }
        }
        return type_size * count;
    }

    template <typename Integral>
    RAINY_INLINE constexpr bool in_range(Integral start, Integral end, Integral wait_for_check) noexcept {
        if (start > end) {
            return false;
        }
        return wait_for_check >= start && wait_for_check <= end;
    }
}

namespace rainy::core::implements {
    RAINY_TOOLKIT_API void stl_internal_check(bool result);
}

#endif

namespace rainy::utility {
    /**
     * \lang english
     * @brief Empty monostate type for use in variants and similar contexts.
     *
     * \lang simp-chinese
     * @brief 用于变体等场景的空monostate类型。
     */
    struct monostate {};
}

#endif
