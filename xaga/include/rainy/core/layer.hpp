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
#ifndef RAINY_CORE_LAYER_HPP
#define RAINY_CORE_LAYER_HPP
#include <rainy/core/platform.hpp>

#if RAINY_USING_MSVC
#if RAINY_IS_X86_PLATFORM
#include <emmintrin.h>
#endif
#include <intrin.h>
#endif

#if RAINY_USING_MSVC
#define rainy_compiler_barrier() _ReadWriteBarrier()
#else
#define rainy_compiler_barrier() asm volatile("" ::: "memory")
#endif

/* 这是一个用C函数库封装的底层调用空间。外部用户不应当使用它。推荐使用foundation提供的pal模块 */
namespace rainy::core::pal {
    enum class open_mode : std::uint8_t {
        no_state = 0x00,
        read = 0x01,
        write = 0x02,
        append = 0x04,
        binary = 0x08,
        truncate = 0x10,
        nocreate = 0x20,
        update = 0x40
    };

    constexpr int max_frames_dump = 128;

    enum class file_attributes : std::int16_t {
        read_only = 0x1,
        hidden = 0x2,
        system = 0x4,
        directory = 0x8,
        archive = 0x10,
        device = 0x20,
        normal = 0x40,
        temporary = 0x80,
        sparse_file = 0x100,
        reparse_point = 0x200,
        invalid = -1,
    };

    enum class reparse_tag : std::uint8_t {
        none,
        mount_point,
        symlink,
    };

    struct file_status {
        std::int64_t last_write_time;
        std::uint64_t file_size;
        file_attributes attributes;
        pal::reparse_tag reparse_tag;
        std::uint32_t link_count;
    };

    enum class file_type : std::uint8_t {
        none,
        not_found,
        regular,
        directory,
        symlink,
        block,
        character,
        fifo,
        socket,
        unknown,
        junction
    };

    using file_handle = std::uintptr_t;
}

namespace rainy::core::pal {
    /* debug tool */
    RAINY_TOOLKIT_API void debug_break();
    RAINY_TOOLKIT_API std::size_t collect_stack_frame(native_frame_ptr_t *out_frames, std::size_t max_frames_count,
                                                      std::size_t skip) noexcept;
    RAINY_TOOLKIT_API std::size_t safe_dump_to(void *memory, std::size_t size, std::size_t skip) noexcept;
    RAINY_TOOLKIT_API bool resolve_stack_frame(native_frame_ptr_t frame, cstring buf, std::size_t buf_size) noexcept;
    RAINY_TOOLKIT_API void demangle(czstring name, cstring buf, std::size_t buffer_length);
}

namespace rainy::core::pal {
    /* atomic:: inc,dec */
    RAINY_TOOLKIT_API long interlocked_increment(volatile long *value);
    RAINY_TOOLKIT_API std::int8_t interlocked_increment8(volatile std::int8_t *value);
    RAINY_TOOLKIT_API std::int16_t interlocked_increment16(volatile std::int16_t *value);
    RAINY_TOOLKIT_API std::int32_t interlocked_increment32(volatile std::int32_t *value);
    RAINY_TOOLKIT_API std::int64_t interlocked_increment64(volatile std::int64_t *value);

    RAINY_TOOLKIT_API long interlocked_decrement(volatile long *value);
    RAINY_TOOLKIT_API std::int8_t interlocked_decrement8(volatile std::int8_t *value);
    RAINY_TOOLKIT_API std::int16_t interlocked_decrement16(volatile std::int16_t *value);
    RAINY_TOOLKIT_API std::int32_t interlocked_decrement32(volatile std::int32_t *value);
    RAINY_TOOLKIT_API std::int64_t interlocked_decrement64(volatile std::int64_t *value);

    RAINY_TOOLKIT_API std::int8_t interlocked_exchange_add8(volatile std::int8_t *value, std::int8_t amount);
    RAINY_TOOLKIT_API std::int16_t interlocked_exchange_add16(volatile std::int16_t *value, std::int16_t amount);
    RAINY_TOOLKIT_API std::int32_t interlocked_exchange_add32(volatile std::int32_t *value, std::int32_t amount);
    RAINY_TOOLKIT_API std::int64_t interlocked_exchange_add64(volatile std::int64_t *value, std::int64_t amount);

