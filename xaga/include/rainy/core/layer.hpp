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
#define RAINY_CORE_LAYER_HPP // NOLINT
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
    /**
     * @brief File open mode flags.
     *        文件打开模式标志。
     *
     * Bitmask flags that specify how a file should be opened.
     * These flags can be combined using bitwise OR operations.
     * 指定文件打开方式的位掩码标志。
     * 这些标志可以通过按位或操作组合使用。
     */
    enum class open_mode : std::uint8_t {
        /**
         * @brief No state
         *        无状态
         *
         * Default state with no mode flags set.
         * 未设置任何模式标志的默认状态。
         */
        no_state = 0x00,

        /**
         * @brief Open for reading
         *        以读模式打开
         *
         * Open file for reading operations only.
         * 仅以读取操作方式打开文件。
         */
        read = 0x01,

        /**
         * @brief Open for writing
         *        以写模式打开
         *
         * Open file for writing operations only.
         * 仅以写入操作方式打开文件。
         */
        write = 0x02,

        /**
         * @brief Append mode
         *        追加模式
         *
         * All write operations append data to the end of the file.
         * 所有写入操作都将数据追加到文件末尾。
         */
        append = 0x04,

        /**
         * @brief Binary mode
         *        二进制模式
         *
         * Open file in binary mode (no text translation).
         * 以二进制模式打开文件（无文本转换）。
         */
        binary = 0x08,

        /**
         * @brief Truncate file
         *        截断文件
         *
         * Truncate file to zero length when opening.
         * 打开时将文件截断为零长度。
         */
        truncate = 0x10,

        /**
         * @brief Do not create
         *        不创建文件
         *
         * Fail if the file does not exist instead of creating it.
         * 如果文件不存在则失败，而不是创建它。
         */
        nocreate = 0x20,

        /**
         * @brief Update mode
         *        更新模式
         *
         * Open file for both reading and writing.
         * 以读写方式打开文件。
         */
        update = 0x40
    };

    /**
     * @brief Maximum number of stack frames to dump.
     *        要转储的最大堆栈帧数。
     *
     * Constant defining the upper limit for stack trace dumping operations.
     * 定义堆栈跟踪转储操作上限的常量。
     */
    constexpr int max_frames_dump = 128;

    /**
     * @brief File attributes enumeration.
     *        文件属性枚举。
     *
     * Bitmask flags representing various file system attributes.
     * 表示各种文件系统属性的位掩码标志。
     */
    enum class file_attributes : std::int16_t {
        /**
         * @brief Read-only file
         *        只读文件
         *
         * File is read-only and cannot be modified.
         * 文件为只读，无法修改。
         */
        read_only = 0x1,

        /**
         * @brief Hidden file
         *        隐藏文件
         *
         * File is hidden from normal directory listings.
         * 文件在常规目录列表中隐藏。
         */
        hidden = 0x2,

        /**
         * @brief System file
         *        系统文件
         *
         * File is part of or used exclusively by the operating system.
         * 文件是操作系统的组成部分或专供操作系统使用。
         */
        system = 0x4,

        /**
         * @brief Directory
         *        目录
         *
         * Entry represents a directory rather than a file.
         * 条目表示目录而非文件。
         */
        directory = 0x8,

        /**
         * @brief Archive file
         *        存档文件
         *
         * File has been modified since last backup (archive bit).
         * 文件自上次备份后已被修改（存档位）。
         */
        archive = 0x10,

        /**
         * @brief Device file
         *        设备文件
         *
         * Entry represents a device (used internally by the system).
         * 条目表示设备（由系统内部使用）。
         */
        device = 0x20,

        /**
         * @brief Normal file
         *        普通文件
         *
         * File has no other attributes set (typical regular file).
         * 文件未设置其他属性（典型的普通文件）。
         */
        normal = 0x40,

        /**
         * @brief Temporary file
         *        临时文件
         *
         * File is temporary and may be deleted by the system when not needed.
         * 文件为临时文件，系统可能在不需用时删除。
         */
        temporary = 0x80,

        /**
         * @brief Sparse file
         *        稀疏文件
         *
         * File contains large blocks of zeros that are not stored physically.
         * 文件包含未物理存储的大块零数据区域。
         */
        sparse_file = 0x100,

        /**
         * @brief Reparse point
         *        重解析点
         *
         * File or directory has associated reparse point (e.g., symlink, junction).
         * 文件或目录有关联的重解析点（如符号链接、接合点）。
         */
        reparse_point = 0x200,

        /**
         * @brief Invalid attributes
         *        无效属性
         *
         * Special value indicating that attributes are invalid or could not be retrieved.
         * 指示属性无效或无法获取的特殊值。
         */
        invalid = -1
    };

    /**
     * @brief Reparse tag types for symbolic links and junctions.
     *        符号链接和接合点的重解析标签类型。
     *
     * Identifies the type of reparse point for file system objects.
     * 标识文件系统对象的重解析点类型。
     */
    enum class reparse_tag : std::uint8_t {
        /**
         * @brief No reparse tag
         *        无重解析标签
         *
         * File or directory has no associated reparse point.
         * 文件或目录没有关联的重解析点。
         */
        none,

        /**
         * @brief Mount point
         *        挂载点
         *
         * Directory junction (mount point) that redirects to another directory.
         * 重定向到另一个目录的目录接合点（挂载点）。
         */
        mount_point,

        /**
         * @brief Symbolic link
         *        符号链接
         *
         * Symbolic link that points to another file or directory.
         * 指向另一个文件或目录的符号链接。
         */
        symlink
    };

    /**
     * @brief File status structure containing metadata.
     *        包含元数据的文件状态结构。
     *
     * Comprehensive structure holding various file metadata information.
     * 包含各种文件元数据信息的综合性结构。
     */
    struct file_status {
        /**
         * @brief Last write time
         *        最后写入时间
         *
         * Timestamp of the last modification to the file.
         * 文件最后一次修改的时间戳。
         */
        std::int64_t last_write_time;

        /**
         * @brief File size
         *        文件大小
         *
         * Size of the file in bytes.
         * 文件的字节大小。
         */
        std::uint64_t file_size;

        /**
         * @brief File attributes
         *        文件属性
         *
         * Bitmask of file attribute flags.
         * 文件属性标志的位掩码。
         */
        file_attributes attributes;

        /**
         * @brief Reparse tag
         *        重解析标签
         *
         * Type of reparse point if the file has one.
         * 如果文件有重解析点，则表示其类型。
         */
        pal::reparse_tag reparse_tag;

        /**
         * @brief Link count
         *        硬链接数量
         *
         * Number of hard links pointing to this file.
         * 指向此文件的硬链接数量。
         */
        std::uint32_t link_count;
    };

    /**
     * @brief File type enumeration.
     *        文件类型枚举。
     *
     * Categorizes file system entries by their fundamental type.
     * 根据基本类型对文件系统条目进行分类。
     */
    enum class file_type : std::uint8_t {
        /**
         * @brief Unknown type
         *        未知类型
         *
         * Type information not available or not yet determined.
         * 类型信息不可用或尚未确定。
         */
        none,

        /**
         * @brief File not found
         *        文件未找到
         *
         * File or directory does not exist at the specified path.
         * 指定路径上不存在文件或目录。
         */
        not_found,

        /**
         * @brief Regular file
         *        普通文件
         *
         * Regular file containing user data.
         * 包含用户数据的普通文件。
         */
        regular,

        /**
         * @brief Directory
         *        目录
         *
         * Directory that can contain other files and directories.
         * 可包含其他文件和目录的目录。
         */
        directory,

        /**
         * @brief Symbolic link
         *        符号链接
         *
         * Symbolic link pointing to another file system entry.
         * 指向另一个文件系统条目的符号链接。
         */
        symlink,

        /**
         * @brief Block device
         *        块设备
         *
         * Block-oriented device (e.g., hard disk, SSD).
         * 面向块的设备（如硬盘、SSD）。
         */
        block,

        /**
         * @brief Character device
         *        字符设备
         *
         * Character-oriented device (e.g., terminal, serial port).
         * 面向字符的设备（如终端、串口）。
         */
        character,

        /**
         * @brief FIFO (named pipe)
         *        命名管道
         *
         * First-in-first-out special file for interprocess communication.
         * 用于进程间通信的先进先出特殊文件。
         */
        fifo,

        /**
         * @brief Socket
         *        套接字
         *
         * Network socket or Unix domain socket.
         * 网络套接字或Unix域套接字。
         */
        socket,

        /**
         * @brief Unknown file type
         *        未知文件类型
         *
         * File exists but its type could not be determined.
         * 文件存在但无法确定其类型。
         */
        unknown,

        /**
         * @brief Directory junction
         *        目录接合点
         *
         * Directory junction (reparse point) redirecting to another directory.
         * 重定向到另一个目录的目录接合点（重解析点）。
         */
        junction
    };

    /**
     * @brief File handle type (opaque).
     *        文件句柄类型（不透明）。
     *
     * Opaque handle type representing an open file.
     * 表示已打开文件的不透明句柄类型。
     */
    using file_handle = std::uintptr_t;
}

