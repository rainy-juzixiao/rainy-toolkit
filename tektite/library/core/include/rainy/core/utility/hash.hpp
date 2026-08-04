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
#ifndef RAINY_CORE_UTILITY_HASH_HPP
#define RAINY_CORE_UTILITY_HASH_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/primary_types.hpp>

namespace rainy::utility::implements {
    inline constexpr std::size_t fnv_offset_basis = static_cast<std::size_t>(14695981039346656037ULL);
    inline constexpr std::size_t fnv_prime = static_cast<std::size_t>(1099511628211ULL);

    RAINY_INLINE_NODISCARD std::size_t fnv1a_append_bytes(const std::size_t offset_basis, const unsigned char *const first,
                                                          const std::size_t count) noexcept {
        std::size_t hash = offset_basis;
        for (std::size_t i = 0; i < count; ++i) {
            hash ^= static_cast<std::size_t>(first[i]);
            hash *= fnv_prime;
        }
        return hash;
    }
}

namespace rainy::utility {
    /**
     * @brief A template for hash function object.
     *        Provides hash computation for various types.
     *
     *        哈希函数对象的模板。
     *        为各种类型提供哈希计算。
     *
     * @tparam key The type to compute hash for
     *             要计算哈希的类型
     */
    template <typename Key>
    struct hash;
}

namespace rainy::utility::implements {
    template <typename Key>
    RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_value(const std::size_t offset_basis, const Key &keyval) noexcept { // NOLINT
        static_assert(type_traits::properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
        return fnv1a_append_bytes(offset_basis, &reinterpret_cast<const unsigned char &>(keyval), sizeof(Key));
    }

    template <typename Ty>
    RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_range(const std::size_t offset_basis, const Ty *const first, // NOLINT
                                                           const Ty *const last) {
        static_assert(type_traits::properties::is_trivial_v<Ty>, "Only trivial types can be directly hashed.");
        const auto *const first_binary = reinterpret_cast<const unsigned char *>(first);
        const auto *const last_binary = reinterpret_cast<const unsigned char *>(last);
        return fnv1a_append_bytes(offset_basis, first_binary, static_cast<std::size_t>(last_binary - first_binary));
    }

    template <typename Key>
    RAINY_AINLINE_NODISCARD std::size_t hash_representation(const Key &keyval) noexcept {
        return fnv1a_append_value(fnv_offset_basis, keyval);
    }

    template <typename Key>
    RAINY_AINLINE_NODISCARD std::size_t hash_array_representation(const Key *const first, const std::size_t count) noexcept {
        static_assert(type_traits::properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
        return fnv1a_append_bytes(fnv_offset_basis, reinterpret_cast<const unsigned char *>(first), count * sizeof(Key));
    }

    /**
     * @tparam key
     * @tparam check 如果为真，此模板将启用
     */
    template <typename key, bool check>
    struct hash_enable_if {
        using argument_type = key;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(const argument_type &val) const
            noexcept(noexcept(hash<key>::hash_this_val(val))) {
            return hash<key>::hash_this_val(val);
        }
    };

    template <typename key>
    struct hash_enable_if<key, false> {
        hash_enable_if() = delete;
        hash_enable_if(const hash_enable_if &) = delete;
        hash_enable_if(hash_enable_if &&) = delete;
        hash_enable_if &operator=(const hash_enable_if &) = delete;
        hash_enable_if &operator=(hash_enable_if &&) = delete;
    };
}

namespace rainy::utility {
    /**
     * @brief Primary template for hash function object.
     *        Provides hash computation for various types.
     *
     *        哈希函数对象的主模板。
     *        为各种类型提供哈希计算。
     *
     * @tparam key The type to compute hash for
     *             要计算哈希的类型
     */
    template <typename key>
    struct hash : implements::hash_enable_if<
                      key, !type_traits::properties::is_const_v<key> && !type_traits::properties::is_volatile_v<key> &&
                               (type_traits::primary_types::is_enum_v<key> || type_traits::primary_types::is_integral_v<key> ||
                                type_traits::primary_types::is_pointer_v<key>)> {
        /**
         * @brief Computes hash value for the given key.
         *        计算给定键的哈希值。
         *
         * @param keyval The value to hash
         *               要哈希的值
         * @return Hash value
         *         哈希值
         */
        static std::size_t hash_this_val(const key &keyval) noexcept {
            return implements::hash_representation(keyval);
        }
    };
}

namespace rainy::utility {
    /**
     * @brief Specialization for float type.
     *        float 类型的特化。
     */
    template <>
    struct hash<float> {
        using argument_type = float;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for float, treating -0.0f as 0.0f.
         *        计算 float 的哈希值，将 -0.0f 视为 0.0f。
         *
         * @param val The float value to hash
         *            要哈希的 float 值
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return implements::hash_representation(val == 0.0f ? 0.0f : val);
        }
    };

    /**
     * @brief Specialization for double type.
     *        double 类型的特化。
     */
    template <>
    struct hash<double> {
        using argument_type = double;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for double, treating -0.0 as 0.0.
         *        计算 double 的哈希值，将 -0.0 视为 0.0。
         *
         * @param val The double value to hash
         *            要哈希的 double 值
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return implements::hash_representation(val == 0.0 ? 0.0 : val);
        }
    };

