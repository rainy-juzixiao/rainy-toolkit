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
#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_STREAMFILE_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_STREAMFILE_HPP
#include <cstdint>
#include <rainy/foundation/io/filesystem/fwd.hpp>
#include <rainy/foundation/io/filesystem/basic_file.hpp>
#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/io_context.hpp>
#include <system_error>

namespace rainy::foundation::io::filesystem {
    class stream_file {
    public:
        using executor_type = io::io_context::executor_type;

        explicit stream_file(io::io_context &ctx, const std::filesystem::path &path, open_mode mode = open_mode::read_only) :
            file_(ctx, path, mode), offset_(0) {
            if (has_flag(mode, open_mode::append)) {
                std::error_code ec;
                offset_ = file_.size(ec); // 失败时 offset_ 保持 0
            }
        }

        explicit stream_file(io::io_context &ctx) : file_(ctx), offset_(0) {
        }

        stream_file(const stream_file &) = delete;
        stream_file &operator=(const stream_file &) = delete;

        stream_file(stream_file &&o) noexcept : file_(utility::move(o.file_)), offset_(o.offset_) {
            o.offset_ = 0;
        }

        stream_file &operator=(stream_file &&o) noexcept {
            if (this != &o) {
                file_ = utility::move(o.file_);
                offset_ = o.offset_;
                o.offset_ = 0;
            }
            return *this;
        }

        std::error_code open(const std::filesystem::path &path, open_mode mode = open_mode::read_only) {
            auto ec = file_.open(path, mode);
            if (!ec) {
                offset_ = has_flag(mode, open_mode::append) ? file_.size(ec) : 0;
            }
            return ec;
        }

        void close() noexcept {
            file_.close();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            return file_.is_open();
        }

        /**
         * @brief  移动内部偏移量
         * @return 移动后的绝对偏移，出错时返回 (uint64_t)-1 并设置 ec
         */
        std::uint64_t seek(std::int64_t offset, seek_basis basis, std::error_code &ec) noexcept {
            std::uint64_t new_offset{};
            switch (basis) {
                case seek_basis::begin:
                    if (offset < 0) {
                        ec = std::make_error_code(std::errc::invalid_argument);
                        return static_cast<std::uint64_t>(-1);
                    }
                    new_offset = static_cast<std::uint64_t>(offset);
                    break;
                case seek_basis::current:
                    if (offset < 0 && static_cast<std::uint64_t>(-offset) > offset_) {
                        ec = std::make_error_code(std::errc::invalid_argument);
                        return static_cast<std::uint64_t>(-1);
                    }
                    new_offset = static_cast<std::uint64_t>(static_cast<std::int64_t>(offset_) + offset);
                    break;
                case seek_basis::end: {
                    std::uint64_t sz = file_.size(ec);
                    if (ec)
                        return static_cast<std::uint64_t>(-1);
                    if (offset > 0 || static_cast<std::uint64_t>(-offset) > sz) {
                        ec = std::make_error_code(std::errc::invalid_argument);
                        return static_cast<std::uint64_t>(-1);
                    }
                    new_offset = static_cast<std::uint64_t>(static_cast<std::int64_t>(sz) + offset);
                    break;
                }
            }
            offset_ = new_offset;
            return offset_;
        }

        std::uint64_t seek(std::int64_t offset, seek_basis basis) {
            std::error_code ec;
            auto pos = this->seek(offset, basis, ec);
            if (ec) {
                throw std::system_error(ec, "stream_file::seek");
            }
            return pos;
        }

        RAINY_NODISCARD std::uint64_t tell() const noexcept {
            return offset_;
        }

        /**
         * @brief  从当前偏移读取，自动推进 offset_
         *
         * 与自由函数 read(stream, buffers, ec) 对接：
         *   read() 循环调用 read_some 直到缓冲区满或 EOF
         */
        std::size_t read_some(mutable_buffer buf, std::error_code &ec) noexcept {
            std::size_t n = file_.read_some_at(offset_, buf, ec);
            offset_ += n;
            return n;
        }

        std::size_t read_some(mutable_buffer buf) {
            std::error_code ec;
            std::size_t n = read_some(buf, ec);
            if (ec) {
                throw std::system_error(ec, "stream_file::read_some");
            }
            return n;
        }

        /**
         * @brief  向当前偏移写入，自动推进 offset_
         */
        std::size_t write_some(const_buffer buf, std::error_code &ec) noexcept {
            std::size_t n = file_.write_some_at(offset_, buf, ec);
            offset_ += n;
            return n;
        }

        std::size_t write_some(const_buffer buf) {
            std::error_code ec;
            std::size_t n = write_some(buf, ec);
            if (ec) {
                throw std::system_error(ec, "stream_file::write_some");
            }
            return n;
        }

        template <typename MutableBufferSequence, typename Handler>
        void async_read_some(const MutableBufferSequence &buf, Handler &&handler) {
            const std::uint64_t op_offset = offset_;
            const std::size_t op_size = buffer_size(buf);
            offset_ += op_size; // 乐观
            file_.async_read_some_at(
                op_offset, buf,
                [this, op_size, h = utility::forward<Handler>(handler)](std::error_code ec, std::size_t transferred) mutable {
                    if (transferred < op_size) {
                        offset_ -= (op_size - transferred);
                    }
                    h(ec, transferred);
                });
        }

        template <typename ConstBufferSequence, typename Handler>
        void async_write_some(const ConstBufferSequence &buf, Handler &&handler) {
            const std::uint64_t op_offset = offset_;
            const std::size_t op_size = buffer_size(buf);
            offset_ += op_size;
            file_.async_write_some_at(
                op_offset, buf,
                [this, op_size, h = utility::forward<Handler>(handler)](std::error_code ec, std::size_t transferred) mutable {
                    if (transferred < op_size) {
                        offset_ -= (op_size - transferred);
                    }
                    h(ec, transferred);
                });
        }

        RAINY_NODISCARD std::uint64_t size() const {
            return file_.size();
        }

        RAINY_NODISCARD std::uint64_t size(std::error_code &ec) const noexcept {
            return file_.size(ec);
        }

        std::error_code resize(std::uint64_t sz) noexcept {
            return file_.resize(sz);
        }

        RAINY_NODISCARD std::uintptr_t native_handle() const noexcept {
            return file_.native_handle();
        }
        RAINY_NODISCARD executor_type get_executor() noexcept {
            return file_.get_executor();
        }

    private:
        basic_file file_;
        std::uint64_t offset_;
    };
}

#endif
