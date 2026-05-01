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
#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_BASIC_FILE_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_BASIC_FILE_HPP
#include <cstdint>
#include <filesystem>
#include <rainy/foundation/io/filesystem/implements/file.hpp>
#include <rainy/foundation/io/io_context.hpp>

namespace rainy::foundation::io::filesystem {
    class basic_file {
    public:
        using executor_type = io_context::executor_type;

        /**
         * @brief  构造并立即打开文件
         * @param  ctx   驱动异步 I/O 的 io_context
         * @param  path  目标路径
         * @param  mode  打开模式，默认只读
         */
        explicit basic_file(io_context &ctx, const std::filesystem::path &path, open_mode mode = open_mode::read_only) :
            ctx_(&ctx), impl_(implements::make_file_impl()) {
            std::error_code ec = impl_->open(path, mode, ctx_->get_executor());
            if (ec) {
                throw std::system_error(ec, "basic_file::open");
            }
        }

        /**
         * @brief  构造后不立即打开，稍后调用 open()
         */
        explicit basic_file(io_context &ctx) : ctx_(&ctx), impl_(implements::make_file_impl()) {
        }

        basic_file(const basic_file &) = delete;
        basic_file &operator=(const basic_file &) = delete;

        basic_file(basic_file &&) = default;
        basic_file &operator=(basic_file &&) = default;

        ~basic_file() {
            if (impl_ && impl_->is_open()) {
                impl_->close();
            }
        }

        std::error_code open(const std::filesystem::path &path, open_mode mode = open_mode::read_only) {
            return impl_->open(path, mode, ctx_->get_executor());
        }

        void close() noexcept {
            impl_->close();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            if (!impl_) {
                return false;
            }
            return impl_->is_open();
        }

        /**
         * @brief  在偏移 offset 处读取至多 buf.size() 字节
         *
         * 配合自由函数 read_at(device, offset, buffers, ec) 使用，
         * 该自由函数会循环调用 read_some_at 直到满足完成条件。
         */
        std::size_t read_some_at(std::uint64_t offset, mutable_buffer buf, std::error_code &ec) noexcept {
            return impl_->read_some_at(buf, offset, ec);
        }

        std::size_t read_some_at(std::uint64_t offset, mutable_buffer buf) {
            std::error_code ec;
            const std::size_t n = read_some_at(offset, buf, ec);
            if (ec) {
                throw std::system_error(ec, "basic_file::read_some_at");
            }
            return n;
        }

        /**
         * @brief  在偏移 offset 处写入至多 buf.size() 字节
         */
        std::size_t write_some_at(std::uint64_t offset, const_buffer buf, std::error_code &ec) noexcept {
            return impl_->write_some_at(buf, offset, ec);
        }

        std::size_t write_some_at(std::uint64_t offset, const_buffer buf) {
            std::error_code ec;
            const std::size_t n = write_some_at(offset, buf, ec);
            if (ec) {
                throw std::system_error(ec, "basic_file::write_some_at");
            }
            return n;
        }

        template <typename MutableBufferSequence, typename Handler>
        void async_read_some_at(std::uint64_t offset, const MutableBufferSequence &buf, Handler &&handler) {
            ctx_->get_executor().on_work_started();
            auto *op = io::implements::make_io_completion_op(
                [h = utility::forward<Handler>(handler), this](const implements::op_result &res, bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (res.error_code) {
                        ec.assign(res.error_code, std::system_category());
                    }
                    h(ec, res.bytes_transferred);
                    ctx_->get_executor().on_work_finished();
                });

            impl_->async_read_some_at(buffer(buf), offset, ctx_->get_executor(), op);
        }

        template <typename ConstBufferSequence, typename Handler>
        void async_write_some_at(std::uint64_t offset, const ConstBufferSequence &buf, Handler &&handler) {
            ctx_->get_executor().on_work_started();
            auto *op = io::implements::make_io_completion_op(
                [h = utility::forward<Handler>(handler), this](const implements::op_result &res, bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (res.error_code) {
                        ec.assign(res.error_code, std::system_category());
                    }
                    h(ec, res.bytes_transferred);
                    ctx_->get_executor().on_work_finished();
                });

            impl_->async_write_some_at(buffer(buf), offset, ctx_->get_executor(), op);
        }
        RAINY_NODISCARD std::uint64_t size() const {
            std::error_code ec;
            auto s = impl_->size(ec);
            if (ec) {
                throw std::system_error(ec, "basic_file::size");
            }
            return s;
        }

        RAINY_NODISCARD std::uint64_t size(std::error_code &ec) const noexcept {
            return impl_->size(ec);
        }

        std::error_code resize(std::uint64_t new_size) noexcept {
            return impl_->resize(new_size);
        }

        RAINY_NODISCARD std::uintptr_t native_handle() const noexcept {
            return impl_->native_handle();
        }

        RAINY_NODISCARD executor_type get_executor() noexcept { // NOLINT
            return ctx_->get_executor();
        }

    private:
        io::io_context *ctx_;
        memory::nebula_ptr<implements::file_impl_base> impl_;
    };
}

#endif
