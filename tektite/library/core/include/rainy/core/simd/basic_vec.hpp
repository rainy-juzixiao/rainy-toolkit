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
#ifndef RAINY_CORE_SIMD_BASIC_SIMD_HPP
#define RAINY_CORE_SIMD_BASIC_SIMD_HPP
#include <iterator>
#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/simd/native_abi.hpp>
#include <rainy/core/simd/register.hpp>
#include <rainy/core/type_traits/helper.hpp>

namespace rainy::core::implements {
    template <typename V>
    struct is_basic_vec : type_traits::helper::false_type {};

    template <typename U, typename UAbi>
    struct is_basic_vec<basic_vec<U, UAbi>> : type_traits::helper::true_type {};

    template <typename V>
    RAINY_CONSTEXPR_BOOL is_basic_vec_v = is_basic_vec<V>::value;

    template <typename From, typename To>
    constexpr bool is_value_preserving() noexcept {
        using from_t = type_traits::implements::remove_cvref_t<From>;
        using to_t = type_traits::implements::remove_cvref_t<To>;
        if constexpr (type_traits::type_relations::is_same_v<from_t, to_t>) {
            return true;
        } else if constexpr (type_traits::type_relations::is_same_v<from_t, bool>) {
            return !type_traits::type_relations::is_same_v<to_t, bool> &&
                   (type_traits::primary_types::is_integral_v<to_t> || type_traits::primary_types::is_floating_point_v<to_t>);
        } else if constexpr (type_traits::type_relations::is_same_v<to_t, bool>) {
            return false;
        } else if constexpr (!(type_traits::primary_types::is_integral_v<from_t> ||
                               type_traits::primary_types::is_floating_point_v<from_t>) ||
                             !(type_traits::primary_types::is_integral_v<to_t> ||
                               type_traits::primary_types::is_floating_point_v<to_t>) ) {
            return false;
        } else if constexpr (type_traits::primary_types::is_floating_point_v<from_t> &&
                             type_traits::primary_types::is_floating_point_v<to_t>) {
            return sizeof(from_t) <= sizeof(to_t);
        } else if constexpr (type_traits::primary_types::is_floating_point_v<from_t>) {
            return false;
        } else if constexpr (type_traits::primary_types::is_floating_point_v<to_t>) {
            return utility::numeric_limits<to_t>::digits >=
                   utility::numeric_limits<from_t>::digits + (type_traits::properties::is_signed_v<from_t> ? 1 : 0);
        } else if constexpr (type_traits::properties::is_signed_v<from_t> && !type_traits::properties::is_signed_v<to_t>) {
            return false;
        } else if constexpr (!type_traits::properties::is_signed_v<from_t> && type_traits::properties::is_signed_v<to_t>) {
            return utility::numeric_limits<from_t>::digits <= utility::numeric_limits<to_t>::digits;
        } else {
            return sizeof(from_t) <= sizeof(to_t);
        }
    }

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_value_preserving_v = is_value_preserving<From, To>();

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_simd_converting_explicit_v =
        !is_value_preserving_v<From, To> ||
        (type_traits::primary_types::is_integral_v<From> && type_traits::primary_types::is_integral_v<To> &&
         (sizeof(From) > sizeof(To) ||
          (sizeof(From) == sizeof(To) &&
           (type_traits::type_relations::is_same_v<type_traits::implements::remove_cv_t<From>, long long> ||
            type_traits::type_relations::is_same_v<type_traits::implements::remove_cv_t<From>, unsigned long long>) &&
           !type_traits::type_relations::is_same_v<type_traits::implements::remove_cv_t<To>, long long> &&
           !type_traits::type_relations::is_same_v<type_traits::implements::remove_cv_t<To>, unsigned long long>) )) ||
        (type_traits::primary_types::is_floating_point_v<From> && type_traits::primary_types::is_floating_point_v<To> &&
         sizeof(From) > sizeof(To));

    template <typename From, typename To>
    RAINY_CONSTEXPR_BOOL is_basic_vec_scalar_arg_v =
        !is_basic_vec_v<type_traits::implements::remove_cvref_t<From>> &&
        type_traits::type_relations::is_convertible_v<type_traits::implements::remove_cvref_t<From>, To> &&
        (!(type_traits::primary_types::is_integral_v<type_traits::implements::remove_cvref_t<From>> ||
           type_traits::primary_types::is_floating_point_v<type_traits::implements::remove_cvref_t<From>>) ||
         is_value_preserving_v<type_traits::implements::remove_cvref_t<From>, To>);

