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
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/type_traits/meta_types.hpp>

using namespace rainy::type_traits::extras::meta_types;

struct container_like {
    using value_type = int;
    using size_type = std::size_t;
    using difference_type = long;
    using reference = int &;
    using const_reference = const int &;
    using pointer = int *;
    using const_pointer = const int *;
    using iterator = int *;
    using const_iterator = const int *;
    using reverse_iterator = int *;
    using const_reverse_iterator = const int *;
    using allocator_type = std::allocator<int>;
};

struct map_like {
    using key_type = int;
    using mapped_type = double;
};

struct char_traits_like {
    using char_type = char;
};

struct plain {};

TEST_CASE("container member type detection", "[type_traits][meta_types]") {
    STATIC_REQUIRE(has_value_type<container_like>::value);
    STATIC_REQUIRE(has_size_type<container_like>::value);
    STATIC_REQUIRE(has_difference_type<container_like>::value);
    STATIC_REQUIRE(has_reference<container_like>::value);
    STATIC_REQUIRE(has_const_reference<container_like>::value);
    STATIC_REQUIRE(has_pointer<container_like>::value);
    STATIC_REQUIRE(has_const_pointer<container_like>::value);
    STATIC_REQUIRE(has_iterator_type<container_like>::value);
    STATIC_REQUIRE(has_const_iterator_type<container_like>::value);
    STATIC_REQUIRE(has_reverse_iterator<container_like>::value);
    STATIC_REQUIRE(has_const_reverse_iterator<container_like>::value);
    STATIC_REQUIRE(has_allocator_type<container_like>::value);

    STATIC_REQUIRE_FALSE(has_value_type<plain>::value);
    STATIC_REQUIRE_FALSE(has_size_type<plain>::value);
    STATIC_REQUIRE_FALSE(has_allocator_type<plain>::value);
}

TEST_CASE("associative and character member type detection", "[type_traits][meta_types]") {
    STATIC_REQUIRE(has_key_type<map_like>::value);
    STATIC_REQUIRE(has_mapped_type<map_like>::value);
    STATIC_REQUIRE_FALSE(has_key_type<container_like>::value);

    STATIC_REQUIRE(has_char_type<char_traits_like>::value);
    STATIC_REQUIRE_FALSE(has_char_type<plain>::value);
}
