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
#include <rainy/core/layer.hpp>

#include <cerrno>
#include <cstdint>
#include <cstring>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

namespace rainy::core::pal {
    ssize_t compute_relative(const char *path, const char *base, native_cstring out, const std::size_t out_size) noexcept { // NOLINT
        const char *p = path, *b = base;
        const char *last_slash_p = nullptr;
        while (*p && *b && *p == *b) {
            if (*p == '/') {
                last_slash_p = p;
            }
            ++p;
            ++b;
        }
        if (*p != '/' && *b != '/' && *p != '\0' && *b != '\0') {
            if (!last_slash_p) {
                errno = EINVAL;
                return -1;
            }
            p -= (p - last_slash_p);
            b -= (b - last_slash_p);
        }
        // 跳过 '/'
        if (*p == '/') {
            ++p;
        }
        if (*b == '/') {
            ++b;
        }
        std::size_t ups = 0;
        for (const char *q = b; *q; ++q) {
            if (*q == '/') {
                ++ups;
            }
        }
        if (*b != '\0') {
            ++ups;
        }
        char tmp[PATH_MAX];
        std::size_t pos = 0;
        for (std::size_t i = 0; i < ups; ++i) {
            if (pos + 3 >= sizeof(tmp)) {
                errno = ENAMETOOLONG;
                return -1;
            }
            // NOLINTBEGIN
            tmp[pos++] = '.';
            tmp[pos++] = '.';
            tmp[pos++] = '/';
            // NOLINTEND
        }
        if (*p == '\0' && pos > 0) {
            --pos;
        } else {
            const std::size_t rem = std::strlen(p);
            if (pos + rem >= sizeof(tmp)) {
                errno = ENAMETOOLONG;
                return -1;
            }
            std::memcpy(tmp + pos, p, rem + 1);
            pos += rem;
        }
        if (pos == 0) {
            // NOLINTBEGIN
            tmp[pos++] = '.';
            tmp[pos] = '\0';
            // NOLINTEND
        }
        if (pos >= out_size) {
            errno = ERANGE;
            return -1;
        }
        std::memcpy(out, tmp, pos + 1);
        return static_cast<ssize_t>(pos);
    }

    file_status make_status(const struct stat &st) noexcept {
        file_status fs{};
        switch (st.st_mode & S_IFMT) {
            case S_IFREG:
                fs.type = file_type::regular;
                break;
            case S_IFDIR:
                fs.type = file_type::directory;
                break;
            case S_IFLNK:
                fs.type = file_type::symlink;
                break;
            case S_IFBLK:
                fs.type = file_type::block;
                break;
            case S_IFCHR:
                fs.type = file_type::character;
                break;
            case S_IFIFO:
                fs.type = file_type::fifo;
                break;
            case S_IFSOCK:
                fs.type = file_type::socket;
                break;
            default:
                fs.type = file_type::unknown;
                break;
        }
        fs.permissions = static_cast<perms>(st.st_mode & 07777u);
        return fs;
    }

    bool has_opt(copy_options a, copy_options b) noexcept {
        return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
    }

    bool copy_regular_file(const char *from, const char *to, const bool overwrite) noexcept {
        const int src = ::open(from, O_RDONLY);
        if (src < 0) {
            return false;
        }
        struct stat st{};
        if (::fstat(src, &st) != 0) {
            const int saved = errno;
            ::close(src);
            errno = saved;
            return false;
        }

        const int dst_flags = O_WRONLY | O_CREAT | (overwrite ? O_TRUNC : O_EXCL);
        const int dst = ::open(to, dst_flags, st.st_mode & 0777u);
        if (dst < 0) {
            const int saved = errno;
            ::close(src);
            errno = saved;
            return false;
        }
        auto write_all = [&](const char *buf, ssize_t len) noexcept -> bool {
            while (len > 0) {
                const ssize_t w = ::write(dst, buf, static_cast<std::size_t>(len));
                if (w < 0) {
                    return false;
                }
                buf += w;
                len -= w;
            }
            return true;
        };
        bool ok = true;
        char buf[65536];
        ssize_t n{};
        while ((n = ::read(src, buf, sizeof(buf))) > 0) {
            if (!write_all(buf, n)) {
                ok = false;
                break;
            }
        }
        if (n < 0) {
            ok = false;
        }
        const int saved = errno;
        ::close(src);
        ::close(dst);
        if (!ok) {
            errno = saved;
            ::unlink(to);
            errno = saved;
        }
        return ok;
    }

