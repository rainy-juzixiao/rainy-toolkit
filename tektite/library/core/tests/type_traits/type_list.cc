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
#include <tuple>
#include <rainy/core/type_traits/type_list.hpp>

using namespace rainy::type_traits::other_trans;

using list_a = type_list<int, double>;
using list_b = type_list<char, float>;

template <typename... Types>
struct pair_template {};

template <typename Ty>
struct pointer_to {
    using type = Ty *;
};

TEST_CASE("type_list construction and access", "[type_traits][type_list]") {
    STATIC_REQUIRE(type_list_size_v<list_a> == 2);
    STATIC_REQUIRE(type_list_size_v<type_list<>> == 0);

    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<type_at_t<0, list_a>, int>);
    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<type_at_t<1, list_a>, double>);
    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<type_list_front<list_b>::type, char>);
}

TEST_CASE("type_list concatenation and insertion", "[type_traits][type_list]") {
    using combined = type_list_concat<list_a, list_b>;
    STATIC_REQUIRE(type_list_size_v<combined> == 4);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<2, combined>, char>);

    using pushed_front = type_list_push_front_t<long, list_a>;
    STATIC_REQUIRE(type_list_size_v<pushed_front> == 3);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, pushed_front>, long>);

    using pushed_back = type_list_push_back<long, list_a>::type;
    STATIC_REQUIRE(type_list_size_v<pushed_back> == 3);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<2, pushed_back>, long>);

    using pushed_all = type_list_push_back_all<list_a, char, float>::type;
    STATIC_REQUIRE(type_list_size_v<pushed_all> == 4);
    using pushed_front_all = type_list_push_front_all<list_a, char, float>::type;
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, pushed_front_all>, char>);
}

TEST_CASE("type_list removal and slicing", "[type_traits][type_list]") {
    using popped_front = type_list_pop_front<list_a>::type;
    STATIC_REQUIRE(type_list_size_v<popped_front> == 1);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, popped_front>, double>);

    using popped_back = type_list_pop_back<list_a>::type;
    STATIC_REQUIRE(type_list_size_v<popped_back> == 1);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, popped_back>, int>);

    using tail = sub_type_list<1, list_a>::type;
    STATIC_REQUIRE(type_list_size_v<tail> == 1);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, tail>, double>);

    using empty_tail = sub_type_list<5, list_a>::type;
    STATIC_REQUIRE(type_list_size_v<empty_tail> == 0);

    STATIC_REQUIRE(type_list_size_v<type_list_pop_front<type_list<>>::type> == 0);
    STATIC_REQUIRE(type_list_size_v<type_list_pop_back<type_list<>>::type> == 0);
}

TEST_CASE("type_list queries", "[type_traits][type_list]") {
    STATIC_REQUIRE(count_type_v<int, type_list<int, int, double>> == 2);
    STATIC_REQUIRE(count_type_v<char, list_a> == 0);

    STATIC_REQUIRE(type_list_contains<int, list_a>::value);
    STATIC_REQUIRE_FALSE(type_list_contains<char, list_a>::value);

    STATIC_REQUIRE(is_type_in_list<int, type_list<double, int, char>>::value);
    STATIC_REQUIRE_FALSE(is_type_in_list<long, type_list<double, int, char>>::value);

    STATIC_REQUIRE(type_find_unique<double, list_a>::value == 1);
    constexpr std::size_t not_found = type_find_unique<char, list_a>::value;
    STATIC_REQUIRE(not_found == static_cast<std::size_t>(-1));

    using uniqued = unique_type_list_t<type_list<int, double, int, double, char>>;
    STATIC_REQUIRE(type_list_size_v<uniqued> == 3);
    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<type_at_t<2, uniqued>, char>);
}

TEST_CASE("type_list selection and transformation", "[type_traits][type_list]") {
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<select_type<1, list_a>::type, int>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<select_type<5, list_a>::type, double>);

    using quoted = type_list_quote<pair_template>;
    using invoked = type_list_invoke<quoted, int, double>;
    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<invoked, pair_template<int, double>>);

    using transformed = type_list_transform<type_list_quote<pointer_to>, list_a>;
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, transformed>, pointer_to<int>>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<1, transformed>, pointer_to<double>>);

    using joined = type_list_join<type_list<list_a, list_b>>;
    STATIC_REQUIRE(type_list_size_v<joined> == 4);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, joined>, int>);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<3, joined>, float>);
}

TEST_CASE("type_list tuple-like conversion", "[type_traits][type_list]") {
    using tuple = std::tuple<int, double>;
    using as_list_result = as_list<tuple>;
    STATIC_REQUIRE(type_list_size_v<as_list_result> == 2);

    using converted = tuple_like_to_type_list<tuple>::type;
    STATIC_REQUIRE(type_list_size_v<converted> == 2);
    STATIC_REQUIRE(rainy::type_traits::type_relations::is_same_v<type_at_t<0, converted>, int>);

    using restored = type_list_to_tuple_like<list_a, pair_template>::type;
    STATIC_REQUIRE(
        rainy::type_traits::type_relations::is_same_v<restored, pair_template<int, double>>);
}
