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
#include <rainy/foundation/io/filesystem/directory_entry.hpp>
#include <rainy/foundation/io/filesystem/operations.hpp>
#include <utility>

#define RAINY_FILESYSTEM_INIT_SYSCALL(ec)                                                                                             \
    do {                                                                                                                              \
        ec.clear();                                                                                                                   \
        errno = 0;                                                                                                                    \
    } while (false)

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

namespace rainy::foundation::io::filesystem {
    directory_entry::directory_entry() noexcept = default;

    directory_entry::directory_entry(const directory_entry &right) = default;

    directory_entry::directory_entry(directory_entry &&right) noexcept :
        pathobject(utility::exchange(right.pathobject, {})), cache(utility::exchange(right.cache, {})) {
    }

    directory_entry::directory_entry(filesystem::path path) : pathobject(std::move(path)) {
        refresh();
    }

    directory_entry::directory_entry(filesystem::path path, std::error_code &ec) : pathobject(std::move(path)) {
        refresh(ec);
    }

    directory_entry::~directory_entry() = default;

    directory_entry &directory_entry::operator=(const directory_entry &right) {
        this->pathobject = right.pathobject;
        return *this;
    }

    directory_entry &directory_entry::operator=(directory_entry &&right) noexcept {
        this->pathobject = utility::move(right.pathobject);
        return *this;
    }

    void directory_entry::assign(const filesystem::path &path) {
        this->pathobject = path;
        refresh();
    }

    void directory_entry::assign(const filesystem::path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        this->pathobject = path;
        refresh(ec);
    }

    void directory_entry::replace_filename(const filesystem::path &path) {
        this->pathobject.replace_filename(path);
        refresh();
    }

    void directory_entry::replace_filename(const filesystem::path &path, std::error_code &ec) {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        this->pathobject.replace_filename(path);
        refresh(ec);
    }

    void directory_entry::refresh() {
        this->cache = symlink_status();
    }

    void directory_entry::refresh(std::error_code &ec) noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        this->cache = symlink_status(ec);
    }

    const filesystem::path &directory_entry::path() const noexcept {
        return pathobject;
    }

    directory_entry::operator const filesystem::path &() const noexcept {
        return pathobject;
    }

    bool directory_entry::exists() const {
        return filesystem::exists(status());
    }

    bool directory_entry::exists(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::exists(status(ec));
    }

    bool directory_entry::is_block_file() const {
        return status().type() == file_type::block;
    }

    bool directory_entry::is_block_file(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return status(ec).type() == file_type::block;
    }

    bool directory_entry::is_character_file() const {
        return status().type() == file_type::character;
    }

    bool directory_entry::is_character_file(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return status(ec).type() == file_type::character;
    }

    bool directory_entry::is_directory() const {
        return status().type() == file_type::directory;
    }

    bool directory_entry::is_directory(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return status(ec).type() == file_type::directory;
    }

    bool directory_entry::is_fifo() const {
        return status().type() == file_type::fifo;
    }

    bool directory_entry::is_fifo(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return status(ec).type() == file_type::fifo;
    }

    bool directory_entry::is_other() const {
        return filesystem::is_other(pathobject);
    }

    bool directory_entry::is_other(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::is_other(pathobject, ec);
    }

    bool directory_entry::is_regular_file() const {
        return status().type() == file_type::regular;
    }

    bool directory_entry::is_regular_file(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return status(ec).type() == file_type::regular;
    }

    bool directory_entry::is_socket() const {
        return status().type() == file_type::socket;
    }

    bool directory_entry::is_socket(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return status(ec).type() == file_type::socket;
    }

    bool directory_entry::is_symlink() const {
        if (cache.type() != file_type::none) {
            return cache.type() == file_type::symlink;
        }
        return symlink_status().type() == file_type::symlink;
    }

    bool directory_entry::is_symlink(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        if (cache.type() != file_type::none) {
            return cache.type() == file_type::symlink;
        }
        return symlink_status(ec).type() == file_type::symlink;
    }

    std::uintmax_t directory_entry::file_size() const {
        return filesystem::file_size(pathobject);
    }

    std::uintmax_t directory_entry::file_size(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::file_size(pathobject, ec);
    }

    std::uintmax_t directory_entry::hard_link_count() const {
        return filesystem::hard_link_count(pathobject);}

    std::uintmax_t directory_entry::hard_link_count(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::hard_link_count(pathobject, ec);
    }

    file_time_type directory_entry::last_write_time() const {
        return filesystem::last_write_time(pathobject);
    }

    file_time_type directory_entry::last_write_time(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::last_write_time(pathobject, ec);
    }

    file_status directory_entry::status() const {
        return filesystem::status(pathobject);
    }

    file_status directory_entry::status(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::status(pathobject, ec);
    }

    file_status directory_entry::symlink_status() const {
        return filesystem::symlink_status(pathobject);
    }

    file_status directory_entry::symlink_status(std::error_code &ec) const noexcept {
        RAINY_FILESYSTEM_INIT_SYSCALL(ec);
        return filesystem::symlink_status(pathobject);
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)
