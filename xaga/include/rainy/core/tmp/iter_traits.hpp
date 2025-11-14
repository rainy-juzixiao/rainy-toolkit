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
#ifndef RAINY_CORE_TMP_ITER_TRAITS_HPP
#define RAINY_CORE_TMP_ITER_TRAITS_HPP
#include <rainy/core/platform.hpp>

namespace rainy::type_traits::extras::iterators {
    template <typename Ty, typename = void>
    struct has_difference_type : helper::false_type {};

    template <typename Ty>
    struct has_difference_type<Ty, other_trans::void_t<typename Ty::difference_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_difference_type_v = has_difference_type<Ty>::value;

    template <typename Ty, typename = void>
    struct has_value_type : helper::false_type {};

    template <typename Ty>
    struct has_value_type<Ty, other_trans::void_t<typename Ty::value_type>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_value_type_v = has_value_type<Ty>::value;

    template <typename Ty, typename = void>
    struct has_pointer : helper::false_type {};

    template <typename Ty>
    struct has_pointer<Ty, other_trans::void_t<typename Ty::pointer>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_pointer_v = has_pointer<Ty>::value;

    template <typename Ty, typename = void>
    struct has_reference : helper::false_type {};

    template <typename Ty>
    struct has_reference<Ty, other_trans::void_t<typename Ty::reference>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_reference_v = has_reference<Ty>::value;

    template <typename Ty, typename = void>
    struct has_iterator_category : helper::false_type {};

    template <typename Ty>
    struct has_iterator_category<Ty, other_trans::void_t<typename Ty::iterator_category>> : helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_iterator_category_v = has_iterator_category<Ty>::value;

    template <typename Ty, typename = void>
    struct has_element_type : helper::false_type {};

    template <typename Ty>
    struct has_element_type<Ty, other_trans::void_t<typename Ty::element_type>> : helper::true_type {};

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
    template <typename Ty>
    struct iterator_traits : implements::iterator_traits_base<Ty> {};

    template <typename Ty>
    struct iterator_traits<Ty *> {
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        using pointer = Ty *;
        using reference = Ty &;
        using value_type = Ty;
    };

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
    template <typename Iter>
    struct iter_value {
        using type = typename utility::iterator_traits<Iter>::value_type;
    };

    template <typename Iter>
    using iter_value_t = typename iter_value<Iter>::type;

    template <typename Iter>
    struct iterator_difference {
        using type = typename utility::iterator_traits<Iter>::difference_type;
    };

    template <typename Iter>
    using iterator_difference_t = typename iterator_difference<Iter>::type;

    template <typename Iter>
    struct iterator_reference {
        using type = typename utility::iterator_traits<Iter>::reference;
    };

    template <typename Iter>
    using iterator_reference_t = typename iterator_reference<Iter>::type;

    template <typename Iter>
    struct iterator_category {
        using type = typename utility::iterator_traits<Iter>::iterator_category;
    };

    template <typename Iter>
    using iterator_category_t = typename iterator_reference<Iter>::type;
}

namespace rainy::type_traits::extras::iterators {
    template <typename Ty>
    struct iterator {
        using type = type_traits::reference_modify::remove_reference_t<decltype(utility::declval<Ty &>().begin())>;
    };

    template <typename Ty>
    using iterator_t = typename iterator<Ty>::type;

    template <typename Ty>
    struct const_iterator {
        using type = type_traits::reference_modify::remove_reference_t<decltype(utility::declval<const Ty &>().cbegin())>;
    };

    template <typename Ty>
    using const_iterator_t = typename const_iterator<Ty>::type;

    template <typename Iter, typename = void>
    RAINY_CONSTEXPR_BOOL is_iterator_v =
        !type_traits::type_relations::is_same_v<typename utility::iterator_traits<Iter>::iterator_category, utility::invalid_type>;
}

namespace rainy::type_traits::extras::iterators {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_iterator_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_iterator_v<Ty, other_trans::void_t<typename cv_modify::remove_cvref_t<Ty>::iterator>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_const_iterator_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_const_iterator_v<Ty, other_trans::void_t<typename cv_modify::remove_cvref_t<Ty>::const_iterator>> = true;
}

#endif
