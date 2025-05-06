/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/core/layer.hpp>

#if RAINY_USING_WINDOWS
#include <rainy/winapi/api_core.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

#define RAINY_TOOLKIT_INTERNAL_WINAPI __declspec(dllimport)

namespace rainy::core::pal {
    io_size_t write(std::uintptr_t fd, const void *buffer, size_t count) {
        return std::fwrite(buffer, 1, count, reinterpret_cast<FILE *>(fd));
    }
}

#endif
