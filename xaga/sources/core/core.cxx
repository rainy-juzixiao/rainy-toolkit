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
#include <immintrin.h>
#include <rainy/core/core.hpp>

#if RAINY_USING_MSVC
#include <intrin.h>
#endif

namespace rainy::foundation::exceptions {
    static exception_handler_t global_exception_handler_impl = &std::terminate;
    thread_local exception_handler_t current_thread_exception_handler_impl = &std::terminate;

    exception_handler_t global_exception_handler(exception_handler_t new_handler) noexcept {
        return (new_handler ? utility::exchange(global_exception_handler_impl, new_handler) : global_exception_handler_impl);
    }

    exception_handler_t current_thread_exception_handler(exception_handler_t new_handler) noexcept {
        return (new_handler ? utility::exchange(current_thread_exception_handler_impl, new_handler)
                            : current_thread_exception_handler_impl);
    }
}

namespace rainy::foundation::exceptions::implements {
    void invoke_exception_handler() noexcept {
        {
            const auto invoke_address = current_thread_exception_handler();
            invoke_address();
        }
        {
            const auto invoke_address = global_exception_handler();
            invoke_address();
        }
    }
}

namespace rainy::core::builtin {
#if RAINY_USING_AVX2
    std::int32_t ctz_avx2(const std::uint32_t x) noexcept {
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
#endif

    template <std::size_t N>
    void *fixed_memcpy(void *dest, const void *src) {
        return std::memcpy(dest, src, N);
    }

    template <std::size_t N>
    void *fixed_memmove(void *dest, const void *src) {
#if RAINY_USING_MSVC
        return ::memmove_s(dest, N, src, N);
#else
        return std::memmove(dest, src, N);
#endif
    }

    void *copy_memory(void *dest, const void *src, std::size_t len) {
        rainy_assume(static_cast<bool>(dest));
        if (len <= 24) {
            switch (len) { // NOLINT
                case 0:
                    return dest;
                case 1:
                    return fixed_memcpy<1>(dest, src);
                case 2:
                    return fixed_memcpy<2>(dest, src);
                case 3:
                    return fixed_memcpy<3>(dest, src);
                case 4:
                    return fixed_memcpy<4>(dest, src);
                case 5:
                    return fixed_memcpy<5>(dest, src);
                case 6:
                    return fixed_memcpy<6>(dest, src);
                case 7:
                    return fixed_memcpy<7>(dest, src);
                case 8:
                    return fixed_memcpy<8>(dest, src);
                case 9:
                    return fixed_memcpy<9>(dest, src);
                case 10:
                    return fixed_memcpy<10>(dest, src);
                case 11:
                    return fixed_memcpy<11>(dest, src);
                case 12:
                    return fixed_memcpy<12>(dest, src);
                case 13:
                    return fixed_memcpy<13>(dest, src);
                case 14:
                    return fixed_memcpy<14>(dest, src);
                case 15:
                    return fixed_memcpy<15>(dest, src);
                case 16:
                    return fixed_memcpy<16>(dest, src);
                case 17:
                    return fixed_memcpy<17>(dest, src);
                case 18:
                    return fixed_memcpy<18>(dest, src);
                case 19:
                    return fixed_memcpy<19>(dest, src);
                case 20:
                    return fixed_memcpy<20>(dest, src);
                case 21:
                    return fixed_memcpy<21>(dest, src);
                case 22:
                    return fixed_memcpy<22>(dest, src);
                case 23:
                    return fixed_memcpy<23>(dest, src);
                case 24:
                    return fixed_memcpy<24>(dest, src);
            }
        } else if (len > 24 && len < 36) {
            rainy_let dst = static_cast<std::uint8_t *>(dest);
            rainy_let source = static_cast<const std::uint8_t *>(src);
            constexpr size_t chunk_size = 16; // 步长调整为 16
            while (len >= chunk_size) {
                const __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source)); // 使用未对齐加载
                _mm_storeu_si128(reinterpret_cast<__m128i *>(dst), data);
                source += chunk_size;
                dst += chunk_size;
                len -= chunk_size;
            }
            // 处理剩余字节（此时 len < 16）
            for (size_t i = 0; i < len; ++i) {
                *dst++ = *source++;
            }
            return dest;
        }
        rainy_let dst = static_cast<std::uint8_t *>(dest);
        rainy_let source = static_cast<const std::uint8_t *>(src);
#if RAINY_USING_AVX2
        constexpr std::size_t block_step[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64}; // 块的步进
        for (auto block_size: block_step) {
            while (len >= block_size) {
                __m256i data1 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(source));
                __m256i data2 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(source + block_size - 32));
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(dst), data1);
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(dst + block_size - 32), data2);
                source += block_size;
                dst += block_size;
                len -= block_size;
            }
        }
