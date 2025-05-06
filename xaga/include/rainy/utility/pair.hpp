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
#ifndef RAINY_UTILITY_PAIR_HPP
#define RAINY_UTILITY_PAIR_HPP

#include <rainy/core/core.hpp>
#include <utility>

namespace rainy::utility {
    template <typename Ty1, typename Ty2>
    struct pair {
        using first_type = Ty1;
        using second_type = Ty2;

        template <
            typename uty1 = Ty1, typename uty2 = Ty2,
            std::enable_if_t<std::conjunction_v<std::is_default_constructible<uty1>, std::is_default_constructible<uty2>>, int> = 0>
        constexpr pair() noexcept(std::is_nothrow_default_constructible_v<Ty1> && std::is_nothrow_default_constructible_v<Ty1>) :
            first(), second() {
        }

        constexpr pair(const pair &) = default;

        constexpr pair(pair &&) = default;

        template <typename uty1 = Ty1, typename uty2 = Ty2,
                  typename = std::enable_if_t<std::conjunction_v<std::is_copy_constructible<uty1>, std::is_copy_constructible<uty2>>>>
        constexpr pair(const Ty1 &val1, const Ty2 &val2) noexcept(std::is_nothrow_copy_constructible_v<uty1> &&
                                                                  std::is_nothrow_copy_constructible_v<uty2>) :
            first(val1), second(val2) {
        }

        template <typename other1, typename other2,
                  typename = std::enable_if_t<
                      std::conjunction_v<std::is_constructible<Ty1, const other1 &>, std::is_constructible<Ty2, const other2 &>>>>
        constexpr pair(const pair<other1, other2> &right) noexcept(std::is_nothrow_constructible_v<Ty1, other1> &&
                                                                   std::is_nothrow_constructible_v<Ty2, other2>) :
            first(right.first), second(right.second) {
        }

        template <
            typename other1, typename other2,
            typename = std::enable_if_t<std::conjunction_v<std::is_constructible<Ty1, other1>, std::is_constructible<Ty2, other2>>>>
        constexpr pair(const pair<other1, other2> &&right) noexcept(std::is_nothrow_constructible_v<Ty1, other1> &&
                                                                    std::is_nothrow_constructible_v<Ty2, other2>) :
            first(utility::forward<const other1>(right.first)), second(utility::forward<const other2>(right.second)) {
        }

        template <
            typename other1, typename other2,
            typename = std::enable_if_t<std::conjunction_v<std::is_constructible<Ty1, other1>, std::is_constructible<Ty2, other2>>>>
        constexpr pair(other1 &&val1, other2 &&val2) noexcept(std::is_nothrow_constructible_v<Ty1, other1> &&
                                                              std::is_nothrow_constructible_v<Ty2, other2>) :
            first(utility::forward<other1>(val1)), second(utility::forward<other2>(val2)) {
        }

        template <typename... Tuple1, typename... Tuple2, size_t... Indices1, size_t... Indices2>
        constexpr pair(std::tuple<Tuple1...> &&first_args, std::tuple<Tuple2...> &&second_args, std::index_sequence<Indices1...>,
                       std::index_sequence<Indices2...>) :
            first(std::get<Indices1>(std::move(first_args))...), second(std::get<Indices2>(std::move(second_args))...) {
        }

        template <typename... Args1, typename... Args2>
        constexpr pair(std::piecewise_construct_t, std::tuple<Args1...> &&first_args, std::tuple<Args2...> &&second_args) :
            pair(std::move(first_args), std::move(second_args), std::index_sequence_for<Args1...>{},
                 std::index_sequence_for<Args2...>{}) {
        }

        constexpr pair &operator=(const volatile pair &) = delete;

        template <typename Other1, typename Other2,
                  type_traits::other_trans::enable_if_t<
            type_traits::logical_traits::conjunction_v<
                type_traits::logical_traits::negation<
                    type_traits::type_relations::is_same<pair, pair<Other1, Other2>>>, type_traits::type_properties:: is_assignable<Ty1 &, const Other1 &>,
                    type_traits::type_properties::is_assignable<Ty2 &, const Other2 &>
            >,
                              int> = 0>
        constexpr pair &operator=(const pair<Other1, Other2> &right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Ty1 &, const Other1 &> &&
            type_traits::type_properties::is_nothrow_assignable_v<Ty2 &, const Other2 &>) /* strengthened */ {
            first = right.first;
            second = right.second;
            return *this;
        }

        template <typename U1, typename U2,
                  typename = std::enable_if_t<
                      std::conjunction_v<std::is_assignable<Ty1 &, const U1 &>, std::is_assignable<Ty2 &, const U2 &>>>>
        constexpr pair &operator=(const pair<U1, U2> &p) {
            first = p.first;
            second = p.second;
            return *this;
        }

        constexpr pair &operator=(pair &&p) noexcept {
            first = p.first;
            second = p.second;
            return *this;
        }

        template <typename U1, typename U2>
        constexpr pair &operator=(pair<U1, U2> &&p) {
            first = std::exchange(p.first, {});
            second = std::exchange(p.second, {});
            return *this;
        }

        constexpr void swap(pair &p) noexcept(std::is_nothrow_swappable_v<first_type> && std::is_nothrow_swappable_v<second_type>) {
            if (this != std::addressof(p)) {
                std::swap(first, p.first);
                std::swap(second, p.second);
            }
        }

        Ty1 first;
        Ty2 second;
    };

    template <typename Ty1, typename Ty2>
    constexpr auto make_pair(const Ty1 &val1, const Ty2 &val2) noexcept(
        std::is_nothrow_constructible_v<rainy::utility::pair<Ty1, Ty2>, const Ty1 &, const Ty2 &>) {
        return pair<Ty1, Ty2>(val1, val2);
    }
}


#if RAINY_HAS_CXX20
template <typename Ty1, typename Ty2>
class std::formatter<rainy::utility::pair<Ty1, Ty2>, char> {
public:
    static_assert(rainy::type_traits::concepts::formattable_with<Ty1, std::format_context>, "Ty1 Is A unsupported type");
    static_assert(rainy::type_traits::concepts::formattable_with<Ty2, std::format_context>, "Ty2 Is A unsupported type");

    explicit formatter() noexcept {
    }

    auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    auto format(const rainy::utility::pair<Ty1, Ty2> &value, std::format_context fc) const noexcept {
        return std::format_to(fc.out(), "{}:{}", value.first, value.second);
    }
};
#endif

#endif