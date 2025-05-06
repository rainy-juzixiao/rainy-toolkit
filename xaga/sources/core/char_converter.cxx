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
#include <rainy/core/core.hpp>

#if RAINY_USING_WINDOWS

#include <windows.h>

namespace rainy::core::builtin {
    int multibyte_to_wchar(const char *source, wchar_t *buffer, std::size_t buffer_length, std::size_t codepage) {
        // 转换需要的字节数
        std::size_t required_size = ::MultiByteToWideChar(static_cast<unsigned int>(codepage), 0, source, -1, nullptr, 0);
        if (required_size == 0) {
            return -1; // 错误
        }

        // 如果缓冲区不足
        if (required_size > buffer_length) {
            return -2; // 缓冲区不足
        }

        // 转换到宽字符
        int bytes_converted = ::MultiByteToWideChar(static_cast<unsigned int>(codepage), 0, source, -1, buffer,
                                                    static_cast<unsigned int>(buffer_length));
        if (bytes_converted == 0) {
            return -1; // 错误
        }

        return bytes_converted; // 返回转换的字节数
    }

    int wchar_to_multibyte(const wchar_t *source, char *buffer, std::size_t buffer_length, std::size_t codepage) {
        std::size_t required_size = ::WideCharToMultiByte(static_cast<unsigned int>(codepage), 0, reinterpret_cast<const wchar_t *>(source),
                                                  -1, nullptr, 0, nullptr, nullptr);
        if (required_size == 0) {
            return -1;
        }
        if (required_size > buffer_length) {
            return -2;
        }
        int bytes_converted =
            ::WideCharToMultiByte(static_cast<unsigned int>(codepage), 0, reinterpret_cast<const wchar_t *>(source), -1, buffer,
                                                    static_cast<int>(buffer_length), nullptr, nullptr);
        if (bytes_converted == 0) {
            return -1;
        }
        return bytes_converted;
    }
}

#endif