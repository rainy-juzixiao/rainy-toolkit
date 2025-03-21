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
    /* atomic */
    RAINY_TOOLKIT_API long interlocked_increment(volatile long *value);
    RAINY_TOOLKIT_API long interlocked_decrement(volatile long *value);
    RAINY_TOOLKIT_API long interlocked_exchange_add(volatile long *value, long amount);
    RAINY_TOOLKIT_API long interlocked_exchange_subtract(volatile long *value, long amount);
    RAINY_TOOLKIT_API std::uint32_t iso_volatile_load32(const volatile int *address);
    RAINY_TOOLKIT_API std::uint64_t iso_volatile_load64(const volatile long long *address);
    RAINY_TOOLKIT_API long interlocked_exchange(volatile long *target, long value);
    RAINY_TOOLKIT_API void *interlocked_exchange_pointer(volatile void **target, void *value);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange(volatile long *destination, long exchange, long comparand);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange_pointer(volatile void **destination, void *exchange, void *comparand);
    RAINY_TOOLKIT_API bool interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange,
                                                          std::int64_t comparand);
    RAINY_TOOLKIT_API long interlocked_and(volatile long *value, long mask);
    RAINY_TOOLKIT_API long interlocked_or(volatile long *value, long mask);
    RAINY_TOOLKIT_API long interlocked_xor(volatile long *value, long mask);
    RAINY_TOOLKIT_API void iso_volatile_store32(volatile int *address, std::uint32_t value);
    RAINY_TOOLKIT_API void iso_volatile_store64(volatile long long *address, std::uint64_t value);
    RAINY_TOOLKIT_API void iso_memory_fence();
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
    RAINY_TOOLKIT_API bool is_aligned(void * ptr, std::size_t alignment);
    RAINY_TOOLKIT_API void *allocate(std::size_t size, std::size_t alignment) noexcept;
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
    RAINY_TOOLKIT_API io_size_t write(std::uintptr_t stream, czstring buffer, io_size_t count);
    RAINY_TOOLKIT_API io_size_t write_line(std::uintptr_t stream, czstring buffer);
}

#endif