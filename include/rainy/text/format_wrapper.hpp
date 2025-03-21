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

#include <cmath>
#include <cstdarg>
#include <iostream>
#include <rainy/containers/array.hpp>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/utility/pair.hpp>
#include <string>

#if RAINY_USING_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#endif

#if RAINY_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

namespace rainy::text {
    template <typename CharType, typename... Args>
    void cstyle_format(std::basic_string<CharType> &ref, const std::basic_string_view<CharType> fmt, Args &&...args);
}

namespace rainy::text::internals {
    class format_placeholder_t {
    public:
        template <typename CharType, typename... Args>
        friend void text::cstyle_format(std::basic_string<CharType> &ref, const std::basic_string_view<CharType> fmt, Args &&...args);

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
            if (fmt_view[offset] == '%' && offset + 1 < fmt_view.size()) {
                const CharType next = fmt_view[offset + 1];
                switch (next) {
                    case '%':
                        format_placeholder.has_placeholder = true;
                        ++offset; // Skip over '%%'
                        continue;
                    case 's':
                        format_placeholder.has_placeholder = true;
                        format_placeholder.has_string_format_placeholder = true;
                        ++format_placeholder.string_format_placeholder_count;
                        ++offset; // Skip over '%s'
                        break;
                    case 'd':
                    case 'p':
                    case 'l':
                    case 'x':
                    case 'f':
                        format_placeholder.has_placeholder = true;
                        format_placeholder.has_number_format_placeholder = true;
                        ++format_placeholder.number_format_placeholder_count;
                        ++offset; // Skip over '%d', '%f', '%p', or '%x'
                        break;
                    case '.':
                        while (++offset < fmt_view.size()) {
                            if (fmt_view[offset] == 'f') {
                                format_placeholder.has_placeholder = true;
                                format_placeholder.has_number_format_placeholder = true;
                                ++format_placeholder.number_format_placeholder_count;
                                break;
                            }
                            ++offset;
                        }
                        break;
                }
                placeholder_count++;
            }
        }
        if (placeholder_count == 0 && !format_placeholder.has_placeholder) {
            ref.assign(fmt_view);
            return format_placeholder; // 因为我们获得的占位符为0。我们可以认为这种字符串是可以直接返回fmt的
        } // 检查占位符的count是否和arity匹配
        if (placeholder_count != arity) {
#if RAINY_ENABLE_DEBUG
            core::pal::debug_break();
#else
            foundation::system::exceptions::runtime::throw_runtime_error("Format string and arguments count mismatch");
#endif
        }
        return format_placeholder;
    }

    RAINY_INLINE int vswprintf_wrapper(wchar_t *buffer, const std::size_t capacity, const wchar_t *fmt, ...) {
        va_list arg_list{};
        va_start(arg_list, fmt);
#if (RAINY_USING_MSVC || RAINY_USING_CLANG)
        int result = ::vswprintf_s(buffer, capacity, fmt, arg_list);
#else
        int result = std::vswprintf(buffer, capacity, fmt, arg_list);
#endif
        va_end(arg_list);
        return result;
    }

    // clang-format off
    static constexpr containers::array ba_bytes_sf_tables(
    {
        utility::make_pair(utility::make_pair(0ull,3ull), 4),
        utility::make_pair(utility::make_pair(4ull,7ull), 5),
        utility::make_pair(utility::make_pair(8ull,11ull), 8)
    });

    static constexpr containers::array ba_bytes_nf_tables(
    {
        utility::make_pair(utility::make_pair(0ull,3ull),3),
        utility::make_pair(utility::make_pair(4ull,7ull),4),
        utility::make_pair(utility::make_pair(8ull,11ull), 5)
    });

    static constexpr containers::array additional_allocation_tables(
    {
        utility::make_pair(utility::make_pair(0ull, 10ull), 3ull),
        utility::make_pair(utility::make_pair(11ull, 16ull), 5ull),
        utility::make_pair(utility::make_pair(17ull, 32ull), 8ull),
        utility::make_pair(utility::make_pair(33ull, 54ull), 14ull),
        utility::make_pair(utility::make_pair(55ull, 92ull), 17ull),
        utility::make_pair(utility::make_pair(93ull, 135ull), 22ull),
        utility::make_pair(utility::make_pair(136ull, 185ull), 24ull),
        utility::make_pair(utility::make_pair(186ull, 227ull), 28ull),
        utility::make_pair(utility::make_pair(228ull, 314ull), 32ull),
        utility::make_pair(utility::make_pair(315ull, 373ull), 42ull),
    });
    // clang-format on

    static constexpr std::size_t alloc_mask = 4;

    constexpr bool is_in_range(std::size_t start, std::size_t end, std::size_t value) {
        return core::internals::in_range<std::size_t>(start, end, value);
    }

    constexpr std::size_t invoke_min(std::size_t first, std::size_t second) {
        return (std::min<std::size_t>) (first, second);
    }

    constexpr std::size_t border_less = 5;

    constexpr int border_calc(std::size_t start, std::size_t end, std::size_t value, std::size_t border_less_ = border_less) {
        return is_in_range(start + border_less_, end - border_less_, value) ? 0 : (value < start + border_less_ ? -1 : 1);
    }

    template <std::size_t Arity>
    std::size_t calc_pre_allocation(std::size_t raw_size, std::size_t number_format_placeholder,std::size_t string_format_placeholder) {
        std::size_t ba_bytes_without_factor_sf = string_format_placeholder > 11 ? 9 : 0;
        std::size_t ba_bytes_without_factor_nf = number_format_placeholder > 11 ? 7 : 0;
        for (const auto &table: ba_bytes_sf_tables) {
            if (!is_in_range(table.first.first, table.first.second, string_format_placeholder)) {
                continue;
            }
            switch (border_calc(table.first.first, table.first.second, string_format_placeholder, 1)) {
                case 0:
                    ba_bytes_without_factor_sf = table.second;
                    break;
                case 1:
                    ba_bytes_without_factor_sf = static_cast<std::size_t>(table.second * 1.2f); // NOLINT
                    break;
                case -1:
                    ba_bytes_without_factor_sf = static_cast<std::size_t>(table.second * 0.8f); // NOLINT
                    break;
            }
        }
        for (const auto & table : ba_bytes_nf_tables){
            if (!is_in_range(table.first.first, table.first.second, number_format_placeholder)) {
                continue;
            }
            switch(border_calc(table.first.first, table.first.second,number_format_placeholder,1)) {
                case 0:
                    ba_bytes_without_factor_nf = table.second;
                    break;
                case 1:
                    ba_bytes_without_factor_nf = static_cast<std::size_t>(table.second * 1.2f); // NOLINT
                    break;
                case -1:
                    ba_bytes_without_factor_nf = static_cast<std::size_t>(table.second * 0.8f); // NOLINT
                    break;
            }
        }
        constexpr std::float_t basic_allocation_factor = is_in_range(1, 4, Arity)   ? 1.2f
                                                     : is_in_range(4, 7, Arity) ? 1.3f
                                                     : is_in_range(7, 9, Arity) ? 1.4f
                                                     : Arity > 12               ? 1.6f
                                                                                : 1.5f;
        // NOLINTBEGIN
        rainy_let basic_allocation = static_cast<std::size_t>(raw_size * 0.55f) +
                                       static_cast<std::size_t>(((ba_bytes_without_factor_nf * number_format_placeholder) +
                                                                 (ba_bytes_without_factor_sf * string_format_placeholder)));
        basic_allocation += static_cast<std::size_t>((basic_allocation / static_cast<float>(10)) * basic_allocation_factor);
        // NOLINTEND
        std::size_t additional_allocation = 0;
        for (const auto &table: additional_allocation_tables) {
            if (!is_in_range(table.first.first, table.first.second, raw_size)) {
                continue;
            }
            switch (border_calc(table.first.first, table.first.second, raw_size)) {
                case 0:
                    additional_allocation = table.second + static_cast<std::size_t>((basic_allocation / 4) * basic_allocation_factor);
                    break;
                case 1:
                    additional_allocation = static_cast<std::size_t>(table.second * 1.2f) + // NOLINT
                                            static_cast<std::size_t>((basic_allocation / 4) * basic_allocation_factor); // NOLINT
                    break;
                case -1:
                    additional_allocation = static_cast<std::size_t>(table.second * 0.8f) + // NOLINT
                                            static_cast<std::size_t>((basic_allocation / 4) * basic_allocation_factor); // NOLINT
                    break;
            }
        }
        if (raw_size > 165) {
            additional_allocation += invoke_min(static_cast<std::size_t>(5.5f * Arity), 92);
        } else {
            additional_allocation += invoke_min(static_cast<std::size_t>(6.5f * Arity), 56);
        }
        std::size_t total_allocation =
            static_cast<std::size_t>((additional_allocation + basic_allocation + (basic_allocation | alloc_mask)) * 0.55f);
        if (total_allocation < 374) {
            return total_allocation;
        }
        switch (border_calc(374, 648, raw_size, 30)) {
            case 0:
                total_allocation += (total_allocation / 2); // NOLINT
                break;
            case -1:
                total_allocation += static_cast<std::size_t>(total_allocation * 0.4f); // NOLINT
                break;
            case 1:
                total_allocation += static_cast<std::size_t>(total_allocation * 0.6f); // NOLINT
                break;
        }
        return total_allocation;
    }

    template <std::size_t Arity>
    std::size_t calc_allocation(std::size_t raw_size,std::size_t ref_capacity, std::size_t number_format_placeholder, std::size_t string_format_placeholder) {
        return calc_pre_allocation<Arity>(static_cast<std::size_t>(ref_capacity * 1.2f) + raw_size, number_format_placeholder,
                                   string_format_placeholder);
    }
}

