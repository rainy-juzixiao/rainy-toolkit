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
#include <catch2/matchers/catch_matchers_all.hpp>
#include <rainy/foundation/container/indirect.hpp>
#include <string>
#include <vector>

using namespace rainy::foundation::container;

// Helper class for testing
struct TestObject {
    int value;
    std::string name;

    TestObject() : value(0), name("default") {
    }
    TestObject(int v) : value(v), name("int_ctor") {
    }
    TestObject(int v, std::string n) : value(v), name(std::move(n)) {
    }
    TestObject(std::initializer_list<int> ilist) : value(*ilist.begin()), name("ilist_ctor") {
    }

    bool operator==(const TestObject &other) const {
        return value == other.value && name == other.name;
    }

    bool operator<(const TestObject &other) const {
        return value < other.value;
    }
};

SCENARIO("indirect can be default constructed", "[indirect][constructor]") {
    GIVEN("a default constructible type") {
        WHEN("creating an indirect with default constructor") {
            indirect<TestObject> obj;

            THEN("the indirect is not empty") {
                REQUIRE_FALSE(obj.empty());
            }

            THEN("the object has default values") {
                REQUIRE(obj->value == 0);
                REQUIRE(obj->name == "default");
            }
        }
    }
}

SCENARIO("indirect can be constructed with deferred initialization", "[indirect][constructor]") {
    GIVEN("the deferred_init tag") {
        WHEN("creating an indirect with deferred_init") {
            indirect<TestObject> obj(defered_init);

            THEN("the indirect is empty") {
                REQUIRE(obj.empty());
            }

            THEN("get() returns nullptr") {
                REQUIRE(obj.get() == nullptr);
            }
        }
    }
}

SCENARIO("indirect can be constructed with in_place and arguments", "[indirect][constructor]") {
    GIVEN("constructor arguments") {
        WHEN("constructing with std::in_place and single argument") {
            indirect<TestObject> obj(std::in_place, 42);

            THEN("the object is constructed with the argument") {
                REQUIRE(obj->value == 42);
                REQUIRE(obj->name == "int_ctor");
            }
        }

        WHEN("constructing with std::in_place and multiple arguments") {
            indirect<TestObject> obj(std::in_place, 100, "custom");

            THEN("the object is constructed with all arguments") {
                REQUIRE(obj->value == 100);
                REQUIRE(obj->name == "custom");
            }
        }
    }
}

SCENARIO("indirect can be constructed with initializer list", "[indirect][constructor]") {
    GIVEN("an initializer list") {
        WHEN("constructing with std::in_place and initializer_list") {
            indirect<TestObject> obj(std::in_place, {5, 10, 15});

            THEN("the object is constructed from the list") {
                REQUIRE(obj->value == 5);
                REQUIRE(obj->name == "ilist_ctor");
            }
        }
    }
}

SCENARIO("indirect can be constructed with custom allocator", "[indirect][constructor]") {
    GIVEN("a custom allocator") {
        std::allocator<TestObject> alloc;

        WHEN("constructing with allocator_arg and allocator") {
            indirect<TestObject> obj(std::allocator_arg, alloc);

            THEN("the indirect is not empty") {
                REQUIRE_FALSE(obj.empty());
            }

            THEN("get_allocator returns the allocator") {
                auto retrieved_alloc = obj.get_allocator();
                REQUIRE(retrieved_alloc == alloc);
            }
        }
    }
}

SCENARIO("indirect can be copy constructed", "[indirect][constructor][copy]") {
    GIVEN("an existing indirect object") {
        indirect<TestObject> original(std::in_place, 42, "original");

        WHEN("copy constructing from it") {
            indirect<TestObject> copy(original);

            THEN("the copy has the same value") {
                REQUIRE(copy->value == 42);
                REQUIRE(copy->name == "original");
            }

            THEN("the copy is independent") {
                copy->value = 100;
                REQUIRE(original->value == 42);
            }
        }
    }

    GIVEN("an empty indirect object") {
        indirect<TestObject> original(defered_init);

        WHEN("copy constructing from it") {
            indirect<TestObject> copy(original);

            THEN("the copy is also empty") {
                REQUIRE(copy.empty());
            }
        }
    }
}

SCENARIO("indirect can be move constructed", "[indirect][constructor][move]") {
    GIVEN("an existing indirect object") {
        indirect<TestObject> original(std::in_place, 42, "original");

        WHEN("move constructing from it") {
            indirect<TestObject> moved(std::move(original));

            THEN("the moved object has the value") {
                REQUIRE(moved->value == 42);
                REQUIRE(moved->name == "original");
            }

            THEN("the original is empty") {
                REQUIRE(original.empty());
            }

            THEN("the original is valueless after move") {
                REQUIRE(original.valueless_after_move());
            }
        }
    }
}

