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
#include <filesystem>
#include <rainy/foundation/io/filesystem/operations.hpp>

#define RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(impl, ...)                                                                            \
    {                                                                                                                                 \
        std::error_code ec;                                                                                                           \
        auto result = impl(__VA_ARGS__, ec);                                                                                          \
        if (ec) {                                                                                                                     \
            throw std::system_error(ec);                                                                                              \
        }                                                                                                                             \
        return result;                                                                                                                \
    }

#define RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(impl, ...)                                                                         \
    {                                                                                                                                 \
        std::error_code ec;                                                                                                           \
        auto result = impl(__VA_ARGS__, ec);                                                                                          \
        return result;                                                                                                                \
    }

#define RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(impl, ...)                                                                       \
    {                                                                                                                                 \
        std::error_code ec;                                                                                                           \
        impl(__VA_ARGS__, ec);                                                                                                        \
        if (ec) {                                                                                                                     \
            throw std::system_error(ec);                                                                                              \
        }                                                                                                                             \
    }


#define RAINY_FILESYSTEM_INIT_SYSCALL(ec)                                                                                             \
    do {                                                                                                                              \
        ec.clear();                                                                                                                   \
        errno = 0;                                                                                                                    \
    } while (false)

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

namespace rainy::foundation::io::filesystem {
    constexpr std::size_t buffer_size = 4096;

    path absolute(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(absolute, path);
    }

    path absolute(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path ret;
        if (path.empty()) {
            ec = make_error_code(std::errc::invalid_argument);
            return ret;
        }
        if (path.is_absolute()) {
            ret = path;
            return ret;
        }
#if RAINY_USING_WINDOWS
        foundation::text::wstring_view s = path.native();
        if (path.has_root_directory()) {
            const auto pos = s.find_first_not_of(L"/\\");
            assert(pos != 0);
            s.remove_prefix((core::min) (s.length(), pos) - 1);
        }
        class path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&s, &len, &path](wchar_t *p, unsigned n) -> std::int32_t {
                len = core::pal::absolute_native(s.data(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            ret = utility::move(buf);
        }
#else
        ret = current_path(ec);
        ret /= path;
#endif
        return ret;
    }

    path canonical(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(canonical, path);
    }

    path canonical(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result;
#if RAINY_USING_WINDOWS
        const class path pa = absolute(path.lexically_normal(), ec);
#else
        const class path pa = absolute(path, ec);
#endif
        if (ec) {
            return result;
        }
        text::basic_string_view<core::native_char> s = pa.native();
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&s, &len](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::canonical_native(s.data(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == -1) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    void copy(const path &from, const path &to) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(copy, from, to)
    }

    void copy(const path &from, const path &to, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::copy_native(from.native().c_str(), to.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    void copy(const path &from, const path &to, const copy_options options) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(copy, from, to, options);
    }

    void copy(const path &from, const path &to, const copy_options options, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::copy_native(from.native().c_str(), to.native().c_str(), options);
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    bool copy_file(const path &from, const path &to) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(copy_file, from, to);
    }

    bool copy_file(const path &from, const path &to, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool success = core::pal::copy_file_native(from.native().c_str(), to.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
            return false;
        }
        return success;
    }

    bool copy_file(const path &from, const path &to, const copy_options option) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(copy_file, from, to, option);
    }

    bool copy_file(const path &from, const path &to, const copy_options option, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool success = core::pal::copy_file_native(from.native().c_str(), to.native().c_str(), option);
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
            return false;
        }
        return success;
    }

    void copy_symlink(const path &existing_symlink, const path &new_symlink) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(copy_symlink, existing_symlink, new_symlink);
    }