    template <typename G, typename To>
    RAINY_CONSTEXPR_BOOL is_basic_vec_gen_arg_v =
        !is_basic_vec_v<type_traits::implements::remove_cvref_t<G>> &&
        !type_traits::type_relations::is_convertible_v<type_traits::implements::remove_cvref_t<G>, To> &&
        is_basic_vec_generator<type_traits::implements::remove_cvref_t<G>>::value;
}

namespace rainy::core {
    template <typename V>
    class simd_iterator {
        V *data_ = nullptr;
        simd_size_type offset_ = 0;

        RAINY_CONSTEXPR26 simd_iterator(V &val, simd_size_type off) noexcept : data_(&val), offset_(off) {
        }

        template <typename>
        friend class simd_iterator;
        template <typename, typename>
        friend class basic_vec;

    public:
        using value_type = typename V::value_type;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = simd_size_type;

        RAINY_CONSTEXPR26 simd_iterator() noexcept = default;
        RAINY_CONSTEXPR26 simd_iterator(const simd_iterator &) = default;
        RAINY_CONSTEXPR26 simd_iterator &operator=(const simd_iterator &) = default;

        template <typename U = V, type_traits::other_trans::enable_if_t<type_traits::properties::is_const_v<U>, int> = 0>
        RAINY_CONSTEXPR26 simd_iterator(const simd_iterator<type_traits::modifers::remove_const_t<U>> &other) noexcept :
            data_(other.data_), offset_(other.offset_) {
        }

        RAINY_CONSTEXPR26 value_type operator*() const noexcept {
            return (*data_)[offset_];
        }

        RAINY_CONSTEXPR26 value_type operator[](difference_type n) const noexcept {
            return (*data_)[offset_ + n];
        }

        RAINY_CONSTEXPR26 simd_iterator &operator++() noexcept {
            ++offset_;
            return *this;
        }

        RAINY_CONSTEXPR26 simd_iterator operator++(int) noexcept {
            simd_iterator old = *this;
            ++*this;
            return old;
        }

        RAINY_CONSTEXPR26 simd_iterator &operator--() noexcept {
            --offset_;
            return *this;
        }

        RAINY_CONSTEXPR26 simd_iterator operator--(int) noexcept {
            simd_iterator old = *this;
            --*this;
            return old;
        }

        RAINY_CONSTEXPR26 simd_iterator &operator+=(difference_type n) noexcept {
            offset_ += n;
            return *this;
        }

        RAINY_CONSTEXPR26 simd_iterator &operator-=(difference_type n) noexcept {
            offset_ -= n;
            return *this;
        }

        friend RAINY_CONSTEXPR26 bool operator==(const simd_iterator &left, const simd_iterator &right) noexcept {
            return left.data_ == right.data_ && left.offset_ == right.offset_;
        }

        friend RAINY_CONSTEXPR26 bool operator!=(const simd_iterator &left, const simd_iterator &right) noexcept {
            return !(left == right);
        }

        friend RAINY_CONSTEXPR26 bool operator<(const simd_iterator &left, const simd_iterator &right) noexcept {
            return left.data_ != right.data_ ? left.data_ < right.data_ : left.offset_ < right.offset_;
        }

        friend RAINY_CONSTEXPR26 bool operator<=(const simd_iterator &left, const simd_iterator &right) noexcept {
            return !(right < left);
        }

        friend RAINY_CONSTEXPR26 bool operator>(const simd_iterator &left, const simd_iterator &right) noexcept {
            return right < left;
        }

        friend RAINY_CONSTEXPR26 bool operator>=(const simd_iterator &left, const simd_iterator &right) noexcept {
            return !(left < right);
        }

        friend RAINY_CONSTEXPR26 simd_iterator operator+(simd_iterator i, difference_type n) noexcept {
            i += n;
            return i;
        }

        friend RAINY_CONSTEXPR26 simd_iterator operator+(difference_type n, simd_iterator i) noexcept {
            i += n;
            return i;
        }

        friend RAINY_CONSTEXPR26 simd_iterator operator-(simd_iterator i, difference_type n) noexcept {
            i -= n;
            return i;
        }

