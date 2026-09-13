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
#ifndef RAINY_TEXT_FORMAT_CONTEXT_HPP
#define RAINY_TEXT_FORMAT_CONTEXT_HPP
#include <locale>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/text/format/format_arg.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/container/optional.hpp>

namespace rainy::core::exceptions::runtime {
    /**
     * \lang english
     * @brief Exception type thrown when a format string is invalid or an argument cannot be formatted.
     *
     * \lang simp-chinese
     * @brief 当格式字符串无效或参数无法格式化时抛出的异常类型。
     */
    RAINY_DEFINE_EXCEPTION_WITH_THROW(format_error, runtime_error, "bad format_error", throw_format_error)
}

namespace rainy::core::text {
    template <typename Context>
    class basic_format_args;

    template <typename Context>
    class basic_format_arg;

    /**
     * \lang english
     * @brief Primary template for user-defined formatters; must be specialized for each formattable type.
     *
     * @tparam Ty The type to format
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 用户自定义 formatter 的主模板；必须为每个可格式化类型进行特化。
     *
     * @tparam Ty 待格式化的类型
     * @tparam CharType 字符类型
     */
    template <typename Ty, typename CharType, typename = void>
    struct formatter {
        static_assert(sizeof(Ty) == 0, "formatter must be specialized for this type");
    };

    template <typename CharType>
    /**
     * \lang english
     * @brief Provides access to the format string being parsed during formatting.
     *
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 在格式化期间提供对正在解析的格式字符串的访问。
     *
     * @tparam CharType 字符类型
     */
    class basic_format_parse_context {
    public:
        /**
         * \lang english
         * @brief The character type of the format string.
         *
         * \lang simp-chinese
         * @brief 格式字符串的字符类型。
         */
        /**
         * \lang english
         * @brief The character type.
         *
         * \lang simp-chinese
         * @brief 字符类型。
         */
        using char_type = CharType;
        /**
         * \lang english
         * @brief The const iterator type over the format string.
         *
         * \lang simp-chinese
         * @brief 遍历格式字符串的常量迭代器类型。
         */
        using const_iterator = typename basic_string_view<CharType>::const_iterator;
        /**
         * \lang english
         * @brief The iterator type used by formatters; an alias of const_iterator.
         *
         * \lang simp-chinese
         * @brief formatter 使用的迭代器类型；即 const_iterator 的别名。
         */
        using iterator = const_iterator;

        /**
         * \lang english
         * @brief Constructs a parse context over the given format string.
         *
         * @param fmt The format string
         * @param num_args The number of arguments available for dynamic format specs
         *
         * \lang simp-chinese
         * @brief 基于给定的格式字符串构造解析上下文。
         *
         * @param fmt 格式字符串
         * @param num_args 可用于动态格式规范的参数数量
         */
        constexpr explicit basic_format_parse_context(basic_string_view<CharType> fmt, size_t num_args = 0) noexcept :
            fmt_(fmt), num_args_(num_args), next_arg_id_(0), manual_indexing_(false) {
        }

        /**
         * \lang english
         * @brief Copy constructor is deleted; parse contexts are non-copyable.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数被删除；解析上下文不可拷贝。
         */
        basic_format_parse_context(const basic_format_parse_context &) = delete;
        /**
         * \lang english
         * @brief Copy assignment is deleted; parse contexts are non-copyable.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值被删除；解析上下文不可拷贝。
         */
        basic_format_parse_context &operator=(const basic_format_parse_context &) = delete;

        /**
         * \lang english
         * @brief Returns an iterator to the beginning of the format string.
         *
         * @return Iterator to the beginning
         *
         * \lang simp-chinese
         * @brief 返回指向格式字符串起始位置的迭代器。
         *
         * @return 指向起始位置的迭代器
         */
        RAINY_NODISCARD constexpr const_iterator begin() const noexcept {
            return fmt_.begin();
        }

        /**
         * \lang english
         * @brief Returns an iterator past the end of the format string.
         *
         * @return Iterator past the end
         *
         * \lang simp-chinese
         * @brief 返回指向格式字符串末尾之后的迭代器。
         *
         * @return 指向末尾之后的迭代器
         */
        RAINY_NODISCARD constexpr const_iterator end() const noexcept {
            return fmt_.end();
        }

        /**
         * \lang english
         * @brief Advances the parse position to the given iterator.
         *
         * @param it The new parse position
         *
         * \lang simp-chinese
         * @brief 将解析位置推进到给定的迭代器。
         *
         * @param it 新的解析位置
         */
        constexpr void advance_to(const_iterator it) {
            fmt_ = basic_string_view<CharType>(it, fmt_.end());
        }

        /**
         * \lang english
         * @brief Returns and increments the automatic argument index.
         *
         * @return The next automatic argument index
         *
         * \lang simp-chinese
         * @brief 返回并递增自动参数索引。
         *
         * @return 下一个自动参数索引
         */
        RAINY_NODISCARD constexpr size_t next_arg_id() {
            if (manual_indexing_) {
                core::exceptions::runtime::throw_format_error("cannot switch from manual to automatic argument indexing");
            }
            return next_arg_id_++;
        }

