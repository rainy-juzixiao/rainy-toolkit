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
        std::size_t req_len = 128;
        class path::string_type buf;
        int ret_code = -1;
        do {
            buf.resize_and_overwrite(req_len, [&s, &req_len, &ret_code, &path](wchar_t *p, unsigned n) -> std::int32_t {
                ret_code = core::pal::absolute_native(s.data(), p, n);
                if (ret_code == -1 && errno == ERANGE) { // 空间不足
                    req_len *= 2;
                    req_len += path.native().size();
                    return 0; // 并未写入
                } else {
                    return ret_code; // 让ret_code作为实际返回字数
                }
            });
        } while (ret_code > buf.size());
        if (ret_code == -1) {
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
        text::basic_string_view<core::native_char> s = path.native();
        std::size_t req_len = 128;
        path::string_type buf;
        int ret_code = -1;
        do {
            buf.resize_and_overwrite(req_len, [&s, &req_len, &ret_code, &path](auto *p, unsigned n) -> std::int32_t {
                ret_code = core::pal::canonical_native(s.data(), p, n);
                if (ret_code == -1 && errno == ERANGE) { // 空间不足
                    req_len *= 2;
                    req_len += path.native().size();
                    return 0; // 并未写入
                }
                return ret_code; // 让ret_code作为实际返回字数
            });
        } while (ret_code > buf.size());
        if (ret_code == -1) {
            ec = std::error_code(errno, std::system_category());
        } else {
            result = utility::move(buf);
        }
        if (ec || !exists(result, ec)) {
            result.clear();
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
        const bool success = core::pal::copy_file(from.native().c_str(), from.native().c_str());
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
        const bool success = core::pal::copy_file(from.native().c_str(), from.native().c_str(), option);
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
        std::size_t req_len = 128;
        path::string_type buf;
        int ret_code = -1;
        do {
            buf.resize_and_overwrite(req_len, [&req_len, &ret_code](auto *p, unsigned n) -> std::int32_t {
                ret_code = core::pal::current_path(p, n);
                if (ret_code == -1 && errno == ERANGE) { // 空间不足
                    req_len *= 2;
                    return 0; // 并未写入
                }
                return ret_code; // 让ret_code作为实际返回字数
            });
        } while (ret_code > buf.size());
        if (ret_code == -1) {
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

    bool is_block_file(file_status string) noexcept;
    bool is_block_file(const path &path);
    bool is_block_file(const path &path, std::error_code &ec) noexcept;

    bool is_character_file(file_status string) noexcept;
    bool is_character_file(const path &path);
    bool is_character_file(const path &path, std::error_code &ec) noexcept;

    bool is_directory(file_status string) noexcept;
    bool is_directory(const path &path);
    bool is_directory(const path &path, std::error_code &ec) noexcept;

    bool is_empty(const path &path);
    bool is_empty(const path &path, std::error_code &ec);

    bool is_fifo(file_status string) noexcept;
    bool is_fifo(const path &path);
    bool is_fifo(const path &path, std::error_code &ec) noexcept;

    bool is_other(file_status string) noexcept;
    bool is_other(const path &path);
    bool is_other(const path &path, std::error_code &ec) noexcept;

    bool is_regular_file(file_status string) noexcept;
    bool is_regular_file(const path &path);
    bool is_regular_file(const path &path, std::error_code &ec) noexcept;

    bool is_socket(file_status string) noexcept;
    bool is_socket(const path &path);
    bool is_socket(const path &path, std::error_code &ec) noexcept;

    bool is_symlink(file_status string) noexcept;
    bool is_symlink(const path &path);
    bool is_symlink(const path &path, std::error_code &ec) noexcept;

    file_time_type last_write_time(const path &path);
    file_time_type last_write_time(const path &path, std::error_code &ec) noexcept;
    void last_write_time(const path &path, file_time_type new_time);
    void last_write_time(const path &path, file_time_type new_time, std::error_code &ec) noexcept;

    void permissions(const path &path, perms prms, perm_options opts);
    void permissions(const path &path, perms prms, std::error_code &ec) noexcept;
    void permissions(const path &path, perms prms, perm_options opts, std::error_code &ec);

    path proximate(const path &path, std::error_code &ec);
    path proximate(const path &path, const filesystem::path &base);
    path proximate(const path &path, const filesystem::path &base, std::error_code &ec);

    path read_symlink(const path &path);
    path read_symlink(const path &path, std::error_code &ec);

    path relative(const path &path, std::error_code &ec);
    path relative(const path &path, const filesystem::path &base);
    path relative(const path &path, const filesystem::path &base, std::error_code &ec);

    bool remove(const path &path);
    bool remove(const path &path, std::error_code &ec) noexcept;

    std::uintmax_t remove_all(const path &path);
    std::uintmax_t remove_all(const path &path, std::error_code &ec);

    void rename(const path &from, const path &to);
    void rename(const path &from, const path &to, std::error_code &ec) noexcept;

    void resize_file(const path &path, std::uintmax_t size);
    void resize_file(const path &path, std::uintmax_t size, std::error_code &ec) noexcept;

    space_info space(const path &path);
    space_info space(const path &path, std::error_code &ec) noexcept;

    file_status status(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(status, path);
    }

    file_status status(const path &path, std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        auto status = core::pal::status(path.c_str());
        if (errno != 0) {
            ec = std::error_code(errno, std::generic_category());
            file_status s{file_type::none, perms::none};
            return s;
        }
        file_status res{status.type, status.permissions};
        return res;
    }

    bool status_known(const file_status &status) noexcept {
        return status.type() != file_type::none;
    }

    file_status symlink_status(const path &path);
    file_status symlink_status(const path &path, std::error_code &ec) noexcept;

    path temp_directory_path();
    path temp_directory_path(std::error_code &ec);

    path weakly_canonical(const path &path) {
        RAINY_FILESYSTEM_EXCEPTION_EDITION_IMPL(weakly_canonical, path);
    }

    path weakly_canonical(const path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        class path result;
        file_status st = status(path, ec);
        if (exists(st)) {
            return canonical(path, ec);
        }

        if (status_known(st)) {
            ec.clear();
        } else {
            return result;
        }
        class path tmp{};
        auto iter = path.begin();
        const auto end = path.end();
        while (iter != end) {
            tmp = result / *iter;
            st = status(tmp, ec);
            if (exists(st)) {
                swap(result, tmp);
            } else {
                if (status_known(st)) {
                    ec.clear();
                }
                break;
            }
            ++iter;
        }
        if (!ec && !result.empty()) {
            result = canonical(result, ec);
        }
        if (ec) {
            result.clear();
        } else {
            while (iter != end) {
                result /= *iter++;
            }
            result = result.lexically_normal();
        }
        return result;
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)
