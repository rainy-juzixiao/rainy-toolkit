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
#include <rainy/foundation/factory/fact.hpp>

#include <unordered_map>

namespace testing {
    template <typename Key, typename Mapped, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>,
              typename Alloc = std::allocator<std::pair<const Key, Mapped>>>
    using unordered_map_storage = std::unordered_map<Key, Mapped, Hash, KeyEqual, Alloc>;

    struct product {
        int value;
        explicit product(int v) : value(v) {
        }
    };

    struct extra_maker {
        product *operator()() const {
            return new product(2);
        }
    };
    struct never_maker {
        product *operator()() const {
            return new product(3);
        }
    };
} // namespace testing

namespace fact = rainy::foundation::fact;

using product_factory = fact::unsynchronized_factory<testing::product, testing::unordered_map_storage>;
using sized_product_factory =
    fact::unsynchronized_factory<testing::product, testing::unordered_map_storage, rainy::core::text::hashed_string,
                                 rainy::core::memory::nebula_ptr, testing::product *(int)>;
using synchronized_product_factory = fact::synchronized_factory<testing::product, testing::unordered_map_storage>;

TEST_CASE("unsynchronized_factory registers and creates products", "[foundation][factory]") {
    product_factory factory;
    factory.emplace("alpha", [] { return new testing::product(42); });
    factory.emplace("beta", [] { return new testing::product(7); });

    SECTION("contains and size") {
        REQUIRE(factory.size() == 2);
        REQUIRE(factory.contains("alpha"));
        REQUIRE(factory.contains("beta"));
        REQUIRE_FALSE(factory.contains("gamma"));
    }
    SECTION("make_product creates via the registered creator") {
        auto alpha = factory.make_product("alpha");
        REQUIRE(alpha);
        REQUIRE(alpha->value == 42);

        auto beta = factory.make_product("beta");
        REQUIRE(beta);
        REQUIRE(beta->value == 7);

        auto missing = factory.make_product("gamma");
        REQUIRE_FALSE(missing);
    }
    SECTION("erase removes a creator") {
        factory.erase("alpha");
        REQUIRE(factory.size() == 1);
        REQUIRE_FALSE(factory.contains("alpha"));
    }
    SECTION("rebind_creater replaces or inserts") {
        factory.rebind_creater("alpha", [] { return new testing::product(100); });
        REQUIRE(factory.make_product("alpha")->value == 100);
        factory.rebind_creater("delta", [] { return new testing::product(8); });
        REQUIRE(factory.make_product("delta")->value == 8);
    }
    SECTION("try_emplace does not overwrite existing creators") {
        factory.try_emplace("alpha", [] { return new testing::product(-1); });
        REQUIRE(factory.make_product("alpha")->value == 42);
        factory.try_emplace("epsilon", [] { return new testing::product(5); });
        REQUIRE(factory.make_product("epsilon")->value == 5);
    }
    SECTION("iteration visits every entry") {
        std::size_t count = 0;
        for (auto it = factory.begin(); it != factory.end(); ++it) {
            ++count;
        }
        REQUIRE(count == factory.size());
    }
    SECTION("clear empties the factory") {
        factory.clear();
        REQUIRE(factory.size() == 0);
    }
}

TEST_CASE("unsynchronized_factory forwards creator arguments", "[foundation][factory]") {
    sized_product_factory factory;
    factory.emplace("adder", [](int base) { return new testing::product(base + 1); });

    auto product = factory.make_product("adder", 41);
    REQUIRE(product);
    REQUIRE(product->value == 42);

    auto missing = factory.make_product("nope", 1);
    REQUIRE_FALSE(missing);
}

TEST_CASE("synchronized_factory guards access with the concurrency mutex", "[foundation][factory]") {
    synchronized_product_factory factory;
    factory.emplace("alpha", [] { return new testing::product(1); });

    REQUIRE(factory.contains("alpha"));
    auto product = factory.make_product("alpha");
    REQUIRE(product);
    REQUIRE(product->value == 1);

    SECTION("supports conditional insertion") {
        factory.emplace_if<testing::extra_maker, true>("extra", testing::extra_maker{});
        REQUIRE(factory.contains("extra"));
        REQUIRE(factory.make_product("extra")->value == 2);
        factory.emplace_if<testing::never_maker, false>("never", testing::never_maker{});
        REQUIRE_FALSE(factory.contains("never"));
        factory.try_emplace_if<testing::extra_maker, true>("alpha", testing::extra_maker{});
        REQUIRE(factory.make_product("alpha")->value == 1);
    }
    SECTION("clear empties the factory") {
        factory.clear();
        REQUIRE(factory.size() == 0);
    }
}

TEST_CASE("factory traits introspect factories", "[foundation][factory]") {
    STATIC_REQUIRE(fact::is_factory_v<product_factory>);
    STATIC_REQUIRE(fact::is_factory_v<synchronized_product_factory>);
    STATIC_REQUIRE_FALSE(fact::is_factory_v<int>);
    STATIC_REQUIRE(std::is_same_v<fact::product_t<product_factory>, testing::product>);
    STATIC_REQUIRE(std::is_same_v<fact::creater_t<product_factory>, rainy::functional::delegate<testing::product *()>>);
    STATIC_REQUIRE(std::is_same_v<fact::projection_holder_t<product_factory>, rainy::core::memory::nebula_ptr<testing::product>>);
}
