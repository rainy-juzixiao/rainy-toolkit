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
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/wstring_convert.hpp>

#include <rainy/foundation/os/module_context.hpp>
#include <string>
#include <vector>
#include <windows.h>

namespace rainy::foundation::dynamic_library::implements {
    RAINY_TOOLKIT_LOCAL_API static bool is_absolute_path(const core::text::string_view file_name) {
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

    RAINY_TOOLKIT_LOCAL_API static bool file_exist(const core::text::wstring &file_name) {
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

namespace rainy::foundation::dynamic_library::implements {
    core::handle load_module(const core::text::string_view module_path, const bool crossplatform) noexcept {
        core::handle hand = 0;
        if (crossplatform) {
            const core::collections::vector<core::text::wstring> prefix_list = {L"", L"lib"};
            const core::collections::vector<core::text::wstring> suffix_list = {L".dll"};
            // module_path 转宽字符，用于前后缀判断
            core::text::wstring_convert<core::text::codecvt_utf8<wchar_t>, core::text::basic_string, wchar_t,
                                              core::text::char_traits, core::text::wstring::allocator_type, core::text::string::allocator_type>
                converter;
            const core::text::wstring wide_path = converter.from_bytes(module_path.begin(), module_path.end());
            rainy_let retry = true;
            core::text::wstring attempt;
            for (rainy_let prefix = 0u; retry && !hand && prefix < prefix_list.size(); ++prefix) {
                for (rainy_let suffix = 0u; retry && !hand && suffix < suffix_list.size(); ++suffix) {
                    // 已有对应前缀则不再拼接（与Linux逻辑对称）
                    if (!prefix_list[prefix].empty() && wide_path.starts_with(prefix_list[prefix])) {
                        attempt = wide_path;
                    } else {
                        attempt = prefix_list[prefix] + wide_path;
                    }
                    // 已有对应后缀则不再拼接
                    if (suffix_list[suffix].empty() || !wide_path.ends_with(suffix_list[suffix])) {
                        attempt += suffix_list[suffix];
                    }
                    HMODULE mod = LoadLibraryExW(attempt.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
                    hand = to_handle(mod);
                    // 绝对路径下，文件存在但加载失败，停止重试（与Linux retry=false逻辑对称）
                    if (!hand && is_absolute_path(module_path) && file_exist(attempt)) {
                        retry = false;
                    }
                }
            }
        } else {
            const core::text::wstring wide_path(module_path.begin(), module_path.end());
            HMODULE mod = LoadLibraryExW(wide_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            hand = to_handle(mod);
        }
        return hand;
    }

    core::handle try_to_get_module(const core::text::string_view module_path, const bool crossplatform) noexcept {
        core::handle hand = 0;
        if (crossplatform) {
            const core::collections::vector<core::text::wstring> prefix_list = {L"", L"lib"};
            const core::collections::vector<core::text::wstring> suffix_list = {L".dll"};
            const core::text::wstring wide_path(module_path.begin(), module_path.end());
            rainy_let retry = true;
            core::text::wstring attempt;
            for (rainy_let prefix = 0u; retry && !hand && prefix < prefix_list.size(); ++prefix) {
                for (rainy_let suffix = 0u; retry && !hand && suffix < suffix_list.size(); ++suffix) {
                    if (!prefix_list[prefix].empty() && wide_path.starts_with(prefix_list[prefix])) {
                        attempt = wide_path;
                    } else {
                        attempt = prefix_list[prefix] + wide_path;
                    }
                    if (suffix_list[suffix].empty() || !wide_path.ends_with(suffix_list[suffix])) {
                        attempt += suffix_list[suffix];
                    }
                    HMODULE mod = GetModuleHandleW(attempt.c_str());
                    hand = to_handle(mod);
                    if (!hand && is_absolute_path(module_path) && file_exist(attempt)) {
                        retry = false;
                    }
                }
            }
            if (!hand) {
                // GetModuleHandle 可能无法通过名称返回已加载系统模块的句柄，这里进行fallback处理
                HMODULE mod = LoadLibraryExW(wide_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
                hand = to_handle(mod);
            }
        } else {
            const core::text::wstring wide_path(module_path.begin(), module_path.end());
            HMODULE mod = GetModuleHandleW(wide_path.c_str());
            if (!mod) {
                mod = LoadLibraryExW(wide_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            }
            hand = to_handle(mod);
        }
        return hand;
    }

    farproc_fn load_symbol(const core::handle handle, const core::text::string_view symbol_name) noexcept {
        if (!handle) {
            return nullptr;
        }
        HMODULE mod = from_handle(handle);
        FARPROC proc = GetProcAddress(mod, core::text::string(symbol_name).c_str());
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