namespace rainy::core::pal {
    /* debug tool */

    /**
     * @brief Triggers a debug breakpoint.
     *        触发调试断点。
     */
    RAINY_TOOLKIT_API rain_fn debug_break() -> void;

    /**
     * @brief Collects stack frames for debugging.
     *        收集堆栈帧用于调试。
     *
     * @param out_frames Output buffer for frame pointers
     *                   帧指针的输出缓冲区
     * @param max_frames_count Maximum number of frames to collect
     *                         要收集的最大帧数
     * @param skip Number of frames to skip from the top
     *             从顶部跳过的帧数
     * @return Actual number of frames collected
     *         实际收集的帧数
     */
    RAINY_TOOLKIT_API rain_fn collect_stack_frame(native_frame_ptr_t *out_frames, std::size_t max_frames_count,
                                                  std::size_t skip) noexcept -> std::size_t;

    /**
     * @brief Safely dumps stack frames to memory.
     *        安全地将堆栈帧转储到内存。
     *
     * @param memory Destination memory buffer
     *               目标内存缓冲区
     * @param size Size of the memory buffer
     *             内存缓冲区大小
     * @param skip Number of frames to skip
     *             要跳过的帧数
     * @return Number of bytes written
     *         写入的字节数
     */
    RAINY_TOOLKIT_API rain_fn safe_dump_to(void *memory, std::size_t size, std::size_t skip) noexcept -> std::size_t;

    /**
     * @brief Resolves a stack frame to a human-readable string.
     *        将堆栈帧解析为人类可读的字符串。
     *
     * @param frame The frame pointer to resolve
     *              要解析的帧指针
     * @param buf Output buffer for the resolved string
     *            解析后字符串的输出缓冲区
     * @param buf_size Size of the output buffer
     *                 输出缓冲区大小
     * @return true if resolution succeeded, false otherwise
     *         如果解析成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn resolve_stack_frame(native_frame_ptr_t frame, cstring buf, std::size_t buf_size) noexcept -> bool;

    /**
     * @brief Demangles a C++ name to a human-readable form.
     *        将C++名称解修饰为人类可读的形式。
     *
     * @param name The mangled name
     *             修饰后的名称
     * @param buf Output buffer for the demangled name
     *            解修饰后名称的输出缓冲区
     * @param buffer_length Size of the output buffer
     *                      输出缓冲区大小
     */
    RAINY_TOOLKIT_API rain_fn demangle(czstring name, cstring buf, std::size_t buffer_length) -> void;
}

namespace rainy::core::pal {
    /* atomic:: inc,dec */

    /**
     * @brief Atomically increments a long value.
     *        原子递增一个long值。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment(volatile long *value) -> long;

    /**
     * @brief Atomically increments an 8-bit integer.
     *        原子递增一个8位整数。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment8(volatile std::int8_t *value) -> std::int8_t;

    /**
     * @brief Atomically increments a 16-bit integer.
     *        原子递增一个16位整数。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment16(volatile std::int16_t *value) -> std::int16_t;

    /**
     * @brief Atomically increments a 32-bit integer.
     *        原子递增一个32位整数。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment32(volatile std::int32_t *value) -> std::int32_t;

    /**
     * @brief Atomically increments a 64-bit integer.
     *        原子递增一个64位整数。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment64(volatile std::int64_t *value) -> std::int64_t;

    /**
     * @brief Atomically decrements a long value.
     *        原子递减一个long值。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement(volatile long *value) -> long;

    /**
     * @brief Atomically decrements an 8-bit integer.
     *        原子递减一个8位整数。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement8(volatile std::int8_t *value) -> std::int8_t;

    /**
     * @brief Atomically decrements a 16-bit integer.
     *        原子递减一个16位整数。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement16(volatile std::int16_t *value) -> std::int16_t;

    /**
     * @brief Atomically decrements a 32-bit integer.
     *        原子递减一个32位整数。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement32(volatile std::int32_t *value) -> std::int32_t;

    /**
     * @brief Atomically decrements a 64-bit integer.
     *        原子递减一个64位整数。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement64(volatile std::int64_t *value) -> std::int64_t;

    /**
     * @brief Atomically adds a value to an 8-bit integer and returns the original value.
     *        原子地将一个值加到8位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add8(volatile std::int8_t *value, std::int8_t amount) -> std::int8_t;

    /**
     * @brief Atomically adds a value to a 16-bit integer and returns the original value.
     *        原子地将一个值加到16位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add16(volatile std::int16_t *value, std::int16_t amount) -> std::int16_t;

    /**
     * @brief Atomically adds a value to a 32-bit integer and returns the original value.
     *        原子地将一个值加到32位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add32(volatile std::int32_t *value, std::int32_t amount) -> std::int32_t;

    /**
     * @brief Atomically adds a value to a 64-bit integer and returns the original value.
     *        原子地将一个值加到64位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add64(volatile std::int64_t *value, std::int64_t amount) -> std::int64_t;

    /**
     * @brief Atomically subtracts a value from an integer and returns the original value.
     *        原子地从整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract(volatile std::intptr_t *value, const std::intptr_t amount)
        -> std::intptr_t;

    /**
     * @brief Atomically subtracts a value from an 8-bit integer and returns the original value.
     *        原子地从8位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract8(volatile std::int8_t *value, std::int8_t amount) -> std::int8_t;

    /**
     * @brief Atomically subtracts a value from a 16-bit integer and returns the original value.
     *        原子地从16位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract16(volatile std::int16_t *value, std::int16_t amount) -> std::int16_t;

    /**
     * @brief Atomically subtracts a value from a 32-bit integer and returns the original value.
     *        原子地从32位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract32(volatile std::int32_t *value, std::int32_t amount) -> std::int32_t;

    /**
     * @brief Atomically subtracts a value from a 64-bit integer and returns the original value.
     *        原子地从64位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract64(volatile std::int64_t *value, std::int64_t amount) -> std::int64_t;

    /* atomic::iso_volatile_load */

