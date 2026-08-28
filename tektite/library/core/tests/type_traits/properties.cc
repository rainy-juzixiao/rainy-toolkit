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
#include <rainy/core/type_traits/properties.hpp>

using namespace rainy::type_traits::properties;

namespace testing {
    struct empty {};
    struct aggregate {
        int x;
        double y;
    };
    class polymorphic_base {
    public:
        virtual ~polymorphic_base() = default;
        virtual void method();
    };
    class polymorphic_derived : public polymorphic_base {};
    class abstract_base {
    public:
        virtual ~abstract_base() = default;
        virtual void pure() = 0;
    };
    struct with_virtual_destructor {
        virtual ~with_virtual_destructor();
    };
    struct not_copyable {
        not_copyable() = default;
        not_copyable(const not_copyable &) = delete;
        not_copyable &operator=(const not_copyable &) = delete;
    };
    struct nothrow_move_only {
        nothrow_move_only() = default;
        nothrow_move_only(nothrow_move_only &&) noexcept = default;
        nothrow_move_only &operator=(nothrow_move_only &&) noexcept = default;
        nothrow_move_only(const nothrow_move_only &) = delete;
    };
    struct throwing_destructor {
        ~throwing_destructor() noexcept(false);
    };
    struct custom_swappable {
        int value;
    };

    void swap(custom_swappable &left, custom_swappable &right) noexcept {
        int temp = left.value;
        left.value = right.value;
        right.value = temp;
    }
} // namespace testing

TEST_CASE("type properties: cv and sign", "[type_traits][properties]") {
    STATIC_REQUIRE(is_const_v<const int>);
    STATIC_REQUIRE_FALSE(is_const_v<int>);
    STATIC_REQUIRE(is_volatile_v<volatile int>);
    STATIC_REQUIRE_FALSE(is_volatile_v<int>);

    STATIC_REQUIRE(is_signed_v<int>);
    STATIC_REQUIRE(is_signed_v<double>);
    STATIC_REQUIRE_FALSE(is_signed_v<unsigned int>);
    STATIC_REQUIRE(is_unsigned_v<unsigned int>);
    STATIC_REQUIRE_FALSE(is_unsigned_v<int>);
}

TEST_CASE("type properties: class shape", "[type_traits][properties]") {
    STATIC_REQUIRE(is_empty_v<testing::empty>);
    STATIC_REQUIRE_FALSE(is_empty_v<testing::aggregate>);

    STATIC_REQUIRE(is_polymorphic_v<testing::polymorphic_base>);
    STATIC_REQUIRE(is_polymorphic_v<testing::polymorphic_derived>);
    STATIC_REQUIRE_FALSE(is_polymorphic_v<testing::aggregate>);

    STATIC_REQUIRE(is_abstract_v<testing::abstract_base>);
    STATIC_REQUIRE_FALSE(is_abstract_v<testing::polymorphic_base>);

    STATIC_REQUIRE(is_aggregate_v<testing::aggregate>);
    STATIC_REQUIRE_FALSE(is_aggregate_v<testing::polymorphic_base>);

    STATIC_REQUIRE(is_standard_layout_v<testing::aggregate>);
    STATIC_REQUIRE(is_trivially_copyable_v<testing::aggregate>);
    STATIC_REQUIRE(is_pod_v<testing::aggregate>);
}

TEST_CASE("type properties: construction and destruction", "[type_traits][properties]") {
    SECTION("constructible") {
        STATIC_REQUIRE(is_default_constructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_copy_constructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_move_constructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_constructible_v<testing::aggregate, int, double>);
        STATIC_REQUIRE_FALSE(is_constructible_v<testing::aggregate, testing::abstract_base>);

        STATIC_REQUIRE(is_nothrow_constructible_v<testing::aggregate, int, double>);
        STATIC_REQUIRE(is_nothrow_move_constructible_v<testing::nothrow_move_only>);

        STATIC_REQUIRE(is_trivially_constructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_trivially_copy_constructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_trivially_move_constructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_trivially_default_constructible_v<testing::aggregate>);
    }
    SECTION("assignable") {
        STATIC_REQUIRE(is_copy_assignable_v<testing::aggregate>);
        STATIC_REQUIRE(is_move_assignable_v<testing::aggregate>);
        STATIC_REQUIRE(is_assignable_v<testing::aggregate &, const testing::aggregate &>);
        STATIC_REQUIRE_FALSE(is_copy_assignable_v<testing::not_copyable>);

        STATIC_REQUIRE(is_nothrow_copy_assignable_v<testing::aggregate>);
        STATIC_REQUIRE(is_nothrow_move_assignable_v<testing::nothrow_move_only>);
        STATIC_REQUIRE(is_nothrow_assignable_v<testing::nothrow_move_only &, testing::nothrow_move_only &&>);

        STATIC_REQUIRE(is_trivially_copy_assignable_v<testing::aggregate>);
        STATIC_REQUIRE(is_trivially_move_assignable_v<testing::aggregate>);
        STATIC_REQUIRE(is_trivially_assignable_v<testing::aggregate &, const testing::aggregate &>);
    }
    SECTION("destructible") {
        STATIC_REQUIRE(is_destructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_nothrow_destructible_v<testing::aggregate>);
        STATIC_REQUIRE(is_trivially_destructible_v<testing::aggregate>);
        STATIC_REQUIRE_FALSE(is_trivially_destructible_v<testing::with_virtual_destructor>);
        STATIC_REQUIRE_FALSE(is_nothrow_destructible_v<testing::throwing_destructor>);
    }
    SECTION("misc class properties") {
        STATIC_REQUIRE(is_trivial_v<testing::aggregate>);
        STATIC_REQUIRE(has_virtual_destructor_v<testing::with_virtual_destructor>);
        STATIC_REQUIRE(has_virtual_destructor_v<testing::polymorphic_base>);
        STATIC_REQUIRE_FALSE(has_virtual_destructor_v<testing::aggregate>);
    }
}

TEST_CASE("swappability", "[type_traits][properties]") {
    STATIC_REQUIRE(is_swappable_v<testing::aggregate>);
    STATIC_REQUIRE(is_nothrow_swappable_v<testing::aggregate>);
    STATIC_REQUIRE(is_swappable_with_v<testing::custom_swappable &, testing::custom_swappable &>);
    STATIC_REQUIRE(is_nothrow_swappable_with_v<testing::custom_swappable &, testing::custom_swappable &>);
    STATIC_REQUIRE(has_adl_swap_v<testing::custom_swappable>);
}
