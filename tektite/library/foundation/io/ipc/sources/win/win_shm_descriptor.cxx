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

#include <rainy/core/text/string.hpp>
#include <rainy/foundation/io/ipc/implements/shared_memory_impl.hpp>
#include <windows.h>

namespace rainy::foundation::io::ipc::shared_memory::implements {
    using io::implements::op_result;

    static std::error_code last_error() noexcept {
        return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
    }

    static auto to_wide(const char *name) {
        int n = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        rainy::core::text::wstring wname;
        wname.resize(static_cast<rainy::core::text::wstring::size_type>(n - 1));
        ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wname.data(), n);
        return wname;
    }

    static DWORD access_for_mode(access_mode mode) {
        switch (mode) {
            case access_mode::read_only:
                return FILE_MAP_READ;
            case access_mode::read_write:
                return FILE_MAP_ALL_ACCESS;
        }
        return FILE_MAP_ALL_ACCESS;
    }

    static DWORD protect_for_mode(access_mode mode) {
        switch (mode) {
            case access_mode::read_only:
                return PAGE_READONLY;
            case access_mode::read_write:
                return PAGE_READWRITE;
        }
        return PAGE_READWRITE;
    }

    class win_shared_memory_impl final : public shared_memory_impl_base {
    public:
        win_shared_memory_impl(HANDLE mapping, void *mapped, std::size_t size, access_mode mode) noexcept :
            mapping_(mapping), mapped_(mapped), size_(size), mode_(mode) {
        }

        ~win_shared_memory_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return mapped_ != nullptr;
        }

        std::error_code close() noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            ::UnmapViewOfFile(mapped_);
            ::CloseHandle(mapping_);
            mapped_ = nullptr;
            mapping_ = INVALID_HANDLE_VALUE;
            size_ = 0;
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(mapping_);
        }

        void *data() const noexcept override {
            return mapped_;
        }

        std::size_t size() const noexcept override {
            return size_;
        }

        std::error_code flush() noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            if (!::FlushViewOfFile(mapped_, 0)) {
                return last_error();
            }
            return {};
        }

        std::error_code flush(std::size_t offset, std::size_t length) noexcept override {
            if (!is_open()) {
                return std::error_code{ERROR_INVALID_HANDLE, std::system_category()};
            }
            if (offset + length > size_) {
                return std::error_code{ERROR_INVALID_PARAMETER, std::system_category()};
            }
            auto *base = static_cast<unsigned char *>(mapped_) + offset;
            if (!::FlushViewOfFile(base, length)) {
                return last_error();
            }
            return {};
        }

    private:
        HANDLE mapping_{INVALID_HANDLE_VALUE};
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
            return reinterpret_cast<native_handle_type>(INVALID_HANDLE_VALUE);
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

    core::memory::nebula_ptr<shared_memory_impl_base> create_shared_memory_impl(executor_type executor, const char *name,
                                                                                const attributes &attr, std::error_code &ec) {
        auto wname = to_wide(name);
        DWORD size_high = static_cast<DWORD>(attr.size >> 32);
        DWORD size_low = static_cast<DWORD>(attr.size);
        HANDLE mapping =
            ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, protect_for_mode(attr.mode), size_high, size_low, wname.c_str());
        if (!mapping) {
            ec = last_error();
            return create_null_shared_memory_impl(executor);
        }
        if (::GetLastError() == ERROR_ALREADY_EXISTS) {
            ::CloseHandle(mapping);
            ec = std::error_code{ERROR_ALREADY_EXISTS, std::system_category()};
            return create_null_shared_memory_impl(executor);
        }
        void *mapped = ::MapViewOfFile(mapping, access_for_mode(attr.mode), 0, 0, attr.size);
        if (!mapped) {
            ec = last_error();
            ::CloseHandle(mapping);
            return create_null_shared_memory_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<win_shared_memory_impl>(mapping, mapped, attr.size, attr.mode);
    }

    core::memory::nebula_ptr<shared_memory_impl_base> open_shared_memory_impl(executor_type executor, const char *name,
                                                                              access_mode mode, std::error_code &ec) {
        auto wname = to_wide(name);
        HANDLE mapping = ::OpenFileMappingW(access_for_mode(mode), FALSE, wname.c_str());
        if (!mapping) {
            ec = last_error();
            return create_null_shared_memory_impl(executor);
        }
        void *mapped = ::MapViewOfFile(mapping, access_for_mode(mode), 0, 0, 0);
        if (!mapped) {
            ec = last_error();
            ::CloseHandle(mapping);
            return create_null_shared_memory_impl(executor);
        }
        MEMORY_BASIC_INFORMATION mbi;
        ::VirtualQuery(mapped, &mbi, sizeof(mbi));
        auto size = static_cast<std::size_t>(mbi.RegionSize);
        ec.clear();
        return core::memory::make_nebula<win_shared_memory_impl>(mapping, mapped, size, mode);
    }

    core::memory::nebula_ptr<shared_memory_impl_base> open_or_create_shared_memory_impl(executor_type executor, const char *name,
                                                                                        const attributes &attr, std::error_code &ec) {
        auto wname = to_wide(name);
        HANDLE mapping = ::OpenFileMappingW(access_for_mode(attr.mode), FALSE, wname.c_str());
        if (mapping) {
            void *mapped = ::MapViewOfFile(mapping, access_for_mode(attr.mode), 0, 0, 0);
            if (!mapped) {
                ec = last_error();
                ::CloseHandle(mapping);
                return create_null_shared_memory_impl(executor);
            }
            MEMORY_BASIC_INFORMATION mbi;
            ::VirtualQuery(mapped, &mbi, sizeof(mbi));
            auto size = static_cast<std::size_t>(mbi.RegionSize);
            ec.clear();
            return core::memory::make_nebula<win_shared_memory_impl>(mapping, mapped, size, attr.mode);
        }
        DWORD size_high = static_cast<DWORD>(attr.size >> 32);
        DWORD size_low = static_cast<DWORD>(attr.size);
        mapping = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, protect_for_mode(attr.mode), size_high, size_low, wname.c_str());
        if (!mapping) {
            ec = last_error();
            return create_null_shared_memory_impl(executor);
        }
        void *mapped = ::MapViewOfFile(mapping, access_for_mode(attr.mode), 0, 0, attr.size);
        if (!mapped) {
            ec = last_error();
            ::CloseHandle(mapping);
            return create_null_shared_memory_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<win_shared_memory_impl>(mapping, mapped, attr.size, attr.mode);
    }

    core::memory::nebula_ptr<shared_memory_impl_base> create_null_shared_memory_impl(executor_type executor) {
        return core::memory::make_nebula<null_shared_memory_impl>(executor);
    }

    std::error_code unlink_shared_memory(const char *name) noexcept {
        return {};
    }
}
