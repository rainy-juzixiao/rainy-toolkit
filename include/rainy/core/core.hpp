/*-------------------------------
文件名： core.hpp
此头文件用于存放核心代码实现，可能存在少量汇编代码，此外还有AVX2指令集的使用
--------------------------------*/
#ifndef RAINY_CORE_HPP
#define RAINY_CORE_HPP
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <new>
#include <random>
#include <typeinfo>

#ifdef __linux__
#include <signal.h>
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
#define RAINY_ENABLE_DEBUG 0
#else
#define RAINY_ENABLE_DEBUG 1
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
#ifdef __clang__
#define RAINY_USING_CLANG 1
#else
#define RAINY_USING_CLANG 0
#endif
#endif

#ifndef RAINY_USING_MSVC
#ifdef _MSC_VER
#define RAINY_USING_MSVC 1
#else
#define RAINY_USING_MSVC 0
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
#define RAINY_USING_WINDOWS true
#else
#define RAINY_USING_WINDOWS 0
#endif

#ifdef __linux__
#define RAINY_USING_LINUX 1
#else
#define RAINY_USING_LINUX false
#endif

#if RAINY_USING_GCC
#define RAINY_AINLINE_NODISCARD RAINY_NODISCARD RAINY_INLINE
#else
#define RAINY_AINLINE_NODISCARD RAINY_NODISCARD RAINY_ALWAYS_INLINE
#endif

#ifdef __AVX2__

#define RAINY_CAN_USING_AVX2 true
#else
#define RAINY_CAN_USING_AVX2 0

#endif

// 启用rainy's toolkit的AVX2支持，这将使得某些函数可以通过AVX2指令集进行较大程度的优化
#define RAINY_USING_AVX2 true

#if RAINY_USING_AVX2

#if !RAINY_CAN_USING_AVX2
static_assert(false, "AVX2 not support!");
#endif

#if RAINY_USING_WINDOWS
#if RAINY_USING_CLANG
#include <intrin0.inl.h>
#else
#include <immintrin.h>
#endif // RAINY_USING_CLANG
#endif

#endif

#if RAINY_HAS_CXX20
#include <compare>
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

#define RAINY_CXX17_DEPRECATED_TYPEDEF RAINY_DEPRECATED_MSG("In C++17, certain parts of the standard library design must adhere to Microsoft-STL specifications, such as deprecated typedef or using declarations. You can define the RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS macro to suppress these and other similar warnings, or define the RAINY_DISABLE_CXX17_DEPRECATED_TYPEDEF macro to disable only specific deprecation warnings.")

#else
#define RAINY_CXX17_DEPRECATED_TYPEDEF
#endif // if !defined(RAINY_DISABLE_CXX17_DEPRECATED_TYPEDEF) || !defined(RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS)

#endif // ifndef RAINY_CXX17_DEPRECATED_TYPEDEF

#endif // RAINY_SILENCE_ALL_CXX17_DEPRECATED_WARNINGS

/* 只有涉及到cast表达式时，我们才会使用这些宏 */
#define rainy_let auto
#define rainy_ref auto &
#define rainy_const const auto
#define rainy_cref const auto &

#endif

#if RAINY_USING_COMMON_SOURCESFILE_BUILD
#define RAINY_HPP_INLINE
#else
#define RAINY_HPP_INLINE RAINY_INLINE
#endif



#include <rainy/core/core_typetraits.hpp>
#include <rainy/core/raw_stringview.hpp>

namespace rainy::information {
    static constexpr internals::raw_string_view<char> libray_name("rainy's toolkit");
    static constexpr internals::raw_string_view<char> creator_name("rainy-juzixiao");
    static constexpr internals::raw_string_view<char> current_version("0.1");
    static constexpr internals::raw_string_view<char> code_name("xaga");

    inline constexpr std::size_t small_object_num_ptrs = 6 + 16 / sizeof(void *);
    inline constexpr std::size_t space_size = (small_object_num_ptrs - 1) * sizeof(void *);

    struct rainy_align_meta {
        std::size_t mask; // 存储加了随机种子的哈希值
        std::size_t seed; // 存储种子
        uintptr_t last;
    };

    static inline constexpr bool can_using_avx2 = RAINY_CAN_USING_AVX2;

    using byte_t = unsigned char;
}

namespace rainy::information::internals {
    constexpr bool is_pow_2(const std::size_t val) noexcept {
        return val != 0 && (val & (val - 1)) == 0;
    }

    template <typename Ty>
    constexpr std::size_t get_size_of_n(const std::size_t count) noexcept {
        constexpr std::size_t type_size = sizeof(Ty);
        constexpr bool overflow_is_possible = type_size > 1;
        if constexpr (overflow_is_possible) {
            constexpr size_t max_possible = static_cast<size_t>(-1) / type_size;
            if (count > max_possible) {
                std::terminate(); // multiply overflow
            }
        }
        return type_size * count;
    }
}

namespace rainy::utility::internals {
    inline constexpr size_t fnv_offset_basis = 14695981039346656037ULL;
    inline constexpr size_t fnv_prime = 1099511628211ULL;

    RAINY_INLINE_NODISCARD std::size_t fnv1a_append_bytes(const std::size_t offset_basis, const unsigned char *const first,
                                                          const std::size_t count) noexcept {
        std::size_t hash = offset_basis;
        for (int i = 0; i < count; ++i) {
            hash ^= static_cast<std::size_t>(first[i]);
            hash *= fnv_prime;
        }
        return hash;
    }
}

#if RAINY_USING_COMMON_SOURCESFILE_BUILD
namespace rainy::information::system_call {
    void debug_break();
    long interlocked_increment(volatile long *value);
    long interlocked_decrement(volatile long *value);
    std::uint32_t iso_volatile_load32(const volatile int *addr);
    bool is_aligned(void *const ptr, const std::size_t alignment);
    void *aligned_malloc_withcheck(const std::size_t size, const std::size_t alignment);
    void aligned_free_withcheck(void *block, const std::size_t alignment, void (*error)(void *error_block) = nullptr);
    void *allocate(const size_t size, const std::size_t alignment);
    void deallocate(void *ptr, const std::size_t size, const std::size_t alignment);
}

