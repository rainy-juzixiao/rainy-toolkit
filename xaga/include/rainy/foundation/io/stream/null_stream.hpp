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
#ifndef RAINY_IO_STREAM_NULL_STREAM_HPP
#define RAINY_IO_STREAM_NULL_STREAM_HPP
#include <rainy/foundation/io/buffer.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>

namespace rainy::foundation::io::stream {
    /**
     * \lang chinese
     *
     * @brief 空流类，用于丢弃所有写入数据或返回空数据
     * @details 该类实现了类似/dev/null的行为，所有写入操作都会成功但数据被丢弃，
     *          所有读取操作都会返回0字节。适用于需要流接口但不希望实际处理数据的场景。
     *
     * \lang english
     *
     * @brief Null stream class for discarding all written data or returning empty data
     * @details This class implements behavior similar to /dev/null, where all write operations
     *          succeed but data is discarded, and all read operations return zero bytes.
     *          Suitable for scenarios requiring a stream interface without actual data processing.
     */
    class RAINY_TOOLKIT_API null_stream {
    public:
        /**
         * \lang chinese
         *
         * @brief 执行器类型定义
         *
         * \lang english
         *
         * @brief Executor type definition
         */
        using executor_type = io_context::executor_type;

        /**
         * \lang chinese
         *
         * @brief 获取空流的单例实例
         * @return 空流对象的引用
         *
         * \lang english
         *
         * @brief Get the singleton instance of the null stream
         * @return Reference to the null stream object
         */
        static null_stream &null();

        /**
         * \lang chinese
         *
         * @brief 删除拷贝构造函数
         *
         * \lang english
         *
         * @brief Deleted copy constructor
         */
        null_stream(const null_stream &) = delete;

        /**
         * \lang chinese
         *
         * @brief 删除拷贝赋值运算符
         * @return 空流对象的引用
         *
         * \lang english
         *
         * @brief Deleted copy assignment operator
         * @return Reference to the null stream object
         */
        null_stream &operator=(const null_stream &) = delete;

        /**
         * \lang chinese
         *
         * @brief 删除移动构造函数
         *
         * \lang english
         *
         * @brief Deleted move constructor
         */
        null_stream(null_stream &&) = delete;

        /**
         * \lang chinese
         *
         * @brief 删除移动赋值运算符
         * @return 空流对象的引用
         *
         * \lang english
         *
         * @brief Deleted move assignment operator
         * @return Reference to the null stream object
         */
        null_stream &operator=(null_stream &&) = delete;

        /**
         * \lang chinese
         *
         * @brief 获取执行器
         * @return 执行器对象
         *
         * \lang english
         *
         * @brief Get the executor
         * @return Executor object
         */
        RAINY_NODISCARD executor_type get_executor() const noexcept;

        /**
         * \lang chinese
         *
         * @brief 从空流读取数据（抛出异常版本）
         * @tparam MutableBufferSequence 可变缓冲区序列类型
         * @param buffers 用于接收数据的缓冲区序列
         * @return 实际读取的字节数（始终为0）
         * @throws exceptions::runtime::system_error 如果发生错误
         *
         * \lang english
         *
         * @brief Read data from null stream (exception throwing version)
         * @tparam MutableBufferSequence Mutable buffer sequence type
         * @param buffers Buffer sequence for receiving data
         * @return Number of bytes actually read (always 0)
         * @throws exceptions::runtime::system_error if an error occurs
         */
        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers) {
            std::error_code ec;
            auto result = read_some(buffers, ec);
            if (ec) {
                throw exceptions::runtime::system_error(ec);
            }
            return result;
        }

