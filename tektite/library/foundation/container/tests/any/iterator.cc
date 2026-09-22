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
#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/any.hpp>
#include <rainy/core/algorithm/sorting.hpp>

#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace rainy::foundation::container;

using rainy::core::exceptions::logic::logic_error;
using rainy::core::exceptions::runtime::bad_cast;
using rainy::core::exceptions::runtime::nullpointer_exception;

TEST_CASE("any - iterator - random access on vector", "[any]") {
    any vec = std::vector<int>{10, 20, 30, 40, 50};

    auto begin = vec.begin();
    auto end = vec.end();

    CHECK(begin[0].as<int>() == 10);
    CHECK(begin[2].as<int>() == 30);
    CHECK((*(begin + 3)).as<int>() == 40);
    CHECK((*(3 + begin)).as<int>() == 40);
    CHECK((*(end - 2)).as<int>() == 40);
    CHECK((end - begin) == 5);

    auto iter = begin;
    iter += 4;
    CHECK((*iter).as<int>() == 50);
    iter -= 4;
    CHECK((*iter).as<int>() == 10);

    CHECK(begin < end);
    CHECK(end > begin);
    CHECK(begin <= begin);
    CHECK(begin >= begin);
}

TEST_CASE("any - iterator - operator arrow on vector elements", "[any]") {
    any vec = std::vector<std::string>{"hello", "world"};

    auto iter = vec.begin();
    CHECK(iter->as<std::string>() == "hello");
    ++iter;
    CHECK(iter->as<std::string>() == "world");
}

TEST_CASE("any - iterator - empty and has_value on bound iterator", "[any]") {
    any vec = std::vector<int>{1, 2, 3};

    auto iter = vec.begin();
    CHECK(iter.empty() == false);
    CHECK(iter.has_value() == true);
}

TEST_CASE("any - iterator - category reflects the underlying iterator", "[any]") {
    any vec = std::vector<int>{1, 2, 3};
    any lst = std::list<int>{1, 2, 3};
    any umap = std::unordered_map<std::string, int>{{"a", 1}};

    CHECK(vec.begin().category() >= any_iterator_category::random_access_iterator);
    CHECK(lst.begin().category() == any_iterator_category::bidirectional_iterator);
    CHECK(umap.begin().category() == any_iterator_category::forward_iterator);
}

TEST_CASE("any - iterator - backward movement on bidirectional container", "[any]") {
    any lst = std::list<int>{1, 2, 3};

    auto iter = lst.end();
    --iter;
    CHECK((*iter).as<int>() == 3);
    iter--;
    CHECK((*iter).as<int>() == 2);
}

TEST_CASE("any - iterator - unsupported operations on bidirectional container", "[any]") {
    any lst = std::list<int>{1, 2, 3};

    CHECK_THROWS_AS(lst.begin() + 1, logic_error);
    CHECK_THROWS_AS(lst.begin() - 1, logic_error);
    CHECK_THROWS_AS(lst.end() - lst.begin(), logic_error);
}

TEST_CASE("any - iterator - backward movement on forward container throws", "[any]") {
    any flist = std::forward_list<int>{1, 2, 3};

    auto iter = flist.end();
    CHECK_THROWS_AS(--iter, logic_error);
}

TEST_CASE("any - iterator - unbound iterator reports empty", "[any]") {
    any value = 42;

    auto iter = value.begin();
    CHECK(iter.empty() == true);
    CHECK(iter.has_value() == false);
    CHECK(iter == value.end());
}

TEST_CASE("any - iterator - unbound iterator throws on use", "[any]") {
    any value = 42;

    CHECK_THROWS_AS(++value.begin(), nullpointer_exception);
    CHECK_THROWS_AS(value.begin()++, nullpointer_exception);
    CHECK_THROWS_AS(--value.begin(), nullpointer_exception);
    CHECK_THROWS_AS(*value.begin(), nullpointer_exception);
    CHECK_THROWS_AS(value.begin()->as<int>(), nullpointer_exception);
    CHECK_THROWS_AS(value.begin()[0], nullpointer_exception);
    CHECK_THROWS_AS(value.begin() + 1, nullpointer_exception);
    CHECK_THROWS_AS(value.begin() - 1, nullpointer_exception);
    CHECK_THROWS_AS(value.begin().category(), nullpointer_exception);
    CHECK_THROWS_AS(value.begin().target_iterator<int *>(), nullpointer_exception);
}

TEST_CASE("any - iterator - comparison of two unbound iterators", "[any]") {
    any value = 42;

    auto left = value.begin();
    auto right = value.end();

    CHECK((left == right) == true);
    CHECK((left != right) == false);
    CHECK((left - right) == 0);
    CHECK((left < right) == false);
    CHECK((left > right) == false);
    CHECK((left <= right) == true);
    CHECK((left >= right) == true);
}

