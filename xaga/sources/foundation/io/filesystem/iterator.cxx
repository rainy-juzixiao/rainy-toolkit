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
#include <rainy/foundation/io/filesystem/iterator.hpp>
#include <rainy/foundation/io/filesystem/operations.hpp>
#include <stack>

#if RAINY_USING_WINDOWS
#include <windows.h>
#else
#include <dirent.h>
#endif

namespace rainy::foundation::io::filesystem {
    struct directory_iterator::dir_impl {
#if RAINY_USING_WINDOWS
        HANDLE handle;
        WIN32_FIND_DATAA find_data;
#else
        DIR *dirp;
#endif
        directory_entry current_entry;
        bool is_valid;
        path dir_path;

        dir_impl() noexcept
#if RAINY_USING_WINDOWS
            :
            handle(INVALID_HANDLE_VALUE),
            is_valid(false){
#else
            : dirp(nullptr), is_valid(false) {
#endif
            }

            dir_impl(const filesystem::path &path, directory_options, std::error_code &ec) : is_valid(false), dir_path(path) {
#if RAINY_USING_WINDOWS
            auto search_path = path.string() + "\\*";
            handle = FindFirstFileA(search_path.c_str(), &find_data);
            if (handle == INVALID_HANDLE_VALUE) {
                ec = std::error_code(GetLastError(), std::system_category());
                return;
            }
            while (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
                if (!FindNextFileA(handle, &find_data)) {
                    if (GetLastError() == ERROR_NO_MORE_FILES) {
                        FindClose(handle);
                        handle = INVALID_HANDLE_VALUE;
                        return;
                    }
                    ec = std::error_code(GetLastError(), std::system_category());
                    FindClose(handle);
                    handle = INVALID_HANDLE_VALUE;
                    return;
                }
            }
            current_entry = directory_entry(path / find_data.cFileName);
            is_valid = true;
#else
            dirp = opendir(path.string().c_str());
            if (dirp == nullptr) {
                ec = std::error_code(errno, std::system_category());
                return;
            }
            const struct dirent *entry = readdir(dirp);
            if (entry == nullptr) {
                closedir(dirp);
                dirp = nullptr;
                return;
            }
            while (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                entry = readdir(dirp);
                if (entry == nullptr) {
                    closedir(dirp);
                    dirp = nullptr;
                    return;
                }
            }
            current_entry = directory_entry(path / entry->d_name);
            is_valid = true;
#endif
        }

        void advance(std::error_code &ec) {
            if (!is_valid) {
                return;
            }
#if RAINY_USING_WINDOWS
            if (!FindNextFileA(handle, &find_data)) {
                if (GetLastError() == ERROR_NO_MORE_FILES) {
                    FindClose(handle);
                    handle = INVALID_HANDLE_VALUE;
                    is_valid = false;
                    return;
                }
                ec = std::error_code(GetLastError(), std::system_category());
                FindClose(handle);
                handle = INVALID_HANDLE_VALUE;
                is_valid = false;
                return;
            }
            while (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
                if (!FindNextFileA(handle, &find_data)) {
                    if (GetLastError() == ERROR_NO_MORE_FILES) {
                        FindClose(handle);
                        handle = INVALID_HANDLE_VALUE;
                        is_valid = false;
                        return;
                    }
                    ec = std::error_code(GetLastError(), std::system_category());
                    FindClose(handle);
                    handle = INVALID_HANDLE_VALUE;
                    is_valid = false;
                    return;
                }
            }
            current_entry = directory_entry(dir_path / find_data.cFileName);
#else
            const struct dirent *entry = readdir(dirp);
            if (entry == nullptr) {
                closedir(dirp);
                dirp = nullptr;
                is_valid = false;
                return;
            }
            while (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                entry = readdir(dirp);
                if (entry == nullptr) {
                    closedir(dirp);
                    dirp = nullptr;
                    is_valid = false;
                    return;
                }
            }
            current_entry = directory_entry(dir_path / entry->d_name);
#endif
        }

        ~dir_impl() {
#if RAINY_USING_WINDOWS
            if (handle != INVALID_HANDLE_VALUE) {
                FindClose(handle);
            }
#else
            if (dirp != nullptr) {
                closedir(dirp);
            }
#endif
        }
    };

    directory_iterator::directory_iterator() noexcept : dir_() {
    }

    directory_iterator::directory_iterator(const filesystem::path &path) : dir_(memory::make_shared<dir_impl>()) {
        std::error_code ec;
        dir_ = memory::make_shared<dir_impl>(path, directory_options::none, ec);
        if (ec || !dir_->is_valid) {
            dir_.reset();
        }
    }

    directory_iterator::directory_iterator(const filesystem::path &path, directory_options options) :
        dir_(memory::make_shared<dir_impl>()) {
        std::error_code ec;
        dir_ = memory::make_shared<dir_impl>(path, options, ec);
        if (ec || !dir_->is_valid) {
            dir_.reset();
        }
    }