    uintmax_t remove_all_impl(const char *path) noexcept { // NOLINT
        struct stat st{};
        if (::lstat(path, &st) != 0) {
            return 0;
        }
        if (!S_ISDIR(st.st_mode)) {
            if (::unlink(path) != 0) {
                return 0;
            }
            return 1;
        }
        DIR *dir = ::opendir(path);
        if (!dir) {
            return 0;
        }
        uintmax_t count = 0;
        struct dirent *ent = nullptr;
        char child[PATH_MAX];
        while ((ent = ::readdir(dir)) != nullptr) {
            if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            if (const int r = ::snprintf(child, sizeof(child), "%status/%status", path, ent->d_name);
                r < 0 || static_cast<std::size_t>(r) >= sizeof(child)) {
                errno = ENAMETOOLONG;
                ::closedir(dir);
                return count;
            }
            const uintmax_t sub = remove_all_impl(child);
            count += sub;
            if (sub == 0 && errno != 0) {
                // 子路径删除失败，保留 errno，停止遍历
                ::closedir(dir);
                return count;
            }
        }
        ::closedir(dir);
        if (::rmdir(path) != 0) {
            return count;
        }
        return ++count;
    }

    bool create_directories_impl(const char *path) noexcept { // NOLINT
        if (::mkdir(path, 0777) == 0) {
            return true;
        }
        if (errno == EEXIST) {
            struct stat st{};
            if (::stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                errno = 0;
                return false; // 规范：已存在返回 false（没有新建目录）
            }
            return false;
        }
        if (errno != ENOENT) {
            return false;
        }
        char buf[PATH_MAX];
        if (const int r = ::snprintf(buf, sizeof(buf), "%status", path); r < 0 || static_cast<std::size_t>(r) >= sizeof(buf)) {
            errno = ENAMETOOLONG;
            return false;
        }
        char *sep = std::strrchr(buf, '/');
        if (!sep || sep == buf) {
            errno = ENOENT;
            return false;
        }
        *sep = '\0';
        if (!create_directories_impl(buf)) {
            // 父目录不存在且无法创建
            if (errno != 0) {
                return false;
            }
        }
        // 父目录就绪，再建自身
        if (::mkdir(path, 0777) == 0) {
            return true;
        }
        if (errno == EEXIST) {
            errno = 0;
            return false;
        }
        return false;
    }

