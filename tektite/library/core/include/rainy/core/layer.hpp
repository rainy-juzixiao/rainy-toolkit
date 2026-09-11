/*
 * Copyright 2026 rainy-juzixiao
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

// @NODOCBEGIN
#define RAINY_INTERNALLAYER_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(EnumType)                                                             \
    inline constexpr EnumType operator|(EnumType left, EnumType right) {                                                              \
        using type = __underlying_type(EnumType);                                                                                     \
        return static_cast<EnumType>(static_cast<type>(left) | static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator&(EnumType left, EnumType right) {                                                              \
        using type = __underlying_type(EnumType);                                                                                     \
        return static_cast<EnumType>(static_cast<type>(left) & static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator^(EnumType left, EnumType right) {                                                              \
        using type = __underlying_type(EnumType);                                                                                     \
        return static_cast<EnumType>(static_cast<type>(left) ^ static_cast<type>(right));                                             \
    }                                                                                                                                 \
    inline constexpr EnumType operator~(EnumType val) {                                                                               \
        using type = __underlying_type(EnumType);                                                                                     \
        return static_cast<EnumType>(~static_cast<type>(val));                                                                        \
    }                                                                                                                                 \
    inline constexpr EnumType &operator|=(EnumType &left, EnumType right) {                                                           \
        left = left | right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline constexpr EnumType &operator&=(EnumType &left, EnumType right) {                                                           \
        left = left & right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline constexpr EnumType &operator^=(EnumType &left, EnumType right) {                                                           \
        left = left ^ right;                                                                                                          \
        return left;                                                                                                                  \
    }
// @NODOCEND

/* 这是一个用C函数库封装的底层调用空间。外部用户不应当使用它。推荐使用foundation提供的模块 */
namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Maximum number of stack frames to dump.
     *
     * Constant defining the upper limit for stack trace dumping operations.
     *
     * \lang simp-chinese
     * @brief 要转储的最大堆栈帧数。
     *
     * 定义堆栈跟踪转储操作上限的常量。
     */
    constexpr int max_frames_dump = 128;

    /**
     * \lang english
     * @brief File attributes enumeration.
     *
     * Bitmask flags representing various file system attributes.
     *
     * \lang simp-chinese
     * @brief 文件属性枚举。
     *
     * 表示各种文件系统属性的位掩码标志。
     */
    enum class file_attributes : std::int16_t {
        /**
         * \lang english
         * @brief Read-only file
         *
         * File is read-only and cannot be modified.
         *
         * \lang simp-chinese
         * @brief 只读文件
         *
         * 文件为只读，无法修改。
         */
        read_only = 0x1,

        /**
         * \lang english
         * @brief Hidden file
         *
         * File is hidden from normal directory listings.
         *
         * \lang simp-chinese
         * @brief 隐藏文件
         *
         * 文件在常规目录列表中隐藏。
         */
        hidden = 0x2,

        /**
         * \lang english
         * @brief System file
         *
         * File is part of or used exclusively by the operating system.
         *
         * \lang simp-chinese
         * @brief 系统文件
         *
         * 文件是操作系统的组成部分或专供操作系统使用。
         */
        system = 0x4,

        /**
         * \lang english
         * @brief Directory
         *
         * Entry represents a directory rather than a file.
         *
         * \lang simp-chinese
         * @brief 目录
         *
         * 条目表示目录而非文件。
         */
        directory = 0x8,

        /**
         * \lang english
         * @brief Archive file
         *
         * File has been modified since last backup (archive bit).
         *
         * \lang simp-chinese
         * @brief 存档文件
         *
         * 文件自上次备份后已被修改（存档位）。
         */
        archive = 0x10,

        /**
         * \lang english
         * @brief Device file
         *
         * Entry represents a device (used internally by the system).
         *
         * \lang simp-chinese
         * @brief 设备文件
         *
         * 条目表示设备（由系统内部使用）。
         */
        device = 0x20,

        /**
         * \lang english
         * @brief Normal file
         *
         * File has no other attributes set (typical regular file).
         *
         * \lang simp-chinese
         * @brief 普通文件
         *
         * 文件未设置其他属性（典型的普通文件）。
         */
        normal = 0x40,

        /**
         * \lang english
         * @brief Temporary file
         *
         * File is temporary and may be deleted by the system when not needed.
         *
         * \lang simp-chinese
         * @brief 临时文件
         *
         * 文件为临时文件，系统可能在不需用时删除。
         */
        temporary = 0x80,

        /**
         * \lang english
         * @brief Sparse file
         *
         * File contains large blocks of zeros that are not stored physically.
         *
         * \lang simp-chinese
         * @brief 稀疏文件
         *
         * 文件包含未物理存储的大块零数据区域。
         */
        sparse_file = 0x100,

        /**
         * \lang english
         * @brief Reparse point
         *
         * File or directory has associated reparse point (e.g., symlink, junction).
         *
         * \lang simp-chinese
         * @brief 重解析点
         *
         * 文件或目录有关联的重解析点（如符号链接、接合点）。
         */
        reparse_point = 0x200,

        /**
         * \lang english
         * @brief Invalid attributes
         *
         * Special value indicating that attributes are invalid or could not be retrieved.
         *
         * \lang simp-chinese
         * @brief 无效属性
         *
         * 指示属性无效或无法获取的特殊值。
         */
        invalid = -1
    };
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief File type enumeration.
     *
     * Identifies the type of file system entry.
     *
     * \lang simp-chinese
     * @brief 文件类型枚举。
     *
     * 标识文件系统条目的类型。
     */
    enum class file_type {
        /**
         * \lang english
         * @brief None or not found
         *
         * Special value indicating that the file type is none or the entry was not found.
         *
         * \lang simp-chinese
         * @brief 无或未找到
         *
         * 指示文件类型为无或未找到条目的特殊值。
         */
        none = -1,
        not_found = 0,

        /**
         * \lang english
         * @brief Regular file
         *
         * Entry is a regular file.
         *
         * \lang simp-chinese
         * @brief 普通文件
         *
         * 条目为普通文件。
         */
        regular = 1,

        /**
         * \lang english
         * @brief Directory
         *
         * Entry is a directory.
         *
         * \lang simp-chinese
         * @brief 目录
         *
         * 条目为目录。
         */
        directory = 2,

        /**
         * \lang english
         * @brief Symbolic link
         *
         * Entry is a symbolic link.
         *
         * \lang simp-chinese
         * @brief 符号链接
         *
         * 条目为符号链接。
         */
        symlink = 3,

        /**
         * \lang english
         * @brief Block device
         *
         * Entry represents a block device.
         *
         * \lang simp-chinese
         * @brief 块设备
         *
         * 条目表示块设备。
         */
        block = 4,

        /**
         * \lang english
         * @brief Character device
         *
         * Entry represents a character device.
         *
         * \lang simp-chinese
         * @brief 字符设备
         *
         * 条目表示字符设备。
         */
        character = 5,

        /**
         * \lang english
         * @brief FIFO (named pipe)
         *
         * Entry represents a FIFO or named pipe.
         *
         * \lang simp-chinese
         * @brief FIFO（命名管道）
         *
         * 条目表示FIFO或命名管道。
         */
        fifo = 6,

        /**
         * \lang english
         * @brief Socket
         *
         * Entry represents a socket.
         *
         * \lang simp-chinese
         * @brief 套接字
         *
         * 条目表示套接字。
         */
        socket = 7,

        /**
         * \lang english
         * @brief Unknown type
         *
         * Entry type is unknown or cannot be determined.
         *
         * \lang simp-chinese
         * @brief 未知类型
         *
         * 条目类型未知或无法确定。
         */
        unknown = 8
    };

    /**
     * \lang english
     * @brief File permissions enumeration.
     *
     * Bitmask flags representing POSIX-style file permissions and special bits.
     *
     * \lang simp-chinese
     * @brief 文件权限枚举。
     *
     * 表示POSIX风格文件权限和特殊位的位掩码标志。
     */
    enum class perms : unsigned {
        /**
         * \lang english
         * @brief No permissions
         *
         * No permission bits set.
         *
         * \lang simp-chinese
         * @brief 无权限
         *
         * 未设置任何权限位。
         */
        none = 0,

        // Owner permissions
        /**
         * \lang english
         * @brief Owner read permission
         *
         * Owner can read the file.
         *
         * \lang simp-chinese
         * @brief 所有者读权限
         *
         * 所有者可读取文件。
         */
        owner_read = 0400,

        /**
         * \lang english
         * @brief Owner write permission
         *
         * Owner can write to the file.
         *
         * \lang simp-chinese
         * @brief 所有者写权限
         *
         * 所有者可写入文件。
         */
        owner_write = 0200,

        /**
         * \lang english
         * @brief Owner execute permission
         *
         * Owner can execute the file.
         *
         * \lang simp-chinese
         * @brief 所有者执行权限
         *
         * 所有者可执行文件。
         */
        owner_exec = 0100,

        /**
         * \lang english
         * @brief Owner all permissions
         *
         * Owner has read, write, and execute permissions.
         *
         * \lang simp-chinese
         * @brief 所有者全部权限
         *
         * 所有者拥有读、写和执行权限。
         */
        owner_all = 0700,

        // Group permissions
        /**
         * \lang english
         * @brief Group read permission
         *
         * Group members can read the file.
         *
         * \lang simp-chinese
         * @brief 组读权限
         *
         * 组成员可读取文件。
         */
        group_read = 040,

        /**
         * \lang english
         * @brief Group write permission
         *
         * Group members can write to the file.
         *
         * \lang simp-chinese
         * @brief 组写权限
         *
         * 组成员可写入文件。
         */
        group_write = 020,

        /**
         * \lang english
         * @brief Group execute permission
         *
         * Group members can execute the file.
         *
         * \lang simp-chinese
         * @brief 组执行权限
         *
         * 组成员可执行文件。
         */
        group_exec = 010,

        /**
         * \lang english
         * @brief Group all permissions
         *
         * Group members have read, write, and execute permissions.
         *
         * \lang simp-chinese
         * @brief 组全部权限
         *
         * 组成员拥有读、写和执行权限。
         */
        group_all = 070,

        // Others permissions
        /**
         * \lang english
         * @brief Others read permission
         *
         * Others can read the file.
         *
         * \lang simp-chinese
         * @brief 其他用户读权限
         *
         * 其他用户可读取文件。
         */
        others_read = 04,

        /**
         * \lang english
         * @brief Others write permission
         *
         * Others can write to the file.
         *
         * \lang simp-chinese
         * @brief 其他用户写权限
         *
         * 其他用户可写入文件。
         */
        others_write = 02,

        /**
         * \lang english
         * @brief Others execute permission
         *
         * Others can execute the file.
         *
         * \lang simp-chinese
         * @brief 其他用户执行权限
         *
         * 其他用户可执行文件。
         */
        others_exec = 01,

        /**
         * \lang english
         * @brief Others all permissions
         *
         * Others have read, write, and execute permissions.
         *
         * \lang simp-chinese
         * @brief 其他用户全部权限
         *
         * 其他用户拥有读、写和执行权限。
         */
        others_all = 07,

        /**
         * \lang english
         * @brief All permissions
         *
         * All users have read, write, and execute permissions.
         *
         * \lang simp-chinese
         * @brief 全部权限
         *
         * 所有用户拥有读、写和执行权限。
         */
        all = 0777,

        // Special bits
        /**
         * \lang english
         * @brief Set user ID bit
         *
         * Executable runs with owner's privileges.
         *
         * \lang simp-chinese
         * @brief 设置用户ID位
         *
         * 可执行文件以所有者权限运行。
         */
        set_uid = 04000,

        /**
         * \lang english
         * @brief Set group ID bit
         *
         * Executable runs with group's privileges.
         *
         * \lang simp-chinese
         * @brief 设置组ID位
         *
         * 可执行文件以组权限运行。
         */
        set_gid = 02000,

        /**
         * \lang english
         * @brief Sticky bit
         *
         * Prevents deletion of files by non-owners in directories.
         *
         * \lang simp-chinese
         * @brief 粘滞位
         *
         * 防止非所有者删除目录中的文件。
         */
        sticky_bit = 01000,

        /**
         * \lang english
         * @brief Permission mask
         *
         * Mask covering all permission and special bits.
         *
         * \lang simp-chinese
         * @brief 权限掩码
         *
         * 覆盖所有权限位和特殊位的掩码。
         */
        mask = 07777,

        /**
         * \lang english
         * @brief Unknown permissions
         *
         * Special value indicating that permissions are unknown or cannot be retrieved.
         *
         * \lang simp-chinese
         * @brief 未知权限
         *
         * 指示权限未知或无法获取的特殊值。
         */
        unknown = 0xFFFF
    };

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    RAINY_INTERNALLAYER_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(perms);
#endif

    /**
     * \lang english
     * @brief Permission modification options enumeration.
     *
     * Flags specifying how permissions should be modified.
     *
     * \lang simp-chinese
     * @brief 权限修改选项枚举。
     *
     * 指定权限修改方式的标志。
     */
    enum class perm_options : unsigned {
        /**
         * \lang english
         * @brief Replace
         *
         * Replace existing permissions with new ones.
         *
         * \lang simp-chinese
         * @brief 替换
         *
         * 用新权限替换现有权限。
         */
        replace = 1,

        /**
         * \lang english
         * @brief Add
         *
         * Add specified permission bits to existing ones.
         *
         * \lang simp-chinese
         * @brief 追加
         *
         * 向现有权限追加指定权限位。
         */
        add = 2,

        /**
         * \lang english
         * @brief Remove
         *
         * Remove specified permission bits from existing ones.
         *
         * \lang simp-chinese
         * @brief 移除
         *
         * 从现有权限中移除指定权限位。
         */
        remove = 4,

        /**
         * \lang english
         * @brief No follow
         *
         * Do not follow symbolic links (operate on the link itself).
         *
         * \lang simp-chinese
         * @brief 不跟随
         *
         * 不跟随符号链接（对链接本身进行操作）。
         */
        nofollow = 8
    };

    /**
     * \lang english
     * @brief Copy operation options enumeration.
     *
     * Flags controlling the behavior of file and directory copy operations.
     *
     * \lang simp-chinese
     * @brief 复制操作选项枚举。
     *
     * 控制文件和目录复制操作行为的标志。
     */
    enum class copy_options : unsigned {
        /**
         * \lang english
         * @brief No options
         *
         * Default copy behavior.
         *
         * \lang simp-chinese
         * @brief 无选项
         *
         * 默认复制行为。
         */
        none = 0,

        /**
         * \lang english
         * @brief Skip existing
         *
         * Skip copying if the destination already exists.
         *
         * \lang simp-chinese
         * @brief 跳过已存在
         *
         * 如果目标已存在则跳过复制。
         */
        skip_existing = 1,

        /**
         * \lang english
         * @brief Overwrite existing
         *
         * Overwrite the destination if it already exists.
         *
         * \lang simp-chinese
         * @brief 覆盖已存在
         *
         * 如果目标已存在则覆盖。
         */
        overwrite_existing = 2,

        /**
         * \lang english
         * @brief Update existing
         *
         * Overwrite only if the source is newer than the destination.
         *
         * \lang simp-chinese
         * @brief 更新已存在
         *
         * 仅当源文件比目标文件更新时覆盖。
         */
        update_existing = 4,

        /**
         * \lang english
         * @brief Recursive
         *
         * Recursively copy directories and their contents.
         *
         * \lang simp-chinese
         * @brief 递归
         *
         * 递归复制目录及其内容。
         */
        recursive = 8,

        /**
         * \lang english
         * @brief Copy symlinks
         *
         * Copy symbolic links as links (not their targets).
         *
         * \lang simp-chinese
         * @brief 复制符号链接
         *
         * 将符号链接作为链接复制（而非其目标）。
         */
        copy_symlinks = 16,

        /**
         * \lang english
         * @brief Skip symlinks
         *
         * Ignore symbolic links during copy.
         *
         * \lang simp-chinese
         * @brief 忽略符号链接
         *
         * 复制时忽略符号链接。
         */
        skip_symlinks = 32,

        /**
         * \lang english
         * @brief Directories only
         *
         * Copy only the directory structure (not files).
         *
         * \lang simp-chinese
         * @brief 仅目录
         *
         * 仅复制目录结构（不复制文件）。
         */
        directories_only = 64,

        /**
         * \lang english
         * @brief Create symlinks
         *
         * Create symbolic links instead of copying files.
         *
         * \lang simp-chinese
         * @brief 创建符号链接
         *
         * 创建符号链接而非复制文件。
         */
        create_symlinks = 128,

        /**
         * \lang english
         * @brief Create hard links
         *
         * Create hard links instead of copying files.
         *
         * \lang simp-chinese
         * @brief 创建硬链接
         *
         * 创建硬链接而非复制文件。
         */
        create_hard_links = 256
    };

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
    RAINY_INTERNALLAYER_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(copy_options);
