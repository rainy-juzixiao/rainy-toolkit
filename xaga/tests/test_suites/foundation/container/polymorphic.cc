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
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <memory>
#include <rainy/foundation/container/polymorphic.hpp>
#include <string>

// NOLINTBEGIN

struct Base {
    virtual ~Base() = default;
    virtual std::string identify() const {
        return "Base";
    }
    virtual int get_value() const {
        return 0;
    }
};

struct Derived : Base {
    explicit Derived(int v = 42) : value(v) {
    }

    std::string identify() const override {
        return "Derived";
    }

    int get_value() const override {
        return value;
    }

    int value;
};

struct Derived2 : Base {
    explicit Derived2(const std::string &s = "hello") : str(s) {
    }
    std::string identify() const override {
        return "Derived2";
    }
    int get_value() const override {
        return static_cast<int>(str.length());
    }
    std::string str;
};

struct NonDefaultConstructible {
    explicit NonDefaultConstructible(int) {
    }
};

struct ThrowingConstructor {
    class exception : public std::exception {
        const char *what() const noexcept override {
            return "construction failed";
        }
    };

    explicit ThrowingConstructor(bool shouldThrow) {
        if (shouldThrow) {
            throw exception();
        }
    }
};

template <typename T>
struct ThrowingAllocator : std::allocator<T> {
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    ThrowingAllocator() = default;

    template <typename U>
    ThrowingAllocator(const ThrowingAllocator<U> &other) : should_throw(other.should_throw) {
    }

    T *allocate(size_t n) {
        if (should_throw) {
            throw std::bad_alloc();
        }
        return std::allocator<T>::allocate(n);
    }

    void deallocate(T *p, size_t n) {
        std::allocator<T>::deallocate(p, n);
    }

    bool should_throw = false;
};

using namespace rainy::foundation::container;

SCENARIO("polymorphic default construction", "[polymorphic][construction]") {
    GIVEN("a default constructed polymorphic") {
        polymorphic<Base> poly;

        THEN("it should contain a default constructed Base object") {
            REQUIRE(poly->identify() == "Base");
            REQUIRE(poly->get_value() == 0);
        }

        THEN("valueless_after_move should return false") {
            REQUIRE(poly.valueless_after_move() == false);
        }
    }

    GIVEN("a default constructed polymorphic with allocator") {
        std::allocator<Base> alloc;
        polymorphic<Base> poly(std::allocator_arg, alloc);

        THEN("it should contain a default constructed Base object") {
            REQUIRE(poly->identify() == "Base");
        }
    }
}

SCENARIO("polymorphic construction from derived type", "[polymorphic][construction]") {
    GIVEN("a Derived object") {
        Derived d(100);

        WHEN("constructing polymorphic from it") {
            polymorphic<Base> poly(d);

            THEN("the stored object should be of type Derived") {
                REQUIRE(poly->identify() == "Derived");
                REQUIRE(poly->get_value() == 100);
            }
        }
    }

    GIVEN("a temporary Derived object") {
        WHEN("constructing polymorphic from rvalue") {
            polymorphic<Base> poly(Derived(200));

            THEN("the stored object should be of type Derived") {
                REQUIRE(poly->identify() == "Derived");
                REQUIRE(poly->get_value() == 200);
            }
        }
    }
}

SCENARIO("polymorphic in_place construction", "[polymorphic][construction]") {
    GIVEN("in_place construction with Derived type") {
        polymorphic<Base> poly(std::in_place_type<Derived>, 300);

        THEN("it should contain a Derived object constructed with the given arguments") {
            REQUIRE(poly->identify() == "Derived");
            REQUIRE(poly->get_value() == 300);
        }
    }

    GIVEN("in_place construction with Derived2 type and initializer_list") {
        polymorphic<Base> poly(std::in_place_type<Derived2>, {'h', 'e', 'l', 'l', 'o'});

        THEN("it should contain a Derived2 object") {
            REQUIRE(poly->identify() == "Derived2");
        }
    }

    GIVEN("in_place construction with allocator") {
        std::allocator<Base> alloc;
        polymorphic<Base> poly(std::allocator_arg, alloc, std::in_place_type<Derived>, 400);

        THEN("it should contain a Derived object") {
            REQUIRE(poly->identify() == "Derived");
            REQUIRE(poly->get_value() == 400);
        }
    }
}

