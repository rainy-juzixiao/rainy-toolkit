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
#define RAINY_STRINGIZE(s) #s
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

// 启用juzixioa's libray的AVX2支持，这将使得某些函数可以通过AVX2指令集进行较大程度的优化
#define RAINY_USING_AVX2 false

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

#if RAINY_ENABLE_DEBUG
#ifndef RAINY_NODEBUG_CONSTEXPR
#define RAINY_NODEBUG_CONSTEXPR
#endif
#else
#ifndef RAINY_NODEBUG_CONSTEXPR
#define RAINY_NODEBUG_CONSTEXPR constexpr
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

#include <rainy/internals/core_typetraits.hpp>
#include <rainy/internals/raw_stringview.hpp>

#if RAINY_HAS_CXX20
namespace rainy::containers {
    enum class memory_order : int {
        relaxed,
        consume,
        acquire,
        release,
        acq_rel,
        seq_cst,

        memory_order_relaxed = relaxed,
        memory_order_consume = consume,
        memory_order_acquire = acquire,
        memory_order_release = release,
        memory_order_acq_rel = acq_rel,
        memory_order_seq_cst = seq_cst
    };

    inline auto memory_order_relaxed = memory_order::relaxed;
    inline auto memory_order_consume = memory_order::consume;
    inline auto memory_order_acquire = memory_order::acquire;
    inline auto memory_order_release = memory_order::release;
    inline auto memory_order_acq_rel = memory_order::acq_rel;
    inline auto memory_order_seq_cst = memory_order::seq_cst;
}
#else
namespace rainy::containers {
    enum memory_order {
        memory_order_relaxed,
        memory_order_consume,
        memory_order_acquire,
        memory_order_release,
        memory_order_acq_rel,
        memory_order_seq_cst
    };
}
#endif

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
        // Process the remaining bytes
        for (int i = 0; i < count; ++i) {
            hash ^= static_cast<std::size_t>(first[i]);
            hash *= fnv_prime;
        }
        return hash;
    }
}

/* 此处封装系统调用指令（asm指令）以及底层功能函数 */
namespace rainy::information::system_call {
    RAINY_INLINE void debug_break() {
#if RAINY_USING_MSVC // 微软编译器
        __debugbreak(); // 使用微软提供的断点指令
#elif RAINY_USING_GCC || RAINY_USING_CLANG // GNU编译器 或 clang编译器

#if RAINY_USING_WINDOWS
        // 使用__volatile__表示此指令不可被优化

#if defined(__i386__) || defined(__x86_64__)
        __asm__ __volatile__("int {$}3" :);
#elif defined(__arm__)
        __asm__ __volatile__("udf #0xfe");
#elif defined(__aarch64__)
        __asm__ __volatile__("brk #0xf000");
#else
        __asm__ __volatile__("unimplemented");
#endif

#elif RAINY_USING_LINUX
        __asm__ __volatile__("int $0x3");
#else // else defined(RAINY_USING_LINUX)
        static_assert(false, "rainy's library only supports Windows and Linux platforms");
#endif // elif defined(RAINY_USING_LINUX)

#endif // elif RAINY_USING_WINDOWS
    }

    RAINY_INLINE long interlocked_increment(volatile long *value) {
#if RAINY_USING_MSVC
        return _InterlockedIncrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        // 由于clang-tidy可能无法理解汇编代码，认为value指针可以为常量，因此需要使用中间变量去除警告
        long result = 0;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clangtidy), "=a"(result) : : "cc");
        return result;
#else
        static_assert(false, "rainy's library only supports GCC Clang and MSVC platforms");
