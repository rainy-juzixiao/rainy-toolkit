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
#include <cstdlib>
#include <initializer_list>
#include <string_view>
#include <string>
#include <utility>

#ifdef __linux__
#include <csignal>
#include <unistd.h>
#include <linux/version.h>
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

#if defined(__arm__) || defined(__aarch64__)
static_assert(false, "ARM architecture is not supported for this project.");
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
    #define RAINY_TOOLKIT_API __attribute__((visibility("default")))
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

    template <typename Ty>
    using pointer = Ty*;

    template <bool IsConst, typename Ty>
    struct maybe_const {
        using type = conditional_t<IsConst, const Ty, Ty>;
    };

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
#if RAINY_USING_AVX2
    RAINY_TOOLKIT_API std::int32_t ctz_avx2(std::uint32_t x) noexcept;
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

    template <typename Ty>
    RAINY_NODISCARD constexpr Ty *addressof(Ty &val) noexcept {
        return __builtin_addressof(val);
    }

    template <typename Ty>
    const Ty *addressof(const Ty &&) = delete;
    
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 Ty *construct_at(Ty *location, Args &&...args) noexcept(noexcept(::new(static_cast<void *>(location))
                                                                                           Ty(builtin::forward<Args>(args)...))) {
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

    constexpr int compare_string(const void *string1, const void *string2, const std::size_t count) noexcept {
        return compare_memory(string1, string2, count);
    }

    constexpr int compare_string(const wchar_t *string1, const wchar_t *string2, const std::size_t count) noexcept {
        return compare_wmemory(string1, string2, count);
    }

    template <typename Ty>
    RAINY_NORETURN RAINY_NOINLINE void throw_exception_builtin(const Ty &exception) {
        throw exception;
    }

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
            using type = typename concat_impl<half_seq, half_seq, Half > ::type;
        };

        template <Ty Count>
        struct gen<Count, type_traits::other_trans::enable_if_t<(Count > 1) && (Count % 2 == 1)>> {
            using prev_seq = typename gen<Count - 1>::type;
            using type = typename concat_impl<prev_seq, Struct<Ty, 0>, Count - 1 > ::type;
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
    using io_size_t = std::intptr_t;

    using czstring = const char *;
    using cstring = char *;

    using native_frame_ptr_t = void *;

    static inline constexpr std::size_t small_object_num_ptrs = 6 + 16 / sizeof(void *);
    static inline constexpr std::size_t small_object_space_size = (small_object_num_ptrs - 1) * sizeof(void *);

    using byte_t = unsigned char;
    using handle = std::uintptr_t;
    using ssize_t = std::intptr_t;

    static inline constexpr handle invalid_handle = 0;
}

namespace rainy::utility {
    struct invalid_type {};
}

namespace rainy::type_traits::helper {
    template <typename CharType, std::size_t N>
    struct basic_constexpr_string {
        using size_type = std::size_t;
        using value_type = CharType;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;

        constexpr basic_constexpr_string() noexcept = default;

        constexpr basic_constexpr_string(std::string_view str_view) {
            std::size_t len_to_copy = str_view.length();
            if (len_to_copy >= N) {
                len_to_copy = N - 1;
            }
            for (std::size_t i = 0; i < len_to_copy; ++i) {
                string[i] = str_view[i];
            }
            string[len_to_copy] = '\0';
        }

        constexpr basic_constexpr_string(const value_type (&arr)[N]) {
            for (std::size_t i = 0; i < N - 1; ++i) {
                string[i] = arr[i];
            }
            string[N - 1] = '\0';
        }

        constexpr size_type size() const noexcept {
            return N;
        }

        constexpr size_type length() const noexcept {
            size_type len = 0;
            while (len < N && string[len] != '\0') {
                len++;
            }
            return len;
        }

        constexpr pointer data() noexcept {
            return string;
        }

        constexpr const_pointer data() const noexcept {
            return string;
        }

        constexpr reference operator[](size_type idx) noexcept {
            return string[idx];
        }

        constexpr const_reference operator[](size_type idx) const noexcept {
            return string[idx];
        }

        CharType string[N]{};
    };

    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr bool operator==(const basic_constexpr_string<CharType, N1> &lhs,
                              const basic_constexpr_string<CharType, N2> &rhs) noexcept {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) ==
               std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    // 不相等比较
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr bool operator!=(const basic_constexpr_string<CharType, N1> &lhs,
                              const basic_constexpr_string<CharType, N2> &rhs) noexcept {
        return !(lhs == rhs);
    }

