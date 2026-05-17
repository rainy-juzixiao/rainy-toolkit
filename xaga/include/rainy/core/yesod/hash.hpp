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
#ifndef RAINY_CORE_IMPLEMENTS_HASH_HPP
#define RAINY_CORE_IMPLEMENTS_HASH_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>

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
        static_assert(type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
        return fnv1a_append_bytes(offset_basis, &reinterpret_cast<const unsigned char &>(keyval), sizeof(Key));
    }

    template <typename Ty>
    RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_range(const std::size_t offset_basis, const Ty *const first, // NOLINT
                                                           const Ty *const last) {
        static_assert(type_traits::type_properties::is_trivial_v<Ty>, "Only trivial types can be directly hashed.");
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
        static_assert(type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
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

#endif