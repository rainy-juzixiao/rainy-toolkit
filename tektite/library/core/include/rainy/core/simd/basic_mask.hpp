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
#ifndef RAINY_CORE_SIMD_BASIC_SIMD_MASK_HPP
#define RAINY_CORE_SIMD_BASIC_SIMD_MASK_HPP

#include <rainy/core/simd/basic_vec.hpp>
#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/simd/native_abi.hpp>
#include <rainy/core/simd/register.hpp>

namespace rainy::core {
    template <std::size_t Bytes, typename Abi>
    class basic_mask {
    public:
        using value_type = bool;
        using abi_type = Abi;
        using register_type = implements::register_t<implements::integer_from_t<Bytes>, Abi>;

        static constexpr type_traits::helper::integral_constant<simd_size_type,
                                                                simd_size_v<implements::integer_from_t<Bytes>, Abi>>
            size{};

        RAINY_CONSTEXPR20 basic_mask() noexcept = default;

        RAINY_CONSTEXPR20 explicit basic_mask(value_type val) noexcept
            : data_(implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::broadcast(
                  static_cast<implements::integer_from_t<Bytes>>(val ? -1 : 0))) {}

        template <std::size_t UBytes, typename UAbi,
                  type_traits::other_trans::enable_if_t<
                      simd_size_v<implements::integer_from_t<UBytes>, UAbi> ==
                          simd_size_v<implements::integer_from_t<Bytes>, Abi>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit basic_mask(const basic_mask<UBytes, UAbi> &other) noexcept {
            using integer = implements::integer_from_t<Bytes>;
            data_ = implements::simd_ops<integer, Abi>::template convert<implements::integer_from_t<UBytes>, UAbi>(
                static_cast<typename basic_mask<UBytes, UAbi>::register_type>(other));
        }

        template <typename G,
                  type_traits::other_trans::enable_if_t<implements::is_basic_vec_generator<G>::value, int> = 0>
        RAINY_CONSTEXPR20 explicit basic_mask(G &&gen) noexcept {
            gen_fill(gen, type_traits::helper::make_index_sequence<
                              static_cast<std::size_t>(simd_size_v<implements::integer_from_t<Bytes>, Abi>)>{});
        }

        template <typename R,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<type_traits::implements::remove_cvref_t<R>, register_type>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit basic_mask(R &&reg) noexcept : data_(reg) {}

        RAINY_CONSTEXPR20 explicit operator register_type() const noexcept { return data_; }

        RAINY_CONSTEXPR20 value_type operator[](simd_size_type i) const {
            return implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::get_lane(data_, i) != 0;
        }

        RAINY_CONSTEXPR20 basic_mask operator!() const noexcept {
            return basic_mask{implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_not(data_)};
        }

        RAINY_CONSTEXPR20 basic_vec<implements::integer_from_t<Bytes>, Abi> operator+() const noexcept {
            return basic_vec<implements::integer_from_t<Bytes>, Abi>{
                implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::negate(data_)};
        }

        RAINY_CONSTEXPR20 basic_vec<implements::integer_from_t<Bytes>, Abi> operator-() const noexcept {
            return basic_vec<implements::integer_from_t<Bytes>, Abi>{data_};
        }

        RAINY_CONSTEXPR20 basic_vec<implements::integer_from_t<Bytes>, Abi> operator~() const noexcept {
            using integer = implements::integer_from_t<Bytes>;
            using ops = implements::simd_ops<integer, Abi>;
            return basic_vec<integer, Abi>{ops::bit_not(ops::bit_and(data_, ops::broadcast(static_cast<integer>(1))))};
        }

        template <typename U, typename A,
                  type_traits::other_trans::enable_if_t<
                      sizeof(U) == Bytes && simd_size_v<U, A> == simd_size_v<implements::integer_from_t<Bytes>, Abi>,
                      int> = 0>
        RAINY_CONSTEXPR20 operator basic_vec<U, A>() const noexcept {
            return make_simd<U, A>();
        }

        template <typename U, typename A,
                  type_traits::other_trans::enable_if_t<
                      sizeof(U) != Bytes && simd_size_v<U, A> == simd_size_v<implements::integer_from_t<Bytes>, Abi>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit operator basic_vec<U, A>() const noexcept {
            return make_simd<U, A>();
        }

