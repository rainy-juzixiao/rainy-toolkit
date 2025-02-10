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
#ifndef RAINY_FORMAT_WRAPPER_HPP
#define RAINY_FORMAT_WRAPPER_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <string>
#include <cstdarg>

namespace rainy::utility {
    template <typename CharType, typename... Args>
    void cstyle_format(std::basic_string<CharType> &ref, const CharType *fmt, Args &&...args);

    namespace internals {
        class format_placeholder_t {
        public:
            template <typename CharType,typename... Args>
            friend void utility::cstyle_format(std::basic_string<CharType> &ref, const CharType *fmt, Args &&...args);

            template <typename CharType>
            friend format_placeholder_t check_format_args(unsigned int arity, std::basic_string<CharType> &ref,
                                                          const std::basic_string_view<CharType> &fmt_view);

        private:
            bool has_placeholder = false;

            bool has_string_format_placeholder = false;
            std::size_t string_format_placeholder_count = 0;

            bool has_number_format_placeholder = false;
            std::size_t number_format_placeholder_count = 0;
        };

        template <typename CharType>
        format_placeholder_t check_format_args(const unsigned int arity, std::basic_string<CharType> &ref,
                                               const std::basic_string_view<CharType> &fmt_view) {
            unsigned int placeholder_count = 0;
            format_placeholder_t format_placeholder;
            if (arity == 0) {
                ref.assign(fmt_view);
                return format_placeholder;
            }
            for (std::size_t offset = 0; offset < fmt_view.size(); ++offset) {
                if (fmt_view[offset] == '%') {
                    if (offset + 1 < fmt_view.size()) {
                        const CharType next = fmt_view[offset + 1];
                        if (next == '%') {
                            format_placeholder.has_placeholder = true;
                            ++offset; // Skip over '%%'
                            continue;
                        } else if (next == 's') {
                            format_placeholder.has_placeholder = true;
                            format_placeholder.has_string_format_placeholder = true;
                            ++format_placeholder.string_format_placeholder_count;
                            ++offset; // Skip over '%s'
                        } else if (next == 'd' || next == 'f' || next == 'p' || next == 'x') {
                            format_placeholder.has_placeholder = true;
                            format_placeholder.has_number_format_placeholder = true;
                            ++format_placeholder.number_format_placeholder_count;
                            ++offset; // Skip over '%d', '%f', '%p', or '%x'
                        }
                        placeholder_count++;
                    }
                }
            }
            if (placeholder_count == 0 && !format_placeholder.has_placeholder) {
                ref.assign(fmt_view);
                return format_placeholder; // 因为我们获得的占位符为0。我们可以认为这种字符串是可以直接返回fmt的
            } // 检查占位符的count是否和arity匹配
            if (placeholder_count != arity) {
#if RAINY_ENABLE_DEBUG
                information::pal::debug_break();
#else
                foundation::system::exceptions::runtime::throw_runtime_error("Format string and arguments count mismatch");
#endif
            }
            return format_placeholder;
        }

        RAINY_INLINE int vswprintf_wrapper(std::wstring &ref, const wchar_t *fmt, ...) {
            va_list arg_list{};
            va_start(arg_list, fmt);
#if (RAINY_USING_MSVC || RAINY_USING_CLANG)
            int result = ::vswprintf_s(&ref[0], ref.capacity(), fmt, arg_list);
#else
            int result = std::vswprintf(&ref[0], ref.capacity(), fmt, arg_list);
#endif
            va_end(arg_list);
            return result;
        };
    }

