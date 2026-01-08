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
#include <catch2/matchers/catch_matchers_all.hpp>
#include <rainy/collections/unordered_map.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

using MapType = rainy::collections::unordered_map<int, std::string>;
using PairType = rainy::utility::pair<int, std::string>;

SCENARIO("std::unordered_map basic operations", "[unordered_map]") {

    GIVEN("an empty unordered_map") {
        MapType map;

        WHEN("checking if it's empty") {
            THEN("size should be 0 and empty() should return true") {
                REQUIRE(map.empty()); // NOLINT
                REQUIRE(map.size() == 0); // NOLINT
            }
        }

        WHEN("inserting a single element") {
            auto result = map.insert({1, "one"});

            THEN("insertion should succeed") {
                REQUIRE(result.second == true);
                REQUIRE(map.size() == 1);
                REQUIRE(map[1] == "one");
            }
        }

        WHEN("inserting multiple elements") {
            map.insert({1, "one"});
            map.insert({2, "two"});
            map.insert({3, "three"});

            THEN("all elements should be accessible") {
                REQUIRE(map.size() == 3);
                REQUIRE(map[1] == "one");
                REQUIRE(map[2] == "two");
                REQUIRE(map[3] == "three");
            }
        }
    }

    GIVEN("an unordered_map with existing elements") {
        MapType map = {{1, "one"}, {2, "two"}, {3, "three"}};

        WHEN("inserting a duplicate key") {
            auto result = map.insert({2, "TWO"});

            THEN("insertion should fail and original value remains") {
                REQUIRE(result.second == false);
                REQUIRE(map[2] == "two");
                REQUIRE(map.size() == 3);
            }
        }

        WHEN("using emplace to insert") {
            auto result = map.emplace(4, "four");

            THEN("element should be emplaced successfully") {
                REQUIRE(result.second == true);
                REQUIRE(map[4] == "four");
                REQUIRE(map.size() == 4);
            }
        }

        WHEN("using operator[] to modify existing value") {
            map[2] = "TWO";

            THEN("value should be updated") {
                REQUIRE(map[2] == "TWO");
                REQUIRE(map.size() == 3);
            }
        }

        WHEN("using operator[] with non-existing key") {
            auto value = map[5];
            THEN("default value should be inserted") {
                REQUIRE(value.empty());
                REQUIRE(map.size() == 4);
                REQUIRE(map.count(5) == 1);
            }
        }

        WHEN("erasing an element by key") {
            size_t erased = map.erase(2);

            THEN("element should be removed") {
                REQUIRE(erased == 1);
                REQUIRE(map.size() == 2);
                REQUIRE(map.count(2) == 0);
            }
        }

        WHEN("erasing a non-existing element") {
            size_t erased = map.erase(99);

            THEN("no element should be removed") {
                REQUIRE(erased == 0);
                REQUIRE(map.size() == 3);
            }
        }

        WHEN("clearing the map") {
            map.clear();

            THEN("map should be empty") {
                REQUIRE(map.empty());
                REQUIRE(map.size() == 0); // NOLINT
            }
        }
    }
}

SCENARIO("std::unordered_map lookup operations", "[unordered_map]") {

    GIVEN("an unordered_map with string keys") {
        std::unordered_map<std::string, int> map = {{"apple", 5}, {"banana", 3}, {"cherry", 8}};

        WHEN("using find() to search for existing key") {
            auto it = map.find("banana");

            THEN("iterator should point to the element") {
                REQUIRE(it != map.end());
                REQUIRE(it->first == "banana");
                REQUIRE(it->second == 3);
            }
        }

        WHEN("using find() to search for non-existing key") {
            auto it = map.find("orange");

            THEN("iterator should be end()") {
                REQUIRE(it == map.end());
            }
        }

        WHEN("using count() to check existence") {
            THEN("should return 1 for existing keys and 0 for non-existing") {
                REQUIRE(map.count("apple") == 1);
                REQUIRE(map.count("grape") == 0);
            }
        }

        WHEN("using contains() to check existence") {
            THEN("should return correct boolean values") {
                REQUIRE(map.contains("cherry") == true);
                REQUIRE(map.contains("mango") == false);
            }
        }

        WHEN("using at() with existing key") {
            int value = map.at("apple");

            THEN("should return the correct value") {
                REQUIRE(value == 5);
            }
        }

        WHEN("using at() with non-existing key") {
            THEN("should throw out_of_range exception") {
                REQUIRE_THROWS_AS(map.at("pear"), std::out_of_range);
            }
        }
    }
}