#endif
        // 每次复制16字节，SSE每个寄存器处理128位数据（16字节）
        std::size_t chunk_size = 16;
        // 对齐到16字节
        while (len >= chunk_size) {
            // 加载128位（16字节）的数据到SSE寄存器
            __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source));
            // 存储到目标内存
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dst), data);
            // 更新指针和剩余长度
            source += chunk_size;
            dst += chunk_size;
            len -= chunk_size;
        }
        // 处理剩余小于16字节的数据
        while (len > 0) {
            *dst++ = *source++;
            len--;
        }
        return dst;
    }

    /*void *move_memory(void *dest, std::size_t dest_size, const void *src, std::size_t src_count) {
        return nullptr;
    }

    void *set_memory(void *dest, std::size_t count, int new_val);
    void *zero_memory(void *dest, std::size_t count);
    void *fill_memory(void *dest, std::size_t count, int new_val);
    void *fill_memory(void *dest, std::size_t count, const void *src);
    void *fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count);
    void *fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count, std::size_t src_offset);
    void *fill_memory(void *dest, std::size_t count, const void *src, std::size_t src_count, std::size_t src_offset,
                                        std::size_t dest_offset);*/
}


namespace rainy::core::pal {
    bool is_aligned(void *const ptr, const std::size_t alignment) {
        return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
    }

    void *allocate(std::size_t size) noexcept {
        if (size == 0) {
            return nullptr;
        }
        return operator new[](size, std::nothrow);
    }

