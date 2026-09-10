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
#include <rainy/foundation/poly/poly.hpp>

#include <string>

namespace fpoly = rainy::foundation::poly;

RAINY_DEF_MEM_DISPATCH(name_dispatch, get_name)
RAINY_DEF_MEM_DISPATCH(value_dispatch, get_value)

using name_only = fpoly::facade_builder::add_convention<name_dispatch, std::string()>::build;
using name_and_value =
    fpoly::facade_builder::add_convention<name_dispatch, std::string()>::add_convention<value_dispatch, int()>::build;
using cloneable_name = fpoly::facade_builder::add_convention<name_dispatch, std::string()>::support_clone::build;

namespace testing {
    struct widget {
        std::string get_name() {
            return "widget";
        }
        int get_value() {
            return 10;
        }
    };

    struct gadget {
        std::string get_name() {
            return "gadget";
        }
        int get_value() {
            return 20;
        }
    };

    struct unique_thing {
        unique_thing() = default;
        unique_thing(const unique_thing &) = delete;
        unique_thing(unique_thing &&) = default;
        std::string get_name() {
            return "unique";
        }
    };
} // namespace testing

TEST_CASE("viewbox dispatches through conventions", "[foundation][poly]") {
    fpoly::viewbox<name_and_value> poly;
    REQUIRE(poly.empty());

    testing::widget widget;
    poly.reset(&widget);
    REQUIRE_FALSE(poly.empty());
    REQUIRE(poly->get_name() == "widget");
    REQUIRE(poly->get_value() == 10);

    poly.reset();
    REQUIRE(poly.empty());
}

TEST_CASE("box owns implementations with small-object optimization", "[foundation][poly]") {
    fpoly::box<name_and_value> box(testing::widget{});
    REQUIRE_FALSE(box.empty());
    REQUIRE(box->get_name() == "widget");
    REQUIRE(box->get_value() == 10);

    SECTION("move transfers ownership") {
        fpoly::box<name_and_value> moved(std::move(box));
        REQUIRE(moved->get_name() == "widget");
        REQUIRE(box.empty());
    }
    SECTION("re-emplacing replaces the implementation") {
        box.emplace<testing::gadget>();
        REQUIRE(box->get_name() == "gadget");
        REQUIRE(box->get_value() == 20);
    }
    SECTION("reset empties the box") {
        box.reset();
        REQUIRE(box.empty());
        REQUIRE(box.target_as_void_ptr() == nullptr);
    }
}

TEST_CASE("large types fall back to heap storage", "[foundation][poly]") {
    struct large_widget {
        std::string get_name() {
            return "large";
        }
        char payload[128]{};
    };
    static_assert(sizeof(large_widget) > 64);

    fpoly::box<name_only, 64> box(large_widget{});
    REQUIRE_FALSE(box.empty());
    REQUIRE(box->get_name() == "large");

    fpoly::box<name_only, 64> moved(std::move(box));
    REQUIRE(moved->get_name() == "large");
}

TEST_CASE("cloneable facades can clone boxes", "[foundation][poly]") {
    fpoly::box<cloneable_name> box(testing::widget{});
    REQUIRE(box.cloneable);

    auto cloned = box.clone();
    REQUIRE_FALSE(cloned.empty());
    REQUIRE(cloned->get_name() == "widget");
    REQUIRE(box->get_name() == "widget");

    SECTION("clone is independent") {
        box.reset();
        REQUIRE(cloned->get_name() == "widget");
    }
}

TEST_CASE("non-cloneable facades do not expose clone", "[foundation][poly]") {
    fpoly::box<name_only> box(testing::unique_thing{});
    REQUIRE_FALSE(box.empty());
    REQUIRE_FALSE(box.cloneable);
    STATIC_REQUIRE_FALSE(std::is_constructible_v<decltype(box), const decltype(box) &>);
}
