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
#ifndef RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_FORMAT_ARG_HPP
#define RAINY_CORE_IMPLEMENTS_TEXT_FORMAT_FORMAT_ARG_HPP
#include <rainy/core/core.hpp>
#include <string_view>
#include <variant>

namespace rainy::foundation::text {
    template <typename Context, typename... Args>
    class __format_arg_store;

    template <typename CharT>
    class basic_format_parse_context;
}

namespace rainy::foundation::text::implements {
    // 检查类型是否为有符号整数
    template <typename T>
    struct is_signed_integer
        : std::bool_constant<std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char> &&
                             !std::is_same_v<T, wchar_t> && !std::is_same_v<T, char16_t> && !std::is_same_v<T, char32_t>> {};

    template <typename T>
    inline constexpr bool is_signed_integer_v = is_signed_integer<T>::value;

    // 检查类型是否为无符号整数
    template <typename T>
    struct is_unsigned_integer : std::bool_constant<std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool>> {};

    template <typename T>
    inline constexpr bool is_unsigned_integer_v = is_unsigned_integer<T>::value;

    // 检查类型是否为字符类型
    template <typename T, typename CharT>
    struct is_char_type : std::bool_constant<std::is_same_v<T, CharT>> {};

    template <typename T, typename CharT>
    inline constexpr bool is_char_type_v = is_char_type<T, CharT>::value;

    // 检查类型是否为字符串类型
    template <typename T, typename CharT>
    struct is_string_type : std::bool_constant<std::is_same_v<T, const CharT *> || std::is_same_v<T, std::basic_string_view<CharT>> ||
                                               std::is_convertible_v<T, std::basic_string_view<CharT>>> {};

    template <typename T, typename CharT>
    inline constexpr bool is_string_type_v = is_string_type<T, CharT>::value;

    // 参数类型枚举
    enum class arg_type : unsigned char {
        none_type,
        int_type,
        uint_type,
        long_long_type,
        ulong_long_type,
        bool_type,
        char_type,
        float_type,
        double_type,
        long_double_type,
        cstring_type,
        string_type,
        pointer_type,
        custom_type
    };

    // 类型到 arg_type 的映射
    template <typename T, typename CharT>
    struct type_to_arg_type {
        static constexpr arg_type value = []() {
            using U = std::decay_t<T>;
            if constexpr (std::is_same_v<U, bool>) {
                return arg_type::bool_type;
            } else if constexpr (std::is_same_v<U, CharT>) {
                return arg_type::char_type;
            } else if constexpr (std::is_same_v<U, int>) {
                return arg_type::int_type;
            } else if constexpr (std::is_same_v<U, unsigned int>) {
                return arg_type::uint_type;
            } else if constexpr (std::is_same_v<U, long long>) {
                return arg_type::long_long_type;
            } else if constexpr (std::is_same_v<U, unsigned long long>) {
                return arg_type::ulong_long_type;
            } else if constexpr (std::is_same_v<U, float>) {
                return arg_type::float_type;
            } else if constexpr (std::is_same_v<U, double>) {
                return arg_type::double_type;
            } else if constexpr (std::is_same_v<U, long double>) {
                return arg_type::long_double_type;
            } else if constexpr (std::is_same_v<U, const CharT *>) {
                return arg_type::cstring_type;
            } else if constexpr (is_string_type_v<U, CharT>) {
                return arg_type::string_type;
            } else if constexpr (std::is_pointer_v<U>) {
                return arg_type::pointer_type;
            } else if constexpr (is_signed_integer_v<U>) {
                return arg_type::long_long_type;
            } else if constexpr (is_unsigned_integer_v<U>) {
                return arg_type::ulong_long_type;
            } else {
                return arg_type::custom_type;
            }
        }();
    };

    template <typename T, typename CharT>
    inline constexpr arg_type type_to_arg_type_v = type_to_arg_type<T, CharT>::value;