    RAINY_TOOLKIT_API std::intptr_t interlocked_exchange_subtract(volatile std::intptr_t *value, const std::intptr_t amount);
    RAINY_TOOLKIT_API std::int8_t interlocked_exchange_subtract8(volatile std::int8_t *value, std::int8_t amount);
    RAINY_TOOLKIT_API std::int16_t interlocked_exchange_subtract16(volatile std::int16_t *value, std::int16_t amount);
    RAINY_TOOLKIT_API std::int32_t interlocked_exchange_subtract32(volatile std::int32_t *value, std::int32_t amount);
    RAINY_TOOLKIT_API std::int64_t interlocked_exchange_subtract64(volatile std::int64_t *value, std::int64_t amount);

    /* atomic::iso_volatile_load */
    RAINY_TOOLKIT_API std::intptr_t iso_volatile_load(const volatile std::intptr_t *address);
    RAINY_TOOLKIT_API std::int8_t iso_volatile_load8(const volatile std::int8_t *address);
    RAINY_TOOLKIT_API std::int16_t iso_volatile_load16(const volatile std::int16_t *address);
    RAINY_TOOLKIT_API std::int32_t iso_volatile_load32(const volatile std::int32_t *address);
    RAINY_TOOLKIT_API std::int64_t iso_volatile_load64(const volatile std::int64_t *address);

    RAINY_TOOLKIT_API std::intptr_t interlocked_exchange(volatile std::intptr_t *target, std::intptr_t value);
    RAINY_TOOLKIT_API std::int8_t interlocked_exchange8(volatile std::int8_t *target, std::int8_t value);
    RAINY_TOOLKIT_API std::int16_t interlocked_exchange16(volatile std::int16_t *target, std::int16_t value);
    RAINY_TOOLKIT_API std::int32_t interlocked_exchange32(volatile std::int32_t *target, std::int32_t value);
    RAINY_TOOLKIT_API std::int64_t interlocked_exchange64(volatile std::int64_t *target, std::int64_t value);

    RAINY_TOOLKIT_API void *interlocked_exchange_pointer(volatile void **target, void *value);

    RAINY_TOOLKIT_API bool interlocked_compare_exchange(volatile long *destination, long exchange, long comparand);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange8(volatile std::int8_t *destination, std::int8_t exchange,
                                                         std::int8_t comparand);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange16(volatile std::int16_t *destination, std::int16_t exchange,
                                                          std::int16_t comparand);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange32(volatile std::int32_t *destination, std::int32_t exchange,
                                                          std::int32_t comparand);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange,
                                                          std::int64_t comparand);

    RAINY_TOOLKIT_API void *interlocked_compare_exchange_pointer(void *volatile *destination, void *exchange, void *comparand);

    RAINY_TOOLKIT_API std::intptr_t interlocked_and(volatile std::intptr_t *value, const std::intptr_t mask);
    RAINY_TOOLKIT_API std::int8_t interlocked_and8(volatile std::int8_t *value, std::int8_t mask);
    RAINY_TOOLKIT_API std::int16_t interlocked_and16(volatile std::int16_t *value, std::int16_t mask);
    RAINY_TOOLKIT_API std::int32_t interlocked_and32(volatile std::int32_t *value, std::int32_t mask);
    RAINY_TOOLKIT_API std::int64_t interlocked_and64(volatile std::int64_t *value, std::int64_t mask);

    RAINY_TOOLKIT_API std::intptr_t interlocked_or(volatile std::intptr_t *value, const std::intptr_t mask);
    RAINY_TOOLKIT_API std::int8_t interlocked_or8(volatile std::int8_t *value, std::int8_t mask);
    RAINY_TOOLKIT_API std::int16_t interlocked_or16(volatile std::int16_t *value, std::int16_t mask);
    RAINY_TOOLKIT_API std::int32_t interlocked_or32(volatile std::int32_t *value, std::int32_t mask);
    RAINY_TOOLKIT_API std::int64_t interlocked_or64(volatile std::int64_t *value, std::int64_t mask);

    RAINY_TOOLKIT_API std::intptr_t interlocked_xor(volatile std::intptr_t *value, std::intptr_t mask);
    RAINY_TOOLKIT_API std::int8_t interlocked_xor8(volatile std::int8_t *value, std::int8_t mask);
    RAINY_TOOLKIT_API std::int16_t interlocked_xor16(volatile std::int16_t *value, std::int16_t mask);
    RAINY_TOOLKIT_API std::int32_t interlocked_xor32(volatile std::int32_t *value, std::int32_t mask);
    RAINY_TOOLKIT_API std::int64_t interlocked_xor64(volatile std::int64_t *value, std::int64_t mask);

    RAINY_TOOLKIT_API void iso_volatile_store(volatile void *address, void *value);
    RAINY_TOOLKIT_API void iso_volatile_store8(volatile std::int8_t *address, std::int8_t value);
    RAINY_TOOLKIT_API void iso_volatile_store16(volatile std::int16_t *address, std::int16_t value);
    RAINY_TOOLKIT_API void iso_volatile_store32(volatile std::int32_t *address, std::int32_t value);
    RAINY_TOOLKIT_API void iso_volatile_store64(volatile std::int64_t *address, std::int64_t value);

    RAINY_TOOLKIT_API void atomic_thread_fence(const memory_order order) noexcept;

    RAINY_TOOLKIT_API void read_write_barrier() noexcept;
    RAINY_TOOLKIT_API void read_barrier() noexcept;
    RAINY_TOOLKIT_API void write_barrier() noexcept;
}

