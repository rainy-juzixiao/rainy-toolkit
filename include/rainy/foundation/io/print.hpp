#ifndef RAINY_FOUNDATION_IO_PRINT_HPP
#define RAINY_FOUNDATION_IO_PRINT_HPP
#include <rainy/core/core.hpp>

#if RAINY_HAS_CXX20
#include <format>
#endif

#include <rainy/winapi/io/console.hpp>

namespace rainy::foundation::io {
    template <typename... Args>
    void print(std::format_string<Args...> fmt, Args &&...args) {
    
    }
    
    template <typename... Args>
    void print(FILE *stream, format_string<Args...> fmt, Args &&...args) {
    
    }

    template <typename... Args>
    void println(format_string<Args...> fmt, Args &&...args) {
    
    }
    
    void println() {
    }

    template <typename... Args>
    void println(FILE *stream, format_string<Args...> fmt, Args &&...args) {
    
    }
    
    void println(FILE *stream) {
    
    }

    void vprint_unicode(string_view fmt, std::format_args args) {
    
    }
    
    void vprint_unicode(FILE *stream, string_view fmt, std::format_args args) {
    
    }

    void vprint_unicode_locking(FILE *stream, string_view fmt, format_args args) {
    
    }

    void vprint_nonunicode(string_view fmt, format_args args) {
    
    }
    
    void vprint_nonunicode(FILE *stream, string_view fmt, format_args args) {
    
    }

    void vprint_nonunicode_locking(FILE *stream, string_view fmt, format_args args) {
    
    }
}

#endif