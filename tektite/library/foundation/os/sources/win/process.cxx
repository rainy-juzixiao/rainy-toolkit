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
#include <rainy/foundation/os/process.hpp>

#include <windows.h>
#include <tlhelp32.h>
#include <cstring>

namespace rainy::foundation::os::implements {
    rain_fn current_process_id() noexcept -> std::uint64_t {
        return static_cast<std::uint64_t>(::GetCurrentProcessId());
    }

    rain_fn create_process(const core::text::string &exec, const core::collections::vector<core::text::string> &args)
        noexcept -> native_process {
        native_process result{};
        core::text::string command_line = exec;
        for (const auto &argument : args) {
            command_line += " \"";
            command_line += argument;
            command_line += "\"";
        }
        STARTUPINFOA startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION information{};
        if (!::CreateProcessA(nullptr, command_line.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup,
                              &information)) {
            return result;
        }
        ::CloseHandle(information.hThread);
        result.id = static_cast<std::uint64_t>(information.dwProcessId);
        result.handle = information.hProcess;
        return result;
    }

    rain_fn wait_process(native_process &process) noexcept -> int {
        if (process.id == 0 || !process.handle) {
            return -1;
        }
        if (::WaitForSingleObject(process.handle, INFINITE) != WAIT_OBJECT_0) {
            return -1;
        }
        DWORD code = 0;
        const bool ok = ::GetExitCodeProcess(process.handle, &code) != 0;
        ::CloseHandle(process.handle);
        process = native_process{};
        return ok ? static_cast<int>(code) : -1;
    }

    rain_fn terminate_process(native_process &process) noexcept -> bool {
        if (process.id == 0 || !process.handle) {
            return false;
        }
        return ::TerminateProcess(process.handle, 1) != 0;
    }

    rain_fn release_process(native_process &process) noexcept -> void {
        if (process.handle) {
            ::CloseHandle(process.handle);
        }
        process = native_process{};
    }

    rain_fn query_process_list() noexcept -> core::collections::vector<process_entry> {
        core::collections::vector<process_entry> entries;
        HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return entries;
        }
        PROCESSENTRY32 item{};
        item.dwSize = sizeof(item);
        if (::Process32First(snapshot, &item)) {
            do {
                process_entry entry{};
                entry.id = static_cast<std::uint64_t>(item.th32ProcessID);
                entry.name = core::text::string{item.szExeFile};
                entries.push_back(utility::move(entry));
            } while (::Process32Next(snapshot, &item));
        }
        ::CloseHandle(snapshot);
        return entries;
    }

    rain_fn current_priority() noexcept -> int {
        return static_cast<int>(::GetPriorityClass(::GetCurrentProcess()));
    }

    rain_fn set_current_priority(int priority) noexcept -> bool {
        return ::SetPriorityClass(::GetCurrentProcess(), static_cast<DWORD>(priority)) != 0;
    }
}
