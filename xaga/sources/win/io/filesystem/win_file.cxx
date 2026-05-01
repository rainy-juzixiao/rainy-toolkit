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
#include <rainy/foundation/io/filesystem/streamfile.hpp>
#include <rainy/foundation/io/implements/io_context.hpp>

#include <windows.h>

namespace rainy::foundation::io::filesystem::implements {
    struct iocp_op : completion_op {
        OVERLAPPED overlapped{};
        HANDLE associated_handle{INVALID_HANDLE_VALUE};
        DWORD transferred{0};
        completion_op *linked_op{nullptr};

        explicit iocp_op(fn_type f) noexcept : completion_op(f) {
        }

        static iocp_op *from_overlapped(OVERLAPPED *ov) noexcept {
            return reinterpret_cast<iocp_op *>(reinterpret_cast<char *>(ov) - offsetof(iocp_op, overlapped));
        }
    };

    struct file_iocp_op : iocp_op {
        explicit file_iocp_op(completion_op *op) noexcept : iocp_op(nullptr) {
            linked_op = op;
        }
    };

    struct win32_file_proxy : io_context::executor_type{
        using executor_type::post_immediate_completion;
        using executor_type::associate_handle;
    };

    static std::error_code last_error() noexcept {
        return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
    }

    class win_file_impl final : public file_impl_base {
    public:
        win_file_impl() = default;

        ~win_file_impl() override {
            if (is_open()) {
                close();
            }
        }

        std::error_code open(const std::filesystem::path &path, open_mode mode, io_context::executor_type executor) noexcept override {
            if (is_open()) {
                return std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            }
            DWORD access = 0;
            if (has_flag(mode, open_mode::read_only))
                access |= GENERIC_READ;
            if (has_flag(mode, open_mode::write_only))
                access |= GENERIC_WRITE;

            DWORD creation = OPEN_EXISTING;
            if (has_flag(mode, open_mode::create)) {
                creation = has_flag(mode, open_mode::exclusive) ? CREATE_NEW : OPEN_ALWAYS;
            }
            if (has_flag(mode, open_mode::truncate)) {
                creation = CREATE_ALWAYS;
            }

            DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
            if (has_flag(mode, open_mode::sync)) {
                flags |= FILE_FLAG_WRITE_THROUGH;
            }
            if (has_flag(mode, open_mode::direct)) {
                flags |= FILE_FLAG_NO_BUFFERING;
            }

            handle_ = ::CreateFileW(path.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, creation, flags, nullptr);

            if (handle_ == INVALID_HANDLE_VALUE) {
                return last_error();
            }
            return {};
        }

        void close() noexcept override {
            if (handle_ == INVALID_HANDLE_VALUE) {
                return;
            }
            BOOL ok = ::CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
            bound_to_iocp_ = false;
            ctx_ = nullptr;
        }

        bool is_open() const noexcept override {
            return handle_ != INVALID_HANDLE_VALUE;
        }

        std::size_t read_some_at(mutable_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept override {
            OVERLAPPED ov{};
            ov.Offset = static_cast<DWORD>(offset & 0xFFFF'FFFF);
            ov.OffsetHigh = static_cast<DWORD>(offset >> 32);
            HANDLE ev = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!ev) {
                ec = last_error();
                return 0;
            }
            ov.hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<std::uintptr_t>(ev) | 1);
            DWORD read = 0;
            BOOL ok = ::ReadFile(handle_, buf.data(), static_cast<DWORD>(buf.size()), &read, &ov);
            if (!ok) {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING) {
                    ok = ::GetOverlappedResult(handle_, &ov, &read, TRUE);
                    if (!ok)
                        err = ::GetLastError();
                }
                if (!ok) {
                    ::CloseHandle(ev);
                    if (err == ERROR_HANDLE_EOF) {
                        // EOF 不是错误，静默返回 0
                        ec.clear();
                        return 0;
                    }
                    ec.assign(static_cast<int>(err), std::system_category());
                    return 0;
                }
            }
            ::CloseHandle(ev);
            return static_cast<std::size_t>(read);
        }

        std::size_t write_some_at(const_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept override {
            OVERLAPPED ov{};
            ov.Offset = static_cast<DWORD>(offset & 0xFFFF'FFFF);
            ov.OffsetHigh = static_cast<DWORD>(offset >> 32);
            HANDLE ev = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!ev) {
                ec = last_error();
                return 0;
            }
            ov.hEvent = reinterpret_cast<HANDLE>(reinterpret_cast<std::uintptr_t>(ev) | 1);
            DWORD written = 0;
            BOOL ok = ::WriteFile(handle_, buf.data(), static_cast<DWORD>(buf.size()), &written, &ov);
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
                    return 0;
                }
            }
            ::CloseHandle(ev);
            return static_cast<std::size_t>(written);
        }

        void async_read_some_at(mutable_buffer buf, std::uint64_t offset, io_context::executor_type executor,
                                completion_op *op) noexcept override {
            bind_to_iocp(executor);
            auto *iop = new file_iocp_op(op);
            iop->overlapped.Offset = static_cast<DWORD>(offset & 0xFFFF'FFFF);
            iop->overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
            iop->associated_handle = handle_;
            BOOL ok = ::ReadFile(handle_, buf.data(), static_cast<DWORD>(buf.size()), nullptr, &iop->overlapped);
            if (!ok && ::GetLastError() != ERROR_IO_PENDING) {
                delete iop;
                win32_file_proxy{executor}.post_immediate_completion(op, false);
            }
        }

        void async_write_some_at(const_buffer buf, std::uint64_t offset, io_context::executor_type executor,
                                 completion_op *op) noexcept override {
            bind_to_iocp(executor);
            auto *iop = new file_iocp_op(op);
            iop->overlapped.Offset = static_cast<DWORD>(offset & 0xFFFF'FFFF);
            iop->overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
            iop->associated_handle = handle_;
            BOOL ok = ::WriteFile(handle_, buf.data(), static_cast<DWORD>(buf.size()), nullptr, &iop->overlapped);
            if (!ok && ::GetLastError() != ERROR_IO_PENDING) {
                delete iop;
                win32_file_proxy{executor}.post_immediate_completion(op, false);
            }
        }

        std::uint64_t size(std::error_code &ec) const noexcept override {
            LARGE_INTEGER li{};
            if (!::GetFileSizeEx(handle_, &li)) {
                ec = last_error();
                return 0;
            }
            return static_cast<std::uint64_t>(li.QuadPart);
        }

        std::error_code resize(std::uint64_t new_size) noexcept override {
            LARGE_INTEGER li{};
            li.QuadPart = static_cast<LONGLONG>(new_size);
            if (!::SetFilePointerEx(handle_, li, nullptr, FILE_BEGIN) || !::SetEndOfFile(handle_)) {
                return last_error();
            }
            return {};
        }

        std::uintptr_t native_handle() const noexcept override {
            return reinterpret_cast<std::uintptr_t>(handle_);
        }

    private:
        void bind_to_iocp(io_context::executor_type executor) noexcept {
            if (!bound_to_iocp_) {
                win32_file_proxy{executor}.associate_handle(nullptr, reinterpret_cast<std::uintptr_t>(handle_), nullptr);
                bound_to_iocp_ = true;
            }
        }

        HANDLE handle_{INVALID_HANDLE_VALUE};
        io_context_impl_base *ctx_{nullptr};
        bool bound_to_iocp_{false};
    };

    memory::nebula_ptr<file_impl_base> make_file_impl() {
        return memory::make_nebula<win_file_impl>();
    }
}
