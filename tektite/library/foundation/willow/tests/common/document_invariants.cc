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
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <type_traits>
#include <unordered_set>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/foundation/willow/document.hpp>

using namespace rainy::foundation::willow;

namespace {
    struct memory_resource_guard {
        memory_resource_guard() : previous(get_memory_resource()) {
        }

        ~memory_resource_guard() {
            set_memory_resource(previous);
        }

        std::pmr::memory_resource *previous;
    };

    struct tracking_memory_resource : std::pmr::memory_resource {
        explicit tracking_memory_resource(std::pmr::memory_resource *upstream) : upstream(upstream) {
        }

        std::pmr::memory_resource *upstream;
        std::unordered_set<void *> allocations;
        bool mismatched{false};

    private:
        void *do_allocate(std::size_t bytes, std::size_t alignment) override {
            void *ptr = upstream->allocate(bytes, alignment);
            allocations.insert(ptr);
            return ptr;
        }

        void do_deallocate(void *ptr, std::size_t bytes, std::size_t alignment) override {
            if (allocations.erase(ptr) == 0) {
                mismatched = true;
            }
            upstream->deallocate(ptr, bytes, alignment);
        }

        bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
            return this == &other;
        }
    };
}

TEST_CASE("willow array operator[] rejects indexes that would overflow", "[willow][document]") {
    document value;
    value[static_cast<document::size_type>(3)] = 7;

    REQUIRE_THROWS_AS(value[(std::numeric_limits<document::size_type>::max)()] = 1,
                      rainy::core::exceptions::logic::length_error);
    REQUIRE(value.size() == 4);
    REQUIRE(value[3].as_integer() == 7);
}

TEST_CASE("willow object initializer list rejects non-object shapes", "[willow][document]") {
    REQUIRE_THROWS_AS(document::object({{1, 2}}),
                      rainy::foundation::exceptions::willow::willow_type_error);
}

TEST_CASE("willow const document indexing yields const nodes", "[willow][document]") {
    const document object = document::object({{"key", 1}});
    const document array = document::array({1, 2});

    static_assert(std::is_same_v<decltype(object["key"]), const document &>);
    static_assert(std::is_same_v<decltype(object[static_cast<const char *>("key")]), const document &>);
    static_assert(std::is_same_v<decltype(array[0]), const document &>);

    REQUIRE(object["key"].as_integer() == 1);
    REQUIRE(array[1].as_integer() == 2);
}

TEST_CASE("willow documents release with the resource that allocated them", "[willow][document]") {
    memory_resource_guard guard;
    std::pmr::unsynchronized_pool_resource pool1;
    std::pmr::unsynchronized_pool_resource pool2;
    tracking_memory_resource first{&pool1};
    tracking_memory_resource second{&pool2};

    set_memory_resource(&first);
    {
        document value;
        value["key"] = "value";
        value["array"] = document::array({1, 2, 3});
        value["object"] = document::object({{"nested", 42}});
        set_memory_resource(&second);
    }

    REQUIRE_FALSE(first.mismatched);
    REQUIRE_FALSE(second.mismatched);
}