    /**
     * @brief Performs a volatile load of an integer.
     *        执行整数的volatile加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load(const volatile std::intptr_t *address) -> std::intptr_t;

    /**
     * @brief Performs a volatile load of an 8-bit integer.
     *        执行8位整数的volatile加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load8(const volatile std::int8_t *address) -> std::int8_t;

    /**
     * @brief Performs a volatile load of a 16-bit integer.
     *        执行16位整数的volatile加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load16(const volatile std::int16_t *address) -> std::int16_t;

    /**
     * @brief Performs a volatile load of a 32-bit integer.
     *        执行32位整数的volatile加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load32(const volatile std::int32_t *address) -> std::int32_t;

    /**
     * @brief Performs a volatile load of a 64-bit integer.
     *        执行64位整数的volatile加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load64(const volatile std::int64_t *address) -> std::int64_t;

    /**
     * @brief Atomically exchanges a value with a target.
     *        原子地交换目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange(volatile std::intptr_t *target, std::intptr_t value) -> std::intptr_t;

    /**
     * @brief Atomically exchanges an 8-bit value with a target.
     *        原子地交换8位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange8(volatile std::int8_t *target, std::int8_t value) -> std::int8_t;

    /**
     * @brief Atomically exchanges a 16-bit value with a target.
     *        原子地交换16位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange16(volatile std::int16_t *target, std::int16_t value) -> std::int16_t;

    /**
     * @brief Atomically exchanges a 32-bit value with a target.
     *        原子地交换32位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange32(volatile std::int32_t *target, std::int32_t value) -> std::int32_t;

    /**
     * @brief Atomically exchanges a 64-bit value with a target.
     *        原子地交换64位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange64(volatile std::int64_t *target, std::int64_t value) -> std::int64_t;

    /**
     * @brief Atomically exchanges a pointer value.
     *        原子地交换指针值。
     *
     * @param target Pointer to the target pointer
     *               目标指针的指针
     * @param value The new pointer value
     *              新的指针值
     * @return The original pointer value
     *         原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_pointer(volatile void **target, void *value) -> void *;

    /**
     * @brief Performs an atomic compare-and-exchange operation on a long value.
     *        对long值执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange(volatile long *destination, long exchange, long comparand) -> bool;

    /**
     * @brief Performs an atomic compare-and-exchange operation on an 8-bit value.
     *        对8位值执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange8(volatile std::int8_t *destination, std::int8_t exchange,
                                                            std::int8_t comparand) -> bool;

    /**
     * @brief Performs an atomic compare-and-exchange operation on a 16-bit value.
     *        对16位值执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange16(volatile std::int16_t *destination, std::int16_t exchange,
                                                             std::int16_t comparand) -> bool;

    /**
     * @brief Performs an atomic compare-and-exchange operation on a 32-bit value.
     *        对32位值执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange32(volatile std::int32_t *destination, std::int32_t exchange,
                                                             std::int32_t comparand) -> bool;

    /**
     * @brief Performs an atomic compare-and-exchange operation on a 64-bit value.
     *        对64位值执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange,
                                                             std::int64_t comparand) -> bool;

    /**
     * @brief Performs an atomic compare-and-exchange operation on a pointer.
     *        对指针执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination pointer
     *                    目标指针的指针
     * @param exchange The pointer value to exchange
     *                 要交换的指针值
     * @param comparand The pointer value to compare against
     *                  要比较的指针值
     * @return The original pointer value
     *         原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_pointer(void *volatile *destination, void *exchange, void *comparand)
        -> void *;

    /**
     * @brief Atomically performs a bitwise AND operation.
     *        原子地执行按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and(volatile std::intptr_t *value, const std::intptr_t mask) -> std::intptr_t;

    /**
     * @brief Atomically performs a bitwise AND operation on an 8-bit value.
     *        原子地对8位值执行按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and8(volatile std::int8_t *value, std::int8_t mask) -> std::int8_t;

    /**
     * @brief Atomically performs a bitwise AND operation on a 16-bit value.
     *        原子地对16位值执行按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and16(volatile std::int16_t *value, std::int16_t mask) -> std::int16_t;

    /**
     * @brief Atomically performs a bitwise AND operation on a 32-bit value.
     *        原子地对32位值执行按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and32(volatile std::int32_t *value, std::int32_t mask) -> std::int32_t;

    /**
     * @brief Atomically performs a bitwise AND operation on a 64-bit value.
     *        原子地对64位值执行按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and64(volatile std::int64_t *value, std::int64_t mask) -> std::int64_t;

    /**
     * @brief Atomically performs a bitwise OR operation.
     *        原子地执行按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or(volatile std::intptr_t *value, const std::intptr_t mask) -> std::intptr_t;

    /**
     * @brief Atomically performs a bitwise OR operation on an 8-bit value.
     *        原子地对8位值执行按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or8(volatile std::int8_t *value, std::int8_t mask) -> std::int8_t;

    /**
     * @brief Atomically performs a bitwise OR operation on a 16-bit value.
     *        原子地对16位值执行按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or16(volatile std::int16_t *value, std::int16_t mask) -> std::int16_t;

    /**
     * @brief Atomically performs a bitwise OR operation on a 32-bit value.
     *        原子地对32位值执行按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or32(volatile std::int32_t *value, std::int32_t mask) -> std::int32_t;

    /**
     * @brief Atomically performs a bitwise OR operation on a 64-bit value.
     *        原子地对64位值执行按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or64(volatile std::int64_t *value, std::int64_t mask) -> std::int64_t;

    /**
     * @brief Atomically performs a bitwise XOR operation.
     *        原子地执行按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor(volatile std::intptr_t *value, std::intptr_t mask) -> std::intptr_t;

    /**
     * @brief Atomically performs a bitwise XOR operation on an 8-bit value.
     *        原子地对8位值执行按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor8(volatile std::int8_t *value, std::int8_t mask) -> std::int8_t;

    /**
     * @brief Atomically performs a bitwise XOR operation on a 16-bit value.
     *        原子地对16位值执行按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor16(volatile std::int16_t *value, std::int16_t mask) -> std::int16_t;

    /**
     * @brief Atomically performs a bitwise XOR operation on a 32-bit value.
     *        原子地对32位值执行按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor32(volatile std::int32_t *value, std::int32_t mask) -> std::int32_t;

    /**
     * @brief Atomically performs a bitwise XOR operation on a 64-bit value.
     *        原子地对64位值执行按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor64(volatile std::int64_t *value, std::int64_t mask) -> std::int64_t;

    /**
     * @brief Performs a volatile store of a value.
     *        执行volatile存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store(volatile void *address, void *value) -> void;

    /**
     * @brief Performs a volatile store of an 8-bit value.
     *        执行8位值的volatile存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store8(volatile std::int8_t *address, std::int8_t value) -> void;

    /**
     * @brief Performs a volatile store of a 16-bit value.
     *        执行16位值的volatile存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store16(volatile std::int16_t *address, std::int16_t value) -> void;

    /**
     * @brief Performs a volatile store of a 32-bit value.
     *        执行32位值的volatile存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store32(volatile std::int32_t *address, std::int32_t value) -> void;

    /**
     * @brief Performs a volatile store of a 64-bit value.
     *        执行64位值的volatile存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store64(volatile std::int64_t *address, std::int64_t value) -> void;

    /**
     * @brief Inserts an atomic thread fence with the specified memory order.
     *        插入具有指定内存顺序的原子线程栅栏。
     *
     * @param order The memory order for the fence
     *              栅栏的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn atomic_thread_fence(const memory_order order) noexcept -> void;

    /**
     * @brief Inserts a read-write memory barrier.
     *        插入读写内存屏障。
     */
    RAINY_TOOLKIT_API rain_fn read_write_barrier() noexcept -> void;