    void copy_symlink(const path &existing_symlink, const path &new_symlink, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::copy_symlink_native(existing_symlink.native().c_str(), new_symlink.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    bool create_directories(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(create_directories, path);
    }

    bool create_directories(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool success = core::pal::create_directories_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return success;
    }

    bool create_directory(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(create_directory, path);
    }

    bool create_directory(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool success = core::pal::create_directory_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return success;
    }

    bool create_directory(const path &path, const filesystem::path &attributes) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(create_directory, path, attributes);
    }

    bool create_directory(const path &path, const filesystem::path &existing_p, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool success = core::pal::create_directory_native(path.native().c_str(), existing_p.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return success;
    }

    void create_directory_symlink(const path &to, const path &new_symlink) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(create_directory_symlink, to, new_symlink);
    }

    void create_directory_symlink(const path &to, const path &new_symlink, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::create_directory_symlink_native(to.native().c_str(), new_symlink.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    void create_hard_link(const path &to, const path &new_hard_link) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(create_hard_link, to, new_hard_link);
    }

    void create_hard_link(const path &to, const path &new_hard_link, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::create_hard_link_native(to.native().c_str(), new_hard_link.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    void create_symlink(const path &to, const path &new_symlink) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(create_symlink, to, new_symlink);
    }

    void create_symlink(const path &to, const path &new_symlink, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::create_symlink_native(to.native().c_str(), new_symlink.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    path current_path() {
        std::error_code ec;
        auto result = current_path(ec);
        if (ec) {
            throw std::system_error(ec);
        }
        return result;
    }

    path current_path(std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        path result{};

        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::current_path_native(p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    void current_path(const path &path) {
        std::error_code ec;
        current_path(path, ec);
        if (ec) {
            throw std::system_error(ec);
        }
    }

    void current_path(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::current_path_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    bool equivalent(const path &left, const path &right) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(equivalent, left, right);
    }

    bool equivalent(const path &left, const path &right, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool success = core::pal::equivalent_native(left.native().c_str(), right.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return success;
    }

    bool exists(const file_status &status) noexcept {
        return status_known(status) && status.type() != file_type::not_found; // NOLINT
    }

    bool exists(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(exists, path);
    }

    bool exists(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool exsits = core::pal::exists_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
            return false;
        }
        ec.clear();
        return exsits;
    }

    std::uintmax_t file_size(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(file_size, path);
    }

    std::uintmax_t file_size(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto size = core::pal::file_size_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
            return -1;
        }
        return size;
    }

    std::uintmax_t hard_link_count(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(hard_link_count, path);
    }

    std::uintmax_t hard_link_count(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto size = core::pal::file_size_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return size;
    }

    bool is_block_file(const file_status &status) noexcept {
        return status.type() == file_type::not_found;
    }

    bool is_block_file(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_block_file, path);
    }

    bool is_block_file(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_block = core::pal::is_block_file_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_block;
    }

    bool is_character_file(const file_status &status) noexcept {
        return status.type() == file_type::character;
    }

    bool is_character_file(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_character_file, path);
    }

    bool is_character_file(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_character = core::pal::is_character_file_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_character;
    }

    bool is_directory(const file_status &status) noexcept {
        return status.type() == file_type::directory;
    }

    bool is_directory(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_directory, path);
    }

    bool is_directory(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_directory = core::pal::is_directory_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_directory;
    }

    bool is_empty(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_empty, path);
    }

    bool is_empty(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_empty = core::pal::is_empty_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_empty;
    }

    bool is_fifo(const file_status &status) noexcept {
        return status.type() == file_type::fifo;
    }

    bool is_fifo(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_fifo, path);
    }

    bool is_fifo(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_fifo = core::pal::is_fifo_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_fifo;
    }

    bool is_other(file_status status) noexcept {
        return exists(status) && !is_regular_file(status) && !is_directory(status) && !is_symlink(status);
    }

    bool is_other(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_other, path);
    }

    bool is_other(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_fifo = core::pal::is_other_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_fifo;
    }

    bool is_regular_file(const file_status &status) noexcept {
        return status.type() == file_type::regular;
    }

