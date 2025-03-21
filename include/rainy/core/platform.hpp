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
 * @brief 简单点说。它不是给用户使用的。是给库开发者使用的。它包含了一些平台相关的宏定义和函数声明以及非常基础的类型定义和模板元编程代码。它由core.hpp使用
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
#include <cstring>

#ifdef __linux__
#include <csignal>
#include <unistd.h>
#endif

/*-----------
MSVC编译器区域
-----------*/
#ifdef _MSC_VER
// 为MSVC编译器提供支持

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

#if RAINY_CURRENT_STANDARD_VERSION == 201703L
#define RAINY_IS_CXX17 true
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
#ifdef __clang__
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

#if RAINY_CURRENT_STANDARD_VERSION < 201703L
static_assert(false, "We detected you are using C++14 and below, and the library only supports C++17 and above, please modify your standard version of C++ to C++17 and above before trying to compile!");
#endif

#if RAINY_USING_AVX2
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
#define RAINY_CAT(a,b) RAINY_CAT_IMPL(a,b)

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
    #define RAINY_TOOLKIT_API
#endif

#if RAINY_USING_DYNAMIC
#define RAINY_STATIC_INLINE
#else
#define RAINY_STATIC_INLINE
#endif

namespace rainy::core::builtin {
#if RAINY_USING_AVX2
    RAINY_TOOLKIT_API RAINY_STATIC_INLINE std::int32_t ctz_avx2(std::uint32_t x) noexcept;
#endif
    constexpr int compare_memory(const void *mem1, const void *mem2, const std::size_t count) {
        return __builtin_memcmp(mem1, mem2, count);
    }

    constexpr int compare_wmemory(const wchar_t *mem1, const wchar_t *mem2, const std::size_t count) {
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

    RAINY_TOOLKIT_API void *copy_memory(void *dest, const void *src, std::size_t len);
    RAINY_TOOLKIT_API void *move_memory(void *dest, std::size_t dest_size, const void *src, std::size_t src_count);
    RAINY_TOOLKIT_API void *set_memory(void *dest, std::size_t count, int new_val);
    RAINY_TOOLKIT_API void *zero_memory(void *dest, std::size_t count);
    RAINY_TOOLKIT_API void *fill_memory(void *dest, std::size_t count, int new_val);
    RAINY_TOOLKIT_API void *fill_memory(void *dest, std::size_t count, const void *src);
    RAINY_TOOLKIT_API void *fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count);
    RAINY_TOOLKIT_API void *fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count, std::size_t src_offset);
    RAINY_TOOLKIT_API void *fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count, std::size_t src_offset,
                                        std::size_t dest_offset);


    template <typename Ty>
    RAINY_NODISCARD constexpr Ty *addressof(Ty &val) noexcept {
        return __builtin_addressof(val);
    }

    template <typename Ty>
    const Ty *addressof(const Ty &&) = delete;

    constexpr std::size_t string_length(const char *str) {
        return __builtin_strlen(str);
    }

    constexpr std::size_t string_length(const wchar_t *wstr) {
#if RAINY_USING_GCC
        std::size_t i = 0;
        while (wstr[i])
            ++i; // NOLINT
        return i;
#else
        return __builtin_wcslen(wstr);
#endif
    }

    constexpr int compare_string(const void *string1, const void *string2,const std::size_t count) noexcept {
        return compare_memory(string1, string2, count);
    }

    constexpr int compare_string(const wchar_t *string1, const wchar_t *string2,const std::size_t count) noexcept {
        return compare_wmemory(string1, string2, count);
    }

    template <typename Ty>
    RAINY_NORETURN RAINY_NOINLINE void throw_exception_builtin(const Ty &exception) {
        throw exception;
    }

    template <template <typename U, U...> typename Struct, typename Ty, Ty N>
    struct make_integer_seq {
        template <Ty... Is>
        static constexpr auto make_seq() {
            if constexpr (sizeof...(Is) == N) {
                return Struct<Ty, Is...>{};
            } else {
                return make_seq<Is..., sizeof...(Is)>();
            }
        }

        using type = decltype(make_seq<>());
    };
}

namespace rainy::core {
    using io_size_t = std::intptr_t;

    using czstring = const char *;
    using cstring = char *;

    using native_frame_ptr_t = const void *;

    inline constexpr std::size_t small_object_num_ptrs = 3 + 16 / sizeof(void *);
    inline constexpr std::size_t space_size = (small_object_num_ptrs - 1) * sizeof(void *);

    using byte_t = unsigned char;
    using handle = std::uintptr_t;
}

namespace rainy::utility {
    struct invalid_type {};
}

namespace rainy::type_traits::other_trans {
    /**
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。 当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     * Condition 才存在（并且是 true 的同义词）。
     * @tparam Test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型。
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef（它是 Ty 的同义词）。如果 test 为
     * false，则 enable_if_t<test, Ty> 结果不会拥有名为“type”的嵌套 typedef
     */
    template <bool Test, typename Ty>
    struct enable_if {
        enable_if() = delete;
        enable_if(const enable_if &) = delete;
        enable_if(enable_if &&) = delete;
        enable_if &operator=(const enable_if &) = delete;
        enable_if &operator=(enable_if &&) = delete;
    };

    template <typename Ty>
    struct enable_if<true, Ty> {
        using type = Ty;
    };

    /**
     * @brief 有条件地为 SFINAE 重载决策设置类型的实例。 当且仅当 enable_if_t<test,Ty> 是 Type 时，嵌套的 typedef
     * Condition 才存在（并且是 true 的同义词）。
     * @tparam Test 确定存在产生的类型的值
     * @tparam Ty test 为 true 时要实例化的类型。
     * @remark 如果 test 为 true，则 enable_if_t<test, Ty> 结果即为typedef（它是 Ty 的同义词）。如果 test 为
     * false，则 enable_if_t<test, Ty> 结果不会拥有名为“type”的嵌套 typedef
     */
    template <bool Test, typename Ty = void>
    using enable_if_t = typename enable_if<Test, Ty>::type;

    template <bool, typename IfTrue, typename>
    struct conditional {
        using type = IfTrue;
    };

    template <typename IfTrue, typename IfFalse>
    struct conditional<false, IfTrue, IfFalse> {
        using type = IfFalse;
    };

    template <bool Test, typename IfTrue, typename IfFalse>
    using conditional_t = typename conditional<Test, IfTrue, IfFalse>::type;

    template <typename...>
    using void_t = void;

    template <bool>
    struct select {
        template <typename Ty1, typename>
        using apply = Ty1;
    };

    template <>
    struct select<false> {
        template <typename, typename Ty2>
        using apply = Ty2;
    };

    struct dummy_t {};
}

#endif
