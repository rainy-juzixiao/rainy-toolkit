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
#ifndef RAINY_ANNOTATIONS_ASPECT_HPP
#define RAINY_ANNOTATIONS_ASPECT_HPP
#include  <rainy/core/core.hpp>

namespace rainy::annotations::aspect {
    /**
     * @tparam Ty 要装饰的类
     * @tparam Aspects 要插入的切面，切面是一个模板，它应当是一个已实例化的装饰模板，例如with、with_conditional、compose这类，
     */
    template <typename Ty, typename... Aspects>
    struct decorated;

    template<typename Ty>
    struct decorated<Ty> {
        using type = Ty;
    };

    template<typename Ty, typename First, typename... Rest>
    struct decorated<Ty, First, Rest...> {
        using type = typename First::template apply<
            typename decorated<Ty, Rest...>::type
        >::type;
    };

    template<typename Ty, typename... Aspects>
    using decorated_t = typename decorated<Ty, Aspects...>::type;

    template<template<typename> class Wrapper>
    struct with {
        template<typename T>
        struct apply {
            using type = Wrapper<T>;
        };
    };

    template<template<typename> class Wrapper, template<typename> class Condition>
    struct with_conditional {
        template<typename T>
        struct apply {
            using type = type_traits::other_trans::conditional_t<
                Condition<T>::value,
                Wrapper<T>,
                T
            >;
        };
    };

    template<template<typename, auto... Args> class Wrapper, auto... Args>
    struct with_param {
        template<typename T>
        struct apply {
            using type = Wrapper<T, Args...>;
        };
    };

    template<typename... Aspects>
    struct compose {
        template<typename T>
        struct apply {
            using type = decorated_t<T, Aspects...>;
        };
    };
}

#endif
