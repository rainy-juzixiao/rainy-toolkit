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
#define _CRT_SECURE_NO_WARNINGS // 禁用Secure CRT警告

#include <rainy/core/layer.hpp>
#include <rainy/core/implements/text/string.hpp>
#include <rainy/core/implements/text/wstring_convert.hpp>

#include <cerrno>
#include <cstdint>
#include <cstring>

#define NOMINMAX
#include <windows.h>
#include <fileapi.h>
#include <winioctl.h>
#include <Shlwapi.h>
#include <AclAPI.h>
#include <sddl.h>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable: 6262)
#endif

namespace rainy::core::pal {
    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer
    typedef struct _REPARSE_DATA_BUFFER { 
        ULONG ReparseTag;
        USHORT ReparseDataLength;
        USHORT Reserved;
        union {
            struct {
                USHORT SubstituteNameOffset;
                USHORT SubstituteNameLength;
                USHORT PrintNameOffset;
                USHORT PrintNameLength;
                ULONG Flags;
                WCHAR PathBuffer[1];
            } SymbolicLinkReparseBuffer;
            struct {
                USHORT SubstituteNameOffset;
                USHORT SubstituteNameLength;
                USHORT PrintNameOffset;
                USHORT PrintNameLength;
                WCHAR PathBuffer[1];
            } MountPointReparseBuffer;
            struct {
                UCHAR DataBuffer[1];
            } GenericReparseBuffer;
        } DUMMYUNIONNAME;
    } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

    thread_local foundation::text::string tls_u8_buffer;
    thread_local foundation::text::wstring tls_w_buffer;
    
    using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, foundation::text::basic_string, wchar_t>;

    inline const wchar_t *to_native(czstring path) {
        static thread_local conv_t conv;
        tls_w_buffer = conv.from_bytes(path, path + std::strlen(path));
        return tls_w_buffer.c_str();
    }
    
    inline foundation::text::string from_native(const wchar_t *wpath) {
        static thread_local conv_t conv;
        return conv.to_bytes(wpath, wpath + std::wcslen(wpath));
    }
    
    void set_errno_from_win32(DWORD error) {
        switch (error) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                errno = ENOENT;
                break;
            case ERROR_ACCESS_DENIED:
                errno = EACCES;
                break;
            case ERROR_ALREADY_EXISTS:
                errno = EEXIST;
                break;
            case ERROR_FILENAME_EXCED_RANGE:
                errno = ENAMETOOLONG;
                break;
            case ERROR_DIR_NOT_EMPTY:
                errno = ENOTEMPTY;
                break;
            case ERROR_NOT_SAME_DEVICE:
                errno = EXDEV;
                break;
            case ERROR_FILE_EXISTS:
                errno = EEXIST;
                break;
            default:
                errno = EINVAL;
        }
    }
    
    file_status make_status(const WIN32_FILE_ATTRIBUTE_DATA& attrs) noexcept {
        file_status fs{};
        if (attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            fs.type = file_type::symlink;
        } else if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fs.type = file_type::directory;
        } else {
            fs.type = file_type::regular;
        }
        fs.permissions = static_cast<perms>((attrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? 0444 : 0666);
        return fs;
    }
    
    bool has_opt(copy_options a, copy_options b) noexcept {
        return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
    }
    
    bool copy_regular_file(const wchar_t* from, const wchar_t* to, bool overwrite) noexcept {
        if (!CopyFileW(from, to, !overwrite)) {
            set_errno_from_win32(GetLastError());
            return false;
        }
        return true;
    }
    
    bool create_directories_impl(const wchar_t* path) noexcept {
        if (CreateDirectoryW(path, nullptr)) {
            return true;
        }
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            WIN32_FILE_ATTRIBUTE_DATA attrs{};
            if (GetFileAttributesExW(path, GetFileExInfoStandard, &attrs) &&
                (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                errno = 0;
                return false;
            }
            return false;
        }
        if (error != ERROR_PATH_NOT_FOUND) {
            set_errno_from_win32(error);
            return false;
        }
        
        foundation::text::wstring parent(path);
        std::size_t pos = parent.find_last_of(L"\\/");
        if (pos == foundation::text::wstring::npos || pos == 0) {
            set_errno_from_win32(ERROR_PATH_NOT_FOUND);
            return false;
        }
        parent[pos] = L'\0';
        
        if (!create_directories_impl(parent.c_str())) {
            if (errno != 0) {
                return false;
            }
        }
        
        if (CreateDirectoryW(path, nullptr)) {
            return true;
        }
        error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            errno = 0;
            return false;
        }
        set_errno_from_win32(error);
        return false;
    }
    
    uintmax_t remove_all_impl(const wchar_t* path) noexcept {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return 0;
        }
        
        if (!(attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (DeleteFileW(path)) {
                return 1;
            }
            return 0;
        }
        
        foundation::text::wstring search_path = path;
        search_path += L"\\*";
        WIN32_FIND_DATAW find_data;
        HANDLE hfind = FindFirstFileW(search_path.c_str(), &find_data);
        if (hfind == INVALID_HANDLE_VALUE) {
            return 0;
        }
        
        uintmax_t count = 0;
        do {
            if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0) {
                continue;
            }
            foundation::text::wstring child = path;
            child += L"\\";
            child += find_data.cFileName;
            uintmax_t sub = remove_all_impl(child.c_str());
            count += sub;
            if (sub == 0 && errno != 0) {
                FindClose(hfind);
                return count;
            }
        } while (FindNextFileW(hfind, &find_data));
        
        FindClose(hfind);
        
        if (RemoveDirectoryW(path)) {
            return ++count;
        }
        return count;
    }
}