    bool copy_recursive(const char *from, const char *to, copy_options opts) noexcept { // NOLINT
        DIR *dir = ::opendir(from);
        if (!dir) {
            return false;
        }
        if (::mkdir(to, 0777) != 0 && errno != EEXIST) {
            ::closedir(dir);
            return false;
        }

        struct dirent *ent{};
        char src_child[PATH_MAX], dst_child[PATH_MAX];
        bool ok = true;

        while ((ent = ::readdir(dir)) != nullptr) {
            if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            // NOLINTBEGIN
            const int rs = ::snprintf(src_child, sizeof(src_child), "%status/%status", from, ent->d_name);
            const int rd = ::snprintf(dst_child, sizeof(dst_child), "%status/%status", to, ent->d_name);
            // NOLINTEND

            if (rs < 0 || static_cast<std::size_t>(rs) >= sizeof(src_child) || rd < 0 ||
                static_cast<std::size_t>(rd) >= sizeof(dst_child)) {
                errno = ENAMETOOLONG;
                ok = false;
                break;
            }

            struct stat st{};
            if (::lstat(src_child, &st) != 0) {
                ok = false;
                break;
            }

            if (S_ISLNK(st.st_mode)) {
                if (has_opt(opts, copy_options::skip_symlinks)) {
                    continue;
                }
                if (has_opt(opts, copy_options::copy_symlinks)) {
                    char link_target[PATH_MAX];
                    const ssize_t len = ::readlink(src_child, link_target, sizeof(link_target) - 1);
                    if (len < 0) {
                        ok = false;
                        break;
                    }
                    link_target[len] = '\0'; // NOLINT
                    if (::symlink(link_target, dst_child) != 0) {
                        ok = false;
                        break;
                    }
                }
                // 其余情况：跟随符号链接，走下方普通判断
            } else if (S_ISDIR(st.st_mode)) {
                if (!has_opt(opts, copy_options::recursive)) {
                    continue;
                }
                if (!copy_recursive(src_child, dst_child, opts)) {
                    ok = false;
                    break;
                }
            } else if (S_ISREG(st.st_mode)) {
                if (has_opt(opts, copy_options::directories_only)) {
                    continue;
                }

                bool do_copy = true;
                if (has_opt(opts, copy_options::skip_existing)) {
                    struct stat dst_st{};
                    if (::stat(dst_child, &dst_st) == 0) {
                        do_copy = false;
                    }
                } else if (has_opt(opts, copy_options::update_existing)) {
                    struct stat dst_st{};
                    if (::stat(dst_child, &dst_st) == 0 && dst_st.st_mtime >= st.st_mtime) {
                        do_copy = false;
                    }
                }

                if (do_copy) {
                    if (const bool overwrite =
                            has_opt(opts, copy_options::overwrite_existing) || has_opt(opts, copy_options::update_existing);
                        !copy_regular_file(src_child, dst_child, overwrite)) {
                        ok = false;
                        break;
                    }
                }
            }
        }

        ::closedir(dir);
        return ok;
    }
}