namespace rainy::core::pal {
    /* file system */
    RAINY_TOOLKIT_API file_status get_file_status(czstring file_path) noexcept;
    RAINY_TOOLKIT_API file_handle open_file(czstring filepath, open_mode mode);
    RAINY_TOOLKIT_API file_type get_file_type(core::czstring file_path) noexcept;
    RAINY_TOOLKIT_API bool close_file(std::uintptr_t handle);
    RAINY_TOOLKIT_API bool file_exists(czstring file_path);
    RAINY_TOOLKIT_API bool create_directory(czstring dir_path);
    RAINY_TOOLKIT_API bool remove_file(czstring file_path);
    RAINY_TOOLKIT_API bool get_file_size(czstring file_path, std::uint64_t &size);
    RAINY_TOOLKIT_API bool rename_file(czstring old_path, czstring new_path);
    /* memory io */
    RAINY_TOOLKIT_API bool is_aligned(void *ptr, std::size_t alignment);
    RAINY_TOOLKIT_API void *allocate(std::size_t size) noexcept;
    RAINY_TOOLKIT_API void *allocate(std::size_t size, std::size_t alignment) noexcept;
    RAINY_TOOLKIT_API void deallocate(void *block);
    RAINY_TOOLKIT_API void deallocate(void *block, std::size_t alignment);
    RAINY_TOOLKIT_API void deallocate(void *ptr, std::size_t size, std::size_t alignment);
    /* read io */
    RAINY_TOOLKIT_API io_size_t read(std::uintptr_t stream, char *buffer, io_size_t buffer_size);
    RAINY_TOOLKIT_API io_size_t read(std::uintptr_t stream, io_size_t read_count, char *buffer, io_size_t buffer_size);
    RAINY_TOOLKIT_API io_size_t read(std::uintptr_t stream, char *buffer, io_size_t buffer_size, char delimiter);
    RAINY_TOOLKIT_API io_size_t read_line(std::uintptr_t stream, char *buffer, io_size_t buffer_size);
    RAINY_TOOLKIT_API io_size_t read_line(std::uintptr_t stream, char *buffer, io_size_t buffer_size, char delimiter);
    RAINY_TOOLKIT_API io_size_t read_binary(std::uintptr_t stream, void *buffer, io_size_t element_size, io_size_t element_count);
    /* write io */
    // RAINY_TOOLKIT_API io_size_t write(std::uintptr_t stream, czstring buffer, io_size_t count);
    // RAINY_TOOLKIT_API io_size_t write_line(std::uintptr_t stream, czstring buffer);
    RAINY_TOOLKIT_API io_size_t write(std::uintptr_t fd, const void *buffer, std::size_t count);
    RAINY_TOOLKIT_API void flush(std::uintptr_t fd);

#if RAINY_USING_64_BIT_PLATFORM
    RAINY_TOOLKIT_API bool interlocked_compare_exchange128(std::int64_t volatile *destination, std::int64_t exchange_high,
                                                           std::int64_t exchange_low, std::int64_t *comparand_result);
#endif
}

namespace rainy::core::pal {
    // 带内存序的原子操作声明
    RAINY_TOOLKIT_API long interlocked_increment_explicit(volatile long *value, memory_order order);
    RAINY_TOOLKIT_API long interlocked_decrement_explicit(volatile long *value, memory_order order);

