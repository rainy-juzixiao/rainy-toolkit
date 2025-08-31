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
#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/implements.hpp>
#include <rainy/core/tmp/sfinae_base.hpp>

namespace rainy::core::builtin {
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr int popcount(const Type value) noexcept {
        return value ? (int(value & 1) + popcount(static_cast<Type>(value >> 1))) : 0;
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr bool has_single_bit(const Type value) noexcept {
        return value && ((value & (value - 1)) == 0);
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr Type next_power_of_two(const Type value) noexcept {
        assert(value < (Type{1u} << (utility::numeric_limits<Type>::digits - 1)) && "Numeric limits has exceeded"); // NOLINT
        Type curr = value - (value != 0u);
        for (int next = 1; next < utility::numeric_limits<Type>::digits; next = next * 2) {
            curr |= (curr >> next);
        }
        return ++curr;
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr int bit_width(const Type value) noexcept {
        int width = 0;
        Type v = value;
        while (v != 0) {
            ++width;
            v >>= 1;
        }
        return width;
    }
    
    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr Type bit_floor(const Type value) noexcept {
        if (value == 0)
            return 0;
        return Type{1} << (bit_width(value) - 1);
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr int countl_zero(Type value) noexcept {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        int count = 0;
        for (int i = bits - 1; i >= 0; --i) {
            if ((value >> i) & 1u) {
                break;
            }
            ++count;
        }
        return count;
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr int countr_zero(Type value) noexcept {
        if (value == 0) {
            return utility::numeric_limits<Type>::digits;
        }
        int count = 0;
        while ((value & 1u) == 0) {
            value >>= 1;
            ++count;
        }
        return count;
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr Type mod(const Type value, const std::size_t mod) noexcept {
        assert(has_single_bit(mod) && "Value must be a power of two");
        return static_cast<Type>(value & (mod - 1u));
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr Type rotate_left(Type value, int shift) noexcept {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        shift %= bits;
        return (value << shift) | (value >> (bits - shift));
    }

    template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_unsigned_v<Type>, int> = 0>
    RAINY_NODISCARD constexpr Type rotate_right(Type value, int shift) noexcept {
        constexpr int bits = utility::numeric_limits<Type>::digits;
        shift %= bits;
        return (value >> shift) | (value << (bits - shift));
    }
}