namespace rainy::text {
    /**
     * @brief 使用 C 风格的格式化功能来格式化 `std::basic_string`
     * @attention 此函数具有静态检查，将检查格式化参数的类型是否与底层函数支持的类型一致
     * @attention 对于空的buffer，将会启用
     * @tparam CharType 由模板自动推导，表示字符串的字符类型
     * @tparam Args 格式化所需的参数类型
     * @param ref 需要格式化的 `std::basic_string` 对象，格式化结果将写入其中
     * @param fmt 格式化字符串
     * @param args 格式化所需的参数
     */
    template<typename CharType, typename... Args>
    void cstyle_format(std::basic_string<CharType> &ref, const std::basic_string_view<CharType> fmt, Args &&...args) {
        static_assert(sizeof...(Args) == 0 ? true : core::check_format_type<Args...>,
                      "Unsupported argument type passed to format");
        auto check_func = [&ref](int update_size) {
            if (update_size < 0) {
                if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                    ref = L"error_formatting_string";
                } else {
                    ref = "error_formatting_string";
                }
#if RAINY_ENABLE_DEBUG
                core::pal::debug_break();
#else
                foundation::system::exceptions::runtime::throw_runtime_error("Error formatting string");
#endif
            }
        };
        constexpr unsigned int arity = sizeof...(args);
        std::size_t possible_size = fmt.size();
        const std::size_t raw_size = fmt.size();
        if (const auto format_placeholder = internals::check_format_args(arity, ref, fmt); ref.empty()) {
            possible_size = internals::calc_pre_allocation<arity>(raw_size, format_placeholder.number_format_placeholder_count,
                                                                  format_placeholder.string_format_placeholder_count);
            ref.reserve(possible_size);
            ref.resize(ref.capacity());
        } else {
            std::size_t ref_capacity = ref.capacity();
            if (possible_size > ref_capacity) {
                const std::size_t alloc =
                    internals::calc_allocation<arity>(raw_size, ref_capacity, format_placeholder.number_format_placeholder_count,
                                                      format_placeholder.string_format_placeholder_count);
                ref.reserve(alloc);
                ref.resize(alloc);
            }
        }
        int update_size = 0;
        do { // NOLINT
            if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                update_size = std::snprintf(&ref[0], ref.size(), fmt.data(), utility::forward<Args>(args)...); // NOLINT
                if (rainy_likely(update_size < ref.size())) {
                    ref.resize(update_size + 1);
                    break;
                }
                ref.resize(update_size + 1);
                update_size = std::snprintf(&ref[0], ref.capacity(), fmt.data(), utility::forward<Args>(args)...);
            } else {
                update_size = internals::vswprintf_wrapper(&ref[0], ref.capacity(), fmt.data(),
                                                           utility::forward<Args>(args)...);
            }
            check_func(update_size);
            if (rainy_likely(update_size < ref.capacity())) {
                ref.resize(update_size + 1);
                break;
            }
            if (update_size > ref.capacity()) {
                ref.reserve(update_size);
            }
        } while (update_size > ref.capacity());
        ref.resize(update_size);
    }

    template<typename CharType, typename... Args>
    void cstyle_format(std::basic_string<CharType> &ref, const CharType *const fmt, Args &&...args) {
        cstyle_format(ref, std::basic_string_view{fmt}, utility::forward<Args>(args)...);
    }
}

#if RAINY_HAS_CXX20

namespace rainy::text {
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

namespace rainy::text {
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
        cstyle_format(ref, fmt.data(), utility::forward<Args>(args)...);
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
        cstyle_format(ref, fmt.data(), utility::forward<Args>(args)...);
#endif
    }
}

#if RAINY_USING_CLANG
#pragma clang diagnostic pop
#endif

#if RAINY_USING_GCC
#pragma GCC diagnostic pop
#endif

namespace rainy::utility {
    using text::cstyle_format;
    using text::format;
#if RAINY_HAS_CXX20
    using text::basic_format;
#endif
}

#endif