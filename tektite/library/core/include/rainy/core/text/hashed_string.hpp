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
#ifndef RAINY_CORE_TEXT_HASHED_STRING_HPP
#define RAINY_CORE_TEXT_HASHED_STRING_HPP

#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/utility/hash.hpp>
#include <rainy/core/text/char_traits.hpp>

namespace rainy::core::text::implements {
    inline constexpr std::size_t fnv_offset_basis = 14695981039346656037ULL;
    inline constexpr std::size_t fnv_prime = 1099511628211ULL;
}

namespace rainy::core::text {
    /**
     * \lang english
     * @brief A compile-time FNV-1a hashed view over a character sequence.
     *
     * The string is not owned; only the hash, pointer and size are stored,
     * so instances are cheap to copy and usable as associative-container keys.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 字符序列的编译期FNV-1a哈希视图。
     *
     * 该类型不持有字符串，仅保存哈希值、指针和长度，拷贝廉价，可作关联容器键。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符traits类型
     */
    template <typename CharType, typename Traits = char_traits<CharType>>
    class basic_hashed_string {
    public:
        using value_type = CharType;
        using const_pointer = const value_type *;
        using traits_type = Traits;

        /**
         * \lang english
         * @brief Constructs an empty hashed string with a null pointer and zero hash.
         *
         * \lang simp-chinese
         * @brief 构造一个空哈希字符串，指针为空、哈希值为零。
         */
        constexpr basic_hashed_string() = default;