namespace rainy::information::internals {
    void stl_internal_check(const bool result, const internal_source_location &source_location = internal_source_location::current());
}
#else
#include <rainy/sources/core.cxx>
#endif

namespace rainy::information {
    template <typename... Test>
    RAINY_CONSTEXPR_BOOL check_format_type =
        (type_traits::type_relations::is_any_convertible_v<type_traits::other_trans::decay_t<Test>, char, int,
                                                                       double, void *, float, long, long long, unsigned int,
                                                                       unsigned long, unsigned long long, const char *> ||
         ...);

    template <typename... Test>
    RAINY_CONSTEXPR_BOOL check_wformat_type =
        (type_traits::type_relations::is_any_convertible_v<type_traits::other_trans::decay_t<Test>, char, int,
                                                                       double, void *, float, long, long long, unsigned int,
                                                                       unsigned long, unsigned long long, const wchar_t *> ||
         ...);
}

#if RAINY_USING_GCC
#include <rainy/core/gnu/typetraits.hpp>

namespace rainy::type_traits::internals::gcc_detail_impl {
    template <typename Ty, _enable_if_t<is_void<Ty>::value, int> = 0>
    void avoid_warning_placeholder() {
    }
}
#endif

#ifndef RAINY_NODISCARD_RAW_PTR_ALLOC
#define RAINY_NODISCARD_RAW_PTR_ALLOC                                                                                                   \
    RAINY_NODISCARD_MSG("This function allocates memory and returns a raw pointer. "                                                    \
                           "Discarding the return value will cause a memory leak.")
#endif

namespace rainy::utility {
    class type_index {
    public:
        type_index(const std::type_info &type_info) noexcept : type_info_ptr(&type_info) {
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            return type_info_ptr->hash_code();
        }

        RAINY_NODISCARD const char *name() const noexcept {
            return type_info_ptr->name();
        }

        RAINY_NODISCARD bool operator==(const type_index &right) const noexcept {
            return *type_info_ptr == *right.type_info_ptr;
        }

#if RAINY_HAS_CXX20
        RAINY_NODISCARD std::strong_ordering operator<=>(const type_index &right) const noexcept {
            if (type_info_ptr == right.type_info_ptr) {
                return std::strong_ordering::equal;
            }
            return std::strcmp(right.type_info_ptr->name(), right.type_info_ptr->name()) <=> 0;
        }
#else
        RAINY_NODISCARD bool operator!=(const type_index &right) const noexcept {
            return !(*this == right);
        }
#endif

        RAINY_NODISCARD bool operator<(const type_index &right) const noexcept {
            return type_info_ptr->before(*right.type_info_ptr);
        }

        RAINY_NODISCARD bool operator>=(const type_index &right) const noexcept {
            return !(*this < right);
        }

        RAINY_NODISCARD bool operator>(const type_index &right) const noexcept {
            return right < *this;
        }

        RAINY_NODISCARD bool operator<=(const type_index &right) const noexcept {
            return !(right < *this);
        }

    private:
        const std::type_info *type_info_ptr;
    };
}