    bool is_regular_file(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_regular_file, path);
    }

    bool is_regular_file(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_regular = core::pal::is_regular_file_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_regular;
    }

    bool is_socket(const file_status &status) noexcept {
        return status.type() == file_type::socket;
    }

    bool is_socket(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_socket, path);
    }

    bool is_socket(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_socket = core::pal::is_socket_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_socket;
    }

    bool is_symlink(const file_status &status) noexcept {
        return status.type() == file_type::symlink;
    }

    bool is_symlink(const path &path) {
        RAINY_FILESYSTEM_NO_EXCEPTION_EDITION_IMPL(is_symlink, path);
    }

    bool is_symlink(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool is_symlink = core::pal::is_symlink_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return is_symlink;
    }

    file_time_type last_write_time(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(last_write_time, path);
    }

    file_time_type last_write_time(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto last_write_time = core::pal::last_write_time_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return file_time_type{file_time_type::duration{last_write_time}};
    }

    void last_write_time(const path &path, const file_time_type new_time) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(last_write_time, path, new_time);
    }

    void last_write_time(const path &path, file_time_type new_time, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::last_write_time_native(path.native().c_str(), new_time.time_since_epoch().count());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    void permissions(const path &path, perms prms, perm_options opts) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(permissions, path, prms, opts);
    }

    void permissions(const path &path, perms prms, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::permissions_native(path.native().c_str(), prms);
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    void permissions(const path &path, perms prms, perm_options opts, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::permissions_native(path.native().c_str(), prms, opts);
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    path proximate(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&path, &len](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::proximate_native(path.native().data(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    path proximate(const path &path, const filesystem::path &base) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(proximate, path, base);
    }

    path proximate(const path &path, const filesystem::path &base, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len, &path](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::proximate_native(path.native().data(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    path read_symlink(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(read_symlink, path);
    }

    path read_symlink(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len, &path](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::read_symlink_native(path.native().c_str(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    path relative(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len, &path](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::relative_native(path.native().c_str(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    path relative(const path &path, const filesystem::path &base) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(relative, path, base);
    }

    path relative(const path &path, const filesystem::path &base, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len, &path, &base](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::relative_native(path.native().c_str(), base.native().c_str(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    bool remove(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(remove, path);
    }

    bool remove(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const bool removed = core::pal::remove_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return removed;
    }

    std::uintmax_t remove_all(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(remove_all, path);
    }

    std::uintmax_t remove_all(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto count = core::pal::remove_all_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return count;
    }

    void rename(const path &from, const path &to) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(rename, from, to);
    }

    void rename(const path &from, const path &to, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::rename_native(from.native().c_str(), to.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    void resize_file(const path &path, std::uintmax_t size) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_VOID_IMPL(resize_file, path, size);
    }

    void resize_file(const path &path, std::uintmax_t size, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        core::pal::resize_file_native(path.native().c_str(), size);
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
    }

    space_info space(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(space, path);
    }

    space_info space(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto [capacity, free, available] = core::pal::space_native(path.native().c_str());
        space_info result = {};
        result.available = available;
        result.capacity = capacity;
        result.free = free;
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
        }
        return result;
    }

    file_status status(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(status, path);
    }

    file_status status(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto [type, permissions] = core::pal::status_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::generic_category());
            file_status s{file_type::none, perms::none};
            return s;
        }
        file_status res{type, permissions};
        return res;
    }

    bool status_known(const file_status &status) noexcept {
        return status.type() != file_type::none;
    }

    file_status symlink_status(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(symlink_status, path);
    }

    file_status symlink_status(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        const auto [type, permissions] = core::pal::symlink_status_native(path.native().c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::system_category());
            return {};
        }
        file_status result{type, permissions};
        return result;
    }

    path temp_directory_path() {
        std::error_code ec;
        auto result = temp_directory_path(ec);
        if (ec) {
            throw std::system_error(ec);
        }
        return result;
    }

    path temp_directory_path(std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::temp_directory_path_native(p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }

    path weakly_canonical(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(weakly_canonical, path);
    }

    path weakly_canonical(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result{};
        path::string_type buf;
        std::int32_t len = 1024;
        do {
            buf.resize_and_overwrite(len, [&len, &path](auto *p, unsigned n) -> std::int32_t {
                len = core::pal::weakly_canonical_native(path.c_str(), p, n);
                return len == -1 ? 0 : len;
            });
        } while (len > buf.size() && len != -1);
        if (len == 0) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        return result;
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)
