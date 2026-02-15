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
#include <rainy/core/core.hpp>

#if RAINY_USING_WINDOWS
#include <rainy/foundation/pal/implements/tgc_layer_module_context.hpp>
#include <string>
#include <vector>
#include <windows.h>

namespace rainy::foundation::pal::module_context::implements {
    RAINY_TOOLKIT_LOCAL_API static bool is_absolute_path(const std::string_view file_name) {
        // Windows绝对路径：带盘符 (C:\...) 或 UNC 路径 (\\...)
        if (file_name.size() >= 3 && std::isalpha(static_cast<unsigned char>(file_name[0])) && file_name[1] == ':' &&
            (file_name[2] == '\\' || file_name[2] == '/')) {
            return true;
        }
        if (file_name.size() >= 2 && file_name[0] == '\\' && file_name[1] == '\\') {
            return true;
        }
        return false;
    }

    RAINY_TOOLKIT_LOCAL_API static bool file_exist(const std::wstring &file_name) {
        DWORD attr = GetFileAttributesW(file_name.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    using native_handle = HMODULE;

    RAINY_INLINE core::handle to_handle(native_handle h) noexcept {
        return reinterpret_cast<core::handle>(h);
    }

    RAINY_INLINE native_handle from_handle(core::handle h) noexcept {
        return reinterpret_cast<native_handle>(h);
    }
}

namespace rainy::foundation::pal::module_context::implements {
    core::handle load_module(const std::string_view module_path, const bool crossplatform) noexcept {
        core::handle hand = 0;
        if (crossplatform) {
            const std::vector<std::wstring> prefix_list = {L"", L"lib"};
            const std::vector<std::wstring> suffix_list = {L".dll"};
            // module_path 转宽字符，用于前后缀判断
            const std::wstring wide_path(module_path.begin(), module_path.end());
            rainy_let retry = true;
            std::wstring attempt;
            for (rainy_let prefix = 0u; retry && !hand && prefix < prefix_list.size(); ++prefix) {
                for (rainy_let suffix = 0u; retry && !hand && suffix < suffix_list.size(); ++suffix) {
                    // 已有对应前缀则跳过（与Linux逻辑对称）
                    if (!prefix_list[prefix].empty() && wide_path.starts_with(prefix_list[prefix])) {
                        continue;
                    }
                    // 已有对应后缀则跳过
                    if (!suffix_list[suffix].empty() && wide_path.ends_with(suffix_list[suffix])) {
                        continue;
                    }
                    attempt = prefix_list[prefix] + wide_path + suffix_list[suffix];
                    HMODULE mod = LoadLibraryExW(attempt.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
                    hand = to_handle(mod);
                    // 绝对路径下，文件存在但加载失败，停止重试（与Linux retry=false逻辑对称）
                    if (!hand && is_absolute_path(module_path) && file_exist(attempt)) {
                        retry = false;
                    }
                }
            }
        } else {
            const std::wstring wide_path(module_path.begin(), module_path.end());
            HMODULE mod = LoadLibraryExW(wide_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            hand = to_handle(mod);
        }
        return hand;
    }

    core::handle try_to_get_module(const std::string_view module_path, const bool crossplatform) noexcept {
        core::handle hand = 0;
        if (crossplatform) {
            const std::vector<std::wstring> prefix_list = {L"", L"lib"};
            const std::vector<std::wstring> suffix_list = {L".dll"};
            const std::wstring wide_path(module_path.begin(), module_path.end());
            rainy_let retry = true;
            std::wstring attempt;
            for (rainy_let prefix = 0u; retry && !hand && prefix < prefix_list.size(); ++prefix) {
                for (rainy_let suffix = 0u; retry && !hand && suffix < suffix_list.size(); ++suffix) {
                    if (!prefix_list[prefix].empty() && wide_path.starts_with(prefix_list[prefix])) {
                        continue;
                    }
                    if (!suffix_list[suffix].empty() && wide_path.ends_with(suffix_list[suffix])) {
                        continue;
                    }
                    attempt = prefix_list[prefix] + wide_path + suffix_list[suffix];
                    // 对应 RTLD_NOLOAD：仅查询已加载，不触发新加载
                    HMODULE mod = GetModuleHandleW(attempt.c_str());
                    hand = to_handle(mod);
                    if (!hand && is_absolute_path(module_path) && file_exist(attempt)) {
                        retry = false;
                    }
                }
            }
        } else {
            const std::wstring wide_path(module_path.begin(), module_path.end());
            HMODULE mod = GetModuleHandleW(wide_path.c_str());
            hand = to_handle(mod);
        }
        return hand;
    }

    farproc_fn load_symbol(const core::handle handle, const std::string_view symbol_name) noexcept {
        if (!handle) {
            return nullptr;
        }
        HMODULE mod = from_handle(handle);
        FARPROC proc = GetProcAddress(mod, std::string(symbol_name).c_str());
        if (!proc) {
            return nullptr;
        }
        return reinterpret_cast<farproc_fn>(proc);
    }

    bool release_module(core::handle *handle) noexcept {
        if (!handle || !*handle) {
            return false;
        }
        HMODULE mod = from_handle(*handle);
        if (!FreeLibrary(mod)) {
            return false;
        }
        *handle = 0;
        return true;
    }
}

#endif