    /**
     * @brief Inserts a read memory barrier.
     *        插入读内存屏障。
     */
    RAINY_TOOLKIT_API rain_fn read_barrier() noexcept -> void;

    /**
     * @brief Inserts a write memory barrier.
     *        插入写内存屏障。
     */
    RAINY_TOOLKIT_API rain_fn write_barrier() noexcept -> void;
}

namespace rainy::core::pal {
    /* file system */

    /**
     * @brief Gets the status of a file.
     *        获取文件状态。
     *
     * @param file_path Path to the file
     *                  文件路径
     * @return File status structure
     *         文件状态结构
     */
    RAINY_TOOLKIT_API rain_fn get_file_status(czstring file_path) noexcept -> file_status;

    /**
     * @brief Opens a file with the specified mode.
     *        以指定模式打开文件。
     *
     * @param filepath Path to the file
     *                 文件路径
     * @param mode Open mode flags
     *             打开模式标志
     * @return File handle
     *         文件句柄
     */
    RAINY_TOOLKIT_API rain_fn open_file(czstring filepath, open_mode mode) -> file_handle;

    /**
     * @brief Gets the type of file.
     *        获取文件类型。
     *
     * @param file_path Path to the file
     *                  文件路径
     * @return File type
     *         文件类型
     */
    RAINY_TOOLKIT_API rain_fn get_file_type(core::czstring file_path) noexcept -> file_type;

    /**
     * @brief Closes a file handle.
     *        关闭文件句柄。
     *
     * @param handle The file handle to close
     *               要关闭的文件句柄
     * @return true if successful, false otherwise
     *         如果成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn close_file(std::uintptr_t handle) -> bool;

    /**
     * @brief Checks if a file exists.
     *        检查文件是否存在。
     *
     * @param file_path Path to the file
     *                  文件路径
     * @return true if the file exists, false otherwise
     *         如果文件存在则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn file_exists(czstring file_path) -> bool;

    /**
     * @brief Creates a directory.
     *        创建目录。
     *
     * @param dir_path Path to the directory to create
     *                 要创建的目录路径
     * @return true if successful, false otherwise
     *         如果成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn create_directory(czstring dir_path) -> bool;

    /**
     * @brief Removes a file.
     *        删除文件。
     *
     * @param file_path Path to the file to remove
     *                  要删除的文件路径
     * @return true if successful, false otherwise
     *         如果成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn remove_file(czstring file_path) -> bool;

    /**
     * @brief Gets the size of a file.
     *        获取文件大小。
     *
     * @param file_path Path to the file
     *                  文件路径
     * @param size Reference to store the file size
     *             用于存储文件大小的引用
     * @return true if successful, false otherwise
     *         如果成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn get_file_size(czstring file_path, std::uint64_t &size) -> bool;

    /**
     * @brief Renames or moves a file.
     *        重命名或移动文件。
     *
     * @param old_path Current path
     *                 当前路径
     * @param new_path New path
     *                 新路径
     * @return true if successful, false otherwise
     *         如果成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn rename_file(czstring old_path, czstring new_path) -> bool;

    /* memory io */

    /**
     * @brief Checks if a pointer is aligned to the specified alignment.
     *        检查指针是否按指定对齐方式对齐。
     *
     * @param ptr The pointer to check
     *            要检查的指针
     * @param alignment The alignment requirement
     *                  对齐要求
     * @return true if aligned, false otherwise
     *         如果对齐则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn is_aligned(void *ptr, std::size_t alignment) -> bool;

    /**
     * @brief Allocates memory of the specified size.
     *        分配指定大小的内存。
     *
     * @param size The size to allocate in bytes
     *             要分配的字节数
     * @return Pointer to the allocated memory, or nullptr on failure
     *         指向已分配内存的指针，失败时返回nullptr
     */
    RAINY_TOOLKIT_API rain_fn allocate(std::size_t size) noexcept -> void *;

    /**
     * @brief Allocates aligned memory of the specified size.
     *        分配指定大小的对齐内存。
     *
     * @param size The size to allocate in bytes
     *             要分配的字节数
     * @param alignment The alignment requirement
     *                  对齐要求
     * @return Pointer to the allocated memory, or nullptr on failure
     *         指向已分配内存的指针，失败时返回nullptr
     */
    RAINY_TOOLKIT_API rain_fn allocate(std::size_t size, std::size_t alignment) noexcept -> void *;