        /**
         * \lang chinese
         *
         * @brief 从空流读取数据（错误码版本）
         * @tparam MutableBufferSequence 可变缓冲区序列类型
         * @param buffers 用于接收数据的缓冲区序列
         * @param ec 错误码引用
         * @return 实际读取的字节数（始终为0）
         *
         * \lang english
         *
         * @brief Read data from null stream (error code version)
         * @tparam MutableBufferSequence Mutable buffer sequence type
         * @param buffers Buffer sequence for receiving data
         * @param ec Error code reference
         * @return Number of bytes actually read (always 0)
         */
        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence &buffers, std::error_code &ec) {
            auto mb = io::buffer(buffers);
            auto r = this->impl_->read_some(mb.data(), mb.size(), ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        /**
         * \lang chinese
         *
         * @brief 向空流写入数据（抛出异常版本）
         * @tparam ConstBufferSequence 常量缓冲区序列类型
         * @param buffers 要写入的数据缓冲区序列
         * @return 实际写入的字节数（等于缓冲区总大小）
         * @throws exceptions::runtime::system_error 如果发生错误
         *
         * \lang english
         *
         * @brief Write data to null stream (exception throwing version)
         * @tparam ConstBufferSequence Constant buffer sequence type
         * @param buffers Buffer sequence containing data to write
         * @return Number of bytes actually written (equal to total buffer size)
         * @throws exceptions::runtime::system_error if an error occurs
         */
        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers) {
            std::error_code ec;
            auto result = write_some(buffers, ec);
            if (ec) {
                throw exceptions::runtime::system_error(ec);
            }
            return result;
        }

        /**
         * \lang chinese
         *
         * @brief 向空流写入数据（错误码版本）
         * @tparam ConstBufferSequence 常量缓冲区序列类型
         * @param buffers 要写入的数据缓冲区序列
         * @param ec 错误码引用
         * @return 实际写入的字节数（等于缓冲区总大小）
         *
         * \lang english
         *
         * @brief Write data to null stream (error code version)
         * @tparam ConstBufferSequence Constant buffer sequence type
         * @param buffers Buffer sequence containing data to write
         * @param ec Error code reference
         * @return Number of bytes actually written (equal to total buffer size)
         */
        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence &buffers, std::error_code &ec) {
            auto cb = io::buffer(buffers);
            auto r = this->impl_->write_some(cb.data(), cb.size(), ec);
            return r < 0 ? 0 : static_cast<std::size_t>(r);
        }

        /**
         * \lang chinese
         *
         * @brief 异步从空流读取数据
         * @tparam MutableBufferSequence 可变缓冲区序列类型
         * @tparam CompletionToken 完成标记类型
         * @param buffers 用于接收数据的缓冲区序列
         * @param token 完成标记
         * @return 异步操作结果
         *
         * \lang english
         *
         * @brief Asynchronously read data from null stream
         * @tparam MutableBufferSequence Mutable buffer sequence type
         * @tparam CompletionToken Completion token type
         * @param buffers Buffer sequence for receiving data
         * @param token Completion token
         * @return Asynchronous operation result
         */
        template <typename MutableBufferSequence, typename CompletionToken>
        rain_fn async_read_some(MutableBufferSequence buffers, CompletionToken &&token) ->
            typename async_result<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type { // NOLINT
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::buffer(buffers);
            auto *op = io::implements::make_executor_completion_op(
                [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                });
            this->impl_->async_read_some(mb.data(), mb.size(), this->executor_, op);
            return init.result.get();
        }

        /**
         * \lang chinese
         *
         * @brief 异步向空流写入数据
         * @tparam ConstBufferSequence 常量缓冲区序列类型
         * @tparam CompletionToken 完成标记类型
         * @param buffers 要写入的数据缓冲区序列
         * @param token 完成标记
         * @return 异步操作结果
         *
         * \lang english
         *
         * @brief Asynchronously write data to null stream
         * @tparam ConstBufferSequence Constant buffer sequence type
         * @tparam CompletionToken Completion token type
         * @param buffers Buffer sequence containing data to write
         * @param token Completion token
         * @return Asynchronous operation result
         */
        template <typename ConstBufferSequence, typename CompletionToken>
        rain_fn async_write_some(ConstBufferSequence buffers, CompletionToken &&token) {
            using token_t = std::decay_t<CompletionToken>;
            async_completion<token_t, void(std::error_code, std::size_t)> init(token);
            auto handler = utility::move(init.completion_handler);
            auto mb = io::buffer(buffers);
            auto *op = io::implements::make_executor_completion_op(
                [handler](const io::implements::op_result &r, const bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (r.error_code) {
                        ec = std::error_code{r.error_code, std::system_category()};
                    }
                    handler(ec, r.bytes_transferred);
                });
            this->impl_->async_write_some(mb.data(), mb.size(), this->executor_, op);
            return init.result.get();
        }

        /**
         * \lang chinese
         *
         * @brief 检查流是否已打开
         * @return 始终返回true
         *
         * \lang english
         *
         * @brief Check if the stream is open
         * @return Always returns true
         */
        RAINY_NODISCARD bool is_open() const noexcept;

        /**
         * \lang chinese
         *
         * @brief 获取原生句柄
         * @return 无效的原生句柄值
         *
         * \lang english
         *
         * @brief Get the native handle
         * @return Invalid native handle value
         */
        RAINY_NODISCARD native_handle_type native_handle() const noexcept;

    private:
        explicit null_stream(executor_type ex);

        static io_context &default_context();

        executor_type executor_;
        memory::nebula_ptr<implements::descriptor_impl_base> impl_;
    };
}

#endif