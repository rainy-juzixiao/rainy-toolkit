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
#include <rainy/foundation/collections/unordered_map.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

using MapType = rainy::collections::unordered_map<int, std::string>;
using PairType = rainy::container::pair<int, std::string>;

TEST_CASE("unordered_map basic operations", "[unordered_map]") {

    SECTION("an empty unordered_map") {
        MapType map;

        SECTION("checking if it's empty") {
            SECTION("size should be 0 and empty() should return true") {
                REQUIRE(map.empty()); // NOLINT
                REQUIRE(map.size() == 0); // NOLINT
            }
        }

        SECTION("inserting a single element") {
            auto result = map.insert({1, "one"});

            SECTION("insertion should succeed") {
                REQUIRE(result.second == true);
                REQUIRE(map.size() == 1);
                REQUIRE(map[1] == "one");
            }
        }

        SECTION("inserting multiple elements") {
            map.insert({1, "one"});
            map.insert({2, "two"});
            map.insert({3, "three"});

            SECTION("all elements should be accessible") {
                REQUIRE(map.size() == 3);
                REQUIRE(map[1] == "one");
                REQUIRE(map[2] == "two");
                REQUIRE(map[3] == "three");
            }
        }
    }

    SECTION("an unordered_map with existing elements") {
        MapType map = {{1, "one"}, {2, "two"}, {3, "three"}};

        SECTION("inserting a duplicate key") {
            auto result = map.insert({2, "TWO"});

            SECTION("insertion should fail and original value remains") {
                REQUIRE(result.second == false);
                REQUIRE(map[2] == "two");
                REQUIRE(map.size() == 3);
            }
        }

        SECTION("using emplace to insert") {
            auto result = map.emplace(4, "four");

            SECTION("element should be emplaced successfully") {
                REQUIRE(result.second == true);
                REQUIRE(map[4] == "four");
                REQUIRE(map.size() == 4);
            }
        }

        SECTION("using operator[] to modify existing value") {
            map[2] = "TWO";

            SECTION("value should be updated") {
                REQUIRE(map[2] == "TWO");
                REQUIRE(map.size() == 3);
            }
        }

        SECTION("using operator[] with non-existing key") {
            auto value = map[5];
            SECTION("default value should be inserted") {
                REQUIRE(value.empty());
                REQUIRE(map.size() == 4);
                REQUIRE(map.count(5) == 1);
            }
        }

        SECTION("erasing an element by key") {
            size_t erased = map.erase(2);

            SECTION("element should be removed") {
                REQUIRE(erased == 1);
                REQUIRE(map.size() == 2);
                REQUIRE(map.count(2) == 0);
            }
        }

        SECTION("erasing a non-existing element") {
            size_t erased = map.erase(99);

            SECTION("no element should be removed") {
                REQUIRE(erased == 0);
                REQUIRE(map.size() == 3);
            }
        }

        SECTION("clearing the map") {
            map.clear();

            SECTION("map should be empty") {
                REQUIRE(map.empty());
                REQUIRE(map.size() == 0); // NOLINT
            }
        }
    }
}

TEST_CASE("unordered_map lookup operations", "[unordered_map]") {

    SECTION("an unordered_map with string keys") {
        rainy::collections::unordered_map<std::string, int> map = {{"apple", 5}, {"banana", 3}, {"cherry", 8}};

        SECTION("using find() to search for existing key") {
            auto it = map.find("banana");

            SECTION("iterator should point to the element") {
                REQUIRE(it != map.end());
                REQUIRE(it->first == "banana");
                REQUIRE(it->second == 3);
            }
        }

        SECTION("using find() to search for non-existing key") {
            auto it = map.find("orange");

            SECTION("iterator should be end()") {
                REQUIRE(it == map.end());
            }
        }

        SECTION("using count() to check existence") {
            SECTION("should return 1 for existing keys and 0 for non-existing") {
                REQUIRE(map.count("apple") == 1);
                REQUIRE(map.count("grape") == 0);
            }
        }

        SECTION("using contains() to check existence") {
            SECTION("should return correct boolean values") {
                REQUIRE(map.contains("cherry") == true);
                REQUIRE(map.contains("mango") == false);
            }
        }

        SECTION("using at() with existing key") {
            int value = map.at("apple");

            SECTION("should return the correct value") {
                REQUIRE(value == 5);
            }
        }

        SECTION("using at() with non-existing key") {
            SECTION("should throw out_of_range exception") {
                REQUIRE_THROWS_AS(map.at("pear"), rainy::core::exceptions::logic::out_of_range);
            }
        }
    }
}

