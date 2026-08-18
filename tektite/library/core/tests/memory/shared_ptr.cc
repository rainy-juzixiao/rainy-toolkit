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
#include <rainy/core/memory/shared_ptr.hpp>

#include <catch2/catch_test_macros.hpp>

namespace fm = rainy::core::memory;

namespace {
    struct lifetime_probe {
        explicit lifetime_probe(int &destroyed) : destroyed(&destroyed) {}
        ~lifetime_probe() { ++*destroyed; }

        int *destroyed;
    };

    struct tracked_deleter {
        void operator()(int *ptr) const {
            ++*calls;
            delete ptr;
        }

        int *calls;
    };

    struct self_aware : fm::enable_shared_from_this<self_aware> {
        int value{42};
    };
}

TEST_CASE("shared_ptr shares ownership and destroys its object once", "[shared_ptr]") {
    int destroyed = 0;
    fm::shared_ptr<lifetime_probe> first(new lifetime_probe(destroyed));
    {
        fm::shared_ptr<lifetime_probe> second(first);
        REQUIRE(first.use_count() == 2);
        REQUIRE(second.use_count() == 2);
    }
    REQUIRE(first.use_count() == 1);
    first.reset();
    REQUIRE(destroyed == 1);
}

TEST_CASE("shared_ptr supports custom deleters and reset", "[shared_ptr]") {
    int calls = 0;
    fm::shared_ptr<int> ptr(new int(7), tracked_deleter{&calls});
    REQUIRE(*ptr == 7);
    ptr.reset();
    REQUIRE(calls == 1);
    REQUIRE_FALSE(ptr);
}

TEST_CASE("shared_ptr transfers ownership from nebula_ptr", "[shared_ptr]") {
    fm::nebula_ptr<int> unique(new int(12));
    fm::shared_ptr<int> shared(std::move(unique));
    REQUIRE_FALSE(unique);
    REQUIRE(*shared == 12);
    REQUIRE(shared.use_count() == 1);
}

TEST_CASE("weak_ptr observes expiry and lock does not resurrect an object", "[shared_ptr][weak_ptr]") {
    fm::weak_ptr<int> weak;
    {
        fm::shared_ptr<int> shared(new int(9));
        weak = shared;
        REQUIRE_FALSE(weak.expired());
        const auto locked = weak.lock();
        REQUIRE(locked);
        REQUIRE(*locked == 9);
    }
    REQUIRE(weak.expired());
    REQUIRE_FALSE(weak.lock());
}

TEST_CASE("make_shared supports scalars and arrays", "[shared_ptr][make_shared]") {
    const auto scalar = fm::make_shared<int>(21);
    REQUIRE(*scalar == 21);

    const auto initialized = fm::make_shared<int[]>(3, 5);
    REQUIRE(initialized[0] == 5);
    REQUIRE(initialized[2] == 5);

    const auto zero_initialized = fm::make_shared<int[2]>();
    REQUIRE(zero_initialized[0] == 0);
    REQUIRE(zero_initialized[1] == 0);
}

TEST_CASE("enable_shared_from_this reuses the existing control block", "[shared_ptr][enable_shared_from_this]") {
    const auto owner = fm::make_shared<self_aware>();
    const auto self = owner->shared_from_this();
    REQUIRE(self.get() == owner.get());
    REQUIRE(owner.use_count() == 2);
    REQUIRE(self->value == 42);
}

TEST_CASE("shared_ptr supports aliasing ownership", "[shared_ptr]") {
    struct pair_value { int first{3}; int second{4}; };
    const auto owner = fm::make_shared<pair_value>();
    fm::shared_ptr<int> alias(owner, &owner->second);
    REQUIRE(alias.get() == &owner->second);
    REQUIRE(*alias == 4);
    REQUIRE(owner.use_count() == 2);
}
