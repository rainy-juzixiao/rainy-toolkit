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

/* 这是一个用C函数库封装的底层调用空间。外部用户不应当使用它。推荐使用foundation提供的模块 */
namespace rainy::core::pal {
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
}

namespace rainy::core::pal {
    /**
     * @brief File type enumeration.
     *        文件类型枚举。
     *
     * Identifies the type of file system entry.
     * 标识文件系统条目的类型。
     */
    enum class file_type {
        /**
         * @brief None or not found
         *        无或未找到
         *
         * Special value indicating that the file type is none or the entry was not found.
         * 指示文件类型为无或未找到条目的特殊值。
         */
        none = -1,
        not_found = -1,

        /**
         * @brief Regular file
         *        普通文件
         *
         * Entry is a regular file.
         * 条目为普通文件。
         */
        regular = 0,

        /**
         * @brief Directory
         *        目录
         *
         * Entry is a directory.
         * 条目为目录。
         */
        directory = 1,

        /**
         * @brief Symbolic link
         *        符号链接
         *
         * Entry is a symbolic link.
         * 条目为符号链接。
         */
        symlink = 2,

        /**
         * @brief Block device
         *        块设备
         *
         * Entry represents a block device.
         * 条目表示块设备。
         */
        block = 3,

        /**
         * @brief Character device
         *        字符设备
         *
         * Entry represents a character device.
         * 条目表示字符设备。
         */
        character = 4,

        /**
         * @brief FIFO (named pipe)
         *        FIFO（命名管道）
         *
         * Entry represents a FIFO or named pipe.
         * 条目表示FIFO或命名管道。
         */
        fifo = 5,

        /**
         * @brief Socket
         *        套接字
         *
         * Entry represents a socket.
         * 条目表示套接字。
         */
        socket = 6,

        /**
         * @brief Unknown type
         *        未知类型
         *
         * Entry type is unknown or cannot be determined.
         * 条目类型未知或无法确定。
         */
        unknown = 7
    };

    /**
     * @brief File permissions enumeration.
     *        文件权限枚举。
     *
     * Bitmask flags representing POSIX-style file permissions and special bits.
     * 表示POSIX风格文件权限和特殊位的位掩码标志。
     */
    enum class perms : unsigned {
        /**
         * @brief No permissions
         *        无权限
         *
         * No permission bits set.
         * 未设置任何权限位。
         */
        none = 0,

        // Owner permissions
        /**
         * @brief Owner read permission
         *        所有者读权限
         *
         * Owner can read the file.
         * 所有者可读取文件。
         */
        owner_read = 0400,

        /**
         * @brief Owner write permission
         *        所有者写权限
         *
         * Owner can write to the file.
         * 所有者可写入文件。
         */
        owner_write = 0200,

        /**
         * @brief Owner execute permission
         *        所有者执行权限
         *
         * Owner can execute the file.
         * 所有者可执行文件。
         */
        owner_exec = 0100,

        /**
         * @brief Owner all permissions
         *        所有者全部权限
         *
         * Owner has read, write, and execute permissions.
         * 所有者拥有读、写和执行权限。
         */
        owner_all = 0700,

        // Group permissions
        /**
         * @brief Group read permission
         *        组读权限
         *
         * Group members can read the file.
         * 组成员可读取文件。
         */
        group_read = 040,

        /**
         * @brief Group write permission
         *        组写权限
         *
         * Group members can write to the file.
         * 组成员可写入文件。
         */
        group_write = 020,

        /**
         * @brief Group execute permission
         *        组执行权限
         *
         * Group members can execute the file.
         * 组成员可执行文件。
         */
        group_exec = 010,

        /**
         * @brief Group all permissions
         *        组全部权限
         *
         * Group members have read, write, and execute permissions.
         * 组成员拥有读、写和执行权限。
         */
        group_all = 070,

        // Others permissions
        /**
         * @brief Others read permission
         *        其他用户读权限
         *
         * Others can read the file.
         * 其他用户可读取文件。
         */
        others_read = 04,

        /**
         * @brief Others write permission
         *        其他用户写权限
         *
         * Others can write to the file.
         * 其他用户可写入文件。
         */
        others_write = 02,

        /**
         * @brief Others execute permission
         *        其他用户执行权限
         *
         * Others can execute the file.
         * 其他用户可执行文件。
         */
        others_exec = 01,

