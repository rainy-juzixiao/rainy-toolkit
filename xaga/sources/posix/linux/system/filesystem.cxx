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
#include <rainy/linux_api/system/filesystem.hpp>

namespace rainy::linux_api::filesystem {
    linux_file_status get_file_status(core::czstring file_path) noexcept {
        linux_file_status result = {};
        // 默认设置无效属性
        result.attributes = linux_file_attributes::invalid;
        result.reparse_tag = linux_reparse_tag::none;
        // 获取文件信息
        struct stat file_stat;
        if (lstat(file_path, &file_stat) != 0) {
            return result; // 无法获取文件信息，返回默认值
        }
        // 设置文件大小
        result.file_size = static_cast<std::uint64_t>(file_stat.st_size);
        // 设置最后修改时间
        result.last_write_time = file_stat.st_mtime;
        // 设置链接计数
        result.link_count = file_stat.st_nlink;
        // 设置文件属性
        linux_file_attributes attrs = static_cast<linux_file_attributes>(0);
        // 检查文件类型
        if (S_ISDIR(file_stat.st_mode)) {
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::directory));
        } else {
            // 普通文件
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::normal));
        }
        // 检查读写权限
        if ((file_stat.st_mode & S_IWUSR) == 0) {
            // 用户没有写入权限，可以视为只读
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::read_only));
        }
        // 检查隐藏文件（Linux中以.开头的文件被视为隐藏文件）
        const char *filename = strrchr(file_path, '/');
        if (filename) {
            filename++; // 跳过斜杠
        } else {
            filename = file_path; // 没有斜杠，直接使用路径
        }
        if (filename[0] == '.') {
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::hidden));
        }
        // 检查符号链接
        if (S_ISLNK(file_stat.st_mode)) {
            attrs =
                static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::reparse_point));
            result.reparse_tag = linux_reparse_tag::symlink;

            // 对于符号链接，获取目标文件的信息
            struct stat target_stat;
            if (stat(file_path, &target_stat) == 0) {
                // 如果目标是目录，可能是挂载点
                if (S_ISDIR(target_stat.st_mode)) {
                    // 简单判断：如果目标是目录且在不同设备上，可能是挂载点
                    if (target_stat.st_dev != file_stat.st_dev) {
                        result.reparse_tag = linux_reparse_tag::mount_point;
                    }
                }
            }
        }
        // 临时文件检测（Linux没有直接的临时文件标志，这里使用一个简单的启发式方法）
        if (strncmp(file_path, "/tmp/", 5) == 0 || strncmp(file_path, "/var/tmp/", 9) == 0) {
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::temporary));
        }
        // Linux没有直接对应的system、archive、device、sparse_file属性
        // 但我们可以尝试映射一些类似的概念
        // 如果是设备文件
        if (S_ISBLK(file_stat.st_mode) || S_ISCHR(file_stat.st_mode)) {
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::device));
        }
        // 存档属性在Linux中没有直接对应，但可以检查文件是否有执行权限
        if (file_stat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
            attrs = static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::archive));
        }
        // 稀疏文件检测（需要特殊处理）
        if (!S_ISDIR(file_stat.st_mode) && !S_ISLNK(file_stat.st_mode)) {
            // 获取文件分配的块数
            blkcnt_t allocated_blocks = file_stat.st_blocks;
            // 计算文件大小理论上需要的块数（一个块通常是512字节）
            blkcnt_t expected_blocks = (file_stat.st_size + 511) / 512;
            // 如果分配的块数明显少于预期块数，这可能是稀疏文件
            if (allocated_blocks < expected_blocks * 0.9) {
                attrs =
                    static_cast<linux_file_attributes>(static_cast<int>(attrs) | static_cast<int>(linux_file_attributes::sparse_file));
            }
        }
        result.attributes = attrs;
        return result;
    }

    RAINY_TOOLKIT_LINUXAPI linux_filehandle open_file(core::czstring filepath, linux_file_open_mode mode) {
        int flags = 0;
        // 设置文件打开模式
        if (static_cast<unsigned int>(mode) & static_cast<unsigned int>(linux_file_open_mode::read)) {
            flags |= O_RDONLY;
        }
        if (static_cast<unsigned int>(mode) & static_cast<unsigned int>(linux_file_open_mode::write)) {
            flags |= O_WRONLY;
        }
        if (static_cast<unsigned int>(mode) & static_cast<unsigned int>(linux_file_open_mode::append)) {
            flags |= O_APPEND;
        }
        if (static_cast<unsigned int>(mode) & static_cast<unsigned int>(linux_file_open_mode::truncate)) {
            flags |= O_TRUNC;
        }
        if (static_cast<unsigned int>(mode) & static_cast<unsigned int>(linux_file_open_mode::nocreate)) {
            flags |= O_EXCL;
        }
        int fd = open(filepath, flags);
        if (fd == -1) {
            return 0; // 返回无效句柄
        }
        return static_cast<linux_filehandle>(fd);
    }

    bool close_file(linux_filehandle handle) {
        if (handle == 0) {
            errno = EINVAL;
            return false;
        }
        if (close(static_cast<int>(handle)) == -1) {
            return false;
        }
        return true;
    }

    bool file_exists(core::czstring file_path) noexcept {
        return get_file_status(file_path).attributes != linux_file_attributes::invalid;
    }

    linux_file_type get_file_type(core::czstring file_path) noexcept {
        struct stat file_stat;
        // 获取文件状态
        if (stat(file_path, &file_stat) == -1) {
            return linux_file_type::not_found;
        }
        // 判断文件类型
        if (S_ISDIR(file_stat.st_mode)) {
            return linux_file_type::directory; // 目录
        }
        if (S_ISLNK(file_stat.st_mode)) {
            return linux_file_type::symlink; // 符号链接
        }
        if (S_ISREG(file_stat.st_mode)) {
            return linux_file_type::regular; // 常规文件
        }
        if (S_ISBLK(file_stat.st_mode)) {
            return linux_file_type::block; // 块设备
        }
        if (S_ISCHR(file_stat.st_mode)) {
            return linux_file_type::character; // 字符设备
        }
        if (S_ISFIFO(file_stat.st_mode)) {
            return linux_file_type::fifo; // FIFO管道
        }
        if (S_ISSOCK(file_stat.st_mode)) {
            return linux_file_type::socket; // 套接字
        }
        return linux_file_type::unknown; // 未知文件类型
    }
}