#endif

    /**
     * \lang english
     * @brief Directory iteration options enumeration.
     *
     * Flags controlling the behavior of directory iteration operations.
     *
     * \lang simp-chinese
     * @brief 目录遍历选项枚举。
     *
     * 控制目录遍历操作行为的标志。
     */
    enum class directory_options : unsigned {
        /**
         * \lang english
         * @brief No options
         *
         * Default directory iteration behavior.
         *
         * \lang simp-chinese
         * @brief 无选项
         *
         * 默认目录遍历行为。
         */
        none = 0,

        /**
         * \lang english
         * @brief Follow directory symlink
         *
         * Follow directory symbolic links during iteration.
         *
         * \lang simp-chinese
         * @brief 跟随目录符号链接
         *
         * 遍历时跟随目录符号链接。
         */
        follow_directory_symlink = 1,

        /**
         * \lang english
         * @brief Skip permission denied
         *
         * Skip directories that cannot be accessed due to permission errors instead of throwing exceptions.
         *
         * \lang simp-chinese
         * @brief 跳过无权限目录
         *
         * 跳过因权限错误无法访问的目录，而不是抛出异常。
         */
        skip_permission_denied = 2
    };

    /**
     * \lang english
     * @brief Space information structure.
     *
     * Contains information about available space on a file system.
     *
     * \lang simp-chinese
     * @brief 空间信息结构体。
     *
     * 包含文件系统上可用空间的信息。
     */
    struct space_info {
        /**
         * \lang english
         * @brief Total capacity
         *
         * Total size of the file system in bytes.
         *
         * \lang simp-chinese
         * @brief 总容量
         *
         * 文件系统的总大小（以字节为单位）。
         */
        std::uintmax_t capacity;

        /**
         * \lang english
         * @brief Free space
         *
         * Total free space on the file system in bytes.
         *
         * \lang simp-chinese
         * @brief 空闲空间
         *
         * 文件系统上的总空闲空间（以字节为单位）。
         */
        std::uintmax_t free;

        /**
         * \lang english
         * @brief Available space
         *
         * Free space available to non-privileged processes.
         *
         * \lang simp-chinese
         * @brief 可用空间
         *
         * 非特权进程可用的空闲空间。
         */
        std::uintmax_t available;

        /**
         * \lang english
         * @brief Equality operator
         *
         * Compares two space_info structures for equality.
         *
         * @param left Left-hand side space_info object to compare
         * @param right Right-hand side space_info object to compare
         * @return true if capacity, free, and available members are all equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 相等运算符
         *
         * 比较两个 space_info 结构体是否相等。
         *
         * @param left 待比较的左侧 space_info 对象
         * @param right 待比较的右侧 space_info 对象
         * @return 若 capacity、free 和 available 成员均相等则返回 true，否则返回 false
         */
        friend bool operator==(const space_info &left, const space_info &right) = default;
    };
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief File status structure.
     *
     * Contains the type and permissions of a file system entry.
     *
     * \lang simp-chinese
     * @brief 文件状态结构体。
     *
     * 包含文件系统条目的类型和权限。
     */
    struct file_status {
        /**
         * \lang english
         * @brief File type
         *
         * The type of the file system entry.
         *
         * \lang simp-chinese
         * @brief 文件类型
         *
         * 文件系统条目的类型。
         */
        file_type type;

        /**
         * \lang english
         * @brief File permissions
         *
         * The permission bits of the file system entry.
         *
         * \lang simp-chinese
         * @brief 文件权限
         *
         * 文件系统条目的权限位。
         */
        perms permissions;
    };
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Get absolute path.
     *
     * Converts the given path to an absolute path using direct system native calls.
     * This implementation directly invokes the operating system's path resolution functions
     * (e.g., realpath on POSIX, GetFullPathNameW on Windows).
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., CreateFileW, GetFullPathNameW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Source path string (native character type)
     * @param out_buffer Output buffer for the resolved path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the resolved path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取绝对路径。
     *
     * 使用直接系统原生调用将给定路径转换为绝对路径。
     * 此实现直接调用操作系统的路径解析函数（例如 POSIX 上的 realpath，Windows 上的 GetFullPathNameW）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 CreateFileW、GetFullPathNameW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 源路径字符串（原生字符类型）
     * @param out_buffer 用于存储解析后路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回解析后路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t absolute_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Get canonical path.
     *
     * Resolves the given path to an absolute, normalized path without symlinks using OS native resolution.
     * This eliminates all symbolic links, dot (.), and dot-dot (..) components through direct system calls.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Source path string (native character type)
     * @param out_buffer Output buffer for the canonical path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the canonical path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取规范路径。
     *
     * 使用操作系统原生解析将给定路径解析为不含符号链接的绝对、标准化路径。
     * 通过直接系统调用消除所有符号链接、点（.）和点点（..）组件。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 源路径字符串（原生字符类型）
     * @param out_buffer 用于存储规范路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回规范路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t canonical_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Copy files or directories.
     *
     * Copies a file or directory from source to destination using default options.
     * Uses native OS copy APIs (e.g., copy_file_range on Linux, CopyFileW on Windows).
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., CopyFileW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param from Source path (native character type)
     * @param to Destination path (native character type)
     *
     * \lang simp-chinese
     * @brief 复制文件或目录。
     *
     * 使用默认选项将文件或目录从源复制到目标。
     * 使用原生操作系统复制API（例如 Linux 上的 copy_file_range，Windows 上的 CopyFileW）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 CopyFileW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param from 源路径（原生字符类型）
     * @param to 目标路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void copy_native(native_czstring from, native_czstring to);

    /**
     * \lang english
     * @brief Copy files or directories with options.
     *
     * Copies a file or directory from source to destination with specified options.
     * Uses native OS copy APIs with platform-specific option handling.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param from Source path (native character type)
     * @param to Destination path (native character type)
     * @param options Copy operation options
     *
     * \lang simp-chinese
     * @brief 使用选项复制文件或目录。
     *
     * 使用指定选项将文件或目录从源复制到目标。
     * 使用带有平台特定选项处理的原生操作系统复制API。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param from 源路径（原生字符类型）
     * @param to 目标路径（原生字符类型）
     * @param options 复制操作选项
     */
    RAINY_TOOLKIT_API void copy_native(native_czstring from, native_czstring to, copy_options options);

    /**
     * \lang english
     * @brief Copy a single file.
     *
     * Copies the contents of one file to another using native OS file I/O.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param from Source file path (native character type)
     * @param to Destination file path (native character type)
     * @return true on success, false on failure
     *
     * \lang simp-chinese
     * @brief 复制单个文件。
     *
     * 使用原生操作系统文件I/O将一个文件的内容复制到另一个文件。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param from 源文件路径（原生字符类型）
     * @param to 目标文件路径（原生字符类型）
     * @return 成功时返回 true，失败时返回 false
     */
    RAINY_TOOLKIT_API bool copy_file_native(native_czstring from, native_czstring to);

    /**
     * \lang english
     * @brief Copy a single file with options.
     *
     * Copies the contents of one file to another with specified options using native OS APIs.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param from Source file path (native character type)
     * @param to Destination file path (native character type)
     * @param option Copy operation option
     * @return true on success, false on failure
     *
     * \lang simp-chinese
     * @brief 使用选项复制单个文件。
     *
     * 使用原生操作系统API，以指定选项将一个文件的内容复制到另一个文件。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param from 源文件路径（原生字符类型）
     * @param to 目标文件路径（原生字符类型）
     * @param option 复制操作选项
     * @return 成功时返回 true，失败时返回 false
     */
    RAINY_TOOLKIT_API bool copy_file_native(native_czstring from, native_czstring to, copy_options option);

    /**
     * \lang english
     * @brief Copy a symbolic link.
     *
     * Copies a symbolic link (creates a new symlink pointing to the same target).
     * Uses native symlink creation APIs (e.g., symlink on POSIX, CreateSymbolicLinkW on Windows).
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., CreateSymbolicLinkW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param existing_symlink Existing symbolic link path (native character type)
     * @param new_symlink Path for the new symbolic link (native character type)
     *
     * \lang simp-chinese
     * @brief 复制符号链接。
     *
     * 复制符号链接（创建指向相同目标的新符号链接）。
     * 使用原生符号链接创建API（例如 POSIX 上的 symlink，Windows 上的 CreateSymbolicLinkW）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 CreateSymbolicLinkW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param existing_symlink 现有符号链接路径（原生字符类型）
     * @param new_symlink 新符号链接的路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void copy_symlink_native(native_czstring existing_symlink, native_czstring new_symlink);

    /**
     * \lang english
     * @brief Create directories for a path.
     *
     * Creates all directories in the given path that do not already exist.
     * Uses native mkdir or CreateDirectoryW calls recursively.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., CreateDirectoryW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path where directories should be created (native character type)
     * @return true if directories were created, false otherwise
     *
     * \lang simp-chinese
     * @brief 为路径创建目录。
     *
     * 创建给定路径中所有不存在的目录。
     * 递归使用原生 mkdir 或 CreateDirectoryW 调用。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 CreateDirectoryW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 需要创建目录的路径（原生字符类型）
     * @return 如果目录被创建则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool create_directories_native(native_czstring path);

    /**
     * \lang english
     * @brief Create a single directory.
     *
     * Creates the final directory in the given path.
     * Uses native mkdir or CreateDirectoryW call.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Directory path to create (native character type)
     * @return true if directory was created, false otherwise
     *
     * \lang simp-chinese
     * @brief 创建单个目录。
     *
     * 创建给定路径中的最后一个目录。
     * 使用原生 mkdir 或 CreateDirectoryW 调用。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要创建的目录路径（原生字符类型）
     * @return 如果目录被创建则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool create_directory_native(native_czstring path);

    /**
     * \lang english
     * @brief Create a directory with attributes from an existing directory.
     *
     * Reads security attributes from `existing_p` and applies them to the new directory `path`.
     * Uses native mkdir or CreateDirectoryW with the extracted attributes.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API. The attributes parameter is platform-specific.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Directory path to create (native character type)
     * @param existing_p Existing directory path to read attributes from (native character type)
     * @return true if directory was created, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用现有目录的属性创建目录。
     *
     * 从 `existing_p` 读取安全属性并应用到新目录 `path`。
     * 使用提取的属性调用原生 mkdir 或 CreateDirectoryW。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *        attributes 参数是平台特定的。
     *
     * @attention 使用 global errno 表示操作结果。
     *
     * @param path 要创建的目录路径（原生字符类型）
     * @param existing_p 用于读取属性的现有目录路径（原生字符类型）
     * @return 如果目录被新创建则返回 true，否则返回 false（包括已存在或失败）
     */
    RAINY_TOOLKIT_API bool create_directory_native(native_czstring path, native_czstring existing_p);

    /**
     * \lang english
     * @brief Create a directory symbolic link.
     *
     * Creates a symbolic link to a directory.
     * Uses native symlink creation APIs with directory flag.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., CreateSymbolicLinkW with SYMBOLIC_LINK_FLAG_DIRECTORY).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param to Target directory path (native character type)
     * @param new_symlink Path for the new symbolic link (native character type)
     *
     * \lang simp-chinese
     * @brief 创建目录符号链接。
     *
     * 创建指向目录的符号链接。
     * 使用带有目录标志的原生符号链接创建API。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如使用 SYMBOLIC_LINK_FLAG_DIRECTORY 标志的 CreateSymbolicLinkW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param to 目标目录路径（原生字符类型）
     * @param new_symlink 新符号链接的路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void create_directory_symlink_native(native_czstring to, native_czstring new_symlink);

    /**
     * \lang english
     * @brief Create a hard link.
     *
     * Creates a hard link from the target to the new link path.
     * Uses native hard link creation APIs (e.g., link on POSIX, CreateHardLinkW on Windows).
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., CreateHardLinkW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param to Target file path (native character type)
     * @param new_hard_link Path for the new hard link (native character type)
     *
     * \lang simp-chinese
     * @brief 创建硬链接。
     *
     * 从目标创建指向新链接路径的硬链接。
     * 使用原生硬链接创建API（例如 POSIX 上的 link，Windows 上的 CreateHardLinkW）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 CreateHardLinkW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param to 目标文件路径（原生字符类型）
     * @param new_hard_link 新硬链接的路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void create_hard_link_native(native_czstring to, native_czstring new_hard_link);

    /**
     * \lang english
     * @brief Create a symbolic link.
     *
     * Creates a symbolic link to the target.
     * Uses native symlink creation APIs (e.g., symlink on POSIX, CreateSymbolicLinkW on Windows).
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param to Target path (native character type)
     * @param new_symlink Path for the new symbolic link (native character type)
     *
     * \lang simp-chinese
     * @brief 创建符号链接。
     *
     * 创建指向目标的符号链接。
     * 使用原生符号链接创建API（例如 POSIX 上的 symlink，Windows 上的 CreateSymbolicLinkW）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param to 目标路径（原生字符类型）
     * @param new_symlink 新符号链接的路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void create_symlink_native(native_czstring to, native_czstring new_symlink);

    /**
     * \lang english
     * @brief Get current working directory.
     *
     * Writes the current working directory path to the output buffer.
     * Uses native getcwd or GetCurrentDirectoryW call.
     *
     * @note On Windows, the output buffer will contain UTF-8 encoded path converted from UTF-16
     *        obtained from GetCurrentDirectoryW.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param out_buffer Output buffer for the current directory path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the current path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取当前工作目录。
     *
     * 将当前工作目录路径写入输出缓冲区。
     * 使用原生 getcwd 或 GetCurrentDirectoryW 调用。
     *
     * @note 在 Windows 上，输出缓冲区将包含从 GetCurrentDirectoryW 获取并转换为 UTF-8 的路径。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param out_buffer 用于存储当前目录路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回当前路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t current_path_native(native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Set current working directory.
     *
     * Changes the current working directory to the specified path.
     * Uses native chdir or SetCurrentDirectoryW call.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., SetCurrentDirectoryW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path New current working directory path (native character type)
     *
     * \lang simp-chinese
     * @brief 设置当前工作目录。
     *
     * 将当前工作目录更改为指定路径。
     * 使用原生 chdir 或 SetCurrentDirectoryW 调用。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 SetCurrentDirectoryW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 新的当前工作目录路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void current_path_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if two paths refer to the same file system entry.
     *
     * Determines whether the two paths resolve to the same entity.
     * Uses native stat or GetFileInformationByHandleW to compare inode/file IDs.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path1 First path (native character type)
     * @param path2 Second path (native character type)
     * @return true if both paths refer to the same file, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查两个路径是否指向相同的文件系统条目。
     *
     * 确定两个路径是否解析为相同的实体。
     * 使用原生 stat 或 GetFileInformationByHandleW 比较 inode/文件 ID。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path1 第一个路径（原生字符类型）
     * @param path2 第二个路径（原生字符类型）
     * @return 如果两个路径指向相同文件则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool equivalent_native(native_czstring path1, native_czstring path2);

    /**
     * \lang english
     * @brief Check if a file exists.
     *
     * Determines if the given path refers to an existing file system entry.
     * Uses native access or GetFileAttributesW call.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., GetFileAttributesW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry exists, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查文件是否存在。
     *
     * 确定给定路径是否指向存在的文件系统条目。
     * 使用原生 access 或 GetFileAttributesW 调用。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 GetFileAttributesW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目存在则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool exists_native(native_czstring path);

    /**
     * \lang english
     * @brief Get file size.
     *
     * Retrieves the size of a file in bytes using native stat or GetFileSizeEx.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @param out_size Pointer to store the file size
     * @return true on success, false on failure
     *
     * \lang simp-chinese
     * @brief 获取文件大小。
     *
     * 使用原生 stat 或 GetFileSizeEx 获取文件的大小（以字节为单位）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @param out_size 用于存储文件大小的指针
     * @return 成功时返回 true，失败时返回 false
     */
    RAINY_TOOLKIT_API bool file_size_native(native_czstring path, std::uintmax_t *out_size);

    /**
     * \lang english
     * @brief Get file size.
     *
     * Retrieves the size of a file in bytes using native stat or GetFileSizeEx.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @return File size in bytes on success, 0 on error
     *
     * \lang simp-chinese
     * @brief 获取文件大小。
     *
     * 使用原生 stat 或 GetFileSizeEx 获取文件的大小（以字节为单位）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @return 成功时返回文件大小（以字节为单位），失败时返回 0
     */
    RAINY_TOOLKIT_API std::uintmax_t file_size_native(native_czstring path);

    /**
     * \lang english
     * @brief Get hard link count.
     *
     * Retrieves the number of hard links referring to the file using native stat or GetFileInformationByHandle.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @param out_count Pointer to store the hard link count
     * @return true on success, false on failure
     *
     * \lang simp-chinese
     * @brief 获取硬链接计数。
     *
     * 使用原生 stat 或 GetFileInformationByHandle 获取指向文件的硬链接数量。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @param out_count 用于存储硬链接计数的指针
     * @return 成功时返回 true，失败时返回 false
     */
    RAINY_TOOLKIT_API bool hard_link_count_native(native_czstring path, std::uintmax_t *out_count);

    /**
     * \lang english
     * @brief Get hard link count.
     *
     * Retrieves the number of hard links referring to the file using native stat or GetFileInformationByHandle.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @return Hard link count on success, 0 on error
     *
     * \lang simp-chinese
     * @brief 获取硬链接计数。
     *
     * 使用原生 stat 或 GetFileInformationByHandle 获取指向文件的硬链接数量。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @return 成功时返回硬链接计数，失败时返回 0
     */
    RAINY_TOOLKIT_API std::uintmax_t hard_link_count_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a block device.
     *
     * Determines if the given path refers to a block device using native stat or GetFileType.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a block device, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向块设备。
     *
     * 使用原生 stat 或 GetFileType 确定给定路径是否指向块设备。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是块设备则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_block_file_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a character device.
     *
     * Determines if the given path refers to a character device using native stat or GetFileType.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a character device, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向字符设备。
     *
     * 使用原生 stat 或 GetFileType 确定给定路径是否指向字符设备。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是字符设备则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_character_file_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a directory.
     *
     * Determines if the given path refers to a directory using native stat or GetFileAttributesW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a directory, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向目录。
     *
     * 使用原生 stat 或 GetFileAttributesW 确定给定路径是否指向目录。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是目录则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_directory_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a directory or file is empty.
     *
     * Determines if the given path refers to an empty file or directory.
     * For directories, uses native opendir/readdir or FindFirstFileW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查目录或文件是否为空。
     *
     * 确定给定路径是否指向空文件或空目录。
     * 对于目录，使用原生 opendir/readdir 或 FindFirstFileW。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目为空则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_empty_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a FIFO (named pipe).
     *
     * Determines if the given path refers to a FIFO using native stat.
     *
     * @note On Windows, named pipes have a different path format (\\\\.\\pipe\\...).
     *        The input path should be UTF-8 encoded and will be converted to UTF-16.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a FIFO, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向FIFO（命名管道）。
     *
     * 使用原生 stat 确定给定路径是否指向FIFO。
     *
     * @note 在 Windows 上，命名管道使用不同的路径格式（\\\\.\\pipe\\...）。
     *        输入路径应为 UTF-8 编码，并将转换为 UTF-16。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是FIFO则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_fifo_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to an "other" file type.
     *
     * Determines if the given path refers to a type that is not regular file, directory, or symlink.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is of other type, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向“其他”文件类型。
     *
     * 确定给定路径是否指向非常规文件、目录或符号链接的类型。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是其他类型则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_other_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a regular file.
     *
     * Determines if the given path refers to a regular file using native stat or GetFileAttributesW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a regular file, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向普通文件。
     *
     * 使用原生 stat 或 GetFileAttributesW 确定给定路径是否指向普通文件。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是普通文件则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_regular_file_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a socket.
     *
     * Determines if the given path refers to a socket using native stat (on POSIX).
     * On Windows, sockets are not represented as filesystem entries.
     *
     * @note On Windows, this function may always return false as sockets are not in the filesystem.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a socket, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向套接字。
     *
     * 使用原生 stat 确定给定路径是否指向套接字（在 POSIX 上）。
     * 在 Windows 上，套接字不作为文件系统条目表示。
     *
     * @note 在 Windows 上，此函数可能始终返回 false，因为套接字不在文件系统中。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是套接字则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_socket_native(native_czstring path);

    /**
     * \lang english
     * @brief Check if a path refers to a symbolic link.
     *
     * Determines if the given path refers to a symbolic link using native lstat or GetFileAttributesW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., GetFileAttributesW with FILE_ATTRIBUTE_REPARSE_POINT).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to check (native character type)
     * @return true if the entry is a symbolic link, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查路径是否指向符号链接。
     *
     * 使用原生 lstat 或 GetFileAttributesW 确定给定路径是否指向符号链接。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如带有 FILE_ATTRIBUTE_REPARSE_POINT 的 GetFileAttributesW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要检查的路径（原生字符类型）
     * @return 如果条目是符号链接则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool is_symlink_native(native_czstring path);

    /**
     * \lang english
     * @brief Get last write time.
     *
     * Retrieves the last modification time of a file using native stat or GetFileTime.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @param out_time Pointer to store the last write time
     * @return true on success, false on failure
     *
     * \lang simp-chinese
     * @brief 获取最后写入时间。
     *
     * 使用原生 stat 或 GetFileTime 获取文件的最后修改时间。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @param out_time 用于存储最后写入时间的指针
     * @return 成功时返回 true，失败时返回 false
     */
    RAINY_TOOLKIT_API bool last_write_time_native(native_czstring path, std::time_t *out_time);

    /**
     * \lang english
     * @brief Get last write time.
     *
     * Retrieves the last modification time of a file using native stat or GetFileTime.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @return Last write time on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取最后写入时间。
     *
     * 使用原生 stat 或 GetFileTime 获取文件的最后修改时间。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @return 成功时返回最后写入时间，失败时返回 -1
     */
    RAINY_TOOLKIT_API std::time_t last_write_time_native(native_czstring path);

    /**
     * \lang english
     * @brief Set last write time.
     *
     * Changes the last modification time of a file using native utimensat or SetFileTime.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @param new_time New last write time
     *
     * \lang simp-chinese
     * @brief 设置最后写入时间。
     *
     * 使用原生 utimensat 或 SetFileTime 更改文件的最后修改时间。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @param new_time 新的最后写入时间
     */
    RAINY_TOOLKIT_API void last_write_time_native(native_czstring path, std::time_t new_time);

    /**
     * \lang english
     * @brief Change file permissions.
     *
     * Modifies the permissions of a file system entry using native chmod or SetFileAttributesW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API. Windows permissions are limited compared to POSIX.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to the file (native character type)
     * @param prms Permissions to apply
     * @param opts Permission modification options (default: replace)
     *
     * \lang simp-chinese
     * @brief 更改文件权限。
     *
     * 使用原生 chmod 或 SetFileAttributesW 修改文件系统条目的权限。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *        Windows 的权限相比 POSIX 有限。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @param prms 要应用的权限
     * @param opts 权限修改选项（默认：替换）
     */
    RAINY_TOOLKIT_API void permissions_native(native_czstring path, perms prms, perm_options opts = perm_options::replace);

    /**
     * \lang english
     * @brief Get proximate path.
     *
     * Converts the given path to a relative path against the current directory.
     * Uses native path comparison and resolution.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to convert (native character type)
     * @param out_buffer Output buffer for the proximate path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the proximate path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取近似路径。
     *
     * 将给定路径转换为相对于当前目录的相对路径。
     * 使用原生路径比较和解析。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要转换的路径（原生字符类型）
     * @param out_buffer 用于存储近似路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回近似路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t proximate_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Get proximate path relative to base.
     *
     * Converts the given path to a relative path against the specified base path.
     * Uses native path comparison and resolution.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to convert (native character type)
     * @param base Base path for relative conversion (native character type)
     * @param out_buffer Output buffer for the proximate path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the proximate path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取相对于基路径的近似路径。
     *
     * 将给定路径转换为相对于指定基路径的相对路径。
     * 使用原生路径比较和解析。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要转换的路径（原生字符类型）
     * @param base 用于相对转换的基路径（原生字符类型）
     * @param out_buffer 用于存储近似路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回近似路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t proximate_native(native_czstring path, native_czstring base, native_cstring out_buffer,
                                               std::size_t buffer_size);

    /**
     * \lang english
     * @brief Read the target of a symbolic link.
     *
     * Reads the target path of a symbolic link using native readlink or GetFinalPathNameByHandleW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Symbolic link path (native character type)
     * @param out_buffer Output buffer for the target path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the target path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 读取符号链接的目标。
     *
     * 使用原生 readlink 或 GetFinalPathNameByHandleW 读取符号链接的目标路径。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 符号链接路径（原生字符类型）
     * @param out_buffer 用于存储目标路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回目标路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t read_symlink_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Get relative path.
     *
     * Converts the given path to a relative path against the current directory.
     * Uses native path comparison and resolution.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to convert (native character type)
     * @param out_buffer Output buffer for the relative path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the relative path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取相对路径。
     *
     * 将给定路径转换为相对于当前目录的相对路径。
     * 使用原生路径比较和解析。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要转换的路径（原生字符类型）
     * @param out_buffer 用于存储相对路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回相对路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t relative_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Get relative path to base.
     *
     * Converts the given path to a relative path against the specified base path.
     * Uses native path comparison and resolution.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to convert (native character type)
     * @param base Base path for relative conversion (native character type)
     * @param out_buffer Output buffer for the relative path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the relative path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取相对于基路径的相对路径。
     *
     * 将给定路径转换为相对于指定基路径的相对路径。
     * 使用原生路径比较和解析。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要转换的路径（原生字符类型）
     * @param base 用于相对转换的基路径（原生字符类型）
     * @param out_buffer 用于存储相对路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回相对路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t relative_native(native_czstring path, native_czstring base, native_cstring out_buffer,
                                              std::size_t buffer_size);

    /**
     * \lang english
     * @brief Remove a file or empty directory.
     *
     * Removes a single file or empty directory using native remove or DeleteFileW/RemoveDirectoryW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., DeleteFileW, RemoveDirectoryW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to remove (native character type)
     * @return true if removed successfully, false otherwise
     *
     * \lang simp-chinese
     * @brief 删除文件或空目录。
     *
     * 使用原生 remove 或 DeleteFileW/RemoveDirectoryW 删除单个文件或空目录。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 DeleteFileW、RemoveDirectoryW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要删除的路径（原生字符类型）
     * @return 如果成功删除则返回 true，否则返回 false
     */
    RAINY_TOOLKIT_API bool remove_native(native_czstring path);

    /**
     * \lang english
     * @brief Remove a file or directory recursively.
     *
     * Removes a file or directory and all its contents recursively using native directory traversal.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to remove (native character type)
     * @return Number of files and directories removed
     *
     * \lang simp-chinese
     * @brief 递归删除文件或目录。
     *
     * 使用原生目录遍历递归删除文件或目录及其所有内容。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要删除的路径（原生字符类型）
     * @return 删除的文件和目录数量
     */
    RAINY_TOOLKIT_API std::uintmax_t remove_all_native(native_czstring path);

    /**
     * \lang english
     * @brief Rename a file or directory.
     *
     * Renames or moves a file or directory using native rename or MoveFileW.
     *
     * @note On Windows, the input paths should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., MoveFileW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param from Source path (native character type)
     * @param to Destination path (native character type)
     *
     * \lang simp-chinese
     * @brief 重命名文件或目录。
     *
     * 使用原生 rename 或 MoveFileW 重命名或移动文件或目录。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 MoveFileW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param from 源路径（原生字符类型）
     * @param to 目标路径（原生字符类型）
     */
    RAINY_TOOLKIT_API void rename_native(native_czstring from, native_czstring to);

    /**
     * \lang english
     * @brief Resize a file.
     *
     * Changes the size of a file (truncates or extends) using native truncate or SetFilePointerEx/SetEndOfFile.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path File path (native character type)
     * @param size New size in bytes
     *
     * \lang simp-chinese
     * @brief 调整文件大小。
     *
     * 使用原生 truncate 或 SetFilePointerEx/SetEndOfFile 更改文件的大小（截断或扩展）。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 文件路径（原生字符类型）
     * @param size 新大小（以字节为单位）
     */
    RAINY_TOOLKIT_API void resize_file_native(native_czstring path, std::uintmax_t size);

    /**
     * \lang english
     * @brief Get file system space information.
     *
     * Retrieves space information for the file system containing the given path.
     * Uses native statvfs or GetDiskFreeSpaceExW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API (e.g., GetDiskFreeSpaceExW).
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Any path on the target file system (native character type)
     * @param out_info Pointer to store space information
     * @return true on success, false on failure
     *
     * \lang simp-chinese
     * @brief 获取文件系统空间信息。
     *
     * 获取包含给定路径的文件系统的空间信息。
     * 使用原生 statvfs 或 GetDiskFreeSpaceExW。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API
     *        （例如 GetDiskFreeSpaceExW）。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 目标文件系统上的任意路径（原生字符类型）
     * @param out_info 用于存储空间信息的指针
     * @return 成功时返回 true，失败时返回 false
     */
    RAINY_TOOLKIT_API bool space_native(native_czstring path, space_info *out_info);

    /**
     * \lang english
     * @brief Get file system space information.
     *
     * Retrieves space information for the file system containing the given path.
     * Uses native statvfs or GetDiskFreeSpaceExW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Any path on the target file system (native character type)
     * @return space_info structure containing capacity, free, and available space
     *
     * \lang simp-chinese
     * @brief 获取文件系统空间信息。
     *
     * 获取包含给定路径的文件系统的空间信息。
     * 使用原生 statvfs 或 GetDiskFreeSpaceExW。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 目标文件系统上的任意路径（原生字符类型）
     * @return 包含总容量、空闲空间和可用空间的 space_info 结构体
     */
    RAINY_TOOLKIT_API space_info space_native(native_czstring path);

    /**
     * \lang english
     * @brief Get file status.
     *
     * Retrieves the type and permissions of a file system entry (follows symlinks).
     * Uses native stat or GetFileAttributesW.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to query (native character type)
     * @return file_status structure containing type and permissions
     *
     * \lang simp-chinese
     * @brief 获取文件状态。
     *
     * 获取文件系统条目的类型和权限（跟随符号链接）。
     * 使用原生 stat 或 GetFileAttributesW。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要查询的路径（原生字符类型）
     * @return 包含类型和权限的 file_status 结构体
     */
    RAINY_TOOLKIT_API file_status status_native(native_czstring path);

    /**
     * \lang english
     * @brief Get symbolic link status.
     *
     * Retrieves the type and permissions of a symbolic link itself (does not follow symlinks).
     * Uses native lstat or GetFileAttributesW with FILE_FLAG_OPEN_REPARSE_POINT.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Path to query (native character type)
     * @return file_status structure containing type and permissions of the symlink
     *
     * \lang simp-chinese
     * @brief 获取符号链接状态。
     *
     * 获取符号链接本身的类型和权限（不跟随符号链接）。
     * 使用原生 lstat 或带有 FILE_FLAG_OPEN_REPARSE_POINT 的 GetFileAttributesW。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 要查询的路径（原生字符类型）
     * @return 包含符号链接类型和权限的 file_status 结构体
     */
    RAINY_TOOLKIT_API file_status symlink_status_native(native_czstring path);

    /**
     * \lang english
     * @brief Get temporary directory path.
     *
     * Retrieves the path to the directory for temporary files.
     * Uses native P_tmpdir, getenv("TMPDIR"), or GetTempPathW.
     *
     * @note On Windows, the output buffer will contain UTF-8 encoded path converted from UTF-16
     *        obtained from GetTempPathW.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param out_buffer Output buffer for the temporary directory path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the temporary directory path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取临时目录路径。
     *
     * 获取临时文件目录的路径。
     * 使用原生 P_tmpdir、getenv("TMPDIR") 或 GetTempPathW。
     *
     * @note 在 Windows 上，输出缓冲区将包含从 GetTempPathW 获取并转换为 UTF-8 的路径。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param out_buffer 用于存储临时目录路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回临时目录路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t temp_directory_path_native(native_cstring out_buffer, std::size_t buffer_size);

    /**
     * \lang english
     * @brief Get weakly canonical path.
     *
     * Converts the given path to a canonical-like form (may not resolve all components).
     * Uses native path resolution with fallback behavior.
     *
     * @note On Windows, the input path should be UTF-8 encoded and will be converted to UTF-16
     *        for the underlying W-API.
     *
     * @attention Uses global errno to indicate operation results.
     *
     * @param path Source path string (native character type)
     * @param out_buffer Output buffer for the weakly canonical path (native character type)
     * @param buffer_size Size of the output buffer in characters
     * @return Length of the weakly canonical path on success, -1 on error
     *
     * \lang simp-chinese
     * @brief 获取弱规范路径。
     *
     * 将给定路径转换为类似规范的形式（可能不会解析所有组件）。
     * 使用具有回退行为的原生路径解析。
     *
     * @note 在 Windows 上，输入路径应为 UTF-8 编码，并将转换为 UTF-16 以用于底层 W-API。
     *
     * @attention 使用全局 errno 表示操作结果。
     *
     * @param path 源路径字符串（原生字符类型）
     * @param out_buffer 用于存储弱规范路径的输出缓冲区（原生字符类型）
     * @param buffer_size 输出缓冲区的大小（以字符为单位）
     * @return 成功时返回弱规范路径的长度，失败时返回 -1
     */
    RAINY_TOOLKIT_API ssize_t weakly_canonical_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size);
}