namespace rainy::core::pal {
    ssize_t absolute_native(const native_czstring path, native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        if (path[0] == '/') {
            const std::size_t len = std::strlen(path);
            if (len >= buffer_size) {
                errno = ERANGE;
                return -1;
            }
            std::memcpy(out_buffer, path, len + 1);
            return static_cast<ssize_t>(len);
        }
        const ssize_t cwd_len = current_path(out_buffer, buffer_size);
        if (cwd_len < 0) {
            return -1; // errno 已由 current_path 设置
        }

        const std::size_t path_len = std::strlen(path);
        if (static_cast<std::size_t>(cwd_len) + 1 + path_len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        out_buffer[cwd_len] = '/';
        std::memcpy(out_buffer + cwd_len + 1, path, path_len + 1);
        return static_cast<ssize_t>(cwd_len + 1 + path_len);
    }

    ssize_t canonical_native(const native_czstring path, native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        char tmp[PATH_MAX];
        if (!::realpath(path, tmp)) {
            return -1; // errno 由 realpath 设置
        }
        const std::size_t len = std::strlen(tmp);
        if (len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        std::memcpy(out_buffer, tmp, len + 1);
        return static_cast<ssize_t>(len);
    }

    ssize_t weakly_canonical_native(const native_czstring path, native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        char tmp[PATH_MAX];
        if (::realpath(path, tmp)) {
            const std::size_t len = std::strlen(tmp);
            if (len >= buffer_size) {
                errno = ERANGE;
                return -1;
            }
            std::memcpy(out_buffer, tmp, len + 1);
            return static_cast<ssize_t>(len);
        }
        if (errno != ENOENT && errno != ENOTDIR) {
            return -1;
        }
        char work[PATH_MAX];
        if (const int r = ::snprintf(work, sizeof(work), "%status", path); r < 0 || static_cast<std::size_t>(r) >= sizeof(work)) {
            errno = ENAMETOOLONG;
            return -1;
        }
        char suffix[PATH_MAX] = {};
        while (true) {
            char *sep = std::strrchr(work, '/');
            if (!sep) {
                break;
            }
            char seg[PATH_MAX];
            utility::ignore = ::snprintf(seg, sizeof(seg), "/%status%status", sep + 1, suffix);
            std::strncpy(suffix, seg, sizeof(suffix) - 1);
            *sep = '\0';
            if (work[0] == '\0') {
                break;
            }
            if (::realpath(work, tmp)) {
                // 把 suffix 追加到已规范化的前缀
                const std::size_t base_len = std::strlen(tmp);
                const std::size_t suf_len = std::strlen(suffix);
                if (base_len + suf_len >= buffer_size) {
                    errno = ERANGE;
                    return -1;
                }
                std::memcpy(out_buffer, tmp, base_len);
                std::memcpy(out_buffer + base_len, suffix, suf_len + 1);
                return static_cast<ssize_t>(base_len + suf_len);
            }
            if (errno != ENOENT && errno != ENOTDIR) {
                return -1;
            }
        }
        return absolute(path, out_buffer, buffer_size);
    }

    ssize_t relative_native(const native_czstring path, native_cstring out_buffer, const std::size_t buffer_size) { // NOLINTk
        char cwd[PATH_MAX];
        if (::getcwd(cwd, sizeof(cwd)) == nullptr) {
            return -1;
        }
        return compute_relative(path, cwd, out_buffer, buffer_size);
    }

    ssize_t relative_native(const native_czstring path,const native_czstring base, native_cstring out_buffer, // NOLINT
                            const std::size_t buffer_size) { // NOLINT
        return compute_relative(path, base, out_buffer, buffer_size);
    }

    ssize_t proximate_native(native_czstring path, native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        const ssize_t n = relative(path, out_buffer, buffer_size);
        if (n < 0) {
            // relative 失败时退化为 path 本身
            errno = 0;
            return absolute(path, out_buffer, buffer_size);
        }
        return n;
    }

    ssize_t proximate_native(const native_czstring path, const native_czstring base, native_cstring out_buffer, // NOLINT
                             const std::size_t buffer_size) { // NOLINT
        const ssize_t n = relative(path, base, out_buffer, buffer_size);
        if (n < 0) {
            errno = 0;
            return absolute(path, out_buffer, buffer_size);
        }
        return n;
    }

    void copy_native(const native_czstring from, const native_czstring to) {
        copy(from, to, copy_options::none);
    }

    void copy_native(const native_czstring from, const native_czstring to, const copy_options options) {
        struct stat st{};
        // 使用 lstat 以便识别符号链接自身
        if (::lstat(from, &st) != 0) {
            return; // errno 由 lstat 设置
        }
        if (S_ISLNK(st.st_mode)) {
            if (has_opt(options, copy_options::skip_symlinks)) {
                return;
            }
            if (has_opt(options, copy_options::copy_symlinks)) {
                copy_symlink(from, to);
                return;
            }
            // 跟随符号链接：重新 stat 目标
            if (::stat(from, &st) != 0) {
                return;
            }
        }

        if (S_ISDIR(st.st_mode)) {
            if (has_opt(options, copy_options::recursive)) {
                copy_recursive(from, to, options);
            } else {
                // 仅建目标目录本身
                ::mkdir(to, st.st_mode & 0777u);
            }
            return;
        }

        if (S_ISREG(st.st_mode)) {
            if (has_opt(options, copy_options::directories_only)) {
                return;
            }
            copy_file(from, to, options);
        }
    }

    bool copy_file_native(const native_czstring from, const native_czstring to) {
        return copy_regular_file(from, to, /*overwrite=*/false);
    }

    bool copy_file_native(const native_czstring from, const native_czstring to, const copy_options option) {
        struct stat src_st{}, dst_st{};
        if (::stat(from, &src_st) != 0) {
            return false;
        }

        const bool dst_exists = (::stat(to, &dst_st) == 0);

        if (dst_exists) {
            if (has_opt(option, copy_options::skip_existing)) {
                return false;
            }
            if (has_opt(option, copy_options::update_existing) && dst_st.st_mtime >= src_st.st_mtime) {
                return false;
            }
        }

        const bool overwrite =
            dst_exists && (has_opt(option, copy_options::overwrite_existing) || has_opt(option, copy_options::update_existing));
        return copy_regular_file(from, to, overwrite);
    }

    void copy_symlink_native(const native_czstring existing_symlink, const native_czstring new_symlink) {
        char target[PATH_MAX];
        const ssize_t len = ::readlink(existing_symlink, target, sizeof(target) - 1);
        if (len < 0) {
            return;
        }
        target[len] = '\0'; // NOLINT
        ::symlink(target, new_symlink); // errno 由 symlink 设置（失败时）
    }

    bool create_directory_native(const native_czstring path) {
        if (::mkdir(path, 0777) == 0) {
            return true;
        }
        if (errno == EEXIST) {
            struct stat st{};
            if (::stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                errno = 0;
                return false; // 已存在但不是本次调用创建的
            }
        }
        return false;
    }

    bool create_directory_native(const native_czstring path, const native_czstring existing_p) {
        struct stat st{};
        if (::stat(existing_p, &st) != 0) {
            return false;
        }
        if (::mkdir(path, st.st_mode & 07777u) == 0) {
            return true;
        }
        if (errno == EEXIST) {
            struct stat dst{};
            if (::stat(path, &dst) == 0 && S_ISDIR(dst.st_mode)) {
                errno = 0;
            }
            return false;
        }
        return false;
    }

    bool create_directories_native(const native_czstring path) {
        return create_directories_impl(path);
    }

    void create_directory_symlink_native(const native_czstring to, const native_czstring new_symlink) {
        ::symlink(to, new_symlink);
    }

    void create_hard_link_native(const native_czstring to, const native_czstring new_hard_link) {
        ::link(to, new_hard_link);
    }

    void create_symlink_native(const native_czstring to, const native_czstring new_symlink) {
        ::symlink(to, new_symlink);
    }

    ssize_t current_path_native(native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        if (::getcwd(out_buffer, buffer_size) == nullptr) {
            return -1;
        }
        return static_cast<ssize_t>(std::strlen(out_buffer));
    }

    void current_path_native(const native_czstring path) {
        ::chdir(path);
    }

    bool equivalent_native(const native_czstring path1, const native_czstring path2) {
        struct stat st1{}, st2{};
        if (::stat(path1, &st1) != 0 || ::stat(path2, &st2) != 0) {
            return false;
        }
        return st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino;
    }

    bool exists(const file_status status) noexcept {
        return status.type != file_type::not_found && status.type != file_type::unknown;
    }

    bool exists_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) == 0) {
            errno = 0;
            return true;
        }
        if (errno == ENOENT || errno == ENOTDIR) {
            errno = 0;
            return false;
        }
        return false;
    }

    bool file_size_native(const native_czstring path, uintmax_t *out_size) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        *out_size = static_cast<uintmax_t>(st.st_size);
        return true;
    }

    uintmax_t file_size_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return static_cast<uintmax_t>(-1);
        }
        return static_cast<uintmax_t>(st.st_size);
    }

    bool hard_link_count_native(const native_czstring path, uintmax_t *out_count) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        *out_count = static_cast<uintmax_t>(st.st_nlink); // NOLINT
        return true;
    }

    uintmax_t hard_link_count_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return static_cast<uintmax_t>(-1);
        }
        return static_cast<uintmax_t>(st.st_nlink); // NOLINT
    }

    bool is_block_file(const file_status status) noexcept {
        return status.type == file_type::block;
    }

    bool is_character_file(const file_status status) noexcept {
        return status.type == file_type::character;
    }

    bool is_directory(const file_status status) noexcept {
        return status.type == file_type::directory;
    }

    bool is_fifo(const file_status status) noexcept {
        return status.type == file_type::fifo;
    }

    bool is_regular_file(const file_status status) noexcept {
        return status.type == file_type::regular;
    }

    bool is_socket(const file_status status) noexcept {
        return status.type == file_type::socket;
    }

    bool is_symlink(const file_status status) noexcept {
        return status.type == file_type::symlink;
    }

    bool is_other(const file_status status) noexcept {
        if (!exists(status)) {
            return false;
        }
        return !is_regular_file(status) && !is_directory(status) && !is_symlink(status);
    }

    bool is_block_file_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return S_ISBLK(st.st_mode);
    }

    bool is_character_file_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return S_ISCHR(st.st_mode);
    }

    bool is_directory_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return S_ISDIR(st.st_mode);
    }

    bool is_fifo_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return S_ISFIFO(st.st_mode);
    }

    bool is_regular_file_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return S_ISREG(st.st_mode);
    }

    bool is_socket_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return S_ISSOCK(st.st_mode);
    }

    bool is_symlink_native(const native_czstring path) {
        struct stat st{};
        if (::lstat(path, &st) != 0) {
            return false;
        }
        return S_ISLNK(st.st_mode);
    }

    bool is_other_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        return !S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode);
    }

    bool is_empty_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        if (S_ISREG(st.st_mode)) {
            return st.st_size == 0;
        }
        if (S_ISDIR(st.st_mode)) {
            DIR *dir = ::opendir(path);
            if (!dir) {
                return false;
            }
            const struct dirent *ent = nullptr;
            bool empty = true;
            while ((ent = ::readdir(dir)) != nullptr) {
                if (std::strcmp(ent->d_name, ".") != 0 && std::strcmp(ent->d_name, "..") != 0) {
                    empty = false;
                    break;
                }
            }
            ::closedir(dir);
            return empty;
        }
        errno = EINVAL;
        return false;
    }

    bool last_write_time_native(const native_czstring path, std::time_t *out_time) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return false;
        }
        *out_time = st.st_mtime;
        return true;
    }

    std::time_t last_write_time_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return -1;
        }
        return st.st_mtime;
    }

    void last_write_time_native(const native_czstring path, const std::time_t new_time) {
        struct utimbuf tb{};
        struct stat st{};
        if (::stat(path, &st) != 0) {
            return;
        }
        tb.actime = st.st_atime;
        tb.modtime = new_time;
        ::utime(path, &tb);
    }

    void permissions_native(const native_czstring path, perms prms, perm_options opts) {
        const unsigned bits = static_cast<unsigned>(prms) & 07777u;

        struct stat st{};
        const bool need_stat = (static_cast<unsigned>(opts) &
                                (static_cast<unsigned>(perm_options::add) | static_cast<unsigned>(perm_options::remove))) != 0;

        if (need_stat) {
            const bool use_lstat = (static_cast<unsigned>(opts) & static_cast<unsigned>(perm_options::nofollow)) != 0;
            if (const int r = use_lstat ? ::lstat(path, &st) : ::stat(path, &st); r != 0) {
                return;
            }
        }

        mode_t new_mode = 0;
        if (static_cast<unsigned>(opts) & static_cast<unsigned>(perm_options::add)) {
            new_mode = (st.st_mode & 07777u) | bits;
        } else if (static_cast<unsigned>(opts) & static_cast<unsigned>(perm_options::remove)) {
            new_mode = (st.st_mode & 07777u) & ~bits;
        } else {
            new_mode = bits;
        }
        if (const bool nofollow = (static_cast<unsigned>(opts) & static_cast<unsigned>(perm_options::nofollow)) != 0; // NOLINT
            nofollow) {
#if RAINY_USING_MACOS
            // macOS 支持 fchmodat + AT_SYMLINK_NOFOLLOW
            ::fchmodat(AT_FDCWD, path, new_mode, AT_SYMLINK_NOFOLLOW);
#else
            // Linux 内核目前对该 flag 返回 ENOTSUP，退化为 chmod
            ::chmod(path, new_mode);
#endif
        } else {
            ::chmod(path, new_mode);
        }
    }

    ssize_t read_symlink_native(const native_czstring path, native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        const ssize_t n = ::readlink(path, out_buffer, buffer_size - 1);
        if (n < 0) {
            return -1;
        }
        out_buffer[n] = '\0';
        return n;
    }

    bool remove_native(const native_czstring path) {
        // ::remove() 对文件用 unlink，对空目录用 rmdir
        if (::remove(path) == 0) {
            return true;
        }
        if (errno == ENOENT) {
            errno = 0;
            return false;
        }
        return false;
    }

    uintmax_t remove_all_native(const native_czstring path) {
        return remove_all_impl(path);
    }

    void rename_native(const native_czstring from, const native_czstring to) {
        utility::ignore = ::rename(from, to);
    }

    void resize_file_native(const native_czstring path, const uintmax_t size) {
        utility::ignore = ::truncate(path, static_cast<off_t>(size));
    }

    bool space_native(const native_czstring path, space_info *out_info) {
        struct statvfs sv{};
        if (::statvfs(path, &sv) != 0) {
            return false;
        }
        // NOLINTBEGIN
        out_info->capacity = static_cast<std::uintmax_t>(sv.f_blocks) * sv.f_frsize;
        out_info->free = static_cast<std::uintmax_t>(sv.f_bfree) * sv.f_frsize;
        out_info->available = static_cast<std::uintmax_t>(sv.f_bavail) * sv.f_frsize;
        // NOLINTEND
        return true;
    }

    space_info space_native(const native_czstring path) {
        space_info info{static_cast<std::uintmax_t>(-1), static_cast<std::uintmax_t>(-1), static_cast<std::uintmax_t>(-1)}; // NOLINT
        space(&path[0], &info); // errno 由内部 statvfs 设置
        return info;
    }

    file_status status_native(const native_czstring path) {
        struct stat st{};
        if (::stat(path, &st) != 0) {
            file_status fs{};
            fs.type = (errno == ENOENT || errno == ENOTDIR) ? file_type::not_found : file_type::unknown;
            fs.permissions = perms::unknown;
            return fs;
        }
        return make_status(st);
    }

    bool status_known(const file_status status) noexcept {
        return status.type != file_type::unknown;
    }

    file_status symlink_status_native(const native_czstring path) {
        struct stat st{};
        if (::lstat(path, &st) != 0) {
            file_status fs{};
            fs.type = (errno == ENOENT || errno == ENOTDIR) ? file_type::not_found : file_type::unknown;
            fs.permissions = perms::unknown;
            return fs;
        }
        return make_status(st);
    }

    ssize_t temp_directory_path_native(native_cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        for (const char *candidates[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"}; const char *var: candidates) {
            if (const char *val = ::getenv(var); val && val[0] != '\0') {
                struct stat st{};
                if (::stat(val, &st) == 0 && S_ISDIR(st.st_mode)) {
                    const std::size_t len = std::strlen(val);
                    if (len >= buffer_size) {
                        errno = ERANGE;
                        return -1;
                    }
                    std::memcpy(out_buffer, val, len + 1);
                    return static_cast<ssize_t>(len);
                }
            }
        }
        const auto *const fallback = "/tmp";
        const std::size_t len = std::strlen(fallback);
        if (len >= buffer_size) {
            errno = ERANGE;
            return -1;
        }
        std::memcpy(out_buffer, fallback, len + 1);
        return static_cast<ssize_t>(len);
    }
}