namespace rainy::core::pal {
    ssize_t absolute_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size) {
        DWORD required_len = GetFullPathNameW(path, 0, nullptr, nullptr);
        if (required_len == 0) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        if (required_len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        wchar_t *full_path = out_buffer;
        DWORD len = GetFullPathNameW(path, buffer_size, full_path, nullptr);
        if (len == 0) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        return static_cast<ssize_t>(len);
    }
    
    ssize_t canonical_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size) {
        wchar_t full_path[MAX_PATH];
        if (!GetFullPathNameW(path, MAX_PATH, full_path, nullptr)) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        
        HANDLE file_handle = CreateFileW(full_path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (file_handle == INVALID_HANDLE_VALUE) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        
        wchar_t canonical[MAX_PATH];
        DWORD len = GetFinalPathNameByHandleW(file_handle, canonical, MAX_PATH, VOLUME_NAME_DOS);
        CloseHandle(file_handle);
        
        if (len == 0 || len >= MAX_PATH) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        
        native_czstring result = canonical;
        if (wcsncmp(canonical, L"\\\\?\\", 4) == 0) {
            result = canonical + 4;
            len -= 4;
        }
        
        if (len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        wcscpy(out_buffer, result);
        return static_cast<ssize_t>(len);
    }
    
    ssize_t weakly_canonical_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size) {
        wchar_t tmp[MAX_PATH];
        if (GetFullPathNameW(path, MAX_PATH, tmp, nullptr)) {
            HANDLE file_handle = CreateFileW(tmp, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                       OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
            if (file_handle != INVALID_HANDLE_VALUE) {
                wchar_t canonical[MAX_PATH];
                DWORD len = GetFinalPathNameByHandleW(file_handle, canonical, MAX_PATH, VOLUME_NAME_DOS);
                CloseHandle(file_handle);
                if (len > 0 && len < MAX_PATH) {
                    native_czstring result = canonical;
                    if (wcsncmp(canonical, L"\\\\?\\", 4) == 0) {
                        result = canonical + 4;
                        len -= 4;
                    }
                    if (len < buffer_size) {
                        wcscpy(out_buffer, result);
                        return static_cast<ssize_t>(len);
                    }
                }
            }
        }
        return absolute_native(path, out_buffer, buffer_size);
    }
    
    ssize_t current_path_native(native_cstring out_buffer, std::size_t buffer_size)  {
        DWORD len = GetCurrentDirectoryW(static_cast<DWORD>(buffer_size), out_buffer);
        if (len == 0) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        if (len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        return static_cast<ssize_t>(len);
    }
    
    void current_path_native(native_czstring path)  {
        if (!SetCurrentDirectoryW(path)) {
            set_errno_from_win32(GetLastError());
        }
    }
    
    bool equivalent_native(native_czstring path1, native_czstring path2)  {
        wchar_t full1[MAX_PATH], full2[MAX_PATH];
        if (!GetFullPathNameW(path1, MAX_PATH, full1, nullptr) ||
            !GetFullPathNameW(path2, MAX_PATH, full2, nullptr)) {
            return false;
        }
        
        BY_HANDLE_FILE_INFORMATION info1{}, info2{};
        HANDLE h1 = CreateFileW(full1, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (h1 == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        HANDLE h2 = CreateFileW(full2, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (h2 == INVALID_HANDLE_VALUE) {
            CloseHandle(h1);
            return false;
        }
        
        bool result = GetFileInformationByHandle(h1, &info1) && 
                      GetFileInformationByHandle(h2, &info2) &&
                      info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
                      info1.nFileIndexHigh == info2.nFileIndexHigh &&
                      info1.nFileIndexLow == info2.nFileIndexLow;
        
        CloseHandle(h1);
        CloseHandle(h2);
        return result;
    }
    
    file_status status_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            file_status fs{};
            DWORD error = GetLastError();
            fs.type = (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) 
                      ? file_type::not_found : file_type::unknown;
            fs.permissions = perms::unknown;
            return fs;
        }
        return make_status(attrs);
    }
    
    file_status symlink_status_native(native_czstring path)  {
        return status_native(path);
    }
    
    bool exists_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return true;
        }
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            errno = 0;
            return false;
        }
        return false;
    }
    
    bool is_regular_file_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return false;
        }
        return !(attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
               !(attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
    }
    
    bool is_directory_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return false;
        }
        return (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    
    bool is_symlink_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return false;
        }
        return (attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }
    
    bool is_other_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return false;
        }
        bool is_reg = !(attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                      !(attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
        bool is_dir = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        bool is_sym = (attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
        return !is_reg && !is_dir && !is_sym;
    }
    
    bool is_empty_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            return false;
        }
        
        if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            foundation::text::wstring search_path = path;
            search_path += L"\\*";
            WIN32_FIND_DATAW find_data;
            HANDLE hfind = FindFirstFileW(search_path.c_str(), &find_data);
            if (hfind == INVALID_HANDLE_VALUE) {
                return false;
            }
            bool empty = true;
            do {
                if (wcscmp(find_data.cFileName, L".") != 0 && wcscmp(find_data.cFileName, L"..") != 0) {
                    empty = false;
                    break;
                }
            } while (FindNextFileW(hfind, &find_data));
            FindClose(hfind);
            return empty;
        }
        
        return ((static_cast<uintmax_t>(attrs.nFileSizeHigh) << 32) | attrs.nFileSizeLow) == 0;
    }
    
    bool file_size_native(native_czstring path, uintmax_t* out_size)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            set_errno_from_win32(GetLastError());
            return false;
        }
        *out_size = (static_cast<uintmax_t>(attrs.nFileSizeHigh) << 32) | attrs.nFileSizeLow;
        return true;
    }
    
    uintmax_t file_size_native(native_czstring path)  {
        uintmax_t size;
        if (!file_size_native(path, &size)) {
            return static_cast<uintmax_t>(-1);
        }
        return size;
    }
    
    bool hard_link_count_native(native_czstring path, uintmax_t* out_count)  {
        HANDLE file_handle = CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (file_handle == INVALID_HANDLE_VALUE) {
            set_errno_from_win32(GetLastError());
            return false;
        }
        
        BY_HANDLE_FILE_INFORMATION info{};
        if (!GetFileInformationByHandle(file_handle, &info)) {
            set_errno_from_win32(GetLastError());
            CloseHandle(file_handle);
            return false;
        }
        
        *out_count = info.nNumberOfLinks;
        CloseHandle(file_handle);
        return true;
    }
    
    uintmax_t hard_link_count_native(native_czstring path)  {
        uintmax_t count;
        if (!hard_link_count_native(path, &count)) {
            return static_cast<uintmax_t>(-1);
        }
        return count;
    }
    
    bool last_write_time_native(native_czstring path, std::time_t* out_time)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            set_errno_from_win32(GetLastError());
            return false;
        }
        
        FILETIME ft = attrs.ftLastWriteTime;
        ULARGE_INTEGER ui;
        ui.LowPart = ft.dwLowDateTime;
        ui.HighPart = ft.dwHighDateTime;
        *out_time = static_cast<std::time_t>((ui.QuadPart - 116444736000000000ULL) / 10000000);
        return true;
    }
    
    std::time_t last_write_time_native(native_czstring path)  {
        std::time_t t;
        if (!last_write_time_native(path, &t)) {
            return -1;
        }
        return t;
    }
    
    void last_write_time_native(native_czstring path, std::time_t new_time)  {
        HANDLE file_handle = CreateFileW(path, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (file_handle == INVALID_HANDLE_VALUE) {
            set_errno_from_win32(GetLastError());
            return;
        }
        
        ULARGE_INTEGER ui;
        ui.QuadPart = static_cast<ULONGLONG>(new_time) * 10000000 + 116444736000000000ULL;
        FILETIME ft;
        ft.dwLowDateTime = ui.LowPart;
        ft.dwHighDateTime = ui.HighPart;
        
        if (!SetFileTime(file_handle, nullptr, nullptr, &ft)) {
            set_errno_from_win32(GetLastError());
        }
        CloseHandle(file_handle);
    }
    
    void permissions_native(native_czstring path, perms prms, perm_options opts)  {
        DWORD attrs = GetFileAttributesW(path);
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            set_errno_from_win32(GetLastError());
            return;
        }
        
        bool readonly = (attrs & FILE_ATTRIBUTE_READONLY) != 0;
        bool want_write = (static_cast<unsigned>(prms) & static_cast<unsigned>(perms::owner_write)) != 0;
        
        if (static_cast<unsigned>(opts) & static_cast<unsigned>(perm_options::add)) {
            if (want_write && readonly) {
                attrs &= ~FILE_ATTRIBUTE_READONLY;
            }
        } else if (static_cast<unsigned>(opts) & static_cast<unsigned>(perm_options::remove)) {
            if (!want_write && !readonly) {
                attrs |= FILE_ATTRIBUTE_READONLY;
            }
        } else {
            if (want_write) {
                attrs &= ~FILE_ATTRIBUTE_READONLY;
            } else {
                attrs |= FILE_ATTRIBUTE_READONLY;
            }
        }
        
        if (!SetFileAttributesW(path, attrs)) {
            set_errno_from_win32(GetLastError());
        }
    }
    
    ssize_t read_symlink_native(native_czstring path, native_cstring out_buffer, std::size_t buffer_size)  {
        HANDLE file_handle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (file_handle == INVALID_HANDLE_VALUE) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        
        char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
        DWORD bytesReturned;
        if (!DeviceIoControl(file_handle, FSCTL_GET_REPARSE_POINT, nullptr, 0,
                             buffer, sizeof(buffer), &bytesReturned, nullptr)) {
            set_errno_from_win32(GetLastError());
            CloseHandle(file_handle);
            return -1;
        }
        
        CloseHandle(file_handle);
        
        REPARSE_DATA_BUFFER* reparse = reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer);
        native_cstring target = nullptr;
        std::size_t len = 0;
        
        if (reparse->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
            target = reparse->SymbolicLinkReparseBuffer.PathBuffer + 
                     (reparse->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t));
            len = reparse->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
        } else if (reparse->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
            target = reparse->MountPointReparseBuffer.PathBuffer + 
                     (reparse->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t));
            len = reparse->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
        } else {
            errno = EINVAL;
            return -1;
        }
        
        if (len >= 4 && wcsncmp(target, L"\\??\\", 4) == 0) {
            target += 4;
            len -= 4;
        }
        
        if (len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        
        wcsncpy(out_buffer, target, len);
        out_buffer[len] = L'\0';
        return static_cast<ssize_t>(len);
    }
    
    bool remove_native(native_czstring path)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(path, GetFileExInfoStandard, &attrs)) {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
                errno = 0;
                return false;
            }
            set_errno_from_win32(error);
            return false;
        }
        
        bool success;
        if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            success = RemoveDirectoryW(path) != 0;
        } else {
            success = DeleteFileW(path) != 0;
        }
        
        if (!success) {
            set_errno_from_win32(GetLastError());
        }
        return success;
    }
    
    uintmax_t remove_all_native(native_czstring path)  {
        return remove_all_impl(path);
    }
    
    void rename_native(native_czstring from, native_czstring to)  {
        if (!MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            set_errno_from_win32(GetLastError());
        }
    }
    
    void resize_file_native(native_czstring path, uintmax_t size)  {
        HANDLE file_handle = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file_handle == INVALID_HANDLE_VALUE) {
            set_errno_from_win32(GetLastError());
            return;
        }
        
        LARGE_INTEGER li;
        li.QuadPart = static_cast<LONGLONG>(size);
        if (!SetFilePointerEx(file_handle, li, nullptr, FILE_BEGIN)) {
            set_errno_from_win32(GetLastError());
            CloseHandle(file_handle);
            return;
        }
        
        if (!SetEndOfFile(file_handle)) {
            set_errno_from_win32(GetLastError());
        }
        CloseHandle(file_handle);
    }
    
    bool space_native(native_czstring path, space_info* out_info)  {
        wchar_t root[MAX_PATH];
        wcscpy(root, path);
        
        if (root[0] == L'\\' && root[1] == L'\\') {
            native_cstring p = root + 2;
            while (*p && *p != L'\\') ++p;
            if (*p) ++p;
            while (*p && *p != L'\\') ++p;
            if (*p) *(p + 1) = L'\0';
        } else if (root[1] == L':') {
            root[2] = L'\\';
            root[3] = L'\0';
        } else {
            root[0] = L'\\';
            root[1] = L'\0';
        }
        
        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
        if (!GetDiskFreeSpaceExW(root, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
            set_errno_from_win32(GetLastError());
            return false;
        }
        
        out_info->available = freeBytesAvailable.QuadPart;
        out_info->capacity = totalBytes.QuadPart;
        out_info->free = totalFreeBytes.QuadPart;
        return true;
    }
    
    space_info space_native(native_czstring path)  {
        space_info info{static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1)};
        space_native(path, &info);
        return info;
    }
    
    bool create_directory_native(native_czstring path)  {
        if (CreateDirectoryW(path, nullptr)) {
            return true;
        }
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            WIN32_FILE_ATTRIBUTE_DATA attrs{};
            if (GetFileAttributesExW(path, GetFileExInfoStandard, &attrs) &&
                (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                errno = 0;
                return false;
            }
        }
        set_errno_from_win32(error);
        return false;
    }
    
    bool create_directory_native(native_czstring path, native_czstring attributes) {
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR *security_desc = nullptr;
        PSECURITY_ATTRIBUTES pSa = nullptr;

        if (attributes && attributes[0] != L'\0') {
            if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                    attributes, SDDL_REVISION_1, reinterpret_cast<PSECURITY_DESCRIPTOR *>(&security_desc), nullptr)) {
                set_errno_from_win32(GetLastError());
                return false;
            }

            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = security_desc;
            sa.bInheritHandle = FALSE;
            pSa = &sa;
        }

        BOOL result = CreateDirectoryW(path, pSa);

        if (security_desc) {
            LocalFree(security_desc);
        }

        if (result) {
            return true;
        }

        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            WIN32_FILE_ATTRIBUTE_DATA attrs{};
            if (GetFileAttributesExW(path, GetFileExInfoStandard, &attrs) && (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                errno = 0;
                return false;
            }
        }
        set_errno_from_win32(error);
        return false;
    }
    
    bool create_directories_native(native_czstring path) {
        return create_directories_impl(path);
    }
    
    void create_hard_link_native(native_czstring to, native_czstring new_hard_link)  {
        if (!CreateHardLinkW(new_hard_link, to, nullptr)) {
            set_errno_from_win32(GetLastError());
        }
    }
    
    void create_symlink_native(native_czstring to, native_czstring new_symlink)  {
        DWORD flags = 0;
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (GetFileAttributesExW(to, GetFileExInfoStandard, &attrs) &&
            (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            flags = SYMBOLIC_LINK_FLAG_DIRECTORY;
        }
        
        if (!CreateSymbolicLinkW(new_symlink, to, flags)) {
            set_errno_from_win32(GetLastError());
        }
    }
    
    void create_directory_symlink_native(native_czstring to, native_czstring new_symlink)  {
        if (!CreateSymbolicLinkW(new_symlink, to, SYMBOLIC_LINK_FLAG_DIRECTORY)) {
            set_errno_from_win32(GetLastError());
        }
    }
    
    void copy_symlink_native(native_czstring existing_symlink, native_czstring new_symlink) {
        wchar_t target[MAX_PATH];
        ssize_t len = read_symlink_native(existing_symlink, target, MAX_PATH);
        if (len < 0) {
            return;
        }
        create_symlink_native(target, new_symlink);
    }
    
    bool copy_file_native(native_czstring from, native_czstring to) {
        return copy_regular_file(from, to, false);
    }
    
    bool copy_file_native(native_czstring from, native_czstring to, copy_options option) {
        WIN32_FILE_ATTRIBUTE_DATA src_attrs{}, dst_attrs{};
        if (!GetFileAttributesExW(from, GetFileExInfoStandard, &src_attrs)) {
            set_errno_from_win32(GetLastError());
            return false;
        }
        
        bool dst_exists = GetFileAttributesExW(to, GetFileExInfoStandard, &dst_attrs);
        
        if (dst_exists) {
            if (has_opt(option, copy_options::skip_existing)) {
                return false;
            }
            if (has_opt(option, copy_options::update_existing)) {
                if (CompareFileTime(&dst_attrs.ftLastWriteTime, &src_attrs.ftLastWriteTime) >= 0) {
                    return false;
                }
            }
        }
        
        bool overwrite = dst_exists && (has_opt(option, copy_options::overwrite_existing) ||
                                        has_opt(option, copy_options::update_existing));
        return copy_regular_file(from, to, overwrite);
    }
    
    void copy_native(native_czstring from, native_czstring to)  {
        copy_native(from, to, copy_options::none);
    }
    
    void copy_native(native_czstring from, native_czstring to, copy_options options)  {
        WIN32_FILE_ATTRIBUTE_DATA attrs{};
        if (!GetFileAttributesExW(from, GetFileExInfoStandard, &attrs)) {
            set_errno_from_win32(GetLastError());
            return;
        }
        
        if (attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            if (has_opt(options, copy_options::skip_symlinks)) {
                return;
            }
            if (has_opt(options, copy_options::copy_symlinks)) {
                copy_symlink_native(from, to);
                return;
            }
            if (!GetFileAttributesExW(from, GetFileExInfoStandard, &attrs)) {
                set_errno_from_win32(GetLastError());
                return;
            }
        }
        
        if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (has_opt(options, copy_options::recursive)) {
                CreateDirectoryW(to, nullptr);
            } else {
                CreateDirectoryW(to, nullptr);
            }
            return;
        }
        
        if (has_opt(options, copy_options::directories_only)) {
            return;
        }
        copy_file_native(from, to, options);
    }
    
    ssize_t temp_directory_path_native(native_cstring out_buffer, std::size_t buffer_size)  {
        DWORD len = GetTempPathW(static_cast<DWORD>(buffer_size), out_buffer);
        if (len == 0) {
            set_errno_from_win32(GetLastError());
            return -1;
        }
        if (len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        return static_cast<ssize_t>(len);
    }
}

