#include <rainy/core/core.hpp>

#if RAINY_USING_COMMON_SOURCESFILE_BUILD
namespace rainy::information::system_call {
    void debug_break();
    long interlocked_increment(volatile long *value);
    long interlocked_decrement(volatile long *value);
    std::uint32_t iso_volatile_load32(const volatile int *addr);
    bool is_aligned(void *const ptr, const std::size_t alignment);
    void *aligned_malloc_withcheck(const std::size_t size, const std::size_t alignment);
    void aligned_free_withcheck(void *block, const std::size_t alignment, void (*error)(void *error_block));
    void *allocate(const size_t size, const std::size_t alignment);
    void deallocate(void *ptr, const std::size_t size, const std::size_t alignment);
}

namespace rainy::information::internals {
    void stl_internal_check(const bool result, const internal_source_location &source_location);
}
#else
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
#endif

namespace rainy::information::system_call {
    RAINY_HPP_INLINE void debug_break() {
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

    RAINY_HPP_INLINE long interlocked_increment(volatile long *value) {
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

    RAINY_HPP_INLINE long interlocked_decrement(volatile long *value) {
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

    RAINY_HPP_INLINE std::uint32_t iso_volatile_load32(const volatile int *addr) {
#if RAINY_USING_MSVC
        return __iso_volatile_load32(addr);
#else
        uint32_t value = 0;
        __asm__ __volatile__("lfence\n movl (%1), %0" : "=r"(value) : "r"(addr) : "memory");
        return value;
#endif
    }

    RAINY_HPP_INLINE bool is_aligned(void *const ptr, const std::size_t alignment) {
        return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
    }

    RAINY_HPP_INLINE void *aligned_malloc_withcheck(const std::size_t size, const std::size_t alignment) {
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
        *reinterpret_cast<rainy_align_meta *>(raw_storage) = rainy_align_meta{check, random_mask, reinterpret_cast<uintptr_t>(end)};
        return real_pointer;
    }

    RAINY_HPP_INLINE void aligned_free_withcheck(void *block, const std::size_t alignment,
                                                 void (*error)(void *error_block)) {
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

    RAINY_HPP_INLINE void *allocate(const size_t size, const std::size_t alignment) {
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

    RAINY_HPP_INLINE void deallocate(void *block, const std::size_t alignment) {
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

    RAINY_HPP_INLINE void deallocate(void *block, const std::size_t byte_count, const std::size_t alignment) {
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

namespace rainy::information::internals {
    RAINY_HPP_INLINE void stl_internal_check(const bool result,
                                         const internal_source_location &source_location) {
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