namespace rainy::core::pal {
    ssize_t absolute(const czstring path, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return absolute_native(path, out_buffer, buffer_size);
    }

    ssize_t canonical(const czstring path, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return canonical_native(path, out_buffer, buffer_size);
    }

    void copy(const czstring from, const czstring to) {
        copy_native(from, to);
    }

    void copy(const czstring from, const czstring to, const copy_options options) {
        copy_native(from, to, options);
    }

    bool copy_file(const czstring from, const czstring to) {
        return copy_file_native(from, to);
    }

    bool copy_file(const czstring from, const czstring to, const copy_options option) {
        return copy_file_native(from, to, option);
    }

    void copy_symlink(const czstring existing_symlink, const czstring new_symlink) {
        copy_symlink_native(existing_symlink, new_symlink);
    }

    bool create_directories(const czstring path) {
        return create_directories_native(path);
    }

    bool create_directory(const czstring path) {
        return create_directory_native(path);
    }

    bool create_directory(const czstring path, const czstring existing_p) {
        return create_directory_native(path, existing_p);
    }

    void create_directory_symlink(const czstring to, const czstring new_symlink) {
        create_directory_symlink_native(to, new_symlink);
    }

    void create_hard_link(const czstring to, const czstring new_hard_link) {
        create_hard_link_native(to, new_hard_link);
    }