namespace rainy::core::layer {
    /* debug tool */

    /**
     * \lang english
     * @brief Triggers a debug breakpoint.
     *
     * \lang simp-chinese
     * @brief 触发调试断点。
     */
    RAINY_TOOLKIT_API rain_fn debug_break() -> void;

    /**
     * \lang english
     * @brief Collects stack frames for debugging.
     *
     * @param out_frames Output buffer for frame pointers
     * @param max_frames_count Maximum number of frames to collect
     * @param skip Number of frames to skip from the top
     * @return Actual number of frames collected
     *
     * \lang simp-chinese
     * @brief 收集堆栈帧用于调试。
     *
     * @param out_frames 帧指针的输出缓冲区
     * @param max_frames_count 要收集的最大帧数
     * @param skip 从顶部跳过的帧数
     * @return 实际收集的帧数
     */
    RAINY_TOOLKIT_API rain_fn collect_stack_frame(native_frame_ptr_t *out_frames, std::size_t max_frames_count,
                                                  std::size_t skip) noexcept -> std::size_t;

    /**
     * \lang english
     * @brief Safely dumps stack frames to memory.
     *
     * @param memory Destination memory buffer
     * @param size Size of the memory buffer
     * @param skip Number of frames to skip
     * @return Number of bytes written
     *
     * \lang simp-chinese
     * @brief 安全地将堆栈帧转储到内存。
     *
     * @param memory 目标内存缓冲区
     * @param size 内存缓冲区大小
     * @param skip 要跳过的帧数
     * @return 写入的字节数
     */
    RAINY_TOOLKIT_API rain_fn safe_dump_to(void *memory, std::size_t size, std::size_t skip) noexcept -> std::size_t;

