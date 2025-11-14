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
#ifndef RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_PIPELINE_HPP
#define RAINY_COLLECTIONS_VIEWS_IMPLEMENTS_PIPELINE_HPP
#include <rainy/core/core.hpp>

namespace rainy::collections::views::implements {
    template <typename _Derived>
    struct base {};

    template <typename Ty>
    Ty *derived_from_range_adaptor_closure(base<Ty> &) {
        return nullptr;
    }

    template <typename Ty, typename = void>
    struct is_range : std::false_type {};

    template <typename Ty>
    struct is_range<Ty, std::void_t<decltype(utility::declval<Ty>().begin()), decltype(utility::declval<Ty>().end())>>
        : std::true_type {};

    template <typename Ty, typename = void>
    struct is_range_adaptor_closure_object : std::false_type {};

    template <typename Ty>
    struct is_range_adaptor_closure_object<
        Ty, std::void_t<decltype(derived_from_range_adaptor_closure(utility::declval<base<typename std::remove_cv<Ty>::type> &>()))>>
        : std::integral_constant<bool, !is_range<typename std::remove_cv<Ty>::type>::value> {};

    template <typename Ty>
    using enable_if_range_adaptor_closure = std::enable_if_t<is_range_adaptor_closure_object<Ty>::value, int>;

    template <typename ClosureLeft, typename ClosureRight>
    struct pipeline : base<pipeline<ClosureLeft, ClosureRight>> {
        static_assert(is_range_adaptor_closure_object<ClosureLeft>::value, "Left is not adaptor closure");
        static_assert(is_range_adaptor_closure_object<ClosureRight>::value, "Right is not adaptor closure");
        
        template <typename Ty1, typename Ty2>
        constexpr explicit pipeline(Ty1 &&v1, Ty2 &&v2) noexcept(std::is_nothrow_constructible<ClosureLeft, Ty1 &&>::value &&
                                                               std::is_nothrow_constructible<ClosureRight, Ty2 &&>::value) :
            left(utility::forward<Ty1>(v1)), right(utility::forward<Ty2>(v2)) {
        }

        template <typename Ty, typename = decltype(utility::declval<ClosureRight &>()(
                                   utility::declval<ClosureLeft &>()(utility::declval<Ty &&>())))>
        constexpr auto operator()(Ty &&v) & {
            return right(left(utility::forward<Ty>(v)));
        }

        template <typename Ty, typename = decltype(utility::declval<const ClosureRight &>()(
                                  utility::declval<const ClosureLeft &>()(utility::declval<Ty &&>())))>
        constexpr auto operator()(Ty &&v) const & {
            return right(left(utility::forward<Ty>(v)));
        }

        template <typename Ty, typename = decltype(utility::declval<ClosureRight &&>()(
                                   utility::declval<ClosureLeft &&>()(utility::declval<Ty &&>())))>
        constexpr auto operator()(Ty &&v) && {
            return utility::move(right)(utility::move(left)(utility::forward<Ty>(v)));
        }

        template <typename Ty, typename = decltype(utility::declval<const ClosureRight &&>()(
                                  utility::declval<const ClosureLeft &&>()(utility::declval<Ty &&>())))>
        constexpr auto operator()(Ty &&v) const && {
            return utility::move(right)(utility::move(left)(utility::forward<Ty>(v)));
        }

        ClosureLeft left;
        ClosureRight right;
    };

    template <typename A, typename B>
    pipeline(A, B) -> pipeline<A, B>;
}

namespace rainy::collections::views::implements {
#if RAINY_HAS_CXX20
    template <typename L, typename R, enable_if_range_adaptor_closure<L> = 0, enable_if_range_adaptor_closure<R> = 0,
              typename = std::enable_if_t<std::is_constructible<typename std::remove_cv<L>::type, L>::value &&
                                          std::is_constructible<typename std::remove_cv<R>::type, R>::value>>
    constexpr auto operator|(L &&l, R &&r) {
        return pipeline{utility::forward<L>(l), utility::forward<R>(r)};
    }

    template <typename L, typename R, enable_if_range_adaptor_closure<R> = 0,
              typename = decltype(utility::declval<R &&>()(utility::declval<L &&>()))>
    constexpr auto operator|(L &&l, R &&r) {
        return utility::forward<R>(r)(utility::forward<L>(l));
    }
#endif
}

#endif