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
#ifndef RAINY_CORE_TYPE_TRAITS_ITER_TRAITS_HPP
#define RAINY_CORE_TYPE_TRAITS_ITER_TRAITS_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/modifers.hpp>

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Primary template for detecting if a type has a nested `difference_type` member.
     *        检测类型是否具有嵌套的 `difference_type` 成员的主模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct has_difference_type : helper::false_type {};

    /**
     * @brief Specialization that detects the presence of `Ty::difference_type`.
     *        检测是否存在 `Ty::difference_type` 的特化。
     *
     * @tparam Ty The type that provides `difference_type`
     *            提供 `difference_type` 的类型
     */
    template <typename Ty>
    struct has_difference_type<Ty, other_trans::void_t<typename Ty::difference_type>> : helper::true_type {};

    /**
     * @brief Variable template indicating whether a type has a nested `difference_type` member.
     *        指示类型是否具有嵌套的 `difference_type` 成员的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool has_difference_type_v = has_difference_type<Ty>::value;

    /**
     * @brief Primary template for detecting if a type has a nested `value_type` member.
     *        检测类型是否具有嵌套的 `value_type` 成员的主模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct has_value_type : helper::false_type {};

    /**
     * @brief Specialization that detects the presence of `Ty::value_type`.
     *        检测是否存在 `Ty::value_type` 的特化。
     *
     * @tparam Ty The type that provides `value_type`
     *            提供 `value_type` 的类型
     */
    template <typename Ty>
    struct has_value_type<Ty, other_trans::void_t<typename Ty::value_type>> : helper::true_type {};

    /**
     * @brief Variable template indicating whether a type has a nested `value_type` member.
     *        指示类型是否具有嵌套的 `value_type` 成员的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool has_value_type_v = has_value_type<Ty>::value;

    /**
     * @brief Primary template for detecting if a type has a nested `pointer` member.
     *        检测类型是否具有嵌套的 `pointer` 成员的主模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct has_pointer : helper::false_type {};

    /**
     * @brief Specialization that detects the presence of `Ty::pointer`.
     *        检测是否存在 `Ty::pointer` 的特化。
     *
     * @tparam Ty The type that provides `pointer`
     *            提供 `pointer` 的类型
     */
    template <typename Ty>
    struct has_pointer<Ty, other_trans::void_t<typename Ty::pointer>> : helper::true_type {};

    /**
     * @brief Variable template indicating whether a type has a nested `pointer` member.
     *        指示类型是否具有嵌套的 `pointer` 成员的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool has_pointer_v = has_pointer<Ty>::value;

    /**
     * @brief Primary template for detecting if a type has a nested `reference` member.
     *        检测类型是否具有嵌套的 `reference` 成员的主模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct has_reference : helper::false_type {};

    /**
     * @brief Specialization that detects the presence of `Ty::reference`.
     *        检测是否存在 `Ty::reference` 的特化。
     *
     * @tparam Ty The type that provides `reference`
     *            提供 `reference` 的类型
     */
    template <typename Ty>
    struct has_reference<Ty, other_trans::void_t<typename Ty::reference>> : helper::true_type {};

    /**
     * @brief Variable template indicating whether a type has a nested `reference` member.
     *        指示类型是否具有嵌套的 `reference` 成员的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool has_reference_v = has_reference<Ty>::value;

    /**
     * @brief Primary template for detecting if a type has a nested `iterator_category` member.
     *        检测类型是否具有嵌套的 `iterator_category` 成员的主模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct has_iterator_category : helper::false_type {};

    /**
     * @brief Specialization that detects the presence of `Ty::iterator_category`.
     *        检测是否存在 `Ty::iterator_category` 的特化。
     *
     * @tparam Ty The type that provides `iterator_category`
     *            提供 `iterator_category` 的类型
     */
    template <typename Ty>
    struct has_iterator_category<Ty, other_trans::void_t<typename Ty::iterator_category>> : helper::true_type {};

    /**
     * @brief Variable template indicating whether a type has a nested `iterator_category` member.
     *        指示类型是否具有嵌套的 `iterator_category` 成员的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool has_iterator_category_v = has_iterator_category<Ty>::value;

    /**
     * @brief Primary template for detecting if a type has a nested `element_type` member.
     *        检测类型是否具有嵌套的 `element_type` 成员的主模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct has_element_type : helper::false_type {};

    /**
     * @brief Specialization that detects the presence of `Ty::element_type`.
     *        检测是否存在 `Ty::element_type` 的特化。
     *
     * @tparam Ty The type that provides `element_type`
     *            提供 `element_type` 的类型
     */
    template <typename Ty>
    struct has_element_type<Ty, other_trans::void_t<typename Ty::element_type>> : helper::true_type {};

    /**
     * @brief Variable template indicating whether a type has a nested `element_type` member.
     *        指示类型是否具有嵌套的 `element_type` 成员的变量模板。
     *
     * @tparam Ty The type to examine
     *            要检查的类型
     */
    template <typename Ty>
    inline constexpr bool has_element_type_v = has_element_type<Ty>::value;
}