    /**
     * @brief Deallocates memory previously allocated with allocate().
     *        释放之前使用allocate()分配的内存。
     *
     * @param block Pointer to the memory to deallocate
     *              要释放的内存指针
     */
    RAINY_TOOLKIT_API rain_fn deallocate(void *block) -> void;

    /**
     * @brief Deallocates aligned memory.
     *        释放对齐的内存。
     *
     * @param block Pointer to the memory to deallocate
     *              要释放的内存指针
     * @param alignment The alignment that was used for allocation
     *                  分配时使用的对齐方式
     */
    RAINY_TOOLKIT_API rain_fn deallocate(void *block, std::size_t alignment) -> void;

    /**
     * @brief Deallocates memory with full allocation parameters.
     *        使用完整的分配参数释放内存。
     *
     * @param ptr Pointer to the memory to deallocate
     *            要释放的内存指针
     * @param size The size that was allocated
     *             分配的大小
     * @param alignment The alignment that was used
     *                  使用的对齐方式
     */
    RAINY_TOOLKIT_API rain_fn deallocate(void *ptr, std::size_t size, std::size_t alignment) -> void;

    /* read io */

    /**
     * @brief Reads data from a stream.
     *        从流中读取数据。
     *
     * @param stream The stream handle
     *               流句柄
     * @param buffer Output buffer
     *               输出缓冲区
     * @param buffer_size Size of the output buffer
     *                    输出缓冲区大小
     * @return Number of bytes read, or -1 on error
     *         读取的字节数，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn read(std::uintptr_t stream, char *buffer, io_size_t buffer_size) -> io_size_t;

    /**
     * @brief Reads a specific number of bytes from a stream.
     *        从流中读取指定数量的字节。
     *
     * @param stream The stream handle
     *               流句柄
     * @param read_count Number of bytes to attempt to read
     *                   尝试读取的字节数
     * @param buffer Output buffer
     *               输出缓冲区
     * @param buffer_size Size of the output buffer
     *                    输出缓冲区大小
     * @return Number of bytes read, or -1 on error
     *         读取的字节数，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn read(std::uintptr_t stream, io_size_t read_count, char *buffer, io_size_t buffer_size) -> io_size_t;

    /**
     * @brief Reads data from a stream until a delimiter is encountered.
     *        从流中读取数据直到遇到分隔符。
     *
     * @param stream The stream handle
     *               流句柄
     * @param buffer Output buffer
     *               输出缓冲区
     * @param buffer_size Size of the output buffer
     *                    输出缓冲区大小
     * @param delimiter The delimiter character
     *                  分隔符字符
     * @return Number of bytes read, or -1 on error
     *         读取的字节数，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn read(std::uintptr_t stream, char *buffer, io_size_t buffer_size, char delimiter) -> io_size_t;

    /**
     * @brief Reads a line from a stream.
     *        从流中读取一行。
     *
     * @param stream The stream handle
     *               流句柄
     * @param buffer Output buffer
     *               输出缓冲区
     * @param buffer_size Size of the output buffer
     *                    输出缓冲区大小
     * @return Number of bytes read, or -1 on error
     *         读取的字节数，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn read_line(std::uintptr_t stream, char *buffer, io_size_t buffer_size) -> io_size_t;

    /**
     * @brief Reads a line from a stream until a delimiter.
     *        从流中读取一行直到遇到分隔符。
     *
     * @param stream The stream handle
     *               流句柄
     * @param buffer Output buffer
     *               输出缓冲区
     * @param buffer_size Size of the output buffer
     *                    输出缓冲区大小
     * @param delimiter The delimiter character
     *                  分隔符字符
     * @return Number of bytes read, or -1 on error
     *         读取的字节数，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn read_line(std::uintptr_t stream, char *buffer, io_size_t buffer_size, char delimiter) -> io_size_t;

    /**
     * @brief Reads binary data from a stream.
     *        从流中读取二进制数据。
     *
     * @param stream The stream handle
     *               流句柄
     * @param buffer Output buffer
     *               输出缓冲区
     * @param element_size Size of each element
     *                     每个元素的大小
     * @param element_count Number of elements to read
     *                      要读取的元素数量
     * @return Number of elements read, or -1 on error
     *         读取的元素数量，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn read_binary(std::uintptr_t stream, void *buffer, io_size_t element_size, io_size_t element_count) -> io_size_t;

    /* write io */

    /**
     * @brief Writes data to a stream.
     *        向流中写入数据。
     *
     * @param fd The file descriptor/handle
     *           文件描述符/句柄
     * @param buffer Source buffer
     *               源缓冲区
     * @param count Number of bytes to write
     *              要写入的字节数
     * @return Number of bytes written, or -1 on error
     *         写入的字节数，错误时返回-1
     */
    RAINY_TOOLKIT_API rain_fn write(std::uintptr_t fd, const void *buffer, std::size_t count) -> io_size_t;