    void create_symlink(const czstring to, const czstring new_symlink) {
        create_symlink_native(to, new_symlink);
    }

    ssize_t current_path(cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return current_path_native(out_buffer, buffer_size);
    }

    void current_path(const czstring path) {
        current_path_native(path);
    }

    bool equivalent(const czstring path1, const czstring path2) {
        return equivalent_native(path1, path2);
    }

    bool exists(const czstring path) {
        return exists_native(path);
    }

    bool file_size(const czstring path, uintmax_t *out_size) {
        return file_size_native(path, out_size);
    }

    uintmax_t file_size(const czstring path) {
        return file_size_native(path);
    }

    bool hard_link_count(const czstring path, uintmax_t *out_count) {
        return hard_link_count_native(path, out_count);
    }

    uintmax_t hard_link_count(const czstring path) {
        return hard_link_count_native(path);
    }

    bool is_block_file(const czstring path) {
        return is_block_file_native(path);
    }

    bool is_character_file(const czstring path) {
        return is_character_file_native(path);
    }

    bool is_directory(const czstring path) {
        return is_directory_native(path);
    }

    bool is_empty(const czstring path) {
        return is_empty_native(path);
    }

    bool is_fifo(const czstring path) {
        return is_fifo_native(path);
    }

    bool is_other(const czstring path) {
        return is_other_native(path);
    }