namespace rainy::type_traits::extras::iterators::implements {
    template <typename Ty, bool Enable = type_traits::extras::iterators::has_value_type_v<Ty>>
    struct try_to_add_value_type {
        using value_type =  utility::invalid_type;
    };

    template <typename Ty>
    struct try_to_add_value_type<Ty, true> {
        using value_type = typename Ty::value_type;
    };

    template <typename Ty, bool Enable = type_traits::extras::iterators::has_difference_type_v<Ty>>
    struct try_to_add_difference_type {
        using difference_type = utility::invalid_type;
    };

    template <typename Ty>
    struct try_to_add_difference_type<Ty, true> {
        using difference_type = typename Ty::difference_type;
    };

    template <typename Ty, bool IsPointer = type_traits::implements::_is_pointer_v<Ty>,
              bool Enable = type_traits::extras::iterators::has_iterator_category_v<Ty>>
    struct try_to_add_iterator_category {
        using iterator_category = utility::invalid_type;
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, true, false> {
        using iterator_category = std::random_access_iterator_tag; // 为了兼容标准库设计
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, false, true> {
        using iterator_category = typename Ty::iterator_category;
    };

    template <typename Ty, bool Enable = type_traits::extras::iterators::has_reference_v<Ty>>
    struct try_to_add_reference {
        using reference = utility::invalid_type;
    };

    template <typename Ty>
    struct try_to_add_reference<Ty, true> {
        using reference = typename Ty::reference;
    };

    template <typename Ty, bool Enable = type_traits::extras::iterators::has_pointer_v<Ty>>
    struct try_to_add_pointer {
        using pointer = utility::invalid_type;
    };

    template <typename Ty>
    struct try_to_add_pointer<Ty, true> {
        using pointer = typename Ty::pointer;
    };

    template <typename Ty, bool Enable = type_traits::extras::iterators::has_element_type_v<Ty>>
    struct try_to_add_element_type {
        using element_type = utility::invalid_type;
    };

    template <typename Ty>
    struct try_to_add_element_type<Ty, true> {
        using element_type = typename Ty::element_type;
    };

    template <typename Ty>
    struct iterator_traits_base : try_to_add_difference_type<Ty>,
                                  try_to_add_iterator_category<Ty>,
                                  try_to_add_pointer<Ty>,
                                  try_to_add_reference<Ty>,
                                  try_to_add_value_type<Ty> {};
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Primary template for iterator_traits, providing uniform interface for iterator properties.
     *        iterator_traits 的主模板，为迭代器属性提供统一接口。
     *
     * @tparam Ty The iterator type to examine
     *            要检查的迭代器类型
     */
    template <typename Ty>
    struct iterator_traits : implements::iterator_traits_base<Ty> {};

    /**
     * @brief Partial specialization of iterator_traits for raw pointers.
     *        iterator_traits 对原始指针的偏特化。
     *
     * This specialization provides the standard iterator traits for pointer types,
     * treating them as random access iterators.
     *
     * 此特化为指针类型提供标准的迭代器特性，将它们视为随机访问迭代器。
     *
     * @tparam Ty The pointed-to type
     *            指向的类型
     */
    template <typename Ty>
    struct iterator_traits<Ty *> {
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        using pointer = Ty *;
        using reference = Ty &;
        using value_type = Ty;
    };

    /**
     * @brief Helper template to explicitly define iterator traits for custom iterators.
     *        为自定义迭代器显式定义迭代器特性的辅助模板。
     *
     * This template allows users to create a custom iterator_traits specialization
     * by providing all five standard iterator trait types.
     *
     * 此模板允许用户通过提供所有五个标准迭代器特性类型来创建自定义的 iterator_traits 特化。
     *
     * @tparam DifferenceType The type used to represent distances between iterators
     *                        用于表示迭代器之间距离的类型
     * @tparam Category The iterator category tag (e.g., std::forward_iterator_tag)
     *                  迭代器类别标签（例如 std::forward_iterator_tag）
     * @tparam Pointer The pointer type (usually value_type*)
     *                 指针类型（通常是 value_type*）
     * @tparam Reference The reference type (usually value_type&)
     *                   引用类型（通常是 value_type&）
     * @tparam ValueType The type of the elements pointed to by the iterator
     *                   迭代器指向的元素类型
     */
    template <typename DifferenceType, typename Category, typename Pointer, typename Reference, typename ValueType>
    struct make_iterator_traits {
        using difference_type = DifferenceType;
        using iterator_category = Category;
        using pointer = Pointer;
        using reference = Reference;
        using value_type = ValueType;
    };
}

namespace rainy::utility {
    using type_traits::extras::iterators::iterator_traits;
    using type_traits::extras::iterators::make_iterator_traits;
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Template for obtaining the value type of an iterator.
     *        获取迭代器的值类型的模板。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    struct iter_value {
        using type = typename utility::iterator_traits<Iter>::value_type;
    };