    directory_iterator::directory_iterator(const filesystem::path &path, std::error_code &ec) :
        dir_(memory::make_shared<dir_impl>(path, directory_options::none, ec)) {
        if (ec || !dir_->is_valid) {
            dir_.reset();
        }
    }

    directory_iterator::directory_iterator(const filesystem::path &path, directory_options options, std::error_code &ec) :
        dir_(memory::make_shared<dir_impl>(path, options, ec)) {
        if (ec || !dir_->is_valid) {
            dir_.reset();
        }
    }

    directory_iterator::directory_iterator(const path &p, directory_options opts, std::error_code *ec) : dir_() {
        if (ec != nullptr) {
            dir_ = memory::make_shared<dir_impl>(p, opts, *ec);
            if (*ec || !dir_->is_valid) {
                dir_.reset();
            }
        } else {
            std::error_code local_ec;
            dir_ = memory::make_shared<dir_impl>(p, opts, local_ec);
            if (local_ec || !dir_->is_valid) {
                dir_.reset();
            }
        }
    }

    directory_iterator::directory_iterator(const directory_iterator &right) = default;

    directory_iterator::directory_iterator(directory_iterator &&right) noexcept : dir_(utility::move(right.dir_)) {
    }

    directory_iterator::~directory_iterator() = default;

    directory_iterator &directory_iterator::operator=(const directory_iterator &right) {
        if (this != &right) {
            dir_ = right.dir_;
        }
        return *this;
    }

    directory_iterator &directory_iterator::operator=(directory_iterator &&right) noexcept {
        if (this != &right) {
            dir_ = utility::move(right.dir_);
        }
        return *this;
    }

    const directory_entry &directory_iterator::operator*() const {
        return dir_->current_entry;
    }

    const directory_entry *directory_iterator::operator->() const {
        return &dir_->current_entry;
    }

    directory_iterator &directory_iterator::operator++() {
        if (!dir_ || !dir_->is_valid) {
            return *this;
        }
        std::error_code ec;
        dir_->advance(ec);
        if (ec || !dir_->is_valid) {
            dir_.reset();
        }
        return *this;
    }

    directory_iterator &directory_iterator::increment(std::error_code &ec) {
        if (!dir_ || !dir_->is_valid) {
            ec = std::make_error_code(std::errc::no_such_file_or_directory);
            return *this;
        }
        dir_->advance(ec);
        if (ec || !dir_->is_valid) {
            dir_.reset();
        }
        return *this;
    }

    struct recursive_directory_iterator::dir_stack {
        struct level {
            level(const path &p, const directory_options o, const int d, std::error_code &ec) :
                iter(p, o, ec), end(), opts(o), depth(d) {
                ec.clear();
            }

            RAINY_NODISCARD bool is_end() const {
                return iter == end;
            }

            directory_iterator iter;
            directory_iterator end;
            directory_options opts;
            bool recursion_pending{true};
            int depth;
        };

        std::stack<level> levels;
        directory_entry current;
        bool is_valid;

        dir_stack() : is_valid(false) {
        }

        dir_stack(const path &p, directory_options o, std::error_code &ec) : is_valid(false) {
            ec.clear();
            levels.emplace(p, o, 0, ec);
            if (ec) {
                return;
            }
            if (levels.top().is_end()) {
                levels.pop();
                return;
            }
            current = directory_entry(*levels.top().iter);
            is_valid = true;
        }

        void advance(std::error_code &ec) {
            ec.clear();
            if (!is_valid || levels.empty()) {
                is_valid = false;
                return;
            }

            level &cur = levels.top();

            if (cur.recursion_pending && filesystem::is_directory(cur.iter->path(), ec)) {
                if (!ec) {
                    std::error_code sub_ec;
                    if (const directory_iterator sub(cur.iter->path(), cur.opts, sub_ec); !sub_ec && sub != directory_iterator()) {
                        levels.emplace(cur.iter->path(), cur.opts, cur.depth + 1, sub_ec);
                        if (!sub_ec && !levels.top().is_end()) {
                            current = directory_entry(*levels.top().iter);
                            return;
                        }
                    }
                } else {
                    if (!(static_cast<int>(cur.opts) & static_cast<int>(directory_options::skip_permission_denied))) {
                        return;
                    }
                    ec.clear();
                }
            }

            ++cur.iter;
            if (ec) {
                is_valid = false;
                return;
            }

            while (!levels.empty() && levels.top().is_end()) {
                levels.pop();
                if (!levels.empty()) {
                    ++levels.top().iter;
                    if (ec) {
                        is_valid = false;
                        return;
                    }
                }
            }

            if (levels.empty()) {
                is_valid = false;
                return;
            }

            current = directory_entry(*levels.top().iter);
        }