TEST_CASE("any - iterator - ordering against a bound iterator throws", "[any]") {
    any value = 42;
    any vec = std::vector<int>{1, 2, 3};

    auto unbound = value.begin();
    auto bound = vec.begin();

    CHECK_THROWS_AS(unbound < bound, nullpointer_exception);
    CHECK_THROWS_AS(bound < unbound, nullpointer_exception);
    CHECK_THROWS_AS(unbound - bound, nullpointer_exception);
}

TEST_CASE("any - iterator - range loop over non-container makes no iteration", "[any]") {
    any value = 42;

    int count = 0;
    for (auto iter = value.begin(); iter != value.end(); ++iter) {
        ++count;
    }
    CHECK(count == 0);
}

TEST_CASE("any - iterator - target_iterator unwraps the concrete iterator", "[any]") {
    any vec = std::vector<int>{1, 2, 3, 4};

    auto iter = vec.begin();
    const std::vector<int>::iterator &raw = iter.target_iterator<std::vector<int>::iterator>();
    CHECK(*raw == 1);

    ++iter;
    CHECK(*iter.target_iterator<std::vector<int>::iterator>() == 2);
}

TEST_CASE("any - iterator - target_iterator with mismatched type throws", "[any]") {
    any vec = std::vector<int>{1, 2, 3};

    auto iter = vec.begin();
    CHECK_THROWS_AS(iter.target_iterator<std::vector<double>::iterator>(), bad_cast);
}

TEST_CASE("any - iterator - copy and move preserve position", "[any]") {
    any vec = std::vector<int>{1, 2, 3};

    auto iter = vec.begin();
    ++iter;

    auto copied = iter;
    CHECK((*copied).as<int>() == 2);

    auto moved = rainy::utility::move(copied);
    CHECK((*moved).as<int>() == 2);

    auto assigned = vec.begin();
    assigned = iter;
    CHECK((*assigned).as<int>() == 2);

    auto move_assigned = vec.begin();
    move_assigned = rainy::utility::move(assigned);
    CHECK((*move_assigned).as<int>() == 2);
}

TEST_CASE("any - iterator - copy of unbound iterator stays unbound", "[any]") {
    any value = 42;

    auto iter = value.begin();
    auto copied = iter;
    CHECK(copied.empty() == true);

    auto moved = rainy::utility::move(copied);
    CHECK(moved.empty() == true);
}

TEST_CASE("any - iterator - swap two bound iterators", "[any]") {
    any vec = std::vector<int>{1, 2, 3};

    auto left = vec.begin();
    auto right = vec.end();

    swap(left, right);
    CHECK((*right).as<int>() == 1);
    CHECK((left - right) == 3);
}

TEST_CASE("any - iterator - map key and value", "[any]") {
    any map = std::map<std::string, int>{{"a", 1}, {"b", 2}, {"c", 3}};

    auto iter = map.begin();
    CHECK(iter.key().as<const std::string>() == "a");
    CHECK(iter.value().as<int>() == 1);

    ++iter;
    CHECK(iter.key().as<const std::string>() == "b");
    CHECK(iter.value().as<int>() == 2);
}

TEST_CASE("any - iterator - map value is writable", "[any]") {
    any map = std::map<std::string, int>{{"a", 1}, {"b", 2}};

    auto iter = map.begin();
    iter.value() = 100;
    CHECK(iter.value().as<int>() == 100);

    iter.value() = 200;
    CHECK(iter.value().as<int>() == 200);
}

TEST_CASE("any - iterator - set key and value yield the element", "[any]") {
    any set = std::set<int>{10, 20, 30};

    auto iter = set.begin();
    CHECK(iter.key().as<const int>() == 10);
    CHECK(iter.value().as<const int>() == 10);

    ++iter;
    CHECK(iter.key().as<const int>() == 20);
}

TEST_CASE("any - iterator - key and value on sequence container throws", "[any]") {
    any vec = std::vector<int>{1, 2, 3};

    CHECK_THROWS_AS(vec.begin().key(), logic_error);
    CHECK_THROWS_AS(vec.begin().value(), logic_error);
}

TEST_CASE("any - iterator - multimap exposes duplicate keys", "[any]") {
    any mmap = std::multimap<std::string, int>{{"a", 1}, {"a", 2}};

    auto iter = mmap.begin();
    CHECK(iter.key().as<const std::string>() == "a");
    CHECK(iter.value().as<int>() == 1);

    ++iter;
    CHECK(iter.key().as<const std::string>() == "a");
    CHECK(iter.value().as<int>() == 2);
}

TEST_CASE("any - iterator - unordered_map key and value", "[any]") {
    any umap = std::unordered_map<std::string, int>{{"only", 7}};

    auto iter = umap.begin();
    CHECK(iter.key().as<const std::string>() == "only");
    CHECK(iter.value().as<int>() == 7);
}

TEST_CASE("any - iterator - algo sort vector", "[any]") {
    any vec = std::vector<int>{7, 8, 10, 1, 2, 3, 5, 4, 6, 9};

    rainy::algorithm::sort(vec.begin(), vec.end());
    CHECK(rainy::algorithm::is_sorted(vec.begin(), vec.end()));
}