    void *allocate(const std::size_t size, const std::size_t alignment) noexcept {
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

    void deallocate(void *block) {
        if (!block) {
            return;
        }
        operator delete[](block);
    }

    void deallocate(void *block, const std::size_t alignment) {
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

    void deallocate(void *block, const std::size_t byte_count, const std::size_t alignment) {
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

    io_size_t read(std::uintptr_t stream, char *buffer, io_size_t buffer_size) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || buffer_size == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let stream_ = reinterpret_cast<FILE *>(stream); // NOLINT
        rainy_const bytes_read = static_cast<io_size_t>(std::fread(buffer, buffer_size, buffer_size, stream_));
        if (bytes_read < buffer_size) {
            if (std::feof(stream_)) {
                return bytes_read;
            }
            if (std::ferror(stream_)) {
                errno = EIO;
                return -1;
            }
        }
        return bytes_read;
    }

    io_size_t read(std::uintptr_t stream, io_size_t read_count, char *buffer, const io_size_t buffer_size) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || buffer_size == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let stream_ = reinterpret_cast<FILE *>(stream); // NOLINT
        io_size_t total_bytes_read{0};
        while ((read_count--) > 0) {
            rainy_const bytes_read = static_cast<io_size_t>(std::fread(buffer + total_bytes_read, 1, buffer_size, stream_));
            if (std::feof(stream_)) {
                return bytes_read;
            }
            if (std::ferror(stream_)) {
                errno = EIO;
                return -1;
            }
            ++total_bytes_read;
        }
        buffer[total_bytes_read] = '\0';
        return total_bytes_read;
    }

    io_size_t read(const std::uintptr_t stream, char *buffer, const io_size_t buffer_size, const char delimiter) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || buffer_size == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let stream_ = reinterpret_cast<FILE *>(stream); // NOLINT
        io_size_t total_bytes_read{0};
        while (total_bytes_read < buffer_size - 1) {
            rainy_const bytes_read = static_cast<io_size_t>(std::fread(buffer, 1, buffer_size, stream_));
            if (bytes_read < buffer_size) {
                if (std::feof(stream_)) {
                    return bytes_read;
                }
                if (std::ferror(stream_)) {
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

    io_size_t read_line(std::uintptr_t stream, char *buffer, io_size_t buffer_size) {
        return read(stream, buffer, buffer_size, '\n');
    }

    io_size_t read_line(std::uintptr_t stream, char *buffer, io_size_t buffer_size, char delimiter) {
        return read(stream, buffer, buffer_size, delimiter);
    }

    io_size_t read_binray(std::uintptr_t stream, void *buffer, io_size_t element_size, io_size_t element_count) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || element_count == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let stream_ = reinterpret_cast<FILE *>(stream); // NOLINT
        rainy_const bytes_read = static_cast<io_size_t>(std::fread(buffer, element_size, element_count, stream_));
        if (bytes_read < element_count * element_size) {
            if (std::feof(stream_)) {
                return bytes_read;
            }
            if (std::ferror(stream_)) {
                errno = EIO;
                return -1;
            }
        }
        return bytes_read;
    }

    io_size_t write(const std::uintptr_t stream, const czstring buffer, const io_size_t count) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream || count == 0) {
            errno = EINVAL;
            return -1;
        }
        rainy_let stream_ = reinterpret_cast<FILE *>(stream); // NOLINT
        return static_cast<io_size_t>(std::fwrite(buffer, 1, count, stream_));
    }

    io_size_t write_line(const std::uintptr_t stream, const czstring buffer) {
        if (!buffer) {
            errno = EFAULT;
            return -1;
        }
        if (!stream) {
            errno = EINVAL;
            return -1;
        }
        rainy_let stream_ = reinterpret_cast<FILE *>(stream); // NOLINT
        rainy_const write_length = static_cast<io_size_t>(builtin::string_length(buffer));
        return static_cast<io_size_t>(std::fwrite(buffer, 1, write_length, stream_));
    }
}

namespace rainy::core::implements {
    void stl_internal_check(const bool result, const internal_source_location &source_location) {
        if (result) {
            return;
        }
        bool release{false};
        constexpr static std::size_t static_memory_size = 120;
        thread_local char static_memory[static_memory_size]; // For Thread Safety
        rainy_let buffer = static_cast<char *>(static_memory);
        std::size_t required_size =
            1 + std::snprintf(nullptr, 0, "%s:%du in function '%s' found a error\n", source_location.file_name(),
                              source_location.line(), source_location.function_name());
        if (required_size > static_memory_size) {
            buffer = static_cast<char *>(pal::allocate(sizeof(char) * required_size,
                                                       alignof(char))); // 通过operator new[]获取内存，而不是通过new char
            release = true;
            if (!buffer) {
                constexpr raw_string_view<char> error_info(
                    "we found a error. and also we can's allocate memory from dynamic_storage. before make more crushing, we must "
                    "terminate this program. you can commit a issue in github.");
                (void) std::fwrite(error_info.c_str(), sizeof(char), error_info.size(), stderr);
                std::abort();
            }
        }
        required_size = std::snprintf(buffer, required_size, "%s:%u in function '%s' found a error\n", source_location.file_name(),
                                      source_location.line(), source_location.function_name());
        (void) std::fwrite(buffer, sizeof(char), required_size, stderr);
        if (release) {
            pal::deallocate(buffer, required_size);
        }
        std::abort();
    }
}

namespace rainy::annotations::lifetime::implements {
    void atomic_counter::operator++() {
        core::pal::interlocked_increment32(&this->count);
    }

    void atomic_counter::operator--() {
        core::pal::interlocked_decrement32(&this->count);
    }

    int atomic_counter::get() {
        return core::pal::iso_volatile_load32(static_cast<const volatile int *>(&this->count));
    }
}