SCENARIO("std::unordered_map iteration", "[unordered_map]") {

    GIVEN("an unordered_map with elements") {
        MapType map = {{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};

        WHEN("iterating through all elements") {
            int count = 0;
            MapType found;

            for (const auto &[key, value]: map) {
                count++;
                found[key] = value;
            }

            THEN("all elements should be visited exactly once") {
                REQUIRE(count == 4);
                REQUIRE(found == map);
            }
        }

        WHEN("using begin() and end() iterators") {
            int count = 0;
            for (auto it = map.begin(); it != map.end(); ++it) {
                count++;
            }

            THEN("should iterate through all elements") {
                REQUIRE(count == 4);
            }
        }
    }
}

SCENARIO("std::unordered_map capacity operations", "[unordered_map]") {

    GIVEN("an empty unordered_map") {
        std::unordered_map<int, int> map;

        WHEN("reserving space for elements") {
            map.reserve(100);

            THEN("bucket count should accommodate reserved size") {
                REQUIRE(map.bucket_count() >= 100);
            }
        }

        WHEN("inserting elements and checking load factor") {
            for (int i = 0; i < 50; ++i) {
                map[i] = i * 2;
            }

            THEN("load factor should be within valid range") {
                const float load = map.load_factor();
                const float max_load = map.max_load_factor();
                REQUIRE(load <= max_load);
                REQUIRE(load > 0.0f);
            }
        }
    }
}

SCENARIO("std::unordered_map copy and move operations", "[unordered_map]") {

    GIVEN("an unordered_map with elements") {
        MapType original = {{1, "one"}, {2, "two"}, {3, "three"}};

        WHEN("copy constructing a new map") {
            MapType copy(original);

            THEN("copy should have identical content") {
                REQUIRE(copy.size() == original.size());
                REQUIRE(copy == original);
            }

            AND_WHEN("modifying the copy") {
                copy[4] = "four";

                THEN("original should remain unchanged") {
                    REQUIRE(original.size() == 3);
                    REQUIRE(copy.size() == 4);
                }
            }
        }

        WHEN("copy assigning to another map") {
            MapType copy;
            copy = original;

            THEN("copy should have identical content") {
                REQUIRE(copy == original);
            }
        }

        WHEN("move constructing a new map") {
            auto original_size = original.size();
            MapType moved(std::move(original));

            THEN("moved map should have the content") {
                REQUIRE(moved.size() == original_size);
                REQUIRE(moved[1] == "one");
                REQUIRE(moved[2] == "two");
                REQUIRE(moved[3] == "three");
            }
        }
    }
}

SCENARIO("std::unordered_map with custom types", "[unordered_map]") {

    struct Point {
        int x, y;
        bool operator==(const Point &other) const {
            return x == other.x && y == other.y;
        }
    };

    struct PointHash {
        size_t operator()(const Point &p) const {
            return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
        }
    };

    GIVEN("an unordered_map with custom key type") {
        std::unordered_map<Point, std::string, PointHash> map; // NOLINT

        WHEN("inserting elements with custom keys") {
            // NOLINTBEGIN
            map[{0, 0}] = "origin";
            map[{1, 1}] = "diagonal";
            map[{5, 3}] = "point";
            // NOLINTEND
            THEN("elements should be stored and retrievable") {
                REQUIRE(map.size() == 3);
                REQUIRE(map[{0, 0}] == "origin");
                REQUIRE(map[{1, 1}] == "diagonal");
                REQUIRE(map[{5, 3}] == "point");
            }
        }
    }
}

SCENARIO("std::unordered_map edge cases", "[unordered_map]") {
    GIVEN("an unordered_map") {
        std::unordered_map<int, int> map; // NOLINT
        WHEN("inserting and erasing repeatedly") {
            for (int i = 0; i < 100; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 50; ++i) {
                map.erase(i);
            }
            THEN("final size should be correct") {
                REQUIRE(map.size() == 50);
                REQUIRE(map.count(25) == 0);
                REQUIRE(map.count(75) == 1);
            }
        }
        WHEN("swapping two maps") {
            std::unordered_map<int, int> map1 = {{1, 10}, {2, 20}};
            std::unordered_map<int, int> map2 = {{3, 30}, {4, 40}, {5, 50}};
            map1.swap(map2);
            THEN("contents should be swapped") {
                REQUIRE(map1.size() == 3);
                REQUIRE(map2.size() == 2);
                REQUIRE(map1[3] == 30);
                REQUIRE(map2[1] == 10);
            }
        }
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)