    /**
     * @brief Flushes any buffered data to the stream.
     *        将任何缓冲的数据刷新到流中。
     *
     * @param fd The file descriptor/handle
     *           文件描述符/句柄
     */
    RAINY_TOOLKIT_API rain_fn flush(std::uintptr_t fd) -> void;

#if RAINY_USING_64_BIT_PLATFORM
    /**
     * @brief Performs a 128-bit atomic compare-and-exchange operation.
     *        执行128位原子比较并交换操作。
     *
     * @param destination Pointer to the destination 128-bit value
     *                    目标128位值的指针
     * @param exchange_high High 64 bits of the exchange value
     *                      交换值的高64位
     * @param exchange_low Low 64 bits of the exchange value
     *                     交换值的低64位
     * @param comparand_result Pointer to store the original value
     *                         用于存储原始值的指针
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange128(std::int64_t volatile *destination, std::int64_t exchange_high,
                                                           std::int64_t exchange_low, std::int64_t *comparand_result) -> bool;
#endif
}
namespace rainy::core::pal {
    /**
     * @brief Atomically increments a long value with specified memory order.
     *        使用指定的内存顺序原子递增一个long值。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment_explicit(volatile long *value, memory_order order) -> long;

    /**
     * @brief Atomically decrements a long value with specified memory order.
     *        使用指定的内存顺序原子递减一个long值。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement_explicit(volatile long *value, memory_order order) -> long;

    /**
     * @brief Atomically increments an 8-bit value with specified memory order.
     *        使用指定的内存顺序原子递增一个8位值。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment8_explicit(volatile std::int8_t *value, memory_order order) -> std::int8_t;

    /**
     * @brief Atomically increments a 16-bit value with specified memory order.
     *        使用指定的内存顺序原子递增一个16位值。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment16_explicit(volatile std::int16_t *value, memory_order order) -> std::int16_t;

    /**
     * @brief Atomically increments a 32-bit value with specified memory order.
     *        使用指定的内存顺序原子递增一个32位值。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment32_explicit(volatile std::int32_t *value, memory_order order) -> std::int32_t;

    /**
     * @brief Atomically increments a 64-bit value with specified memory order.
     *        使用指定的内存顺序原子递增一个64位值。
     *
     * @param value Pointer to the value to increment
     *              要递增的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the increment operation
     *         递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment64_explicit(volatile std::int64_t *value, memory_order order) -> std::int64_t;

    /**
     * @brief Atomically decrements an 8-bit value with specified memory order.
     *        使用指定的内存顺序原子递减一个8位值。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement8_explicit(volatile std::int8_t *value, memory_order order) -> std::int8_t;

    /**
     * @brief Atomically decrements a 16-bit value with specified memory order.
     *        使用指定的内存顺序原子递减一个16位值。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement16_explicit(volatile std::int16_t *value, memory_order order) -> std::int16_t;

    /**
     * @brief Atomically decrements a 32-bit value with specified memory order.
     *        使用指定的内存顺序原子递减一个32位值。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement32_explicit(volatile std::int32_t *value, memory_order order) -> std::int32_t;

    /**
     * @brief Atomically decrements a 64-bit value with specified memory order.
     *        使用指定的内存顺序原子递减一个64位值。
     *
     * @param value Pointer to the value to decrement
     *              要递减的值的指针
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The result of the decrement operation
     *         递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement64_explicit(volatile std::int64_t *value, memory_order order) -> std::int64_t;

    /**
     * @brief Atomically adds a value and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地加一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                                memory_order order) -> std::intptr_t;

    /**
     * @brief Atomically adds a value to an 8-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地将一个值加到8位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order)
        -> std::int8_t;

    /**
     * @brief Atomically adds a value to a 16-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地将一个值加到16位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add16_explicit(volatile std::int16_t *value, std::int16_t amount,
                                                                  memory_order order) -> std::int16_t;

    /**
     * @brief Atomically adds a value to a 32-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地将一个值加到32位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add32_explicit(volatile std::int32_t *value, std::int32_t amount,
                                                                  memory_order order) -> std::int32_t;

    /**
     * @brief Atomically adds a value to a 64-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地将一个值加到64位整数并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to add
     *               要增加的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before addition
     *         加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add64_explicit(volatile std::int64_t *value, std::int64_t amount,
                                                                  memory_order order) -> std::int64_t;

    /**
     * @brief Atomically subtracts a value and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地减一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                                     memory_order order) -> std::intptr_t;

    /**
     * @brief Atomically subtracts a value from an 8-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地从8位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract8_explicit(volatile std::int8_t *value, std::int8_t amount,
                                                                      memory_order order) -> std::int8_t;

    /**
     * @brief Atomically subtracts a value from a 16-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地从16位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract16_explicit(volatile std::int16_t *value, std::int16_t amount,
                                                                       memory_order order) -> std::int16_t;

    /**
     * @brief Atomically subtracts a value from a 32-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地从32位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract32_explicit(volatile std::int32_t *value, std::int32_t amount,
                                                                       memory_order order) -> std::int32_t;

    /**
     * @brief Atomically subtracts a value from a 64-bit integer and returns the original value, with specified memory order.
     *        使用指定的内存顺序原子地从64位整数减去一个值并返回原值。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param amount The amount to subtract
     *               要减去的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value before subtraction
     *         减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract64_explicit(volatile std::int64_t *value, std::int64_t amount,
                                                                       memory_order order) -> std::int64_t;

    /**
     * @brief Atomically exchanges a value with a target, with specified memory order.
     *        使用指定的内存顺序原子地交换目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_explicit(volatile std::intptr_t *target, std::intptr_t value, memory_order order)
        -> std::intptr_t;

    /**
     * @brief Atomically exchanges an 8-bit value with a target, with specified memory order.
     *        使用指定的内存顺序原子地交换8位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange8_explicit(volatile std::int8_t *target, std::int8_t value, memory_order order)
        -> std::int8_t;

    /**
     * @brief Atomically exchanges a 16-bit value with a target, with specified memory order.
     *        使用指定的内存顺序原子地交换16位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange16_explicit(volatile std::int16_t *target, std::int16_t value, memory_order order)
        -> std::int16_t;

    /**
     * @brief Atomically exchanges a 32-bit value with a target, with specified memory order.
     *        使用指定的内存顺序原子地交换32位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange32_explicit(volatile std::int32_t *target, std::int32_t value, memory_order order)
        -> std::int32_t;

    /**
     * @brief Atomically exchanges a 64-bit value with a target, with specified memory order.
     *        使用指定的内存顺序原子地交换64位目标值。
     *
     * @param target Pointer to the target value
     *               目标值的指针
     * @param value The new value to set
     *              要设置的新值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange64_explicit(volatile std::int64_t *target, std::int64_t value, memory_order order)
        -> std::int64_t;

    /**
     * @brief Atomically exchanges a pointer value, with specified memory order.
     *        使用指定的内存顺序原子地交换指针值。
     *
     * @param target Pointer to the target pointer
     *               目标指针的指针
     * @param value The new pointer value
     *              新的指针值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original pointer value
     *         原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_pointer_explicit(volatile void **target, void *value, memory_order order) -> void *;

    /**
     * @brief Performs an atomic compare-and-exchange operation with explicit memory orders.
     *        使用显式内存顺序执行原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @param success The memory ordering for success case
     *                成功情况的内存顺序
     * @param failure The memory ordering for failure case
     *                失败情况的内存顺序
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_explicit(volatile long *destination, long exchange, long comparand,
                                                                    memory_order success, memory_order failure) -> bool;

    /**
     * @brief Performs an 8-bit atomic compare-and-exchange operation with explicit memory orders.
     *        使用显式内存顺序执行8位原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @param success The memory ordering for success case
     *                成功情况的内存顺序
     * @param failure The memory ordering for failure case
     *                失败情况的内存顺序
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange8_explicit(volatile std::int8_t *destination, std::int8_t exchange,
                                                                     std::int8_t comparand, memory_order success, memory_order failure)
        -> bool;

    /**
     * @brief Performs a 16-bit atomic compare-and-exchange operation with explicit memory orders.
     *        使用显式内存顺序执行16位原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @param success The memory ordering for success case
     *                成功情况的内存顺序
     * @param failure The memory ordering for failure case
     *                失败情况的内存顺序
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange16_explicit(volatile std::int16_t *destination, std::int16_t exchange,
                                                                      std::int16_t comparand, memory_order success,
                                                                      memory_order failure) -> bool;

    /**
     * @brief Performs a 32-bit atomic compare-and-exchange operation with explicit memory orders.
     *        使用显式内存顺序执行32位原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @param success The memory ordering for success case
     *                成功情况的内存顺序
     * @param failure The memory ordering for failure case
     *                失败情况的内存顺序
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange32_explicit(volatile std::int32_t *destination, std::int32_t exchange,
                                                                      std::int32_t comparand, memory_order success,
                                                                      memory_order failure) -> bool;

    /**
     * @brief Performs a 64-bit atomic compare-and-exchange operation with explicit memory orders.
     *        使用显式内存顺序执行64位原子比较并交换操作。
     *
     * @param destination Pointer to the destination value
     *                    目标值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand The value to compare against
     *                  要比较的值
     * @param success The memory ordering for success case
     *                成功情况的内存顺序
     * @param failure The memory ordering for failure case
     *                失败情况的内存顺序
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange64_explicit(volatile std::int64_t *destination, std::int64_t exchange,
                                                                      std::int64_t comparand, memory_order success,
                                                                      memory_order failure) -> bool;

    /**
     * @brief Performs a pointer atomic compare-and-exchange operation with explicit memory orders.
     *        使用显式内存顺序执行指针原子比较并交换操作。
     *
     * @param destination Pointer to the destination pointer
     *                    目标指针的指针
     * @param exchange The pointer value to exchange
     *                 要交换的指针值
     * @param comparand The pointer value to compare against
     *                  要比较的指针值
     * @param success The memory ordering for success case
     *                成功情况的内存顺序
     * @param failure The memory ordering for failure case
     *                失败情况的内存顺序
     * @return The original pointer value
     *         原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_pointer_explicit(volatile void **destination, void *exchange,
                                                                            void *comparand, memory_order success,
                                                                            memory_order failure) -> void *;

    // 带内存序的原子位操作

    /**
     * @brief Atomically performs a bitwise AND operation with specified memory order.
     *        使用指定的内存顺序原子地执行按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order)
        -> std::intptr_t;

    /**
     * @brief Atomically performs an 8-bit bitwise AND operation with specified memory order.
     *        使用指定的内存顺序原子地执行8位按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and8_explicit(volatile int8_t *value, int8_t mask, memory_order order) -> int8_t;

    /**
     * @brief Atomically performs a 16-bit bitwise AND operation with specified memory order.
     *        使用指定的内存顺序原子地执行16位按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and16_explicit(volatile int16_t *value, int16_t mask, memory_order order) -> int16_t;

    /**
     * @brief Atomically performs a 32-bit bitwise AND operation with specified memory order.
     *        使用指定的内存顺序原子地执行32位按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and32_explicit(volatile int32_t *value, int32_t mask, memory_order order) -> int32_t;

    /**
     * @brief Atomically performs a 64-bit bitwise AND operation with specified memory order.
     *        使用指定的内存顺序原子地执行64位按位与操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to AND with
     *             要与之进行AND操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and64_explicit(volatile int64_t *value, int64_t mask, memory_order order) -> int64_t;

    /**
     * @brief Atomically performs a bitwise OR operation with specified memory order.
     *        使用指定的内存顺序原子地执行按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order)
        -> std::intptr_t;

    /**
     * @brief Atomically performs an 8-bit bitwise OR operation with specified memory order.
     *        使用指定的内存顺序原子地执行8位按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order)
        -> std::int8_t;

    /**
     * @brief Atomically performs a 16-bit bitwise OR operation with specified memory order.
     *        使用指定的内存顺序原子地执行16位按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order)
        -> std::int16_t;

    /**
     * @brief Atomically performs a 32-bit bitwise OR operation with specified memory order.
     *        使用指定的内存顺序原子地执行32位按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order)
        -> std::int32_t;

    /**
     * @brief Atomically performs a 64-bit bitwise OR operation with specified memory order.
     *        使用指定的内存顺序原子地执行64位按位或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to OR with
     *             要与之进行OR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order)
        -> std::int64_t;

    /**
     * @brief Atomically performs a bitwise XOR operation with specified memory order.
     *        使用指定的内存顺序原子地执行按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order)
        -> std::intptr_t;

    /**
     * @brief Atomically performs an 8-bit bitwise XOR operation with specified memory order.
     *        使用指定的内存顺序原子地执行8位按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order)
        -> std::int8_t;

    /**
     * @brief Atomically performs a 16-bit bitwise XOR operation with specified memory order.
     *        使用指定的内存顺序原子地执行16位按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order)
        -> std::int16_t;

    /**
     * @brief Atomically performs a 32-bit bitwise XOR operation with specified memory order.
     *        使用指定的内存顺序原子地执行32位按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order)
        -> std::int32_t;

    /**
     * @brief Atomically performs a 64-bit bitwise XOR operation with specified memory order.
     *        使用指定的内存顺序原子地执行64位按位异或操作。
     *
     * @param value Pointer to the value to modify
     *              要修改的值的指针
     * @param mask The mask to XOR with
     *             要与之进行XOR操作的掩码
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The original value
     *         原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order)
        -> std::int64_t;

    // 带内存序的原子加载

    /**
     * @brief Performs an atomic load with specified memory order.
     *        使用指定的内存顺序执行原子加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load_explicit(const volatile std::intptr_t *address, memory_order order) -> std::intptr_t;

    /**
     * @brief Performs an 8-bit atomic load with specified memory order.
     *        使用指定的内存顺序执行8位原子加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load8_explicit(const volatile std::int8_t *address, memory_order order) -> std::int8_t;

    /**
     * @brief Performs a 16-bit atomic load with specified memory order.
     *        使用指定的内存顺序执行16位原子加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load16_explicit(const volatile std::int16_t *address, memory_order order) -> std::int16_t;

    /**
     * @brief Performs a 32-bit atomic load with specified memory order.
     *        使用指定的内存顺序执行32位原子加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load32_explicit(const volatile std::int32_t *address, memory_order order) -> std::int32_t;

    /**
     * @brief Performs a 64-bit atomic load with specified memory order.
     *        使用指定的内存顺序执行64位原子加载。
     *
     * @param address The address to load from
     *                要加载的地址
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The loaded value
     *         加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load64_explicit(const volatile std::int64_t *address, memory_order order) -> std::int64_t;

    // 带内存序的原子存储

    /**
     * @brief Performs an atomic store with specified memory order.
     *        使用指定的内存顺序执行原子存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store_explicit(volatile void *address, void *value, memory_order order) -> void;

    /**
     * @brief Performs an 8-bit atomic store with specified memory order.
     *        使用指定的内存顺序执行8位原子存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store8_explicit(volatile std::int8_t *address, std::int8_t value, memory_order order)
        -> void;

    /**
     * @brief Performs a 16-bit atomic store with specified memory order.
     *        使用指定的内存顺序执行16位原子存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store16_explicit(volatile std::int16_t *address, std::int16_t value, memory_order order)
        -> void;

    /**
     * @brief Performs a 32-bit atomic store with specified memory order.
     *        使用指定的内存顺序执行32位原子存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store32_explicit(volatile std::int32_t *address, std::int32_t value, memory_order order)
        -> void;

    /**
     * @brief Performs a 64-bit atomic store with specified memory order.
     *        使用指定的内存顺序执行64位原子存储。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store64_explicit(volatile std::int64_t *address, std::int64_t value, memory_order order)
        -> void;
}

namespace rainy::core::pal {
    /**
     * @brief Function type for atomic wait equality comparison.
     *        原子等待相等比较的函数类型。
     *
     * @param storage Pointer to the storage location
     *                存储位置的指针
     * @param comparand Pointer to the value to compare against
     *                  要比较的值的指针
     * @param size Size of the values in bytes
     *             值的字节大小
     * @param ctx User context pointer
     *            用户上下文指针
     * @return true if the values are equal, false otherwise
     *         如果值相等则为true，否则为false
     */
    using atomic_wait_equal_fn = bool (*)(const void *storage, const void *comparand, std::size_t size, void *ctx) noexcept;