    /**
     * @brief Alias template for iter_value, providing direct access to the iterator's value type.
     *        iter_value 的别名模板，提供对迭代器值类型的直接访问。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    using iter_value_t = typename iter_value<Iter>::type;

    /**
     * @brief Template for obtaining the difference type of an iterator.
     *        获取迭代器的差类型的模板。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    struct iterator_difference {
        using type = typename utility::iterator_traits<Iter>::difference_type;
    };

    /**
     * @brief Alias template for iterator_difference, providing direct access to the iterator's difference type.
     *        iterator_difference 的别名模板，提供对迭代器差类型的直接访问。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    using iterator_difference_t = typename iterator_difference<Iter>::type;

    /**
     * @brief Template for obtaining the reference type of an iterator.
     *        获取迭代器的引用类型的模板。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    struct iterator_reference {
        using type = typename utility::iterator_traits<Iter>::reference;
    };

    /**
     * @brief Alias template for iterator_reference, providing direct access to the iterator's reference type.
     *        iterator_reference 的别名模板，提供对迭代器引用类型的直接访问。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    using iterator_reference_t = typename iterator_reference<Iter>::type;

    /**
     * @brief Template for obtaining the iterator category of an iterator.
     *        获取迭代器的迭代器类别的模板。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    struct iterator_category {
        using type = typename utility::iterator_traits<Iter>::iterator_category;
    };

    /**
     * @brief Alias template for iterator_category, providing direct access to the iterator's category.
     *        iterator_category 的别名模板，提供对迭代器类别的直接访问。
     *
     * @tparam Iter The iterator type
     *              迭代器类型
     */
    template <typename Iter>
    using iterator_category_t = typename iterator_reference<Iter>::type;
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Template for obtaining the iterator type of a container.
     *        获取容器的迭代器类型的模板。
     *
     * @tparam Ty The container type
     *            容器类型
     */
    template <typename Ty>
    struct iterator {
        using type = type_traits::reference_modify::remove_reference_t<decltype(utility::declval<Ty &>().begin())>;
    };

    /**
     * @brief Alias template for iterator, providing direct access to a container's iterator type.
     *        iterator 的别名模板，提供对容器迭代器类型的直接访问。
     *
     * @tparam Ty The container type
     *            容器类型
     */
    template <typename Ty>
    using iterator_t = typename iterator<Ty>::type;

    /**
     * @brief Template for obtaining the const iterator type of a container.
     *        获取容器的常量迭代器类型的模板。
     *
     * @tparam Ty The container type
     *            容器类型
     */
    template <typename Ty>
    struct const_iterator {
        using type = type_traits::reference_modify::remove_reference_t<decltype(utility::declval<const Ty &>().cbegin())>;
    };

    /**
     * @brief Alias template for const_iterator, providing direct access to a container's const iterator type.
     *        const_iterator 的别名模板，提供对容器常量迭代器类型的直接访问。
     *
     * @tparam Ty The container type
     *            容器类型
     */
    template <typename Ty>
    using const_iterator_t = typename const_iterator<Ty>::type;

    /**
     * @brief Variable template for checking if a type satisfies the iterator concept.
     *        检查类型是否满足迭代器概念的变量模板。
     *
     * @tparam Iter The type to check
     *              要检查的类型
     */
    template <typename Iter, typename = void>
    RAINY_CONSTEXPR_BOOL is_iterator_v =
        !type_traits::type_relations::is_same_v<typename utility::iterator_traits<Iter>::iterator_category, utility::invalid_type>;
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Variable template for checking if a container has a nested `iterator` type.
     *        检查容器是否具有嵌套的 `iterator` 类型的变量模板。
     *
     * @tparam Ty The container type to check
     *            要检查的容器类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_iterator_v = false;

    /**
     * @brief Specialization that detects the presence of `Ty::iterator`.
     *        检测是否存在 `Ty::iterator` 的特化。
     *
     * @tparam Ty The container type that provides `iterator`
     *            提供 `iterator` 的容器类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_iterator_v<Ty, other_trans::void_t<typename cv_modify::remove_cvref_t<Ty>::iterator>> = true;

    /**
     * @brief Variable template for checking if a container has a nested `const_iterator` type.
     *        检查容器是否具有嵌套的 `const_iterator` 类型的变量模板。
     *
     * @tparam Ty The container type to check
     *            要检查的容器类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_iterator_v = false;

    /**
     * @brief Specialization that detects the presence of `Ty::const_iterator`.
     *        检测是否存在 `Ty::const_iterator` 的特化。
     *
     * @tparam Ty The container type that provides `const_iterator`
     *            提供 `const_iterator` 的容器类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_iterator_v<Ty, other_trans::void_t<typename cv_modify::remove_cvref_t<Ty>::const_iterator>> = true;
}

#endif