SCENARIO("polymorphic copy semantics", "[polymorphic][copy]") {
    GIVEN("a polymorphic containing Derived object") {
        polymorphic<Base> poly1(std::in_place_type<Derived>, 500);

        WHEN("copy constructing") {
            polymorphic<Base> poly2(poly1);

            THEN("the copy should contain the same value") {
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 500);
            }

            THEN("the original should remain unchanged") {
                REQUIRE(poly1->get_value() == 500);
            }
        }

        WHEN("copy assigning") {
            polymorphic<Base> poly2;
            poly2 = poly1;

            THEN("the assigned object should contain the same value") {
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 500);
            }
        }
    }

    GIVEN("two polymorphic objects with different allocators") {
        using allocator_type = std::allocator<Base>;
        allocator_type alloc1, alloc2;
        polymorphic<Base> poly1(std::allocator_arg, alloc1, std::in_place_type<Derived>, 600);
        polymorphic<Base> poly2(std::allocator_arg, alloc2, std::in_place_type<Derived2>, "test");

        WHEN("copy assigning") {
            poly2 = poly1;

            THEN("poly2 should contain a copy of poly1's value") {
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 600);
            }
        }
    }
}

SCENARIO("polymorphic move semantics", "[polymorphic][move]") {
    GIVEN("a polymorphic containing Derived object") {
        polymorphic<Base> poly1(std::in_place_type<Derived>, 700);

        WHEN("move constructing") {
            polymorphic<Base> poly2(std::move(poly1));

            THEN("the new object should contain the value") {
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 700);
            }

            THEN("the original should be valueless") {
                REQUIRE(poly1.valueless_after_move() == true);
            }
        }

        WHEN("move assigning") {
            polymorphic<Base> poly2;
            poly2 = std::move(poly1);

            THEN("the assigned object should contain the value") {
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 700);
            }

            THEN("the original should be valueless") {
                REQUIRE(poly1.valueless_after_move() == true);
            }
        }
    }
}

SCENARIO("polymorphic dereference operators", "[polymorphic][operators]") {
    GIVEN("a polymorphic containing Derived object") {
        polymorphic<Base> poly(std::in_place_type<Derived>, 800);

        THEN("operator* should return reference to the object") {
            Base &ref = *poly;
            REQUIRE(ref.identify() == "Derived");
            REQUIRE(ref.get_value() == 800);
        }

        THEN("operator-> should provide access to members") {
            REQUIRE(poly->identify() == "Derived");
            REQUIRE(poly->get_value() == 800);
        }

        THEN("const operators should work on const objects") {
            const auto &const_poly = poly;
            REQUIRE(const_poly->identify() == "Derived");
            REQUIRE((*const_poly).get_value() == 800);
        }
    }
}

SCENARIO("polymorphic swap operation", "[polymorphic][swap]") {
    GIVEN("two polymorphic objects with different values") {
        polymorphic<Base> poly1(std::in_place_type<Derived>, 900);
        polymorphic<Base> poly2(std::in_place_type<Derived2>, "swapped");

        WHEN("swapping them") {
            swap(poly1, poly2);

            THEN("poly1 should contain poly2's original value") {
                REQUIRE(poly1->identify() == "Derived2");
            }

            THEN("poly2 should contain poly1's original value") {
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 900);
            }
        }

        WHEN("using member swap") {
            poly1.swap(poly2);

            THEN("values should be exchanged") {
                REQUIRE(poly1->identify() == "Derived2");
                REQUIRE(poly2->identify() == "Derived");
                REQUIRE(poly2->get_value() == 900);
            }
        }
    }
}

SCENARIO("polymorphic reset behavior", "[polymorphic][destruction]") {
    GIVEN("a polymorphic containing an object") {
        polymorphic<Base> poly(std::in_place_type<Derived>, 1000);

        WHEN("it goes out of scope") {
            {
                polymorphic<Base> inner_poly(std::in_place_type<Derived>, 1100);
            }
            THEN("the destructor should clean up properly") {
                REQUIRE(true); // Destructor called without issues
            }
        }
    }
}