        /**
         * \lang english
         * @brief Validates a manually specified argument index.
         *
         * @param id The argument index to validate
         *
         * \lang simp-chinese
         * @brief 校验手动指定的参数索引。
         *
         * @param id 待校验的参数索引
         */
        constexpr void check_arg_id(size_t id) {
            if (!manual_indexing_ && next_arg_id_ > 0) {
                core::exceptions::runtime::throw_format_error("cannot switch from automatic to manual argument indexing");
            }
            if (id >= num_args_) {
                core::exceptions::runtime::throw_format_error("argument index out of range");
            }
            manual_indexing_ = true;
        }

        /**
         * \lang english
         * @brief Validates the argument index used by an integral dynamic format spec.
         *
         * @tparam Args The argument types
         * @param id The argument index to validate
         *
         * \lang simp-chinese
         * @brief 校验整数动态格式规范使用的参数索引。
         *
         * @tparam Args 参数类型
         * @param id 待校验的参数索引
         */
        template <typename... Args>
        constexpr void check_dynamic_spec_integral(size_t id) const {
            if (id >= num_args_) {
                core::exceptions::runtime::throw_format_error("dynamic spec argument index out of range");
            }
        }

        /**
         * \lang english
         * @brief Validates the argument index used by a string dynamic format spec.
         *
         * @tparam Args The argument types
         * @param id The argument index to validate
         *
         * \lang simp-chinese
         * @brief 校验字符串动态格式规范使用的参数索引。
         *
         * @tparam Args 参数类型
         * @param id 待校验的参数索引
         */
        template <typename... Args>
        constexpr void check_dynamic_spec_string(size_t id) const {
            if (id >= num_args_) {
                core::exceptions::runtime::throw_format_error("dynamic spec string argument index out of range");
            }
        }

    private:
        basic_string_view<CharType> fmt_;
        size_t num_args_;
        size_t next_arg_id_;
        bool manual_indexing_;
    };

    /**
     * \lang english
     * @brief Holds the output iterator, format arguments and optional locale during formatting.
     *
     * @tparam Out The output iterator type
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 在格式化期间持有输出迭代器、格式化参数与可选的 locale。
     *
     * @tparam Out 输出迭代器类型
     * @tparam CharType 字符类型
     */
    template <typename Out, class CharType>
    class basic_format_context {
    public:
        /**
         * \lang english
         * @brief The output iterator type.
         *
         * \lang simp-chinese
         * @brief 输出迭代器类型。
         */
        using iterator = Out;
        using char_type = CharType;

        /**
         * \lang english
         * @brief The formatter type used for a given value type.
         *
         * @tparam Ty The value type to format
         *
         * \lang simp-chinese
         * @brief 给定值类型所使用的 formatter 类型。
         *
         * @tparam Ty 待格式化的值类型
         */
        template <typename Ty>
        using formatter_type = formatter<Ty, CharType>;

    private:
        basic_format_args<basic_format_context> args_;
        Out out_;
        container::optional<std::locale> loc_; // 可选的 locale

        basic_format_context(const basic_format_context &) = delete;
        basic_format_context &operator=(const basic_format_context &) = delete;

    public:
        /**
         * \lang english
         * @brief Constructs a format context without a locale.
         *
         * @param out The output iterator
         * @param args The format arguments
         *
         * \lang simp-chinese
         * @brief 构造不带 locale 的格式化上下文。
         *
         * @param out 输出迭代器
         * @param args 格式化参数
         */
        basic_format_context(Out out, basic_format_args<basic_format_context> args) :
            args_(args), out_(utility::move(out)), loc_(std::nullopt) {
        }

        /**
         * \lang english
         * @brief Constructs a format context with the given locale.
         *
         * @param out The output iterator
         * @param args The format arguments
         * @param loc The locale to use
         *
         * \lang simp-chinese
         * @brief 使用给定的 locale 构造格式化上下文。
         *
         * @param out 输出迭代器
         * @param args 格式化参数
         * @param loc 使用的 locale
         */
        basic_format_context(Out out, basic_format_args<basic_format_context> args, const std::locale &loc) :
            args_(args), out_(utility::move(out)), loc_(loc) {
        }

        /**
         * \lang english
         * @brief Returns the format argument at the given index.
         *
         * @param id The argument index
         * @return The format argument, or an empty argument if out of range
         *
         * \lang simp-chinese
         * @brief 返回指定索引处的格式化参数。
         *
         * @param id 参数索引
         * @return 格式化参数；越界时返回空参数
         */
        basic_format_arg<basic_format_context> arg(size_t id) const noexcept {
            return args_.get(id);
        }

        /**
         * \lang english
         * @brief Returns the locale, or a default-constructed locale if none was provided.
         *
         * @return The locale
         *
         * \lang simp-chinese
         * @brief 返回 locale；未提供时返回默认构造的 locale。
         *
         * @return locale
         */
        std::locale locale() const {
            return loc_.value_or(std::locale());
        }