    /**
     * @brief Specialization for long double type.
     *        long double 类型的特化。
     */
    template <>
    struct hash<long double> {
        using argument_type = long double;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for long double, treating -0.0L as 0.0L.
         *        计算 long double 的哈希值，将 -0.0L 视为 0.0L。
         *
         * @param val The long double value to hash
         *            要哈希的 long double 值
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return implements::hash_representation(val == 0.0L ? 0.0L : val);
        }
    };

    /**
     * @brief Specialization for nullptr_t type.
     *        nullptr_t 类型的特化。
     */
    template <>
    struct hash<std::nullptr_t> {
        using argument_type = std::nullptr_t;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for nullptr.
         *        计算 nullptr 的哈希值。
         *
         * @param  nullptr_t
         * @return Hash value (hash of null pointer)
         *         哈希值（空指针的哈希）
         */
        static rain_fn hash_this_val(std::nullptr_t) noexcept -> std::size_t {
            void *null_pointer{};
            return implements::hash_representation(null_pointer);
        }

        /**
         * @brief Function call operator for nullptr.
         *        nullptr 的函数调用运算符。
         *
         * @param  nullptr_t
         * @return Hash value (hash of null pointer)
         *         哈希值（空指针的哈希）
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(std::nullptr_t) const->result_type {
            void *null_pointer{};
            return implements::hash_representation(null_pointer);
        }
    };

    /**
     * @brief Specialization for basic_string_view types.
     *        basic_string_view 类型的特化。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam Traits String traits type
     *                字符串特性类型
     */
    template <typename CharType, typename Traits>
    struct hash<std::basic_string_view<CharType, Traits>> {
        using argument_type = std::basic_string_view<CharType, Traits>;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for a string view.
         *        计算字符串视图的哈希值。
         *
         * @param val The string view to hash
         *            要哈希的字符串视图
         * @return Hash value based on the string's characters
         *         基于字符串字符的哈希值
         */
        static rain_fn hash_this_val(const argument_type &val) noexcept -> std::size_t {
            return implements::hash_array_representation(val.data(), val.size());
        }

        /**
         * @brief Function call operator for string view.
         *        字符串视图的函数调用运算符。
         *
         * @param val The string view to hash
         *            要哈希的字符串视图
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return hash_this_val(val);
        }
    };

    /**
     * @brief Specialization for basic_string types.
     *        basic_string 类型的特化。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam Traits String traits type
     *                字符串特性类型
     * @tparam Alloc Allocator type
     *               分配器类型
     */
    template <typename CharType, typename Traits, typename Alloc>
    struct hash<std::basic_string<CharType, Traits, Alloc>> {
        using argument_type = std::basic_string<CharType, Traits, Alloc>;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for a string.
         *        计算字符串的哈希值。
         *
         * @param val The string to hash
         *            要哈希的字符串
         * @return Hash value based on the string's characters
         *         基于字符串字符的哈希值
         */
        static rain_fn hash_this_val(const argument_type &val) noexcept -> std::size_t {
            return implements::hash_array_representation(val.data(), val.size());
        }

        /**
         * @brief Function call operator for string.
         *        字符串的函数调用运算符。
         *
         * @param val The string to hash
         *            要哈希的字符串
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(const argument_type &val) const->result_type {
            return hash_this_val(val);
        }
    };

    /**
     * @brief Type trait to check if standard hasher is available for a type.
     *        检查类型的标准哈希器是否可用的类型特性。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct is_support_standard_hasher_available : type_traits::helper::false_type {};

    /**
     * @brief Specialization that detects if std::hash<Ty> is callable.
     *        检测 std::hash<Ty> 是否可调用的特化。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_support_standard_hasher_available<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::hash<Ty>>()(utility::declval<Ty>()))>>
        : type_traits::helper::true_type {};

    /**
     * @brief Type trait to check if Rainy Toolkit hasher is available for a type.
     *        检查类型的 Rainy Toolkit 哈希器是否可用的类型特性。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct is_support_rainytoolkit_hasher_available : type_traits::helper::false_type {};

    /**
     * @brief Specialization that detects if rainy::utility::hash<Ty> is callable.
     *        检测 rainy::utility::hash<Ty> 是否可调用的特化。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_support_rainytoolkit_hasher_available<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::hash<Ty>>()(utility::declval<Ty>()))>>
        : type_traits::helper::true_type {};
}

#endif