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
#include <dlfcn.h>
#include <filesystem>
#include <rainy/foundation/pal/implements/tgc_layer_module_context.hpp>

namespace rainy::foundation::pal::module_context::implements {
    RAINY_TOOLKIT_LOCAL_API static bool is_absolute_path(const std::string_view file_name) {
        return (!file_name.empty() && file_name[0] == '/'); // NOLINT
    }

    constexpr std::string get_suffixes_sys() {
        return ".so";
    }

    RAINY_TOOLKIT_LOCAL_API static bool file_exist(const std::string_view fileName) {
        if (rainy_const file = fopen(fileName.data(), "r")) { // NOLINT
            (void) fclose(file);
            return true;
        }
        return false;
    }

    using native_handle = void *;

    RAINY_TOOLKIT_LOCAL_API static core::handle to_handle(native_handle h) noexcept {
        return reinterpret_cast<core::handle>(h);
    }

    RAINY_TOOLKIT_LOCAL_API static native_handle from_handle(const core::handle h) noexcept {
        return reinterpret_cast<native_handle>(h);
    }
}

namespace rainy::foundation::pal::module_context::implements {
    core::handle load_module(const std::string_view module_path, const bool crossplatform) noexcept {
        core::handle hand = 0;
        if (crossplatform) {
            const std::vector<std::string> prefix_list = {"lib"};
            const std::vector<std::string> suffix_list = {get_suffixes_sys()};
            rainy_let retry = true;
            std::string attempt;
            for (rainy_let prefix = 0u; retry && !hand && prefix < prefix_list.size(); ++prefix) {
                for (rainy_let suffix = 0u; retry && !hand && suffix < suffix_list.size(); ++suffix) {
                    if (!prefix_list[prefix].empty() && module_path.starts_with(prefix_list[prefix])) { // NOLINT
                        continue;
                    }
                    if (!suffix_list[suffix].empty() && module_path.ends_with(suffix_list[suffix])) { // NOLINT
                        continue;
                    }
                    attempt =
                        prefix_list[prefix] + std::string{module_path.data(), module_path.size()} + suffix_list[suffix]; // NOLINT
                    hand = to_handle(dlopen(attempt.c_str(), RTLD_NOW));
                    if (!hand && is_absolute_path(module_path) && file_exist(attempt)) {
                        retry = false;
                    }
                }
            }
        } else {
            hand = to_handle(dlopen(module_path.data(), RTLD_NOW)); // NOLINT
        }
        return hand;
    }

    core::handle try_to_get_module(const std::string_view module_path, const bool crossplatform) noexcept {
        core::handle hand = 0;
        if (crossplatform) {
            const std::vector<std::string> prefix_list = {"lib"};
            const std::vector<std::string> suffix_list = {get_suffixes_sys()};
            rainy_let retry = true;
            std::string attempt;
            for (rainy_let prefix = 0u; retry && !hand && prefix < prefix_list.size(); ++prefix) {
                for (rainy_let suffix = 0u; retry && !hand && suffix < suffix_list.size(); ++suffix) {
                    if (!prefix_list[prefix].empty() && module_path.starts_with(prefix_list[prefix])) { // NOLINT
                        continue;
                    }
                    if (!suffix_list[suffix].empty() && module_path.ends_with(suffix_list[suffix])) { // NOLINT
                        continue;
                    }
                    attempt =
                        prefix_list[prefix] + std::string{module_path.data(), module_path.size()} + suffix_list[suffix]; // NOLINT
                    hand = to_handle(dlopen(attempt.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NOLOAD));
                    if (!hand && is_absolute_path(module_path) && file_exist(attempt)) {
                        retry = false;
                    }
                }
            }
        } else {
            hand = to_handle(dlopen(module_path.data(), RTLD_NOW | RTLD_LOCAL | RTLD_NOLOAD)); // NOLINT
        }
        return hand;
    }

    farproc_fn load_symbol(const core::handle handle, const std::string_view symbol_name) noexcept {
        if (!handle) {
            return nullptr;
        }
        const native_handle hand = from_handle(handle); // NOLINT
        dlerror();
        void *sym = dlsym(hand, std::string(symbol_name).c_str());
        if (!sym) {
            return nullptr;
        }
        return reinterpret_cast<farproc_fn>(sym);
    }

    bool release_module(core::handle *handle) noexcept {
        if (!handle || !*handle) {
            return false;
        }
        if (const native_handle h = from_handle(*handle); dlclose(h) != 0) { // NOLINT
            return false;
        }
        *handle = 0;
        return true;
    }
}

#endif