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
#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_RANDOMACCESSFILE_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_RANDOMACCESSFILE_HPP

#include <cstdint>
#include <system_error>
#include <rainy/foundation/io/filesystem/fwd.hpp>
#include <rainy/foundation/io/filesystem/basic_file.hpp>
#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/io_context.hpp>

namespace rainy::foundation::io::filesystem {
    /**
     * @brief  随机访问文件
     *
     * 与 stream_file 的本质区别：
     *   - 不持有内部偏移量，每个读写操作均需调用方显式提供 offset
     *   - 直接透传至 basic_file 的 *_some_at 系列接口
     *   - 适合数据库、日志、索引等需要在文件任意位置并发读写的场景
     */
    class random_access_file {
    public:
        using executor_type = io::io_context::executor_type;

        /**
         * @brief  构造并立即打开文件
         */
        explicit random_access_file(io::io_context &ctx,
                                    const std::filesystem::path &path,
                                    open_mode mode = open_mode::read_only)
            : file_(ctx, path, mode) {}

        /**
         * @brief  构造后不立即打开，稍后调用 open()
         */
        explicit random_access_file(io::io_context &ctx)
            : file_(ctx) {}

        random_access_file(const random_access_file &) = delete;
        random_access_file &operator=(const random_access_file &) = delete;

        random_access_file(random_access_file &&) noexcept = default;
        random_access_file &operator=(random_access_file &&) noexcept = default;

        std::error_code open(const std::filesystem::path &path,
                             open_mode mode = open_mode::read_only) {
            return file_.open(path, mode);
        }

        void close() noexcept {
            file_.close();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            return file_.is_open();
        }

        /**
         * @brief  在指定偏移处读取至多 buf.size() 字节
         * @return 实际读取字节数；出错时设置 ec
         */
        std::size_t read_some_at(std::uint64_t offset,
                                 mutable_buffer buf,
                                 std::error_code &ec) noexcept {
            return file_.read_some_at(offset, buf, ec);
        }

        /**
         * @brief  在指定偏移处读取至多 buf.size() 字节（抛出异常版本）
         */
        std::size_t read_some_at(std::uint64_t offset, mutable_buffer buf) {
            return file_.read_some_at(offset, buf);
        }

        /**
         * @brief  在指定偏移处写入至多 buf.size() 字节
         * @return 实际写入字节数；出错时设置 ec
         */
        std::size_t write_some_at(std::uint64_t offset,
                                  const_buffer buf,
                                  std::error_code &ec) noexcept {
            return file_.write_some_at(offset, buf, ec);
        }

        /**
         * @brief  在指定偏移处写入至多 buf.size() 字节（抛出异常版本）
         */
        std::size_t write_some_at(std::uint64_t offset, const_buffer buf) {
            return file_.write_some_at(offset, buf);
        }

        /**
         * @brief  在指定偏移处异步读取
         *
         * @param  offset   文件偏移（字节）
         * @param  buf      目标缓冲区序列
         * @param  handler  完成回调，签名须为 void(std::error_code, std::size_t)
         *
         * @note   与 stream_file::async_read_some 不同，此处不做乐观偏移推进，
         *         offset 完全由调用方管理，天然支持并发多区间读写。
         */
        template <typename MutableBufferSequence, typename Handler>
        void async_read_some_at(std::uint64_t offset,
                                const MutableBufferSequence &buf,
                                Handler &&handler) {
            file_.async_read_some_at(offset, buf,
                                     utility::forward<Handler>(handler));
        }

        /**
         * @brief  在指定偏移处异步写入
         *
         * @param  offset   文件偏移（字节）
         * @param  buf      源缓冲区序列
         * @param  handler  完成回调，签名须为 void(std::error_code, std::size_t)
         */
        template <typename ConstBufferSequence, typename Handler>
        void async_write_some_at(std::uint64_t offset,
                                 const ConstBufferSequence &buf,
                                 Handler &&handler) {
            file_.async_write_some_at(offset, buf,
                                      utility::forward<Handler>(handler));
        }

        /**
         * @brief  返回文件字节大小（抛出异常版本）
         */
        RAINY_NODISCARD std::uint64_t size() const {
            return file_.size();
        }

        /**
         * @brief  返回文件字节大小（错误码版本）
         */
        RAINY_NODISCARD std::uint64_t size(std::error_code &ec) const noexcept {
            return file_.size(ec);
        }

        /**
         * @brief  调整文件大小
         * @return 操作结果错误码，无错误时为默认构造的 error_code
         */
        std::error_code resize(std::uint64_t new_size) noexcept {
            return file_.resize(new_size);
        }

        RAINY_NODISCARD std::uintptr_t native_handle() const noexcept {
            return file_.native_handle();
        }

        RAINY_NODISCARD executor_type get_executor() noexcept {
            return file_.get_executor();
        }

    private:
        basic_file file_;
    };
}

#endif