    // 检查类型是否为整数类型（用于动态宽度/精度）
    constexpr bool is_integral_arg_type(arg_type type) noexcept {
        return type == arg_type::int_type || type == arg_type::uint_type || type == arg_type::long_long_type ||
               type == arg_type::ulong_long_type;
    }

    // 检查类型是否为字符串类型
    constexpr bool is_string_arg_type(arg_type type) noexcept {
        return type == arg_type::cstring_type || type == arg_type::string_type;
    }

    // 动态规范约束
    struct dynamic_spec_constraint {
        enum class constraint_type {
            none, // 无约束
            integral, // 必须是整数类型
            string, // 必须是字符串类型
            any // 任意类型
        };

        constraint_type type = constraint_type::none;
        size_t source_arg_id = static_cast<size_t>(-1); // 引入此约束的参数ID

        constexpr dynamic_spec_constraint() noexcept = default;

        constexpr dynamic_spec_constraint(constraint_type t, size_t id) noexcept : type(t), source_arg_id(id) {
        }
    };

    template <typename>
    struct is_format_arg_store : std::false_type {};

    template <class Context, class... Args>
    struct is_format_arg_store<__format_arg_store<Context, Args...>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_format_arg_store_v = is_format_arg_store<T>::value;
}

namespace rainy::foundation::text {
    template <class Context>
    class basic_format_arg {
    public:
        class handle;
        using char_type = typename Context::char_type;

        basic_format_arg() noexcept : value_(std::monostate{}), type_(implements::arg_type::none_type) {
        }

        // 布尔类型
        explicit basic_format_arg(bool v) noexcept : value_(v), type_(implements::arg_type::bool_type) {
        }

        // 字符类型
        explicit basic_format_arg(char_type v) noexcept : value_(v), type_(implements::arg_type::char_type) {
        }

        // 整数类型
        explicit basic_format_arg(int v) noexcept : value_(v), type_(implements::arg_type::int_type) {
        }

        explicit basic_format_arg(unsigned int v) noexcept : value_(v), type_(implements::arg_type::uint_type) {
        }

        explicit basic_format_arg(long long int v) noexcept : value_(v), type_(implements::arg_type::long_long_type) {
        }

        explicit basic_format_arg(unsigned long long int v) noexcept : value_(v), type_(implements::arg_type::ulong_long_type) {
        }

        // 其他整数类型的转换构造函数
        template <
            typename T,
            std::enable_if_t<implements::is_signed_integer_v<T> && !std::is_same_v<T, int> && !std::is_same_v<T, long long int>, int> = 0>
        explicit basic_format_arg(T v) noexcept : value_(static_cast<long long int>(v)), type_(implements::arg_type::long_long_type) {
        }

        template <typename T, std::enable_if_t<implements::is_unsigned_integer_v<T> && !std::is_same_v<T, unsigned int> &&
                                                   !std::is_same_v<T, unsigned long long int>,
                                               int> = 0>
        explicit basic_format_arg(T v) noexcept :
            value_(static_cast<unsigned long long int>(v)), type_(implements::arg_type::ulong_long_type) {
        }

        // 浮点类型
        explicit basic_format_arg(float v) noexcept : value_(v), type_(implements::arg_type::float_type) {
        }

        explicit basic_format_arg(double v) noexcept : value_(v), type_(implements::arg_type::double_type) {
        }

        explicit basic_format_arg(long double v) noexcept : value_(v), type_(implements::arg_type::long_double_type) {
        }

        // 字符串类型
        explicit basic_format_arg(const char_type *s) : value_(s), type_(implements::arg_type::cstring_type) {
        }

        template <class traits>
        explicit basic_format_arg(std::basic_string_view<char_type, traits> s) noexcept :
            value_(basic_string_view<char_type>(s.data(), s.size())), type_(implements::arg_type::string_type) {
        }

        template <class traits, class Allocator>
        explicit basic_format_arg(const std::basic_string<char_type, traits, Allocator> &s) noexcept :
            value_(basic_string_view<char_type>(s.data(), s.size())), type_(implements::arg_type::string_type) {
        }

