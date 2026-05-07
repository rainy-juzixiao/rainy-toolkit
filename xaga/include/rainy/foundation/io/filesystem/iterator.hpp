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
#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_ITERATOR_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_ITERATOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/filesystem/directory_entry.hpp>
#include <rainy/foundation/memory/shared_ptr.hpp>

namespace rainy::foundation::io::filesystem::implements {
    struct directory_iterator_proxy {
        const directory_entry &operator*() const & noexcept {
            return entry;
        }

        directory_entry operator*() && noexcept {
            return utility::move(entry);
        }

    private:
        friend class filesystem::directory_iterator;
        friend class filesystem::recursive_directory_iterator;

        explicit directory_iterator_proxy(directory_entry entry) : entry(utility::move(entry)) {
        }

        directory_entry entry;
    };
}

namespace rainy::foundation::io::filesystem {
    class directory_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = directory_entry;
        using difference_type = std::ptrdiff_t;
        using pointer = const directory_entry *;
        using reference = const directory_entry &;

        directory_iterator() noexcept;
        explicit directory_iterator(const filesystem::path &path);
        directory_iterator(const filesystem::path &path, directory_options options);
        directory_iterator(const filesystem::path &path, std::error_code &ec);
        directory_iterator(const filesystem::path &path, directory_options options, std::error_code &ec);
        directory_iterator(const directory_iterator &right);
        directory_iterator(directory_iterator &&right) noexcept;
        ~directory_iterator();

        directory_iterator &operator=(const directory_iterator &right);
        directory_iterator &operator=(directory_iterator &&right) noexcept;

        const directory_entry &operator*() const;
        const directory_entry *operator->() const;
        directory_iterator &operator++();
        directory_iterator &increment(std::error_code &ec);

        implements::directory_iterator_proxy operator++(int) {
            implements::directory_iterator_proxy proxy{**this};
            ++*this;
            return proxy;
        }

        friend bool operator==(const directory_iterator &left, const directory_iterator &right) noexcept {
            return !right.dir_.owner_before(left.dir_) && !left.dir_.owner_before(right.dir_);
        }

#if RAINY_HAS_CXX20
        bool operator==(std::default_sentinel_t) const noexcept {
            return !dir_;
        }
#endif

    private:
        directory_iterator(const path &, directory_options, std::error_code *);

        struct dir_impl;

        memory::shared_ptr<dir_impl> dir_;
    };

    RAINY_NODISCARD RAINY_INLINE directory_iterator begin(directory_iterator iter) noexcept {
        return {utility::move(iter)}; // NOLINT
    }

    RAINY_NODISCARD RAINY_INLINE directory_iterator end(const directory_iterator&) noexcept {
        return {};
    }
}

namespace rainy::foundation::io::filesystem {
    class recursive_directory_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = directory_entry;
        using difference_type = std::ptrdiff_t;
        using pointer = const directory_entry *;
        using reference = const directory_entry &;

        recursive_directory_iterator() noexcept;
        explicit recursive_directory_iterator(const filesystem::path &path);
        recursive_directory_iterator(const filesystem::path &path, directory_options options);
        recursive_directory_iterator(const filesystem::path &path, directory_options options, std::error_code &ec);
        recursive_directory_iterator(const filesystem::path &path, std::error_code &ec);
        recursive_directory_iterator(const recursive_directory_iterator &right);
        recursive_directory_iterator(recursive_directory_iterator &&right) noexcept;
        ~recursive_directory_iterator();

        RAINY_NODISCARD directory_options options() const;
        RAINY_NODISCARD int depth() const;
        RAINY_NODISCARD bool recursion_pending() const;

        const directory_entry &operator*() const;
        const directory_entry *operator->() const;

        recursive_directory_iterator &operator=(const recursive_directory_iterator &right);
        recursive_directory_iterator &operator=(recursive_directory_iterator &&right) noexcept;

        recursive_directory_iterator &operator++();
        recursive_directory_iterator &increment(std::error_code &ec);

        implements::directory_iterator_proxy operator++(int) {
            implements::directory_iterator_proxy proxy{**this};
            ++*this;
            return proxy;
        }

        void pop();
        void pop(std::error_code &ec);
        void disable_recursion_pending();

        friend bool operator==(const recursive_directory_iterator &left, const recursive_directory_iterator &right) noexcept {
            return !right.dirs.owner_before(left.dirs) && !left.dirs.owner_before(right.dirs);
        }

#if RAINY_HAS_CXX20
        bool operator==(std::default_sentinel_t) const noexcept {
            return *this == recursive_directory_iterator();
        }
#endif
    private:
        recursive_directory_iterator(const path &, directory_options, std::error_code *);
        recursive_directory_iterator &erase(std::error_code * = nullptr);

        struct dir_stack;
        memory::shared_ptr<dir_stack> dirs;
    };

    RAINY_NODISCARD RAINY_INLINE recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept {
        return {utility::move(iter)}; // NOLINT
    }

    RAINY_NODISCARD RAINY_INLINE recursive_directory_iterator end(const recursive_directory_iterator&) noexcept {
        return {};
    }
}

#endif