        friend RAINY_CONSTEXPR20 basic_mask operator&&(const basic_mask &left,
                                                            const basic_mask &right) noexcept {
            return basic_mask{
                implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_and(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR20 basic_mask operator||(const basic_mask &left,
                                                            const basic_mask &right) noexcept {
            return basic_mask{
                implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_or(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR20 basic_mask operator&(const basic_mask &left,
                                                           const basic_mask &right) noexcept {
            return basic_mask{
                implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_and(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR20 basic_mask operator|(const basic_mask &left,
                                                           const basic_mask &right) noexcept {
            return basic_mask{
                implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_or(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR20 basic_mask operator^(const basic_mask &left,
                                                           const basic_mask &right) noexcept {
            return basic_mask{
                implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_xor(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR20 basic_mask &operator&=(basic_mask &left,
                                                             const basic_mask &right) noexcept {
            left.data_ = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_and(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR20 basic_mask &operator|=(basic_mask &left,
                                                             const basic_mask &right) noexcept {
            left.data_ = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_or(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR20 basic_mask &operator^=(basic_mask &left,
                                                             const basic_mask &right) noexcept {
            left.data_ = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>::bit_xor(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR20 basic_mask operator==(const basic_mask &left,
                                                            const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return make_compare<ops::cmp_eq>(left, right);
        }

        friend RAINY_CONSTEXPR20 basic_mask operator!=(const basic_mask &left,
                                                            const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return make_compare<ops::cmp_ne>(left, right);
        }

        friend RAINY_CONSTEXPR20 basic_mask operator>=(const basic_mask &left,
                                                            const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return make_compare<ops::cmp_ge>(left, right);
        }

        friend RAINY_CONSTEXPR20 basic_mask operator<=(const basic_mask &left,
                                                            const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return make_compare<ops::cmp_le>(left, right);
        }

        friend RAINY_CONSTEXPR20 basic_mask operator>(const basic_mask &left,
                                                           const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return make_compare<ops::cmp_gt>(left, right);
        }

        friend RAINY_CONSTEXPR20 basic_mask operator<(const basic_mask &left,
                                                           const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return make_compare<ops::cmp_lt>(left, right);
        }

        friend RAINY_CONSTEXPR20 basic_mask simd_select_impl(const basic_mask &mask,
                                                                  const basic_mask &left,
                                                                  const basic_mask &right) noexcept {
            using ops = implements::simd_ops<implements::integer_from_t<Bytes>, Abi>;
            return basic_mask{ops::blend(mask.data_, left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR20 basic_mask simd_select_impl(const basic_mask &mask, bool left,
                                                                  bool right) noexcept {
            using integer = implements::integer_from_t<Bytes>;
            using ops = implements::simd_ops<integer, Abi>;
            return basic_mask{ops::blend(mask.data_, ops::broadcast(static_cast<integer>(left ? -1 : 0)),
                                              ops::broadcast(static_cast<integer>(right ? -1 : 0)))};
        }

        template <typename T0,
                  type_traits::other_trans::enable_if_t<implements::is_vectorizable_v<T0> && sizeof(T0) == Bytes, int> = 0>
        friend RAINY_CONSTEXPR20 basic_vec<T0, Abi> simd_select_impl(const basic_mask &mask, const T0 &left,
                                                                      const T0 &right) noexcept {
            using target = basic_vec<T0, Abi>;
            using target_ops = implements::simd_ops<T0, Abi>;
            return target{target_ops::blend(mask.data_, target_ops::broadcast(left), target_ops::broadcast(right))};
        }

    private:
        template <auto Op>
        RAINY_CONSTEXPR20 static basic_mask make_compare(const basic_mask &left,
                                                              const basic_mask &right) noexcept {
            using integer = implements::integer_from_t<Bytes>;
            using ops = implements::simd_ops<integer, Abi>;
            auto one = ops::broadcast(static_cast<integer>(1));
            return basic_mask{Op(ops::bit_and(left.data_, one), ops::bit_and(right.data_, one))};
        }

        template <typename U, typename A>
        RAINY_CONSTEXPR20 basic_vec<U, A> make_simd() const noexcept {
            using integer = implements::integer_from_t<Bytes>;
            using int_ops = implements::simd_ops<integer, Abi>;
            auto bits = int_ops::bit_and(data_, int_ops::broadcast(static_cast<integer>(1)));
            return basic_vec<U, A>{implements::simd_ops<U, A>::template convert<integer, Abi>(bits)};
        }

        template <typename G, std::size_t... I>
        RAINY_CONSTEXPR20 void gen_fill(G &&gen, type_traits::helper::index_sequence<I...>) noexcept {
            using integer = implements::integer_from_t<Bytes>;
            if constexpr (simd_size_v<integer, Abi> > 0) {
                (implements::simd_ops<integer, Abi>::set_lane(
                     data_, static_cast<simd_size_type>(I),
                     static_cast<integer>(
                         gen(type_traits::helper::integral_constant<simd_size_type, static_cast<simd_size_type>(I)>{}) ? -1
                                                                                                                      : 0)),
                 ...);
            }
        }

        register_type data_;
    };
}

namespace rainy::core::implements {
    template <typename Result, bool Ok>
    struct mask_select_result {};

    template <typename Result>
    struct mask_select_result<Result, true> {
        using type = Result;
    };

    template <std::size_t Bytes, typename Abi>
    struct simd_select_result<basic_mask<Bytes, Abi>, basic_mask<Bytes, Abi>, basic_mask<Bytes, Abi>> {
        using type = basic_mask<Bytes, Abi>;
    };

    template <std::size_t Bytes, typename Abi>
    struct simd_select_result<basic_mask<Bytes, Abi>, bool, bool> {
        using type = basic_mask<Bytes, Abi>;
    };

    template <std::size_t Bytes, typename Abi, typename T0>
    struct simd_select_result<basic_mask<Bytes, Abi>, T0, T0>
        : mask_select_result<basic_vec<T0, Abi>, is_vectorizable_v<T0> && sizeof(T0) == Bytes> {};

    template <std::size_t Bytes, typename Abi, typename T, typename A>
    struct simd_select_result<basic_mask<Bytes, Abi>, basic_vec<T, A>, basic_vec<T, A>>
        : mask_select_result<basic_vec<T, A>,
                             sizeof(T) == Bytes && type_traits::type_relations::is_same_v<A, Abi>> {};
}

namespace rainy::core {
    template <std::size_t Bytes, typename Abi, typename Ty, typename U>
    constexpr auto select(const basic_mask<Bytes, Abi> &c, const Ty &a, const U &b) noexcept
        -> decltype(simd_select_impl(c, a, b)) {
        return simd_select_impl(c, a, b);
    }

    template <std::size_t Bytes, typename Abi>
    constexpr bool all_of(const basic_mask<Bytes, Abi> &k) noexcept {
        for (simd_size_type i = 0; i < basic_mask<Bytes, Abi>::size(); ++i) {
            if (!k[i]) {
                return false;
            }
        }
        return true;
    }

    template <std::size_t Bytes, typename Abi>
    constexpr bool any_of(const basic_mask<Bytes, Abi> &k) noexcept {
        for (simd_size_type i = 0; i < basic_mask<Bytes, Abi>::size(); ++i) {
            if (k[i]) {
                return true;
            }
        }
        return false;
    }

    template <std::size_t Bytes, typename Abi>
    constexpr bool none_of(const basic_mask<Bytes, Abi> &k) noexcept {
        return !any_of(k);
    }

    template <std::size_t Bytes, typename Abi>
    constexpr simd_size_type reduce_count(const basic_mask<Bytes, Abi> &k) noexcept {
        simd_size_type count = 0;
        for (simd_size_type i = 0; i < basic_mask<Bytes, Abi>::size(); ++i) {
            if (k[i]) {
                ++count;
            }
        }
        return count;
    }

    template <std::size_t Bytes, typename Abi>
    constexpr simd_size_type reduce_min_index(const basic_mask<Bytes, Abi> &k) {
        for (simd_size_type i = 0; i < basic_mask<Bytes, Abi>::size(); ++i) {
            if (k[i]) {
                return i;
            }
        }
        return 0;
    }

    template <std::size_t Bytes, typename Abi>
    constexpr simd_size_type reduce_max_index(const basic_mask<Bytes, Abi> &k) {
        simd_size_type result = 0;
        for (simd_size_type i = 0; i < basic_mask<Bytes, Abi>::size(); ++i) {
            if (k[i]) {
                result = i;
            }
        }
        return result;
    }

    constexpr bool all_of(bool val) noexcept {
        return val;
    }

    constexpr bool any_of(bool val) noexcept {
        return val;
    }

    constexpr bool none_of(bool val) noexcept {
        return !val;
    }

    constexpr simd_size_type reduce_count(bool val) noexcept {
        return static_cast<simd_size_type>(val);
    }

    constexpr simd_size_type reduce_min_index(bool val) {
        return static_cast<simd_size_type>(0);
    }

    constexpr simd_size_type reduce_max_index(bool val) {
        return static_cast<simd_size_type>(0);
    }

    template <typename T, typename Abi, typename UTy>
    struct alignment<basic_vec<T, Abi>, UTy>
        : type_traits::helper::integral_constant<std::size_t, sizeof(implements::register_t<T, Abi>)> {};

    template <std::size_t Bytes, typename Abi, typename UTy>
    struct alignment<basic_mask<Bytes, Abi>, UTy>
        : type_traits::helper::integral_constant<std::size_t,
                                                 sizeof(implements::register_t<implements::integer_from_t<Bytes>, Abi>)> {
    };

    template <typename Ty, typename V, typename = void>
    struct rebind_impl {};

    template <typename Ty, typename U, typename Abi>
    struct rebind_impl<
        Ty, basic_vec<U, Abi>,
        type_traits::implements::void_t<typename implements::deduce_abi_impl<Ty, simd_size_v<U, Abi>>::type>> {
        using type = basic_vec<Ty, typename implements::deduce_abi_impl<Ty, simd_size_v<U, Abi>>::type>;
    };

    template <typename Ty, std::size_t Bytes, typename Abi>
    struct rebind_impl<
        Ty, basic_mask<Bytes, Abi>,
        type_traits::implements::void_t<
            typename implements::deduce_abi_impl<Ty, simd_size_v<implements::integer_from_t<Bytes>, Abi>>::type>> {
        using type =
            basic_mask<sizeof(Ty), typename implements::deduce_abi_impl<Ty, simd_size_v<implements::integer_from_t<Bytes>, Abi>>::type>;
    };

    template <typename Ty, typename V>
    struct rebind : rebind_impl<Ty, V> {};
}

#endif