TEST_CASE("unordered_map iteration", "[unordered_map]") {

    SECTION("an unordered_map with elements") {
        MapType map = {{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};

        SECTION("iterating through all elements") {
            int count = 0;
            MapType found;

            for (const auto &[key, value]: map) {
                count++;
                found[key] = value;
            }

            SECTION("all elements should be visited exactly once") {
                REQUIRE(count == 4);
                REQUIRE(found == map);
            }
        }

        SECTION("using begin() and end() iterators") {
            int count = 0;
            for (auto it = map.begin(); it != map.end(); ++it) {
                count++;
            }

            SECTION("should iterate through all elements") {
                REQUIRE(count == 4);
            }
        }
    }
}

TEST_CASE("unordered_map capacity operations", "[unordered_map]") {

    SECTION("an empty unordered_map") {
        rainy::collections::unordered_map<int, int> map;

        SECTION("reserving space for elements") {
            map.reserve(100);

            SECTION("bucket count should accommodate reserved size") {
                REQUIRE(map.bucket_count() >= 100);
            }
        }

        SECTION("inserting elements and checking load factor") {
            for (int i = 0; i < 50; ++i) {
                map[i] = i * 2;
            }

            SECTION("load factor should be within valid range") {
                const float load = map.load_factor();
                const float max_load = map.max_load_factor();
                REQUIRE(load <= max_load);
                REQUIRE(load > 0.0f);
            }
        }
    }
}

TEST_CASE("unordered_map copy and move operations", "[unordered_map]") {

    SECTION("an unordered_map with elements") {
        MapType original = {{1, "one"}, {2, "two"}, {3, "three"}};

        SECTION("copy constructing a new map") {
            MapType copy(original);

            SECTION("copy should have identical content") {
                REQUIRE(copy.size() == original.size());
                REQUIRE(copy == original);
            }

            SECTION("modifying the copy") {
                copy[4] = "four";

                SECTION("original should remain unchanged") {
                    REQUIRE(original.size() == 3);
                    REQUIRE(copy.size() == 4);
                }
            }
        }

        SECTION("copy assigning to another map") {
            MapType copy;
            copy = original;

            SECTION("copy should have identical content") {
                REQUIRE(copy == original);
            }
        }

        SECTION("move constructing a new map") {
            auto original_size = original.size();
            MapType moved(std::move(original));

            SECTION("moved map should have the content") {
                REQUIRE(moved.size() == original_size);
                REQUIRE(moved[1] == "one");
                REQUIRE(moved[2] == "two");
                REQUIRE(moved[3] == "three");
            }
        }
    }
}

TEST_CASE("unordered_map with custom types", "[unordered_map]") {

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

    SECTION("an unordered_map with custom key type") {
        rainy::collections::unordered_map<Point, std::string, PointHash> map; // NOLINT

        SECTION("inserting elements with custom keys") {
            // NOLINTBEGIN
            map[{0, 0}] = "origin";
            map[{1, 1}] = "diagonal";
            map[{5, 3}] = "point";
            // NOLINTEND
            SECTION("elements should be stored and retrievable") {
                REQUIRE(map.size() == 3);
                REQUIRE(map[{0, 0}] == "origin");
                REQUIRE(map[{1, 1}] == "diagonal");
                REQUIRE(map[{5, 3}] == "point");
            }
        }
    }
}

TEST_CASE("unordered_map edge cases", "[unordered_map]") {
    SECTION("an unordered_map") {
        rainy::collections::unordered_map<int, int> map; // NOLINT
        SECTION("inserting and erasing repeatedly") {
            for (int i = 0; i < 100; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 50; ++i) {
                map.erase(i);
            }
            SECTION("final size should be correct") {
                REQUIRE(map.size() == 50);
                REQUIRE(map.count(25) == 0);
                REQUIRE(map.count(75) == 1);
            }
        }
        SECTION("swapping two maps") {
            rainy::collections::unordered_map<int, int> map1 = {{1, 10}, {2, 20}};
            rainy::collections::unordered_map<int, int> map2 = {{3, 30}, {4, 40}, {5, 50}};
            map1.swap(map2);
            SECTION("contents should be swapped") {
                REQUIRE(map1.size() == 3);
                REQUIRE(map2.size() == 2);
                REQUIRE(map1[3] == 30);
                REQUIRE(map2[1] == 10);
            }
        }
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)