        // 指针类型
        explicit basic_format_arg(std::nullptr_t) noexcept :
            value_(static_cast<const void *>(nullptr)), type_(implements::arg_type::pointer_type) {
        }

        template <class T>
        explicit basic_format_arg(T *p) noexcept : value_(static_cast<const void *>(p)), type_(implements::arg_type::pointer_type) {
        }

        template <typename T,
                  std::enable_if_t<!std::is_same_v<std::decay_t<T>, bool> && !std::is_same_v<std::decay_t<T>, char_type> &&
                                       !std::is_integral_v<std::decay_t<T>> && !std::is_floating_point_v<std::decay_t<T>> &&
                                       !std::is_pointer_v<std::decay_t<T>> && !implements::is_string_type_v<std::decay_t<T>, char_type> &&
                                       !implements::is_format_arg_store_v<std::decay_t<T>>,
                                   int> = 0>
        explicit basic_format_arg(T &&v) noexcept : value_(handle(std::forward<T>(v))), type_(implements::arg_type::custom_type) {
        }

        explicit operator bool() const noexcept {
            return !std::holds_alternative<std::monostate>(value_);
        }

        // 获取类型信息
        implements::arg_type get_type() const noexcept {
            return type_;
        }

        // 检查是否为整数类型
        bool is_integral() const noexcept {
            return implements::is_integral_arg_type(type_);
        }

        // 检查是否为字符串类型
        bool is_string() const noexcept {
            return implements::is_string_arg_type(type_);
        }

        // visit 实现
        template <typename Visitor>
        decltype(auto) visit(Visitor &&vis) {
            return std::visit(std::forward<Visitor>(vis), value_);
        }

        template <typename R, typename Visitor>
        R visit(Visitor &&vis) {
            return std::visit<R>(std::forward<Visitor>(vis), value_);
        }

    private:
        std::variant<std::monostate, bool, char_type, int, unsigned int, long long int, unsigned long long int, float, double,
                     long double, const char_type *, basic_string<char_type>, basic_string_view<char_type>, const void *, handle>
            value_;

        // 存储类型信息用于运行时检查
        implements::arg_type type_;
    };

    template <class Context>
    class basic_format_arg<Context>::handle {
    private:
        using char_type = typename Context::char_type;

        const void *ptr_;
        void (*format_)(basic_format_parse_context<char_type> &, Context &, const void *);

        template <class T>
        static void format_impl(basic_format_parse_context<char_type> &parse_ctx, Context &format_ctx, const void *ptr) {
            typename Context::template formatter_type<T> f;
            parse_ctx.advance_to(f.parse(parse_ctx));
            format_ctx.advance_to(f.format(*static_cast<const T *>(ptr), format_ctx));
        }

    public:
        template <class T>
        explicit handle(T &&val) noexcept : ptr_(std::addressof(val)), format_(format_impl<std::remove_reference_t<T>>) {
        }

        void format(basic_format_parse_context<char_type> &parse_ctx, Context &ctx) const {
            format_(parse_ctx, ctx, ptr_);
        }

        friend class basic_format_arg<Context>;
    };

    template <class Context, class... Args>
    class __format_arg_store {
    public:
        std::array<basic_format_arg<Context>, sizeof...(Args)> args;

        template <size_t... Is>
        constexpr __format_arg_store(std::index_sequence<Is...>, const Args &...values) noexcept :
            args{basic_format_arg<Context>(values)...} {
        }
    };

    template <class Context>
    class basic_format_args {
    private:
        size_t size_;
        const basic_format_arg<Context> *data_;

    public:
        basic_format_args() noexcept : size_(0), data_(nullptr) {
        }

        template <typename... Args>
        basic_format_args(const __format_arg_store<Context, Args...> &store) noexcept : // NOLINT
            size_(sizeof...(Args)), data_(store.args.data()) {
        }

        basic_format_arg<Context> get(size_t i) const noexcept {
            if (i < size_) {
                return data_[i];
            }
            return basic_format_arg<Context>{};
        }

        RAINY_NODISCARD std::size_t size() const noexcept {
            return size_;
        }
    };
}

#endif