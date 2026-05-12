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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <rainy/foundation/concurrency/executor.hpp>
#include <rainy/foundation/io/implements/io_context.hpp>
#include <rainy/foundation/io/stream/implements/descriptor.hpp>
#include <windows.h>

namespace rainy::foundation::io::stream::implements {
    using io::implements::op_result;

    static std::error_code last_error() noexcept {
        return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
    }

    struct iocp_op : io::implements::completion_op {
        OVERLAPPED overlapped{};
        HANDLE associated_handle{INVALID_HANDLE_VALUE};
        DWORD transferred{0};
        io::implements::completion_op *linked_op{nullptr};

        explicit iocp_op(fn_type f) noexcept : completion_op(f) {
        }

        static iocp_op *from_overlapped(OVERLAPPED *ov) noexcept {
            return reinterpret_cast<iocp_op *>(reinterpret_cast<char *>(ov) - offsetof(iocp_op, overlapped));
        }
    };

    struct stream_iocp_op : iocp_op {
        explicit stream_iocp_op(io::implements::completion_op *op) noexcept : iocp_op(nullptr) {
            linked_op = op;
        }
    };

    struct win32_stream_proxy : io_context::executor_type {
        using executor_type::associate_handle;
        using executor_type::post_immediate_completion;
    };

    static concurrency::executor &get_executor() noexcept {
        return concurrency::get_global_pooled_executor();
    }

    class win_descriptor_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit win_descriptor_impl(executor_type executor) noexcept : executor_(executor) {
        }

        ~win_descriptor_impl() override {
            if (is_open()) {
                close();
            }
        }

        bool is_open() const noexcept override {
            return handle_ != INVALID_HANDLE_VALUE;
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(handle_);
        }

        std::error_code attach(native_handle_type handle) noexcept override {
            if (is_open()) {
                return std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            }
            handle_ = reinterpret_cast<HANDLE>(handle);
            bound_to_iocp_ = false;
            return {};
        }

        native_handle_type release() noexcept override {
            HANDLE h = handle_;
            handle_ = INVALID_HANDLE_VALUE;
            bound_to_iocp_ = false;
            wants_read_ = false;
            wants_write_ = false;
            return reinterpret_cast<native_handle_type>(h);
        }

        std::error_code attach_from(descriptor_impl_base *other) noexcept override {
            if (!other) {
                return std::error_code{ERROR_INVALID_PARAMETER, std::system_category()};
            }
            if (is_open()) {
                return std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            }
            return attach(other->release());
        }

        std::error_code close() noexcept override {
            if (handle_ == INVALID_HANDLE_VALUE) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            ::CancelIo(handle_);
            ::CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
            bound_to_iocp_ = false;
            wants_read_ = false;
            wants_write_ = false;
            return {};
        }

        std::error_code cancel() noexcept override {
            if (handle_ == INVALID_HANDLE_VALUE) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            if (!::CancelIo(handle_)) {
                return last_error();
            }
            return {};
        }