    /**
     * \lang english
     * @brief Resolves a stack frame to a human-readable string.
     *
     * @param frame The frame pointer to resolve
     * @param buf Output buffer for the resolved string
     * @param buf_size Size of the output buffer
     * @return true if resolution succeeded, false otherwise
     *
     * \lang simp-chinese
     * @brief 将堆栈帧解析为人类可读的字符串。
     *
     * @param frame 要解析的帧指针
     * @param buf 解析后字符串的输出缓冲区
     * @param buf_size 输出缓冲区大小
     * @return 如果解析成功则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn resolve_stack_frame(native_frame_ptr_t frame, cstring buf, std::size_t buf_size) noexcept -> bool;

    /**
     * \lang english
     * @brief Demangles a C++ name to a human-readable form.
     *
     * @param name The mangled name
     * @param buf Output buffer for the demangled name
     * @param buffer_length Size of the output buffer
     *
     * \lang simp-chinese
     * @brief 将C++名称解修饰为人类可读的形式。
     *
     * @param name 修饰后的名称
     * @param buf 解修饰后名称的输出缓冲区
     * @param buffer_length 输出缓冲区大小
     */
    RAINY_TOOLKIT_API rain_fn demangle(czstring name, cstring buf, std::size_t buffer_length) -> void;

    /**
     * \lang english
     * @brief Triggers a debug breakpoint.
     *
     * @note This function will always trigger a breakpoint regardless of whether
     *        a debugger is attached. Use with caution in production code.
     *
     * \lang simp-chinese
     * @brief 触发调试断点。
     *
     * @note 无论是否有调试器附加，此函数都会触发断点。在生产代码中请谨慎使用。
     */
    RAINY_TOOLKIT_API void breakpoint() noexcept;

    /**
     * \lang english
     * @brief Triggers a debug breakpoint only if a debugger is present.
     *
     * @note This function checks whether a debugger is attached before triggering
     *        the breakpoint, making it safer for use in debug builds.
     *
     * @see is_debugger_present()
     * @see breakpoint()
     *
     * \lang simp-chinese
     * @brief 仅在调试器存在时触发调试断点。
     *
     * @note 此函数在触发断点前会检查是否有调试器附加，使其在调试构建中使用更安全。
     */
    RAINY_TOOLKIT_API void breakpoint_if_debugging() noexcept;

    /**
     * \lang english
     * @brief Checks whether a debugger is currently attached to the process.
     *
     * @return true  A debugger is present
     * @return false No debugger is present
     *
     * @note This function is typically used to conditionally enable debug-only
     *        behavior such as breakpoints or logging.
     *
     * @see breakpoint_if_debugging()
     *
     * \lang simp-chinese
     * @brief 检查当前是否有调试器附加到进程。
     *
     * @return 存在调试器
     * @return 不存在调试器
     *
     * @note 此函数通常用于条件性地启用仅调试行为，如断点或日志记录。
     */
    RAINY_TOOLKIT_API bool is_debugger_present() noexcept;
}

namespace rainy::core::layer {
    /* atomic:: inc,dec */

    /**
     * \lang english
     * @brief Atomically increments a long value.
     *
     * @param value Pointer to the value to increment
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 原子递增一个long值。
     *
     * @param value 要递增的值的指针
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment(volatile long *value) -> long;

    /**
     * \lang english
     * @brief Atomically increments an 8-bit integer.
     *
     * @param value Pointer to the value to increment
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 原子递增一个8位整数。
     *
     * @param value 要递增的值的指针
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment8(volatile std::int8_t *value) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically increments a 16-bit integer.
     *
     * @param value Pointer to the value to increment
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 原子递增一个16位整数。
     *
     * @param value 要递增的值的指针
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment16(volatile std::int16_t *value) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically increments a 32-bit integer.
     *
     * @param value Pointer to the value to increment
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 原子递增一个32位整数。
     *
     * @param value 要递增的值的指针
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment32(volatile std::int32_t *value) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically increments a 64-bit integer.
     *
     * @param value Pointer to the value to increment
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 原子递增一个64位整数。
     *
     * @param value 要递增的值的指针
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment64(volatile std::int64_t *value) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically decrements a long value.
     *
     * @param value Pointer to the value to decrement
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 原子递减一个long值。
     *
     * @param value 要递减的值的指针
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement(volatile long *value) -> long;

    /**
     * \lang english
     * @brief Atomically decrements an 8-bit integer.
     *
     * @param value Pointer to the value to decrement
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 原子递减一个8位整数。
     *
     * @param value 要递减的值的指针
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement8(volatile std::int8_t *value) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically decrements a 16-bit integer.
     *
     * @param value Pointer to the value to decrement
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 原子递减一个16位整数。
     *
     * @param value 要递减的值的指针
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement16(volatile std::int16_t *value) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically decrements a 32-bit integer.
     *
     * @param value Pointer to the value to decrement
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 原子递减一个32位整数。
     *
     * @param value 要递减的值的指针
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement32(volatile std::int32_t *value) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically decrements a 64-bit integer.
     *
     * @param value Pointer to the value to decrement
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 原子递减一个64位整数。
     *
     * @param value 要递减的值的指针
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement64(volatile std::int64_t *value) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically adds a value to an 8-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 原子地将一个值加到8位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add8(volatile std::int8_t *value, std::int8_t amount) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically adds a value to a 16-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 原子地将一个值加到16位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add16(volatile std::int16_t *value, std::int16_t amount) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically adds a value to a 32-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 原子地将一个值加到32位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add32(volatile std::int32_t *value, std::int32_t amount) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically adds a value to a 64-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 原子地将一个值加到64位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add64(volatile std::int64_t *value, std::int64_t amount) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from an integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 原子地从整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract(volatile std::intptr_t *value, const std::intptr_t amount)
        -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from an 8-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 原子地从8位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract8(volatile std::int8_t *value, std::int8_t amount) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from a 16-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 原子地从16位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract16(volatile std::int16_t *value, std::int16_t amount) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from a 32-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 原子地从32位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract32(volatile std::int32_t *value, std::int32_t amount) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from a 64-bit integer and returns the original value.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 原子地从64位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract64(volatile std::int64_t *value, std::int64_t amount) -> std::int64_t;

    /* atomic::iso_volatile_load */

