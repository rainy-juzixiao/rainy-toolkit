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
#include <rainy/winapi/system/filesystem.hpp>

namespace rainy::winapi::filesystem {
    win32_file_status get_file_status(core::czstring file_path) noexcept {
        win32_file_status result = {};
        // 默认设置无效属性
        result.attributes = win32_file_attributes::invalid;
        // 获取文件信息
        WIN32_FILE_ATTRIBUTE_DATA file_attr_data = {};
        if (!GetFileAttributesExA(file_path, GetFileExInfoStandard, &file_attr_data)) {
            return result; // 无法获取文件信息，返回默认值
        }
        // 设置文件大小
        LARGE_INTEGER file_size;
        file_size.LowPart = file_attr_data.nFileSizeLow;
        file_size.HighPart = file_attr_data.nFileSizeHigh;
        result.file_size = file_size.QuadPart;
        // 设置最后修改时间
        LARGE_INTEGER li{};
        li.LowPart = file_attr_data.ftLastWriteTime.dwLowDateTime;
        li.HighPart = file_attr_data.ftLastWriteTime.dwHighDateTime;
        // 将Windows文件时间转换为UNIX时间戳（秒）
        constexpr std::int64_t WINDOWS_TICK = 10000000;
        constexpr std::int64_t SEC_TO_UNIX_EPOCH = 11644473600LL;
        result.last_write_time = static_cast<std::int64_t>(li.QuadPart / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
        // 设置文件属性
        DWORD win_attrs = file_attr_data.dwFileAttributes;
        win32_file_attributes attrs = static_cast<win32_file_attributes>(0);
        if (win_attrs & FILE_ATTRIBUTE_READONLY)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::read_only));
        if (win_attrs & FILE_ATTRIBUTE_HIDDEN)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::hidden));
        if (win_attrs & FILE_ATTRIBUTE_SYSTEM)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::system));
        if (win_attrs & FILE_ATTRIBUTE_DIRECTORY)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::directory));
        if (win_attrs & FILE_ATTRIBUTE_ARCHIVE)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::archive));
        if (win_attrs & FILE_ATTRIBUTE_DEVICE)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::device));
        if (win_attrs & FILE_ATTRIBUTE_NORMAL)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::normal));
        if (win_attrs & FILE_ATTRIBUTE_TEMPORARY)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::temporary));
        if (win_attrs & FILE_ATTRIBUTE_SPARSE_FILE)
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::sparse_file));
        if (win_attrs & FILE_ATTRIBUTE_REPARSE_POINT)
            attrs =
                static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::reparse_point));
        // 如果没有设置任何属性，但文件存在，至少设置normal属性
        if (static_cast<int>(attrs) == 0) {
            attrs = static_cast<win32_file_attributes>(static_cast<int>(attrs) | static_cast<int>(win32_file_attributes::normal));
        }
        result.attributes = attrs;
        // 处理重解析点和链接计数
        if (win_attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
            // 获取重解析点信息
            WIN32_FIND_DATAA find_data = {};
            HANDLE handle = FindFirstFileA(file_path, &find_data);
            if (handle != INVALID_HANDLE_VALUE) {
                DWORD reparse_tag_value = find_data.dwReserved0;
                FindClose(handle);

                if (reparse_tag_value == IO_REPARSE_TAG_MOUNT_POINT)
                    result.reparse_tag = win32_reparse_tag::mount_point;
                else if (reparse_tag_value == IO_REPARSE_TAG_SYMLINK)
                    result.reparse_tag = win32_reparse_tag::symlink;
                else
                    result.reparse_tag = win32_reparse_tag::none;
            }
        } else {
            result.reparse_tag = win32_reparse_tag::none;
        }
        // 获取硬链接计数
        result.link_count = 1; // 默认至少有一个链接
        HANDLE file_handle = CreateFileA(file_path,
                                         FILE_READ_ATTRIBUTES, // 只需要读取属性权限
                                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                                         FILE_FLAG_BACKUP_SEMANTICS, // 允许打开目录
                                         nullptr);

        if (file_handle != INVALID_HANDLE_VALUE) {
            BY_HANDLE_FILE_INFORMATION file_info = {};
            if (GetFileInformationByHandle(file_handle, &file_info)) {
                result.link_count = file_info.nNumberOfLinks;
            }
            CloseHandle(file_handle);
        }
        return result;
    }

    RAINY_TOOLKIT_WINAPI std::uintptr_t open_file(core::czstring filepath, win32_file_open_mode mode) {
        constexpr rainy_let no_state_ = static_cast<unsigned int>(win32_file_open_mode::no_state);
        constexpr rainy_let read_ = static_cast<unsigned int>(win32_file_open_mode::read);
        constexpr rainy_let write_ = static_cast<unsigned int>(win32_file_open_mode::write);
        constexpr rainy_let append_ = static_cast<unsigned int>(win32_file_open_mode::append);
        constexpr rainy_let binary_ = static_cast<unsigned int>(win32_file_open_mode::binary);
        constexpr rainy_let truncate_ = static_cast<unsigned int>(win32_file_open_mode::truncate);
        constexpr rainy_let nocreate_ = static_cast<unsigned int>(win32_file_open_mode::nocreate);
        constexpr rainy_let update_ = static_cast<unsigned int>(win32_file_open_mode::update);
        int mode_ = static_cast<unsigned int>(mode);
        if (mode_ & no_state_ || !filepath) {
            errno = EINVAL;
            return 0;
        }
        unsigned long desired_access{0};
        unsigned long creation_disposition{0};
        unsigned long flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
        if (mode_ & read_) {
            desired_access |= GENERIC_READ;
        }
        if (mode_ & write_) {
            desired_access |= GENERIC_WRITE;
        }
        if (mode_ & update_) {
            desired_access |= GENERIC_READ | GENERIC_WRITE;
        }
        if (mode_ & truncate_) {
            creation_disposition = CREATE_ALWAYS;
        } else if (mode_ & nocreate_) {
            creation_disposition = OPEN_EXISTING;
        } else if (mode_ & append_) {
            creation_disposition = OPEN_ALWAYS;
            desired_access |= FILE_APPEND_DATA;
        } else {
            creation_disposition = OPEN_ALWAYS;
        }
        if (mode_ & binary_) {
            flags_and_attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        }
        HANDLE handle = CreateFileA(filepath, desired_access, 0, nullptr, creation_disposition, flags_and_attributes, nullptr);
        if (handle != INVALID_HANDLE_VALUE) {
            return reinterpret_cast<std::uintptr_t>(handle);
        }
        switch (GetLastError()) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                errno = ENOENT;
                break;
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
                errno = EACCES;
                break;
            case ERROR_FILE_EXISTS:
            case ERROR_ALREADY_EXISTS:
                errno = EEXIST;
                break;
            case ERROR_INVALID_HANDLE:
                errno = EBADF;
                break;
            case ERROR_NOT_SAME_DEVICE:
                errno = EXDEV;
                break;
            case ERROR_FILENAME_EXCED_RANGE:
                errno = ENAMETOOLONG;
                break;
            case ERROR_DIRECTORY:
                errno = EISDIR;
                break;
        }
        return 0;
    }

    bool close_file(win32_filehandle handle) {
        if (!handle) {
            errno = EINVAL;
            return false;
        }
        if (!CloseHandle(reinterpret_cast<HANDLE>(handle))) {
            switch (GetLastError()) {
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                    errno = EINVAL;
                    break;
                case ERROR_ACCESS_DENIED:
                case ERROR_SHARING_VIOLATION:
                    errno = EACCES;
                    break;
                case ERROR_NOT_ENOUGH_MEMORY:
                    errno = EAGAIN;
            }
            return false;
        }
        return true;
    }

    win32_file_type get_file_type(core::czstring file_path) noexcept {
        // 获取文件状态
        win32_file_status status = get_file_status(file_path);
        // 如果无法获取文件信息，返回 not_found
        if (status.attributes == win32_file_attributes::invalid) {
            return win32_file_type::not_found;
        }
        // 判断是否是目录
        if (static_cast<int>(status.attributes) & static_cast<int>(win32_file_attributes::directory)) {
            // 检查是否是联接点（Junction）或符号链接（Symlink）
            if (status.reparse_tag == win32_reparse_tag::mount_point) {
                return win32_file_type::junction; // 目录联接点
            }
            if (status.reparse_tag == win32_reparse_tag::symlink) {
                return win32_file_type::symlink; // 符号链接
            }
            return win32_file_type::directory; // 目录
        }
        // 检查是否是符号链接
        if (status.reparse_tag == win32_reparse_tag::symlink) {
            return win32_file_type::symlink; // 符号链接
        }
        // 如果是常规文件
        if (static_cast<int>(status.attributes) & static_cast<int>(win32_file_attributes::normal)) {
            return win32_file_type::regular; // 常规文件
        }
        // 由于Windows不处理块设备、字符设备、FIFO文件、套接字文件，直接返回unknown
        return win32_file_type::unknown;
    }

    bool file_exists(core::czstring file_path) noexcept {
        if (!file_path) {
            return false;
        }
        return get_file_status(file_path).attributes != win32_file_attributes::invalid;
    }
}
