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

#ifndef RAINY_NODISCARD
#define RAINY_NODISCARD [[nodiscard]]
#endif

#ifndef RAINY_NODISCARD_MSG
#define RAINY_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#endif

#if RAINY_HAS_CXX20
#define rainy_constEXPR20 constexpr
#else
#define rainy_constEXPR20
#endif

#ifndef RAINY_NODISCARD_CONSTEXPR20
#define RAINY_NODISCARD_CONSTEXPR20 RAINY_NODISCARD rainy_constEXPR20
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
#define rainy_constEXPR_BOOL RAINY_INLINE_CONSTEXPR bool
#define RAINY_FALLTHROUGH [[fallthrough]]
#define rainy_constEXPR constexpr

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
#define RAINY_AINLINE_NODISCARD RAINY_NODISCARD RAINY_ALWAYS_INLINE
#else
#define RAINY_AINLINE_NODISCARD RAINY_NODISCARD RAINY_INLINE
#endif

#ifdef __AVX2__

#define RAINY_CAN_USING_AVX2 true
#else
#define RAINY_CAN_USING_AVX2 0

#endif

// 启用juzixioa's libray的AVX2支持，这将使得某些函数可以通过AVX2指令集进行较大程度的优化
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
namespace rainy::foundation::containers {
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
namespace rainy::foundation::containers {
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

    RAINY_INLINE uint32_t iso_volatile_load32(const volatile __int32 *addr) {
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
    rainy_constEXPR_BOOL check_format_type =
        (foundation::type_traits::type_relations::is_any_convertible_v<foundation::type_traits::internals::decay_t_<Test>, char, int,
                                                                       double, void *, float, long, long long, unsigned int,
                                                                       unsigned long, unsigned long long, const char *> ||
         ...);

    template <typename... Test>
    rainy_constEXPR_BOOL check_wformat_type =
        (foundation::type_traits::type_relations::is_any_convertible_v<foundation::type_traits::internals::decay_t_<Test>, char, int,
                                                                       double, void *, float, long, long long, unsigned int,
                                                                       unsigned long, unsigned long long, const wchar_t *> ||
         ...);
}

#if RAINY_USING_GCC
#include <rainy/internals/gnu/typetraits.hpp>

namespace rainy::foundation::type_traits::internals::gcc_detail_impl {
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

#endif