    // 小于比较
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr bool operator<(const basic_constexpr_string<CharType, N1> &lhs,
                             const basic_constexpr_string<CharType, N2> &rhs) noexcept {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) < std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    // 小于等于比较
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr bool operator<=(const basic_constexpr_string<CharType, N1> &lhs,
                              const basic_constexpr_string<CharType, N2> &rhs) noexcept {
        return !(rhs < lhs);
    }

    // 大于比较
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr bool operator>(const basic_constexpr_string<CharType, N1> &lhs,
                             const basic_constexpr_string<CharType, N2> &rhs) noexcept {
        return rhs < lhs;
    }

    // 大于等于比较
    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr bool operator>=(const basic_constexpr_string<CharType, N1> &lhs,
                              const basic_constexpr_string<CharType, N2> &rhs) noexcept {
        return !(lhs < rhs);
    }

    // 与 std::string_view 比较的重载
    template <typename CharType, std::size_t N>
    constexpr bool operator==(const basic_constexpr_string<CharType, N> &lhs, std::basic_string_view<CharType> rhs) noexcept {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) == rhs;
    }

    template <typename CharType, std::size_t N>
    constexpr bool operator==(std::basic_string_view<CharType> lhs, const basic_constexpr_string<CharType, N> &rhs) noexcept {
        return lhs == std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    // 与 C 风格字符串比较
    template <typename CharType, std::size_t N>
    constexpr bool operator==(const basic_constexpr_string<CharType, N> &lhs, const CharType *rhs) noexcept {
        return std::basic_string_view<CharType>(lhs.data(), lhs.length()) == std::basic_string_view<CharType>(rhs);
    }

    template <typename CharType, std::size_t N>
    constexpr bool operator==(const CharType *lhs, const basic_constexpr_string<CharType, N> &rhs) noexcept {
        return std::basic_string_view<CharType>(lhs) == std::basic_string_view<CharType>(rhs.data(), rhs.length());
    }

    template <std::size_t N>
    using constexpr_string = basic_constexpr_string<char, N>;

    template <typename CharType, std::size_t N>
    constexpr auto make_constexpr_string(const CharType (&str)[N]) {
        return basic_constexpr_string<CharType, N>(str);
    }

    template <typename CharType, std::size_t N>
    basic_constexpr_string(const CharType (&)[N]) -> basic_constexpr_string<CharType, N>;

    template <typename CharType, std::size_t N1, std::size_t N2>
    constexpr auto concat(const basic_constexpr_string<CharType, N1> &lhs, const basic_constexpr_string<CharType, N2> &rhs) {
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

namespace rainy::utility {
    struct ignore_static_warning_tag {};

    struct require_static_warning_tag {};
}

#define rainy_make_template_warning(cond, msg)                                                                                        \
    typename = ::rainy::utility::static_warning<(cond), ::rainy::type_traits::helper::constexpr_string{msg}>::type

namespace rainy::core {
    template <typename Iter>
    RAINY_CONSTEXPR Iter max_element(Iter first, Iter end) {
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

    template <typename Iter, typename Pred>
    RAINY_CONSTEXPR Iter max_element(Iter first, Iter end, Pred pred) {
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

    template <typename Iter>
    RAINY_CONSTEXPR Iter min_element(Iter first, Iter end) {
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

    template <typename Iter, typename Pred>
    RAINY_CONSTEXPR Iter min_element(Iter first, Iter end, Pred pred) {
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

    template <typename Ty>
    RAINY_CONSTEXPR const Ty &(max) (const Ty &a, const Ty &b) {
        return (a < b) ? b : a;
    }

    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR const Ty &(max) (const Ty &a, const Ty &b, Pred comp) {
        return (comp(a, b)) ? b : a;
    }

    template <typename Ty>
    RAINY_CONSTEXPR Ty(max)(std::initializer_list<Ty> ilist) {
        return *(max_element(ilist.begin(), ilist.end()));
    }

    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR Ty(max)(std::initializer_list<Ty> ilist, Pred pred) {
        return *(max_element(ilist.begin(), ilist.end(), pred));
    }

    template <typename Ty>
    RAINY_CONSTEXPR const Ty &(min) (const Ty &a, const Ty &b) {
        return (a < b) ? a : b;
    }

    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR const Ty &(min) (const Ty &a, const Ty &b, Pred comp) {
        return (comp(a, b)) ? a : b;
    }

    template <typename Ty>
    RAINY_CONSTEXPR Ty(min)(std::initializer_list<Ty> ilist) {
        return *(min_element(ilist.begin(), ilist.end()));
    }

    template <typename Ty, typename Pred>
    RAINY_CONSTEXPR Ty(min)(std::initializer_list<Ty> ilist, Pred pred) {
        return *(min_element(ilist.begin(), ilist.end(), pred));
    }
}

namespace rainy::core::builtin {
    static RAINY_INLINE bool almost_equal(double p1, double p2) {
        return (std::abs(p1 - p2) * 1000000000000. <= (core::min)(std::abs(p1), std::abs(p2)));
    }
}

namespace rainy::core {
    // 对于32位系统，为了避免内存对齐导致的stack-overrun，额外扩充一段空间用于防止此类问题
    static constexpr inline std::size_t fn_obj_soo_buffer_size = (small_object_num_ptrs - 1) * sizeof(void *);
}

namespace rainy::core {
    struct internal_construct_tag_t {};

    RAINY_INLINE_CONSTEXPR internal_construct_tag_t internal_construct_tag{};

    inline constexpr std::size_t hardware_constructive_interference_size = 64;
    inline constexpr std::size_t hardware_destructive_interference_size = 64;
}

namespace rainy::core::builtin {
    constexpr double huge_val() noexcept {
        return __builtin_huge_val();
    }

    constexpr float huge_valf() noexcept {
        return __builtin_huge_valf();
    }
}

namespace rainy::utility {
    struct auto_deduce_t {
        explicit constexpr auto_deduce_t() = default;
    };

    static constexpr auto_deduce_t auto_deduce{};
}

#define RAINY_TOOLKIT_VERSION                                                                                                         \
    "rainy-toolkit:"                                                                                                         \
    RAINY_STRINGIFY(RAINY_TOOLKIT_PROJECT_MAJOR)                                                                                      \
    "." RAINY_STRINGIFY(RAINY_TOOLKIT_PROJECT_MINOR) "." RAINY_STRINGIFY(RAINY_TOOLKIT_PROJECT_PATCH) ".xaga"

#define RAINY_ABI_BRIDGE_CALL_HANDLER_TOTAL_COUNT 0
#define RAINY_ABI_BRIDGE_CALL_GET_VERSION 1
#define RAINY_ABI_BRIDGE_CALL_GET_FULLVERSION 2
#define RAINY_ABI_BRIDGE_CALL_GET_COMPILE_STANDARD 3
#define RAINY_ABI_BRIDGE_CALL_GET_COMPILE_IDENTIFIER 4
#define RAINY_ABI_BRIDGE_CALL_GET_VERSION_NAME 5

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
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_export_library_context(std::uintptr_t* context);
    RAINY_EXTERN_C RAINY_TOOLKIT_API long rainy_toolkit_is_abi_compatible(std::uintptr_t *context);
}

namespace rainy::type_traits::helper {
    /**
     * @brief 从类型和值生成整型常量。
     * @tparam Ty 类型
     * @tparam Data 值
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
     * @brief 将 bool 用作 Ty 参数的 integral_constant 的显式部分特化
     */
    template <bool Boolean>
    using bool_constant = integral_constant<bool, Boolean>;

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    template <typename>
    struct char_space : integral_constant<char, ' '> {};

    template <>
    struct char_space<wchar_t> : integral_constant<wchar_t, ' '> {};

    template <>
    struct char_space<char16_t> : integral_constant<char16_t, u' '> {};

    template <>
    struct char_space<char32_t> : integral_constant<char32_t, u' '> {};

    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_space_v = char_space<CharType>::value;

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    template <>
    struct char_space<char8_t> : integral_constant<char8_t, ' '> {};
#endif
    template <typename CharType>
    struct char_null : integral_constant<char, '\0'> {};

    template <>
    struct char_null<wchar_t> : integral_constant<wchar_t, '\0'> {};

    template <>
    struct char_null<char16_t> : integral_constant<char16_t, u'\0'> {};

    template <>
    struct char_null<char32_t> : integral_constant<char32_t, u'\0'> {};

#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
    template <>
    struct char_null<char8_t> : integral_constant<char8_t, '\0'> {};
#endif

    template <typename CharType>
    RAINY_INLINE_CONSTEXPR CharType char_null_v = char_null<CharType>::value;

    template <typename CharType>
    RAINY_CONSTEXPR_BOOL is_wchar_t = false;

    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<wchar_t> = true;

    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<const wchar_t> = true;

    template <>
    RAINY_CONSTEXPR_BOOL is_wchar_t<volatile wchar_t> = true;

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

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_void_v = is_same_v<Ty, void>;

    template <typename Ty>
    struct is_void : helper::bool_constant<is_void_v<Ty>> {};
}

namespace rainy::core::builtin {
    template <typename Ty>
    constexpr Ty ceil(Ty a, Ty b) {
        return a / b + ((a % b) != 0 && ((a > 0) == (b > 0)));
    }
}

#endif