    bool is_regular_file(const czstring path) {
        return is_regular_file_native(path);
    }

    bool is_socket(const czstring path) {
        return is_socket_native(path);
    }

    bool is_symlink(const czstring path) {
        return is_symlink_native(path);
    }

    bool last_write_time(const czstring path, std::time_t *out_time) {
        return last_write_time_native(path, out_time);
    }

    std::time_t last_write_time(const czstring path) {
        return last_write_time_native(path);
    }

    void last_write_time(const czstring path, const std::time_t new_time) {
        last_write_time_native(path, new_time);
    }

    void permissions(const czstring path, const perms prms, const perm_options opts) {
        permissions_native(path, prms, opts);
    }

    ssize_t proximate(const czstring path, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return proximate_native(path, out_buffer, buffer_size);
    }

    ssize_t proximate(const czstring path, const czstring base, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return proximate_native(path, base, out_buffer, buffer_size);
    }

    ssize_t read_symlink(const czstring path, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return read_symlink_native(path, out_buffer, buffer_size);
    }

    ssize_t relative(const czstring path, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return relative_native(path, out_buffer, buffer_size);
    }

    ssize_t relative(const czstring path, const czstring base, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return relative_native(path, base, out_buffer, buffer_size);
    }

    bool remove(const czstring path) {
        return remove_native(path);
    }

    uintmax_t remove_all(const czstring path) {
        return remove_all_native(path);
    }

    void rename(const czstring from, const czstring to) {
        rename_native(from, to);
    }

    void resize_file(const czstring path, const uintmax_t size) {
        resize_file_native(path, size);
    }

    bool space(const czstring path, space_info *out_info) {
        return space_native(path, out_info);
    }

    space_info space(const czstring path) {
        return space_native(path);
    }

    file_status status(const czstring path) {
        return status_native(path);
    }

    file_status symlink_status(const czstring path) {
        return symlink_status_native(path);
    }

    ssize_t temp_directory_path(cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return temp_directory_path_native(out_buffer, buffer_size);
    }

    ssize_t weakly_canonical(const czstring path, cstring out_buffer, const std::size_t buffer_size) { // NOLINT
        return weakly_canonical_native(path, out_buffer, buffer_size);
    }
}