        /**
         * \lang english
         * @brief Moves the output iterator out of the context.
         *
         * @return The output iterator
         *
         * \lang simp-chinese
         * @brief 将输出迭代器移出上下文。
         *
         * @return 输出迭代器
         */
        iterator out() {
            return utility::move(out_);
        }

        /**
         * \lang english
         * @brief Replaces the stored output iterator with the given one.
         *
         * @param it The new output iterator
         *
         * \lang simp-chinese
         * @brief 用给定的迭代器替换存储的输出迭代器。
         *
         * @param it 新的输出迭代器
         */
        void advance_to(iterator it) {
            out_ = utility::move(it);
        }
    };

    template <typename CharType>
    /**
     * \lang english
     * @brief Output iterator that appends formatted characters to a basic_string.
     *
     * @tparam CharType The character type
     *
     * \lang simp-chinese
     * @brief 将格式化字符追加到 basic_string 的输出迭代器。
     *
     * @tparam CharType 字符类型
     */
    using format_buffer_iterator = utility::back_insert_iterator<basic_string<CharType>>;

    /**
     * \lang english
     * @brief The format context for char strings.
     *
     * \lang simp-chinese
     * @brief 面向 char 字符串的格式化上下文。
     */
    using format_context = basic_format_context<format_buffer_iterator<char>, char>;
    /**
     * \lang english
     * @brief The format context for wchar_t strings.
     *
     * \lang simp-chinese
     * @brief 面向 wchar_t 字符串的格式化上下文。
     */
    using wformat_context = basic_format_context<format_buffer_iterator<wchar_t>, wchar_t>;

    /**
     * \lang english
     * @brief The parse context for char format strings.
     *
     * \lang simp-chinese
     * @brief 面向 char 格式字符串的解析上下文。
     */
    using format_parse_context = basic_format_parse_context<char>;
    /**
     * \lang english
     * @brief The parse context for wchar_t format strings.
     *
     * \lang simp-chinese
     * @brief 面向 wchar_t 格式字符串的解析上下文。
     */
    using wformat_parse_context = basic_format_parse_context<wchar_t>;

    /**
     * \lang english
     * @brief The format argument holder for the char format context.
     *
     * \lang simp-chinese
     * @brief 面向 char 格式化上下文的格式化参数容器。
     */
    using format_args = basic_format_args<format_context>;
    /**
     * \lang english
     * @brief The format argument holder for the wchar_t format context.
     *
     * \lang simp-chinese
     * @brief 面向 wchar_t 格式化上下文的格式化参数容器。
     */
    using wformat_args = basic_format_args<wformat_context>;

    /**
     * \lang english
     * @brief Creates a format argument store for the given context and arguments.
     *
     * @tparam Context The format context type
     * @tparam Args The argument types
     * @param args The arguments to store
     * @return A format argument store
     *
     * \lang simp-chinese
     * @brief 为给定的上下文与参数创建格式化参数存储。
     *
     * @tparam Context 格式化上下文类型
     * @tparam Args 参数类型
     * @param args 待存储的参数
     * @return 格式化参数存储
     */
    template <typename Context, typename... Args>
    constexpr auto make_format_args(const Args &...args) noexcept {
        return __format_arg_store<Context, Args...>(type_traits::helper::index_sequence_for<Args...>{}, args...);
    }

    /**
     * \lang english
     * @brief Creates a format argument store using the default char format context.
     *
     * @tparam Args The argument types
     * @param args The arguments to store
     * @return A format argument store
     *
     * \lang simp-chinese
     * @brief 使用默认的 char 格式化上下文创建格式化参数存储。
     *
     * @tparam Args 参数类型
     * @param args 待存储的参数
     * @return 格式化参数存储
     */
    template <typename... Args>
    constexpr auto make_format_args(const Args &...args) noexcept {
        return make_format_args<format_context>(args...);
    }

    /**
     * \lang english
     * @brief Creates a format argument store using the wchar_t format context.
     *
     * @tparam Args The argument types
     * @param args The arguments to store
     * @return A format argument store
     *
     * \lang simp-chinese
     * @brief 使用 wchar_t 格式化上下文创建格式化参数存储。
     *
     * @tparam Args 参数类型
     * @param args 待存储的参数
     * @return 格式化参数存储
     */
    template <typename... Args>
    constexpr auto make_wformat_args(const Args &...args) noexcept {
        return make_format_args<wformat_context>(args...);
    }
}

namespace rainy::text {
    using core::text::basic_format_parse_context;
    using core::text::format_parse_context;
    using core::text::wformat_parse_context;
    using core::text::basic_format_context;
    using core::text::format_context;
    using core::text::wformat_context;
    using core::text::format_args;
    using core::text::wformat_args;
    using core::text::make_format_args;
    using core::text::make_wformat_args;
}

#endif
