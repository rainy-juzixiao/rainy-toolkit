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
#include <atomic>
#include <barrier>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <rainy/collections/concurrency/forward_list.hpp>
#include <random>
#include <set>
#include <thread>
#include <vector>

using namespace rainy::collections::concurrency;

SCENARIO("concurrent forward_list basic operations", "[forward_list]") {

    GIVEN("An empty forward_list of ints") {
        forward_list<int> list;
        THEN("it should be empty") {
            REQUIRE(list.empty());
            REQUIRE(list.size() == 0);
        }

        WHEN("we push elements to the front") {
            list.push_front(1);
            list.push_front(2);
            list.push_front(3);

            THEN("size should increase and front() returns the last pushed element") {
                REQUIRE(list.size() == 3);
                REQUIRE(list.front() == 3);
            }

            THEN("iteration should return elements in correct order") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{3, 2, 1});
            }
        }

        WHEN("we emplace elements to the front") {
            list.emplace_front(42);

            THEN("the front element should be correct") {
                REQUIRE(list.front() == 42);
                REQUIRE(list.size() == 1);
            }
        }

        WHEN("we pop elements from the front") {
            list.push_front(10);
            list.push_front(20);
            list.pop_front();

            THEN("the front element and size should update") {
                REQUIRE(list.size() == 1);
                REQUIRE(list.front() == 10);
            }
        }

        WHEN("we use insert_after and erase_after") {
            list.push_front(1);
            list.push_front(0); // list: 0->1

            auto it = list.begin();
            list.insert_after(it, 5); // list: 0->5->1

            THEN("insertion works") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{0, 5, 1});
            }

            list.erase_after(it); // remove 5

            THEN("erasure works") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{0, 1});
            }
        }

        WHEN("we use remove_if and unique") {
            list.push_front(1);
            list.push_front(1);
            list.push_front(2);
            list.push_front(2);
            list.push_front(3); // list: 3,2,2,1,1

            list.unique();

            THEN("unique should remove consecutive duplicates") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{3, 2, 1});
            }

            list.remove_if([](int v) { return v % 2 == 1; });

            THEN("remove_if should remove odd numbers") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{2});
            }
        }
    }
}

SCENARIO("concurrent push_front/pop_front in multiple threads", "[forward_list][concurrency]") {

    GIVEN("A forward_list shared among threads") {
        forward_list<int> list;
        constexpr int num_threads = 8;
        constexpr int num_ops_per_thread = 1000;

        WHEN("multiple threads push elements concurrently") {
            std::vector<std::thread> threads;
            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back([&list, t, num_ops_per_thread]() {
                    for (int i = 0; i < num_ops_per_thread; ++i) {
                        list.push_front(t * num_ops_per_thread + i);
                    }
                });
            }

            for (auto &th: threads)
                th.join();

            THEN("all elements should be present") {
                REQUIRE(list.size() == num_threads * num_ops_per_thread);
            }
        }

        WHEN("multiple threads pop elements concurrently after pushing") {
            for (int i = 0; i < num_threads * num_ops_per_thread; ++i)
                list.push_front(i);

            std::vector<std::thread> threads;
            std::atomic<int> pop_count{0};

            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back([&list, &pop_count, num_ops_per_thread]() {
                    for (int i = 0; i < num_ops_per_thread; ++i) {
                        list.pop_front();
                        pop_count.fetch_add(1);
                    }
                });
            }

            for (auto &th: threads)
                th.join();

            THEN("all elements should be removed") {
                REQUIRE(list.empty());
                REQUIRE(pop_count == num_threads * num_ops_per_thread);
            }
        }
    }
}

