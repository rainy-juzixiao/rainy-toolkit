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
#include <rainy/core/type_traits/meta_methods.hpp>

using namespace rainy::type_traits::extras::meta_method;

struct rich_container {
    using value_type = int;

    int *begin();
    int *end();
    const int *cbegin() const;
    const int *cend() const;
    int *rbegin();
    int *rend();
    std::size_t size() const;
    bool empty() const;
    void clear();
    void push_back(int);
    void pop_back();
    int &front();
    int &back();
    int *data();
    const int *data() const;
    std::size_t max_size() const;
};

struct arithmetic {
    arithmetic operator+(const arithmetic &) const;
    arithmetic operator-(const arithmetic &) const;
    arithmetic operator*(const arithmetic &) const;
    arithmetic operator/(const arithmetic &) const;
    arithmetic &operator+=(const arithmetic &);
    arithmetic &operator-=(const arithmetic &);
    bool operator==(const arithmetic &) const;
    bool operator!=(const arithmetic &) const;
    bool operator<(const arithmetic &) const;
    arithmetic &operator++();
    arithmetic operator++(int);
    arithmetic &operator--();
};

struct callable {
    using size_type = std::size_t;

    int operator()(size_type);
    int &operator*();
    callable *operator->();
    int &operator[](std::size_t);
};

struct plain {};

TEST_CASE("container method detection", "[type_traits][meta_method]") {
    STATIC_REQUIRE(has_begin_v<rich_container>);
    STATIC_REQUIRE(has_end_v<rich_container>);
    STATIC_REQUIRE(has_cbegin_v<rich_container>);
    STATIC_REQUIRE(has_cend_v<rich_container>);
    STATIC_REQUIRE(has_rbegin_v<rich_container>);
    STATIC_REQUIRE(has_rend_v<rich_container>);
    STATIC_REQUIRE(has_size_v<rich_container>);
    STATIC_REQUIRE(has_empty_v<rich_container>);
    STATIC_REQUIRE(has_clear_v<rich_container>);
    STATIC_REQUIRE(has_push_back_v<rich_container>);
    STATIC_REQUIRE(has_pop_back_v<rich_container>);
    STATIC_REQUIRE(has_front_v<rich_container>);
    STATIC_REQUIRE(has_back_v<rich_container>);
    STATIC_REQUIRE(has_data_v<rich_container>);
    STATIC_REQUIRE(has_max_size_v<rich_container>);

    STATIC_REQUIRE_FALSE(has_size_v<plain>);
    STATIC_REQUIRE_FALSE(has_begin_v<plain>);
    STATIC_REQUIRE_FALSE(has_push_back_v<plain>);
    STATIC_REQUIRE(has_iterator_v<rich_container>);
    STATIC_REQUIRE_FALSE(has_iterator_v<plain>);
}

TEST_CASE("operator detection", "[type_traits][meta_method]") {
    STATIC_REQUIRE(has_operator_add_v<arithmetic>);
    STATIC_REQUIRE(has_operator_sub_v<arithmetic>);
    STATIC_REQUIRE(has_operator_mul_v<arithmetic>);
    STATIC_REQUIRE(has_operator_div_v<arithmetic>);
    STATIC_REQUIRE(has_operator_plus_equal_v<arithmetic>);
    STATIC_REQUIRE(has_operator_sub_equal_v<arithmetic>);
    STATIC_REQUIRE(has_operator_eq_v<arithmetic>);
    STATIC_REQUIRE(has_operator_neq_v<arithmetic>);
    STATIC_REQUIRE(has_operator_lt_v<arithmetic>);
    STATIC_REQUIRE(has_operator_preinc_v<arithmetic>);
    STATIC_REQUIRE(has_operator_postinc_v<arithmetic>);
    STATIC_REQUIRE(has_operator_predec_v<arithmetic>);

    STATIC_REQUIRE(has_operator_call_v<callable>);
    STATIC_REQUIRE(has_operator_deref_v<callable>);
    STATIC_REQUIRE(has_operator_arrow_v<callable>);
    STATIC_REQUIRE(has_operator_index_v<callable>);

    STATIC_REQUIRE_FALSE(has_operator_add_v<plain>);
    STATIC_REQUIRE_FALSE(has_operator_call_v<plain>);
    STATIC_REQUIRE_FALSE(has_operator_index_v<arithmetic>);
}
