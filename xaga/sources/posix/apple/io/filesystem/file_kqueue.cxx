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
#include <rainy/foundation/concurrency/executor.hpp>
#include <rainy/foundation/io/filesystem/streamfile.hpp>
#include <rainy/foundation/io/net/io_context.hpp>

#include <fcntl.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rainy::foundation::io::filesystem::implements {
    class kqueue_file_impl final : public file_impl_base {
    public:
        kqueue_file_impl() = default;

        ~kqueue_file_impl() override {
            close();
        }

        std::error_code open(const std::filesystem::path &path, open_mode mode,
                             net::implements::io_context_impl_base &ctx) noexcept override {
            int flags = 0;
            const bool r = has_flag(mode, open_mode::read_only);
            const bool w = has_flag(mode, open_mode::write_only);
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

            fd_ = ::open(path.c_str(), flags, 0644);
            if (fd_ < 0) {
                return {errno, std::system_category()};
            }
            auto res = ctx.associate_handle(nullptr, static_cast<std::uintptr_t>(fd_), nullptr);
            if (res != concurrency::thrd_result::success) {
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

        std::size_t read_some_at(net::mutable_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept override {
            const ssize_t n = ::pread(fd_, buf.data(), buf.size(), static_cast<::off_t>(offset));
            if (n < 0) {
                ec.assign(errno, std::system_category());
                return 0;
            }
            return static_cast<std::size_t>(n);
        }

        std::size_t write_some_at(net::const_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept override {
            const ssize_t n = ::pwrite(fd_, buf.data(), buf.size(), static_cast<::off_t>(offset));
            if (n < 0) {
                ec.assign(errno, std::system_category());
                return 0;
            }
            return static_cast<std::size_t>(n);
        }

        void async_read_some_at(net::mutable_buffer buf, std::uint64_t offset, net::implements::io_context_impl_base &ctx,
                                completion_op *op) noexcept override {
            const int fd = fd_;
            get_executor().submit([fd, buf, offset, op]() mutable {
                op_result result{};
                result.user_data = op;
                const ssize_t n = ::pread(fd, buf.data(), buf.size(), static_cast<::off_t>(offset));
                if (n < 0) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(n);
                }
                op->complete(result, false);
            });
        }

        void async_write_some_at(net::const_buffer buf, std::uint64_t offset, net::implements::io_context_impl_base &ctx,
                                 completion_op *op) noexcept override {
            const int fd = fd_;
            get_executor().submit([fd, buf, offset, op]() mutable {
                op_result result{};
                result.user_data = op;
                const ssize_t n = ::pwrite(fd, buf.data(), buf.size(), static_cast<::off_t>(offset));
                if (n < 0) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(n);
                }
                op->complete(result, false);
            });
        }

        std::uint64_t size(std::error_code &ec) const noexcept override {
            struct ::stat st{};
            if (::fstat(fd_, &st) < 0) {
                ec.assign(errno, std::system_category());
                return 0;
            }
            return static_cast<std::uint64_t>(st.st_size);
        }

        std::error_code resize(std::uint64_t new_size) noexcept override {
            if (::ftruncate(fd_, static_cast<::off_t>(new_size)) < 0) {
                return {errno, std::system_category()};
            }
            return {};
        }

        std::uintptr_t native_handle() const noexcept override {
            return static_cast<std::uintptr_t>(fd_);
        }

    private:
        static concurrency::executor &get_executor() noexcept {
            return concurrency::get_global_pooled_executor();
        }

        int fd_{-1};
        net::implements::io_context_impl_base *ctx_{nullptr};
    };
}

namespace rainy::foundation::io::filesystem::implements {
    memory::unique_ptr<file_impl_base> make_file_impl() {
        return memory::make_unique<kqueue_file_impl>();
    }
}