/* 此部分宏由ChatGPT生成 */
#define RAINY_TO_TUPLE_EXPAND_ARGS(N) RAINY_TO_TUPLE_EXPAND_##N
#define RAINY_TO_TUPLE_EXPAND_1 _1
#define RAINY_TO_TUPLE_EXPAND_2 _1, _2
#define RAINY_TO_TUPLE_EXPAND_3 _1, _2, _3
#define RAINY_TO_TUPLE_EXPAND_4 _1, _2, _3, _4
#define RAINY_TO_TUPLE_EXPAND_5 _1, _2, _3, _4, _5
#define RAINY_TO_TUPLE_EXPAND_6 _1, _2, _3, _4, _5, _6
#define RAINY_TO_TUPLE_EXPAND_7 _1, _2, _3, _4, _5, _6, _7
#define RAINY_TO_TUPLE_EXPAND_8 _1, _2, _3, _4, _5, _6, _7, _8
#define RAINY_TO_TUPLE_EXPAND_9 _1, _2, _3, _4, _5, _6, _7, _8, _9
#define RAINY_TO_TUPLE_EXPAND_10 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10
#define RAINY_TO_TUPLE_EXPAND_11 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11
#define RAINY_TO_TUPLE_EXPAND_12 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12
#define RAINY_TO_TUPLE_EXPAND_13 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13
#define RAINY_TO_TUPLE_EXPAND_14 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14
#define RAINY_TO_TUPLE_EXPAND_15 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15
#define RAINY_TO_TUPLE_EXPAND_16 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
#define RAINY_TO_TUPLE_EXPAND_17 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17
#define RAINY_TO_TUPLE_EXPAND_18 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18
#define RAINY_TO_TUPLE_EXPAND_19 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19
#define RAINY_TO_TUPLE_EXPAND_20 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20
#define RAINY_TO_TUPLE_EXPAND_21 RAINY_TO_TUPLE_EXPAND_20, _21
#define RAINY_TO_TUPLE_EXPAND_22 RAINY_TO_TUPLE_EXPAND_21, _22
#define RAINY_TO_TUPLE_EXPAND_23 RAINY_TO_TUPLE_EXPAND_22, _23
#define RAINY_TO_TUPLE_EXPAND_24 RAINY_TO_TUPLE_EXPAND_23, _24
#define RAINY_TO_TUPLE_EXPAND_25 RAINY_TO_TUPLE_EXPAND_24, _25
#define RAINY_TO_TUPLE_EXPAND_26 RAINY_TO_TUPLE_EXPAND_25, _26
#define RAINY_TO_TUPLE_EXPAND_27 RAINY_TO_TUPLE_EXPAND_26, _27
#define RAINY_TO_TUPLE_EXPAND_28 RAINY_TO_TUPLE_EXPAND_27, _28
#define RAINY_TO_TUPLE_EXPAND_29 RAINY_TO_TUPLE_EXPAND_28, _29
#define RAINY_TO_TUPLE_EXPAND_30 RAINY_TO_TUPLE_EXPAND_29, _30
#define RAINY_TO_TUPLE_EXPAND_31 RAINY_TO_TUPLE_EXPAND_30, _31
#define RAINY_TO_TUPLE_EXPAND_32 RAINY_TO_TUPLE_EXPAND_31, _32
#define RAINY_TO_TUPLE_EXPAND_33 RAINY_TO_TUPLE_EXPAND_32, _33
#define RAINY_TO_TUPLE_EXPAND_34 RAINY_TO_TUPLE_EXPAND_33, _34
#define RAINY_TO_TUPLE_EXPAND_35 RAINY_TO_TUPLE_EXPAND_34, _35
#define RAINY_TO_TUPLE_EXPAND_36 RAINY_TO_TUPLE_EXPAND_35, _36
#define RAINY_TO_TUPLE_EXPAND_37 RAINY_TO_TUPLE_EXPAND_36, _37
#define RAINY_TO_TUPLE_EXPAND_38 RAINY_TO_TUPLE_EXPAND_37, _38
#define RAINY_TO_TUPLE_EXPAND_39 RAINY_TO_TUPLE_EXPAND_38, _39
#define RAINY_TO_TUPLE_EXPAND_40 RAINY_TO_TUPLE_EXPAND_39, _40
#define RAINY_TO_TUPLE_EXPAND_41 RAINY_TO_TUPLE_EXPAND_40, _41
#define RAINY_TO_TUPLE_EXPAND_42 RAINY_TO_TUPLE_EXPAND_41, _42
#define RAINY_TO_TUPLE_EXPAND_43 RAINY_TO_TUPLE_EXPAND_42, _43
#define RAINY_TO_TUPLE_EXPAND_44 RAINY_TO_TUPLE_EXPAND_43, _44
#define RAINY_TO_TUPLE_EXPAND_45 RAINY_TO_TUPLE_EXPAND_44, _45
#define RAINY_TO_TUPLE_EXPAND_46 RAINY_TO_TUPLE_EXPAND_45, _46
#define RAINY_TO_TUPLE_EXPAND_47 RAINY_TO_TUPLE_EXPAND_46, _47
#define RAINY_TO_TUPLE_EXPAND_48 RAINY_TO_TUPLE_EXPAND_47, _48
#define RAINY_TO_TUPLE_EXPAND_49 RAINY_TO_TUPLE_EXPAND_48, _49
#define RAINY_TO_TUPLE_EXPAND_50 RAINY_TO_TUPLE_EXPAND_49, _50
#define RAINY_TO_TUPLE_EXPAND_51 RAINY_TO_TUPLE_EXPAND_50, _51
#define RAINY_TO_TUPLE_EXPAND_52 RAINY_TO_TUPLE_EXPAND_51, _52
#define RAINY_TO_TUPLE_EXPAND_53 RAINY_TO_TUPLE_EXPAND_52, _53
#define RAINY_TO_TUPLE_EXPAND_54 RAINY_TO_TUPLE_EXPAND_53, _54
#define RAINY_TO_TUPLE_EXPAND_55 RAINY_TO_TUPLE_EXPAND_54, _55
#define RAINY_TO_TUPLE_EXPAND_56 RAINY_TO_TUPLE_EXPAND_55, _56
#define RAINY_TO_TUPLE_EXPAND_57 RAINY_TO_TUPLE_EXPAND_56, _57
#define RAINY_TO_TUPLE_EXPAND_58 RAINY_TO_TUPLE_EXPAND_57, _58
#define RAINY_TO_TUPLE_EXPAND_59 RAINY_TO_TUPLE_EXPAND_58, _59
#define RAINY_TO_TUPLE_EXPAND_60 RAINY_TO_TUPLE_EXPAND_59, _60
#define RAINY_TO_TUPLE_EXPAND_61 RAINY_TO_TUPLE_EXPAND_60, _61
#define RAINY_TO_TUPLE_EXPAND_62 RAINY_TO_TUPLE_EXPAND_61, _62
#define RAINY_TO_TUPLE_EXPAND_63 RAINY_TO_TUPLE_EXPAND_62, _63
#define RAINY_TO_TUPLE_EXPAND_64 RAINY_TO_TUPLE_EXPAND_63, _64
#define RAINY_TO_TUPLE_EXPAND_65 RAINY_TO_TUPLE_EXPAND_64, _65
#define RAINY_TO_TUPLE_EXPAND_66 RAINY_TO_TUPLE_EXPAND_65, _66
#define RAINY_TO_TUPLE_EXPAND_67 RAINY_TO_TUPLE_EXPAND_66, _67
#define RAINY_TO_TUPLE_EXPAND_68 RAINY_TO_TUPLE_EXPAND_67, _68
#define RAINY_TO_TUPLE_EXPAND_69 RAINY_TO_TUPLE_EXPAND_68, _69
#define RAINY_TO_TUPLE_EXPAND_70 RAINY_TO_TUPLE_EXPAND_69, _70
#define RAINY_TO_TUPLE_EXPAND_71 RAINY_TO_TUPLE_EXPAND_70, _71
#define RAINY_TO_TUPLE_EXPAND_72 RAINY_TO_TUPLE_EXPAND_71, _72
#define RAINY_TO_TUPLE_EXPAND_73 RAINY_TO_TUPLE_EXPAND_72, _73
#define RAINY_TO_TUPLE_EXPAND_74 RAINY_TO_TUPLE_EXPAND_73, _74
#define RAINY_TO_TUPLE_EXPAND_75 RAINY_TO_TUPLE_EXPAND_74, _75
#define RAINY_TO_TUPLE_EXPAND_76 RAINY_TO_TUPLE_EXPAND_75, _76
#define RAINY_TO_TUPLE_EXPAND_77 RAINY_TO_TUPLE_EXPAND_76, _77
#define RAINY_TO_TUPLE_EXPAND_78 RAINY_TO_TUPLE_EXPAND_77, _78
#define RAINY_TO_TUPLE_EXPAND_79 RAINY_TO_TUPLE_EXPAND_78, _79
#define RAINY_TO_TUPLE_EXPAND_80 RAINY_TO_TUPLE_EXPAND_79, _80