    /**
     * \lang english
     * @brief Performs a volatile load of an integer.
     *
     * @param address The address to load from
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 执行整数的volatile加载。
     *
     * @param address 要加载的地址
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load(const volatile std::intptr_t *address) -> std::intptr_t;

    /**
     * \lang english
     * @brief Performs a volatile load of an 8-bit integer.
     *
     * @param address The address to load from
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 执行8位整数的volatile加载。
     *
     * @param address 要加载的地址
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load8(const volatile std::int8_t *address) -> std::int8_t;

    /**
     * \lang english
     * @brief Performs a volatile load of a 16-bit integer.
     *
     * @param address The address to load from
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 执行16位整数的volatile加载。
     *
     * @param address 要加载的地址
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load16(const volatile std::int16_t *address) -> std::int16_t;

    /**
     * \lang english
     * @brief Performs a volatile load of a 32-bit integer.
     *
     * @param address The address to load from
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 执行32位整数的volatile加载。
     *
     * @param address 要加载的地址
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load32(const volatile std::int32_t *address) -> std::int32_t;

    /**
     * \lang english
     * @brief Performs a volatile load of a 64-bit integer.
     *
     * @param address The address to load from
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 执行64位整数的volatile加载。
     *
     * @param address 要加载的地址
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load64(const volatile std::int64_t *address) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically exchanges a value with a target.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地交换目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange(volatile std::intptr_t *target, std::intptr_t value) -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically exchanges an 8-bit value with a target.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地交换8位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange8(volatile std::int8_t *target, std::int8_t value) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically exchanges a 16-bit value with a target.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地交换16位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange16(volatile std::int16_t *target, std::int16_t value) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically exchanges a 32-bit value with a target.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地交换32位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange32(volatile std::int32_t *target, std::int32_t value) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically exchanges a 64-bit value with a target.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地交换64位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange64(volatile std::int64_t *target, std::int64_t value) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically exchanges a pointer value.
     *
     * @param target Pointer to the target pointer
     * @param value The new pointer value
     * @return The original pointer value
     *
     * \lang simp-chinese
     * @brief 原子地交换指针值。
     *
     * @param target 目标指针的指针
     * @param value 新的指针值
     * @return 原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_pointer(volatile void **target, void *value) -> void *;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation on a long value.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 对long值执行原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange(volatile long *destination, long exchange, long comparand) -> bool;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation on an 8-bit value.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 对8位值执行原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange8(volatile std::int8_t *destination, std::int8_t exchange,
                                                            std::int8_t comparand) -> bool;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation on a 16-bit value.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 对16位值执行原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange16(volatile std::int16_t *destination, std::int16_t exchange,
                                                             std::int16_t comparand) -> bool;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation on a 32-bit value.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 对32位值执行原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange32(volatile std::int32_t *destination, std::int32_t exchange,
                                                             std::int32_t comparand) -> bool;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation on a 64-bit value.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 对64位值执行原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange,
                                                             std::int64_t comparand) -> bool;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation on a pointer.
     *
     * @param destination Pointer to the destination pointer
     * @param exchange The pointer value to exchange
     * @param comparand The pointer value to compare against
     * @return The original pointer value
     *
     * \lang simp-chinese
     * @brief 对指针执行原子比较并交换操作。
     *
     * @param destination 目标指针的指针
     * @param exchange 要交换的指针值
     * @param comparand 要比较的指针值
     * @return 原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_pointer(volatile void **destination, void *exchange, void *comparand)
        -> void *;

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND operation.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地执行按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and(volatile std::intptr_t *value, const std::intptr_t mask) -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND operation on an 8-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对8位值执行按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and8(volatile std::int8_t *value, std::int8_t mask) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND operation on a 16-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对16位值执行按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and16(volatile std::int16_t *value, std::int16_t mask) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND operation on a 32-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对32位值执行按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and32(volatile std::int32_t *value, std::int32_t mask) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND operation on a 64-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对64位值执行按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and64(volatile std::int64_t *value, std::int64_t mask) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR operation.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地执行按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or(volatile std::intptr_t *value, const std::intptr_t mask) -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR operation on an 8-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对8位值执行按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or8(volatile std::int8_t *value, std::int8_t mask) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR operation on a 16-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对16位值执行按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or16(volatile std::int16_t *value, std::int16_t mask) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR operation on a 32-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对32位值执行按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or32(volatile std::int32_t *value, std::int32_t mask) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR operation on a 64-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对64位值执行按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or64(volatile std::int64_t *value, std::int64_t mask) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR operation.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地执行按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor(volatile std::intptr_t *value, std::intptr_t mask) -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR operation on an 8-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对8位值执行按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor8(volatile std::int8_t *value, std::int8_t mask) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR operation on a 16-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对16位值执行按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor16(volatile std::int16_t *value, std::int16_t mask) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR operation on a 32-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对32位值执行按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor32(volatile std::int32_t *value, std::int32_t mask) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR operation on a 64-bit value.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 原子地对64位值执行按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor64(volatile std::int64_t *value, std::int64_t mask) -> std::int64_t;

    /**
     * \lang english
     * @brief Performs a volatile store of a value.
     *
     * @param address The address to store to
     * @param value The value to store
     *
     * \lang simp-chinese
     * @brief 执行volatile存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store(volatile void *address, void *value) -> void;

    /**
     * \lang english
     * @brief Performs a volatile store of an 8-bit value.
     *
     * @param address The address to store to
     * @param value The value to store
     *
     * \lang simp-chinese
     * @brief 执行8位值的volatile存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store8(volatile std::int8_t *address, std::int8_t value) -> void;

    /**
     * \lang english
     * @brief Performs a volatile store of a 16-bit value.
     *
     * @param address The address to store to
     * @param value The value to store
     *
     * \lang simp-chinese
     * @brief 执行16位值的volatile存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store16(volatile std::int16_t *address, std::int16_t value) -> void;

    /**
     * \lang english
     * @brief Performs a volatile store of a 32-bit value.
     *
     * @param address The address to store to
     * @param value The value to store
     *
     * \lang simp-chinese
     * @brief 执行32位值的volatile存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store32(volatile std::int32_t *address, std::int32_t value) -> void;

    /**
     * \lang english
     * @brief Performs a volatile store of a 64-bit value.
     *
     * @param address The address to store to
     * @param value The value to store
     *
     * \lang simp-chinese
     * @brief 执行64位值的volatile存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store64(volatile std::int64_t *address, std::int64_t value) -> void;

    /**
     * \lang english
     * @brief Inserts an atomic thread fence with the specified memory order.
     *
     * @param order The memory order for the fence
     *
     * \lang simp-chinese
     * @brief 插入具有指定内存顺序的原子线程栅栏。
     *
     * @param order 栅栏的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn atomic_thread_fence(const memory_order order) noexcept -> void;

    /**
     * \lang english
     * @brief Inserts a read-write memory barrier.
     *
     * \lang simp-chinese
     * @brief 插入读写内存屏障。
     */
    RAINY_TOOLKIT_API rain_fn read_write_barrier() noexcept -> void;

    /**
     * \lang english
     * @brief Inserts a read memory barrier.
     *
     * \lang simp-chinese
     * @brief 插入读内存屏障。
     */
    RAINY_TOOLKIT_API rain_fn read_barrier() noexcept -> void;

    /**
     * \lang english
     * @brief Inserts a write memory barrier.
     *
     * \lang simp-chinese
     * @brief 插入写内存屏障。
     */
    RAINY_TOOLKIT_API rain_fn write_barrier() noexcept -> void;
}

