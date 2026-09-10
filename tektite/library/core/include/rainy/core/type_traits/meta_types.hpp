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
#ifndef RAINY_CORE_TYPE_TRAITS_META_TYPE_HPP
#define RAINY_CORE_TYPE_TRAITS_META_TYPE_HPP

#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>

namespace rainy::type_traits::extras::meta_types {
    /**
     * @brief Variable template for checking if a type has a nested `value_type` member.
     *        检查类型是否具有嵌套的 `value_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_value_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::value_type.
     *        检测是否存在 Ty::value_type 的特化。
     *
     * @tparam Ty The type that provides value_type
     *            提供 value_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_value_type_v<Ty, type_traits::other_trans::void_t<typename Ty::value_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `value_type` member.
     *        检查类型是否具有嵌套的 `value_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_value_type : helper::bool_constant<has_value_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `size_type` member.
     *        检查类型是否具有嵌套的 `size_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_size_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::size_type.
     *        检测是否存在 Ty::size_type 的特化。
     *
     * @tparam Ty The type that provides size_type
     *            提供 size_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_size_type_v<Ty, type_traits::other_trans::void_t<typename Ty::size_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `size_type` member.
     *        检查类型是否具有嵌套的 `size_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_size_type : helper::bool_constant<has_size_type_v<Ty>> {};


    /**
     * @brief Variable template for checking if a type has a nested `difference_type` member.
     *        检查类型是否具有嵌套的 `difference_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_difference_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::difference_type.
     *        检测是否存在 Ty::difference_type 的特化。
     *
     * @tparam Ty The type that provides difference_type
     *            提供 difference_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_difference_type_v<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `difference_type` member.
     *        检查类型是否具有嵌套的 `difference_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_difference_type : helper::bool_constant<has_difference_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `reference` member.
     *        检查类型是否具有嵌套的 `reference` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_reference_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::reference.
     *        检测是否存在 Ty::reference 的特化。
     *
     * @tparam Ty The type that provides reference
     *            提供 reference 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_reference_v<Ty, type_traits::other_trans::void_t<typename Ty::reference>> = true;

    /**
     * @brief Type template for checking if a type has a nested `reference` member.
     *        检查类型是否具有嵌套的 `reference` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_reference : helper::bool_constant<has_reference_v<Ty>> {};


    /**
     * @brief Variable template for checking if a type has a nested `const_reference` member.
     *        检查类型是否具有嵌套的 `const_reference` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_reference_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::const_reference.
     *        检测是否存在 Ty::const_reference 的特化。
     *
     * @tparam Ty The type that provides const_reference
     *            提供 const_reference 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_reference_v<Ty, type_traits::other_trans::void_t<typename Ty::const_reference>> = true;

    /**
     * @brief Type template for checking if a type has a nested `const_reference` member.
     *        检查类型是否具有嵌套的 `const_reference` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_const_reference : helper::bool_constant<has_const_reference_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `pointer` member.
     *        检查类型是否具有嵌套的 `pointer` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_pointer_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::pointer.
     *        检测是否存在 Ty::pointer 的特化。
     *
     * @tparam Ty The type that provides pointer
     *            提供 pointer 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_pointer_v<Ty, type_traits::other_trans::void_t<typename Ty::pointer>> = true;

    /**
     * @brief Type template for checking if a type has a nested `pointer` member.
     *        检查类型是否具有嵌套的 `pointer` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_pointer : helper::bool_constant<has_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `const_pointer` member.
     *        检查类型是否具有嵌套的 `const_pointer` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_pointer_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::const_pointer.
     *        检测是否存在 Ty::const_pointer 的特化。
     *
     * @tparam Ty The type that provides const_pointer
     *            提供 const_pointer 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_pointer_v<Ty, type_traits::other_trans::void_t<typename Ty::const_pointer>> = true;

    /**
     * @brief Type template for checking if a type has a nested `const_pointer` member.
     *        检查类型是否具有嵌套的 `const_pointer` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_const_pointer : helper::bool_constant<has_const_pointer_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `iterator` type.
     *        检查类型是否具有嵌套的 `iterator` 类型的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_iterator_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::iterator.
     *        检测是否存在 Ty::iterator 的特化。
     *
     * @tparam Ty The type that provides iterator
     *            提供 iterator 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_iterator_type_v<Ty, type_traits::other_trans::void_t<typename Ty::iterator>> = true;

    /**
     * @brief Type template for checking if a type has a nested `iterator` type.
     *        检查类型是否具有嵌套的 `iterator` 类型的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_iterator_type : helper::bool_constant<has_iterator_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `const_iterator` type.
     *        检查类型是否具有嵌套的 `const_iterator` 类型的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_iterator_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::const_iterator.
     *        检测是否存在 Ty::const_iterator 的特化。
     *
     * @tparam Ty The type that provides const_iterator
     *            提供 const_iterator 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_iterator_type_v<Ty, type_traits::other_trans::void_t<typename Ty::const_iterator>> = true;

    /**
     * @brief Type template for checking if a type has a nested `const_iterator` type.
     *        检查类型是否具有嵌套的 `const_iterator` 类型的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_const_iterator_type : helper::bool_constant<has_const_iterator_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `reverse_iterator` type.
     *        检查类型是否具有嵌套的 `reverse_iterator` 类型的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_reverse_iterator_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::reverse_iterator.
     *        检测是否存在 Ty::reverse_iterator 的特化。
     *
     * @tparam Ty The type that provides reverse_iterator
     *            提供 reverse_iterator 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_reverse_iterator_v<Ty, type_traits::other_trans::void_t<typename Ty::reverse_iterator>> = true;

    /**
     * @brief Type template for checking if a type has a nested `reverse_iterator` type.
     *        检查类型是否具有嵌套的 `reverse_iterator` 类型的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_reverse_iterator : helper::bool_constant<has_reverse_iterator_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `const_reverse_iterator` type.
     *        检查类型是否具有嵌套的 `const_reverse_iterator` 类型的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_reverse_iterator_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::const_reverse_iterator.
     *        检测是否存在 Ty::const_reverse_iterator 的特化。
     *
     * @tparam Ty The type that provides const_reverse_iterator
     *            提供 const_reverse_iterator 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_reverse_iterator_v<Ty, type_traits::other_trans::void_t<typename Ty::const_reverse_iterator>> =
        true;

    /**
     * @brief Type template for checking if a type has a nested `const_reverse_iterator` type.
     *        检查类型是否具有嵌套的 `const_reverse_iterator` 类型的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_const_reverse_iterator : helper::bool_constant<has_const_reverse_iterator_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `key_type` member.
     *        检查类型是否具有嵌套的 `key_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_key_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::key_type.
     *        检测是否存在 Ty::key_type 的特化。
     *
     * @tparam Ty The type that provides key_type
     *            提供 key_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_key_type_v<Ty, type_traits::other_trans::void_t<typename Ty::key_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `key_type` member.
     *        检查类型是否具有嵌套的 `key_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_key_type : helper::bool_constant<has_key_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `mapped_type` member.
     *        检查类型是否具有嵌套的 `mapped_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::mapped_type.
     *        检测是否存在 Ty::mapped_type 的特化。
     *
     * @tparam Ty The type that provides mapped_type
     *            提供 mapped_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v<Ty, type_traits::other_trans::void_t<typename Ty::mapped_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `mapped_type` member.
     *        检查类型是否具有嵌套的 `mapped_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_mapped_type : helper::bool_constant<has_mapped_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `allocator_type` member.
     *        检查类型是否具有嵌套的 `allocator_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_allocator_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::allocator_type.
     *        检测是否存在 Ty::allocator_type 的特化。
     *
     * @tparam Ty The type that provides allocator_type
     *            提供 allocator_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_allocator_type_v<Ty, type_traits::other_trans::void_t<typename Ty::allocator_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `allocator_type` member.
     *        检查类型是否具有嵌套的 `allocator_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_allocator_type : helper::bool_constant<has_allocator_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested `char_type` member.
     *        检查类型是否具有嵌套的 `char_type` 成员的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_char_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::char_type.
     *        检测是否存在 Ty::char_type 的特化。
     *
     * @tparam Ty The type that provides char_type
     *            提供 char_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_char_type_v<Ty, type_traits::other_trans::void_t<typename Ty::char_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested `char_type` member.
     *        检查类型是否具有嵌套的 `char_type` 成员的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_char_type : helper::bool_constant<has_char_type_v<Ty>> {};
}

#endif