/* 用于创建初始化器 */
#define RAINY_INITIALIZER_LIST(N) RAINY_INITIALIZER_LIST_##N
#define RAINY_INITIALIZER_LIST_1 {}
#define RAINY_INITIALIZER_LIST_2 {} ,{}
#define RAINY_INITIALIZER_LIST_3 {} ,{} ,{}
#define RAINY_INITIALIZER_LIST_4 {} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_5 {} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_6 {} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_7 {} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_8 {} ,{} ,{} ,{} ,{} ,{} ,{}, {}
#define RAINY_INITIALIZER_LIST_9 {} ,{} ,{} ,{} ,{} ,{} ,{}, {}, {}
#define RAINY_INITIALIZER_LIST_10 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_11 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}, {}
#define RAINY_INITIALIZER_LIST_12 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_13 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_14 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_15 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_16 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_17 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_18 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_19 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_20 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_21 RAINY_INITIALIZER_LIST_20 ,{}
#define RAINY_INITIALIZER_LIST_22 RAINY_INITIALIZER_LIST_21 ,{}
#define RAINY_INITIALIZER_LIST_23 RAINY_INITIALIZER_LIST_22 ,{}
#define RAINY_INITIALIZER_LIST_24 RAINY_INITIALIZER_LIST_23 ,{}
#define RAINY_INITIALIZER_LIST_25 RAINY_INITIALIZER_LIST_24 ,{}
#define RAINY_INITIALIZER_LIST_26 RAINY_INITIALIZER_LIST_25 ,{}
#define RAINY_INITIALIZER_LIST_27 RAINY_INITIALIZER_LIST_26 ,{}
#define RAINY_INITIALIZER_LIST_28 RAINY_INITIALIZER_LIST_27 ,{}
#define RAINY_INITIALIZER_LIST_29 RAINY_INITIALIZER_LIST_28 ,{}
#define RAINY_INITIALIZER_LIST_30 RAINY_INITIALIZER_LIST_29 ,{}
#define RAINY_INITIALIZER_LIST_31 RAINY_INITIALIZER_LIST_30 ,{}
#define RAINY_INITIALIZER_LIST_32 RAINY_INITIALIZER_LIST_31 ,{}
#define RAINY_INITIALIZER_LIST_33 RAINY_INITIALIZER_LIST_32 ,{}
#define RAINY_INITIALIZER_LIST_34 RAINY_INITIALIZER_LIST_33 ,{}
#define RAINY_INITIALIZER_LIST_35 RAINY_INITIALIZER_LIST_34 ,{}
#define RAINY_INITIALIZER_LIST_36 RAINY_INITIALIZER_LIST_35 ,{}
#define RAINY_INITIALIZER_LIST_37 RAINY_INITIALIZER_LIST_36 ,{}
#define RAINY_INITIALIZER_LIST_38 RAINY_INITIALIZER_LIST_37 ,{}
#define RAINY_INITIALIZER_LIST_39 RAINY_INITIALIZER_LIST_38 ,{}
#define RAINY_INITIALIZER_LIST_40 RAINY_INITIALIZER_LIST_39 ,{}
#define RAINY_INITIALIZER_LIST_41 RAINY_INITIALIZER_LIST_40 ,{}
#define RAINY_INITIALIZER_LIST_42 RAINY_INITIALIZER_LIST_41 ,{}
#define RAINY_INITIALIZER_LIST_43 RAINY_INITIALIZER_LIST_42 ,{}
#define RAINY_INITIALIZER_LIST_44 RAINY_INITIALIZER_LIST_43 ,{}
#define RAINY_INITIALIZER_LIST_45 RAINY_INITIALIZER_LIST_44 ,{}
#define RAINY_INITIALIZER_LIST_46 RAINY_INITIALIZER_LIST_45 ,{}
#define RAINY_INITIALIZER_LIST_47 RAINY_INITIALIZER_LIST_46 ,{}
#define RAINY_INITIALIZER_LIST_48 RAINY_INITIALIZER_LIST_47 ,{}
#define RAINY_INITIALIZER_LIST_49 RAINY_INITIALIZER_LIST_48 ,{}
#define RAINY_INITIALIZER_LIST_50 RAINY_INITIALIZER_LIST_49 ,{}
#define RAINY_INITIALIZER_LIST_51 RAINY_INITIALIZER_LIST_50 ,{}
#define RAINY_INITIALIZER_LIST_52 RAINY_INITIALIZER_LIST_51 ,{}
#define RAINY_INITIALIZER_LIST_53 RAINY_INITIALIZER_LIST_52 ,{}
#define RAINY_INITIALIZER_LIST_54 RAINY_INITIALIZER_LIST_53 ,{}
#define RAINY_INITIALIZER_LIST_55 RAINY_INITIALIZER_LIST_54 ,{}
#define RAINY_INITIALIZER_LIST_56 RAINY_INITIALIZER_LIST_55 ,{}
#define RAINY_INITIALIZER_LIST_57 RAINY_INITIALIZER_LIST_56 ,{}
#define RAINY_INITIALIZER_LIST_58 RAINY_INITIALIZER_LIST_57 ,{}
#define RAINY_INITIALIZER_LIST_59 RAINY_INITIALIZER_LIST_58 ,{}
#define RAINY_INITIALIZER_LIST_60 RAINY_INITIALIZER_LIST_59 ,{}
#define RAINY_INITIALIZER_LIST_61 RAINY_INITIALIZER_LIST_60 ,{}
#define RAINY_INITIALIZER_LIST_62 RAINY_INITIALIZER_LIST_61 ,{}
#define RAINY_INITIALIZER_LIST_63 RAINY_INITIALIZER_LIST_62 ,{}
#define RAINY_INITIALIZER_LIST_64 RAINY_INITIALIZER_LIST_63 ,{}
#define RAINY_INITIALIZER_LIST_65 RAINY_INITIALIZER_LIST_64 ,{}
#define RAINY_INITIALIZER_LIST_66 RAINY_INITIALIZER_LIST_65 ,{}
#define RAINY_INITIALIZER_LIST_67 RAINY_INITIALIZER_LIST_66 ,{}
#define RAINY_INITIALIZER_LIST_68 RAINY_INITIALIZER_LIST_67 ,{}
#define RAINY_INITIALIZER_LIST_69 RAINY_INITIALIZER_LIST_68 ,{}
#define RAINY_INITIALIZER_LIST_70 RAINY_INITIALIZER_LIST_69 ,{}
#define RAINY_INITIALIZER_LIST_71 RAINY_INITIALIZER_LIST_70 ,{}
#define RAINY_INITIALIZER_LIST_72 RAINY_INITIALIZER_LIST_71 ,{}
#define RAINY_INITIALIZER_LIST_73 RAINY_INITIALIZER_LIST_72 ,{}
#define RAINY_INITIALIZER_LIST_74 RAINY_INITIALIZER_LIST_73 ,{}
#define RAINY_INITIALIZER_LIST_75 RAINY_INITIALIZER_LIST_74 ,{}
#define RAINY_INITIALIZER_LIST_76 RAINY_INITIALIZER_LIST_75 ,{}
#define RAINY_INITIALIZER_LIST_77 RAINY_INITIALIZER_LIST_76 ,{}
#define RAINY_INITIALIZER_LIST_78 RAINY_INITIALIZER_LIST_77 ,{}
#define RAINY_INITIALIZER_LIST_79 RAINY_INITIALIZER_LIST_78 ,{}
#define RAINY_INITIALIZER_LIST_80 RAINY_INITIALIZER_LIST_79 ,{}