        void pop(std::error_code &ec) {
            ec.clear();
            if (levels.empty()) {
                is_valid = false;
                return;
            }

            levels.pop();

            if (levels.empty()) {
                is_valid = false;
                return;
            }

            ++levels.top().iter;
            if (ec) {
                is_valid = false;
                return;
            }

            while (!levels.empty() && levels.top().is_end()) {
                levels.pop();
                if (!levels.empty()) {
                    ++levels.top().iter;
                    if (ec) {
                        is_valid = false;
                        return;
                    }
                }
            }

            if (levels.empty()) {
                is_valid = false;
                return;
            }

            current = directory_entry(*levels.top().iter);
        }

        void disable_recursion_pending() {
            if (!levels.empty()) {
                levels.top().recursion_pending = false;
            }
        }

        int depth() const {
            if (levels.empty()) {
                return 0;
            }
            return static_cast<int>(levels.size()) - 1;
        }

        bool recursion_pending() const {
            if (levels.empty()) {
                return false;
            }
            return levels.top().recursion_pending;
        }

        directory_options options() const {
            if (levels.empty()) {
                return directory_options::none;
            }
            return levels.top().opts;
        }
    };

    recursive_directory_iterator::recursive_directory_iterator() noexcept : dirs() {
    }

    recursive_directory_iterator::recursive_directory_iterator(const filesystem::path &path) : dirs(memory::make_shared<dir_stack>()) {
        std::error_code ec;
        dirs = memory::make_shared<dir_stack>(path, directory_options::none, ec);
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(const filesystem::path &path, directory_options options) :
        dirs(memory::make_shared<dir_stack>()) {
        std::error_code ec;
        dirs = memory::make_shared<dir_stack>(path, options, ec);
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(const filesystem::path &path, std::error_code &ec) :
        dirs(memory::make_shared<dir_stack>(path, directory_options::none, ec)) {
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(const filesystem::path &path, directory_options options,
                                                               std::error_code &ec) :
        dirs(memory::make_shared<dir_stack>(path, options, ec)) {
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(const path &p, directory_options opts, std::error_code *ec) : dirs() {
        if (ec != nullptr) {
            dirs = memory::make_shared<dir_stack>(p, opts, *ec);
            if (*ec || !dirs->is_valid) {
                dirs.reset();
            }
        } else {
            std::error_code local_ec;
            dirs = memory::make_shared<dir_stack>(p, opts, local_ec);
            if (local_ec || !dirs->is_valid) {
                dirs.reset();
            }
        }
    }

    recursive_directory_iterator::recursive_directory_iterator(const recursive_directory_iterator &right) : dirs(right.dirs) {
    }

    recursive_directory_iterator::recursive_directory_iterator(recursive_directory_iterator &&right) noexcept :
        dirs(utility::move(right.dirs)) {
    }

    recursive_directory_iterator::~recursive_directory_iterator() = default;

    directory_options recursive_directory_iterator::options() const {
        if (!dirs) {
            return directory_options::none;
        }
        return dirs->options();
    }

    int recursive_directory_iterator::depth() const {
        if (!dirs) {
            return 0;
        }
        return dirs->depth();
    }

    bool recursive_directory_iterator::recursion_pending() const {
        if (!dirs) {
            return false;
        }
        return dirs->recursion_pending();
    }

    const directory_entry &recursive_directory_iterator::operator*() const {
        return dirs->current;
    }

    const directory_entry *recursive_directory_iterator::operator->() const {
        return &dirs->current;
    }

    recursive_directory_iterator &recursive_directory_iterator::operator=(const recursive_directory_iterator &right) {
        if (this != &right) {
            dirs = right.dirs;
        }
        return *this;
    }

    recursive_directory_iterator &recursive_directory_iterator::operator=(recursive_directory_iterator &&right) noexcept {
        if (this != &right) {
            dirs = utility::move(right.dirs);
        }
        return *this;
    }

    recursive_directory_iterator &recursive_directory_iterator::operator++() {
        if (!dirs || !dirs->is_valid) {
            return *this;
        }
        std::error_code ec;
        dirs->advance(ec);
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
        return *this;
    }

    recursive_directory_iterator &recursive_directory_iterator::increment(std::error_code &ec) {
        if (!dirs || !dirs->is_valid) {
            ec = std::make_error_code(std::errc::no_such_file_or_directory);
            return *this;
        }
        dirs->advance(ec);
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
        return *this;
    }

    void recursive_directory_iterator::pop() {
        if (!dirs || !dirs->is_valid) {
            return;
        }
        std::error_code ec;
        dirs->pop(ec);
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
    }

    void recursive_directory_iterator::pop(std::error_code &ec) {
        if (!dirs || !dirs->is_valid) {
            ec = std::make_error_code(std::errc::no_such_file_or_directory);
            return;
        }
        dirs->pop(ec);
        if (ec || !dirs->is_valid) {
            dirs.reset();
        }
    }

    void recursive_directory_iterator::disable_recursion_pending() { // NOLINT
        if (dirs && dirs->is_valid) {
            dirs->disable_recursion_pending();
        }
    }

    recursive_directory_iterator &recursive_directory_iterator::erase(std::error_code *ec) {
        dirs.reset();
        if (ec != nullptr) {
            *ec = std::error_code();
        }
        return *this;
    }
}