SCENARIO("concurrent push and pop mixed operations", "[forward_list][concurrency]") {
    GIVEN("A forward_list shared among threads for mixed operations") {
        forward_list<int> list;
        constexpr int num_threads = 4;
        constexpr int num_ops_per_thread = 500;

        WHEN("threads push and pop concurrently") {
            std::vector<std::thread> threads;

            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back([&list, t, num_ops_per_thread]() {
                    for (int i = 0; i < num_ops_per_thread; ++i) {
                        if (i % 2 == 0)
                            list.push_front(t * num_ops_per_thread + i);
                        else
                            list.pop_front();
                    }
                });
            }

            for (auto &th: threads) {
                th.join();
            }

            THEN("the size is between 0 and num_threads * num_ops_per_thread") {
                // 由于 pop_front 可能比 push_front 多或少，最终 size 可能不为固定值
                REQUIRE(list.size() <= static_cast<std::size_t>(num_threads * num_ops_per_thread));
            }

            THEN("all remaining elements are valid integers") {
                for (auto &v: list) {
                    REQUIRE(v >= 0);
                }
            }
        }
    }
}

SCENARIO("forward_list advanced operations", "[forward_list]") {

    GIVEN("A forward_list with multiple elements") {
        forward_list<int> list;
        list.push_front(1);
        list.push_front(2);
        list.push_front(3); // list: 3,2,1

        WHEN("we reverse the list") {
            list.reverse();

            THEN("elements should be reversed") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{1, 2, 3});
            }
        }

        WHEN("we resize the list to larger size") {
            list.resize(5, 42);

            THEN("new elements should be added with given value") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values.size() == 5);
                REQUIRE(values[3] == 42);
                REQUIRE(values[4] == 42);
            }
        }

        WHEN("we resize the list to smaller size") {
            list.resize(2);

            THEN("elements beyond new size are removed") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values.size() == 2);
            }
        }

        WHEN("we use splice_after with another list") {
            forward_list<int> other;
            other.push_front(9);
            other.push_front(8); // other: 8,9

            auto it = list.begin();
            ++it; // point to second element in list
            list.splice_after(it, other);

            THEN("elements from other list are inserted correctly") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{3, 2, 8, 9, 1});
                REQUIRE(other.empty());
            }
        }

        WHEN("we test remove method") {
            list.push_front(2);
            list.push_front(3); // list: 3,2,3,2,1

            list.remove(2);

            THEN("all elements equal to 2 are removed") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{3, 3, 1});
            }
        }

        WHEN("we test unique with custom predicate") {
            list.push_front(6);
            list.push_front(4);
            list.push_front(2); // list: 2,4,6,3,2,1

            list.unique([](int a, int b) { return a % 2 == b % 2; });

            THEN("consecutive elements with same parity are merged") {
                std::vector<int> values;
                for (auto &v: list)
                    values.push_back(v);
                REQUIRE(values == std::vector<int>{2, 3, 2, 1});
            }
        }
    }
}

SCENARIO("concurrent splice and merge operations", "[forward_list][concurrency]") {

    GIVEN("Two forward_lists shared among threads") {
        forward_list<int> list1, list2;

        for (int i = 0; i < 1000; ++i) {
            list1.push_front(i);
            list2.push_front(i + 1000);
        }

        WHEN("threads splice and merge concurrently") {
            std::thread t1([&]() { list1.merge(list2); });
            std::thread t2([&]() { list2.merge(list1); });

            t1.join();
            t2.join();

            THEN("the total number of elements should be correct") {
                REQUIRE(list1.size() + list2.size() == 2000);
            }
        }
    }
}

SCENARIO("edge cases for forward_list", "[forward_list]") {

    GIVEN("An empty forward_list") {
        forward_list<int> list;
        WHEN("pop_front is called on empty list") {
            list.pop_front();
            THEN("list remains empty") {
                REQUIRE(list.empty());
                REQUIRE(list.size() == 0);
            }
        }
        WHEN("erase_after is called on before_begin") {
            list.erase_after(list.before_begin());

            THEN("list remains empty") {
                REQUIRE(list.empty());
            }
        }
        WHEN("unique is called on empty list") {
            list.unique();
            THEN("list remains empty") {
                REQUIRE(list.empty());
            }
        }
    }
}