#endif
    }

    RAINY_INLINE long interlocked_decrement(volatile long *value) {
#if RAINY_USING_MSVC
        return _InterlockedDecrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        // 由于clang-tidy可能无法理解汇编代码，认为value指针可以为常量，因此需要使用中间变量去除警告
        long result = 0;
        __asm__ __volatile__("lock; decl %0" : "+m"(*avoid_clangtidy), "=a"(result) : : "cc");
        return result;
#else
        static_assert(false, "rainy's library only supports GCC Clang and MSVC platforms");
#endif
    }

    RAINY_INLINE uint32_t iso_volatile_load32(const volatile int *addr) {
#if RAINY_USING_MSVC
        return __iso_volatile_load32(addr);
#else
        uint32_t value = 0;
        __asm__ __volatile__("lfence\n movl (%1), %0" : "=r"(value) : "r"(addr) : "memory");
        return value;
#endif
    }

    RAINY_INLINE bool is_aligned(void *const ptr, const std::size_t alignment) {
        return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
    }

    RAINY_INLINE void *aligned_malloc_withcheck(const std::size_t size, const std::size_t alignment) {
        using namespace utility::internals;
        const std::size_t offset = alignment - 1 + sizeof(rainy_align_meta);
        const std::size_t total = size + offset;
        void *raw = operator new[](total);
        if (!raw) {
            return nullptr; // 交由上层负责
        }
        rainy_const raw_location = reinterpret_cast<std::size_t>(raw);
        const std::size_t real_location = (raw_location + offset) & ~(alignment - 1);
        rainy_let real_pointer = reinterpret_cast<void *>(real_location);
        const std::size_t raw_storage = real_location - sizeof(rainy_align_meta);
        rainy_let end = reinterpret_cast<void *>(raw_storage + total);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::size_t> dis(1, 0xFF);
        const std::size_t random_mask = dis(gen);
        // 计算哈希并应用随机掩码
        const std::size_t check =
            fnv1a_append_bytes(fnv_offset_basis, reinterpret_cast<const unsigned char *>(raw), total) | random_mask;
        *reinterpret_cast<rainy_align_meta *>(raw_storage) =
            rainy_align_meta{check, random_mask, reinterpret_cast<uintptr_t>(end)};
        return real_pointer;
    }

    RAINY_INLINE void aligned_free_withcheck(void *block, const std::size_t alignment, void (*error)(void *error_block) = nullptr) {
        if (!block) {
            return;
        }
        if (is_aligned(block, alignment)) {
            using namespace utility;
            const std::size_t offset = alignment - 1 + sizeof(rainy_align_meta);
            rainy_const real_location = reinterpret_cast<std::size_t>(block);
            const std::size_t raw_location = real_location - offset;
            rainy_let raw = reinterpret_cast<void *>(raw_location);
            rainy_const p = static_cast<rainy_align_meta *>(raw);
            if ((p->mask & p->seed) != p->seed) {
                if (error) {
                    error(raw);
                }
                operator delete[](block); // 尝试释放内存以防止泄漏
                return;
            }
            const std::size_t free_size = p->last - reinterpret_cast<uintptr_t>(raw);
            operator delete[](raw, free_size);
        }
    }

    RAINY_INLINE void *allocate(const size_t size, const std::size_t alignment) {
        if (size == 0) {
            return nullptr;
        }
        try {
#ifdef __cpp_aligned_new
            return operator new[](size, std::align_val_t{alignment});
#else
            const std::size_t offset = Alignment - 1 + sizeof(void *);
            const std::size_t total = size + offset;
            void *ptr = operator new[](total);
            rainy_const raw_location = reinterpret_cast<std::size_t>(ptr);
            const std::size_t aligned_location = (raw_location + offset) & ~(Alignment - 1);
            rainy_let aligned_ptr = reinterpret_cast<void *>(aligned_location);
            *(reinterpret_cast<void **>(aligned_location - sizeof(void *))) = ptr;
            return aligned_ptr;
#endif
        } catch (std::bad_alloc &) {
            information::internals::raw_string_view<char> view("error in alloc memory");
            (void) std::fwrite(view.c_str(), sizeof(char), view.size(), stdout);
            std::terminate();
        }
    }

    RAINY_INLINE void deallocate(void *block, const std::size_t alignment) {
        if (!block) {
            return;
        }
#ifdef __cpp_aligned_new
        operator delete[](block, std::align_val_t{alignment});
#else
        if (is_aligned(block, alignment)) {
            const std::size_t aligned_location = reinterpret_cast<std::size_t>(block);
            void *original_ptr = *(reinterpret_cast<void **>(aligned_location - sizeof(void *)));
            operator delete[](original_ptr);
        }
#endif
    }

    RAINY_INLINE void deallocate(void *block, const std::size_t byte_count, const std::size_t alignment) {
        if (!block || byte_count == 0) {
            return;
        }
#ifdef __cpp_aligned_new
        operator delete[](block, byte_count, std::align_val_t{alignment});
#else
        if (is_aligned(block, alignment)) {
            const std::size_t aligned_location = reinterpret_cast<std::size_t>(block);
            void *original_ptr = *(reinterpret_cast<void **>(aligned_location - sizeof(void *)));
            operator delete[](original_ptr, byte_count);
        }
#endif
    }
}

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
#include <rainy/internals/gnu/typetraits.hpp>

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

namespace rainy::information::internals {
    RAINY_INLINE void stl_internal_check(const bool result,
                                            const internal_source_location &source_location = internal_source_location::current()) {
        if (!result) {
            bool release{false};
            constexpr static std::size_t static_memory_size = 120;
            thread_local char static_memory[static_memory_size]; // For Thread Safety
            rainy_let buffer = static_cast<char *>(static_memory);
            rainy_let required_size =
                1 + std::snprintf(nullptr, 0, "%s:%du in function '%s' found a error\n", source_location.file_name(),
                                  source_location.line(), source_location.function_name());
            if (required_size > static_memory_size) {
                try {
                    buffer =
                        static_cast<char *>(system_call::allocate(sizeof(char) * required_size,
                                                                  alignof(char))); // 通过operator new[]获取内存，而不是通过new char
                    release = true;
                } catch (std::bad_alloc &) {
                    // 即使无法分配也要强行退出程序（无论如何）
                    std::abort();
                }
            }
            required_size = std::snprintf(buffer, required_size, "%s:%u in function '%s' found a error\n", source_location.file_name(),
                                          source_location.line(), source_location.function_name());
            (void) std::fwrite(buffer, sizeof(char), required_size, stderr);
            if (release) {
                operator delete[](buffer, required_size);
            }
            std::abort();
        }
    }
}

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

namespace rainy::utility {
    template <typename Ty>
    RAINY_NODISCARD Ty *addressof(Ty &val) noexcept {
        return __builtin_addressof(val);
    }

    template <typename Ty>
    const Ty *addressof(const Ty &&) = delete;

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 Ty *construct_at(Ty *location, Args &&...args) noexcept(noexcept(::new(static_cast<void *>(location))
                                                                                           Ty(utility::forward<Args>(args)...))) {
        if (!location) {
            return nullptr;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return std::construct_at(location, std::forward<Args>(args)...);
        }
#endif
        return ::new (static_cast<void *>(location)) Ty(std::forward<Args>(args)...);
    }

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 void construct_in_place(Ty &object, Args &&...args) noexcept(
        type_traits::internals::_is_nothrow_constructible_v<Ty, Args...>) {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            construct_at(utility::addressof(object), forward<Args>(args)...);
        } else
#endif
        {
            ::new (static_cast<void *>(addressof(object))) Ty(forward<Args>(args)...);
        }
    }
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


#endif