#define RAINY_OVERLOAD_METHOD(QUAL)                                                                                                   \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_method(ReturnType (ClassType::*method)(Args...) QUAL) {                                                   \
        return method;                                                                                                                \
    }

#define RAINY_OVERLOAD_NOEXCEPT_METHOD(QUAL)                                                                                          \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_noexcept_method(ReturnType (ClassType::*noexcept_method)(Args...) QUAL) {                                 \
        return noexcept_method;                                                                                                       \
    }

#define RAINY_OVERLOAD_CONST_METHOD(QUAL)                                                                                             \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_cmethod(ReturnType (ClassType::*cmethod)(Args...) QUAL) {                                                 \
        return cmethod;                                                                                                               \
    }

#define RAINY_OVERLOAD_VOLATILE_METHOD(QUAL)                                                                                          \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_volatile_method(ReturnType (ClassType::*volatile_method)(Args...) QUAL) {                                 \
        return volatile_method;                                                                                                       \
    }

#define RAINY_OVERLOAD_CV_METHOD(QUAL) template <typename ClassType, typename ReturnType, typename... Args>\
constexpr auto overload_cv_method(ReturnType (ClassType::*cv_method)(Args...) const volatile QUAL) {\
    return cv_method;\
}

namespace rainy::utility {
    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_method(ReturnType (ClassType::*method)(Args...)) {
        return method;
    }

    // 使用宏批量生成代码，虽然牺牲了可读性，但是是必要的。避免过多代码
    RAINY_OVERLOAD_METHOD(&)
    RAINY_OVERLOAD_METHOD(&&)
    RAINY_OVERLOAD_METHOD(volatile)
    RAINY_OVERLOAD_METHOD(volatile &)
    RAINY_OVERLOAD_METHOD(volatile &&)
    RAINY_OVERLOAD_METHOD(noexcept)
    RAINY_OVERLOAD_METHOD(& noexcept)
    RAINY_OVERLOAD_METHOD(&& noexcept)
    RAINY_OVERLOAD_METHOD(volatile noexcept)
    RAINY_OVERLOAD_METHOD(volatile & noexcept)
    RAINY_OVERLOAD_METHOD(volatile && noexcept)
    RAINY_OVERLOAD_METHOD(const &)
    RAINY_OVERLOAD_METHOD(const &&)
    RAINY_OVERLOAD_METHOD(const volatile)
    RAINY_OVERLOAD_METHOD(const volatile &)
    RAINY_OVERLOAD_METHOD(const volatile &&)
    RAINY_OVERLOAD_METHOD(const noexcept)
    RAINY_OVERLOAD_METHOD(const & noexcept)
    RAINY_OVERLOAD_METHOD(const && noexcept)
    RAINY_OVERLOAD_METHOD(const volatile noexcept)
    RAINY_OVERLOAD_METHOD(const volatile & noexcept)
    RAINY_OVERLOAD_METHOD(const volatile && noexcept)

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_cmethod(ReturnType (ClassType::*cmethod)(Args...) const) {
        return cmethod;
    }

    RAINY_OVERLOAD_CONST_METHOD(const &)
    RAINY_OVERLOAD_CONST_METHOD(const &&)
    RAINY_OVERLOAD_CONST_METHOD(const volatile)
    RAINY_OVERLOAD_CONST_METHOD(const volatile &)
    RAINY_OVERLOAD_CONST_METHOD(const volatile &&)
    RAINY_OVERLOAD_CONST_METHOD(const noexcept)
    RAINY_OVERLOAD_CONST_METHOD(const & noexcept)
    RAINY_OVERLOAD_CONST_METHOD(const && noexcept)
    RAINY_OVERLOAD_CONST_METHOD(const volatile noexcept)
    RAINY_OVERLOAD_CONST_METHOD(const volatile & noexcept)
    RAINY_OVERLOAD_CONST_METHOD(const volatile && noexcept)

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_volatile_method(ReturnType (ClassType::*volatile_method)(Args...) volatile) {
        return volatile_method;
    }

