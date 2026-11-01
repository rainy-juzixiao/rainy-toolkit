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
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/annotations/aspect.hpp>

namespace {
    template <typename T>
    struct boxed {
        T value;
    };

    template <typename T>
    struct counted {
        T value;
        static inline int instances = 0;
    };

    template <typename T>
    struct always_true : rainy::type_traits::helper::true_type {};

    template <typename T>
    struct always_false : rainy::type_traits::helper::false_type {};

    template <typename T, int N>
    struct sized {
        T value;
    };
}

using namespace rainy::annotations::aspect;

TEST_CASE("aspect decorated", "[aspect]") {
    SECTION("no aspect yields the type itself") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int>, int>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated<int>::type, int>);
    }

    SECTION("single aspect wraps once") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int, with<boxed>>, boxed<int>>);
    }

    SECTION("aspects apply right to left") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int, with<boxed>, with<counted>>,
                                                                    boxed<counted<int>>>);
    }

    SECTION("with_conditional applies only when the predicate holds") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int, with_conditional<boxed, always_true>>,
                                                                    boxed<int>>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int, with_conditional<boxed, always_false>>, int>);
    }

    SECTION("with_param forwards the non-type arguments") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int, with_param<sized, 8>>, sized<int, 8>>);
    }

    SECTION("compose is itself an aspect") {
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<
                       decorated_t<int, compose<with<boxed>, with<counted>>>, boxed<counted<int>>>);
        STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<decorated_t<int, with<boxed>, compose<with<counted>>>,
                                                                    boxed<counted<int>>>);
    }
}
