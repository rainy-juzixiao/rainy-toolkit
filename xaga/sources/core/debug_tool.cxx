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
#include <cstdio>

#if RAINY_USING_WINDOWS
#include <DbgEng.h>
#include <DbgHelp.h>
#include <windows.h>
#if RAINY_USING_MSVC || RAINY_USING_CLANG
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "DbgEng.lib")
#pragma comment(lib, "Dbghelp.lib")
#endif
#elif RAINY_USING_LINUX
#include <execinfo.h>
#endif

#if RAINY_USING_GCC
#if __has_include("cxxabi.h")
#include <cxxabi.h>
#endif
#endif

namespace rainy::core::pal {
    void debug_break() {
#if RAINY_USING_MSVC
        __debugbreak();
#elif RAINY_USING_GCC || RAINY_USING_CLANG
#if RAINY_USING_WINDOWS
#if defined(__i386__) || defined(__x86_64__)
        __asm__ __volatile__("int {$}3" :);
#endif
#elif RAINY_USING_LINUX
        __asm__ __volatile__("int $0x3");
#else
        static_assert(false, "rainy-toolkit only supports Windows and Linux platforms");
#endif
#endif
    }

    std::size_t collect_stack_frame(native_frame_ptr_t *out_frames, std::size_t max_frames_count, const std::size_t skip) noexcept {
#if RAINY_USING_WINDOWS
        return ::RtlCaptureStackBackTrace(static_cast<unsigned long>(skip), static_cast<unsigned long>(max_frames_count), out_frames,
                                          nullptr);
#elif RAINY_USING_LINUX
        (void) skip;
        return backtrace(const_cast<void **>(out_frames), max_frames_count);
#endif
    }

    std::size_t safe_dump_to(void *memory, std::size_t size, std::size_t skip) noexcept {
        if (size < sizeof(void *)) {
            return 0; // 如果提供的内存空间不够，就返回 0
        }
        auto *mem = static_cast<void **>(memory);
#if RAINY_USING_WINDOWS
        const std::size_t frames_count = collect_stack_frame(mem, size / sizeof(native_frame_ptr_t) - 1, skip + 1);
        mem[frames_count] = nullptr;
        return frames_count + 1;
#elif RAINY_USING_LINUX
        const std::size_t frames_count = collect_stack_frame(mem, size / sizeof(void *), skip + 1);
        mem[frames_count] = nullptr;
        return frames_count;
#endif
    }

    bool resolve_stack_frame(native_frame_ptr_t frame, cstring buf, std::size_t buf_size) noexcept {
        if (!buf || buf_size == 0 || !frame) {
            return false;
        }
#if RAINY_USING_WINDOWS
        HANDLE process = GetCurrentProcess();
        static bool sym_initialized = false;
        if (!sym_initialized) {
            (void) SymInitialize(process, nullptr, TRUE);
            sym_initialized = true;
        }
        char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] = {0};
        auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbol_buffer);
        symbol->MaxNameLen = MAX_SYM_NAME;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        if (SymFromAddr(process, reinterpret_cast<DWORD64>(frame), nullptr, symbol)) {
            (void) std::snprintf(buf, buf_size, "%s", symbol->Name);
            return true;
        }
        return false;
#elif RAINY_USING_LINUX
        char *symbols[1] = {nullptr};
        symbols[0] = nullptr;
        char **resolved = backtrace_symbols(const_cast<void **>(&frame), 1);
        if (resolved) {
            std::snprintf(buf, buf_size, "%s", resolved[0] ? resolved[0] : "(unknown)");
            return true;
        }
        std::snprintf(buf, buf_size, "(unknown)");
        return false;
#else
        return false;
#endif
    }

#if RAINY_USING_GCC
    static void abi_demangle(czstring name, cstring buf, std::size_t buffer_length) {
        if (!buf || buffer_length == 0) {
            errno = EINVAL;
            return;
        }
        int status;
        std::size_t copy_size{};
        auto ptr = ::abi::__cxa_demangle(name, nullptr, &copy_size, &status);
        switch (status) {
            case 0:
                errno = 0;
                if (buffer_length <= copy_size) {
                    errno = ERANGE;
                } else {
                    std::snprintf(buf, copy_size, "%s", ptr);
                }
                std::free(ptr);
                break;
            case -1:
            case -2:
                copy_size = strlen(name);
                if (buffer_length > copy_size) {
                    std::snprintf(buf, copy_size, "%s", name);
                }
                errno = EINVAL;
                break;
        }
    }
#endif

    void demangle(czstring name, cstring buf, std::size_t buffer_length) {
#if RAINY_USING_WINDOWS
#if RAINY_USING_MSVC || RAINY_USING_CLANG
        auto ret = UnDecorateSymbolName(name, buf,static_cast<DWORD>(buffer_length), 0);
        if (ret != 0) {
            buf[ret] = 0;
        }
#elif RAINY_USING_GCC
        abi_demangle(name, buf, buffer_length);
#else
        static_assert(false, "unsupported compiler from " RAINY_STRINGIZE(rainy::core::pal::demangle));
#endif
#else
        abi_demangle(name, buf, buffer_length);
#endif
    }
}