    RAINY_TOOLKIT_API std::int8_t interlocked_increment8_explicit(volatile std::int8_t *value, memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_increment16_explicit(volatile std::int16_t *value, memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_increment32_explicit(volatile std::int32_t *value, memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_increment64_explicit(volatile std::int64_t *value, memory_order order);

    RAINY_TOOLKIT_API std::int8_t interlocked_decrement8_explicit(volatile std::int8_t *value, memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_decrement16_explicit(volatile std::int16_t *value, memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_decrement32_explicit(volatile std::int32_t *value, memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_decrement64_explicit(volatile std::int64_t *value, memory_order order);

    RAINY_TOOLKIT_API std::intptr_t interlocked_exchange_add_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                                      memory_order order);
    // 带内存序的原子加操作
    RAINY_TOOLKIT_API std::int8_t interlocked_exchange_add8_explicit(volatile std::int8_t *value, std::int8_t amount,
                                                                     memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_exchange_add16_explicit(volatile std::int16_t *value, std::int16_t amount,
                                                                       memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_exchange_add32_explicit(volatile std::int32_t *value, std::int32_t amount,
                                                                       memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_exchange_add64_explicit(volatile std::int64_t *value, std::int64_t amount,
                                                                       memory_order order);

    // 带内存序的原子减操作
    RAINY_TOOLKIT_API std::intptr_t interlocked_exchange_subtract_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                                           memory_order order);
    RAINY_TOOLKIT_API std::int8_t interlocked_exchange_subtract8_explicit(volatile std::int8_t *value, std::int8_t amount,
                                                                          memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_exchange_subtract16_explicit(volatile std::int16_t *value, std::int16_t amount,
                                                                            memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_exchange_subtract32_explicit(volatile std::int32_t *value, std::int32_t amount,
                                                                            memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_exchange_subtract64_explicit(volatile std::int64_t *value, std::int64_t amount,
                                                                            memory_order order);

    // 带内存序的原子交换操作
    RAINY_TOOLKIT_API std::intptr_t interlocked_exchange_explicit(volatile std::intptr_t *target, std::intptr_t value,
                                                                  memory_order order);
    RAINY_TOOLKIT_API std::int8_t interlocked_exchange8_explicit(volatile std::int8_t *target, std::int8_t value, memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_exchange16_explicit(volatile std::int16_t *target, std::int16_t value,
                                                                   memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_exchange32_explicit(volatile std::int32_t *target, std::int32_t value,
                                                                   memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_exchange64_explicit(volatile std::int64_t *target, std::int64_t value,
                                                                   memory_order order);
    RAINY_TOOLKIT_API void *interlocked_exchange_pointer_explicit(volatile void **target, void *value, memory_order order);

    // 带内存序的CAS操作
    RAINY_TOOLKIT_API bool interlocked_compare_exchange_explicit(volatile long *destination, long exchange, long comparand,
                                                                 memory_order success, memory_order failure);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange8_explicit(volatile std::int8_t *destination, std::int8_t exchange,
                                                                  std::int8_t comparand, memory_order success, memory_order failure);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange16_explicit(volatile std::int16_t *destination, std::int16_t exchange,
                                                                   std::int16_t comparand, memory_order success, memory_order failure);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange32_explicit(volatile std::int32_t *destination, std::int32_t exchange,
                                                                   std::int32_t comparand, memory_order success, memory_order failure);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange64_explicit(volatile std::int64_t *destination, std::int64_t exchange,
                                                                   std::int64_t comparand, memory_order success, memory_order failure);
    RAINY_TOOLKIT_API void *interlocked_compare_exchange_pointer_explicit(volatile void **destination, void *exchange, void *comparand,
                                                                          memory_order success, memory_order failure);

    // 带内存序的原子位操作
    RAINY_TOOLKIT_API std::intptr_t interlocked_and_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order);
    RAINY_TOOLKIT_API int8_t interlocked_and8_explicit(volatile int8_t *value, int8_t mask, memory_order order);
    RAINY_TOOLKIT_API int16_t interlocked_and16_explicit(volatile int16_t *value, int16_t mask, memory_order order);
    RAINY_TOOLKIT_API int32_t interlocked_and32_explicit(volatile int32_t *value, int32_t mask, memory_order order);
    RAINY_TOOLKIT_API int64_t interlocked_and64_explicit(volatile int64_t *value, int64_t mask, memory_order order);

    RAINY_TOOLKIT_API std::intptr_t interlocked_or_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int8_t interlocked_or8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_or16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_or32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_or64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order);

    RAINY_TOOLKIT_API std::intptr_t interlocked_xor_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int8_t interlocked_xor8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int16_t interlocked_xor16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int32_t interlocked_xor32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order);
    RAINY_TOOLKIT_API std::int64_t interlocked_xor64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order);

    // 带内存序的原子加载
    RAINY_TOOLKIT_API std::intptr_t iso_volatile_load_explicit(const volatile std::intptr_t *address, memory_order order);
    RAINY_TOOLKIT_API std::int8_t iso_volatile_load8_explicit(const volatile std::int8_t *address, memory_order order);
    RAINY_TOOLKIT_API std::int16_t iso_volatile_load16_explicit(const volatile std::int16_t *address, memory_order order);
    RAINY_TOOLKIT_API std::int32_t iso_volatile_load32_explicit(const volatile std::int32_t *address, memory_order order);
    RAINY_TOOLKIT_API std::int64_t iso_volatile_load64_explicit(const volatile std::int64_t *address, memory_order order);

    // 带内存序的原子存储
    RAINY_TOOLKIT_API void iso_volatile_store_explicit(volatile void *address, void *value, memory_order order);
    RAINY_TOOLKIT_API void iso_volatile_store8_explicit(volatile std::int8_t *address, std::int8_t value, memory_order order);
    RAINY_TOOLKIT_API void iso_volatile_store16_explicit(volatile std::int16_t *address, std::int16_t value, memory_order order);
    RAINY_TOOLKIT_API void iso_volatile_store32_explicit(volatile std::int32_t *address, std::int32_t value, memory_order order);
    RAINY_TOOLKIT_API void iso_volatile_store64_explicit(volatile std::int64_t *address, std::int64_t value, memory_order order);
}

namespace rainy::core::pal {
    using atomic_wait_equal_fn = bool (*)(const void *storage, const void *comparand, std::size_t size, void *ctx) noexcept;

    RAINY_TOOLKIT_API void atomic_wait(const void *storage, const void *comparand, std::size_t size,
                                       atomic_wait_equal_fn equal_fn = nullptr, void *ctx = nullptr) noexcept;

    RAINY_TOOLKIT_API void atomic_notify_one(const void *storage, std::size_t size) noexcept;

    RAINY_TOOLKIT_API void atomic_notify_all(const void *storage, std::size_t size) noexcept;
}

namespace rainy::core::pal {
    template <typename T>
    struct alignas(sizeof(T) * 2) double_word_t {
        T lo; // 低字（第一个字）
        T hi; // 高字（第二个字）

        bool operator==(const double_word_t &other) const noexcept {
            return lo == other.lo && hi == other.hi;
        }
        bool operator!=(const double_word_t &other) const noexcept {
            return !(*this == other);
        }
    };

    static_assert(sizeof(double_word_t<std::uint32_t>) == 8);
    static_assert(sizeof(double_word_t<std::uint64_t>) == 16);
    static_assert(sizeof(double_word_t<void *>) == sizeof(void *) * 2);

    // 平台默认双字类型：与指针等宽，直接对应 CMPXCHG8B / CMPXCHG16B
    using native_double_word_t = double_word_t<std::uintptr_t>;

    // 原子 CAS：成功返回 true，失败时 comparand 被更新为当前值
    RAINY_TOOLKIT_API bool interlocked_compare_exchange_double_word(volatile native_double_word_t *destination,
                                                                    native_double_word_t exchange,
                                                                    native_double_word_t *comparand) noexcept;

    // 原子加载
    RAINY_TOOLKIT_API native_double_word_t atomic_load_double_word(const volatile native_double_word_t *address,
                                                                   memory_order order) noexcept;

    // 原子存储
    RAINY_TOOLKIT_API void atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value,
                                                    memory_order order) noexcept;

    static constexpr bool is_always_lock_free =
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64))
        true;
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__amd64__))
        true;
#else
        false; // mutex 回退，包括未加 -mcx16 的 Clang/GCC
#endif
}

#endif