    RAINY_OVERLOAD_VOLATILE_METHOD(volatile &)
    RAINY_OVERLOAD_VOLATILE_METHOD(volatile &&)
    RAINY_OVERLOAD_VOLATILE_METHOD(volatile noexcept)
    RAINY_OVERLOAD_VOLATILE_METHOD(volatile & noexcept)
    RAINY_OVERLOAD_VOLATILE_METHOD(volatile && noexcept)
    RAINY_OVERLOAD_VOLATILE_METHOD(const volatile)
    RAINY_OVERLOAD_VOLATILE_METHOD(const volatile &)
    RAINY_OVERLOAD_VOLATILE_METHOD(const volatile &&)
    RAINY_OVERLOAD_VOLATILE_METHOD(const volatile noexcept)
    RAINY_OVERLOAD_VOLATILE_METHOD(const volatile & noexcept)
    RAINY_OVERLOAD_VOLATILE_METHOD(const volatile && noexcept)

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_noexcept_method(ReturnType (ClassType::*noexcept_method)(Args...) noexcept) {
        return noexcept_method;
    }

    RAINY_OVERLOAD_NOEXCEPT_METHOD(& noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(&& noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(const noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(const & noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(const && noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(const volatile noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(const volatile & noexcept)
    RAINY_OVERLOAD_NOEXCEPT_METHOD(const volatile && noexcept)

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_cv_method(ReturnType (ClassType::*cv_method)(Args...) const volatile) {
        return cv_method;
    }

    RAINY_OVERLOAD_CV_METHOD(&)
    RAINY_OVERLOAD_CV_METHOD(&&)
    RAINY_OVERLOAD_CV_METHOD(noexcept)
    RAINY_OVERLOAD_CV_METHOD(& noexcept)
    RAINY_OVERLOAD_CV_METHOD(&& noexcept)

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_static_method(ReturnType (*method)(Args...)) {
        return method;
    }
}

// 阻止滥用
#undef RAINY_OVERLOAD_METHOD
#undef RAINY_OVERLOAD_CONST_METHOD
#undef RAINY_OVERLOAD_VOLATILE_METHOD
#undef RAINY_OVERLOAD_NOEXCEPT_METHOD

#define RAINY_OVERLOAD_LEFT_METHOD(QUAL)                                                                                                   \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_left_method(ReturnType (ClassType::*method)(Args...) QUAL) {                                                   \
        return method;                                                                                                                \
    }

#define RAINY_OVERLOAD_LEFT_CONST_METHOD(QUAL)                                                                                             \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_left_cmethod(ReturnType (ClassType::*cmethod)(Args...) QUAL) {                                                 \
        return cmethod;                                                                                                               \
    }

#define RAINY_OVERLOAD_LEFT_VOLATILE_METHOD(QUAL)                                                                                          \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_left_volatile_method(ReturnType (ClassType::*volatile_method)(Args...) QUAL) {                                 \
        return volatile_method;                                                                                                       \
    }

#define RAINY_OVERLOAD_LEFT_NOEXCEPT_METHOD(QUAL)                                                                                     \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_left_noexcept_method(ReturnType (ClassType::*noexcept_method)(Args...) QUAL) {                                 \
        return noexcept_method;                                                                                                       \
    }


#define RAINY_OVERLOAD_LEFT_CV_METHOD(QUAL)                                                                                                \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_left_cv_method(ReturnType (ClassType::*cv_method)(Args...) const volatile QUAL) {                                   \
        return cv_method;                                                                                                             \
    }

namespace rainy::utility {
    RAINY_OVERLOAD_LEFT_METHOD(&)
    RAINY_OVERLOAD_LEFT_METHOD(volatile &)
    RAINY_OVERLOAD_LEFT_METHOD(volatile & noexcept)
    RAINY_OVERLOAD_LEFT_METHOD(const &)
    RAINY_OVERLOAD_LEFT_METHOD(const volatile &)
    RAINY_OVERLOAD_LEFT_METHOD(const & noexcept)
    RAINY_OVERLOAD_LEFT_METHOD(const volatile & noexcept)

    RAINY_OVERLOAD_LEFT_CONST_METHOD(const &)
    RAINY_OVERLOAD_LEFT_CONST_METHOD(const volatile &)
    RAINY_OVERLOAD_LEFT_CONST_METHOD(const & noexcept)
    RAINY_OVERLOAD_LEFT_CONST_METHOD(const volatile & noexcept)
    
    RAINY_OVERLOAD_LEFT_VOLATILE_METHOD(volatile &)
    RAINY_OVERLOAD_LEFT_VOLATILE_METHOD(volatile & noexcept)
    RAINY_OVERLOAD_LEFT_VOLATILE_METHOD(const volatile &)
    RAINY_OVERLOAD_LEFT_VOLATILE_METHOD(const volatile & noexcept)
    
    RAINY_OVERLOAD_LEFT_NOEXCEPT_METHOD(& noexcept)
    RAINY_OVERLOAD_LEFT_NOEXCEPT_METHOD(const & noexcept)
    RAINY_OVERLOAD_LEFT_NOEXCEPT_METHOD(const volatile & noexcept)
    
    RAINY_OVERLOAD_LEFT_CV_METHOD(&)
    RAINY_OVERLOAD_LEFT_CV_METHOD(& noexcept)
}

#undef RAINY_OVERLOAD_LEFT_METHOD
#undef RAINY_OVERLOAD_LEFT_CONST_METHOD
#undef RAINY_OVERLOAD_LEFT_VOLATILE_METHOD
#undef RAINY_OVERLOAD_LEFT_NOEXCEPT_METHOD

#define RAINY_OVERLOAD_RIGHT_METHOD(QUAL)                                                                                              \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_right_method(ReturnType (ClassType::*method)(Args...) QUAL) {                                              \
        return method;                                                                                                                \
    }

#define RAINY_OVERLOAD_RIGHT_CONST_METHOD(QUAL)                                                                                        \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_right_cmethod(ReturnType (ClassType::*cmethod)(Args...) QUAL) {                                            \
        return cmethod;                                                                                                               \
    }

#define RAINY_OVERLOAD_RIGHT_VOLATILE_METHOD(QUAL)                                                                                     \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_right_volatile_method(ReturnType (ClassType::*volatile_method)(Args...) QUAL) {                            \
        return volatile_method;                                                                                                       \
    }

