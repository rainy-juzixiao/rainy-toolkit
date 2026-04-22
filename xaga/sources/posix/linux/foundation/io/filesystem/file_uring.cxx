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
#include <rainy/foundation/io/filesystem/streamfile.hpp>
#include <rainy/foundation/io/io_context.hpp>

#include <fcntl.h>
#include <liburing.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rainy::foundation::io::filesystem::implements {
    static io_uring_sqe *get_sqe_from_op(completion_op *op, io_context_impl_base &ctx_impl, const int fd) {
        ctx_impl.associate_handle(op, static_cast<std::uintptr_t>(fd), nullptr);
        if (!op->io_handle) {
            return nullptr;
        }
        return ::io_uring_get_sqe(static_cast<io_uring *>(op->io_handle));
    }

    static void submit_ring(const completion_op *op) noexcept {
        if (op->io_handle) {
            ::io_uring_submit(static_cast<io_uring *>(op->io_handle));
        }
    }

    class uring_file_impl final : public file_impl_base {
    public:
        uring_file_impl() = default;

        ~uring_file_impl() override {
            close();
        }

        std::error_code open(const std::filesystem::path &path, const open_mode mode, io_context_impl_base &ctx) noexcept override {
            int flags = 0;
            // NOLINTBEGIN
            const bool r = has_flag(mode, open_mode::read_only);
            const bool w = has_flag(mode, open_mode::write_only);
            // NOLINTEND
            if (r && w) {
                flags = O_RDWR;
            } else if (w) {
                flags = O_WRONLY;
            } else {
                flags = O_RDONLY;
            }
            if (has_flag(mode, open_mode::create)) {
                flags |= O_CREAT;
            }
            if (has_flag(mode, open_mode::truncate)) {
                flags |= O_TRUNC;
            }
            if (has_flag(mode, open_mode::exclusive)) {
                flags |= O_EXCL;
            }
            if (has_flag(mode, open_mode::append)) {
                flags |= O_APPEND;
            }
            if (has_flag(mode, open_mode::sync)) {
                flags |= O_SYNC;
            }
            if (has_flag(mode, open_mode::direct)) {
                flags |= O_DIRECT;
            }
            fd_ = ::open(path.c_str(), flags, 0644);
            if (fd_ < 0) {
                return {errno, std::system_category()};
            }
            if (rainy_const res = ctx.associate_handle(nullptr, static_cast<std::uintptr_t>(fd_), nullptr);
                res != concurrency::thrd_result::success) {
                ::close(fd_);
                fd_ = -1;
                return {EINVAL, std::system_category()};
            }

            ctx_ = &ctx;
            return {};
        }

        void close() noexcept override {
            if (fd_ >= 0) {
                ::close(fd_);
                fd_ = -1;
            }
            ctx_ = nullptr;
        }

        bool is_open() const noexcept override {
            return fd_ >= 0;
        }

        std::size_t read_some_at(const mutable_buffer buf, const std::uint64_t offset, std::error_code &ec) noexcept override {
            const ssize_t n = ::pread(fd_, buf.data(), buf.size(), static_cast<::off_t>(offset));
            if (n < 0) {
                ec.assign(errno, std::system_category());
                return 0;
            }
            return static_cast<std::size_t>(n);
        }

        std::size_t write_some_at(const const_buffer buf, const std::uint64_t offset, std::error_code &ec) noexcept override {
            const ssize_t n = ::pwrite(fd_, buf.data(), buf.size(), static_cast<::off_t>(offset));
            if (n < 0) {
                ec.assign(errno, std::system_category());
                return 0;
            }
            return static_cast<std::size_t>(n);
        }

        void async_read_some_at(const mutable_buffer buf, const std::uint64_t offset, io_context_impl_base &ctx,
                                completion_op *op) noexcept override {
            io_uring_sqe *sqe = get_sqe_from_op(op, ctx, fd_);
            if (!sqe) {
                ctx.post_immediate_completion(op, false);
                return;
            }
            ::io_uring_prep_read(sqe, fd_, buf.data(), buf.size(), offset);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        void async_write_some_at(const const_buffer buf, const std::uint64_t offset, io_context_impl_base &ctx,
                                 completion_op *op) noexcept override {
            io_uring_sqe *sqe = get_sqe_from_op(op, ctx, fd_);
            if (!sqe) {
                ctx.post_immediate_completion(op, false);
                return;
            }
            ::io_uring_prep_write(sqe, fd_, buf.data(), buf.size(), offset);
            ::io_uring_sqe_set_data(sqe, op);
            submit_ring(op);
        }

        std::uint64_t size(std::error_code &ec) const noexcept override {
            struct ::stat st{};
            if (::fstat(fd_, &st) < 0) {
                ec.assign(errno, std::system_category());
                return 0;
            }
            return static_cast<std::uint64_t>(st.st_size);
        }

        std::error_code resize(const std::uint64_t new_size) noexcept override {
            if (::ftruncate(fd_, static_cast<::off_t>(new_size)) < 0) {
                return {errno, std::system_category()};
            }
            return {};
        }

        std::uintptr_t native_handle() const noexcept override {
            return static_cast<std::uintptr_t>(fd_);
        }

    private:
        struct submit_desc {
            enum op_kind {
                readv,
                writev
            } op_type;
            int fd;
            std::uint64_t offset;
            ::iovec *iov;
            int iov_cnt;
        };

        int fd_{-1};
        io_context_impl_base *ctx_{nullptr};
    };

    memory::unique_ptr<file_impl_base> make_file_impl() {
        return memory::make_unique<uring_file_impl>();
    }
}
