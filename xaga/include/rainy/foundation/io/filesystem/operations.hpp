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
#ifndef RAINY_FOUNDATION_IO_OPERATIONS_HPP
#define RAINY_FOUNDATION_IO_OPERATIONS_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/filesystem/fwd.hpp>
#include <rainy/foundation/io/filesystem/path.hpp>

namespace rainy::foundation::io::filesystem {
    // check...
    RAINY_TOOLKIT_API path absolute(const path &path);
    RAINY_TOOLKIT_API path absolute(const path &path, std::error_code &ec);

    // check...
    RAINY_TOOLKIT_API path canonical(const path &path);
    RAINY_TOOLKIT_API path canonical(const path &path, std::error_code &ec);

    // check...
    RAINY_TOOLKIT_API void copy(const path &from, const path &to);
    RAINY_TOOLKIT_API void copy(const path &from, const path &to, std::error_code &ec);
    RAINY_TOOLKIT_API void copy(const path &from, const path &to, copy_options options);
    RAINY_TOOLKIT_API void copy(const path &from, const path &to, copy_options options, std::error_code &ec);

    // check...
    RAINY_TOOLKIT_API bool copy_file(const path &from, const path &to);
    RAINY_TOOLKIT_API bool copy_file(const path &from, const path &to, std::error_code &ec);
    RAINY_TOOLKIT_API bool copy_file(const path &from, const path &to, copy_options option);
    RAINY_TOOLKIT_API bool copy_file(const path &from, const path &to, copy_options option, std::error_code &ec);

    // check...
    RAINY_TOOLKIT_API void copy_symlink(const path &existing_symlink, const path &new_symlink);
    RAINY_TOOLKIT_API void copy_symlink(const path &existing_symlink, const path &new_symlink, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API bool create_directories(const path &path);
    RAINY_TOOLKIT_API bool create_directories(const path &path, std::error_code &ec);

    // check...
    RAINY_TOOLKIT_API bool create_directory(const path &path);
    RAINY_TOOLKIT_API bool create_directory(const path &path, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API bool create_directory(const path &path, const filesystem::path &existing_p);
    RAINY_TOOLKIT_API bool create_directory(const path &path, const filesystem::path &existing_p, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API void create_directory_symlink(const path &to, const path &new_symlink);
    RAINY_TOOLKIT_API void create_directory_symlink(const path &to, const path &new_symlink, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API void create_hard_link(const path &to, const path &new_hard_link);
    RAINY_TOOLKIT_API void create_hard_link(const path &to, const path &new_hard_link, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API void create_symlink(const path &to, const path &new_symlink);
    RAINY_TOOLKIT_API void create_symlink(const path &to, const path &new_symlink, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API path current_path();
    RAINY_TOOLKIT_API path current_path(std::error_code &ec);
    RAINY_TOOLKIT_API void current_path(const path &path);
    RAINY_TOOLKIT_API void current_path(const path &path, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API bool equivalent(const path &left, const path &right);
    RAINY_TOOLKIT_API bool equivalent(const path &left, const path &right, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API bool exists(const file_status &status) noexcept;
    RAINY_TOOLKIT_API bool exists(const path &path);
    RAINY_TOOLKIT_API bool exists(const path &path, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API std::uintmax_t file_size(const path &path);
    RAINY_TOOLKIT_API std::uintmax_t file_size(const path &path, std::error_code &ec) noexcept;

    // check...
    RAINY_TOOLKIT_API std::uintmax_t hard_link_count(const path &path);
    RAINY_TOOLKIT_API std::uintmax_t hard_link_count(const path &path, std::error_code &ec) noexcept;

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

    void permissions(const path &path, perms prms, perm_options opts = perm_options::replace);
    void permissions(const path &path, perms prms, std::error_code &ec) noexcept;
    void permissions(const path &path, perms prms, perm_options opts, std::error_code &ec);

    path proximate(const path &path, std::error_code &ec);
    path proximate(const path &path, const filesystem::path &base = current_path());
    path proximate(const path &path, const filesystem::path &base, std::error_code &ec);

    path read_symlink(const path &path);
    path read_symlink(const path &path, std::error_code &ec);

    path relative(const path &path, std::error_code &ec);
    path relative(const path &path, const filesystem::path &base = current_path());
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

    // check...
    file_status status(const path &path);
    file_status status(const path &path, std::error_code &ec) noexcept;

    // check...
    bool status_known(const file_status &status) noexcept;

    file_status symlink_status(const path &path);
    file_status symlink_status(const path &path, std::error_code &ec) noexcept;

    path temp_directory_path();
    path temp_directory_path(std::error_code &ec);

    // check...
    path weakly_canonical(const path &path);
    path weakly_canonical(const path &path, std::error_code &ec);
}

#endif