namespace rainy::core::layer {
#if RAINY_USING_64_BIT_PLATFORM
    /**
     * \lang english
     * @brief Performs a 128-bit atomic compare-and-exchange operation.
     *
     * @param destination Pointer to the destination 128-bit value
     * @param exchange_high 64 bits of the exchange value
     * @param exchange_low Low 64 bits of the exchange value
     * @param comparand_result Pointer to store the original value
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 执行128位原子比较并交换操作。
     *
     * @param destination 目标128位值的指针
     * @param exchange_high 交换值的高64位
     * @param exchange_low 交换值的低64位
     * @param comparand_result 用于存储原始值的指针
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange128(std::int64_t volatile *destination, std::int64_t exchange_high,
                                                              std::int64_t exchange_low, std::int64_t *comparand_result) -> bool;
#endif

    /**
     * \lang english
     * @brief Atomically increments a long value with specified memory order.
     *
     * @param value Pointer to the value to increment
     * @param order The memory ordering for the operation
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递增一个long值。
     *
     * @param value 要递增的值的指针
     * @param order 操作的内存顺序
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment_explicit(volatile long *value, memory_order order) -> long;

    /**
     * \lang english
     * @brief Atomically decrements a long value with specified memory order.
     *
     * @param value Pointer to the value to decrement
     * @param order The memory ordering for the operation
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递减一个long值。
     *
     * @param value 要递减的值的指针
     * @param order 操作的内存顺序
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement_explicit(volatile long *value, memory_order order) -> long;

    /**
     * \lang english
     * @brief Atomically increments an 8-bit value with specified memory order.
     *
     * @param value Pointer to the value to increment
     * @param order The memory ordering for the operation
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递增一个8位值。
     *
     * @param value 要递增的值的指针
     * @param order 操作的内存顺序
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment8_explicit(volatile std::int8_t *value, memory_order order) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically increments a 16-bit value with specified memory order.
     *
     * @param value Pointer to the value to increment
     * @param order The memory ordering for the operation
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递增一个16位值。
     *
     * @param value 要递增的值的指针
     * @param order 操作的内存顺序
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment16_explicit(volatile std::int16_t *value, memory_order order) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically increments a 32-bit value with specified memory order.
     *
     * @param value Pointer to the value to increment
     * @param order The memory ordering for the operation
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递增一个32位值。
     *
     * @param value 要递增的值的指针
     * @param order 操作的内存顺序
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment32_explicit(volatile std::int32_t *value, memory_order order) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically increments a 64-bit value with specified memory order.
     *
     * @param value Pointer to the value to increment
     * @param order The memory ordering for the operation
     * @return The result of the increment operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递增一个64位值。
     *
     * @param value 要递增的值的指针
     * @param order 操作的内存顺序
     * @return 递增操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_increment64_explicit(volatile std::int64_t *value, memory_order order) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically decrements an 8-bit value with specified memory order.
     *
     * @param value Pointer to the value to decrement
     * @param order The memory ordering for the operation
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递减一个8位值。
     *
     * @param value 要递减的值的指针
     * @param order 操作的内存顺序
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement8_explicit(volatile std::int8_t *value, memory_order order) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically decrements a 16-bit value with specified memory order.
     *
     * @param value Pointer to the value to decrement
     * @param order The memory ordering for the operation
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递减一个16位值。
     *
     * @param value 要递减的值的指针
     * @param order 操作的内存顺序
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement16_explicit(volatile std::int16_t *value, memory_order order) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically decrements a 32-bit value with specified memory order.
     *
     * @param value Pointer to the value to decrement
     * @param order The memory ordering for the operation
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递减一个32位值。
     *
     * @param value 要递减的值的指针
     * @param order 操作的内存顺序
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement32_explicit(volatile std::int32_t *value, memory_order order) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically decrements a 64-bit value with specified memory order.
     *
     * @param value Pointer to the value to decrement
     * @param order The memory ordering for the operation
     * @return The result of the decrement operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子递减一个64位值。
     *
     * @param value 要递减的值的指针
     * @param order 操作的内存顺序
     * @return 递减操作的结果
     */
    RAINY_TOOLKIT_API rain_fn interlocked_decrement64_explicit(volatile std::int64_t *value, memory_order order) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically adds a value and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @param order The memory ordering for the operation
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地加一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @param order 操作的内存顺序
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                                memory_order order) -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically adds a value to an 8-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @param order The memory ordering for the operation
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地将一个值加到8位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @param order 操作的内存顺序
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order)
        -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically adds a value to a 16-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @param order The memory ordering for the operation
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地将一个值加到16位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @param order 操作的内存顺序
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add16_explicit(volatile std::int16_t *value, std::int16_t amount,
                                                                  memory_order order) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically adds a value to a 32-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @param order The memory ordering for the operation
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地将一个值加到32位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @param order 操作的内存顺序
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add32_explicit(volatile std::int32_t *value, std::int32_t amount,
                                                                  memory_order order) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically adds a value to a 64-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to add
     * @param order The memory ordering for the operation
     * @return The original value before addition
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地将一个值加到64位整数并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要增加的值
     * @param order 操作的内存顺序
     * @return 加法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_add64_explicit(volatile std::int64_t *value, std::int64_t amount,
                                                                  memory_order order) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @param order The memory ordering for the operation
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地减一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @param order 操作的内存顺序
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                                     memory_order order) -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from an 8-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @param order The memory ordering for the operation
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地从8位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @param order 操作的内存顺序
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract8_explicit(volatile std::int8_t *value, std::int8_t amount,
                                                                      memory_order order) -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from a 16-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @param order The memory ordering for the operation
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地从16位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @param order 操作的内存顺序
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract16_explicit(volatile std::int16_t *value, std::int16_t amount,
                                                                       memory_order order) -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from a 32-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @param order The memory ordering for the operation
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地从32位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @param order 操作的内存顺序
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract32_explicit(volatile std::int32_t *value, std::int32_t amount,
                                                                       memory_order order) -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically subtracts a value from a 64-bit integer and returns the original value, with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param amount The amount to subtract
     * @param order The memory ordering for the operation
     * @return The original value before subtraction
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地从64位整数减去一个值并返回原值。
     *
     * @param value 要修改的值的指针
     * @param amount 要减去的值
     * @param order 操作的内存顺序
     * @return 减法前的原值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_subtract64_explicit(volatile std::int64_t *value, std::int64_t amount,
                                                                       memory_order order) -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically exchanges a value with a target, with specified memory order.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地交换目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_explicit(volatile std::intptr_t *target, std::intptr_t value, memory_order order)
        -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically exchanges an 8-bit value with a target, with specified memory order.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地交换8位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange8_explicit(volatile std::int8_t *target, std::int8_t value, memory_order order)
        -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically exchanges a 16-bit value with a target, with specified memory order.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地交换16位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange16_explicit(volatile std::int16_t *target, std::int16_t value, memory_order order)
        -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically exchanges a 32-bit value with a target, with specified memory order.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地交换32位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange32_explicit(volatile std::int32_t *target, std::int32_t value, memory_order order)
        -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically exchanges a 64-bit value with a target, with specified memory order.
     *
     * @param target Pointer to the target value
     * @param value The new value to set
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地交换64位目标值。
     *
     * @param target 目标值的指针
     * @param value 要设置的新值
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange64_explicit(volatile std::int64_t *target, std::int64_t value, memory_order order)
        -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically exchanges a pointer value, with specified memory order.
     *
     * @param target Pointer to the target pointer
     * @param value The new pointer value
     * @param order The memory ordering for the operation
     * @return The original pointer value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地交换指针值。
     *
     * @param target 目标指针的指针
     * @param value 新的指针值
     * @param order 操作的内存顺序
     * @return 原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_exchange_pointer_explicit(volatile void **target, void *value, memory_order order) -> void *;

    /**
     * \lang english
     * @brief Performs an atomic compare-and-exchange operation with explicit memory orders.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @param success The memory ordering for success case
     * @param failure The memory ordering for failure case
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用显式内存顺序执行原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @param success 成功情况的内存顺序
     * @param failure 失败情况的内存顺序
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_explicit(volatile long *destination, long exchange, long comparand,
                                                                    memory_order success, memory_order failure) -> bool;

    /**
     * \lang english
     * @brief Performs an 8-bit atomic compare-and-exchange operation with explicit memory orders.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @param success The memory ordering for success case
     * @param failure The memory ordering for failure case
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用显式内存顺序执行8位原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @param success 成功情况的内存顺序
     * @param failure 失败情况的内存顺序
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange8_explicit(volatile std::int8_t *destination, std::int8_t exchange,
                                                                     std::int8_t comparand, memory_order success, memory_order failure)
        -> bool;

    /**
     * \lang english
     * @brief Performs a 16-bit atomic compare-and-exchange operation with explicit memory orders.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @param success The memory ordering for success case
     * @param failure The memory ordering for failure case
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用显式内存顺序执行16位原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @param success 成功情况的内存顺序
     * @param failure 失败情况的内存顺序
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange16_explicit(volatile std::int16_t *destination, std::int16_t exchange,
                                                                      std::int16_t comparand, memory_order success,
                                                                      memory_order failure) -> bool;

    /**
     * \lang english
     * @brief Performs a 32-bit atomic compare-and-exchange operation with explicit memory orders.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @param success The memory ordering for success case
     * @param failure The memory ordering for failure case
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用显式内存顺序执行32位原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @param success 成功情况的内存顺序
     * @param failure 失败情况的内存顺序
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange32_explicit(volatile std::int32_t *destination, std::int32_t exchange,
                                                                      std::int32_t comparand, memory_order success,
                                                                      memory_order failure) -> bool;

    /**
     * \lang english
     * @brief Performs a 64-bit atomic compare-and-exchange operation with explicit memory orders.
     *
     * @param destination Pointer to the destination value
     * @param exchange The value to exchange
     * @param comparand The value to compare against
     * @param success The memory ordering for success case
     * @param failure The memory ordering for failure case
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用显式内存顺序执行64位原子比较并交换操作。
     *
     * @param destination 目标值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值
     * @param success 成功情况的内存顺序
     * @param failure 失败情况的内存顺序
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange64_explicit(volatile std::int64_t *destination, std::int64_t exchange,
                                                                      std::int64_t comparand, memory_order success,
                                                                      memory_order failure) -> bool;

    /**
     * \lang english
     * @brief Performs a pointer atomic compare-and-exchange operation with explicit memory orders.
     *
     * @param destination Pointer to the destination pointer
     * @param exchange The pointer value to exchange
     * @param comparand The pointer value to compare against
     * @param success The memory ordering for success case
     * @param failure The memory ordering for failure case
     * @return The original pointer value
     *
     * \lang simp-chinese
     * @brief 使用显式内存顺序执行指针原子比较并交换操作。
     *
     * @param destination 目标指针的指针
     * @param exchange 要交换的指针值
     * @param comparand 要比较的指针值
     * @param success 成功情况的内存顺序
     * @param failure 失败情况的内存顺序
     * @return 原始指针值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_pointer_explicit(volatile void **destination, void *exchange,
                                                                            void *comparand, memory_order success,
                                                                            memory_order failure) -> void *;

    // 带内存序的原子位操作

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order)
        -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically performs an 8-bit bitwise AND operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行8位按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and8_explicit(volatile int8_t *value, int8_t mask, memory_order order) -> int8_t;

    /**
     * \lang english
     * @brief Atomically performs a 16-bit bitwise AND operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行16位按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and16_explicit(volatile int16_t *value, int16_t mask, memory_order order) -> int16_t;

    /**
     * \lang english
     * @brief Atomically performs a 32-bit bitwise AND operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行32位按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and32_explicit(volatile int32_t *value, int32_t mask, memory_order order) -> int32_t;

    /**
     * \lang english
     * @brief Atomically performs a 64-bit bitwise AND operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to AND with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行64位按位与操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行AND操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_and64_explicit(volatile int64_t *value, int64_t mask, memory_order order) -> int64_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order)
        -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically performs an 8-bit bitwise OR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行8位按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order)
        -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically performs a 16-bit bitwise OR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行16位按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order)
        -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically performs a 32-bit bitwise OR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行32位按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order)
        -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically performs a 64-bit bitwise OR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to OR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行64位按位或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行OR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_or64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order)
        -> std::int64_t;

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order)
        -> std::intptr_t;

    /**
     * \lang english
     * @brief Atomically performs an 8-bit bitwise XOR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行8位按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order)
        -> std::int8_t;

    /**
     * \lang english
     * @brief Atomically performs a 16-bit bitwise XOR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行16位按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order)
        -> std::int16_t;

    /**
     * \lang english
     * @brief Atomically performs a 32-bit bitwise XOR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行32位按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order)
        -> std::int32_t;

    /**
     * \lang english
     * @brief Atomically performs a 64-bit bitwise XOR operation with specified memory order.
     *
     * @param value Pointer to the value to modify
     * @param mask The mask to XOR with
     * @param order The memory ordering for the operation
     * @return The original value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子地执行64位按位异或操作。
     *
     * @param value 要修改的值的指针
     * @param mask 要与之进行XOR操作的掩码
     * @param order 操作的内存顺序
     * @return 原始值
     */
    RAINY_TOOLKIT_API rain_fn interlocked_xor64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order)
        -> std::int64_t;

    // 带内存序的原子加载

    /**
     * \lang english
     * @brief Performs an atomic load with specified memory order.
     *
     * @param address The address to load from
     * @param order The memory ordering for the operation
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行原子加载。
     *
     * @param address 要加载的地址
     * @param order 操作的内存顺序
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load_explicit(const volatile std::intptr_t *address, memory_order order) -> std::intptr_t;

    /**
     * \lang english
     * @brief Performs an 8-bit atomic load with specified memory order.
     *
     * @param address The address to load from
     * @param order The memory ordering for the operation
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行8位原子加载。
     *
     * @param address 要加载的地址
     * @param order 操作的内存顺序
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load8_explicit(const volatile std::int8_t *address, memory_order order) -> std::int8_t;

    /**
     * \lang english
     * @brief Performs a 16-bit atomic load with specified memory order.
     *
     * @param address The address to load from
     * @param order The memory ordering for the operation
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行16位原子加载。
     *
     * @param address 要加载的地址
     * @param order 操作的内存顺序
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load16_explicit(const volatile std::int16_t *address, memory_order order) -> std::int16_t;

    /**
     * \lang english
     * @brief Performs a 32-bit atomic load with specified memory order.
     *
     * @param address The address to load from
     * @param order The memory ordering for the operation
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行32位原子加载。
     *
     * @param address 要加载的地址
     * @param order 操作的内存顺序
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load32_explicit(const volatile std::int32_t *address, memory_order order) -> std::int32_t;

    /**
     * \lang english
     * @brief Performs a 64-bit atomic load with specified memory order.
     *
     * @param address The address to load from
     * @param order The memory ordering for the operation
     * @return The loaded value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行64位原子加载。
     *
     * @param address 要加载的地址
     * @param order 操作的内存顺序
     * @return 加载的值
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_load64_explicit(const volatile std::int64_t *address, memory_order order) -> std::int64_t;

    // 带内存序的原子存储

    /**
     * \lang english
     * @brief Performs an atomic store with specified memory order.
     *
     * @param address The address to store to
     * @param value The value to store
     * @param order The memory ordering for the operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行原子存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     * @param order 操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store_explicit(volatile void *address, void *value, memory_order order) -> void;

    /**
     * \lang english
     * @brief Performs an 8-bit atomic store with specified memory order.
     *
     * @param address The address to store to
     * @param value The value to store
     * @param order The memory ordering for the operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行8位原子存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     * @param order 操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store8_explicit(volatile std::int8_t *address, std::int8_t value, memory_order order)
        -> void;

    /**
     * \lang english
     * @brief Performs a 16-bit atomic store with specified memory order.
     *
     * @param address The address to store to
     * @param value The value to store
     * @param order The memory ordering for the operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行16位原子存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     * @param order 操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store16_explicit(volatile std::int16_t *address, std::int16_t value, memory_order order)
        -> void;

    /**
     * \lang english
     * @brief Performs a 32-bit atomic store with specified memory order.
     *
     * @param address The address to store to
     * @param value The value to store
     * @param order The memory ordering for the operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行32位原子存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     * @param order 操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store32_explicit(volatile std::int32_t *address, std::int32_t value, memory_order order)
        -> void;

    /**
     * \lang english
     * @brief Performs a 64-bit atomic store with specified memory order.
     *
     * @param address The address to store to
     * @param value The value to store
     * @param order The memory ordering for the operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序执行64位原子存储。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     * @param order 操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn iso_volatile_store64_explicit(volatile std::int64_t *address, std::int64_t value, memory_order order)
        -> void;
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Function type for atomic wait equality comparison.
     *
     * @param storage Pointer to the storage location
     * @param comparand Pointer to the value to compare against
     * @param size Size of the values in bytes
     * @param ctx User context pointer
     * @return true if the values are equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 原子等待相等比较的函数类型。
     *
     * @param storage 存储位置的指针
     * @param comparand 要比较的值的指针
     * @param size 值的字节大小
     * @param ctx 用户上下文指针
     * @return 如果值相等则为true，否则为false
     */
    using atomic_wait_equal_fn = bool (*)(const void *storage, const void *comparand, std::size_t size, void *ctx) noexcept;

    /**
     * \lang english
     * @brief Atomically waits for a value to change.
     *
     * @param storage Pointer to the storage location to wait on
     * @param comparand Pointer to the value to compare against
     * @param size Size of the values in bytes
     * @param equal_fn Optional equality comparison function (defaults to memcmp)
     * @param ctx User context passed to equal_fn
     *
     * \lang simp-chinese
     * @brief 原子等待值发生变化。
     *
     * @param storage 要等待的存储位置的指针
     * @param comparand 要比较的值的指针
     * @param size 值的字节大小
     * @param equal_fn 可选的相等比较函数（默认为memcmp）
     * @param ctx 传递给equal_fn的用户上下文
     */
    RAINY_TOOLKIT_API rain_fn atomic_wait(const void *storage, const void *comparand, std::size_t size,
                                          atomic_wait_equal_fn equal_fn = nullptr, void *ctx = nullptr) noexcept -> void;

    /**
     * \lang english
     * @brief Notifies one waiting thread that the storage location has changed.
     *
     * @param storage Pointer to the storage location that was waited on
     * @param size Size of the value in bytes
     *
     * \lang simp-chinese
     * @brief 通知一个等待线程存储位置已更改。
     *
     * @param storage 被等待的存储位置的指针
     * @param size 值的字节大小
     */
    RAINY_TOOLKIT_API rain_fn atomic_notify_one(const void *storage, std::size_t size) noexcept -> void;

    /**
     * \lang english
     * @brief Notifies all waiting threads that the storage location has changed.
     *
     * @param storage Pointer to the storage location that was waited on
     * @param size Size of the value in bytes
     *
     * \lang simp-chinese
     * @brief 通知所有等待线程存储位置已更改。
     *
     * @param storage 被等待的存储位置的指针
     * @param size 值的字节大小
     */
    RAINY_TOOLKIT_API rain_fn atomic_notify_all(const void *storage, std::size_t size) noexcept -> void;
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Double-word type for double-width atomic operations.
     *
     * This type represents a pair of values that can be operated on atomically
     * using double-word CAS instructions (CMPXCHG8B on x86, CMPXCHG16B on x64).
     *
     * @tparam Ty The base word type (typically uint32_t or uint64_t)
     *
     * \lang simp-chinese
     * @brief 用于双倍宽度原子操作的双字类型。
     *
     * 此类型表示可以使用双字CAS指令（x86上的CMPXCHG8B，x64上的CMPXCHG16B）
     * 进行原子操作的一对值。
     *
     * @tparam Ty 基本字类型（通常为uint32_t或uint64_t）
     */
    template <typename Ty>
    struct alignas(sizeof(Ty) * 2) double_word_t {
        Ty lo; ///< Low word (first word) / 低字（第一个字）
        Ty hi; ///< High word (second word) / 高字（第二个字）

        /**
         * \lang english
         * @brief Equality comparison operator.
         *
         * @param other Another double_word_t to compare with
         * @return true if both words are equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 相等比较运算符。
         *
         * @param other 要比较的另一个double_word_t
         * @return 如果两个字都相等则为true，否则为false
         */
        bool operator==(const double_word_t &other) const noexcept {
            return lo == other.lo && hi == other.hi;
        }

        /**
         * \lang english
         * @brief Inequality comparison operator.
         *
         * @param other Another double_word_t to compare with
         * @return true if words differ, false otherwise
         *
         * \lang simp-chinese
         * @brief 不等比较运算符。
         *
         * @param other 要比较的另一个double_word_t
         * @return 如果两个字有差异则为true，否则为false
         */
        bool operator!=(const double_word_t &other) const noexcept {
            return !(*this == other);
        }
    };

    static_assert(sizeof(double_word_t<std::uint32_t>) == 8);
    static_assert(sizeof(double_word_t<std::uint64_t>) == 16);
    static_assert(sizeof(double_word_t<void *>) == sizeof(void *) * 2);

    /**
     * \lang english
     * @brief Platform default double-word type, matching pointer width.
     *
     * This type directly corresponds to CMPXCHG8B (32-bit) or CMPXCHG16B (64-bit)
     * instructions.
     *
     * \lang simp-chinese
     * @brief 平台默认双字类型，与指针等宽。
     *
     * 此类型直接对应CMPXCHG8B（32位）或CMPXCHG16B（64位）指令。
     */
    using native_double_word_t = double_word_t<std::uintptr_t>;

    /**
     * \lang english
     * @brief Performs an atomic double-word compare-and-exchange operation.
     *
     * @param destination Pointer to the destination double-word value
     * @param exchange The value to exchange
     * @param comparand Pointer to the value to compare against.
     * @param comparand On failure, updated to the current value.
     * @return true if the exchange occurred, false otherwise
     *
     * \lang simp-chinese
     * @brief 执行原子双字比较并交换操作。
     *
     * @param destination 目标双字值的指针
     * @param exchange 要交换的值
     * @param comparand 要比较的值的指针。失败时更新为当前值。
     * @return 如果交换发生则为true，否则为false
     */
    RAINY_TOOLKIT_API rain_fn interlocked_compare_exchange_double_word(volatile native_double_word_t *destination,
                                                                       native_double_word_t exchange,
                                                                       native_double_word_t *comparand) noexcept -> bool;

    /**
     * \lang english
     * @brief Atomically loads a double-word value with specified memory order.
     *
     * @param address The address to load from
     * @param order The memory ordering for the operation
     * @return The loaded double-word value
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子加载双字值。
     *
     * @param address 要加载的地址
     * @param order 操作的内存顺序
     * @return 加载的双字值
     */
    RAINY_TOOLKIT_API rain_fn atomic_load_double_word(const volatile native_double_word_t *address, memory_order order) noexcept
        -> native_double_word_t;

    /**
     * \lang english
     * @brief Atomically stores a double-word value with specified memory order.
     *
     * @param address The address to store to
     * @param value The value to store
     * @param order The memory ordering for the operation
     *
     * \lang simp-chinese
     * @brief 使用指定的内存顺序原子存储双字值。
     *
     * @param address 要存储的地址
     * @param value 要存储的值
     * @param order 操作的内存顺序
     */
    RAINY_TOOLKIT_API rain_fn atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value,
                                                       memory_order order) noexcept -> void;

    /**
     * \lang english
     * @brief Indicates whether double-word operations are always lock-free.
     *
     * This constant is true on x86/x64 platforms with appropriate instruction support,
     * false on platforms where double-word operations may require a mutex fallback.
     *
     * \lang simp-chinese
     * @brief 指示双字操作是否始终是无锁的。
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

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Thread operation result codes.
     *
     * This enumeration unifies error states across different platforms.
     *
     * \lang simp-chinese
     * @brief 线程操作返回值。
     *
     * 该枚举用于统一不同平台的错误状态。
     */
    enum class thrd_result : int {
        /**
         * \lang english
         * @brief Operation succeeded
         *
         * The operation completed successfully.
         *
         * \lang simp-chinese
         * @brief 操作成功
         *
         * 操作成功完成。
         */
        success,

        /**
         * \lang english
         * @brief Insufficient memory or invalid argument
         *
         * Typically indicates that memory allocation failed or an invalid parameter was provided.
         * Usually accompanied by errno = EINVAL.
         *
         * \lang simp-chinese
         * @brief 资源不足或参数非法
         *
         * 通常表示内存分配失败或提供了非法参数，一般伴随 errno = EINVAL。
         */
        nomem,

        /**
         * \lang english
         * @brief Operation timed out
         *
         * The operation did not complete within the expected time limit.
         *
         * \lang simp-chinese
         * @brief 超时
         *
         * 操作未在预期时间内完成。
         */
        timed_out,

        /**
         * \lang english
         * @brief Resource is busy
         *
         * The required resource is temporarily unavailable or locked.
         *
         * \lang simp-chinese
         * @brief 资源当前不可用
         *
         * 所需资源暂时不可用或被锁定。
         */
        busy,

        /**
         * \lang english
         * @brief Unclassified error
         *
         * An unspecified error occurred that does not fit other categories.
         *
         * \lang simp-chinese
         * @brief 未分类错误
         *
         * 发生了不属于其他类别的未指定错误。
         */
        error
    };
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Scheduled thread handle.
     *
     * handle:
     *      Platform-specific thread object handle (an abstract representation of pthread_t / HANDLE, etc.)
     *
     * tid:
     *      System thread ID, used for debugging or identification purposes.
     *
     * This structure is a lightweight value type and does not own the thread lifetime.
     *
     * \lang simp-chinese
     * @brief 调度线程句柄
     *
     * handle:
     *      平台相关线程对象句柄（pthread_t / HANDLE 等的抽象表示）
     *
     * tid:
     *      系统线程 ID，用于调试或标识用途。
     *
     * 该结构为轻量值类型，不拥有线程生命周期。
     */
    struct schd_thread_t {
        std::uintptr_t handle;
        std::uint64_t tid;
    };
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Ends the current thread (no return value), equivalent to calling the platform thread exit function.
     * @attention Code after the call stack will not be executed.
     *
     * \lang simp-chinese
     * @brief 结束当前线程（无返回值版本）等价于调用平台线程退出函数。
     * @attention 不会执行调用栈之后的代码。
     */
    RAINY_TOOLKIT_API void endthread();

    /**
     * \lang english
     * @brief Ends the current thread and sets a return code.
     * @param return_code Thread exit code
     *
     * \lang simp-chinese
     * @brief 结束当前线程并设置返回码
     * @param return_code 线程退出码
     */
    RAINY_TOOLKIT_API void endthreadex(unsigned int return_code);

    /**
     * \lang english
     * @brief Creates a thread.
     *
     * @param invoke_function_addr Thread entry function
     * @param stack_size           Stack size (0 means system default)
     * @param arg_list             Argument passed to the thread entry
     *
     * @return Handle of the newly created thread
     *
     * If creation fails, the returned handle may be 0.
     *
     * \lang simp-chinese
     * @brief 创建线程
     *
     * @param invoke_function_addr 线程入口函数
     * @param stack_size           栈大小（0 表示使用系统默认）
     * @param arg_list             传递给线程入口的参数
     *
     * @return 新创建线程的句柄
     *
     * 若创建失败，返回的 handle 可能为 0。
     */
    RAINY_NODISCARD RAINY_TOOLKIT_API schd_thread_t create_thread(unsigned int (*invoke_function_addr)(void *),
                                                                  unsigned int stack_size, void *arg_list);
    
    /**
     * \lang english
     * @brief Waits for a thread to finish.
     *
     * @param thread_handle Target thread
     * @param result_receiver Receives the thread return value; may be nullptr
     *
     * @return
     *  - success     Wait succeeded
     *  - busy        Thread not joinable
     *  - error       Wait failed
     *
     * After a successful join, thread resources are reclaimed.
     *
     * \lang simp-chinese
     * @brief 等待线程结束
     *
     * @param thread_handle 目标线程
     * @param result_receiver 接收线程返回值，可为 nullptr
     *
     * @return
     *  - success     成功等待
     *  - busy        不可等待
     *  - error       等待失败
     *
     * 调用成功后线程资源被回收。
     */
    RAINY_TOOLKIT_API thrd_result thread_join(schd_thread_t thread_handle, std::int64_t *result_receiver) noexcept;

    /**
     * \lang english
     * @brief Detaches a thread.
     *
     * After the thread ends, resources are reclaimed automatically by the system.
     * After detaching, the thread cannot be joined.
     *
     * \lang simp-chinese
     * @brief 分离线程
     *
     * 线程结束后由系统自动回收资源。
     * 调用后不可再 join。
     */
    RAINY_TOOLKIT_API thrd_result thread_detach(schd_thread_t thread_handle) noexcept;

    /**
     * \lang english
     * @brief Returns the number of hardware concurrency available on the system.
     *
     * @remark
     * The return value is the number of logical CPU cores reported by the system.
     * This value is for reference only.
     *
     * \lang simp-chinese
     * @brief 返回系统可用硬件并发数
     *
     * @remark
     * 返回值为系统报告的 CPU 逻辑核心数量。
     * 该值仅供参考。
     */
    RAINY_TOOLKIT_API unsigned int thread_hardware_concurrency() noexcept;

    /**
     * \lang english
     * @brief Gets the current thread ID.
     *
     * \lang simp-chinese
     * @brief 获取当前线程 ID
     */
    RAINY_TOOLKIT_API std::uint64_t get_thread_id() noexcept;

    /**
     * \lang english
     * @brief Actively yields the CPU time slice.
     *
     * \lang simp-chinese
     * @brief 主动让出 CPU 时间片
     */
    RAINY_TOOLKIT_API void thread_yield() noexcept;

    /**
     * \lang english
     * @brief Sleeps the current thread for the specified number of milliseconds.
     * @remark Precision depends on the operating system scheduler.
     *
     * \lang simp-chinese
     * @brief 当前线程休眠指定毫秒
     * @remark 精度取决于操作系统调度器。
     */
    RAINY_TOOLKIT_API void thread_sleep_for(unsigned long ms) noexcept;

    /**
     * \lang english
     * @brief Suspends a thread.
     *
     * @attention May cause deadlock risks; only use in controlled environments.
     *
     * \lang simp-chinese
     * @brief 挂起线程
     *
     * @attention 可能导致死锁风险，仅用于受控环境。
     */
    RAINY_TOOLKIT_API void suspend_thread(schd_thread_t thread_handle) noexcept;

    /**
     * \lang english
     * @brief Resumes a suspended thread.
     *
     * \lang simp-chinese
     * @brief 恢复被挂起线程
     */
    RAINY_TOOLKIT_API void resume_thread(schd_thread_t thread_handle) noexcept;
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Mutex type flags.
     *
     * Can be combined bitwise:
     *
     * plain_mtx      Plain mutex
     * try_mtx        Supports trylock
     * timed_mtx      Supports timedlock
     * recursive_mtx  Recursive lock
     *
     * \lang simp-chinese
     * @brief mutex 类型标志
     *
     * 可按位组合：
     *
     * plain_mtx      普通互斥锁
     * try_mtx        支持 trylock
     * timed_mtx      支持 timedlock
     * recursive_mtx  递归锁
     */
    struct mutex_types {
        static constexpr int plain_mtx = 0x01;
        static constexpr int try_mtx = 0x02;
        static constexpr int timed_mtx = 0x04;
        static constexpr int recursive_mtx = 0x100;
    };

    using mtx_t = void *;

    /**
     * \lang english
     * @brief Concrete implementation function for locking a mutex. This part should not be called by developers; it is an internal implementation.
     * @brief The behavior depends on the type the mutex lock handle points to. The type is selected from mutex_types.
     * @brief
     * If the type is plain_mtx, it is used as a plain lock. With try_mtx it can be used for try_lock. If it is timed_mtx, it can be used for timedlock, while recursive_mtx supports recursion.
     * @param mtx Handle of the mutex lock
     * @param target Timeout duration; nullptr means lock indefinitely
     * @return Returns success if the lock was acquired; otherwise returns busy or timed_out as appropriate
     * @return However, if mtx is null, errno is set to EINVAL and nomem is returned
     *
     * \lang simp-chinese
     * @brief 锁定互斥锁的具体实现函数。此部分不应当由开发者调用，作为内部实现
     * @brief 将根据mutex锁的handle指向的类型决定。类型是从mutex_types中选定的
     * @brief
     * 如果类型为plain_mtx，将用于普通锁。带有try_mtx可以用于try_lock。如果是timed_mtx，可以用于timedlock，而recursive_mtx将支持递归
     * @param mtx mutex锁的handle
     * @param target 超时时间，如果为nullptr表示无限期锁定
     * @return 如果成功获取锁，则返回success，否则根据情况返回busy或timed_out
     * @return 但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept;

    /**
     * \lang english
     * @brief Initializes a mutex lock.
     * @attention This function requires a valid instance handle pointing to an implements::mutex_handle instance. If necessary,
     * @attention consider calling mtx_create, which automatically creates the instance and calls this initializer. Only use mtx_destroy when it is no longer needed.
     * @param mtx Handle of the mutex lock
     * @param flags Selected from mutex_types, indicating the type of this mutex lock
     * @return Generally returns success to indicate success. However, if mtx is null, errno is set to EINVAL and nomem is returned
     *
     * \lang simp-chinese
     * @brief 初始化mutex锁
     * @attention 此函数需要一个有效的实例句柄。这个句柄指向了implements::mutex_handle实例。如果有必要
     * @attention 考虑调用mtx_create，它将为你自动创建实例并调用此初始化。只需要在不需要它的时候，使用mtx_destory即可
     * @param mtx mutex锁的handle
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_init(mtx_t *const mtx, int flags) noexcept;

    /**
     * \lang english
     * @brief Creates a mutex lock.
     * @attention This function overwrites the mtx handle and allocates memory from dynamic storage. It also automatically calls mtx_init for initialization.
     * @attention Therefore, if it is not destroyed with mtx_destroy, it may cause a memory leak.
     * @param mtx Handle of the mutex lock, used to receive the new instance
     * @param flags Selected from mutex_types, indicating the type of this mutex lock
     * @return Generally returns success to indicate success. However, if there is no handle to receive it, errno is set to EINVAL and nomem is returned
     *
     * \lang simp-chinese
     * @brief 创建mutex锁
     * @attention 此函数将会覆写mtx句柄，并且将会从动态存储空间获取一段内存用于存储。且会自动调用mtx_init进行自动的初始化
     * @attention 因此，如果它没有被mtx_destory销毁，可能会导致内存泄漏
     * @param mtx mutex锁的handle，用于接收新的实例
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是如果没有句柄用于接收，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_create(mtx_t *const mtx, int flags) noexcept;

    /**
     * \lang english
     * @brief Locks a mutex.
     * @param mtx Handle of the mutex lock, used for the locking operation
     * @return Generally returns success to indicate success. However, if mtx is null, errno is set to EINVAL and nomem is returned
     *
     * \lang simp-chinese
     * @brief 使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_lock(mtx_t *const mtx) noexcept;

    /**
     * \lang english
     * @brief Attempts to lock a mutex.
     * @param mtx Handle of the mutex lock, used for the locking operation
     * @return Generally returns success to indicate success. However, if mtx is null, errno is set to EINVAL and nomem is returned.
     * @return Additionally, if the mutex is already held by another thread, busy will be the return value of this function
     *
     * \lang simp-chinese
     * @brief 尝试使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem。
     * @return 此外，如果互斥锁已被其他线程占用。busy将是此函数的返回值
     */
    RAINY_TOOLKIT_API thrd_result mtx_trylock(mtx_t *const mtx) noexcept;

    /**
     * \lang english
     * @brief Unlocks a mutex; this function releases the mutex.
     * @brief If the current thread holds the lock, it will be unlocked.
     * @param mtx Handle of the mutex lock
     * @return Generally returns success to indicate success. However, if mtx is null, errno is set to EINVAL and nomem is returned
     *
     * \lang simp-chinese
     * @brief 解锁mutex锁该函数释放互斥锁
     * @brief 如果当前线程持有该锁，则将其解锁
     * @param mtx mutex锁的handle
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_unlock(mtx_t *const mtx) noexcept;

    /**
     * \lang english
     * @brief Attempts to lock a mutex with a timeout mechanism.
     * @param mtx Handle of the mutex lock
     * @param xt Timeout parameter, indicating the maximum time for locking
     * @return Generally returns success to indicate success. However, if mtx is null, errno is set to EINVAL and nomem is returned
     * @return If the lock cannot be acquired within the specified time, timed_out is returned
     *
     * \lang simp-chinese
     * @brief 尝试对mutex锁进行锁定，支持超时机制
     * @param mtx mutex锁的handle
     * @param xt 超时参数，表示锁定的最大时间
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     * @return 如果在指定的时间内无法获取锁，则返回timed_out
     */
    RAINY_TOOLKIT_API thrd_result mtx_timedlock(mtx_t *const mtx, const ::timespec *xt) noexcept;

    /**
     * \lang english
     * @brief Checks whether the current thread already holds the specified mutex.
     * @param mtx Handle of the mutex lock
     * @return bool Returns true if the current thread holds the mutex, otherwise returns false
     *
     * \lang simp-chinese
     * @brief 检查当前线程是否已经持有指定的互斥锁
     * @param mtx mutex锁的handle
     * @return bool 如果当前线程持有该互斥锁，返回true，否则返回false
     */
    RAINY_TOOLKIT_API bool mtx_current_owns(mtx_t *const mtx) noexcept;

    /**
     * \lang english
     * @brief Destroys the mutex lock.
     *
     * Releases the system resources associated with the mutex.
     *
     * @param mtx Handle of the mutex lock
     *
     * @return
     *  - success Destroyed successfully
     *  - nomem   mtx is null or invalid (errno = EINVAL)
     *
     * @warning
     *  - If a thread still holds the lock, the behavior is undefined.
     *  - After destroy, mtx is no longer usable and must be re-initialized.
     *
     * \lang simp-chinese
     * @brief 销毁 mutex 锁
     *
     * 释放互斥锁关联的系统资源。
     *
     * @param mtx mutex锁的handle
     *
     * @return
     *  - success 成功销毁
     *  - nomem   mtx 为空或无效（errno = EINVAL）
     *
     * @warning
     *  - 若仍有线程持有该锁，行为未定义。
     *  - destroy 后 mtx 不再可用，必须重新初始化。
     */
    RAINY_TOOLKIT_API thrd_result mtx_destroy(mtx_t *const mtx) noexcept;

    /**
     * \lang english
     * @brief Gets the underlying platform mutex handle.
     *
     * The return type depends on the platform:
     *  - Linux: pthread_mutex_t*
     *  - Windows: CRITICAL_SECTION / HANDLE, etc.
     *
     * Only for low-level integration or debugging purposes.
     *
     * \lang simp-chinese
     * @brief 获取底层平台 mutex 句柄
     *
     * 返回值类型依赖平台：
     *  - Linux: pthread_mutex_t*
     *  - Windows: CRITICAL_SECTION / HANDLE 等
     *
     * 仅用于底层集成或调试用途。
     */
    RAINY_TOOLKIT_API void *native_mtx_handle(mtx_t *const mtx) noexcept;
}

namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Creates a thread-local storage key (TSS Key).
     *
     * Each thread can store an independent pointer value for this key.
     *
     * @return A valid handle; returns an invalid value on failure.
     *
     * \lang simp-chinese
     * @brief 创建线程局部存储键（TSS Key）
     *
     * 每个线程可为该 key 存储独立的指针值。
     *
     * @return 有效 handle，失败时返回无效值。
     */
    RAINY_TOOLKIT_API core::handle tss_create();

    /**
     * \lang english
     * @brief Gets the TSS value of the current thread.
     *
     * Returns nullptr if not set.
     *
     * \lang simp-chinese
     * @brief 获取当前线程的 TSS 值
     *
     * 若未设置返回 nullptr。
     */
    RAINY_TOOLKIT_API void *tss_get(core::handle tss_key);

    /**
     * \lang english
     * @brief Sets the TSS value of the current thread.
     *
     * @return Returns true on success, otherwise false
     *
     * \lang simp-chinese
     * @brief 设置当前线程的 TSS 值
     *
     * @return true 成功，否则失败
     */
    RAINY_TOOLKIT_API bool tss_set(core::handle tss_key, const void *value);

    /**
     * \lang english
     * @brief Deletes the TSS key.
     *
     * After deletion, the associated values on all threads become invalid.
     * It is not responsible for releasing the memory pointed to by value.
     *
     * \lang simp-chinese
     * @brief 删除 TSS 键
     *
     * 删除后所有线程上的关联值失效。
     * 不负责释放 value 指向的内存。
     */
    RAINY_TOOLKIT_API bool tss_delete(core::handle tss_key);
}

