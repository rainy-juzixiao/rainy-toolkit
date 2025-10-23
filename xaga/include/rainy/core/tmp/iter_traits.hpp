#ifndef RAINY_CORE_TMP_ITER_TRAITS_HPP
#define RAINY_CORE_TMP_ITER_TRAITS_HPP
#include <rainy/core/platform.hpp>

namespace rainy::type_traits::extras::templates {
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

namespace rainy::utility::implements {
    template <typename Ty, bool Enable = type_traits::extras::templates::has_value_type_v<Ty>>
    struct try_to_add_value_type {
        using value_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_value_type<Ty, true> {
        using value_type = typename Ty::value_type;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_difference_type_v<Ty>>
    struct try_to_add_difference_type {
        using difference_type = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_difference_type<Ty, true> {
        using difference_type = typename Ty::difference_type;
    };

    template <typename Ty, bool IsPointer = type_traits::implements::_is_pointer_v<Ty>,
              bool Enable = type_traits::extras::templates::has_iterator_category_v<Ty>>
    struct try_to_add_iterator_category {
        using iterator_category = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, true, false> {
        using iterator_category = std::random_access_iterator_tag; // 为了兼容标准库设计
    };

    template <typename Ty>
    struct try_to_add_iterator_category<Ty, false, true> {
        using iterator_category = typename Ty::iterator_category;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_reference_v<Ty>>
    struct try_to_add_reference {
        using reference = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_reference<Ty, true> {
        using reference = typename Ty::reference;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_pointer_v<Ty>>
    struct try_to_add_pointer {
        using pointer = invalid_type;
    };

    template <typename Ty>
    struct try_to_add_pointer<Ty, true> {
        using pointer = typename Ty::pointer;
    };

    template <typename Ty, bool Enable = type_traits::extras::templates::has_element_type_v<Ty>>
    struct try_to_add_element_type {
        using element_type = invalid_type;
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

namespace rainy::utility {
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

namespace rainy::type_traits::extras::templates {
    template <typename Iter>
    struct iter_value_type {
        using type = typename utility::iterator_traits<Iter>::value_type;
    };

    template <typename Iter, typename = void>
    RAINY_CONSTEXPR_BOOL is_iterator_v = !type_traits::type_relations::is_same_v<typename utility::iterator_traits<Iter>::iterator_category, utility::invalid_type>;
}

namespace rainy::type_traits::extras::templates {
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