#define RAINY_OVERLOAD_RIGHT_NOEXCEPT_METHOD(QUAL)                                                                                     \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_right_noexcept_method(ReturnType (ClassType::*noexcept_method)(Args...) QUAL) {                            \
        return noexcept_method;                                                                                                       \
    }


#define RAINY_OVERLOAD_RIGHT_CV_METHOD(QUAL)                                                                                           \
    template <typename ClassType, typename ReturnType, typename... Args>                                                              \
    constexpr auto overload_right_cv_method(ReturnType (ClassType::*cv_method)(Args...) const volatile QUAL) {                         \
        return cv_method;                                                                                                             \
    }

namespace rainy::utility {
    RAINY_OVERLOAD_RIGHT_METHOD(&&)
    RAINY_OVERLOAD_RIGHT_METHOD(volatile &&)
    RAINY_OVERLOAD_RIGHT_METHOD(&& noexcept)
    RAINY_OVERLOAD_RIGHT_METHOD(volatile && noexcept)
    RAINY_OVERLOAD_RIGHT_METHOD(const &&)
    RAINY_OVERLOAD_RIGHT_METHOD(const volatile &&)
    RAINY_OVERLOAD_RIGHT_METHOD(const && noexcept)
    RAINY_OVERLOAD_RIGHT_METHOD(const volatile && noexcept)
    RAINY_OVERLOAD_RIGHT_CONST_METHOD(const &&)
    RAINY_OVERLOAD_RIGHT_CONST_METHOD(const volatile &&)
    RAINY_OVERLOAD_RIGHT_CONST_METHOD(const && noexcept)
    RAINY_OVERLOAD_RIGHT_CONST_METHOD(const volatile && noexcept)
    RAINY_OVERLOAD_RIGHT_VOLATILE_METHOD(volatile &&)
    RAINY_OVERLOAD_RIGHT_VOLATILE_METHOD(volatile && noexcept)
    RAINY_OVERLOAD_RIGHT_VOLATILE_METHOD(const volatile &&)
    RAINY_OVERLOAD_RIGHT_VOLATILE_METHOD(const volatile && noexcept)
    RAINY_OVERLOAD_RIGHT_NOEXCEPT_METHOD(&& noexcept)
    RAINY_OVERLOAD_RIGHT_NOEXCEPT_METHOD(const && noexcept)
    RAINY_OVERLOAD_RIGHT_NOEXCEPT_METHOD(const volatile && noexcept)
    RAINY_OVERLOAD_RIGHT_CV_METHOD(&&)
    RAINY_OVERLOAD_RIGHT_CV_METHOD(&& noexcept)
}

#undef RAINY_OVERLOAD_RIGHT_METHOD
#undef RAINY_OVERLOAD_RIGHT_CONST_METHOD
#undef RAINY_OVERLOAD_RIGHT_VOLATILE_METHOD
#undef RAINY_OVERLOAD_RIGHT_NOEXCEPT_METHOD

namespace rainy::utility {
    struct invalid_type {};
}

#define RAINY_DECLARE_SIGNLE_INSTANCE(CLASSNAME) static CLASSNAME &instance() noexcept { static CLASSNAME instance;return instance;}

namespace rainy::information::internals {
    constexpr static raw_string_view<char> token_charset("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
}

#include <thread>

namespace rainy::information {
    static const std::size_t max_threads = std::thread::hardware_concurrency();
}

#if RAINY_HAS_CXX20
#include <format>
namespace rainy::type_traits::concepts {
    template <typename Ty1, typename Ty2>
    concept same_as = internals::_is_same_v<Ty1, Ty2>;

    template <typename base, typename derived>
    concept derived_from =
        __is_base_of(base, derived) && type_relations::is_convertible_v<const volatile derived *, const volatile base *>;

    template <typename Ty, typename Context = std::format_context,
              typename Formatter = Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with = std::semiregular<Formatter> &&
                               requires(Formatter &formatter, const Formatter &const_formatter, Ty &&type, Context format_context,
                                        std::basic_format_parse_context<typename Context::char_type> parse_context) {
                                   { formatter.parse(parse_context) } -> same_as<typename decltype(parse_context)::iterator>;
                                   { const_formatter.format(type, format_context) } -> same_as<typename Context::iterator>;
                               };

    template <typename Ty, typename Context = std::format_context,
              typename Formatter = Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with_non_const =
        std::semiregular<Formatter> && requires(Formatter &formatter, Ty &&type, Context format_context,
                                                std::basic_format_parse_context<typename Context::char_type> parse_context) {
            { formatter.parse(format_context) } -> std::same_as<typename decltype(parse_context)::iterator>;
            { formatter.format(type, format_context) } -> std::same_as<typename Context::iterator>;
        };
}
#endif

namespace rainy::foundation::system::memory {
    template <typename Ty>
    struct default_deleter {
        constexpr default_deleter() noexcept = default;

        constexpr default_deleter(utility::placeholder) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        RAINY_CONSTEXPR20 void operator()(const Ty *resource) const noexcept {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type"); // NOLINT
            delete resource;
        }
    };

    template <typename Ty>
    struct default_deleter<Ty[]> {
        constexpr default_deleter() = default;

        constexpr default_deleter(utility::placeholder) noexcept {
        }

        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        template <typename U, type_traits::other_trans::enable_if_t<std::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 void operator()(const U *resource) const noexcept {
            static_assert(!std::is_abstract_v<U>, "can't delete an incomplete type");
            delete[] resource;
        }
    };

    template <typename Ty>
    struct no_delete {
        constexpr no_delete() noexcept = default;

        constexpr no_delete(utility::placeholder) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        RAINY_CONSTEXPR20 void operator()(const Ty *resource) const noexcept {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type"); // NOLINT
        }
    };

    template <typename Ty>
    struct no_delete<Ty[]> {
        constexpr no_delete() = default;

