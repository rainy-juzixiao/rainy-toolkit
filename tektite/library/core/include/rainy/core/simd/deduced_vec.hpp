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
#ifndef RAINY_CORE_SIMD_DEDUCED_SIMD_HPP
#define RAINY_CORE_SIMD_DEDUCED_SIMD_HPP

#include <rainy/core/simd/fwd.hpp>

namespace rainy::core::implements {
    template <typename V, typename = void>
    struct is_enabled_vec : type_traits::helper::false_type {};

    template <typename V>
    struct is_enabled_vec<V, type_traits::implements::void_t<typename V::value_type, typename V::abi_type>>
        : type_traits::helper::bool_constant<
              type_traits::type_relations::is_same_v<V, basic_vec<typename V::value_type, typename V::abi_type>> &&
              type_traits::properties::is_default_constructible_v<V>> {};

    template <typename Ty>
    using deduced_vec_plus_t =
        type_traits::implements::remove_cvref_t<decltype(utility::declval<const Ty &>() + utility::declval<const Ty &>())>;

    template <typename Ty, typename = void>
    struct deduced_vec_impl {
        using type = void;
    };

    template <typename Ty>
    struct deduced_vec_impl<
        Ty, type_traits::implements::void_t<decltype(utility::declval<const Ty &>() + utility::declval<const Ty &>())>> {
        using type =
            type_traits::other_trans::conditional_t<is_enabled_vec<deduced_vec_plus_t<Ty>>::value, deduced_vec_plus_t<Ty>, void>;
    };

    template <typename Ty>
    struct deduced_vec : deduced_vec_impl<Ty> {};
}

#endif