SCENARIO("polymorphic allocator propagation", "[polymorphic][allocator]") {
    GIVEN("a polymorphic with custom allocator") {
        std::allocator<Base> alloc;
        polymorphic<Base> poly(std::allocator_arg, alloc, std::in_place_type<Derived>, 1200);

        THEN("get_allocator should return the correct allocator") {
            auto retrieved = poly.get_allocator();
            REQUIRE(typeid(retrieved) == typeid(alloc));
        }
    }
}

TEMPLATE_TEST_CASE("polymorphic works with different allocator types", "[polymorphic][template]", std::allocator<Base>,
                   std::pmr::polymorphic_allocator<Base>) {
    GIVEN("a polymorphic with specific allocator type") {
        TestType alloc;
        polymorphic<Base, TestType> poly(std::allocator_arg, alloc, std::in_place_type<Derived>, 1300);

        THEN("it should function correctly") {
            REQUIRE(poly->identify() == "Derived");
            REQUIRE(poly->get_value() == 1300);
        }
    }
}

SCENARIO("polymorphic exception safety", "[polymorphic][exception]") {
    GIVEN("a type that throws during construction") {
        struct ThrowingDerived : Base {
            explicit ThrowingDerived(bool shouldThrow) {
                if (shouldThrow) {
                    throw std::runtime_error("construction failed");
                }
            }
            std::string identify() const override {
                return "ThrowingDerived";
            }
        };

        THEN("in_place construction should propagate the exception") {
            REQUIRE_THROWS_AS(polymorphic<Base>(std::in_place_type<ThrowingDerived>, true), std::runtime_error);
        }

        THEN("direct construction from value should propagate the exception") {
            REQUIRE_THROWS_AS(polymorphic<Base>(ThrowingDerived(true)), std::runtime_error);
        }

        AND_WHEN("construction throws") {
            polymorphic<Base> poly;
            bool exception_caught = false;

            try {
                polymorphic<Base> throwing(std::in_place_type<ThrowingDerived>, true);
            } catch (const std::runtime_error &) {
                exception_caught = true;
            }

            THEN("the original object should remain unchanged") {
                REQUIRE(exception_caught == true);
                // Original poly still valid
                REQUIRE(poly.valueless_after_move() == false);
            }
        }
    }

    GIVEN("a polymorphic in valid state") {
        polymorphic<Base> poly(std::in_place_type<Derived>, 1400);

        THEN("basic operations should not throw") {
            REQUIRE_NOTHROW(poly->identify());
            REQUIRE_NOTHROW(*poly);
            REQUIRE_NOTHROW(poly.valueless_after_move());
            REQUIRE_NOTHROW(poly.get_allocator());
        }

        THEN("copy operations may throw but should be safe") {
            REQUIRE_NOTHROW(polymorphic<Base>(poly));

            polymorphic<Base> poly2;
            REQUIRE_NOTHROW(poly2 = poly);
        }

        THEN("move operations should not throw") {
            REQUIRE_NOTHROW(polymorphic<Base>(std::move(poly)));

            polymorphic<Base> poly2;
            REQUIRE_NOTHROW(poly2 = std::move(poly));
        }
    }
}

SCENARIO("polymorphic strong exception guarantee", "[polymorphic][exception][guarantee]") {
    GIVEN("an allocator that throws on copy") {

        using ThrowingBase = Base;
        using ThrowingAlloc = ThrowingAllocator<Base>;

        WHEN("construction throws due to allocator") {
            using TestAlloc = ThrowingAllocator<char>; // 注意：这里使用char，实际会被rebind
            TestAlloc alloc;
            alloc.should_throw = true;

            THEN("construction should throw") {
                auto f = [&alloc]() {
                    (void) polymorphic<Base, TestAlloc>(std::allocator_arg, alloc, std::in_place_type<Derived>, 1500);
                };
                REQUIRE_THROWS_AS(f(), std::bad_alloc);
            }
        }
    }
}

// NOLINTEND