    template <typename CharType, typename... Args>
    void cstyle_format(std::basic_string<CharType> &ref, const CharType *const fmt, Args &&...args) {
        static_assert(sizeof...(Args) == 0 ? true : information::check_format_type<Args...>,
                      "Unsupported argument type passed to format");
        auto check_func = [&ref](int update_size) {
            if (update_size < 0) {
                if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                    ref = L"error_formatting_string";
                } else {
                    ref = "error_formatting_string";
                }
#if RAINY_ENABLE_DEBUG
                information::pal::debug_break();
#else
                foundation::system::exceptions::runtime::throw_runtime_error("Error formatting string");
#endif
            }
        };
        constexpr unsigned int arity = sizeof...(args);
        const std::basic_string_view<CharType> view{fmt};
        std::size_t possible_size = view.size();
        if (const auto format_placeholder = internals::check_format_args(arity, ref, view); ref.empty()) {
            const std::size_t raw_size = view.size();
            possible_size = raw_size >= 16 && arity > 4 ? raw_size + 50 : raw_size + 30;
            if (format_placeholder.has_string_format_placeholder) {
                possible_size += (16 * format_placeholder.string_format_placeholder_count);
            } else if (format_placeholder.has_number_format_placeholder) {
                possible_size += (8 * format_placeholder.number_format_placeholder_count);
            }
            ref.resize(possible_size);
        } else {
            if (format_placeholder.has_string_format_placeholder) {
                possible_size += (16 * format_placeholder.string_format_placeholder_count);
            } else if (format_placeholder.has_number_format_placeholder) {
                possible_size += (8 * format_placeholder.number_format_placeholder_count);
            }
            if (possible_size > ref.capacity()) {
                ref.resize(possible_size);
            }
        }
        int update_size = 0;
        if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
            update_size = std::snprintf(&ref[0], ref.capacity(), fmt, utility::forward<Args>(args)...);
        } else {
            update_size = internals::vswprintf_wrapper(ref, fmt, utility::forward<Args>(args)...);
        }
        check_func(update_size);
        std::size_t ref_size = ref.capacity();
        while (update_size > ref_size) {
            ref.resize(update_size);
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                update_size = std::snprintf(&ref[0], ref.capacity(), fmt, utility::forward<Args>(args)...);
            } else {
                update_size = internals::vswprintf_wrapper(ref, fmt, utility::forward<Args>(args)...);
            }
            ref_size = ref.capacity();
        }
        ref.resize(update_size);
    }
}

#if RAINY_HAS_CXX20

namespace rainy::utility {
    /**
     * @brief 使用 C++20 的格式化功能来格式化 `std::basic_string`。
     *
     * @param ref 需要格式化的 `std::basic_string` 对象，格式化结果将写入其中。
     * @param fmt C++20 的格式化字符串视图。
     * @param args 格式化所需的参数。
     *
     * 使用 `std::vformat` 进行格式化。如果发生格式化错误，将抛出 `std::format_error` 异常。
     */
    template <typename CharType, typename... Args>
    void basic_format(std::basic_string<CharType> &ref, std::basic_string_view<CharType> fmt, Args... args) {
        try {
            if constexpr (sizeof...(args) == 0) {
                ref = fmt;
                return;
            }
            if constexpr (std::is_same_v<CharType, char>) {
                auto fmt_args = std::make_format_args(args...);
                ref = std::vformat(fmt, fmt_args);
            } else {
                auto fmt_args = std::make_wformat_args(args...);
                ref = std::vformat(fmt, fmt_args);
            }
        } catch (std::format_error &except) {
            utility::throw_exception(std::format_error(std::format("found a error in function: {} \n the error is: {}",
                                                                   RAINY_STRINGIZE(rainy::utility::format), except.what())));
        }
    }
}

#endif

namespace rainy::utility {
    /**
     * @brief 根据编译器标准选择合适的格式化函数来格式化 `std::string`。
     *
     * @param ref 需要格式化的 `std::string` 对象，格式化结果将写入其中。
     * @param fmt 格式化字符串。
     * @param args 格式化所需的参数。
     *
     * @attention 在 C++20 及以上标准中，调用 `basic_format`。在更早标准中，调用 `cstyle_format`。
     */
    template <typename... Args>
    void format(std::string &ref, const std::string_view &fmt, Args &&...args) {
#if RAINY_HAS_CXX20
        basic_format(ref, fmt, utility::forward<Args>(args)...);
#else
        cstyle_format(ref, fmt.data(), std::forward<Args>(args)...);
#endif
    }

    /**
     * @brief 根据编译器标准选择合适的格式化函数来格式化 `std::wstring`。
     *
     * @param ref 需要格式化的 `std::wstring` 对象，格式化结果将写入其中。
     * @param fmt 格式化字符串。
     * @param args 格式化所需的参数。
     *
     * @attention 在 C++20 及以上标准中，调用 `basic_format`。在C++17标准中，调用 `cstyle_format`。
     */
    template <typename... Args>
    void format(std::wstring &ref, const std::wstring_view &fmt, Args &&...args) {
#if RAINY_HAS_CXX20
        basic_format(ref, fmt, utility::forward<Args>(args)...);
#else
        cstyle_format(ref, fmt.data(), std::forward<Args>(args)...);
#endif
    }
}

#endif