namespace rainy::core::pal {
    ssize_t absolute(czstring path, cstring out_buffer, std::size_t buffer_size) {
        const wchar_t* wpath = to_native(path);
        wchar_t wout[MAX_PATH];
        ssize_t result = absolute_native(wpath, wout, MAX_PATH);
        if (result < 0) {
            return -1;
        }
        foundation::text::string out = from_native(wout);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    ssize_t canonical(czstring path, cstring out_buffer, std::size_t buffer_size) {
        const wchar_t* wpath = to_native(path);
        wchar_t wout[MAX_PATH];
        ssize_t result = canonical_native(wpath, wout, MAX_PATH);
        if (result < 0) return -1;
        
        foundation::text::string out = from_native(wout);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    ssize_t weakly_canonical(czstring path, cstring out_buffer, std::size_t buffer_size) {
        const wchar_t* wpath = to_native(path);
        wchar_t wout[MAX_PATH];
        ssize_t result = weakly_canonical_native(wpath, wout, MAX_PATH);
        if (result < 0) return -1;
        
        foundation::text::string out = from_native(wout);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    ssize_t relative(czstring path, cstring out_buffer, std::size_t buffer_size) {
        const wchar_t* wpath = to_native(path);
        wchar_t cwd[MAX_PATH];
        if (current_path_native(cwd, MAX_PATH) < 0) {
            return -1;
        }
        
        wchar_t relative[MAX_PATH];
        if (!PathRelativePathToW(relative, cwd, FILE_ATTRIBUTE_DIRECTORY, wpath, 0)) {
            return absolute(path, out_buffer, buffer_size);
        }
        
        foundation::text::string out = from_native(relative);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    ssize_t relative(czstring path, czstring base, cstring out_buffer, std::size_t buffer_size) {
        const wchar_t* wpath = to_native(path);
        const wchar_t* wbase = to_native(base);
        
        wchar_t relative[MAX_PATH];
        DWORD attrs = GetFileAttributesW(wbase);
        DWORD base_attrs = (attrs & FILE_ATTRIBUTE_DIRECTORY) ? FILE_ATTRIBUTE_DIRECTORY : 0;
        
        if (!PathRelativePathToW(relative, wbase, base_attrs, wpath, 0)) {
            return absolute(path, out_buffer, buffer_size);
        }
        
        foundation::text::string out = from_native(relative);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    ssize_t proximate(czstring path, cstring out_buffer, std::size_t buffer_size) {
        ssize_t n = relative(path, out_buffer, buffer_size);
        if (n < 0) {
            errno = 0;
            return absolute(path, out_buffer, buffer_size);
        }
        return n;
    }
    
    ssize_t proximate(czstring path, czstring base, cstring out_buffer, std::size_t buffer_size) {
        ssize_t n = relative(path, base, out_buffer, buffer_size);
        if (n < 0) {
            errno = 0;
            return absolute(path, out_buffer, buffer_size);
        }
        return n;
    }
    
    ssize_t current_path(cstring out_buffer, std::size_t buffer_size) {
        wchar_t wout[MAX_PATH];
        ssize_t result = current_path_native(wout, MAX_PATH);
        if (result < 0) return -1;
        
        foundation::text::string out = from_native(wout);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    void current_path(czstring path) {
        const wchar_t* wpath = to_native(path);
        current_path_native(wpath);
    }
    
    bool equivalent(czstring path1, czstring path2) {
        const wchar_t* wp1 = to_native(path1);
        const wchar_t* wp2 = to_native(path2);
        return equivalent_native(wp1, wp2);
    }
    
    bool exists(czstring path) {
        const wchar_t* wpath = to_native(path);
        return exists_native(wpath);
    }
    
    bool file_size(czstring path, uintmax_t* out_size) {
        const wchar_t* wpath = to_native(path);
        return file_size_native(wpath, out_size);
    }
    
    uintmax_t file_size(czstring path) {
        const wchar_t* wpath = to_native(path);
        return file_size_native(wpath);
    }
    
    bool hard_link_count(czstring path, uintmax_t* out_count) {
        const wchar_t* wpath = to_native(path);
        return hard_link_count_native(wpath, out_count);
    }
    
    uintmax_t hard_link_count(czstring path) {
        const wchar_t* wpath = to_native(path);
        return hard_link_count_native(wpath);
    }
    
    bool is_block_file(czstring path) {
        utility::ignore = path;
        return false;
    }
    
    bool is_character_file(czstring path) {
        utility::ignore = path;
        return false;
    }
    
    bool is_directory(czstring path) {
        const wchar_t* wpath = to_native(path);
        return is_directory_native(wpath);
    }
    
    bool is_empty(czstring path) {
        const wchar_t* wpath = to_native(path);
        return is_empty_native(wpath);
    }
    
    bool is_fifo(czstring path) {
        utility::ignore = path;
        return false;
    }
    
    bool is_other(czstring path) {
        const wchar_t* wpath = to_native(path);
        return is_other_native(wpath);
    }
    
    bool is_regular_file(czstring path) {
        const wchar_t* wpath = to_native(path);
        return is_regular_file_native(wpath);
    }
    
    bool is_socket(czstring path) {
        utility::ignore = path;
        return false;
    }
    
    bool is_symlink(czstring path) {
        const wchar_t* wpath = to_native(path);
        return is_symlink_native(wpath);
    }
    
    bool last_write_time(czstring path, std::time_t* out_time) {
        const wchar_t* wpath = to_native(path);
        return last_write_time_native(wpath, out_time);
    }
    
    std::time_t last_write_time(czstring path) {
        const wchar_t* wpath = to_native(path);
        return last_write_time_native(wpath);
    }
    
    void last_write_time(czstring path, std::time_t new_time) {
        const wchar_t* wpath = to_native(path);
        last_write_time_native(wpath, new_time);
    }
    
    void permissions(czstring path, perms prms, perm_options opts) {
        const wchar_t* wpath = to_native(path);
        permissions_native(wpath, prms, opts);
    }
    
    ssize_t read_symlink(czstring path, cstring out_buffer, std::size_t buffer_size) {
        const wchar_t* wpath = to_native(path);
        wchar_t wout[MAX_PATH];
        ssize_t result = read_symlink_native(wpath, wout, MAX_PATH);
        if (result < 0) return -1;
        
        foundation::text::string out = from_native(wout);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    bool remove(czstring path) {
        const wchar_t* wpath = to_native(path);
        return remove_native(wpath);
    }
    
    uintmax_t remove_all(czstring path) {
        const wchar_t* wpath = to_native(path);
        return remove_all_native(wpath);
    }
    
    void rename(czstring from, czstring to) {
        const wchar_t* wfrom = to_native(from);
        const wchar_t* wto = to_native(to);
        rename_native(wfrom, wto);
    }
    
    void resize_file(czstring path, uintmax_t size) {
        const wchar_t* wpath = to_native(path);
        resize_file_native(wpath, size);
    }
    
    bool space(czstring path, space_info* out_info) {
        const wchar_t* wpath = to_native(path);
        return space_native(wpath, out_info);
    }
    
    space_info space(czstring path) {
        const wchar_t* wpath = to_native(path);
        return space_native(wpath);
    }
    
    file_status status(czstring path) {
        const wchar_t* wpath = to_native(path);
        return status_native(wpath);
    }
    
    file_status symlink_status(czstring path) {
        const wchar_t* wpath = to_native(path);
        return symlink_status_native(wpath);
    }
    
    ssize_t temp_directory_path(cstring out_buffer, std::size_t buffer_size) {
        wchar_t wout[MAX_PATH];
        ssize_t result = temp_directory_path_native(wout, MAX_PATH);
        if (result < 0) return -1;
        
        foundation::text::string out = from_native(wout);
        if (out.size() >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        memcpy(out_buffer, out.data(), out.size() + 1);
        return static_cast<ssize_t>(out.size());
    }
    
    void copy(czstring from, czstring to) {
        const wchar_t* wfrom = to_native(from);
        const wchar_t* wto = to_native(to);
        copy_native(wfrom, wto);
    }
    
    void copy(czstring from, czstring to, copy_options options) {
        const wchar_t* wfrom = to_native(from);
        const wchar_t* wto = to_native(to);
        copy_native(wfrom, wto, options);
    }
    
    bool copy_file(czstring from, czstring to) {
        const wchar_t* wfrom = to_native(from);
        const wchar_t* wto = to_native(to);
        return copy_file_native(wfrom, wto);
    }
    
    bool copy_file(czstring from, czstring to, copy_options option) {
        const wchar_t* wfrom = to_native(from);
        const wchar_t* wto = to_native(to);
        return copy_file_native(wfrom, wto, option);
    }
    
    void copy_symlink(czstring existing_symlink, czstring new_symlink) {
        const wchar_t* wexisting = to_native(existing_symlink);
        const wchar_t* wnew = to_native(new_symlink);
        copy_symlink_native(wexisting, wnew);
    }
    
    bool create_directory(czstring path) {
        const wchar_t* wpath = to_native(path);
        return create_directory_native(wpath);
    }
    
    bool create_directory(czstring path, czstring attributes) {
        const wchar_t *wpath = to_native(path);
        foundation::text::wstring sddl;
        if (attributes && attributes[0] != '\0') {
            // 解析八进制权限字符串，如 "0755"
            unsigned long mode = std::strtoul(attributes, nullptr, 8);
            // D: 表示DACL
            // (A; ; ; ; ; 表示允许所有人访问)
            wchar_t sddl_buf[256];
            if (mode & 0222) {
                // 可读可写
                swprintf(sddl_buf, sizeof(sddl_buf) / sizeof(wchar_t), L"D:(A;;FA;;;WD)");
            } else {
                swprintf(sddl_buf, sizeof(sddl_buf) / sizeof(wchar_t), L"D:(A;;FR;;;WD)");
            }
            sddl = sddl_buf;
        }
        return create_directory_native(wpath, sddl.empty() ? nullptr : sddl.c_str());
    }
    
    bool create_directories(czstring path) {
        const wchar_t* wpath = to_native(path);
        return create_directories_native(wpath);
    }
    
    void create_directory_symlink(czstring to, czstring new_symlink) {
        const wchar_t* wto = to_native(to);
        const wchar_t* wnew = to_native(new_symlink);
        create_directory_symlink_native(wto, wnew);
    }
    
    void create_hard_link(czstring to, czstring new_hard_link) {
        const wchar_t* wto = to_native(to);
        const wchar_t* wnew = to_native(new_hard_link);
        create_hard_link_native(wto, wnew);
    }
    
    void create_symlink(czstring to, czstring new_symlink) {
        const wchar_t* wto = to_native(to);
        const wchar_t* wnew = to_native(new_symlink);
        create_symlink_native(wto, wnew);
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