SCENARIO("indirect can be constructed from a value", "[indirect][constructor]") {
    GIVEN("a value of the contained type") {
        TestObject value(77, "explicit");

        WHEN("constructing with explicit constructor") {
            indirect<TestObject> obj(value);

            THEN("the object contains a copy of the value") {
                REQUIRE(obj->value == 77);
                REQUIRE(obj->name == "explicit");
            }
        }
    }
}

SCENARIO("indirect supports copy assignment", "[indirect][assignment]") {
    GIVEN("two indirect objects") {
        indirect<TestObject> lhs(std::in_place, 10, "left");
        indirect<TestObject> rhs(std::in_place, 20, "right");

        WHEN("copy assigning") {
            lhs = rhs;

            THEN("lhs has rhs's value") {
                REQUIRE(lhs->value == 20);
                REQUIRE(lhs->name == "right");
            }

            THEN("they are independent") {
                lhs->value = 99;
                REQUIRE(rhs->value == 20);
            }
        }
    }

    GIVEN("an empty indirect and a non-empty indirect") {
        indirect<TestObject> empty_obj(defered_init);
        indirect<TestObject> non_empty(std::in_place, 30, "filled");

        WHEN("assigning non-empty to empty") {
            empty_obj = non_empty;

            THEN("empty is no longer empty") {
                REQUIRE_FALSE(empty_obj.empty());
                REQUIRE(empty_obj->value == 30);
            }
        }

        WHEN("assigning empty to non-empty") {
            non_empty = empty_obj;

            THEN("non-empty becomes empty") {
                REQUIRE(non_empty.empty());
            }
        }
    }
}

SCENARIO("indirect supports move assignment", "[indirect][assignment][move]") {
    GIVEN("two indirect objects") {
        indirect<TestObject> lhs(std::in_place, 10, "left");
        indirect<TestObject> rhs(std::in_place, 20, "right");

        WHEN("move assigning") {
            lhs = std::move(rhs);

            THEN("lhs has the moved value") {
                REQUIRE(lhs->value == 20);
                REQUIRE(lhs->name == "right");
            }

            THEN("rhs is empty") {
                REQUIRE(rhs.empty());
            }
        }
    }
}

SCENARIO("indirect supports value assignment", "[indirect][assignment]") {
    GIVEN("an indirect object and a value") {
        indirect<TestObject> obj(std::in_place, 10, "old");
        TestObject new_value(50, "new");

        WHEN("assigning the value") {
            obj = new_value;

            THEN("the indirect contains the new value") {
                REQUIRE(obj->value == 50);
                REQUIRE(obj->name == "new");
            }
        }
    }

    GIVEN("an empty indirect and a value") {
        indirect<TestObject> obj(defered_init);
        TestObject value(60, "first");

        WHEN("assigning the value") {
            obj = value;

            THEN("the indirect is no longer empty") {
                REQUIRE_FALSE(obj.empty());
                REQUIRE(obj->value == 60);
            }
        }
    }
}

SCENARIO("indirect supports emplace", "[indirect][emplace]") {
    GIVEN("an indirect object") {
        indirect<TestObject> obj(std::in_place, 10, "initial");

        WHEN("emplacing with new arguments") {
            auto &ref = obj.emplace(99, "emplaced");

            THEN("the object is replaced") {
                REQUIRE(obj->value == 99);
                REQUIRE(obj->name == "emplaced");
            }

            THEN("emplace returns a reference to the value") {
                REQUIRE(ref.value == 99);
            }
        }
    }

    GIVEN("an empty indirect") {
        indirect<TestObject> obj(defered_init);

        WHEN("emplacing") {
            obj.emplace(123, "created");

            THEN("the object is created") {
                REQUIRE_FALSE(obj.empty());
                REQUIRE(obj->value == 123);
            }
        }
    }

    GIVEN("an indirect object") {
        indirect<TestObject> obj(std::in_place, 1);

        WHEN("emplacing with initializer list") {
            obj.emplace({7, 8, 9});

            THEN("the object is constructed from the list") {
                REQUIRE(obj->value == 7);
                REQUIRE(obj->name == "ilist_ctor");
            }
        }
    }
}

SCENARIO("indirect can be reset", "[indirect][reset]") {
    GIVEN("a non-empty indirect") {
        indirect<TestObject> obj(std::in_place, 42, "value");

        WHEN("calling reset") {
            obj.reset();

            THEN("the indirect becomes empty") {
                REQUIRE(obj.empty());
            }

            THEN("get returns nullptr") {
                REQUIRE(obj.get() == nullptr);
            }
        }
    }

    GIVEN("an empty indirect") {
        indirect<TestObject> obj(defered_init);

        WHEN("calling reset") {
            obj.reset();

            THEN("it remains empty") {
                REQUIRE(obj.empty());
            }
        }
    }
}

