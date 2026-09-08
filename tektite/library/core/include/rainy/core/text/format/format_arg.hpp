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
#ifndef RAINY_YESOD_TEXT_FORMAT_FORMAT_ARG_HPP
#define RAINY_YESOD_TEXT_FORMAT_FORMAT_ARG_HPP
#include <rainy/core/container/variant.hpp>
#include <rainy/core/text/string.hpp>
#include <string_view>
#include <variant>

namespace rainy::core::text {
    template <typename Context, typename... Args>
    class __format_arg_store;

    template <typename CharType>
    class basic_format_parse_context;
}

namespace rainy::core::text::implements {
    // 检查类型是否为有符号整数
    template <typename Ty>
    struct is_signed_integer
        : type_traits::helper::bool_constant<
              type_traits::primary_types::is_integral_v<Ty> && type_traits::properties::is_signed_v<Ty> &&
              !type_traits::type_relations::is_same_v<Ty, bool> && !type_traits::type_relations::is_same_v<Ty, char> &&
              !type_traits::type_relations::is_same_v<Ty, wchar_t> && !type_traits::type_relations::is_same_v<Ty, char16_t> &&
              !type_traits::type_relations::is_same_v<Ty, char32_t>> {};

    template <typename Ty>
    inline constexpr bool is_signed_integer_v = is_signed_integer<Ty>::value;

    // 检查类型是否为无符号整数
    template <typename Ty>
    struct is_unsigned_integer : type_traits::helper::bool_constant<type_traits::primary_types::is_integral_v<Ty> &&
                                                                    type_traits::properties::is_unsigned_v<Ty> &&
                                                                    !type_traits::type_relations::is_same_v<Ty, bool>> {};

    template <typename Ty>
    inline constexpr bool is_unsigned_integer_v = is_unsigned_integer<Ty>::value;

    // 检查类型是否为字符类型
    template <typename Ty, typename CharType>
    struct is_char_type : type_traits::helper::bool_constant<type_traits::type_relations::is_same_v<Ty, CharType>> {};

    template <typename Ty, typename CharType>
    inline constexpr bool is_char_type_v = is_char_type<Ty, CharType>::value;

    // 检查类型是否为字符串类型
    template <typename Ty, typename CharType>
    struct is_string_type
        : type_traits::helper::bool_constant<type_traits::type_relations::is_same_v<Ty, const CharType *> ||
                                             type_traits::type_relations::is_same_v<Ty, std::basic_string_view<CharType>> ||
                                             type_traits::type_relations::is_convertible_v<Ty, std::basic_string_view<CharType>>> {};

