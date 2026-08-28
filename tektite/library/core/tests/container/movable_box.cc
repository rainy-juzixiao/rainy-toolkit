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
#include <rainy/core/container/movable_box.hpp>

using rainy::container::movable_box;

TEST_CASE("movable_box basics", "[container][movable_box]") {
    SECTION("default construction") {
        movable_box<int> box;
        REQUIRE(static_cast<bool>(box));
        REQUIRE(*box == 0);

        *box = 5;
        REQUIRE(*box == 5);
    }
    SECTION("in_place construction") {
        movable_box<std::string> box(std::in_place, "rainy");
        REQUIRE(static_cast<bool>(box));
        REQUIRE(*box == "rainy");

        movable_box<std::string> with_size(std::in_place, 3, 'x');
        REQUIRE(*with_size == "xxx");
    }
}

TEST_CASE("movable_box copy and move", "[container][movable_box]") {
    movable_box<std::string> source(std::in_place, "data");
    movable_box<std::string> copied(source);
    REQUIRE(*copied == "data");
    REQUIRE(*source == "data");

    movable_box<std::string> moved(std::move(source));
    REQUIRE(*moved == "data");

    movable_box<std::string> assigned;
    assigned = std::move(moved);
    REQUIRE(*assigned == "data");

    movable_box<std::string> copy_assigned;
    copy_assigned = assigned;
    REQUIRE(*copy_assigned == "data");
    REQUIRE(*assigned == "data");
}

namespace testing {
    struct move_only {
        int value;
        explicit move_only(int v) : value(v) {}
        move_only(move_only &&other) noexcept : value(other.value) {}
        move_only &operator=(move_only &&other) noexcept {
            value = other.value;
            return *this;
        }
        move_only(const move_only &) = delete;
        move_only &operator=(const move_only &) = delete;
    };
} // namespace testing

TEST_CASE("movable_box holds move-only types", "[container][movable_box]") {
    movable_box<testing::move_only> box(std::in_place, 42);
    REQUIRE(static_cast<bool>(box));
    REQUIRE((*box).value == 42);

    movable_box<testing::move_only> moved(std::move(box));
    REQUIRE((*moved).value == 42);
}