    /**
     * @brief Atomically waits for a value to change.
     *        原子等待值发生变化。
     *
     * @param storage Pointer to the storage location to wait on
     *                要等待的存储位置的指针
     * @param comparand Pointer to the value to compare against
     *                  要比较的值的指针
     * @param size Size of the values in bytes
     *             值的字节大小
     * @param equal_fn Optional equality comparison function (defaults to memcmp)
     *                 可选的相等比较函数（默认为memcmp）
     * @param ctx User context passed to equal_fn
     *            传递给equal_fn的用户上下文
     */
    RAINY_TOOLKIT_API rain_fn atomic_wait(const void *storage, const void *comparand, std::size_t size,
                                          atomic_wait_equal_fn equal_fn = nullptr, void *ctx = nullptr) noexcept -> void;

    /**
     * @brief Notifies one waiting thread that the storage location has changed.
     *        通知一个等待线程存储位置已更改。
     *
     * @param storage Pointer to the storage location that was waited on
     *                被等待的存储位置的指针
     * @param size Size of the value in bytes
     *             值的字节大小
     */
    RAINY_TOOLKIT_API rain_fn atomic_notify_one(const void *storage, std::size_t size) noexcept -> void;

    /**
     * @brief Notifies all waiting threads that the storage location has changed.
     *        通知所有等待线程存储位置已更改。
     *
     * @param storage Pointer to the storage location that was waited on
     *                被等待的存储位置的指针
     * @param size Size of the value in bytes
     *             值的字节大小
     */
    RAINY_TOOLKIT_API rain_fn atomic_notify_all(const void *storage, std::size_t size) noexcept -> void;
}

