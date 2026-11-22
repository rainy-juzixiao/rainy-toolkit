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
#include <rainy/foundation/io/ipc/implements/shared_memory_impl.hpp>
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rainy::foundation::io::ipc::shared_memory::implements {
    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    class posix_shared_memory_impl final : public shared_memory_impl_base {
    public:
        posix_shared_memory_impl(int fd, void *mapped, std::size_t size, access_mode mode) noexcept
            : fd_(fd), mapped_(mapped), size_(size), mode_(mode) {
        }

        ~posix_shared_memory_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return mapped_ != nullptr && mapped_ != MAP_FAILED;
        }

        std::error_code close() noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            ::munmap(mapped_, size_);
            ::close(fd_);
            mapped_ = nullptr;
            fd_ = -1;
            size_ = 0;
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(fd_);
        }

        void *data() const noexcept override {
            return mapped_;
        }

        std::size_t size() const noexcept override {
            return size_;
        }

        std::error_code flush() noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            if (::msync(mapped_, size_, MS_SYNC) == -1) {
                return posix_error();
            }
            return {};
        }

        std::error_code flush(std::size_t offset, std::size_t length) noexcept override {
            if (!is_open()) {
                return posix_error(EBADF);
            }
            if (offset + length > size_) {
                return posix_error(EINVAL);
            }
            auto *base = static_cast<unsigned char *>(mapped_) + offset;
            if (::msync(base, length, MS_SYNC) == -1) {
                return posix_error();
            }
            return {};
        }

    private:
        int fd_{-1};
        void *mapped_{nullptr};
        std::size_t size_{0};
        access_mode mode_{access_mode::read_write};
    };

    class null_shared_memory_impl final : public shared_memory_impl_base {
    public:
        explicit null_shared_memory_impl(executor_type) noexcept {
        }

        bool is_open() const noexcept override {
            return false;
        }

        std::error_code close() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(-1);
        }

        void *data() const noexcept override {
            return nullptr;
        }

        std::size_t size() const noexcept override {
            return 0;
        }

        std::error_code flush() noexcept override {
            return {};
        }

        std::error_code flush(std::size_t, std::size_t) noexcept override {
            return {};
        }
    };

    static int oflags_for_mode(access_mode mode) {
        switch (mode) {
            case access_mode::read_only:
                return O_RDONLY;
            case access_mode::read_write:
                return O_RDWR;
        }
        return O_RDWR;
    }

    static int prot_for_mode(access_mode mode) {
        switch (mode) {
            case access_mode::read_only:
                return PROT_READ;
            case access_mode::read_write:
                return PROT_READ | PROT_WRITE;
        }
        return PROT_READ | PROT_WRITE;
    }

    core::memory::nebula_ptr<shared_memory_impl_base>
    create_shared_memory_impl(executor_type executor, const char *name, const attributes &attr, std::error_code &ec) {
        int oflag = O_CREAT | O_EXCL | oflags_for_mode(attr.mode);
        int fd = ::shm_open(name, oflag, 0600);
        if (fd == -1) {
            ec = posix_error();
            return create_null_shared_memory_impl(executor);
        }
        if (::ftruncate(fd, static_cast<off_t>(attr.size)) == -1) {
            ec = posix_error();
            ::close(fd);
            return create_null_shared_memory_impl(executor);
        }
        void *mapped = ::mmap(nullptr, attr.size, prot_for_mode(attr.mode), MAP_SHARED, fd, 0);
        if (mapped == MAP_FAILED) {
            ec = posix_error();
            ::close(fd);
            return create_null_shared_memory_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_shared_memory_impl>(fd, mapped, attr.size, attr.mode);
    }

    core::memory::nebula_ptr<shared_memory_impl_base>
    open_shared_memory_impl(executor_type executor, const char *name, access_mode mode, std::error_code &ec) {
        int fd = ::shm_open(name, oflags_for_mode(mode), 0600);
        if (fd == -1) {
            ec = posix_error();
            return create_null_shared_memory_impl(executor);
        }
        struct stat st;
        if (::fstat(fd, &st) == -1) {
            ec = posix_error();
            ::close(fd);
            return create_null_shared_memory_impl(executor);
        }
        auto size = static_cast<std::size_t>(st.st_size);
        void *mapped = ::mmap(nullptr, size, prot_for_mode(mode), MAP_SHARED, fd, 0);
        if (mapped == MAP_FAILED) {
            ec = posix_error();
            ::close(fd);
            return create_null_shared_memory_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_shared_memory_impl>(fd, mapped, size, mode);
    }

    core::memory::nebula_ptr<shared_memory_impl_base>
    open_or_create_shared_memory_impl(executor_type executor, const char *name, const attributes &attr, std::error_code &ec) {
        int fd = ::shm_open(name, O_CREAT | oflags_for_mode(attr.mode), 0600);
        if (fd == -1) {
            ec = posix_error();
            return create_null_shared_memory_impl(executor);
        }
        struct stat st;
        bool needs_truncate = true;
        if (::fstat(fd, &st) == 0) {
            needs_truncate = static_cast<std::size_t>(st.st_size) < attr.size;
        }
        if (needs_truncate && ::ftruncate(fd, static_cast<off_t>(attr.size)) == -1) {
            ec = posix_error();
            ::close(fd);
            return create_null_shared_memory_impl(executor);
        }
        auto actual_size = static_cast<std::size_t>(st.st_size);
        if (actual_size < attr.size) {
            actual_size = attr.size;
        }
        void *mapped = ::mmap(nullptr, actual_size, prot_for_mode(attr.mode), MAP_SHARED, fd, 0);
        if (mapped == MAP_FAILED) {
            ec = posix_error();
            ::close(fd);
            return create_null_shared_memory_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_shared_memory_impl>(fd, mapped, actual_size, attr.mode);
    }

    core::memory::nebula_ptr<shared_memory_impl_base> create_null_shared_memory_impl(executor_type executor) {
        return core::memory::make_nebula<null_shared_memory_impl>(executor);
    }

    std::error_code unlink_shared_memory(const char *name) noexcept {
        if (::shm_unlink(name) == -1) {
            return posix_error();
        }
        return {};
    }
}