        /**
         * @brief Others all permissions
         *        其他用户全部权限
         *
         * Others have read, write, and execute permissions.
         * 其他用户拥有读、写和执行权限。
         */
        others_all = 07,

        /**
         * @brief All permissions
         *        全部权限
         *
         * All users have read, write, and execute permissions.
         * 所有用户拥有读、写和执行权限。
         */
        all = 0777,

        // Special bits
        /**
         * @brief Set user ID bit
         *        设置用户ID位
         *
         * Executable runs with owner's privileges.
         * 可执行文件以所有者权限运行。
         */
        set_uid = 04000,

        /**
         * @brief Set group ID bit
         *        设置组ID位
         *
         * Executable runs with group's privileges.
         * 可执行文件以组权限运行。
         */
        set_gid = 02000,

        /**
         * @brief Sticky bit
         *        粘滞位
         *
         * Prevents deletion of files by non-owners in directories.
         * 防止非所有者删除目录中的文件。
         */
        sticky_bit = 01000,

        /**
         * @brief Permission mask
         *        权限掩码
         *
         * Mask covering all permission and special bits.
         * 覆盖所有权限位和特殊位的掩码。
         */
        mask = 07777,

        /**
         * @brief Unknown permissions
         *        未知权限
         *
         * Special value indicating that permissions are unknown or cannot be retrieved.
         * 指示权限未知或无法获取的特殊值。
         */
        unknown = 0xFFFF
    };

    /**
     * @brief Permission modification options enumeration.
     *        权限修改选项枚举。
     *
     * Flags specifying how permissions should be modified.
     * 指定权限修改方式的标志。
     */
    enum class perm_options : unsigned {
        /**
         * @brief Replace
         *        替换
         *
         * Replace existing permissions with new ones.
         * 用新权限替换现有权限。
         */
        replace = 1,

        /**
         * @brief Add
         *        追加
         *
         * Add specified permission bits to existing ones.
         * 向现有权限追加指定权限位。
         */
        add = 2,

        /**
         * @brief Remove
         *        移除
         *
         * Remove specified permission bits from existing ones.
         * 从现有权限中移除指定权限位。
         */
        remove = 4,

        /**
         * @brief No follow
         *        不跟随
         *
         * Do not follow symbolic links (operate on the link itself).
         * 不跟随符号链接（对链接本身进行操作）。
         */
        nofollow = 8
    };

    /**
     * @brief Copy operation options enumeration.
     *        复制操作选项枚举。
     *
     * Flags controlling the behavior of file and directory copy operations.
     * 控制文件和目录复制操作行为的标志。
     */
    enum class copy_options : unsigned {
        /**
         * @brief No options
         *        无选项
         *
         * Default copy behavior.
         * 默认复制行为。
         */
        none = 0,

        /**
         * @brief Skip existing
         *        跳过已存在
         *
         * Skip copying if the destination already exists.
         * 如果目标已存在则跳过复制。
         */
        skip_existing = 1,

        /**
         * @brief Overwrite existing
         *        覆盖已存在
         *
         * Overwrite the destination if it already exists.
         * 如果目标已存在则覆盖。
         */
        overwrite_existing = 2,

        /**
         * @brief Update existing
         *        更新已存在
         *
         * Overwrite only if the source is newer than the destination.
         * 仅当源文件比目标文件更新时覆盖。
         */
        update_existing = 4,

        /**
         * @brief Recursive
         *        递归
         *
         * Recursively copy directories and their contents.
         * 递归复制目录及其内容。
         */
        recursive = 8,

        /**
         * @brief Copy symlinks
         *        复制符号链接
         *
         * Copy symbolic links as links (not their targets).
         * 将符号链接作为链接复制（而非其目标）。
         */
        copy_symlinks = 16,

        /**
         * @brief Skip symlinks
         *        忽略符号链接
         *
         * Ignore symbolic links during copy.
         * 复制时忽略符号链接。
         */
        skip_symlinks = 32,

        /**
         * @brief Directories only
         *        仅目录
         *
         * Copy only the directory structure (not files).
         * 仅复制目录结构（不复制文件）。
         */
        directories_only = 64,

        /**
         * @brief Create symlinks
         *        创建符号链接
         *
         * Create symbolic links instead of copying files.
         * 创建符号链接而非复制文件。
         */
        create_symlinks = 128,

        /**
         * @brief Create hard links
         *        创建硬链接
         *
         * Create hard links instead of copying files.
         * 创建硬链接而非复制文件。
         */
        create_hard_links = 256
    };