        /**
         * \lang english
         * @brief Constructs a hashed string from a null-terminated character pointer.
         *
         * @param ptr The null-terminated character pointer
         *
         * \lang simp-chinese
         * @brief 由以空字符结尾的字符指针构造哈希字符串。
         *
         * @param ptr 以空字符结尾的字符指针
         */
        constexpr basic_hashed_string(const_pointer ptr) noexcept :
            hash_val{fnv1a_hash(ptr)}, str{ptr}, size_{static_cast<std::size_t>(traits_type::length(ptr))} {
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param right The string to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param right 被拷贝的字符串
         */
        constexpr basic_hashed_string(const basic_hashed_string &right) noexcept :
            hash_val{right.hash_val}, str{right.str}, size_{right.size_} {
        }

        /**
         * \lang english
         * @brief Move constructor; leaves the source empty.
         *
         * @param right The string to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数；移动后源对象为空。
         *
         * @param right 被移动的字符串
         */
        constexpr basic_hashed_string(basic_hashed_string &&right) noexcept :
            hash_val{utility::exchange(right.hash_val, 0)}, str{utility::exchange(right.str, nullptr)},
            size_{utility::exchange(right.size_, 0)} {
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<type_traits::extras::meta_method::has_begin_v<StringViewLike> &&
                                                            type_traits::extras::meta_method::has_end_v<StringViewLike> &&
                                                            type_traits::extras::meta_method::has_size_v<StringViewLike> &&
                                                            type_traits::extras::meta_method::has_data_v<StringViewLike>,
                                                        int> = 0>
        /**
         * \lang english
         * @brief Constructs a hashed string from any string-view-like object that provides data() and size().
         *
         * @tparam StringViewLike The string-view-like type
         * @param svlike The source object
         *
         * \lang simp-chinese
         * @brief 由任意提供 data() 与 size() 的类字符串视图对象构造哈希字符串。
         *
         * @tparam StringViewLike 类字符串视图类型
         * @param svlike 源对象
         */
        constexpr basic_hashed_string(const StringViewLike &svlike) :
            hash_val{fnv1a_hash(svlike)}, str{svlike.data()}, size_{svlike.size()} {
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         */
        constexpr basic_hashed_string &operator=(const basic_hashed_string &) noexcept = default;

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         */
        constexpr basic_hashed_string &operator=(basic_hashed_string &&) noexcept = default;

        /**
         * \lang english
         * @brief Returns the maximum number of characters the string can theoretically hold.
         *
         * @return The maximum size
         *
         * \lang simp-chinese
         * @brief 返回理论上可容纳的最大字符数。
         *
         * @return 最大大小
         */
        RAINY_NODISCARD constexpr std::size_t max_size() const noexcept {
            constexpr std::size_t max_by_type = static_cast<std::size_t>(-1) / sizeof(value_type);
            constexpr std::size_t max_by_diff = static_cast<std::size_t>(utility::numeric_limits<std::ptrdiff_t>::max());
            return max_by_type < max_by_diff ? max_by_type : max_by_diff;
        }

        /**
         * \lang english
         * @brief Implicit conversion to the underlying character pointer.
         *
         * @return The underlying character pointer
         *
         * \lang simp-chinese
         * @brief 隐式转换为底层字符指针。
         *
         * @return 底层字符指针
         */
        constexpr operator const_pointer() const noexcept {
            return str;
        }

        /**
         * \lang english
         * @brief Returns the precomputed FNV-1a hash value.
         *
         * @return The hash value
         *
         * \lang simp-chinese
         * @brief 返回预计算的 FNV-1a 哈希值。
         *
         * @return 哈希值
         */
        RAINY_NODISCARD constexpr std::size_t hash_code() const noexcept {
            return hash_val;
        }

        /**
         * \lang english
         * @brief Returns the number of characters in the string.
         *
         * @return The character count
         *
         * \lang simp-chinese
         * @brief 返回字符串中的字符数量。
         *
         * @return 字符数量
         */
        RAINY_NODISCARD constexpr std::size_t size() const noexcept {
            return size_;
        }

        /**
         * \lang english
         * @brief Returns a pointer to the underlying character data.
         *
         * @return The underlying character pointer
         *
         * \lang simp-chinese
         * @brief 返回指向底层字符数据的指针。
         *
         * @return 底层字符指针
         */
        RAINY_NODISCARD constexpr const_pointer data() const noexcept {
            return str;
        }

        /**
         * \lang english
         * @brief Compares two hashed strings by their hash values.
         *
         * @param left The left operand
         * @param right The right operand
         * @return true if the hash values are equal
         *
         * \lang simp-chinese
         * @brief 按哈希值比较两个哈希字符串是否相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 哈希值相等时返回 true
         */
        friend constexpr bool operator==(const basic_hashed_string &left, const basic_hashed_string &right) noexcept {
            return left.hash_val == right.hash_val;
        }

        /**
         * \lang english
         * @brief Compares two hashed strings by their hash values for inequality.
         *
         * @param left The left operand
         * @param right The right operand
         * @return true if the hash values differ
         *
         * \lang simp-chinese
         * @brief 按哈希值比较两个哈希字符串是否不相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 哈希值不同时返回 true
         */
        friend constexpr bool operator!=(const basic_hashed_string &left, const basic_hashed_string &right) noexcept {
            return left.hash_val != right.hash_val;
        }

        /**
         * \lang english
         * @brief Creates a hashed string from a null-terminated character pointer.
         *
         * @param ptr The null-terminated character pointer
         * @return The constructed hashed string
         *
         * \lang simp-chinese
         * @brief 由以空字符结尾的字符指针创建哈希字符串。
         *
         * @param ptr 以空字符结尾的字符指针
         * @return 构造得到的哈希字符串
         */
        static constexpr basic_hashed_string make_hashed_string(const_pointer ptr) noexcept {
            return basic_hashed_string{ptr};
        }

        /**
         * \lang english
         * @brief Returns the hash value of the given hashed string.
         *
         * @param right The hashed string
         * @return The hash value
         *
         * \lang simp-chinese
         * @brief 返回给定哈希字符串的哈希值。
         *
         * @param right 哈希字符串
         * @return 哈希值
         */
        static constexpr std::size_t eval_hash(const basic_hashed_string &right) noexcept {
            return right.hash_val;
        }

    private:
        template <typename Input>
        static constexpr std::size_t fnv1a_hash(const Input &val,
                                                std::size_t offset_basis = implements::fnv_offset_basis) noexcept {
            std::size_t hash = offset_basis;
            if constexpr (type_traits::extras::meta_method::has_begin_v<Input> &&
                          type_traits::extras::meta_method::has_end_v<Input>) {
                for (value_type ch: val) {
                    hash ^= static_cast<std::size_t>(static_cast<type_traits::helper::make_unsigned_t<value_type>>(ch));
                    hash *= implements::fnv_prime;
                }
            } else {
                const_pointer cur{val};
                while (*cur != '\0') {
                    hash ^= static_cast<std::size_t>(static_cast<type_traits::helper::make_unsigned_t<value_type>>(*cur));
                    hash *= implements::fnv_prime;
                    ++cur;
                }
            }
            return hash;
        }

        std::size_t hash_val{0};
        const_pointer str{nullptr};
        std::size_t size_{0};
    };

    /**
     * \lang english
     * @brief basic_hashed_string specialization for char.
     *
     * \lang simp-chinese
     * @brief 面向 char 的 basic_hashed_string 特化。
     */
    using hashed_string = basic_hashed_string<char>;
    /**
     * \lang english
     * @brief basic_hashed_string specialization for wchar_t.
     *
     * \lang simp-chinese
     * @brief 面向 wchar_t 的 basic_hashed_string 特化。
     */
    using whashed_string = basic_hashed_string<wchar_t>;
    /**
     * \lang english
     * @brief basic_hashed_string specialization for char16_t.
     *
     * \lang simp-chinese
     * @brief 面向 char16_t 的 basic_hashed_string 特化。
     */
    using u16hashed_string = basic_hashed_string<char16_t>;
    /**
     * \lang english
     * @brief basic_hashed_string specialization for char32_t.
     *
     * \lang simp-chinese
     * @brief 面向 char32_t 的 basic_hashed_string 特化。
     */
    using u32hashed_string = basic_hashed_string<char32_t>;
#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief basic_hashed_string specialization for char8_t.
     *
     * \lang simp-chinese
     * @brief 面向 char8_t 的 basic_hashed_string 特化。
     */
    using u8hashed_string = basic_hashed_string<char8_t>;
#endif
}

#define rainy_switch_hashed_string(x) switch (::rainy::core::text::hashed_string::eval_hash(x))
#define rainy_switch_whashed_string(x) switch (::rainy::core::text::whashed_string::eval_hash(x))
#define rainy_switch_u16hashed_string(x) switch (::rainy::core::text::u16hashed_string::eval_hash(x))
#define rainy_switch_u32hashed_string(x) switch (::rainy::core::text::u32hashed_string::eval_hash(x))
#if RAINY_HAS_CXX20
#define rainy_switch_u8hashed_string(x) switch (::rainy::core::text::u8hashed_string::eval_hash(x))
#endif

#define rainy_match_hashed_string(x) ::rainy::core::text::hashed_string::eval_hash(x)
#define rainy_match_whashed_string(x) ::rainy::core::text::whashed_string::eval_hash(x)
#define rainy_match_u16hashed_string(x) ::rainy::core::text::u16hashed_string::eval_hash(x)
#define rainy_match_u32hashed_string(x) ::rainy::core::text::u32hashed_string::eval_hash(x)
#if RAINY_HAS_CXX20
#define rainy_match_u8hashed_string(x) ::rainy::core::text::u8hashed_string::eval_hash(x)
#endif

namespace rainy::utility {
    template <typename CharType, typename Traits>
    struct hash<core::text::basic_hashed_string<CharType, Traits>> {
        using argument_type = core::text::basic_hashed_string<CharType, Traits>;
        using result_type = std::size_t;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return val.hash_code();
        }

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return hash_this_val(val);
        }
    };
}

namespace rainy::text {
    using core::text::basic_hashed_string;
    using core::text::hashed_string;
    using core::text::whashed_string;
    using core::text::u16hashed_string;
    using core::text::u32hashed_string;
#if RAINY_HAS_CXX20
    using core::text::u8hashed_string;
#endif
}

#endif
