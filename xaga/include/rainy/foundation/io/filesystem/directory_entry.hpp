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
#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_DIRECTORY_ENTRY_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_DIRECTORY_ENTRY_HPP
#include <rainy/foundation/io/filesystem/fwd.hpp>
#include <rainy/foundation/io/filesystem/path.hpp>

namespace rainy::foundation::io::filesystem {
    class RAINY_TOOLKIT_API directory_entry {
    public:
        // constructors and destructor
        directory_entry() noexcept;
        directory_entry(const directory_entry &right);
        directory_entry(directory_entry &&) noexcept;
        explicit directory_entry(filesystem::path path);
        directory_entry(filesystem::path path, std::error_code &ec);
        ~directory_entry();

        // assignments
        directory_entry &operator=(const directory_entry &);
        directory_entry &operator=(directory_entry &&) noexcept;

        // modifiers
        void assign(const filesystem::path &path);
        void assign(const filesystem::path &path, std::error_code &ec);
        void replace_filename(const filesystem::path &path);
        void replace_filename(const filesystem::path &path, std::error_code &ec);
        void refresh();
        void refresh(std::error_code &ec) noexcept;

        // observers
        RAINY_NODISCARD const filesystem::path &path() const noexcept;
        RAINY_NODISCARD operator const filesystem::path &() const noexcept; // NOLINT
        RAINY_NODISCARD bool exists() const;
        RAINY_NODISCARD bool exists(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_block_file() const;
        RAINY_NODISCARD bool is_block_file(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_character_file() const;
        RAINY_NODISCARD bool is_character_file(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_directory() const;
        RAINY_NODISCARD bool is_directory(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_fifo() const;
        RAINY_NODISCARD bool is_fifo(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_other() const;
        RAINY_NODISCARD bool is_other(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_regular_file() const;
        RAINY_NODISCARD bool is_regular_file(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_socket() const;
        RAINY_NODISCARD bool is_socket(std::error_code &ec) const noexcept;
        RAINY_NODISCARD bool is_symlink() const;
        RAINY_NODISCARD bool is_symlink(std::error_code &ec) const noexcept;
        RAINY_NODISCARD std::uintmax_t file_size() const;
        RAINY_NODISCARD std::uintmax_t file_size(std::error_code &ec) const noexcept;
        RAINY_NODISCARD std::uintmax_t hard_link_count() const;
        RAINY_NODISCARD std::uintmax_t hard_link_count(std::error_code &ec) const noexcept;
        RAINY_NODISCARD file_time_type last_write_time() const;
        RAINY_NODISCARD file_time_type last_write_time(std::error_code &ec) const noexcept;
        RAINY_NODISCARD file_status status() const;
        RAINY_NODISCARD file_status status(std::error_code &ec) const noexcept;
        RAINY_NODISCARD file_status symlink_status() const;
        RAINY_NODISCARD file_status symlink_status(std::error_code &ec) const noexcept;

        friend bool operator==(const directory_entry &left, const directory_entry &right) noexcept;
        friend bool operator!=(const directory_entry &left, const directory_entry &right) noexcept;
        friend bool operator<(const directory_entry &left, const directory_entry &right) noexcept;
        friend bool operator<=(const directory_entry &left, const directory_entry &right) noexcept;
        friend bool operator>(const directory_entry &left, const directory_entry &right) noexcept;
        friend bool operator>=(const directory_entry &left, const directory_entry &right) noexcept;

        // inserter
        template <typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &ostream,
                                                             const directory_entry &directory) {
            return ostream << directory.path();
        }

    private:
        filesystem::path pathobject;
        file_status cache;

        friend class directory_iterator;
    };
}

#endif
