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
#include <rainy/core/type_traits/value_list.hpp>

using namespace rainy::type_traits::other_trans;

using int_list = value_list<0, 1, 2, 3>;

TEST_CASE("value_list basics", "[type_traits][value_list]") {
    STATIC_REQUIRE(int_list::size == 4);
    STATIC_REQUIRE(value_list<>::size == 0);
    STATIC_REQUIRE(value_at<0, int_list>::value == 0);
    STATIC_REQUIRE(value_at<3, int_list>::value == 3);
    STATIC_REQUIRE(value_list_size_v<int_list> == 4);
}

TEST_CASE("value_list insertion", "[type_traits][value_list]") {
    SECTION("push single value") {
        using pushed_front = value_list_push_front<-1, int_list>::type;
        using pushed_back = value_list_push_back<4, int_list>::type;
        STATIC_REQUIRE(pushed_front::size == 5);
        STATIC_REQUIRE(value_at<0, pushed_front>::value == -1);
        STATIC_REQUIRE(pushed_back::size == 5);
        STATIC_REQUIRE(value_at<4, pushed_back>::value == 4);
    }
    SECTION("push multiple values") {
        using pushed_back_all = value_list_push_back_all<int_list, 4, 5>::type;
        using pushed_front_all = value_list_push_front_all<int_list, -2, -1>::type;
        STATIC_REQUIRE(pushed_back_all::size == 6);
        STATIC_REQUIRE(value_at<4, pushed_back_all>::value == 4);
        STATIC_REQUIRE(pushed_front_all::size == 6);
        STATIC_REQUIRE(value_at<0, pushed_front_all>::value == -2);
    }
}

TEST_CASE("value_list queries and slicing", "[type_traits][value_list]") {
    STATIC_REQUIRE(count_value_v<2, int_list> == 1);
    STATIC_REQUIRE(count_value_v<42, int_list> == 0);

    using tail = sub_value_list<2, int_list>::type;
    STATIC_REQUIRE(tail::size == 2);
    STATIC_REQUIRE(value_at<0, tail>::value == 2);
    STATIC_REQUIRE(value_at<1, tail>::value == 3);

    using empty_tail = sub_value_list<10, int_list>::type;
    STATIC_REQUIRE(empty_tail::size == 0);
}
