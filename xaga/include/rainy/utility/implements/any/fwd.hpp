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
#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_FWD_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_FWD_HPP

// NOLINTBEGIN

#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/functor.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/utility/implements/any/exceptions.hpp>
#include <rainy/meta/moon/tuple_like_traits.hpp>
#include <utility>
#include <variant>

// NOLINTEND

namespace rainy::utility {
    template <std::size_t Length = core::small_object_space_size - (sizeof(void *) * 2), std::size_t Align = alignof(std::max_align_t)>
    class basic_any;

    template <typename TargetType, typename = void>
    struct any_converter {
        static constexpr bool invalid_mark = true;

        static bool is_convertible(const foundation::ctti::typeinfo &) {
            return false;
        }
    };

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_convert_invocable = true;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL
        is_any_convert_invocable<Type, type_traits::other_trans::void_t<decltype(any_converter<Type>::invalid_mark)>> = false;

    template <typename Any>
    struct is_any : type_traits::helper::false_type {};

    template <std::size_t Length, std::size_t Align>
    struct is_any<basic_any<Length, Align>> : type_traits::helper::true_type {};

    template <typename Any>
    RAINY_CONSTEXPR_BOOL is_any_v = is_any<Any>::value;

    template <typename TargetType>
    RAINY_NODISCARD bool is_any_convertible(const foundation::ctti::typeinfo &source_type) noexcept {
        if constexpr (is_any_convert_invocable<TargetType>) {
            return utility::any_converter<TargetType>::is_convertible(source_type);
        } else {
            return source_type == rainy_typeid(TargetType);
        }
    }

    enum class any_iterator_category : std::int8_t {
        input_iterator,
        output_iterator,
        forward_iterator,
        bidirectional_iterator,
        random_access_iterator,
        contiguous_iterator // for cxx 20
    };

    struct any_default_match {
        any_default_match() noexcept = default;
        ~any_default_match() = default;
        any_default_match(const any_default_match &) = default;
        any_default_match(any_default_match &&) = default;
    };
}

namespace rainy::utility::implements {
    struct any_binding_package {
        const void *payload;
        const foundation::ctti::typeinfo *type;
    };

    template <typename BasicAny, typename Type>
    struct const_any_proxy_iterator;

    template <typename BasicAny, typename Type>
    struct any_proxy_iterator;
}

namespace rainy::utility::implements {
    template <typename Ty>
    using add_const_helper_for_access_element =
        type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;

    template <typename Ty>
    using access_elements_construct_type = type_traits::other_trans::conditional_t<
        type_traits::composite_types::is_reference_v<Ty>,
        type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_rvalue_reference_v<Ty>,
            type_traits::reference_modify::add_rvalue_reference_t<add_const_helper_for_access_element<Ty>>,
            type_traits::reference_modify::add_lvalue_reference_t<add_const_helper_for_access_element<Ty>>>,
        add_const_helper_for_access_element<Ty>>;

    template <typename Iter>
    constexpr any_iterator_category get_iterator_category() noexcept {
        using namespace type_traits::extras::iterators;
        if (is_contiguous_iterator_v<Iter>) {
            return any_iterator_category::contiguous_iterator;
        } else if constexpr (is_random_access_iterator_v<Iter>) { // NOLINT
            return any_iterator_category::random_access_iterator;
        } else if constexpr (is_bidirectional_iterator_v<Iter>) {
            return any_iterator_category::bidirectional_iterator;
        } else if constexpr (is_bidirectional_iterator_v<Iter>) {
            return any_iterator_category::forward_iterator;
        } else if constexpr (is_output_iterator_v<Iter> && !is_input_iterator_v<Iter>) {
            return any_iterator_category::output_iterator;
        } else {
            static_assert(is_input_iterator_v<Iter> && !is_output_iterator_v<Iter>);
            return any_iterator_category::input_iterator;
        }
    }

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_char_any_can_output = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_char_any_can_output<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::basic_ostream<char>>() << utility::declval<Ty>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_wchar_any_can_output = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_wchar_any_can_output<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::basic_ostream<wchar_t>>() << utility::declval<Ty>())>> =
        true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_index_tuple_v = false;

    template <typename... Args>
    RAINY_CONSTEXPR_BOOL is_index_tuple_v<std::tuple<Args...>> = true;
}

namespace rainy::utility::implements {
    enum class any_compare_operation {
        less,
        less_eq,
        eq,
        greater_eq,
        greater
    };

    enum class any_operation {
        compare,
        eval_hash,
        query_for_is_tuple_like,
        destructre_this_pack,
        output_any,
        add,
        subtract,
        incr_prefix,
        decr_prefix,
        incr_postfix,
        decr_postfix,
        multiply,
        divide,
        mod,
        dereference,
        access_element,
        call_begin,
        call_end,
        assign,
        get_reference,
        get_lvalue_reference,
        get_rvalue_reference,
        construct_from,
        swap_value
    };
}

namespace rainy::utility::implements {
    template <typename Ty>
    constexpr std::size_t eval_for_destructure_pack_receiver_size() {
        using implements::any_binding_package;
        using namespace type_traits;
        using namespace type_traits::primary_types;
        if constexpr (function_traits<Ty>::valid && !is_member_object_pointer_v<Ty>) {
            return function_traits<Ty>::arity;
        } else if constexpr (is_pair_v<Ty>) {
            return pair_traits<Ty>::size;
        } else if constexpr (is_tuple_v<Ty>) {
            return tuple_traits<Ty>::size;
        } else if constexpr (constexpr std::size_t size = member_count_v<cv_modify::remove_cvref_t<Ty>>; size != 0) {
            return size;
        } else {
            return 0;
        }
    }
}

#endif
