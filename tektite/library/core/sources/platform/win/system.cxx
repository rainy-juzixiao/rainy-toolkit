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
#include <rainy/core/platform.hpp>
#include <rainy/core/layer.hpp>

#include <windows.h>
#include <lmcons.h>

namespace rainy::core::layer {
    rain_fn query_memory_capacity(std::size_t *total, std::size_t *available) noexcept -> bool {
        if (!total || !available) {
            return false;
        }
        MEMORYSTATUSEX status{};
        status.dwLength = sizeof(status);
        if (!::GlobalMemoryStatusEx(&status)) {
            return false;
        }
        *total = static_cast<std::size_t>(status.ullTotalPhys);
        *available = static_cast<std::size_t>(status.ullAvailPhys);
        return true;
    }

    rain_fn query_environment(const char *name, char *buffer, std::size_t length) noexcept -> bool {
        if (!name || !buffer || length == 0 || length > 0xFFFFFFFFu) {
            return false;
        }
        const DWORD size = ::GetEnvironmentVariableA(name, buffer, static_cast<DWORD>(length));
        return size > 0 && size < length;
    }

    rain_fn set_environment(const char *name, const char *value) noexcept -> bool {
        if (!name || !value) {
            return false;
        }
        return ::SetEnvironmentVariableA(name, value) != 0;
    }

    rain_fn remove_environment(const char *name) noexcept -> bool {
        if (!name) {
            return false;
        }
        return ::SetEnvironmentVariableA(name, nullptr) != 0;
    }

    rain_fn query_user_name(char *buffer, std::size_t length) noexcept -> bool {
        if (!buffer || length < UNLEN + 1) {
            return false;
        }
        DWORD size = static_cast<DWORD>(length);
        return ::GetUserNameA(buffer, &size) != 0;
    }

    rain_fn current_group_id() noexcept -> std::uint32_t {
        // Windows 没有与 POSIX 等价的数字组id概念。
        return 0;
    }

    rain_fn query_group_name(std::uint32_t, char *, std::size_t) noexcept -> bool {
        // Windows 没有与 POSIX 等价的数字组id概念。
        return false;
    }

    rain_fn system_random(void *buffer, std::size_t length) noexcept -> bool {
        if (!buffer || length == 0 || length > 0xFFFFFFFFu) {
            return false;
        }
        // RtlGenRandom（SystemFunction036）是系统自带的加密随机源。
        using rtl_gen_random_fn = BOOLEAN (__stdcall *)(void *, ULONG);
        const auto procedure = ::GetProcAddress(::GetModuleHandleA("advapi32.dll"), "SystemFunction036");
        if (!procedure) {
            return false;
        }
        const auto rtl_gen_random = reinterpret_cast<rtl_gen_random_fn>(procedure);
        return rtl_gen_random(buffer, static_cast<ULONG>(length)) != 0;
    }
}
