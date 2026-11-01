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
#include <rainy/core/annotations/dont.hpp>

namespace {
    struct copyable {
        int value = 0;
    };

    struct movable_only {
        movable_only() = default;
        movable_only(const movable_only &) = delete;
        movable_only &operator=(const movable_only &) = delete;
        movable_only(movable_only &&) noexcept = default;
        movable_only &operator=(movable_only &&) noexcept = default;
    };

    struct payload {
        int value = 0;

        payload() = default;
        explicit payload(int v) : value{v} {
        }
    };

    struct only_movable_payload {
        int value = 0;

        only_movable_payload() = default;
        explicit only_movable_payload(int v) : value{v} {
        }
        only_movable_payload(const only_movable_payload &) = delete;
        only_movable_payload &operator=(const only_movable_payload &) = delete;
        only_movable_payload(only_movable_payload &&) noexcept = default;
        only_movable_payload &operator=(only_movable_payload &&) noexcept = default;
    };
}

using namespace rainy::annotations::dont;
using rainy::type_traits::properties::is_copy_assignable_v;
using rainy::type_traits::properties::is_copy_constructible_v;
using rainy::type_traits::properties::is_move_assignable_v;
using rainy::type_traits::properties::is_move_constructible_v;
using rainy::type_traits::type_relations::is_base_of_v;

TEST_CASE("dont::non_copyable", "[dont]") {
    SECTION("inherits from the wrapped type") {
        STATIC_REQUIRE(is_base_of_v<payload, non_copyable<payload>>);
    }

    SECTION("copy operations are deleted") {
        STATIC_REQUIRE_FALSE(is_copy_constructible_v<non_copyable<payload>>);
        STATIC_REQUIRE_FALSE(is_copy_assignable_v<non_copyable<payload>>);
    }

    SECTION("move operations are preserved") {
        STATIC_REQUIRE(is_move_constructible_v<non_copyable<payload>>);
        STATIC_REQUIRE(is_move_assignable_v<non_copyable<payload>>);
    }

    SECTION("values can still be constructed and read") {
        non_copyable<payload> wrapped{42};
        REQUIRE(wrapped.value == 42);

        non_copyable<payload> moved{rainy::utility::move(wrapped)};
        REQUIRE(moved.value == 42);
    }

    SECTION("non_copyable_if depends on the constraints") {
        STATIC_REQUIRE_FALSE(is_copy_constructible_v<non_copyable_if<payload, rainy::type_traits::helper::true_type>>);
        STATIC_REQUIRE(is_copy_constructible_v<non_copyable_if<payload, rainy::type_traits::helper::false_type>>);
        STATIC_REQUIRE(is_copy_constructible_v<non_copyable_if<payload, rainy::type_traits::helper::false_type, rainy::type_traits::helper::true_type>>);
    }
}

TEST_CASE("dont::no_copy_construct", "[dont]") {
    SECTION("inherits from the wrapped type") {
        STATIC_REQUIRE(is_base_of_v<payload, no_copy_construct<payload>>);
    }

    SECTION("copy construction is deleted") {
        STATIC_REQUIRE_FALSE(is_copy_constructible_v<no_copy_construct<payload>>);
    }

    SECTION("copy assignment is preserved") {
        STATIC_REQUIRE(is_copy_assignable_v<no_copy_construct<payload>>);
    }

    SECTION("move operations are preserved") {
        STATIC_REQUIRE(is_move_constructible_v<no_copy_construct<payload>>);
        STATIC_REQUIRE(is_move_assignable_v<no_copy_construct<payload>>);
    }

    SECTION("move-only payloads stay usable") {
        STATIC_REQUIRE(is_move_constructible_v<no_copy_construct<only_movable_payload>>);
        no_copy_construct<only_movable_payload> wrapped{7};
        REQUIRE(wrapped.value == 7);
    }

    SECTION("no_copy_construct_if depends on the constraints") {
        STATIC_REQUIRE_FALSE(is_copy_constructible_v<no_copy_construct_if<payload, rainy::type_traits::helper::true_type>>);
        STATIC_REQUIRE(is_copy_constructible_v<no_copy_construct_if<payload, rainy::type_traits::helper::false_type>>);
    }
}

TEST_CASE("dont wrappers keep copyable bases intact", "[dont]") {
    STATIC_REQUIRE(is_copy_constructible_v<copyable>);
    STATIC_REQUIRE(is_copy_assignable_v<copyable>);
    STATIC_REQUIRE_FALSE(is_copy_constructible_v<non_copyable<copyable>>);
    STATIC_REQUIRE_FALSE(is_copy_constructible_v<no_copy_construct<copyable>>);
}
