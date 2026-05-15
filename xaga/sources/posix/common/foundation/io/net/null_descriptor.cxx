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
#include <rainy/foundation/io/implements/io_context.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>

namespace rainy::foundation::io::stream::implements {
    static std::error_code posix_error(const int e = errno) noexcept { // NOLINT
        return std::error_code{e, std::system_category()};
    }

    class null_descriptor_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit null_descriptor_impl(executor_type executor) noexcept : executor_(utility::move(executor)) {
        }

        RAINY_NODISCARD bool is_open() const noexcept override {
            return true;
        }

        std::error_code close() noexcept override {
            return {};
        }

        std::error_code cancel() noexcept override {
            return {};
        }

        RAINY_NODISCARD native_handle_type native_handle() const noexcept override {
            return -1;
        }

        std::error_code attach(native_handle_type) noexcept override {
            return posix_error(ENOTSUP);
        }

        native_handle_type release() noexcept override {
            return -1;
        }

        std::error_code attach_from(descriptor_impl_base *) noexcept override {
            return posix_error(ENOTSUP);
        }

        std::ptrdiff_t read_some(void *, std::size_t, std::error_code &ec) noexcept override {
            ec.clear();
            return 0; // EOF
        }

        std::ptrdiff_t write_some(const void *, const std::size_t len, std::error_code &ec) noexcept override {
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        void async_read_some(void *, std::size_t, executor_type executor, completion_op *op) noexcept override {
            io::implements::op_result result{};
            result.user_data = op;
            result.error_code = 0;
            result.bytes_transferred = 0;
            op->complete(result, false);
        }

        void async_write_some(const void *, const std::size_t len, executor_type executor, completion_op *op) noexcept override {
            io::implements::op_result result{};
            result.user_data = op;
            result.error_code = 0;
            result.bytes_transferred = len;
            op->complete(result, false);
        }

        RAINY_NODISCARD bool wants_read() const noexcept override {
            return false;
        }

        RAINY_NODISCARD bool wants_write() const noexcept override {
            return false;
        }

        void reset_operation() noexcept override {
        }

    private:
        executor_type executor_;
    };

    memory::nebula_ptr<descriptor_impl_base> create_null_impl(executor_type executor) {
        return memory::make_nebula<null_descriptor_impl>(executor);
    }
}
