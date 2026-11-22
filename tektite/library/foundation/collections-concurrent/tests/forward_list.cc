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
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/concurrency/atomic.hpp>
#include <rainy/foundation/collections/concurrency/forward_list.hpp>
#include <rainy/foundation/concurrency/thread.hpp>

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
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{3, 2, 1});
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
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{0, 5, 1});
            }

            list.erase_after(it); // remove 5

            THEN("erasure works") {
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{0, 1});
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
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{3, 2, 1});
            }

            list.remove_if([](int v) { return v % 2 == 1; });

            THEN("remove_if should remove odd numbers") {
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{2});
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
            rainy::core::collections::vector<rainy::foundation::concurrency::thread> threads;
            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back(rainy::foundation::concurrency::thread::policy::auto_join,
                                     [&list, t, num_ops_per_thread]() {
                                         for (int i = 0; i < num_ops_per_thread; ++i) {
                                             list.push_front(t * num_ops_per_thread + i);
                                         }
                                     });
            }
            threads.clear();
            THEN("all elements should be present") {
                REQUIRE(list.size() == num_threads * num_ops_per_thread);
            }
        }

        WHEN("multiple threads pop elements concurrently after pushing") {
            for (int i = 0; i < num_threads * num_ops_per_thread; ++i) {
                list.push_front(i);
            }

            rainy::core::collections::vector<rainy::foundation::concurrency::thread> threads;
            rainy::core::concurrency::atomic<int> pop_count{0};

            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back(rainy::foundation::concurrency::thread::policy::auto_join,
                                     [&list, &pop_count, num_ops_per_thread]() {
                                         for (int i = 0; i < num_ops_per_thread; ++i) {
                                             list.pop_front();
                                             pop_count.fetch_add(1);
                                         }
                                     });
            }
            threads.clear();

            THEN("all elements should be removed") {
                REQUIRE(list.empty());
                REQUIRE(pop_count.load() == num_threads * num_ops_per_thread);
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
            rainy::core::collections::vector<rainy::foundation::concurrency::thread> threads;

            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back(rainy::foundation::concurrency::thread::policy::auto_join,
                                     [&list, t, num_ops_per_thread]() {
                                         for (int i = 0; i < num_ops_per_thread; ++i) {
                                             if (i % 2 == 0) {
                                                 list.push_front(t * num_ops_per_thread + i);
                                             } else {
                                                 list.pop_front();
                                             }
                                         }
                                     });
            }
            threads.clear();

            THEN("the size is between 0 and num_threads * num_ops_per_thread") {
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
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{1, 2, 3});
            }
        }

        WHEN("we resize the list to larger size") {
            list.resize(5, 42);

            THEN("new elements should be added with given value") {
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values.size() == 5);
                REQUIRE(values[3] == 42);
                REQUIRE(values[4] == 42);
            }
        }

        WHEN("we resize the list to smaller size") {
            list.resize(2);

            THEN("elements beyond new size are removed") {
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
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
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{3, 2, 8, 9, 1});
                REQUIRE(other.empty());
            }
        }

        WHEN("we test remove method") {
            list.push_front(2);
            list.push_front(3); // list: 3,2,3,2,1

            list.remove(2);

            THEN("all elements equal to 2 are removed") {
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{3, 3, 1});
            }
        }

        WHEN("we test unique with custom predicate") {
            list.push_front(6);
            list.push_front(4);
            list.push_front(2); // list: 2,4,6,3,2,1

            list.unique([](int a, int b) { return a % 2 == b % 2; });

            THEN("consecutive elements with same parity are merged") {
                rainy::core::collections::vector<int> values;
                for (auto &v: list) {
                    values.push_back(v);
                }
                REQUIRE(values == rainy::core::collections::vector<int>{2, 3, 2, 1});
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
            rainy::foundation::concurrency::thread t1(
                rainy::foundation::concurrency::thread::policy::auto_join, [&]() { list1.merge(list2); });
            rainy::foundation::concurrency::thread t2(
                rainy::foundation::concurrency::thread::policy::auto_join, [&]() { list2.merge(list1); });

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

TEST_CASE("forward_list assign preserves input order", "[forward_list][assign]") {
    forward_list<int> list;
    const int values[] = {1, 2, 3};
    list.assign(values, values + 3);
    auto it = list.begin();
    REQUIRE(*it++ == 1);
    REQUIRE(*it++ == 2);
    REQUIRE(*it++ == 3);
    REQUIRE(it == list.end());

    list.assign({4, 5, 6});
    it = list.begin();
    REQUIRE(*it++ == 4);
    REQUIRE(*it++ == 5);
    REQUIRE(*it++ == 6);
    REQUIRE(it == list.end());

    list.assign(3, 7);
    it = list.begin();
    REQUIRE(*it++ == 7);
    REQUIRE(*it++ == 7);
    REQUIRE(*it++ == 7);
    REQUIRE(it == list.end());
}
