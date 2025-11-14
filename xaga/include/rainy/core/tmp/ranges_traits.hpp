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
#ifndef RAINY_CORE_TMP_RANGES_TRAITS_HPP
#define RAINY_CORE_TMP_RANGES_TRAITS_HPP
#include <rainy/core/platform.hpp>

namespace rainy::type_traits::extras::ranges::implements {
    template <typename Range, typename = void>
    struct add_range_traits {};

    template <typename Range>
    struct add_range_traits<
        Range, other_trans::void_t<typename Range::difference_type, typename Range::value_type, typename Range::reference,
                                   typename Range::const_reference, typename Range::iterator, typename Range::const_iterator>> {
        using difference_type = typename Range::difference_type;
        using value_type = typename Range::value_type;
        using reference = typename Range::reference;
        using const_reference = typename Range::const_reference;
        using iterator = typename Range::iterator;
        using const_iterator = typename Range::const_iterator;
    };
}

namespace rainy::type_traits::extras::ranges {
    template <typename Range>
    struct range_traits : implements::add_range_traits<Range> {};

    template <typename Range>
    struct range_reference {
        using type = typename range_traits<Range>::reference;
    };

    template <typename Range>
    using range_reference_t = typename range_reference<Range>::type;

    template <typename Range>
    struct range_const_reference {
        using type = typename range_traits<Range>::const_reference;
    };

    template <typename Range>
    using range_const_reference_t = typename range_const_reference<Range>::type;

    template <typename Range>
    struct range_difference {
        using type = typename range_traits<Range>::difference_type;
    };

    template <typename Range>
    using range_difference_t = typename range_difference<Range>::type;

    template <typename Range>
    struct range_value_type {
        using type = typename range_traits<Range>::value_type;
    };

    template <typename Range>
    using range_value_type_t = typename range_value_type<Range>::type;
}

#endif