        constexpr no_delete(utility::placeholder) noexcept {
        }

        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 void operator()(const U *resource) const noexcept {
            static_assert(!std::is_abstract_v<U>, "can't delete an incomplete type");
        }
    };
}

namespace rainy::utility::internals {
    template <typename Dx, typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_deleter_invocable_v = false;

    template <typename Dx, typename Ty>
    RAINY_CONSTEXPR_BOOL
        is_deleter_invocable_v<Dx, Ty, type_traits::other_trans::void_t<decltype(utility::declval<Dx>()(utility::declval<Ty *>()))>> =
            true;
}

namespace rainy::utility {
    template <typename Ty, typename Dx = foundation::system::memory::default_deleter<Ty>>
    class scope_guard {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        static_assert(!type_traits::type_relations::is_same_v<Ty, std::nullptr_t>, "Ty should not be nullptr_t");

        RAINY_CONSTEXPR20 scope_guard() = default;

        scope_guard(const scope_guard &) = delete;
        scope_guard(scope_guard &&) = delete;
        scope_guard &operator=(const scope_guard &) = delete;
        scope_guard &operator=(scope_guard &&) = delete;
        scope_guard(std::nullptr_t) = delete;

        RAINY_CONSTEXPR20 scope_guard(pointer data) : data_({}, data) {
            check_null(data);
        }

        RAINY_CONSTEXPR20 scope_guard(pointer data, Dx deleter) : data_(deleter, data) {
            check_null(data);
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          internals::is_deleter_invocable_v<Dx, Ty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 scope_guard(Uty *data) : data_({}, data) {
            check_null(data);
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          internals::is_deleter_invocable_v<Dx, Ty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 scope_guard(Uty *data, Dx deleter) : data_(deleter, data) {
            check_null(data);
        }

        RAINY_CONSTEXPR20 ~scope_guard() {
            reset();
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          internals::is_deleter_invocable_v<Dx, Uty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 void reset(Uty *new_data) {
            pointer release_data = get();
            if (release_data) {
                data_.get_first()(release_data);
            }
            data_.get_second() = new_data;
        }

        template <type_traits::other_trans::enable_if_t<internals::is_deleter_invocable_v<Dx, Ty>,int> = 0>
        RAINY_CONSTEXPR20 void reset(Ty *new_data = nullptr) {
            pointer release_data = get();
            if (release_data) {
                data_.get_first()(release_data);
            }
            data_.get_second() = new_data;
        }

        RAINY_CONSTEXPR20 pointer release() {
            return utility::exchange(data_.get_second(), nullptr);
        }

        RAINY_CONSTEXPR20 pointer get() {
            return data_.get_second();
        }

        RAINY_CONSTEXPR20 const_pointer get() const {
            return data_.get_second();
        }

    private:
        template <typename Type>
        RAINY_CONSTEXPR20 static void check_null(Type *data) {
            if (!data) {
                information::system_call::debug_break();
            }
        }

        utility::compressed_pair<Dx, pointer> data_{};
    };
}

namespace rainy::utility {
    template <typename key>
    struct hash;

    namespace internals {
        template <typename Key>
        RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_value(const std::size_t offset_basis, const Key &keyval) noexcept {
            static_assert(type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
            return fnv1a_append_bytes(offset_basis, &reinterpret_cast<const unsigned char &>(keyval), sizeof(Key));
        }

        template <typename Ty>
        RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_range(const std::size_t offset_basis, const Ty *const first,
                                                               const Ty *const last) {
            static_assert(type_traits::type_properties::is_trivial_v<Ty>, "Only trivial types can be directly hashed.");
            const auto *const first_binary = reinterpret_cast<const unsigned char *>(first);
            const auto *const last_binary = reinterpret_cast<const unsigned char *>(last);
            return fnv1a_append_bytes(offset_basis, first_binary, static_cast<size_t>(last_binary - first_binary));
        }

        template <typename Key>
        RAINY_AINLINE_NODISCARD std::size_t hash_representation(const Key &keyval) noexcept {
            return fnv1a_append_value(fnv_offset_basis, keyval);
        }

        template <typename Key>
        RAINY_AINLINE_NODISCARD std::size_t hash_array_representation(const Key *const first, const std::size_t count) noexcept {
            static_assert(type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
            return fnv1a_append_bytes(fnv_offset_basis, reinterpret_cast<const unsigned char *>(first), count * sizeof(Key));
        }

        /**
         *
         * @tparam key
         * @tparam check 如果为真，此模板将启用
         */
        template <typename key, bool check>
        struct hash_enable_if {
            using argument_type = key;
            using result_type = std::size_t;

            RAINY_AINLINE_NODISCARD result_type operator()(const argument_type &val) const
                noexcept(noexcept(hash<key>::hash_this_val(val))) {
                return hash<key>::hash_this_val(val);
            }
        };

        template <typename key>
        struct hash_enable_if<key, false> {
            hash_enable_if() = delete;
            hash_enable_if(const hash_enable_if &) = delete;
            hash_enable_if(hash_enable_if &&) = delete;
            hash_enable_if &operator=(const hash_enable_if &) = delete;
            hash_enable_if &operator=(hash_enable_if &&) = delete;
        };
    }

    template <typename key>
    struct hash : internals::hash_enable_if<
                      key, !type_traits::type_properties::is_const_v<key> && !type_traits::type_properties::is_volatile_v<key> &&
                               (type_traits::primary_types::is_enum_v<key> || type_traits::primary_types::is_integral_v<key> ||
                                type_traits::primary_types::is_pointer_v<key>)> {
        static size_t hash_this_val(const key &keyval) noexcept {
            return internals::hash_representation(keyval);
        }
    };

    template <>
    struct hash<float> {
        using argument_type = float;
        using result_type = std::size_t;


        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_representation(val == 0.0f ? 0.0f : val);
        }
    };

    template <>
    struct hash<double> {
        using argument_type = double;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_representation(val == 0.0 ? 0.0 : val);
        }
    };

    template <>
    struct hash<long double> {
        using argument_type = long double;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_representation(val == 0.0L ? 0.0L : val);
        }
    };
}

namespace rainy::utility {
    struct monostate : std::monostate {};
}

#endif
