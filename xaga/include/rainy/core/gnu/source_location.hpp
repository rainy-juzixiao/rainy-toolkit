#ifndef RAINY_CORE_GNU_SOURCE_LOCATION_HPP
#define RAINY_CORE_GNU_SOURCE_LOCATION_HPP

#include <rainy/core/platform.hpp>

#if RAINY_USING_GCC

#if RAINY_IS_CXX17

/*
注意，为了确保GNU编译器环境下，可以顺利获取column信息。
 我们必须伪造一个source_location结构体，并将其作为参数传入
*/
namespace std {
    struct source_location { // NOLINT
        struct __impl // NOLINT
        {
            [[maybe_unused]] const char* _M_file_name; // NOLINT
            [[maybe_unused]] const char* _M_function_name; // NOLINT
            [[maybe_unused]] unsigned int _M_line; // NOLINT
            unsigned int _M_column; // NOLINT
        };
    };
}

#elif RAINY_HAS_CXX20
#include <source_location>
#endif

/*
此命名空间是预先声明给gnu编译器的，请勿使用
原因是此区域是处于混乱的
*/
namespace rainy::foundation::diagnostics::gcc_detail_impl {
#if RAINY_IS_CXX17
    using source_location_impl = std::source_location::__impl;
    constexpr unsigned int get_column(
        const source_location_impl *loc = static_cast<const source_location_impl *>(__builtin_source_location())) {
        return loc->_M_column;
    }
#else
    constexpr unsigned int get_column(const std::source_location location = std::source_location::current()) {
        return location.column();
    }
#endif
}

#endif

#endif