namespace rainy::core::layer {
    using cnd_t = void *;

    /**
     * \lang english
     * @brief Initializes a condition variable.
     *
     * @param cnd Handle of the condition variable
     * @return
     *  - success Succeeded
     *  - nomem  cnd is invalid
     *
     * \lang simp-chinese
     * @brief 条件变量初始化
     *
     * @param cnd 条件变量句柄
     * @return
     *  - success 成功
     *  - nomem  cnd 无效
     */
    RAINY_TOOLKIT_API thrd_result cnd_init(cnd_t *const cnd) noexcept;

    /**
     * \lang english
     * @brief Creates a condition variable.
     *
     * @param cnd Handle of the condition variable
     * @return
     *  - success Succeeded
     *  - nomem  Insufficient memory or invalid handle
     *
     * \lang simp-chinese
     * @brief 创建条件变量
     *
     * @param cnd 条件变量句柄
     * @return
     *  - success 成功
     *  - nomem  内存不足或句柄无效
     */
    RAINY_TOOLKIT_API thrd_result cnd_create(cnd_t *cnd) noexcept;

    /**
     * \lang english
     * @brief Waits on a condition variable.
     *
     * Before calling this function, the mutex must be locked.
     * After calling, the mutex is automatically released and reacquired after being woken up.
     *
     * @param cnd Condition variable
     * @param mtx Mutex associated with it
     * @return
     *  - success Succeeded
     *  - nomem  Invalid parameter
     *  - timed_out Timed out
     *
     * \lang simp-chinese
     * @brief 条件变量等待
     *
     * 调用该函数时，必须先锁定 mutex。
     * 调用后 mutex 会被自动释放，等待被唤醒后重新获取 mutex。
     *
     * @param cnd 条件变量
     * @param mtx 与之关联的 mutex
     * @return
     *  - success 成功
     *  - nomem  无效参数
     *  - timed_out 超时
     */
    RAINY_TOOLKIT_API thrd_result cnd_wait(cnd_t *const cnd, mtx_t *const mtx) noexcept;

    /**
     * \lang english
     * @brief Waits on a condition variable with a timeout.
     *
     * @param cnd Condition variable
     * @param mtx Mutex
     * @param timeout Absolute time (timespec)
     * @return
     *  - success Succeeded
     *  - timed_out Timed out
     *  - nomem Invalid parameter
     *
     * \lang simp-chinese
     * @brief 条件变量等待，带超时
     *
     * @param cnd 条件变量
     * @param mtx mutex
     * @param timeout 绝对时间（timespec）
     * @return
     *  - success 成功
     *  - timed_out 超时
     *  - nomem 无效参数
     */
    RAINY_TOOLKIT_API thrd_result cnd_timedwait(cnd_t *const cnd, mtx_t *const mtx, const ::timespec *timeout) noexcept;

    /**
     * \lang english
     * @brief Wakes up one waiting thread.
     *
     * @param cnd Condition variable
     * @return success / nomem
     *
     * \lang simp-chinese
     * @brief 唤醒一个等待线程
     *
     * @param cnd 条件变量
     * @return success / nomem
     */
    RAINY_TOOLKIT_API thrd_result cnd_signal(cnd_t *const cnd) noexcept;

    /**
     * \lang english
     * @brief Wakes up all waiting threads.
     *
     * @param cnd Condition variable
     * @return success / nomem
     *
     * \lang simp-chinese
     * @brief 唤醒所有等待线程
     *
     * @param cnd 条件变量
     * @return success / nomem
     */
    RAINY_TOOLKIT_API thrd_result cnd_broadcast(cnd_t *const cnd) noexcept;

    /**
     * \lang english
     * @brief Destroys a condition variable.
     *
     * @param cnd Condition variable
     * @return
     *  - success Succeeded
     *  - nomem Invalid handle
     *
     * @warning Destroying a condition variable that still has waiting threads is undefined behavior.
     *
     * \lang simp-chinese
     * @brief 条件变量销毁
     *
     * @param cnd 条件变量
     * @return
     *  - success 成功
     *  - nomem 无效句柄
     *
     * @warning 未释放等待线程的条件变量销毁行为未定义。
     */
    RAINY_TOOLKIT_API thrd_result cnd_destroy(cnd_t *cnd) noexcept;

    /**
     * \lang english
     * @brief Gets the native condition variable handle of the underlying platform.
     *
     * \lang simp-chinese
     * @brief 获取底层平台条件变量原生句柄
     */
    RAINY_TOOLKIT_API void *native_cnd_handle(cnd_t *const cnd) noexcept;
}

namespace rainy::core::layer {
    using smtx_t = void *;

    /**
     * \lang english
     * @brief Initializes a shared_mutex.
     *
     * @attention
     * A valid instance handle is required, which must point to
     * implements::shared_mutex_handle.
     *
     * To automatically allocate an instance, use smtx_create.
     *
     * @param smtx shared_mutex handle
     *
     * @return
     *  - success Initialized successfully
     *  - nomem   smtx is null (errno = EINVAL)
     *
     * \lang simp-chinese
     * @brief 初始化 shared_mutex
     *
     * @attention
     * 需要一个有效的实例句柄，该句柄必须指向
     * implements::shared_mutex_handle。
     *
     * 如需自动分配实例，请使用 smtx_create。
     *
     * @param smtx shared_mutex 句柄
     *
     * @return
     *  - success 初始化成功
     *  - nomem   smtx 为空（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_init(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Creates a shared_mutex.
     *
     * Allocates an instance from dynamic storage and automatically calls smtx_init.
     *
     * @warning
     * If smtx_destroy is not called, a memory leak may occur.
     *
     * @param smtx Handle for receiving the instance
     *
     * @return
     *  - success Created successfully
     *  - nomem   Invalid handle (errno = EINVAL)
     *
     * \lang simp-chinese
     * @brief 创建 shared_mutex
     *
     * 从动态存储空间分配实例，并自动调用 smtx_init。
     *
     * @warning
     * 若未调用 smtx_destroy，可能导致内存泄漏。
     *
     * @param smtx 用于接收实例的句柄
     *
     * @return
     *  - success 创建成功
     *  - nomem   无效句柄（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_create(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Locks the shared_mutex in exclusive mode.
     *
     * Blocks until exclusive access is acquired.
     *
     * The exclusive lock is mutually exclusive with:
     *  - all shared locks
     *  - other exclusive locks
     *
     * @param smtx shared_mutex handle
     *
     * \lang simp-chinese
     * @brief 以独占模式锁定 shared_mutex
     *
     * 阻塞直到获得独占访问权限。
     *
     * 独占锁与：
     *  - 所有 shared 锁互斥
     *  - 其他 exclusive 锁互斥
     *
     * @param smtx shared_mutex 句柄
     */
    RAINY_TOOLKIT_API thrd_result smtx_lock(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Locks the shared_mutex in shared mode.
     *
     * Multiple threads can hold shared locks simultaneously,
     * but they are mutually exclusive with exclusive locks.
     *
     * @param smtx shared_mutex handle
     *
     * \lang simp-chinese
     * @brief 以共享模式锁定 shared_mutex
     *
     * 多个线程可同时持有 shared 锁，
     * 但会与 exclusive 锁互斥。
     *
     * @param smtx shared_mutex 句柄
     */
    RAINY_TOOLKIT_API thrd_result smtx_lock_shared(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Attempts to acquire the exclusive lock.
     *
     * @return
     *  - success Acquired successfully
     *  - busy    Currently cannot be acquired
     *  - nomem   smtx is invalid (errno = EINVAL)
     *
     * \lang simp-chinese
     * @brief 尝试获取独占锁
     *
     * @return
     *  - success 获取成功
     *  - busy    当前无法获取
     *  - nomem   smtx 无效（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_try_lock(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Attempts to acquire the exclusive lock within the specified time.
     *
     * @param smtx shared_mutex handle
     * @param timeout Timeout duration (timespec)
     * @return
     *  - success Acquired successfully
     *  - timed_out Timed out
     *  - nomem   smtx is invalid (errno = EINVAL)
     *
     * \lang simp-chinese
     * @brief 尝试在指定时间内获取独占锁
     *
     * @param smtx shared_mutex 句柄
     * @param timeout 超时时间（timespec）
     * @return
     *  - success 获取成功
     *  - timed_out 超时
     *  - nomem   smtx 无效（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_timed_lock(smtx_t *const smtx, const ::timespec *timeout) noexcept;

    /**
     * \lang english
     * @brief Attempts to acquire the shared lock within the specified time.
     *
     * @param smtx shared_mutex handle
     * @param timeout Timeout duration (timespec)
     * @return
     *  - success Acquired successfully
     *  - timed_out Timed out
     *  - nomem   smtx is invalid (errno = EINVAL)
     *
     * \lang simp-chinese
     * @brief 尝试在指定时间内获取共享锁
     *
     * @param smtx shared_mutex 句柄
     * @param timeout 超时时间（timespec）
     * @return
     *  - success 获取成功
     *  - timed_out 超时
     *  - nomem   smtx 无效（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_timed_lock_shared(smtx_t *const smtx, const ::timespec *timeout) noexcept;

    /**
     * \lang english
     * @brief Attempts to acquire the shared lock.
     *
     * @return
     *  - success Acquired successfully
     *  - busy    Currently cannot be acquired
     *  - nomem   smtx is invalid (errno = EINVAL)
     *
     * \lang simp-chinese
     * @brief 尝试获取共享锁
     *
     * @return
     *  - success 获取成功
     *  - busy    当前无法获取
     *  - nomem   smtx 无效（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_try_lock_shared(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Releases the exclusive lock.
     *
     * The current thread must hold the exclusive lock.
     *
     * \lang simp-chinese
     * @brief 释放独占锁
     *
     * 当前线程必须持有 exclusive 锁。
     */
    RAINY_TOOLKIT_API thrd_result smtx_unlock(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Releases the shared lock.
     *
     * The current thread must hold one of the shared locks.
     *
     * \lang simp-chinese
     * @brief 释放共享锁
     *
     * 当前线程必须持有 shared 锁之一。
     */
    RAINY_TOOLKIT_API thrd_result smtx_unlock_shared(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Destroys the shared_mutex.
     *
     * Releases the associated system resources.
     *
     * @warning
     * If a thread still holds the lock, the behavior is undefined.
     *
     * \lang simp-chinese
     * @brief 销毁 shared_mutex
     *
     * 释放关联的系统资源。
     *
     * @warning
     * 若仍有线程持有锁，行为未定义。
     */
    RAINY_TOOLKIT_API thrd_result smtx_destroy(smtx_t *const smtx) noexcept;

    /**
     * \lang english
     * @brief Gets the underlying platform shared_mutex handle.
     *
     * The return type depends on the platform:
     *  - Windows : SRWLOCK*
     *  - Linux   : pthread_rwlock_t*
     *
     * Only for low-level integration or debugging.
     *
     * \lang simp-chinese
     * @brief 获取底层平台 shared_mutex 句柄
     *
     * 返回类型依赖平台：
     *  - Windows : SRWLOCK*
     *  - Linux   : pthread_rwlock_t*
     *
     * 仅用于底层集成或调试。
     */
    RAINY_TOOLKIT_API void *native_smtx_handle(smtx_t *const smtx) noexcept;
}


namespace rainy::core::layer {
    /**
     * \lang english
     * @brief Queries the total and available physical memory of the system.
     *
     * @param total Receives the total physical memory in bytes
     * @param available Receives the available physical memory in bytes
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 查询系统物理内存的总量与可用量。
     *
     * @param total 接收物理内存总字节数
     * @param available 接收物理内存可用字节数
     * @return 成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn query_memory_capacity(std::size_t *total, std::size_t *available) noexcept -> bool;

    /**
     * \lang english
     * @brief Reads an environment variable into a caller-provided buffer.
     *
     * @param name The environment variable name
     * @param buffer Receives the value followed by a null terminator
     * @param length The buffer capacity in bytes
     * @return true if the variable exists and fits into the buffer
     *
     * \lang simp-chinese
     * @brief 将环境变量读取到调用方提供的缓冲区。
     *
     * @param name 环境变量名
     * @param buffer 接收值与终止空字符
     * @param length 缓冲区容量（字节）
     * @return 变量存在且能放入缓冲区时返回true
     */
    RAINY_TOOLKIT_API rain_fn query_environment(const char *name, char *buffer, std::size_t length) noexcept -> bool;

    /**
     * \lang english
     * @brief Creates or updates an environment variable.
     *
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 创建或更新环境变量。
     *
     * @return 成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn set_environment(const char *name, const char *value) noexcept -> bool;

    /**
     * \lang english
     * @brief Removes an environment variable.
     *
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 移除环境变量。
     *
     * @return 成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn remove_environment(const char *name) noexcept -> bool;

    /**
     * \lang english
     * @brief Queries the login name of the current user.
     *
     * @param buffer Receives the null-terminated login name
     * @param length The buffer capacity in bytes
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 查询当前用户的登录名。
     *
     * @param buffer 接收以空字符结尾的登录名
     * @param length 缓冲区容量（字节）
     * @return 成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn query_user_name(char *buffer, std::size_t length) noexcept -> bool;

    /**
     * \lang english
     * @brief Queries the group id of the current user.
     *
     * @return The group id; 0 when the platform has no group concept
     *
     * \lang simp-chinese
     * @brief 查询当前用户所属组的id。
     *
     * @return 组id；平台无组概念时返回0
     */
    RAINY_TOOLKIT_API rain_fn current_group_id() noexcept -> std::uint32_t;

    /**
     * \lang english
     * @brief Queries the group name for a group id.
     *
     * @param group_id The group id to look up
     * @param buffer Receives the null-terminated group name
     * @param length The buffer capacity in bytes
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 查询组id对应的组名。
     *
     * @param group_id 要查找的组id
     * @param buffer 接收以空字符结尾的组名
     * @param length 缓冲区容量（字节）
     * @return 成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn query_group_name(std::uint32_t group_id, char *buffer, std::size_t length) noexcept -> bool;

    /**
     * \lang english
     * @brief Fills a buffer with cryptographically secure random bytes from the system source.
     *
     * @param buffer The buffer to fill
     * @param length The number of bytes to generate
     * @return true on success
     *
     * \lang simp-chinese
     * @brief 使用系统随机源以加密安全方式填充缓冲区。
     *
     * @param buffer 要填充的缓冲区
     * @param length 要生成的字节数
     * @return 成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn system_random(void *buffer, std::size_t length) noexcept -> bool;

#if RAINY_IS_ARM64
    /**
     * \lang english
     * @brief Reads the ARM64 ELF hardware capability bitmap (AT_HWCAP).
     *
     * @return The HWCAP bitmap; 0 when unavailable
     *
     * \lang simp-chinese
     * @brief 读取ARM64的ELF硬件能力位图（AT_HWCAP）。
     *
     * @return HWCAP位图；不可用时返回0
     */
    RAINY_TOOLKIT_API rain_fn arm64_hwcap() noexcept -> unsigned long;
#endif
}

#undef RAINY_INTERNALLAYER_ENABLE_ENUM_CLASS_BITMASK_OPERATORS

#endif
