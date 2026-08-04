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
#include <rainy/core/poly/basic_poly.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <string>

using namespace rainy;
using namespace rainy::core;
using namespace rainy::core::implements;

TEST_CASE("poly_inspector type traits", "[poly][inspector]") {
    SECTION("default constructible") {
        REQUIRE(std::is_default_constructible_v<poly_inspector>);
        REQUIRE(std::is_trivially_default_constructible_v<poly_inspector>);
    }

    SECTION("operator Type&&") {
        using conv_check = decltype(std::declval<const poly_inspector>().operator int&&());
        REQUIRE((std::is_same_v<conv_check, int &&>));
    }

    SECTION("invoke returns poly_inspector") {
        using invoke_result = decltype(std::declval<const poly_inspector>().template invoke<0>());
        REQUIRE((std::is_same_v<invoke_result, poly_inspector>));

        using invoke_result_mut = decltype(std::declval<poly_inspector>().template invoke<0>());
        REQUIRE((std::is_same_v<invoke_result_mut, poly_inspector>));
    }
}

struct test_concept_draw {
    template <typename Base>
    struct type : Base {
        using Base::invoke;
    };

    template <typename T>
    struct impl;
};

template <>
struct test_concept_draw::impl<test_concept_draw::type<implements::poly_inspector>>
    : type_traits::other_trans::value_list<
        static_cast<poly_inspector (poly_inspector::*)() const>(&poly_inspector::template invoke<0>)
    > {};

TEST_CASE("poly_vtable type traits", "[poly][vtable]") {
    SECTION("vtable_info type definitions") {
        using vtable_info = poly_vtable<test_concept_draw>;
        REQUIRE(vtable_info::is_mono == true);

        using vtable_type = typename vtable_info::type;
        REQUIRE((std::is_pointer_v<vtable_type>));
    }
}

TEST_CASE("basic_poly construction and value semantics", "[poly][basic_poly]") {
    SECTION("default constructor") {
        basic_poly<test_concept_draw> poly;
        REQUIRE(poly.empty() == true);
        REQUIRE(static_cast<bool>(poly) == false);
    }

    SECTION("nullptr constructor") {
        basic_poly<test_concept_draw> poly(nullptr);
        REQUIRE(poly.empty() == true);
    }

    SECTION("nullptr assignment") {
        basic_poly<test_concept_draw> poly;
        poly = nullptr;
        REQUIRE(poly.empty() == true);
    }

    SECTION("reset") {
        basic_poly<test_concept_draw> poly;
        poly.reset();
        REQUIRE(poly.empty() == true);
    }

    SECTION("move") {
        basic_poly<test_concept_draw> src;
        basic_poly<test_concept_draw> dst(std::move(src));
        REQUIRE(dst.empty() == true);
    }

    SECTION("move assign") {
        basic_poly<test_concept_draw> src;
        basic_poly<test_concept_draw> dst;
        dst = std::move(src);
        REQUIRE(dst.empty() == true);
    }

    SECTION("copy") {
        basic_poly<test_concept_draw> src;
        basic_poly<test_concept_draw> dst(src);
        REQUIRE(dst.empty() == true);
    }

    SECTION("copy assign") {
        basic_poly<test_concept_draw> src;
        basic_poly<test_concept_draw> dst;
        dst = src;
        REQUIRE(dst.empty() == true);
    }

    SECTION("arrow operator") {
        basic_poly<test_concept_draw> poly;
        using abstract_type = basic_poly<test_concept_draw>::abstract_type;
        abstract_type *ptr = poly.operator->();
        REQUIRE(ptr != nullptr);
    }

    SECTION("const arrow operator") {
        const basic_poly<test_concept_draw> poly;
        using abstract_type = basic_poly<test_concept_draw>::abstract_type;
        const abstract_type *ptr = poly.operator->();
        REQUIRE(ptr != nullptr);
    }

    SECTION("target_as_void_ptr") {
        basic_poly<test_concept_draw> poly;
        REQUIRE(poly.target_as_void_ptr() == nullptr);
    }
}

struct test_concept_draw2 {
    template <typename Base>
    struct type : Base {
        using Base::invoke;
    };

    template <typename T>
    struct impl;
};

template <>
struct test_concept_draw2::impl<test_concept_draw2::type<implements::poly_inspector>>
    : type_traits::other_trans::value_list<
        static_cast<poly_inspector (poly_inspector::*)() const>(&poly_inspector::template invoke<0>)
    > {};

TEST_CASE("basic_poly type traits", "[poly][type_traits]") {
    SECTION("member types") {
        using poly_type = basic_poly<test_concept_draw>;

        REQUIRE((std::is_same_v<typename poly_type::abstract_type,
                                test_concept_draw::template type<poly_base<poly_type>>>));

        REQUIRE((std::is_default_constructible_v<poly_type>));
        REQUIRE((std::is_move_constructible_v<poly_type>));
        REQUIRE((std::is_move_assignable_v<poly_type>));
        REQUIRE((std::is_copy_constructible_v<poly_type>));
        REQUIRE((std::is_copy_assignable_v<poly_type>));
    }

    SECTION("different concepts are distinct types") {
        basic_poly<test_concept_draw> poly1;
        basic_poly<test_concept_draw2> poly2;
        REQUIRE_FALSE((std::is_same_v<decltype(poly1), decltype(poly2)>));
    }
}
