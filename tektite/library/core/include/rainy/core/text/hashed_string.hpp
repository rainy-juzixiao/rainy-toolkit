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
     * @brief A compile-time FNV-1a hashed view over a character sequence.
     *        字符序列的编译期FNV-1a哈希视图。
     *
     * The string is not owned; only the hash, pointer and size are stored,
     * so instances are cheap to copy and usable as associative-container keys.
     * 该类型不持有字符串，仅保存哈希值、指针和长度，拷贝廉价，可作关联容器键。
     *
     * @tparam CharType The character type
     *                  字符类型
     * @tparam Traits The character traits type
     *                字符traits类型
     */
    template <typename CharType, typename Traits = char_traits<CharType>>
    class basic_hashed_string {
    public:
        using value_type = CharType;
        using const_pointer = const value_type *;
        using traits_type = Traits;

        constexpr basic_hashed_string() = default;

        constexpr basic_hashed_string(const_pointer ptr) noexcept :
            hash_val{fnv1a_hash(ptr)}, str{ptr}, size_{static_cast<std::size_t>(traits_type::length(ptr))} {
        }

        constexpr basic_hashed_string(const basic_hashed_string &right) noexcept :
            hash_val{right.hash_val}, str{right.str}, size_{right.size_} {
        }

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
        constexpr basic_hashed_string(const StringViewLike &svlike) :
            hash_val{fnv1a_hash(svlike)}, str{svlike.data()}, size_{svlike.size()} {
        }

        constexpr basic_hashed_string &operator=(const basic_hashed_string &) noexcept = default;

        constexpr basic_hashed_string &operator=(basic_hashed_string &&) noexcept = default;

        RAINY_NODISCARD constexpr std::size_t max_size() const noexcept {
            constexpr std::size_t max_by_type = static_cast<std::size_t>(-1) / sizeof(value_type);
            constexpr std::size_t max_by_diff = static_cast<std::size_t>(utility::numeric_limits<std::ptrdiff_t>::max());
            return max_by_type < max_by_diff ? max_by_type : max_by_diff;
        }

        constexpr operator const_pointer() const noexcept {
            return str;
        }

        RAINY_NODISCARD constexpr std::size_t hash_code() const noexcept {
            return hash_val;
        }

        RAINY_NODISCARD constexpr std::size_t size() const noexcept {
            return size_;
        }

        RAINY_NODISCARD constexpr const_pointer data() const noexcept {
            return str;
        }

        friend constexpr bool operator==(const basic_hashed_string &left, const basic_hashed_string &right) noexcept {
            return left.hash_val == right.hash_val;
        }

        friend constexpr bool operator!=(const basic_hashed_string &left, const basic_hashed_string &right) noexcept {
            return left.hash_val != right.hash_val;
        }

        static constexpr basic_hashed_string make_hashed_string(const_pointer ptr) noexcept {
            return basic_hashed_string{ptr};
        }

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

    using hashed_string = basic_hashed_string<char>;
    using whashed_string = basic_hashed_string<wchar_t>;
    using u16hashed_string = basic_hashed_string<char16_t>;
    using u32hashed_string = basic_hashed_string<char32_t>;
#if RAINY_HAS_CXX20
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

#endif
