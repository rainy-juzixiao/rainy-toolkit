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
#include <rainy/core/text/print.hpp>

#if RAINY_USING_WINDOWS
#include <windows.h>
#include <io.h>

namespace rainy::core::text::implements {
    class print_lock {
    public:
        print_lock() noexcept {
            ::InitializeCriticalSection(&cs_);
        }

        ~print_lock() noexcept {
            ::DeleteCriticalSection(&cs_);
        }

        print_lock(const print_lock &) = delete;
        print_lock &operator=(const print_lock &) = delete;

        void lock() noexcept {
            ::EnterCriticalSection(&cs_);
        }

        void unlock() noexcept {
            ::LeaveCriticalSection(&cs_);
        }

    private:
        CRITICAL_SECTION cs_;
    };

    static print_lock &get_print_lock() noexcept {
        static print_lock lock;
        return lock;
    }
}
#endif

namespace rainy::core::text {
    void vprint_unicode(const string_view fmt, const format_args args) {
        vprint_unicode(stdout, fmt, args);
    }

    void vprint_unicode(FILE *stream, const string_view fmt, const format_args args) {
        string result = vformat(fmt, args);
#if RAINY_USING_WINDOWS
        HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(::_fileno(stream)));
        if (handle == INVALID_HANDLE_VALUE) {
            return;
        }
        DWORD mode;
        if (::GetConsoleMode(handle, &mode)) {
            int wide_len = ::MultiByteToWideChar(CP_UTF8, 0, result.data(), static_cast<int>(result.size()), nullptr, 0);
            if (wide_len > 0) {
                wchar_t wide_buf[1024];
                wchar_t *wide_str = wide_buf;
                if (static_cast<std::size_t>(wide_len) > sizeof(wide_buf) / sizeof(wide_buf[0])) {
                    wide_str = new wchar_t[static_cast<std::size_t>(wide_len)];
                }
                ::MultiByteToWideChar(CP_UTF8, 0, result.data(), static_cast<int>(result.size()), wide_str, wide_len);
                DWORD written{};
                ::WriteConsoleW(handle, wide_str, static_cast<DWORD>(wide_len), &written, nullptr);
                if (wide_str != wide_buf) {
                    delete[] wide_str;
                }
            }
        } else {
            DWORD written{};
            ::WriteFile(handle, result.data(), static_cast<DWORD>(result.size()), &written, nullptr);
        }
#else
        std::fwrite(result.data(), 1, result.size(), stream);
        std::fflush(stream);
#endif
    }

    void vprint_unicode_locking(FILE *stream, const string_view fmt, const format_args args) {
#if RAINY_USING_WINDOWS
        implements::get_print_lock().lock();
#else
        ::flockfile(stream);
#endif
        vprint_unicode(stream, fmt, args);
#if RAINY_USING_WINDOWS
        implements::get_print_lock().unlock();
#else
        ::funlockfile(stream);
#endif
    }

    void vprint_nonunicode(const string_view fmt, const format_args args) {
        vprint_nonunicode(stdout, fmt, args);
    }

    void vprint_nonunicode(FILE *stream, const string_view fmt, const format_args args) {
        string result = vformat(fmt, args);
#if RAINY_USING_WINDOWS
        HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(::_fileno(stream)));
        if (handle == INVALID_HANDLE_VALUE) {
            return;
        }
        DWORD written{};
        ::WriteFile(handle, result.data(), static_cast<DWORD>(result.size()), &written, nullptr);
#else
        std::fwrite(result.data(), 1, result.size(), stream);
        std::fflush(stream);
#endif
    }

    void vprint_nonunicode_locking(FILE *stream, const string_view fmt, const format_args args) {
#if RAINY_USING_WINDOWS
        implements::get_print_lock().lock();
#else
        ::flockfile(stream);
#endif
        vprint_nonunicode(stream, fmt, args);
#if RAINY_USING_WINDOWS
        implements::get_print_lock().unlock();
#else
        ::funlockfile(stream);
#endif
    }

    void println() {
        println(stdout);
    }

    void println(FILE *stream) {
        std::fputc('\n', stream);
        std::fflush(stream);
    }
}