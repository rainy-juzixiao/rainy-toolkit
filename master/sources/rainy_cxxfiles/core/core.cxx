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
#include <rainy/core/core.hpp>

namespace rainy::information::pal {
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
        static_assert(false, "rainy-toolkit only supports Windows and Linux platforms");
#endif // elif defined(RAINY_USING_LINUX)
#endif // elif RAINY_USING_WINDOWS
    }

    RAINY_HPP_INLINE long interlocked_increment(volatile long *value) {
#if RAINY_USING_MSVC
        return _InterlockedIncrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
    }

    RAINY_HPP_INLINE long interlocked_decrement(volatile long *value) {
#if RAINY_USING_MSVC
        return _InterlockedDecrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decl %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
    }

    RAINY_HPP_INLINE long interlocked_exchange_add(volatile long *value, long amount) {
#if RAINY_USING_MSVC
        return ::_InterlockedExchangeAdd(value, amount) + amount;
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xadd %0, %1" : "+m"(*avoid_clangtidy) : "r"(amount) : "memory", "cc");
        return *avoid_clangtidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
    }

    RAINY_HPP_INLINE long interlocked_exchange_subtract(volatile long *value, long amount) {
#if RAINY_USING_MSVC
        return ::_InterlockedExchangeAdd(value, -amount) - amount;
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xadd %0, %1" : "+m"(*avoid_clangtidy) : "r"(-amount) : "memory", "cc");
        return *avoid_clangtidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
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

    RAINY_HPP_INLINE void *allocate(const size_t size, const std::size_t alignment) {
        if (size == 0) {
            return nullptr;
        }
#ifdef __cpp_aligned_new
        return operator new[](size, std::align_val_t{alignment}, std::nothrow); // 由调用它的人，负责处理分配失败的问题
#else
        const std::size_t offset = alignment - 1 + sizeof(void *);
        const std::size_t total = size + offset;
        void *ptr = operator new[](total, std::nothrow);
        if (!ptr) {
            return nullptr; // 由调用它的人，负责处理分配失败的问题
        }
        rainy_const raw_location = reinterpret_cast<std::size_t>(ptr);
        const std::size_t aligned_location = (raw_location + offset) & ~(alignment - 1);
        rainy_let aligned_ptr = reinterpret_cast<void *>(aligned_location);
        *(reinterpret_cast<void **>(aligned_location - sizeof(void *))) = ptr;
        return aligned_ptr;
#endif
    }

    RAINY_HPP_INLINE void deallocate(void *block, const std::size_t alignment) {
        if (!block) {
            return;
        }
#ifdef __cpp_aligned_new
        operator delete[](block, std::align_val_t{alignment});
#else
        if (is_aligned(block, alignment)) {
            rainy_const aligned_location = reinterpret_cast<std::size_t>(block);
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
            rainy_const aligned_location = reinterpret_cast<std::size_t>(block);
            void *original_ptr = *(reinterpret_cast<void **>(aligned_location - sizeof(void *)));
            operator delete[](original_ptr, byte_count);
        }
#endif
    }

    RAINY_HPP_INLINE io_size_t read(FILE *stream, char *buffer, io_size_t buffer_size) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || buffer_size == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let bytes_read = static_cast<io_size_t>(std::fread(buffer, buffer_size, buffer_size, stream));
        if (bytes_read < buffer_size) {
            if (std::feof(stream)) {
                return bytes_read;
            } 
            if (std::ferror(stream)) {
                errno = EIO;
                return -1;
            }
        }
        return bytes_read;
    }

    RAINY_HPP_INLINE io_size_t read(FILE *stream, io_size_t read_count, char *buffer, io_size_t buffer_size) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || buffer_size == 0) {
            errno = EINVAL;
            return -1;
        }
        io_size_t total_bytes_read{0};
        io_size_t bytes_read{0};
        while ((read_count--) > 0) {
            bytes_read = static_cast<io_size_t>(std::fread(buffer + total_bytes_read, 1, buffer_size, stream));
            if (std::feof(stream)) {
                return bytes_read;
            } 
            if (std::ferror(stream)) {
                errno = EIO;
                return -1;
            }
            ++total_bytes_read;
        }
        buffer[total_bytes_read] = '\0';
        return total_bytes_read;
    }

    RAINY_HPP_INLINE io_size_t read(FILE *stream, char *buffer, io_size_t buffer_size, char delimiter) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || buffer_size == 0) {
            errno = EINVAL;
            return -1;
        }
        io_size_t total_bytes_read{0};
        io_size_t bytes_read{0};
        while (total_bytes_read < buffer_size - 1) {
            bytes_read = static_cast<io_size_t>(std::fread(buffer, 1, buffer_size, stream));
            if (bytes_read < buffer_size) {
                if (std::feof(stream)) {
                    return bytes_read;
                } 
                if (std::ferror(stream)) {
                    errno = EIO;
                    return -1;
                }
            }
            total_bytes_read += bytes_read;
            if (buffer[total_bytes_read - 1] == delimiter) {
                break;
            }
        }
        return total_bytes_read;
    }

    RAINY_HPP_INLINE io_size_t read_line(FILE *stream, char *buffer, io_size_t buffer_size) {
        return read(stream, buffer, buffer_size, '\n');
    }

    RAINY_HPP_INLINE io_size_t read_line(FILE *stream, char *buffer, io_size_t buffer_size, char delimiter) {
        return read(stream, buffer, buffer_size, delimiter);
    }

    RAINY_HPP_INLINE io_size_t read_binray(FILE *stream, void *buffer,io_size_t element_size, io_size_t element_count) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || element_count == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let bytes_read = static_cast<io_size_t>(std::fread(buffer, element_size, element_count, stream));
        if (bytes_read < element_count * element_size) {
            if (std::feof(stream)) {
                return bytes_read;
            } 
            if (std::ferror(stream)) {
                errno = EIO;
                return -1;
            }
        }
        return bytes_read;
    }

    RAINY_HPP_INLINE io_size_t write(FILE *stream, const char *buffer, io_size_t count) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || count == 0) {
            errno = EINVAL;
            return -1;
        }
        return static_cast<io_size_t>(std::fwrite(buffer, 1, count, stream));
    }

    RAINY_HPP_INLINE io_size_t write_line(FILE *stream, const char *buffer) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream) {
            errno = EINVAL;
            return -1;
        }
        io_size_t write_length = internals::string_length(reinterpret_cast<const char *>(buffer));
        return static_cast<io_size_t>(std::fwrite(buffer, 1, write_length, stream));
    }
}

namespace rainy::information::internals {
    RAINY_HPP_INLINE void stl_internal_check(const bool result, const internal_source_location &source_location) {
        if (result) {
            return;
        }
        bool release{false};
        constexpr static std::size_t static_memory_size = 120;
        thread_local char static_memory[static_memory_size]; // For Thread Safety
        rainy_let buffer = static_cast<char *>(static_memory);
        rainy_let required_size = 1 + std::snprintf(nullptr, 0, "%s:%du in function '%s' found a error\n", source_location.file_name(),
                                                    source_location.line(), source_location.function_name());
        if (required_size > static_memory_size) {
            buffer = static_cast<char *>(pal::allocate(sizeof(char) * required_size,
                                                               alignof(char))); // 通过operator new[]获取内存，而不是通过new char
            release = true;
            if (!buffer) {
                raw_string_view<char> error_info(
                    "we found a error. and also we can's allocate memory from dynamic_storage. before make more cursh, we must "
                    "terminate this program. you can commit a issue in github.");
                (void) std::fwrite(error_info.c_str(), sizeof(char), error_info.size(), stderr);
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
