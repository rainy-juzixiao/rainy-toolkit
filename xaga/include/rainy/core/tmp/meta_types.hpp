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
#ifndef RAINY_CORE_TMP_META_TYPE_HPP
#define RAINY_CORE_TMP_META_TYPE_HPP

#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/implements.hpp>

namespace rainy::type_traits::extras::meta_types {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_value_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_value_type_v<Ty, type_traits::other_trans::void_t<typename Ty::value_type>> = true;

    template <typename Ty>
    struct has_value_type : helper::bool_constant<has_value_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_size_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_size_type_v<Ty, type_traits::other_trans::void_t<typename Ty::size_type>> = true;

    template <typename Ty>
    struct has_size_type : helper::bool_constant<has_size_type_v<Ty>> {};


    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_difference_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_difference_type_v<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> = true;

    template <typename Ty>
    struct has_difference_type : helper::bool_constant<has_difference_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_reference_v<Ty, type_traits::other_trans::void_t<typename Ty::reference>> = true;

    template <typename Ty>
    struct has_reference : helper::bool_constant<has_reference_v<Ty>> {};


    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_reference_v<Ty, type_traits::other_trans::void_t<typename Ty::const_reference>> = true;

    template <typename Ty>
    struct has_const_reference : helper::bool_constant<has_const_reference_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_pointer_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_pointer_v<Ty, type_traits::other_trans::void_t<typename Ty::pointer>> = true;

    template <typename Ty>
    struct has_pointer : helper::bool_constant<has_pointer_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_pointer_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_pointer_v<Ty, type_traits::other_trans::void_t<typename Ty::const_pointer>> = true;

    template <typename Ty>
    struct has_const_pointer : helper::bool_constant<has_const_pointer_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_iterator_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_iterator_type_v<Ty, type_traits::other_trans::void_t<typename Ty::iterator>> = true;

    template <typename Ty>
    struct has_iterator_type : helper::bool_constant<has_iterator_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_iterator_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_iterator_type_v<Ty, type_traits::other_trans::void_t<typename Ty::const_iterator>> = true;

    template <typename Ty>
    struct has_const_iterator_type : helper::bool_constant<has_const_iterator_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_reverse_iterator_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_reverse_iterator_v<Ty, type_traits::other_trans::void_t<typename Ty::reverse_iterator>> = true;

    template <typename Ty>
    struct has_reverse_iterator : helper::bool_constant<has_reverse_iterator_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_reverse_iterator_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_reverse_iterator_v<Ty, type_traits::other_trans::void_t<typename Ty::const_reverse_iterator>> =
        true;

    template <typename Ty>
    struct has_const_reverse_iterator : helper::bool_constant<has_const_reverse_iterator_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_key_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_key_type_v<Ty, type_traits::other_trans::void_t<typename Ty::key_type>> = true;

    template <typename Ty>
    struct has_key_type : helper::bool_constant<has_key_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v<Ty, type_traits::other_trans::void_t<typename Ty::mapped_type>> = true;

    template <typename Ty>
    struct has_mapped_type : helper::bool_constant<has_mapped_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_allocator_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_allocator_type_v<Ty, type_traits::other_trans::void_t<typename Ty::allocator_type>> = true;

    template <typename Ty>
    struct has_allocator_type : helper::bool_constant<has_allocator_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_char_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_char_type_v<Ty, type_traits::other_trans::void_t<typename Ty::char_type>> = true;

    template <typename Ty>
    struct has_char_type : helper::bool_constant<has_char_type_v<Ty>> {};
}

#endif