        friend RAINY_CONSTEXPR26 difference_type operator-(const simd_iterator &left, const simd_iterator &right) noexcept {
            return left.offset_ - right.offset_;
        }
    };

    template <typename Ty, typename Abi>
    class basic_vec {
    public:
        using value_type = Ty;
        using mask_type = basic_mask<sizeof(Ty), Abi>;
        using abi_type = Abi;
        using register_type = implements::register_t<Ty, Abi>;
        using iterator = simd_iterator<basic_vec>;
        using const_iterator = simd_iterator<const basic_vec>;

        static constexpr type_traits::helper::integral_constant<simd_size_type, simd_size_v<Ty, Abi>> size{};

        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return iterator(*this, 0);
        }

        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return const_iterator(*this, 0);
        }

        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return const_iterator(*this, 0);
        }

        RAINY_CONSTEXPR20 iterator end() noexcept {
            return iterator(*this, simd_size_v<Ty, Abi>);
        }

        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return const_iterator(*this, simd_size_v<Ty, Abi>);
        }

        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return const_iterator(*this, simd_size_v<Ty, Abi>);
        }

        RAINY_CONSTEXPR26 basic_vec() noexcept = default;

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::is_basic_vec_scalar_arg_v<UTy, value_type>, int> = 0>
        RAINY_CONSTEXPR26 basic_vec(UTy &&value) noexcept :
            data_(implements::simd_ops<Ty, Abi>::broadcast(static_cast<value_type>(value))) {
        }

        template <typename UTy, typename UAbi,
                  type_traits::other_trans::enable_if_t<
                      simd_size_v<UTy, UAbi> == simd_size_v<Ty, Abi> && !implements::is_simd_converting_explicit_v<UTy, Ty>, int> = 0>
        RAINY_CONSTEXPR26 basic_vec(const basic_vec<UTy, UAbi> &other) noexcept {
            data_ = implements::simd_ops<Ty, Abi>::template convert<UTy, UAbi>(
                static_cast<typename basic_vec<UTy, UAbi>::register_type>(other));
        }

        template <typename UTy, typename UAbi,
                  type_traits::other_trans::enable_if_t<
                      simd_size_v<UTy, UAbi> == simd_size_v<Ty, Abi> && implements::is_simd_converting_explicit_v<UTy, Ty>, int> = 0>
        RAINY_CONSTEXPR26 explicit basic_vec(const basic_vec<UTy, UAbi> &other) noexcept {
            data_ = implements::simd_ops<Ty, Abi>::template convert<UTy, UAbi>(
                static_cast<typename basic_vec<UTy, UAbi>::register_type>(other));
        }

        template <typename G, type_traits::other_trans::enable_if_t<implements::is_basic_vec_gen_arg_v<G, value_type>, int> = 0>
        RAINY_CONSTEXPR26 explicit basic_vec(G &&gen) noexcept {
            gen_fill(gen, type_traits::helper::make_index_sequence<static_cast<std::size_t>(simd_size_v<Ty, Abi>)>{});
        }

        template <typename R,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<register_type, value_type> &&
                          type_traits::type_relations::is_same_v<type_traits::implements::remove_cvref_t<R>, register_type>,
                      int> = 0>
        RAINY_CONSTEXPR26 explicit basic_vec(R &&reg) noexcept : data_(reg) {
        }

        RAINY_CONSTEXPR26 explicit operator register_type() const noexcept {
            return data_;
        }

        RAINY_CONSTEXPR26 value_type operator[](simd_size_type i) const {
            return implements::simd_ops<Ty, Abi>::get_lane(data_, i);
        }

        RAINY_CONSTEXPR26 basic_vec &operator++() noexcept {
            data_ = implements::simd_ops<Ty, Abi>::add(data_, implements::simd_ops<Ty, Abi>::broadcast(static_cast<value_type>(1)));
            return *this;
        }

        RAINY_CONSTEXPR26 basic_vec operator++(int) noexcept {
            basic_vec old = *this;
            ++*this;
            return old;
        }

        RAINY_CONSTEXPR26 basic_vec &operator--() noexcept {
            data_ = implements::simd_ops<Ty, Abi>::sub(data_, implements::simd_ops<Ty, Abi>::broadcast(static_cast<value_type>(1)));
            return *this;
        }

        RAINY_CONSTEXPR26 basic_vec operator--(int) noexcept {
            basic_vec old = *this;
            --*this;
            return old;
        }

        RAINY_CONSTEXPR26 mask_type operator!() const noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_eq(data_, implements::simd_ops<Ty, Abi>::broadcast(value_type()))};
        }

        RAINY_CONSTEXPR26 basic_vec operator~() const noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::bit_not(data_)};
        }

        RAINY_CONSTEXPR26 basic_vec operator+() const noexcept {
            return *this;
        }

        RAINY_CONSTEXPR26 basic_vec operator-() const noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::negate(data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator+(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::add(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator-(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::sub(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator*(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::mul(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator/(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::div(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator%(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::mod(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator&(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::bit_and(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator|(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::bit_or(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator^(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::bit_xor(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator<<(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::shl(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator>>(const basic_vec &left, const basic_vec &right) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::shr(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator<<(const basic_vec &left, simd_size_type count) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::shl_scalar(left.data_, count)};
        }

        friend RAINY_CONSTEXPR26 basic_vec operator>>(const basic_vec &left, simd_size_type count) noexcept {
            return basic_vec{implements::simd_ops<Ty, Abi>::shr_scalar(left.data_, count)};
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator+=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::add(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator-=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::sub(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator*=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::mul(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator/=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::div(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator%=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::mod(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator&=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::bit_and(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator|=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::bit_or(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator^=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::bit_xor(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator<<=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::shl(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator>>=(basic_vec &left, const basic_vec &right) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::shr(left.data_, right.data_);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator<<=(basic_vec &left, simd_size_type count) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::shl_scalar(left.data_, count);
            return left;
        }

        friend RAINY_CONSTEXPR26 basic_vec &operator>>=(basic_vec &left, simd_size_type count) noexcept {
            left.data_ = implements::simd_ops<Ty, Abi>::shr_scalar(left.data_, count);
            return left;
        }

        friend RAINY_CONSTEXPR26 mask_type operator==(const basic_vec &left, const basic_vec &right) noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_eq(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 mask_type operator!=(const basic_vec &left, const basic_vec &right) noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_ne(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 mask_type operator>=(const basic_vec &left, const basic_vec &right) noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_ge(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 mask_type operator<=(const basic_vec &left, const basic_vec &right) noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_le(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 mask_type operator>(const basic_vec &left, const basic_vec &right) noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_gt(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 mask_type operator<(const basic_vec &left, const basic_vec &right) noexcept {
            return mask_type{implements::simd_ops<Ty, Abi>::cmp_lt(left.data_, right.data_)};
        }

        friend RAINY_CONSTEXPR26 basic_vec simd_select_impl(const mask_type &mask, const basic_vec &left,
                                                            const basic_vec &right) noexcept {
            return basic_vec{
                implements::simd_ops<Ty, Abi>::blend(static_cast<typename mask_type::register_type>(mask), left.data_, right.data_)};
        }

    private:
        template <typename G, std::size_t... I>
        RAINY_CONSTEXPR26 void gen_fill(G &&gen, type_traits::helper::index_sequence<I...>) noexcept {
            if constexpr (simd_size_v<Ty, Abi> > 0) {
                (implements::simd_ops<Ty, Abi>::set_lane(
                     data_, static_cast<simd_size_type>(I),
                     static_cast<value_type>(
                         gen(type_traits::helper::integral_constant<simd_size_type, static_cast<simd_size_type>(I)>{}))),
                 ...);
            }
        }

        register_type data_;
    };
}

namespace rainy::core::implements {
    template <typename Op, typename Ty, typename>
    struct is_reduction_op : type_traits::helper::false_type {};

    template <typename Op, typename Ty>
    struct is_reduction_op<Op, Ty,
                           type_traits::implements::void_t<decltype(utility::declval<Op>()(
                               utility::declval<basic_vec<Ty, scalar_abi>>(), utility::declval<basic_vec<Ty, scalar_abi>>()))>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename Abi, typename BinaryOperation, typename Selected>
    RAINY_CONSTEXPR26 Ty reduce_selected(const basic_vec<Ty, Abi> &val, Selected selected,
                                         type_traits::primary_types::type_identity_t<Ty> identity_element, BinaryOperation binary_op) {
        using carrier = basic_vec<Ty, scalar_abi>;
        using ops = simd_ops<Ty, Abi>;
        const auto data = static_cast<typename basic_vec<Ty, Abi>::register_type>(val);
        carrier acc(identity_element);
        bool started = false;
        for (simd_size_type i = 0; i < simd_size_v<Ty, Abi>; ++i) {
            if (selected(i)) {
                const Ty lane = ops::get_lane(data, i);
                if (!started) {
                    acc = carrier(lane);
                    started = true;
                } else {
                    acc = binary_op(acc, carrier(lane));
                }
            }
        }
        return started ? acc[0] : identity_element;
    }
}

namespace rainy::core::implements {
    template <typename Elem, typename Abi, simd_size_type N, typename Source>
    RAINY_CONSTEXPR26 auto chunk_vec_impl(const Source &val) noexcept {
        using result_t = basic_vec<Elem, scalar_abi>;
        result_t out[N]{};
        for (simd_size_type i = 0; i < N; ++i) {
            out[i] = result_t(static_cast<Elem>(val[i]));
        }
        return out;
    }

    template <std::size_t Bytes, typename Abi, simd_size_type N, typename Source>
    RAINY_CONSTEXPR26 auto chunk_mask_impl(const Source &val) noexcept {
        using mask_t = basic_mask<Bytes, scalar_abi>;
        mask_t out[N]{};
        for (simd_size_type i = 0; i < N; ++i) {
            out[i] = mask_t(val[i]);
        }
        return out;
    }
}


namespace rainy::core {
    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 auto chunk(const basic_vec<typename Ty::value_type, Abi> &val) noexcept {
        using elem_t = typename Ty::value_type;
        constexpr simd_size_type N = simd_size_v<elem_t, Abi>;
        return implements::chunk_vec_impl<elem_t, Abi, N>(val);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 auto chunk(const basic_mask<mask_element_size<Ty>, Abi> &val) noexcept {
        using elem_t = Ty;
        constexpr std::size_t Bytes = mask_element_size<Ty>;
        constexpr simd_size_type N = simd_size_v<elem_t, Abi>;
        return implements::chunk_mask_impl<Bytes, Abi, N>(val);
    }

    template <simd_size_type N, typename Ty, typename Abi>
    RAINY_CONSTEXPR26 auto chunk(const basic_vec<Ty, Abi> &val) noexcept {
        static_assert(N <= simd_size_v<Ty, Abi>, "chunk<N>: N exceeds lane count");
        return implements::chunk_vec_impl<Ty, Abi, N>(val);
    }

    template <simd_size_type N, std::size_t Bytes, typename Abi>
    RAINY_CONSTEXPR26 auto chunk(const basic_mask<Bytes, Abi> &val) noexcept {
        static_assert(N <= simd_size_v<unsigned char, Abi> * Bytes,
                      "chunk<N>: N exceeds mask bit count");
        return implements::chunk_mask_impl<Bytes, Abi, N>(val);
    }

    template <typename Ty, typename Abi, typename BinaryOperation,
              type_traits::other_trans::enable_if_t<implements::is_reduction_op<BinaryOperation, Ty>::value, int>>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, BinaryOperation binary_op) {
        return implements::reduce_selected(
            val, [](simd_size_type) { return true; }, type_traits::primary_types::type_identity_t<Ty>{}, binary_op);
    }

    template <typename Ty, typename Abi, typename BinaryOperation>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                                type_traits::primary_types::type_identity_t<Ty> identity_element, BinaryOperation binary_op) {
        return implements::reduce_selected(val, [&mask](simd_size_type i) { return mask[i]; }, identity_element, binary_op);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                                functional::plus<> binary_op) noexcept {
        return reduce(val, mask, type_traits::primary_types::type_identity_t<Ty>{}, binary_op);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                                functional::multiplies<> binary_op) noexcept {
        return reduce(val, mask, type_traits::primary_types::type_identity_t<Ty>(1), binary_op);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                                functional::bit_and<> binary_op) noexcept {
        return reduce(val, mask, type_traits::primary_types::type_identity_t<Ty>(~Ty()), binary_op);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                                functional::bit_or<> binary_op) noexcept {
        return reduce(val, mask, type_traits::primary_types::type_identity_t<Ty>{}, binary_op);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask,
                                functional::bit_xor<> binary_op) noexcept {
        return reduce(val, mask, type_traits::primary_types::type_identity_t<Ty>{}, binary_op);
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce_min(const basic_vec<Ty, Abi> &val) noexcept {
        using ops = implements::simd_ops<Ty, Abi>;
        const auto data = static_cast<typename basic_vec<Ty, Abi>::register_type>(val);
        Ty result = ops::get_lane(data, 0);
        for (simd_size_type i = 1; i < simd_size_v<Ty, Abi>; ++i) {
            const Ty lane = ops::get_lane(data, i);
            if (lane < result) {
                result = lane;
            }
        }
        return result;
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce_min(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask) noexcept {
        Ty result = utility::numeric_limits<Ty>::max();
        for (simd_size_type i = 0; i < simd_size_v<Ty, Abi>; ++i) {
            if (mask[i] && val[i] < result) {
                result = val[i];
            }
        }
        return result;
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce_max(const basic_vec<Ty, Abi> &val) noexcept {
        using ops = implements::simd_ops<Ty, Abi>;
        const auto data = static_cast<typename basic_vec<Ty, Abi>::register_type>(val);
        Ty result = ops::get_lane(data, 0);
        for (simd_size_type i = 1; i < simd_size_v<Ty, Abi>; ++i) {
            const Ty lane = ops::get_lane(data, i);
            if (lane > result) {
                result = lane;
            }
        }
        return result;
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 Ty reduce_max(const basic_vec<Ty, Abi> &val, const typename basic_vec<Ty, Abi>::mask_type &mask) noexcept {
        Ty result = utility::numeric_limits<Ty>::lowest();
        for (simd_size_type i = 0; i < simd_size_v<Ty, Abi>; ++i) {
            if (mask[i] && val[i] > result) {
                result = val[i];
            }
        }
        return result;
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 basic_vec<Ty, Abi> min(const basic_vec<Ty, Abi> &a, const basic_vec<Ty, Abi> &b) noexcept {
        return basic_vec<Ty, Abi>{implements::simd_ops<Ty, Abi>::min_reg(static_cast<typename basic_vec<Ty, Abi>::register_type>(a),
                                                                         static_cast<typename basic_vec<Ty, Abi>::register_type>(b))};
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 basic_vec<Ty, Abi> max(const basic_vec<Ty, Abi> &a, const basic_vec<Ty, Abi> &b) noexcept {
        return basic_vec<Ty, Abi>{implements::simd_ops<Ty, Abi>::max_reg(static_cast<typename basic_vec<Ty, Abi>::register_type>(a),
                                                                         static_cast<typename basic_vec<Ty, Abi>::register_type>(b))};
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 container::pair<basic_vec<Ty, Abi>, basic_vec<Ty, Abi>> minmax(const basic_vec<Ty, Abi> &a,
                                                                                     const basic_vec<Ty, Abi> &b) noexcept {
        return container::pair<basic_vec<Ty, Abi>, basic_vec<Ty, Abi>>(min(a, b), max(a, b));
    }

    template <typename Ty, typename Abi>
    RAINY_CONSTEXPR26 basic_vec<Ty, Abi> clamp(const basic_vec<Ty, Abi> &val, const basic_vec<Ty, Abi> &lo, const basic_vec<Ty, Abi> &hi) {
        using ops = implements::simd_ops<Ty, Abi>;
        using reg = typename basic_vec<Ty, Abi>::register_type;
        auto val_reg = static_cast<reg>(val);
        auto lo_reg = static_cast<reg>(lo);
        auto hi_reg = static_cast<reg>(hi);
        auto stage1 = ops::blend(ops::cmp_lt(val_reg, lo_reg), lo_reg, val_reg);
        auto stage2 = ops::blend(ops::cmp_lt(hi_reg, stage1), hi_reg, stage1);
        return basic_vec<Ty, Abi>{stage2};
    }

    template <typename Ty, typename UTy>
    RAINY_CONSTEXPR26 auto select(bool c, const Ty &a, const UTy &b) -> type_traits::implements::remove_cvref_t<decltype(c ? a : b)> {
        return c ? a : b;
    }
}

#endif
