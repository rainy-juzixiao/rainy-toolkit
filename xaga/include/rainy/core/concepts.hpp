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
#ifndef RAINY_CORE_CONCEPTS_HPP
#define RAINY_CORE_CONCEPTS_HPP

#if RAINY_HAS_CXX20

#include <rainy/core/type_traits.hpp>

namespace rainy::type_traits::concepts::implements {
    template <typename Ty, typename UTy>
    concept partially_ordered_with_impl = requires(const type_traits::reference_modify::remove_reference_t<Ty> &left,
                                                   const type_traits::reference_modify::remove_reference_t<UTy> &right) {
        { left <=> right } -> std::convertible_to<std::partial_ordering>;
        { right <=> left } -> std::convertible_to<std::partial_ordering>;
    };
}

namespace rainy::type_traits::concepts {
    template <typename Ty, typename Cat = std::partial_ordering>
    concept three_way_comparable = requires(const type_traits::reference_modify::remove_reference_t<Ty> &left,
                                            const type_traits::reference_modify::remove_reference_t<Ty> &right) {
        { left <=> right } -> std::convertible_to<Cat>;
    };

    template <typename Ty, typename UTy, typename Cat = std::partial_ordering>
    concept three_way_comparable_with = three_way_comparable<Ty, Cat> && three_way_comparable<UTy, Cat> &&
                                        requires(const type_traits::reference_modify::remove_reference_t<Ty> &left,
                                                 const type_traits::reference_modify::remove_reference_t<UTy> &right) {
                                            { left <=> right } -> std::convertible_to<Cat>;
                                            { right <=> left } -> std::convertible_to<Cat>;
                                        } &&
                                        requires(const type_traits::reference_modify::remove_reference_t<Ty> &left,
                                                 const type_traits::reference_modify::remove_reference_t<UTy> &right) {
                                            { left == right } -> std::convertible_to<bool>;
                                            { right == left } -> std::convertible_to<bool>;
                                        };
}

#endif

#endif
