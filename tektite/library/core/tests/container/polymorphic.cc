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
#include <rainy/core/container/polymorphic.hpp>

using rainy::container::polymorphic;

namespace testing {
    struct base {
        virtual ~base() = default;
        virtual int id() const {
            return 0;
        }
    };

    struct derived final : base {
        int id() const override {
            return 1;
        }
    };

    struct other_derived final : base {
        int id() const override {
            return 2;
        }
    };
}

TEST_CASE("polymorphic construction", "[container][polymorphic]") {
    polymorphic<testing::base> value;
    REQUIRE((*value).id() == 0);
    REQUIRE(value->id() == 0);
    REQUIRE_FALSE(value.valueless_after_move());

    polymorphic<testing::derived> with_alloc(std::allocator_arg, std::allocator<testing::derived>{});
    REQUIRE(with_alloc->id() == 1);
}

TEST_CASE("polymorphic virtual dispatch", "[container][polymorphic]") {
    polymorphic<testing::base> value(std::in_place_type<testing::derived>);
    REQUIRE(value->id() == 1);

    value = polymorphic<testing::base>(std::in_place_type<testing::other_derived>);
    REQUIRE(value->id() == 2);
}

TEST_CASE("polymorphic copy and move", "[container][polymorphic]") {
    polymorphic<testing::base> source(std::in_place_type<testing::derived>);
    polymorphic<testing::base> copied(source);
    REQUIRE(copied->id() == 1);
    REQUIRE(source->id() == 1);

    polymorphic<testing::base> moved(std::move(source));
    REQUIRE(moved->id() == 1);
    REQUIRE(source.valueless_after_move());
}

TEST_CASE("polymorphic allocator access and swap", "[container][polymorphic]") {
    polymorphic<testing::base> left(std::in_place_type<testing::derived>);
    polymorphic<testing::base> right(std::in_place_type<testing::other_derived>);

    left.swap(right);
    REQUIRE(left->id() == 2);
    REQUIRE(right->id() == 1);

    using rainy::container::swap;
    swap(left, right);
    REQUIRE(left->id() == 1);
    REQUIRE(right->id() == 2);
}
