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
    /**
     * \lang english
     * @brief A type-erased holder for a single format argument.
     *
     * @tparam Context The format context type
     *
     * \lang simp-chinese
     * @brief 单个格式化参数的类型擦除容器。
     *
     * @tparam Context 格式化上下文类型
     */
    class basic_format_arg {
    public:
        /**
         * \lang english
         * @brief Handle that forwards formatting of a custom type to its formatter.
         *
         * \lang simp-chinese
         * @brief 将自定义类型的格式化转发给其 formatter 的句柄。
         */
        class handle;
        /**
         * \lang english
         * @brief The character type of the context.
         *
         * \lang simp-chinese
         * @brief 上下文的字符类型。
         */
        using char_type = typename Context::char_type;

        /**
         * \lang english
         * @brief Constructs an empty format argument holding no value.
         *
         * \lang simp-chinese
         * @brief 构造不持有任何值的空格式化参数。
         */
        basic_format_arg() noexcept : value_(std::monostate{}), type_(implements::arg_type::none_type) {
        }

        // 布尔类型
        /**
         * \lang english
         * @brief Constructs an argument from a bool value.
         *
         * @param v The bool value
         *
         * \lang simp-chinese
         * @brief 由 bool 值构造参数。
         *
         * @param v bool 值
         */
        explicit basic_format_arg(bool v) noexcept : value_(v), type_(implements::arg_type::bool_type) {
        }

        // 字符类型
        /**
         * \lang english
         * @brief Constructs an argument from a character value.
         *
         * @param v The character value
         *
         * \lang simp-chinese
         * @brief 由字符值构造参数。
         *
         * @param v 字符值
         */
        explicit basic_format_arg(char_type v) noexcept : value_(v), type_(implements::arg_type::char_type) {
        }

        // 整数类型
        /**
         * \lang english
         * @brief Constructs an argument from an int value.
         *
         * @param v The int value
         *
         * \lang simp-chinese
         * @brief 由 int 值构造参数。
         *
         * @param v int 值
         */
        explicit basic_format_arg(int v) noexcept : value_(v), type_(implements::arg_type::int_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from an unsigned int value.
         *
         * @param v The unsigned int value
         *
         * \lang simp-chinese
         * @brief 由 unsigned int 值构造参数。
         *
         * @param v unsigned int 值
         */
        explicit basic_format_arg(unsigned int v) noexcept : value_(v), type_(implements::arg_type::uint_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from a long long value.
         *
         * @param v The long long value
         *
         * \lang simp-chinese
         * @brief 由 long long 值构造参数。
         *
         * @param v long long 值
         */
        explicit basic_format_arg(long long int v) noexcept : value_(v), type_(implements::arg_type::long_long_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from an unsigned long long value.
         *
         * @param v The unsigned long long value
         *
         * \lang simp-chinese
         * @brief 由 unsigned long long 值构造参数。
         *
         * @param v unsigned long long 值
         */
        explicit basic_format_arg(unsigned long long int v) noexcept : value_(v), type_(implements::arg_type::ulong_long_type) {
        }

        // 其他整数类型的转换构造函数
        /**
         * \lang english
         * @brief Constructs an argument from any other signed integer type, widened to long long.
         *
         * @tparam Ty The signed integer type
         * @param v The value
         *
         * \lang simp-chinese
         * @brief 由任意其他有符号整数类型构造参数，并加宽为 long long。
         *
         * @tparam Ty 有符号整数类型
         * @param v 值
         */
        template <typename Ty, type_traits::other_trans::enable_if_t<implements::is_signed_integer_v<Ty> &&
                                                                         !type_traits::type_relations::is_same_v<Ty, int> &&
                                                                         !type_traits::type_relations::is_same_v<Ty, long long int>,
                                                                     int> = 0>
        explicit basic_format_arg(Ty v) noexcept : value_(static_cast<long long int>(v)), type_(implements::arg_type::long_long_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from any other unsigned integer type, widened to unsigned long long.
         *
         * @tparam Ty The unsigned integer type
         * @param v The value
         *
         * \lang simp-chinese
         * @brief 由任意其他无符号整数类型构造参数，并加宽为 unsigned long long。
         *
         * @tparam Ty 无符号整数类型
         * @param v 值
         */
        template <typename Ty,
                  type_traits::other_trans::enable_if_t<implements::is_unsigned_integer_v<Ty> &&
                                                            !type_traits::type_relations::is_same_v<Ty, unsigned int> &&
                                                            !type_traits::type_relations::is_same_v<Ty, unsigned long long int>,
                                                        int> = 0>
        explicit basic_format_arg(Ty v) noexcept :
            value_(static_cast<unsigned long long int>(v)), type_(implements::arg_type::ulong_long_type) {
        }

        // 浮点类型
        /**
         * \lang english
         * @brief Constructs an argument from a float value.
         *
         * @param v The float value
         *
         * \lang simp-chinese
         * @brief 由 float 值构造参数。
         *
         * @param v float 值
         */
        explicit basic_format_arg(float v) noexcept : value_(v), type_(implements::arg_type::float_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from a double value.
         *
         * @param v The double value
         *
         * \lang simp-chinese
         * @brief 由 double 值构造参数。
         *
         * @param v double 值
         */
        explicit basic_format_arg(double v) noexcept : value_(v), type_(implements::arg_type::double_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from a long double value.
         *
         * @param v The long double value
         *
         * \lang simp-chinese
         * @brief 由 long double 值构造参数。
         *
         * @param v long double 值
         */
        explicit basic_format_arg(long double v) noexcept : value_(v), type_(implements::arg_type::long_double_type) {
        }

        // 字符串类型
        /**
         * \lang english
         * @brief Constructs an argument from a null-terminated character pointer.
         *
         * @param s The null-terminated character pointer
         *
         * \lang simp-chinese
         * @brief 由以空字符结尾的字符指针构造参数。
         *
         * @param s 以空字符结尾的字符指针
         */
        explicit basic_format_arg(const char_type *s) : value_(s), type_(implements::arg_type::cstring_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from a std::basic_string_view.
         *
         * @tparam traits The string view traits type
         * @param s The string view
         *
         * \lang simp-chinese
         * @brief 由 std::basic_string_view 构造参数。
         *
         * @tparam traits 字符串视图的 traits 类型
         * @param s 字符串视图
         */
        template <typename traits>
        explicit basic_format_arg(std::basic_string_view<char_type, traits> s) noexcept :
            value_(basic_string_view<char_type>(s.data(), s.size())), type_(implements::arg_type::string_type) {
        }

        /**
         * \lang english
         * @brief Constructs an argument from a std::basic_string.
         *
         * @tparam traits The string traits type
         * @tparam Allocator The string allocator type
         * @param s The string
         *
         * \lang simp-chinese
         * @brief 由 std::basic_string 构造参数。
         *
         * @tparam traits 字符串的 traits 类型
         * @tparam Allocator 字符串的分配器类型
         * @param s 字符串
         */
        template <typename traits, typename Allocator>
        explicit basic_format_arg(const std::basic_string<char_type, traits, Allocator> &s) noexcept :
            value_(basic_string_view<char_type>(s.data(), s.size())), type_(implements::arg_type::string_type) {
        }

        // 指针类型
        /**
         * \lang english
         * @brief Constructs a null pointer argument.
         *
         * @param v The null pointer
         *
         * \lang simp-chinese
         * @brief 构造空指针参数。
         *
         * @param v 空指针
         */
        explicit basic_format_arg(std::nullptr_t) noexcept :
            value_(static_cast<const void *>(nullptr)), type_(implements::arg_type::pointer_type) {
        }

        template <typename Ty>
        /**
         * \lang english
         * @brief Constructs an argument from an arbitrary object pointer.
         *
         * @tparam Ty The pointed-to type
         * @param p The pointer
         *
         * \lang simp-chinese
         * @brief 由任意对象指针构造参数。
         *
         * @tparam Ty 所指向的类型
         * @param p 指针
         */
        explicit basic_format_arg(Ty *p) noexcept : value_(static_cast<const void *>(p)), type_(implements::arg_type::pointer_type) {
        }

        /**
         * \lang english
         * @brief Constructs a custom-type argument that is formatted through its formatter.
         *
         * @tparam Ty The custom type
         * @param v The value to store
         *
         * \lang simp-chinese
         * @brief 构造自定义类型参数，该参数通过其 formatter 进行格式化。
         *
         * @tparam Ty 自定义类型
         * @param v 待存储的值
         */
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

        /**
         * \lang english
         * @brief Checks whether the argument holds a value.
         *
         * @return true if the argument is not empty
         *
         * \lang simp-chinese
         * @brief 检查参数是否持有值。
         *
         * @return 参数非空时返回 true
         */
        explicit operator bool() const noexcept {
            return !core::container::holds_alternative<std::monostate>(value_);
        }

        // 获取类型信息
        /**
         * \lang english
         * @brief Returns the runtime type information of the stored argument.
         *
         * @return The argument type
         *
         * \lang simp-chinese
         * @brief 返回所存储参数的运行时类型信息。
         *
         * @return 参数类型
         */
        implements::arg_type get_type() const noexcept {
            return type_;
        }

        // 检查是否为整数类型
        /**
         * \lang english
         * @brief Checks whether the argument holds an integral type.
         *
         * @return true if the argument is integral
         *
         * \lang simp-chinese
         * @brief 检查参数是否持有整数类型。
         *
         * @return 参数为整数类型时返回 true
         */
        bool is_integral() const noexcept {
            return implements::is_integral_arg_type(type_);
        }

        // 检查是否为字符串类型
        /**
         * \lang english
         * @brief Checks whether the argument holds a string type.
         *
         * @return true if the argument is a string
         *
         * \lang simp-chinese
         * @brief 检查参数是否持有字符串类型。
         *
         * @return 参数为字符串类型时返回 true
         */
        bool is_string() const noexcept {
            return implements::is_string_arg_type(type_);
        }

        // visit 实现
        /**
         * \lang english
         * @brief Applies a visitor to the stored value.
         *
         * @tparam Visitor The visitor type
         * @param vis The visitor to apply
         * @return The result of the visitor
         *
         * \lang simp-chinese
         * @brief 对存储的值应用访问器。
         *
         * @tparam Visitor 访问器类型
         * @param vis 应用的访问器
         * @return 访问器的返回结果
         */
        template <typename Visitor>
        decltype(auto) visit(Visitor &&vis) {
            return core::container::visit(utility::forward<Visitor>(vis), value_);
        }

        /**
         * \lang english
         * @brief Applies a visitor to the stored value with an explicit result type.
         *
         * @tparam R The result type
         * @tparam Visitor The visitor type
         * @param vis The visitor to apply
         * @return The visitor result converted to R
         *
         * \lang simp-chinese
         * @brief 以显式结果类型对存储的值应用访问器。
         *
         * @tparam R 结果类型
         * @tparam Visitor 访问器类型
         * @param vis 应用的访问器
         * @return 转换为 R 的访问器结果
         */
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

    /**
     * \lang english
     * @brief Stores a pointer to a custom value and forwards formatting to its formatter.
     *
     * @tparam Context The format context type
     *
     * \lang simp-chinese
     * @brief 存储指向自定义值的指针，并将格式化转发给其 formatter。
     *
     * @tparam Context 格式化上下文类型
     */
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
        /**
         * \lang english
         * @brief Constructs a handle referring to the given value.
         *
         * @tparam Ty The value type
         * @param val The value to refer to
         *
         * \lang simp-chinese
         * @brief 构造引用给定值的句柄。
         *
         * @tparam Ty 值类型
         * @param val 被引用的值
         */
        template <typename Ty>
        explicit handle(Ty &&val) noexcept :
            ptr_(utility::addressof(val)), format_(format_impl<type_traits::modifers::remove_reference_t<Ty>>) {
        }

        /**
         * \lang english
         * @brief Formats the referenced value into the given context.
         *
         * @param parse_ctx The parse context
         * @param ctx The format context
         *
         * \lang simp-chinese
         * @brief 将引用的值格式化到给定的上下文中。
         *
         * @param parse_ctx 解析上下文
         * @param ctx 格式化上下文
         */
        void format(basic_format_parse_context<char_type> &parse_ctx, Context &ctx) const {
            format_(parse_ctx, ctx, ptr_);
        }

        friend class basic_format_arg<Context>;
    };

    /**
     * \lang english
     * @brief Stores a fixed set of format arguments for a given context.
     *
     * @tparam Context The format context type
     * @tparam Args The argument types
     *
     * \lang simp-chinese
     * @brief 为给定上下文存储一组固定的格式化参数。
     *
     * @tparam Context 格式化上下文类型
     * @tparam Args 参数类型
     */
    template <typename Context, typename... Args>
    class __format_arg_store {
    public:
        /**
         * \lang english
         * @brief Constructs the store from the argument values.
         *
         * @tparam Is The index sequence
         * @param values The argument values
         *
         * \lang simp-chinese
         * @brief 由参数值构造存储。
         *
         * @tparam Is 索引序列
         * @param values 参数值
         */
        template <size_t... Is>
        constexpr __format_arg_store(type_traits::helper::index_sequence<Is...>, const Args &...values) noexcept :
            args{basic_format_arg<Context>(values)...} {
        }

        collections::array<basic_format_arg<Context>, sizeof...(Args)> args;
    };

    template <typename Context>
    /**
     * \lang english
     * @brief A non-owning view over a sequence of format arguments.
     *
     * @tparam Context The format context type
     *
     * \lang simp-chinese
     * @brief 对一组格式化参数的非拥有视图。
     *
     * @tparam Context 格式化上下文类型
     */
    class basic_format_args {
    public:
        /**
         * \lang english
         * @brief Constructs an empty argument list.
         *
         * \lang simp-chinese
         * @brief 构造空的参数列表。
         */
        basic_format_args() noexcept : size_(0), data_(nullptr) {
        }

        /**
         * \lang english
         * @brief Constructs a view over the arguments in the given store.
         *
         * @tparam Args The argument types
         * @param store The argument store
         *
         * \lang simp-chinese
         * @brief 在给定存储之上构造参数视图。
         *
         * @tparam Args 参数类型
         * @param store 参数存储
         */
        template <typename... Args>
        basic_format_args(const __format_arg_store<Context, Args...> &store) noexcept : // NOLINT
            size_(sizeof...(Args)), data_(store.args.data()) {
        }

        /**
         * \lang english
         * @brief Returns the argument at the given index, or an empty argument if out of range.
         *
         * @param i The argument index
         * @return The format argument
         *
         * \lang simp-chinese
         * @brief 返回指定索引处的参数；越界时返回空参数。
         *
         * @param i 参数索引
         * @return 格式化参数
         */
        basic_format_arg<Context> get(size_t i) const noexcept {
            if (i < size_) {
                return data_[i];
            }
            return basic_format_arg<Context>{};
        }

        /**
         * \lang english
         * @brief Returns the number of arguments in the list.
         *
         * @return The argument count
         *
         * \lang simp-chinese
         * @brief 返回列表中参数的数量。
         *
         * @return 参数数量
         */
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
