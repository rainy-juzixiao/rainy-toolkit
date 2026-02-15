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
#include <filesystem>
#include <rainy/foundation/pal/implements/tgc_layer_module_context.hpp>
#include <string>
#include <vector>
#include <windows.h>

namespace rainy::foundation::pal::module_context::implements {
    static std::vector<std::wstring> build_module_candidates(const std::string_view module_path, const bool crossplatform) {
        std::vector<std::wstring> result;
        result.reserve(4);
        std::filesystem::path p(module_path);
        auto to_w = [](const std::filesystem::path &v) { return v.wstring(); };
        // 原始名称
        result.push_back(to_w(p));
        if (!crossplatform) {
            return result;
        }
        // 已有扩展名
        if (p.has_extension()) {
            return result;
        }
        std::wstring name = p.filename().wstring();
        // Windows 不需要 lib 前缀，但允许尝试
        result.push_back(name + L".dll");
        if (!name.starts_with(L"lib")) {
            result.push_back(L"lib" + name + L".dll");
        }
        // 带目录情况
        if (p.has_parent_path()) {
            auto parent = p.parent_path();
            for (auto &r: result) {
                r = (parent / r).wstring();
            }
        }
        return result;
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
    core::handle load_module(std::string_view module_path, bool crossplatform) noexcept {
        rainy_const candidates = build_module_candidates(module_path, crossplatform);
        for (const auto &path: candidates) {
            HMODULE mod = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            if (mod) {
                return to_handle(mod);
            }
        }
        return 0;
    }

    core::handle try_to_get_module(std::string_view module_path,
                               bool crossplatform) noexcept
    {
        auto candidates = build_module_candidates(module_path, crossplatform);
        for (const auto& path : candidates)
        {
            // 仅查询已加载模块
            HMODULE mod = GetModuleHandleW(path.c_str());
            if (mod) {
                return to_handle(mod);
            }
        }
        return 0;
    }

    farproc_fn load_symbol(core::handle handle, std::string_view symbol_name) noexcept {
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
