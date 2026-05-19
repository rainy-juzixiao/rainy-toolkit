#ifndef RAINY_IMPLEMENTS_HYBRID_JENOVA_IMPL_HPP
#define RAINY_IMPLEMENTS_HYBRID_JENOVA_IMPL_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <string>
#include <string_view>

namespace rainy::core::implements {
    template <typename Ty = void>
    struct constexpr_hash {
        static_assert(type_traits::primary_types::is_integral<Ty>::value || type_traits::primary_types::is_enum<Ty>::value,
                      "only supports integral types, specialize for other types");

        constexpr std::size_t operator()(const Ty &value) const {
            std::size_t key = static_cast<std::size_t>(value);
            key = (~key) + (key << 21); // key = (key << 21) - key - 1;
            key = key ^ (key >> 24);
            key = (key + (key << 3)) + (key << 8); // key * 265
            key = key ^ (key >> 14);
            key = (key + (key << 2)) + (key << 4); // key * 21
            key = key ^ (key >> 28);
            key = key + (key << 31);
            return key;
        }
    };

    template <typename Elem, typename Traits, typename Alloc>
    struct constexpr_hash<std::basic_string<Elem, Traits, Alloc>> {
        constexpr std::size_t operator()(const std::basic_string<Elem, Traits, Alloc> &value, std::size_t seed = 0) const {
            std::size_t hash_value = seed;
            for (auto c: value) {
                hash_value = (hash_value ^ static_cast<std::size_t>(c)) * 1099511628211U;
            }
            return hash_value;
        }
    };
}

namespace rainy::utility {
    using core::implements::constexpr_hash;
}

#endif