        std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!is_open()) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            OVERLAPPED ov{};
            HANDLE ev = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!ev) {
                ec = last_error();
                return -1;
            }
            ov.hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<std::uintptr_t>(ev) | 1);

            DWORD read = 0;
            BOOL ok = ::ReadFile(handle_, buf, static_cast<DWORD>(len), &read, &ov);
            if (!ok) {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING) {
                    ok = ::GetOverlappedResult(handle_, &ov, &read, TRUE);
                    if (!ok)
                        err = ::GetLastError();
                }
                if (!ok) {
                    ::CloseHandle(ev);
                    if (err == ERROR_BROKEN_PIPE || err == ERROR_HANDLE_EOF) {
                        ec.clear();
                        return 0;
                    }
                    ec.assign(static_cast<int>(err), std::system_category());
                    return -1;
                }
            }
            ::CloseHandle(ev);
            ec.clear();
            return static_cast<std::ptrdiff_t>(read);
        }

        std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!is_open()) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            OVERLAPPED ov{};
            HANDLE ev = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!ev) {
                ec = last_error();
                return -1;
            }
            ov.hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<std::uintptr_t>(ev) | 1);

            DWORD written = 0;
            BOOL ok = ::WriteFile(handle_, buf, static_cast<DWORD>(len), &written, &ov);
            if (!ok) {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING) {
                    ok = ::GetOverlappedResult(handle_, &ov, &written, TRUE);
                    if (!ok)
                        err = ::GetLastError();
                }
                if (!ok) {
                    ::CloseHandle(ev);
                    ec.assign(static_cast<int>(err), std::system_category());
                    return -1;
                }
            }
            ::CloseHandle(ev);
            ec.clear();
            return static_cast<std::ptrdiff_t>(written);
        }

        void async_read_some(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (!is_open()) {
                win32_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            bind_to_iocp(executor);
            wants_read_ = true;
            auto *iop = new stream_iocp_op(op);
            iop->associated_handle = handle_;
            BOOL ok = ::ReadFile(handle_, buf, static_cast<DWORD>(len), nullptr, &iop->overlapped);
            if (!ok && ::GetLastError() != ERROR_IO_PENDING) {
                wants_read_ = false;
                delete iop;
                win32_stream_proxy{executor}.post_immediate_completion(op, false);
            }
        }

        void async_write_some(const void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            if (!is_open()) {
                win32_stream_proxy{executor}.post_immediate_completion(op, false);
                return;
            }
            bind_to_iocp(executor);
            wants_write_ = true;

            auto *iop = new stream_iocp_op(op);
            iop->associated_handle = handle_;

            BOOL ok = ::WriteFile(handle_, buf, static_cast<DWORD>(len), nullptr, &iop->overlapped);
            if (!ok && ::GetLastError() != ERROR_IO_PENDING) {
                wants_write_ = false;
                delete iop;
                win32_stream_proxy{executor}.post_immediate_completion(op, false);
            }
        }

        bool wants_read() const noexcept override {
            return wants_read_;
        }
        bool wants_write() const noexcept override {
            return wants_write_;
        }
        void reset_operation() noexcept override {
            wants_read_ = false;
            wants_write_ = false;
        }

    private:
        void bind_to_iocp(executor_type executor) noexcept {
            if (!bound_to_iocp_) {
                win32_stream_proxy{executor}.associate_handle(nullptr, reinterpret_cast<std::uintptr_t>(handle_), nullptr);
                bound_to_iocp_ = true;
            }
        }

        HANDLE handle_{INVALID_HANDLE_VALUE};
        executor_type executor_;
        bool bound_to_iocp_{false};
        bool wants_read_{false};
        bool wants_write_{false};
    };

    class anon_pipe_descriptor_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit anon_pipe_descriptor_impl(executor_type executor) noexcept : executor_(executor) {
        }

        ~anon_pipe_descriptor_impl() override {
            if (is_open()) {
                close();
            }
        }

        bool is_open() const noexcept override {
            return handle_ != INVALID_HANDLE_VALUE;
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(handle_);
        }

        std::error_code attach(native_handle_type handle) noexcept override {
            if (is_open()) {
                return std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            }
            handle_ = reinterpret_cast<HANDLE>(handle);
            return {};
        }

        native_handle_type release() noexcept override {
            HANDLE h = handle_;
            handle_ = INVALID_HANDLE_VALUE;
            wants_read_ = false;
            wants_write_ = false;
            return reinterpret_cast<native_handle_type>(h);
        }

        std::error_code attach_from(descriptor_impl_base *other) noexcept override {
            if (!other) {
                return std::error_code{ERROR_INVALID_PARAMETER, std::system_category()};
            }
            if (is_open()) {
                return std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            }
            return attach(other->release());
        }

        std::error_code close() noexcept override {
            if (handle_ == INVALID_HANDLE_VALUE) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            cancel_flag_.store(true, std::memory_order_release);
            ::CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
            wants_read_ = false;
            wants_write_ = false;
            return {};
        }

        std::error_code cancel() noexcept override {
            cancel_flag_.store(true, std::memory_order_release);
            return {};
        }

        std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!is_open()) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            DWORD read = 0;
            // 匿名管道是同步句柄，直接阻塞读
            BOOL ok = ::ReadFile(handle_, buf, static_cast<DWORD>(len), &read, nullptr);
            if (!ok) {
                DWORD err = ::GetLastError();
                if (err == ERROR_BROKEN_PIPE) {
                    // 写端已关闭 → EOF
                    ec.clear();
                    return 0;
                }
                ec.assign(static_cast<int>(err), std::system_category());
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(read);
        }

        std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!is_open()) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            DWORD written = 0;
            BOOL ok = ::WriteFile(handle_, buf, static_cast<DWORD>(len), &written, nullptr);
            if (!ok) {
                ec.assign(static_cast<int>(::GetLastError()), std::system_category());
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(written);
        }

        void async_read_some(void *buf, std::size_t len, executor_type /*executor*/, completion_op *op) noexcept override {
            if (!is_open()) {
                op_result result{};
                result.user_data = op;
                result.error_code = ERROR_INVALID_HANDLE;
                result.bytes_transferred = 0;
                op->complete(result, false);
                return;
            }
            wants_read_ = true;
            HANDLE h = handle_;
            std::atomic<bool> *flag = &cancel_flag_;

            get_executor().submit([h, buf, len, op, flag]() mutable {
                op_result result{};
                result.user_data = op;

                if (flag->load(std::memory_order_acquire)) {
                    result.error_code = ERROR_OPERATION_ABORTED;
                    result.bytes_transferred = 0;
                    op->complete(result, false);
                    return;
                }

                DWORD read = 0;
                BOOL ok = ::ReadFile(h, buf, static_cast<DWORD>(len), &read, nullptr);
                if (!ok) {
                    DWORD err = ::GetLastError();
                    if (err == ERROR_BROKEN_PIPE) {
                        // EOF：写端已关闭
                        result.error_code = 0;
                        result.bytes_transferred = 0;
                    } else {
                        result.error_code = static_cast<int>(err);
                        result.bytes_transferred = 0;
                    }
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(read);
                }
                op->complete(result, false);
            });
        }

        void async_write_some(const void *buf, std::size_t len, executor_type /*executor*/, completion_op *op) noexcept override {
            if (!is_open()) {
                op_result result{};
                result.user_data = op;
                result.error_code = ERROR_INVALID_HANDLE;
                result.bytes_transferred = 0;
                op->complete(result, false);
                return;
            }
            wants_write_ = true;
            HANDLE h = handle_;
            std::atomic<bool> *flag = &cancel_flag_;

            get_executor().submit([h, buf, len, op, flag]() mutable {
                op_result result{};
                result.user_data = op;

                if (flag->load(std::memory_order_acquire)) {
                    result.error_code = ERROR_OPERATION_ABORTED;
                    result.bytes_transferred = 0;
                    op->complete(result, false);
                    return;
                }

                DWORD written = 0;
                BOOL ok = ::WriteFile(h, buf, static_cast<DWORD>(len), &written, nullptr);
                if (!ok) {
                    result.error_code = static_cast<int>(::GetLastError());
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(written);
                }
                op->complete(result, false);
            });
        }

        bool wants_read() const noexcept override {
            return wants_read_;
        }
        bool wants_write() const noexcept override {
            return wants_write_;
        }

        void reset_operation() noexcept override {
            wants_read_ = false;
            wants_write_ = false;
            cancel_flag_.store(false, std::memory_order_release);
        }

    private:
        HANDLE handle_{INVALID_HANDLE_VALUE};
        executor_type executor_;
        std::atomic<bool> cancel_flag_{false};
        bool wants_read_{false};
        bool wants_write_{false};
    };

    class null_descriptor_impl final : public descriptor_impl_base {
    public:
        using completion_op = io::implements::completion_op;

        explicit null_descriptor_impl(executor_type executor) noexcept : executor_(executor) {
        }

        bool is_open() const noexcept override {
            return true;
        }

        std::error_code close() noexcept override {
            return {};
        }
        std::error_code cancel() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(INVALID_HANDLE_VALUE);
        }

        std::error_code attach(native_handle_type) noexcept override {
            return std::error_code{ERROR_NOT_SUPPORTED, std::system_category()};
        }

        native_handle_type release() noexcept override {
            return reinterpret_cast<native_handle_type>(INVALID_HANDLE_VALUE);
        }

        std::error_code attach_from(descriptor_impl_base *) noexcept override {
            return std::error_code{ERROR_NOT_SUPPORTED, std::system_category()};
        }

        std::ptrdiff_t read_some(void *, std::size_t, std::error_code &ec) noexcept override {
            ec.clear();
            return 0;
        }

        std::ptrdiff_t write_some(const void *, std::size_t len, std::error_code &ec) noexcept override {
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        void async_read_some(void *, std::size_t, executor_type executor, completion_op *op) noexcept override {
            win32_stream_proxy{executor}.post_immediate_completion(op, false);
        }

        void async_write_some(const void *, std::size_t, executor_type executor, completion_op *op) noexcept override {
            win32_stream_proxy{executor}.post_immediate_completion(op, false);
        }

        bool wants_read() const noexcept override {
            return false;
        }
        bool wants_write() const noexcept override {
            return false;
        }
        void reset_operation() noexcept override {
        }

    private:
        executor_type executor_;
    };

    memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl(executor_type executor) {
        return memory::make_nebula<win_descriptor_impl>(executor);
    }

    memory::nebula_ptr<descriptor_impl_base> create_descriptor_impl_from_native(executor_type executor, native_handle_type handle) {
        auto impl = memory::make_nebula<win_descriptor_impl>(executor);
        static_cast<void>(impl->attach(handle));
        return impl;
    }

    memory::nebula_ptr<descriptor_impl_base> create_console_impl(executor_type executor, console_stream_kind kind) {
        DWORD std_id{};
        switch (kind) {
            case console_stream_kind::input:
                std_id = STD_INPUT_HANDLE;
                break;
            case console_stream_kind::output:
                std_id = STD_OUTPUT_HANDLE;
                break;
            case console_stream_kind::error:
                std_id = STD_ERROR_HANDLE;
                break;
            default:
                return memory::make_nebula<null_descriptor_impl>(executor);
        }
        HANDLE h = ::GetStdHandle(std_id);
        auto impl = memory::make_nebula<win_descriptor_impl>(executor);
        if (h && h != INVALID_HANDLE_VALUE) {
            static_cast<void>(impl->attach(reinterpret_cast<native_handle_type>(h)));
        }
        return impl;
    }

    utility::pair<memory::nebula_ptr<descriptor_impl_base>, memory::nebula_ptr<descriptor_impl_base>> create_pipe_impl(
        executor_type executor, std::error_code &ec) {
        HANDLE read_h = INVALID_HANDLE_VALUE;
        HANDLE write_h = INVALID_HANDLE_VALUE;

        // 安全属性：允许句柄继承（方便子进程场景）
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        if (!::CreatePipe(&read_h, &write_h, &sa, 0)) {
            ec = last_error();
            return {};
        }

        auto read_impl = memory::make_nebula<anon_pipe_descriptor_impl>(executor);
        auto write_impl = memory::make_nebula<anon_pipe_descriptor_impl>(executor);

        static_cast<void>(read_impl->attach(reinterpret_cast<native_handle_type>(read_h)));
        static_cast<void>(write_impl->attach(reinterpret_cast<native_handle_type>(write_h)));

        ec.clear();
        return {std::move(read_impl), std::move(write_impl)};
    }

    memory::nebula_ptr<descriptor_impl_base> create_named_pipe_server_impl(executor_type executor, const char *name,
                                                                           pipe_direction dir, std::error_code &ec) {
        std::wstring wide_name;
        {
            int n = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
            wide_name.resize(n - 1);
            ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name.data(), n);
        }
        if (wide_name.find(L"\\\\.\\pipe\\") == std::wstring::npos) {
            wide_name = L"\\\\.\\pipe\\" + wide_name;
        }

        DWORD access{};
        switch (dir) {
            case pipe_direction::in:
                access = PIPE_ACCESS_INBOUND;
                break;
            case pipe_direction::out:
                access = PIPE_ACCESS_OUTBOUND;
                break;
            case pipe_direction::inout:
                access = PIPE_ACCESS_DUPLEX;
                break;
        }

        HANDLE h =
            ::CreateNamedPipeW(wide_name.c_str(), access | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                               PIPE_UNLIMITED_INSTANCES, 65536, 65536, 0, nullptr);

        if (h == INVALID_HANDLE_VALUE) {
            ec = last_error();
            return memory::make_nebula<null_descriptor_impl>(executor);
        }
        OVERLAPPED ov{};
        HANDLE ev = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!ev) {
            ec = last_error();
            ::CloseHandle(h);
            return memory::make_nebula<null_descriptor_impl>(executor);
        }
        ov.hEvent = ev;
        BOOL ok = ::ConnectNamedPipe(h, &ov);
        if (!ok) {
            DWORD err = ::GetLastError();
            if (err == ERROR_IO_PENDING) {
                ::WaitForSingleObject(ev, INFINITE);
            } else if (err != ERROR_PIPE_CONNECTED) {
                ec.assign(static_cast<int>(err), std::system_category());
                ::CloseHandle(ev);
                ::CloseHandle(h);
                return memory::make_nebula<null_descriptor_impl>(executor);
            }
        }
        ::CloseHandle(ev);

        auto impl = memory::make_nebula<win_descriptor_impl>(executor);
        static_cast<void>(impl->attach(reinterpret_cast<native_handle_type>(h)));
        ec.clear();
        return impl;
    }

    memory::nebula_ptr<descriptor_impl_base> create_named_pipe_client_impl(executor_type executor, const char *name,
                                                                           pipe_direction dir, std::error_code &ec) {
        std::wstring wide_name;
        {
            int n = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
            wide_name.resize(n - 1);
            ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name.data(), n);
        }
        if (wide_name.find(L"\\\\.\\pipe\\") == std::wstring::npos) {
            wide_name = L"\\\\.\\pipe\\" + wide_name;
        }

        DWORD access{};
        switch (dir) {
            case pipe_direction::in:
                access = GENERIC_READ;
                break;
            case pipe_direction::out:
                access = GENERIC_WRITE;
                break;
            case pipe_direction::inout:
                access = GENERIC_READ | GENERIC_WRITE;
                break;
        }

        if (!::WaitNamedPipeW(wide_name.c_str(), 5000)) {
            ec = last_error();
            return memory::make_nebula<null_descriptor_impl>(executor);
        }

        HANDLE h = ::CreateFileW(wide_name.c_str(), access, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

        if (h == INVALID_HANDLE_VALUE) {
            ec = last_error();
            return memory::make_nebula<null_descriptor_impl>(executor);
        }

        auto impl = memory::make_nebula<win_descriptor_impl>(executor);
        static_cast<void>(impl->attach(reinterpret_cast<native_handle_type>(h)));
        ec.clear();
        return impl;
    }

    memory::nebula_ptr<descriptor_impl_base> create_null_impl(executor_type executor) {
        return memory::make_nebula<null_descriptor_impl>(executor);
    }
}