namespace rainy::core::pal {
    /**
     * @brief Double-word type for double-width atomic operations.
     *        用于双倍宽度原子操作的双字类型。
     *
     * This type represents a pair of values that can be operated on atomically
     * using double-word CAS instructions (CMPXCHG8B on x86, CMPXCHG16B on x64).
     *
     * 此类型表示可以使用双字CAS指令（x86上的CMPXCHG8B，x64上的CMPXCHG16B）
     * 进行原子操作的一对值。
     *
     * @tparam T The base word type (typically uint32_t or uint64_t)
     *           基本字类型（通常为uint32_t或uint64_t）
     */
    template <typename T>
    struct alignas(sizeof(T) * 2) double_word_t {
        T lo; ///< Low word (first word) / 低字（第一个字）
        T hi; ///< High word (second word) / 高字（第二个字）

        /**
         * @brief Equality comparison operator.
         *        相等比较运算符。
         *
         * @param other Another double_word_t to compare with
         *              要比较的另一个double_word_t
         * @return true if both words are equal, false otherwise
         *         如果两个字都相等则为true，否则为false
         */
        bool operator==(const double_word_t &other) const noexcept {
            return lo == other.lo && hi == other.hi;
        }

        /**
         * @brief Inequality comparison operator.
         *        不等比较运算符。
         *
         * @param other Another double_word_t to compare with
         *              要比较的另一个double_word_t
         * @return true if words differ, false otherwise
         *         如果两个字有差异则为true，否则为false
         */
        bool operator!=(const double_word_t &other) const noexcept {
            return !(*this == other);
        }
    };

    static_assert(sizeof(double_word_t<std::uint32_t>) == 8);
    static_assert(sizeof(double_word_t<std::uint64_t>) == 16);
    static_assert(sizeof(double_word_t<void *>) == sizeof(void *) * 2);

    /**
     * @brief Platform default double-word type, matching pointer width.
     *        平台默认双字类型，与指针等宽。
     *
     * This type directly corresponds to CMPXCHG8B (32-bit) or CMPXCHG16B (64-bit)
     * instructions.
     *
     * 此类型直接对应CMPXCHG8B（32位）或CMPXCHG16B（64位）指令。
     */
    using native_double_word_t = double_word_t<std::uintptr_t>;

    /**
     * @brief Performs an atomic double-word compare-and-exchange operation.
     *        执行原子双字比较并交换操作。
     *
     * @param destination Pointer to the destination double-word value
     *                    目标双字值的指针
     * @param exchange The value to exchange
     *                 要交换的值
     * @param comparand Pointer to the value to compare against.
     *                  On failure, updated to the current value.
     *                  要比较的值的指针。失败时更新为当前值。
     * @return true if the exchange occurred, false otherwise
     *         如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_double_word(volatile native_double_word_t *destination,
                                                                       native_double_word_t exchange,
                                                                       native_double_word_t *comparand) noexcept -> bool;

    /**
     * @brief Atomically loads a double-word value with specified memory order.
     *        使用指定的内存顺序原子加载双字值。
     *
     * @param address The address to load from
     *                要加载的地址
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     * @return The loaded double-word value
     *         加载的双字值
     */
    RAINY_TOOLKIT_API rain_fn atomic_load_double_word(const volatile native_double_word_t *address, memory_order order) noexcept
        -> native_double_word_t;

    /**
     * @brief Atomically stores a double-word value with specified memory order.
     *        使用指定的内存顺序原子存储双字值。
     *
     * @param address The address to store to
     *                要存储的地址
     * @param value The value to store
     *              要存储的值
     * @param order The memory ordering for the operation
     *              操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value,
                                                       memory_order order) noexcept -> void;

    /**
     * @brief Indicates whether double-word operations are always lock-free.
     *        指示双字操作是否始终是无锁的。
     *
     * This constant is true on x86/x64 platforms with appropriate instruction support,
     * false on platforms where double-word operations may require a mutex fallback.
     *
     * 在具有适当指令支持的x86/x64平台上为true，
     * 在双字操作可能需要互斥回退的平台上为false。
     */
    static constexpr bool is_always_lock_free =
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64))
        true;
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__amd64__))
        true;
#else
        false; // mutex fallback, including Clang/GCC without -mcx16
#endif
}

#endif