    template <typename Ty, typename CharType>
    inline constexpr bool is_string_type_v = is_string_type<Ty, CharType>::value;

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
    template <typename Ty, typename CharType>
    struct type_to_arg_type {
        static constexpr arg_type value = []() {
            using U = type_traits::other_trans::decay_t<Ty>;
            if constexpr (type_traits::type_relations::is_same_v<U, bool>) {
                return arg_type::bool_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, CharType>) {
                return arg_type::char_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, int>) {
                return arg_type::int_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, unsigned int>) {
                return arg_type::uint_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, long long>) {
                return arg_type::long_long_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, unsigned long long>) {
                return arg_type::ulong_long_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, float>) {
                return arg_type::float_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, double>) {
                return arg_type::double_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, long double>) {
                return arg_type::long_double_type;
            } else if constexpr (type_traits::type_relations::is_same_v<U, const CharType *>) {
                return arg_type::cstring_type;
            } else if constexpr (is_string_type_v<U, CharType>) {
                return arg_type::string_type;
            } else if constexpr (type_traits::primary_types::is_pointer_v<U>) {
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

    template <typename Ty, typename CharType>
    inline constexpr arg_type type_to_arg_type_v = type_to_arg_type<Ty, CharType>::value;

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
    struct is_format_arg_store : type_traits::helper::false_type {};

    template <typename Context, typename... Args>
    struct is_format_arg_store<__format_arg_store<Context, Args...>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_format_arg_store_v = is_format_arg_store<Ty>::value;
}

namespace rainy::core::text {
    template <typename Context>
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
        template <typename Ty, type_traits::other_trans::enable_if_t<implements::is_signed_integer_v<Ty> &&
                                                                         !type_traits::type_relations::is_same_v<Ty, int> &&
                                                                         !type_traits::type_relations::is_same_v<Ty, long long int>,
                                                                     int> = 0>
        explicit basic_format_arg(Ty v) noexcept : value_(static_cast<long long int>(v)), type_(implements::arg_type::long_long_type) {
        }

        template <typename Ty,
                  type_traits::other_trans::enable_if_t<implements::is_unsigned_integer_v<Ty> &&
                                                            !type_traits::type_relations::is_same_v<Ty, unsigned int> &&
                                                            !type_traits::type_relations::is_same_v<Ty, unsigned long long int>,
                                                        int> = 0>
        explicit basic_format_arg(Ty v) noexcept :
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

        template <typename traits>
        explicit basic_format_arg(std::basic_string_view<char_type, traits> s) noexcept :
            value_(basic_string_view<char_type>(s.data(), s.size())), type_(implements::arg_type::string_type) {
        }

        template <typename traits, typename Allocator>
        explicit basic_format_arg(const std::basic_string<char_type, traits, Allocator> &s) noexcept :
            value_(basic_string_view<char_type>(s.data(), s.size())), type_(implements::arg_type::string_type) {
        }

        // 指针类型
        explicit basic_format_arg(std::nullptr_t) noexcept :
            value_(static_cast<const void *>(nullptr)), type_(implements::arg_type::pointer_type) {
        }

        template <typename Ty>
        explicit basic_format_arg(Ty *p) noexcept : value_(static_cast<const void *>(p)), type_(implements::arg_type::pointer_type) {
        }

        template <typename Ty, type_traits::other_trans::enable_if_t<
                                   !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Ty>, bool> &&
                                       !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Ty>, char_type> &&
                                       !type_traits::primary_types::is_integral_v<type_traits::other_trans::decay_t<Ty>> &&
                                       !type_traits::primary_types::is_floating_point_v<type_traits::other_trans::decay_t<Ty>> &&
                                       !type_traits::primary_types::is_pointer_v<type_traits::other_trans::decay_t<Ty>> &&
                                       !implements::is_string_type_v<type_traits::other_trans::decay_t<Ty>, char_type> &&
                                       !implements::is_format_arg_store_v<type_traits::other_trans::decay_t<Ty>>,
                                   int> = 0>
        explicit basic_format_arg(Ty &&v) noexcept :
            value_(handle(utility::forward<Ty>(v))), type_(implements::arg_type::custom_type) {
        }

        explicit operator bool() const noexcept {
            return !core::container::holds_alternative<std::monostate>(value_);
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
            return core::container::visit(utility::forward<Visitor>(vis), value_);
        }

        template <typename R, typename Visitor>
        R visit(Visitor &&vis) {
            return core::container::visit<R>(utility::forward<Visitor>(vis), value_);
        }

    private:
        container::variant<std::monostate, bool, char_type, int, unsigned int, long long int, unsigned long long int, float, double,
                           long double, const char_type *, basic_string<char_type>, basic_string_view<char_type>, const void *, handle>
            value_;

        // 存储类型信息用于运行时检查
        implements::arg_type type_;
    };

    template <typename Context>
    class basic_format_arg<Context>::handle {
    private:
        using char_type = typename Context::char_type;

        const void *ptr_;
        void (*format_)(basic_format_parse_context<char_type> &, Context &, const void *);

        template <typename Ty>
        static void format_impl(basic_format_parse_context<char_type> &parse_ctx, Context &format_ctx, const void *ptr) {
            typename Context::template formatter_type<type_traits::modifers::remove_cvref_t<Ty>> f;
            parse_ctx.advance_to(f.parse(parse_ctx));
            format_ctx.advance_to(f.format(*static_cast<const Ty *>(ptr), format_ctx));
        }

    public:
        template <typename Ty>
        explicit handle(Ty &&val) noexcept :
            ptr_(utility::addressof(val)), format_(format_impl<type_traits::modifers::remove_reference_t<Ty>>) {
        }

        void format(basic_format_parse_context<char_type> &parse_ctx, Context &ctx) const {
            format_(parse_ctx, ctx, ptr_);
        }

        friend class basic_format_arg<Context>;
    };

    template <typename Context, typename... Args>
    class __format_arg_store {
    public:
        template <size_t... Is>
        constexpr __format_arg_store(type_traits::helper::index_sequence<Is...>, const Args &...values) noexcept :
            args{basic_format_arg<Context>(values)...} {
        }

        collections::array<basic_format_arg<Context>, sizeof...(Args)> args;
    };

    template <typename Context>
    class basic_format_args {
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

    private:
        size_t size_;
        const basic_format_arg<Context> *data_;
    };
}

namespace rainy::text {
    using core::text::basic_format_arg;
    using core::text::basic_format_args;
}

#endif