    /**
     * @brief Directory iteration options enumeration.
     *        目录遍历选项枚举。
     *
     * Flags controlling the behavior of directory iteration operations.
     * 控制目录遍历操作行为的标志。
     */
    enum class directory_options : unsigned {
        /**
         * @brief No options
         *        无选项
         *
         * Default directory iteration behavior.
         * 默认目录遍历行为。
         */
        none = 0,

        /**
         * @brief Follow directory symlink
         *        跟随目录符号链接
         *
         * Follow directory symbolic links during iteration.
         * 遍历时跟随目录符号链接。
         */
        follow_directory_symlink = 1,

        /**
         * @brief Skip permission denied
         *        跳过无权限目录
         *
         * Skip directories that cannot be accessed due to permission errors instead of throwing exceptions.
         * 跳过因权限错误无法访问的目录，而不是抛出异常。
         */
        skip_permission_denied = 2
    };

    /**
     * @brief Space information structure.
     *        空间信息结构体。
     *
     * Contains information about available space on a file system.
     * 包含文件系统上可用空间的信息。
     */
    struct space_info {
        /**
         * @brief Total capacity
         *        总容量
         *
         * Total size of the file system in bytes.
         * 文件系统的总大小（以字节为单位）。
         */
        std::uintmax_t capacity;

        /**
         * @brief Free space
         *        空闲空间
         *
         * Total free space on the file system in bytes.
         * 文件系统上的总空闲空间（以字节为单位）。
         */
        std::uintmax_t free;

        /**
         * @brief Available space
         *        可用空间
         *
         * Free space available to non-privileged processes.
         * 非特权进程可用的空闲空间。
         */
        std::uintmax_t available;

        /**
         * @brief Equality operator
         *        相等运算符
         *
         * Compares two space_info structures for equality.
         * 比较两个 space_info 结构体是否相等。
         *
         * @param left Left-hand side space_info object to compare
         *            待比较的左侧 space_info 对象
         * @param right Right-hand side space_info object to compare
         *            待比较的右侧 space_info 对象
         * @return true if capacity, free, and available members are all equal, false otherwise
         *         若 capacity、free 和 available 成员均相等则返回 true，否则返回 false
         */
        friend bool operator==(const space_info &left, const space_info &right) = default;
    };
}

namespace rainy::core::pal {
    /**
     * @brief File status structure.
     *        文件状态结构体。
     *
     * Contains the type and permissions of a file system entry.
     * 包含文件系统条目的类型和权限。
     */
    struct file_status {
        /**
         * @brief File type
         *        文件类型
         *
         * The type of the file system entry.
         * 文件系统条目的类型。
         */
        file_type type;

        /**
         * @brief File permissions
         *        文件权限
         *
         * The permission bits of the file system entry.
         * 文件系统条目的权限位。
         */
        perms permissions;
    };