SCENARIO("indirect provides pointer-like access", "[indirect][access]") {
    GIVEN("a non-empty indirect") {
        indirect<TestObject> obj(std::in_place, 42, "test");

        WHEN("using operator->") {
            auto value = obj->value;
            auto name = obj->name;

            THEN("members can be accessed") {
                REQUIRE(value == 42);
                REQUIRE(name == "test");
            }
        }

        WHEN("using operator*") {
            auto &ref = *obj;

            THEN("a reference to the value is returned") {
                REQUIRE(ref.value == 42);
                ref.value = 100;
                REQUIRE(obj->value == 100);
            }
        }

        WHEN("using get()") {
            auto *ptr = obj.get();

            THEN("a pointer to the value is returned") {
                REQUIRE(ptr != nullptr);
                REQUIRE(ptr->value == 42);
            }
        }
    }
}

SCENARIO("indirect provides const access", "[indirect][access][const]") {
    GIVEN("a const indirect") {
        const indirect<TestObject> obj(std::in_place, 42, "const_test");

        WHEN("using operator-> const") {
            auto value = obj->value;

            THEN("members can be read") {
                REQUIRE(value == 42);
            }
        }

        WHEN("using operator* const") {
            const auto &ref = *obj;

            THEN("a const reference is returned") {
                REQUIRE(ref.value == 42);
            }
        }
    }
}

SCENARIO("indirect provides iterator interface", "[indirect][iterator]") {
    GIVEN("a non-empty indirect") {
        indirect<TestObject> obj(std::in_place, 42, "iter");

        WHEN("using begin() and end()") {
            auto begin_it = obj.begin();
            auto end_it = obj.end();

            THEN("begin points to the value") {
                REQUIRE(begin_it != nullptr);
                REQUIRE(begin_it->value == 42);
            }

            THEN("the range has size 1") {
                REQUIRE(end_it - begin_it == 1);
            }
        }

        WHEN("using range-based for loop") {
            int count = 0;
            for (auto &item: obj) {
                REQUIRE(item.value == 42);
                count++;
            }

            THEN("the loop iterates once") {
                REQUIRE(count == 1);
            }
        }
    }

    GIVEN("an empty indirect") {
        indirect<TestObject> obj(defered_init);

        WHEN("using begin() and end()") {
            auto begin_it = obj.begin();
            auto end_it = obj.end();

            THEN("both are nullptr") {
                REQUIRE(begin_it == nullptr);
                REQUIRE(end_it == nullptr);
            }
        }
    }
}

SCENARIO("indirect can be swapped", "[indirect][swap]") {
    GIVEN("two indirect objects") {
        indirect<TestObject> obj1(std::in_place, 10, "first");
        indirect<TestObject> obj2(std::in_place, 20, "second");

        WHEN("swapping them") {
            obj1.swap(obj2);

            THEN("their values are exchanged") {
                REQUIRE(obj1->value == 20);
                REQUIRE(obj1->name == "second");
                REQUIRE(obj2->value == 10);
                REQUIRE(obj2->name == "first");
            }
        }
    }

    GIVEN("one empty and one non-empty indirect") {
        indirect<TestObject> empty_obj(defered_init);
        indirect<TestObject> filled_obj(std::in_place, 30, "filled");

        WHEN("swapping them") {
            empty_obj.swap(filled_obj);

            THEN("states are exchanged") {
                REQUIRE_FALSE(empty_obj.empty());
                REQUIRE(empty_obj->value == 30);
                REQUIRE(filled_obj.empty());
            }
        }
    }
}

SCENARIO("indirect supports equality comparison", "[indirect][comparison]") {
    GIVEN("two indirect objects with same values") {
        indirect<TestObject> obj1(std::in_place, 42, "same");
        indirect<TestObject> obj2(std::in_place, 42, "same");

        WHEN("comparing with ==") {
            THEN("they are equal") {
                REQUIRE(obj1 == obj2);
            }
        }
    }

    GIVEN("two indirect objects with different values") {
        indirect<TestObject> obj1(std::in_place, 10, "first");
        indirect<TestObject> obj2(std::in_place, 20, "second");

        WHEN("comparing with !=") {
            THEN("they are not equal") {
                REQUIRE(obj1 != obj2);
            }
        }
    }

    GIVEN("two empty indirect objects") {
        indirect<TestObject> obj1(defered_init);
        indirect<TestObject> obj2(defered_init);

        WHEN("comparing with ==") {
            THEN("they are equal") {
                REQUIRE(obj1 == obj2);
            }
        }
    }

    GIVEN("one empty and one non-empty indirect") {
        indirect<TestObject> empty_obj(defered_init);
        indirect<TestObject> filled_obj(std::in_place, 42, "filled");

        WHEN("comparing") {
            THEN("they are not equal") {
                REQUIRE(empty_obj != filled_obj);
            }
        }
    }
}

