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
#include <rainy/foundation/factory/auto_register.hpp>
#include <rainy/foundation/factory/pool_factory.hpp>
#include <rainy/foundation/factory/polymorphic_factory.hpp>

#include <unordered_map>

namespace testing {
    template <typename Key, typename Mapped, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>,
              typename Alloc = std::allocator<std::pair<const Key, Mapped>>>
    using unordered_map_storage = std::unordered_map<Key, Mapped, Hash, KeyEqual, Alloc>;

    struct pooled_product {
        int value = 0;

        void reset() {
            value = 0;
        }
    };

    struct bare_product {
        int value = 0;
    };

    struct shape {
        virtual ~shape() = default;
        virtual int kind() const = 0;
    };

    struct circle final : shape {
        int kind() const override {
            return 1;
        }
    };

    struct square final : shape {
        int kind() const override {
            return 2;
        }
    };
} // namespace testing

template <>
struct std::hash<rainy::core::text::hashed_string> {
    std::size_t operator()(const rainy::core::text::hashed_string &key) const noexcept {
        return key.hash_code();
    }
};

namespace fact = rainy::foundation::fact;

using pooled_factory_t = fact::pool_factory<testing::pooled_product, testing::unordered_map_storage>;
using bare_pool_factory_t = fact::pool_factory<testing::bare_product, testing::unordered_map_storage>;
using shape_factory_t = fact::polymorphic_factory<testing::shape, testing::unordered_map_storage>;
using auto_shape_factory_t = fact::polymorphic_factory<testing::shape, testing::unordered_map_storage>;

TEST_CASE("pool_factory reuses recycled instances", "[foundation][factory][pool]") {
    pooled_factory_t factory;
    factory.emplace("counter", [] { return new testing::pooled_product{10}; });

    SECTION("acquire falls back to creation when the pool is empty") {
        REQUIRE(factory.pooled_size("counter") == 0);
        auto lease = factory.acquire("counter");
        REQUIRE(lease);
        REQUIRE(lease->value == 10);
        REQUIRE(factory.pooled_size("counter") == 0);
    }
    SECTION("release returns the instance to the pool and it is reused") {
        testing::pooled_product *raw = nullptr;
        {
            auto lease = factory.acquire("counter");
            raw = lease.get();
        }
        REQUIRE(factory.pooled_size("counter") == 1);
        auto again = factory.acquire("counter");
        REQUIRE(again.get() == raw);
        REQUIRE(factory.pooled_size("counter") == 0);
    }
    SECTION("reset is invoked when reusing a pooled instance") {
        {
            auto lease = factory.acquire("counter");
            lease->value = 99;
        }
        auto reused = factory.acquire("counter");
        REQUIRE(reused->value == 0);
    }
    SECTION("acquire on unknown key yields an empty lease") {
        auto missing = factory.acquire("nope");
        REQUIRE_FALSE(missing);
    }
    SECTION("shrink destroys idle instances") {
        {
            auto lease = factory.acquire("counter");
        }
        REQUIRE(factory.pooled_size("counter") == 1);
        factory.shrink("counter");
        REQUIRE(factory.pooled_size("counter") == 0);
        auto fresh = factory.acquire("counter");
        REQUIRE(fresh);
        REQUIRE(fresh->value == 10);
    }
    SECTION("contains reflects registration only") {
        REQUIRE(factory.contains("counter"));
        REQUIRE_FALSE(factory.contains("other"));
    }
}

TEST_CASE("pool_factory works with types lacking reset", "[foundation][factory][pool]") {
    bare_pool_factory_t factory;
    factory.emplace("bare", [] { return new testing::bare_product{5}; });

    testing::bare_product *raw = nullptr;
    {
        auto lease = factory.acquire("bare");
        raw = lease.get();
        lease->value = 77;
    }
    REQUIRE(factory.pooled_size("bare") == 1);
    auto reused = factory.acquire("bare");
    REQUIRE(reused.get() == raw);
    REQUIRE(reused->value == 77);
}

TEST_CASE("polymorphic_factory creates derived implementations", "[foundation][factory][polymorphic]") {
    shape_factory_t factory;
    factory.register_type<testing::circle>("circle");
    factory.emplace("square", []() -> testing::shape * { return new testing::square(); });

    REQUIRE(factory.size() == 2);
    SECTION("create returns the projected base pointer") {
        auto circle = factory.create("circle");
        REQUIRE(circle);
        REQUIRE(circle->kind() == 1);
        auto square = factory.create("square");
        REQUIRE(square);
        REQUIRE(square->kind() == 2);
        REQUIRE_FALSE(factory.create("triangle"));
    }
    SECTION("keys lists every registered implementation") {
        auto all_keys = factory.keys();
        REQUIRE(all_keys.size() == 2);
        bool has_circle = false;
        bool has_square = false;
        for (const auto &key : all_keys) {
            if (key == rainy::core::text::hashed_string{"circle"}) {
                has_circle = true;
            }
            if (key == rainy::core::text::hashed_string{"square"}) {
                has_square = true;
            }
        }
        REQUIRE(has_circle);
        REQUIRE(has_square);
    }
    SECTION("create_all instantiates every implementation") {
        auto products = factory.create_all();
        REQUIRE(products.size() == 2);
        int kinds = 0;
        for (const auto &product : products) {
            kinds += product->kind();
        }
        REQUIRE(kinds == 3);
    }
    SECTION("erase and contains") {
        factory.erase("circle");
        REQUIRE_FALSE(factory.contains("circle"));
        REQUIRE(factory.size() == 1);
    }
}

TEST_CASE("RAINY_FACTORY_REGISTER registers at static initialization", "[foundation][factory][auto-register]") {
    auto &factory = fact::auto_registrar<auto_shape_factory_t>::instance();
    REQUIRE(factory.contains("auto_circle"));
    auto product = factory.create("auto_circle");
    REQUIRE(product);
    REQUIRE(product->kind() == 1);
    REQUIRE_FALSE(factory.create("missing"));
}

namespace {
    RAINY_FACTORY_REGISTER(auto_shape_factory_t, "auto_circle", []() -> testing::shape * { return new testing::circle(); });
}

TEST_CASE("registration token via macro compiles in anonymous namespace", "[foundation][factory][auto-register]") {
    auto &factory = fact::auto_registrar<auto_shape_factory_t>::instance();
    REQUIRE(factory.contains("auto_circle"));
}
