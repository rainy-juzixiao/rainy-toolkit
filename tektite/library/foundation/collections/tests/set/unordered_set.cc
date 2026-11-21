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
#include <rainy/foundation/collections/unordered_set.hpp>
#include <string>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

using SetType = rainy::collections::unordered_set<int>;

TEST_CASE("unordered_set basic operations", "[unordered_set]") {
    SECTION("an empty unordered_set") {
        SetType set;

        SECTION("size should be 0 and empty() should return true") {
            REQUIRE(set.empty());
            REQUIRE(set.size() == 0);
        }

        SECTION("inserting a single element") {
            auto result = set.insert(1);

            SECTION("insertion should succeed") {
                REQUIRE(result.second == true);
                REQUIRE(set.size() == 1);
                REQUIRE(set.contains(1));
            }
        }

        SECTION("inserting multiple elements") {
            set.insert(1);
            set.insert(2);
            set.insert(3);

            SECTION("all elements should be accessible") {
                REQUIRE(set.size() == 3);
                REQUIRE(set.contains(1));
                REQUIRE(set.contains(2));
                REQUIRE(set.contains(3));
            }
        }
    }

    SECTION("an unordered_set with existing elements") {
        SetType set = {1, 2, 3};

        SECTION("inserting a duplicate key") {
            auto result = set.insert(2);

            SECTION("insertion should fail") {
                REQUIRE(result.second == false);
                REQUIRE(set.size() == 3);
            }
        }

        SECTION("using emplace to insert") {
            auto result = set.emplace(4);

            SECTION("element should be emplaced successfully") {
                REQUIRE(result.second == true);
                REQUIRE(set.contains(4));
                REQUIRE(set.size() == 4);
            }
        }

        SECTION("erasing an element by key") {
            size_t erased = set.erase(2);

            SECTION("element should be removed") {
                REQUIRE(erased == 1);
                REQUIRE(set.size() == 2);
                REQUIRE(!set.contains(2));
            }
        }

        SECTION("erasing a non-existing element") {
            size_t erased = set.erase(99);

            SECTION("no element should be removed") {
                REQUIRE(erased == 0);
                REQUIRE(set.size() == 3);
            }
        }

        SECTION("clearing the set") {
            set.clear();

            SECTION("set should be empty") {
                REQUIRE(set.empty());
                REQUIRE(set.size() == 0);
            }
        }
    }
}

TEST_CASE("unordered_set lookup operations", "[unordered_set]") {
    SECTION("an unordered_set with string keys") {
        rainy::collections::unordered_set<std::string> set = {"apple", "banana", "cherry"};

        SECTION("using find() to search for existing key") {
            auto it = set.find("banana");

            SECTION("iterator should point to the element") {
                REQUIRE(it != set.end());
                REQUIRE(*it == "banana");
            }
        }

        SECTION("using find() to search for non-existing key") {
            auto it = set.find("orange");

            SECTION("iterator should be end()") {
                REQUIRE(it == set.end());
            }
        }

        SECTION("using count() to check existence") {
            SECTION("should return 1 for existing keys and 0 for non-existing") {
                REQUIRE(set.count("apple") == 1);
                REQUIRE(set.count("grape") == 0);
            }
        }

        SECTION("using contains() to check existence") {
            SECTION("should return correct boolean values") {
                REQUIRE(set.contains("cherry") == true);
                REQUIRE(set.contains("mango") == false);
            }
        }
    }
}

TEST_CASE("unordered_set iteration", "[unordered_set]") {
    SECTION("an unordered_set with elements") {
        SetType set = {1, 2, 3, 4};

        SECTION("iterating through all elements") {
            int count = 0;
            for (const auto &key : set) {
                count++;
            }

            SECTION("all elements should be visited exactly once") {
                REQUIRE(count == 4);
            }
        }

        SECTION("using begin() and end() iterators") {
            int count = 0;
            for (auto it = set.begin(); it != set.end(); ++it) {
                count++;
            }

            SECTION("should iterate through all elements") {
                REQUIRE(count == 4);
            }
        }
    }
}

TEST_CASE("unordered_set capacity operations", "[unordered_set]") {
    SECTION("an empty unordered_set") {
        rainy::collections::unordered_set<int> set;

        SECTION("reserving space for elements") {
            set.reserve(100);

            SECTION("bucket count should accommodate reserved size") {
                REQUIRE(set.bucket_count() >= 100);
            }
        }

        SECTION("inserting elements and checking load factor") {
            for (int i = 0; i < 50; ++i) {
                set.insert(i);
            }

            SECTION("load factor should be within valid range") {
                const float load = set.load_factor();
                const float max_load = set.max_load_factor();
                REQUIRE(load <= max_load);
                REQUIRE(load > 0.0f);
            }
        }
    }
}

TEST_CASE("unordered_set copy and move operations", "[unordered_set]") {
    SECTION("an unordered_set with elements") {
        SetType original = {1, 2, 3};

        SECTION("copy constructing a new set") {
            SetType copy(original);

            SECTION("copy should have identical content") {
                REQUIRE(copy.size() == original.size());
                REQUIRE(copy == original);
            }

            SECTION("modifying the copy") {
                copy.insert(4);

                SECTION("original should remain unchanged") {
                    REQUIRE(original.size() == 3);
                    REQUIRE(copy.size() == 4);
                }
            }
        }

        SECTION("copy assigning to another set") {
            SetType copy;
            copy = original;

            SECTION("copy should have identical content") {
                REQUIRE(copy == original);
            }
        }

        SECTION("move constructing a new set") {
            auto original_size = original.size();
            SetType moved(std::move(original));

            SECTION("moved set should have the content") {
                REQUIRE(moved.size() == original_size);
                REQUIRE(moved.contains(1));
                REQUIRE(moved.contains(2));
                REQUIRE(moved.contains(3));
            }
        }
    }
}

TEST_CASE("unordered_set with custom types", "[unordered_set]") {
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

    SECTION("an unordered_set with custom key type") {
        rainy::collections::unordered_set<Point, PointHash> set;

        SECTION("inserting elements with custom keys") {
            set.insert({0, 0});
            set.insert({1, 1});
            set.insert({5, 3});

            SECTION("elements should be stored and retrievable") {
                REQUIRE(set.size() == 3);
                REQUIRE(set.contains({0, 0}));
                REQUIRE(set.contains({1, 1}));
                REQUIRE(set.contains({5, 3}));
            }
        }
    }
}

TEST_CASE("unordered_set edge cases", "[unordered_set]") {
    SECTION("an unordered_set") {
        rainy::collections::unordered_set<int> set;

        SECTION("inserting and erasing repeatedly") {
            for (int i = 0; i < 100; ++i) {
                set.insert(i);
            }
            for (int i = 0; i < 50; ++i) {
                set.erase(i);
            }

            SECTION("final size should be correct") {
                REQUIRE(set.size() == 50);
                REQUIRE(!set.contains(25));
                REQUIRE(set.contains(75));
            }
        }

        SECTION("swapping two sets") {
            rainy::collections::unordered_set<int> set1 = {1, 2};
            rainy::collections::unordered_set<int> set2 = {3, 4, 5};
            set1.swap(set2);

            SECTION("contents should be swapped") {
                REQUIRE(set1.size() == 3);
                REQUIRE(set2.size() == 2);
                REQUIRE(set1.contains(3));
                REQUIRE(set2.contains(1));
            }
        }
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)