    /**
     * @brief Get absolute path.
     *        获取绝对路径。
     *
     * Converts the given path to an absolute path.
     * 将给定路径转换为绝对路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Source path string
     *             源路径字符串
     * @param out_buffer Output buffer for the resolved path
     *                   用于存储解析后路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the resolved path on success, -1 on error
     *         成功时返回解析后路径的长度，失败时返回 -1
     */
    ssize_t absolute(czstring path, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Get canonical (absolute and normalized) path.
     *        获取规范路径（绝对且标准化）。
     *
     * Resolves the given path to an absolute, normalized path without symlinks.
     * 将给定路径解析为不含符号链接的绝对、标准化路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Source path string
     *             源路径字符串
     * @param out_buffer Output buffer for the canonical path
     *                   用于存储规范路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the canonical path on success, -1 on error
     *         成功时返回规范路径的长度，失败时返回 -1
     */
    ssize_t canonical(czstring path, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Copy files or directories.
     *        复制文件或目录。
     *
     * Copies a file or directory from source to destination using default options.
     * 使用默认选项将文件或目录从源复制到目标。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param from Source path
     *             源路径
     * @param to Destination path
     *           目标路径
     */
    void copy(czstring from, czstring to);

    /**
     * @brief Copy files or directories with options.
     *        使用选项复制文件或目录。
     *
     * Copies a file or directory from source to destination with specified options.
     * 使用指定选项将文件或目录从源复制到目标。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param from Source path
     *             源路径
     * @param to Destination path
     *           目标路径
     * @param options Copy operation options
     *                复制操作选项
     */
    void copy(czstring from, czstring to, copy_options options);

    /**
     * @brief Copy a single file.
     *        复制单个文件。
     *
     * Copies the contents of one file to another.
     * 将一个文件的内容复制到另一个文件。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param from Source file path
     *             源文件路径
     * @param to Destination file path
     *           目标文件路径
     * @return true on success, false on failure
     *         成功时返回 true，失败时返回 false
     */
    bool copy_file(czstring from, czstring to);

    /**
     * @brief Copy a single file with options.
     *        使用选项复制单个文件。
     *
     * Copies the contents of one file to another with specified options.
     * 使用指定选项将一个文件的内容复制到另一个文件。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param from Source file path
     *             源文件路径
     * @param to Destination file path
     *           目标文件路径
     * @param option Copy operation option
     *               复制操作选项
     * @return true on success, false on failure
     *         成功时返回 true，失败时返回 false
     */
    bool copy_file(czstring from, czstring to, copy_options option);

    /**
     * @brief Copy a symbolic link.
     *        复制符号链接。
     *
     * Copies a symbolic link (creates a new symlink pointing to the same target).
     * 复制符号链接（创建指向相同目标的新符号链接）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param existing_symlink Existing symbolic link path
     *                         现有符号链接路径
     * @param new_symlink Path for the new symbolic link
     *                    新符号链接的路径
     */
    void copy_symlink(czstring existing_symlink, czstring new_symlink);

    /**
     * @brief Create directories for a path.
     *        为路径创建目录。
     *
     * Creates all directories in the given path that do not already exist.
     * 创建给定路径中所有不存在的目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path where directories should be created
     *             需要创建目录的路径
     * @return true if directories were created, false otherwise
     *         如果目录被创建则返回 true，否则返回 false
     */
    bool create_directories(czstring path);

    /**
     * @brief Create a single directory.
     *        创建单个目录。
     *
     * Creates the final directory in the given path.
     * 创建给定路径中的最后一个目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Directory path to create
     *             要创建的目录路径
     * @return true if directory was created, false otherwise
     *         如果目录被创建则返回 true，否则返回 false
     */
    bool create_directory(czstring path);

    /**
     * @brief Create a directory with attributes.
     *        使用属性创建目录。
     *
     * Creates a directory with specified attributes.
     * 使用指定的属性创建目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Directory path to create
     *             要创建的目录路径
     * @param attributes Attribute string (platform-specific)
     *                   属性字符串（平台相关）
     * @return true if directory was created, false otherwise
     *         如果目录被创建则返回 true，否则返回 false
     */
    bool create_directory(czstring path, czstring attributes);

    /**
     * @brief Create a directory symbolic link.
     *        创建目录符号链接。
     *
     * Creates a symbolic link to a directory.
     * 创建指向目录的符号链接。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param to Target directory path
     *           目标目录路径
     * @param new_symlink Path for the new symbolic link
     *                    新符号链接的路径
     */
    void create_directory_symlink(czstring to, czstring new_symlink);

    /**
     * @brief Create a hard link.
     *        创建硬链接。
     *
     * Creates a hard link from the target to the new link path.
     * 从目标创建指向新链接路径的硬链接。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param to Target file path
     *           目标文件路径
     * @param new_hard_link Path for the new hard link
     *                      新硬链接的路径
     */
    void create_hard_link(czstring to, czstring new_hard_link);

    /**
     * @brief Create a symbolic link.
     *        创建符号链接。
     *
     * Creates a symbolic link to the target.
     * 创建指向目标的符号链接。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param to Target path
     *           目标路径
     * @param new_symlink Path for the new symbolic link
     *                    新符号链接的路径
     */
    void create_symlink(czstring to, czstring new_symlink);

    /**
     * @brief Get current working directory.
     *        获取当前工作目录。
     *
     * Writes the current working directory path to the output buffer.
     * 将当前工作目录路径写入输出缓冲区。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param out_buffer Output buffer for the current directory path
     *                   用于存储当前目录路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the current path on success, -1 on error
     *         成功时返回当前路径的长度，失败时返回 -1
     */
    ssize_t current_path(cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Set current working directory.
     *        设置当前工作目录。
     *
     * Changes the current working directory to the specified path.
     * 将当前工作目录更改为指定路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path New current working directory path
     *             新的当前工作目录路径
     */
    void current_path(czstring path);

    /**
     * @brief Check if two paths refer to the same file system entry.
     *        检查两个路径是否指向相同的文件系统条目。
     *
     * Determines whether the two paths resolve to the same entity.
     * 确定两个路径是否解析为相同的实体。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path1 First path
     *              第一个路径
     * @param path2 Second path
     *              第二个路径
     * @return true if both paths refer to the same file, false otherwise
     *         如果两个路径指向相同文件则返回 true，否则返回 false
     */
    bool equivalent(czstring path1, czstring path2);

    /**
     * @brief Check if file status indicates existence.
     *        检查文件状态是否表示存在。
     *
     * Determines if the file status indicates an existing file system entry.
     * 确定文件状态是否表示存在的文件系统条目。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry exists, false otherwise
     *         如果条目存在则返回 true，否则返回 false
     */
    bool exists(file_status status) noexcept;

    /**
     * @brief Check if a file exists.
     *        检查文件是否存在。
     *
     * Determines if the given path refers to an existing file system entry.
     * 确定给定路径是否指向存在的文件系统条目。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry exists, false otherwise
     *         如果条目存在则返回 true，否则返回 false
     */
    bool exists(czstring path);

    /**
     * @brief Get file size (output parameter version).
     *        获取文件大小（输出参数版本）。
     *
     * Retrieves the size of a file in bytes.
     * 获取文件的大小（以字节为单位）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @param out_size Pointer to store the file size
     *                 用于存储文件大小的指针
     * @return true on success, false on failure
     *         成功时返回 true，失败时返回 false
     */
    bool file_size(czstring path, uintmax_t *out_size);

    /**
     * @brief Get file size (return value version).
     *        获取文件大小（返回值版本）。
     *
     * Retrieves the size of a file in bytes.
     * 获取文件的大小（以字节为单位）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @return File size in bytes on success, 0 on error
     *         成功时返回文件大小（以字节为单位），失败时返回 0
     */
    uintmax_t file_size(czstring path);

    /**
     * @brief Get hard link count (output parameter version).
     *        获取硬链接计数（输出参数版本）。
     *
     * Retrieves the number of hard links referring to the file.
     * 获取指向文件的硬链接数量。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @param out_count Pointer to store the hard link count
     *                  用于存储硬链接计数的指针
     * @return true on success, false on failure
     *         成功时返回 true，失败时返回 false
     */
    bool hard_link_count(czstring path, uintmax_t *out_count);

    /**
     * @brief Get hard link count (return value version).
     *        获取硬链接计数（返回值版本）。
     *
     * Retrieves the number of hard links referring to the file.
     * 获取指向文件的硬链接数量。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @return Hard link count on success, 0 on error
     *         成功时返回硬链接计数，失败时返回 0
     */
    uintmax_t hard_link_count(czstring path);

    /**
     * @brief Check if file status indicates a block device.
     *        检查文件状态是否表示块设备。
     *
     * Determines if the file status corresponds to a block device.
     * 确定文件状态是否对应于块设备。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a block device, false otherwise
     *         如果条目是块设备则返回 true，否则返回 false
     */
    bool is_block_file(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a block device.
     *        检查路径是否指向块设备。
     *
     * Determines if the given path refers to a block device.
     * 确定给定路径是否指向块设备。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a block device, false otherwise
     *         如果条目是块设备则返回 true，否则返回 false
     */
    bool is_block_file(czstring path);

    /**
     * @brief Check if file status indicates a character device.
     *        检查文件状态是否表示字符设备。
     *
     * Determines if the file status corresponds to a character device.
     * 确定文件状态是否对应于字符设备。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a character device, false otherwise
     *         如果条目是字符设备则返回 true，否则返回 false
     */
    bool is_character_file(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a character device.
     *        检查路径是否指向字符设备。
     *
     * Determines if the given path refers to a character device.
     * 确定给定路径是否指向字符设备。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a character device, false otherwise
     *         如果条目是字符设备则返回 true，否则返回 false
     */
    bool is_character_file(czstring path);

    /**
     * @brief Check if file status indicates a directory.
     *        检查文件状态是否表示目录。
     *
     * Determines if the file status corresponds to a directory.
     * 确定文件状态是否对应于目录。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a directory, false otherwise
     *         如果条目是目录则返回 true，否则返回 false
     */
    bool is_directory(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a directory.
     *        检查路径是否指向目录。
     *
     * Determines if the given path refers to a directory.
     * 确定给定路径是否指向目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a directory, false otherwise
     *         如果条目是目录则返回 true，否则返回 false
     */
    bool is_directory(czstring path);

    /**
     * @brief Check if a directory or file is empty.
     *        检查目录或文件是否为空。
     *
     * Determines if the given path refers to an empty file or directory.
     * 确定给定路径是否指向空文件或空目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is empty, false otherwise
     *         如果条目为空则返回 true，否则返回 false
     */
    bool is_empty(czstring path);

    /**
     * @brief Check if file status indicates a FIFO (named pipe).
     *        检查文件状态是否表示FIFO（命名管道）。
     *
     * Determines if the file status corresponds to a FIFO.
     * 确定文件状态是否对应于FIFO。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a FIFO, false otherwise
     *         如果条目是FIFO则返回 true，否则返回 false
     */
    bool is_fifo(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a FIFO (named pipe).
     *        检查路径是否指向FIFO（命名管道）。
     *
     * Determines if the given path refers to a FIFO.
     * 确定给定路径是否指向FIFO。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a FIFO, false otherwise
     *         如果条目是FIFO则返回 true，否则返回 false
     */
    bool is_fifo(czstring path);

    /**
     * @brief Check if file status indicates "other" type.
     *        检查文件状态是否表示“其他”类型。
     *
     * Determines if the file status corresponds to a type that is not regular file, directory, or symlink.
     * 确定文件状态是否对应于非常规文件、目录或符号链接的类型。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is of other type, false otherwise
     *         如果条目是其他类型则返回 true，否则返回 false
     */
    bool is_other(file_status status) noexcept;

    /**
     * @brief Check if a path refers to an "other" file type.
     *        检查路径是否指向“其他”文件类型。
     *
     * Determines if the given path refers to a type that is not regular file, directory, or symlink.
     * 确定给定路径是否指向非常规文件、目录或符号链接的类型。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is of other type, false otherwise
     *         如果条目是其他类型则返回 true，否则返回 false
     */
    bool is_other(czstring path);

    /**
     * @brief Check if file status indicates a regular file.
     *        检查文件状态是否表示普通文件。
     *
     * Determines if the file status corresponds to a regular file.
     * 确定文件状态是否对应于普通文件。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a regular file, false otherwise
     *         如果条目是普通文件则返回 true，否则返回 false
     */
    bool is_regular_file(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a regular file.
     *        检查路径是否指向普通文件。
     *
     * Determines if the given path refers to a regular file.
     * 确定给定路径是否指向普通文件。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a regular file, false otherwise
     *         如果条目是普通文件则返回 true，否则返回 false
     */
    bool is_regular_file(czstring path);

    /**
     * @brief Check if file status indicates a socket.
     *        检查文件状态是否表示套接字。
     *
     * Determines if the file status corresponds to a socket.
     * 确定文件状态是否对应于套接字。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a socket, false otherwise
     *         如果条目是套接字则返回 true，否则返回 false
     */
    bool is_socket(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a socket.
     *        检查路径是否指向套接字。
     *
     * Determines if the given path refers to a socket.
     * 确定给定路径是否指向套接字。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a socket, false otherwise
     *         如果条目是套接字则返回 true，否则返回 false
     */
    bool is_socket(czstring path);

    /**
     * @brief Check if file status indicates a symbolic link.
     *        检查文件状态是否表示符号链接。
     *
     * Determines if the file status corresponds to a symbolic link.
     * 确定文件状态是否对应于符号链接。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the entry is a symbolic link, false otherwise
     *         如果条目是符号链接则返回 true，否则返回 false
     */
    bool is_symlink(file_status status) noexcept;

    /**
     * @brief Check if a path refers to a symbolic link.
     *        检查路径是否指向符号链接。
     *
     * Determines if the given path refers to a symbolic link.
     * 确定给定路径是否指向符号链接。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to check
     *             要检查的路径
     * @return true if the entry is a symbolic link, false otherwise
     *         如果条目是符号链接则返回 true，否则返回 false
     */
    bool is_symlink(czstring path);

    /**
     * @brief Get last write time (output parameter version).
     *        获取最后写入时间（输出参数版本）。
     *
     * Retrieves the last modification time of a file.
     * 获取文件的最后修改时间。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @param out_time Pointer to store the last write time
     *                 用于存储最后写入时间的指针
     * @return true on success, false on failure
     *         成功时返回 true，失败时返回 false
     */
    bool last_write_time(czstring path, std::time_t *out_time);

    /**
     * @brief Get last write time (return value version).
     *        获取最后写入时间（返回值版本）。
     *
     * Retrieves the last modification time of a file.
     * 获取文件的最后修改时间。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @return Last write time on success, -1 on error
     *         成功时返回最后写入时间，失败时返回 -1
     */
    std::time_t last_write_time(czstring path);

    /**
     * @brief Set last write time.
     *        设置最后写入时间。
     *
     * Changes the last modification time of a file.
     * 更改文件的最后修改时间。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @param new_time New last write time
     *                 新的最后写入时间
     */
    void last_write_time(czstring path, std::time_t new_time);

    /**
     * @brief Change file permissions.
     *        更改文件权限。
     *
     * Modifies the permissions of a file system entry.
     * 修改文件系统条目的权限。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to the file
     *             文件路径
     * @param prms Permissions to apply
     *             要应用的权限
     * @param opts Permission modification options (default: replace)
     *             权限修改选项（默认：替换）
     */
    void permissions(czstring path, perms prms, perm_options opts = perm_options::replace);

    /**
     * @brief Get proximate path (relative form).
     *        获取近似路径（相对形式）。
     *
     * Converts the given path to a relative path against the current directory.
     * 将给定路径转换为相对于当前目录的相对路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to convert
     *             要转换的路径
     * @param out_buffer Output buffer for the proximate path
     *                   用于存储近似路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the proximate path on success, -1 on error
     *         成功时返回近似路径的长度，失败时返回 -1
     */
    ssize_t proximate(czstring path, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Get proximate path relative to base.
     *        获取相对于基路径的近似路径。
     *
     * Converts the given path to a relative path against the specified base path.
     * 将给定路径转换为相对于指定基路径的相对路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to convert
     *             要转换的路径
     * @param base Base path for relative conversion
     *             用于相对转换的基路径
     * @param out_buffer Output buffer for the proximate path
     *                   用于存储近似路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the proximate path on success, -1 on error
     *         成功时返回近似路径的长度，失败时返回 -1
     */
    ssize_t proximate(czstring path, czstring base, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Read the target of a symbolic link.
     *        读取符号链接的目标。
     *
     * Reads the target path of a symbolic link.
     * 读取符号链接的目标路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Symbolic link path
     *             符号链接路径
     * @param out_buffer Output buffer for the target path
     *                   用于存储目标路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the target path on success, -1 on error
     *         成功时返回目标路径的长度，失败时返回 -1
     */
    ssize_t read_symlink(czstring path, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Get relative path.
     *        获取相对路径。
     *
     * Converts the given path to a relative path against the current directory.
     * 将给定路径转换为相对于当前目录的相对路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to convert
     *             要转换的路径
     * @param out_buffer Output buffer for the relative path
     *                   用于存储相对路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the relative path on success, -1 on error
     *         成功时返回相对路径的长度，失败时返回 -1
     */
    ssize_t relative(czstring path, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Get relative path to base.
     *        获取相对于基路径的相对路径。
     *
     * Converts the given path to a relative path against the specified base path.
     * 将给定路径转换为相对于指定基路径的相对路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to convert
     *             要转换的路径
     * @param base Base path for relative conversion
     *             用于相对转换的基路径
     * @param out_buffer Output buffer for the relative path
     *                   用于存储相对路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the relative path on success, -1 on error
     *         成功时返回相对路径的长度，失败时返回 -1
     */
    ssize_t relative(czstring path, czstring base, cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Remove a file or empty directory.
     *        删除文件或空目录。
     *
     * Removes a single file or empty directory.
     * 删除单个文件或空目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to remove
     *             要删除的路径
     * @return true if removed successfully, false otherwise
     *         如果成功删除则返回 true，否则返回 false
     */
    bool remove(czstring path);

    /**
     * @brief Remove a file or directory recursively.
     *        递归删除文件或目录。
     *
     * Removes a file or directory and all its contents recursively.
     * 递归删除文件或目录及其所有内容。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to remove
     *             要删除的路径
     * @return Number of files and directories removed
     *         删除的文件和目录数量
     */
    uintmax_t remove_all(czstring path);

    /**
     * @brief Rename a file or directory.
     *        重命名文件或目录。
     *
     * Renames or moves a file or directory.
     * 重命名或移动文件或目录。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param from Source path
     *             源路径
     * @param to Destination path
     *           目标路径
     */
    void rename(czstring from, czstring to);

    /**
     * @brief Resize a file.
     *        调整文件大小。
     *
     * Changes the size of a file (truncates or extends).
     * 更改文件的大小（截断或扩展）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path File path
     *             文件路径
     * @param size New size in bytes
     *            新大小（以字节为单位）
     */
    void resize_file(czstring path, uintmax_t size);

    /**
     * @brief Get file system space information (output parameter version).
     *        获取文件系统空间信息（输出参数版本）。
     *
     * Retrieves space information for the file system containing the given path.
     * 获取包含给定路径的文件系统的空间信息。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Any path on the target file system
     *             目标文件系统上的任意路径
     * @param out_info Pointer to store space information
     *                 用于存储空间信息的指针
     * @return true on success, false on failure
     *         成功时返回 true，失败时返回 false
     */
    bool space(czstring path, space_info *out_info);

    /**
     * @brief Get file system space information (return value version).
     *        获取文件系统空间信息（返回值版本）。
     *
     * Retrieves space information for the file system containing the given path.
     * 获取包含给定路径的文件系统的空间信息。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Any path on the target file system
     *             目标文件系统上的任意路径
     * @return space_info structure containing capacity, free, and available space
     *         包含总容量、空闲空间和可用空间的 space_info 结构体
     */
    space_info space(czstring path);

    /**
     * @brief Get file status.
     *        获取文件状态。
     *
     * Retrieves the type and permissions of a file system entry (follows symlinks).
     * 获取文件系统条目的类型和权限（跟随符号链接）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to query
     *             要查询的路径
     * @return file_status structure containing type and permissions
     *         包含类型和权限的 file_status 结构体
     */
    file_status status(czstring path);

    /**
     * @brief Check if file status is known.
     *        检查文件状态是否已知。
     *
     * Determines if the file status contains valid information.
     * 确定文件状态是否包含有效信息。
     *
     * @param status File status to check
     *               要检查的文件状态
     * @return true if the status is known (not unknown), false otherwise
     *         如果状态已知则返回 true，否则返回 false
     */
    bool status_known(file_status status) noexcept;

    /**
     * @brief Get symbolic link status.
     *        获取符号链接状态。
     *
     * Retrieves the type and permissions of a symbolic link itself (does not follow symlinks).
     * 获取符号链接本身的类型和权限（不跟随符号链接）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Path to query
     *             要查询的路径
     * @return file_status structure containing type and permissions of the symlink
     *         包含符号链接类型和权限的 file_status 结构体
     */
    file_status symlink_status(czstring path);

    /**
     * @brief Get temporary directory path.
     *        获取临时目录路径。
     *
     * Retrieves the path to the directory for temporary files.
     * 获取临时文件目录的路径。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param out_buffer Output buffer for the temporary directory path
     *                   用于存储临时目录路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the temporary directory path on success, -1 on error
     *         成功时返回临时目录路径的长度，失败时返回 -1
     */
    ssize_t temp_directory_path(cstring out_buffer, std::size_t buffer_size);

    /**
     * @brief Get weakly canonical path.
     *        获取弱规范路径。
     *
     * Converts the given path to a canonical-like form (may not resolve all components).
     * 将给定路径转换为类似规范的形式（可能不会解析所有组件）。
     *
     * @attention Uses global errno to indicate operation results.
     *            使用全局 errno 表示操作结果。
     *
     * @param path Source path string
     *             源路径字符串
     * @param out_buffer Output buffer for the weakly canonical path
     *                   用于存储弱规范路径的输出缓冲区
     * @param buffer_size Size of the output buffer in bytes
     *                    输出缓冲区的大小（以字节为单位）
     * @return Length of the weakly canonical path on success, -1 on error
     *         成功时返回弱规范路径的长度，失败时返回 -1
     */
    ssize_t weakly_canonical(czstring path, cstring out_buffer, std::size_t buffer_size);
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
}

namespace rainy::core::pal {
#if RAINY_USING_64_BIT_PLATFORM
    /**
     * @brief Performs a 128-bit atomic compare-and-exchange operation.
     *        执行128位原子比较并交换操作。
     *
     * @param destination Pointer to the destination 128-bit value
     *                    目标128位值的指针
     * @param exchange_high 64 bits of the exchange value
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