SCENARIO("indirect supports nullptr comparison", "[indirect][comparison][nullptr]") {
    GIVEN("an empty indirect") {
        indirect<TestObject> obj(defered_init);

        WHEN("comparing with nullptr") {
            THEN("it equals nullptr") {
                REQUIRE(obj == nullptr);
                REQUIRE(nullptr == obj);
            }
        }
    }

    GIVEN("a non-empty indirect") {
        indirect<TestObject> obj(std::in_place, 42);

        WHEN("comparing with nullptr") {
            THEN("it does not equal nullptr") {
                REQUIRE(obj != nullptr);
                REQUIRE(nullptr != obj);
            }
        }
    }
}

SCENARIO("indirect supports relational comparisons", "[indirect][comparison]") {
    GIVEN("two indirect objects") {
        indirect<TestObject> obj1(std::in_place, 10, "first");
        indirect<TestObject> obj2(std::in_place, 20, "second");

        WHEN("comparing with <") {
            THEN("smaller value compares less") {
                REQUIRE(obj1 < obj2);
                REQUIRE_FALSE(obj2 < obj1);
            }
        }

        WHEN("comparing with >") {
            THEN("larger value compares greater") {
                REQUIRE(obj2 > obj1);
                REQUIRE_FALSE(obj1 > obj2);
            }
        }

        WHEN("comparing with <= and >=") {
            THEN("comparisons work correctly") {
                REQUIRE(obj1 <= obj2);
                REQUIRE(obj2 >= obj1);
            }
        }
    }

    GIVEN("an empty indirect and a non-empty indirect") {
        indirect<TestObject> empty_obj(defered_init);
        indirect<TestObject> filled_obj(std::in_place, 10);

        WHEN("comparing") {
            THEN("empty compares less than non-empty") {
                REQUIRE(empty_obj < filled_obj);
                REQUIRE(filled_obj > empty_obj);
            }
        }
    }
}

SCENARIO("indirect supports comparison with values", "[indirect][comparison]") {
    GIVEN("an indirect and a value") {
        indirect<TestObject> obj(std::in_place, 42, "test");
        TestObject value(42, "test");

        WHEN("comparing with ==") {
            THEN("they are equal if values match") {
                REQUIRE(obj == value);
                REQUIRE(value == obj);
            }
        }
    }

    GIVEN("an empty indirect and a value") {
        indirect<TestObject> obj(defered_init);
        TestObject value(10, "test");

        WHEN("comparing") {
            THEN("they are not equal") {
                REQUIRE(obj != value);
                REQUIRE(value != obj);
            }
        }
    }
}

SCENARIO("indirect supports make_indirect factory functions", "[indirect][factory]") {
    WHEN("using make_indirect with arguments") {
        auto obj = make_indirect<TestObject>(99, "factory");

        THEN("object is created correctly") {
            REQUIRE(obj->value == 99);
            REQUIRE(obj->name == "factory");
        }
    }

    WHEN("using make_indirect with allocator") {
        std::allocator<TestObject> alloc;
        auto obj = make_indirect<TestObject>(alloc, 88, "alloc_factory");

        THEN("object is created with custom allocator") {
            REQUIRE(obj->value == 88);
            REQUIRE(obj->name == "alloc_factory");
        }
    }

    WHEN("using make_indirect with initializer list") {
        auto obj = make_indirect<TestObject>({3, 6, 9});

        THEN("object is created from initializer list") {
            REQUIRE(obj->value == 3);
            REQUIRE(obj->name == "ilist_ctor");
        }
    }
}

SCENARIO("indirect supports CTAD (Class Template Argument Deduction)", "[indirect][ctad]") {
    WHEN("constructing with a value") {
        TestObject value(55, "ctad");
        indirect obj(value);

        THEN("type is deduced correctly") {
            REQUIRE(obj->value == 55);
            REQUIRE(obj->name == "ctad");
        }
    }

    WHEN("constructing with allocator_arg") {
        std::allocator<TestObject> alloc;
        TestObject value(66, "ctad_alloc");
        indirect obj(std::allocator_arg, alloc, value);

        THEN("type is deduced with allocator") {
            REQUIRE(obj->value == 66);
        }
    }
}
