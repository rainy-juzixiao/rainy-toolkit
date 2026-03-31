#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/container/optional.hpp>
#include <string>
#include <vector>

using namespace rainy::foundation::container;

SCENARIO("optional can be constructed in various ways", "[optional][construction]") {
    GIVEN("a default constructed optional") {
        optional<int> opt;

        THEN("it should not have a value") {
            REQUIRE_FALSE(opt.has_value());
            REQUIRE_FALSE(opt);
        }
    }

    GIVEN("an optional constructed with nullopt") {
        optional<int> opt(nullopt);

        THEN("it should not have a value") {
            REQUIRE_FALSE(opt.has_value());
        }
    }

    GIVEN("an optional constructed with a value") {
        optional<int> opt(42);

        THEN("it should contain the value") {
            REQUIRE(opt.has_value());
            REQUIRE(*opt == 42);
            REQUIRE(opt.value() == 42);
        }
    }

    GIVEN("an optional constructed with in_place") {
        optional<std::string> opt(std::in_place, "hello");

        THEN("it should contain the constructed value") {
            REQUIRE(opt.has_value());
            REQUIRE(*opt == "hello");
        }
    }

    GIVEN("an optional constructed with initializer_list") {
        optional<std::vector<int>> opt(std::in_place, {1, 2, 3, 4, 5});

        THEN("it should contain the vector with all elements") {
            REQUIRE(opt.has_value());
            REQUIRE(opt->size() == 5);
            REQUIRE((*opt)[0] == 1);
            REQUIRE((*opt)[4] == 5);
        }
    }
}

SCENARIO("optional can be copy and move constructed", "[optional][construction]") {
    GIVEN("an optional with a value") {
        constexpr optional<int> o(10);
        optional<std::string> opt1("test");

        WHEN("copy constructed") {
            optional<std::string> opt2(opt1);

            THEN("both should have the same value") {
                REQUIRE(opt1.has_value());
                REQUIRE(opt2.has_value());
                REQUIRE(*opt1 == *opt2);
                REQUIRE(*opt2 == "test");
            }
        }

        WHEN("move constructed") {
            optional<std::string> opt2(std::move(opt1));

            THEN("the new optional should have the value") {
                REQUIRE(opt2.has_value());
                REQUIRE(*opt2 == "test");
            }
        }
    }

    GIVEN("an empty optional") {
        optional<int> opt1;

        WHEN("copy constructed") {
            optional<int> opt2(opt1);

            THEN("both should be empty") {
                REQUIRE_FALSE(opt1.has_value());
                REQUIRE_FALSE(opt2.has_value());
            }
        }
    }
}

SCENARIO("optional supports assignment operations", "[optional][assignment]") {
    GIVEN("an optional with a value") {
        optional<int> opt(42);

        WHEN("assigned nullopt") {
            opt = nullopt;

            THEN("it should become empty") {
                REQUIRE_FALSE(opt.has_value());
            }
        }

        WHEN("assigned a new value") {
            opt = 100;

            THEN("it should contain the new value") {
                REQUIRE(opt.has_value());
                REQUIRE(*opt == 100);
            }
        }

        WHEN("assigned from another optional") {
            optional<int> opt2(99);
            opt = opt2;

            THEN("it should contain the copied value") {
                REQUIRE(opt.has_value());
                REQUIRE(*opt == 99);
            }
        }

        WHEN("assigned from an empty optional") {
            optional<int> opt2;
            opt = opt2;

            THEN("it should become empty") {
                REQUIRE_FALSE(opt.has_value());
            }
        }
    }

    GIVEN("an empty optional") {
        optional<int> opt;

        WHEN("assigned a value") {
            opt = 42;

            THEN("it should contain the value") {
                REQUIRE(opt.has_value());
                REQUIRE(*opt == 42);
            }
        }
    }
}

SCENARIO("optional provides value access methods", "[optional][access]") {
    GIVEN("an optional with a value") {
        optional<std::string> opt("hello");

        WHEN("accessing via operator*") {
            std::string &value = *opt;

            THEN("it should return the value") {
                REQUIRE(value == "hello");
            }
        }

        WHEN("accessing via operator->") {
            THEN("it should allow member access") {
                REQUIRE(opt->size() == 5);
                REQUIRE(opt->empty() == false);
            }
        }

        WHEN("accessing via value()") {
            std::string &value = opt.value();

            THEN("it should return the value") {
                REQUIRE(value == "hello");
            }
        }

        WHEN("using value_or with an optional that has value") {
            auto result = opt.value_or("default");

            THEN("it should return the contained value") {
                REQUIRE(result == "hello");
            }
        }
    }

    GIVEN("an empty optional") {
        optional<std::string> opt;

        WHEN("accessing via value()") {
            THEN("it should throw bad_optional_access") {
                REQUIRE_THROWS(opt.value());
            }
        }

        WHEN("accessing via operator->") {
            THEN("it should throw bad_optional_access") {
                REQUIRE_THROWS(opt->size());
            }
        }

        WHEN("using value_or") {
            auto result = opt.value_or("default");

            THEN("it should return the default value") {
                REQUIRE(result == "default");
            }
        }
    }
}

SCENARIO("optional supports emplace operations", "[optional][emplace]") {
    GIVEN("an empty optional") {
        optional<std::string> opt;

        WHEN("emplacing a value") {
            opt.emplace("new value");

            THEN("it should contain the emplaced value") {
                REQUIRE(opt.has_value());
                REQUIRE(*opt == "new value");
            }
        }
    }

    GIVEN("an optional with a value") {
        optional<std::string> opt("old value");

        WHEN("emplacing a new value") {
            opt.emplace("new value");

            THEN("it should replace with the new value") {
                REQUIRE(opt.has_value());
                REQUIRE(*opt == "new value");
            }
        }
    }

    GIVEN("an optional for vector") {
        optional<std::vector<int>> opt;

        WHEN("emplacing with initializer list") {
            opt.emplace({1, 2, 3});

            THEN("it should contain the vector") {
                REQUIRE(opt.has_value());
                REQUIRE(opt->size() == 3);
            }
        }
    }
}

SCENARIO("optional supports reset operation", "[optional][reset]") {
    GIVEN("an optional with a value") {
        optional<int> opt(42);

        WHEN("reset is called") {
            opt.reset();

            THEN("it should become empty") {
                REQUIRE_FALSE(opt.has_value());
            }
        }
    }

    GIVEN("an empty optional") {
        optional<int> opt;

        WHEN("reset is called") {
            opt.reset();

            THEN("it should remain empty") {
                REQUIRE_FALSE(opt.has_value());
            }
        }
    }
}

SCENARIO("optional supports swap operation", "[optional][swap]") {
    GIVEN("two optionals with values") {
        optional<int> opt1(10);
        optional<int> opt2(20);

        WHEN("swapped") {
            opt1.swap(opt2);

            THEN("values should be exchanged") {
                REQUIRE(*opt1 == 20);
                REQUIRE(*opt2 == 10);
            }
        }
    }

    GIVEN("one empty and one filled optional") {
        optional<int> opt1(42);
        optional<int> opt2;

        WHEN("swapped") {
            opt1.swap(opt2);

            THEN("the states should be exchanged") {
                REQUIRE_FALSE(opt1.has_value());
                REQUIRE(opt2.has_value());
                REQUIRE(*opt2 == 42);
            }
        }
    }

    GIVEN("two empty optionals") {
        optional<int> opt1;
        optional<int> opt2;

        WHEN("swapped") {
            opt1.swap(opt2);

            THEN("both should remain empty") {
                REQUIRE_FALSE(opt1.has_value());
                REQUIRE_FALSE(opt2.has_value());
            }
        }
    }
}

SCENARIO("optional supports monadic operations", "[optional][monadic]") {
    GIVEN("an optional with a value") {
        optional<int> opt(5);

        WHEN("using and_then") {
            auto result = opt.and_then([](int x) { return optional<int>(x * 2); });

            THEN("it should apply the function and return optional") {
                REQUIRE(result.has_value());
                REQUIRE(*result == 10);
            }
        }

        WHEN("using transform") {
            auto result = opt.transform([](int x) { return x * 2; });

            THEN("it should transform the value") {
                REQUIRE(result.has_value());
                REQUIRE(*result == 10);
            }
        }

        WHEN("using or_else") {
            auto result = opt.or_else([]() { return optional<int>(999); });

            THEN("it should return the original optional") {
                REQUIRE(result.has_value());
                REQUIRE(*result == 5);
            }
        }
    }

    GIVEN("an empty optional") {
        optional<int> opt;

        WHEN("using and_then") {
            auto result = opt.and_then([](int x) { return optional<int>(x * 2); });

            THEN("it should return empty optional") {
                REQUIRE_FALSE(result.has_value());
            }
        }

        WHEN("using transform") {
            auto result = opt.transform([](int x) { return x * 2; });

            THEN("it should return empty optional") {
                REQUIRE_FALSE(result.has_value());
            }
        }

        WHEN("using or_else") {
            auto result = opt.or_else([]() { return optional<int>(999); });

            THEN("it should return the alternative") {
                REQUIRE(result.has_value());
                REQUIRE(*result == 999);
            }
        }
    }
}

SCENARIO("optional supports iterator interface", "[optional][iterators]") {
    GIVEN("an optional with a value") {
        optional<int> opt(42);

        WHEN("iterating with begin/end") {
            THEN("it should iterate once") {
                int count = 0;
                for (auto &val: opt) {
                    REQUIRE(val == 42);
                    count++;
                }
                REQUIRE(count == 1);
            }
        }
    }

    GIVEN("an empty optional") {
        optional<int> opt;

        WHEN("iterating with begin/end") {
            THEN("it should not iterate") {
                int count = 0;
                for (auto &val: opt) {
                    (void) val;
                    count++;
                }
                REQUIRE(count == 0);
            }
        }
    }
}

SCENARIO("make_optional factory functions", "[optional][factory]") {
    WHEN("using make_optional with a value") {
        auto opt = make_optional(42);

        THEN("it should create an optional with the value") {
            REQUIRE(opt.has_value());
            REQUIRE(*opt == 42);
        }
    }

    WHEN("using make_optional with type and args") {
        auto opt = make_optional<std::string>("hello", 3);

        THEN("it should construct the value") {
            REQUIRE(opt.has_value());
            REQUIRE(*opt == "hel");
        }
    }

    WHEN("using make_optional with initializer_list") {
        auto opt = make_optional<std::vector<int>>({1, 2, 3});

        THEN("it should create optional with the vector") {
            REQUIRE(opt.has_value());
            REQUIRE(opt->size() == 3);
        }
    }
}

SCENARIO("optional handles complex types", "[optional][complex]") {
    struct ComplexType {
        int id;
        std::string name;
        std::vector<int> data;

        ComplexType(int i, std::string n) : id(i), name(std::move(n)) {
        }
    };

    GIVEN("an optional of complex type") {
        optional<ComplexType> opt;

        WHEN("emplacing a complex object") {
            opt.emplace(1, "test");

            THEN("it should contain the object") {
                REQUIRE(opt.has_value());
                REQUIRE(opt->id == 1);
                REQUIRE(opt->name == "test");
            }
        }
    }
}

SCENARIO("optional comparison operators", "[optional][comparison]") {
    GIVEN("two optionals with values") {
        optional<int> opt1{42};
        optional<int> opt2{42};
        optional<int> opt3{100};

        WHEN("comparing equal optionals") {
            THEN("equality comparison should work") {
                REQUIRE(opt1 == opt2);
#if !RAINY_HAS_CXX20
                REQUIRE_FALSE(opt1 != opt2);
#endif
            }
        }

        WHEN("comparing different optionals") {
            THEN("inequality comparison should work") {
#if !RAINY_HAS_CXX20
                REQUIRE(opt1 != opt3);
#endif
                REQUIRE_FALSE(opt1 == opt3);
            }
            THEN("relational comparisons should work") {
                REQUIRE(opt1 < opt3);
#if !RAINY_HAS_CXX20
                REQUIRE(opt1 <= opt3);
                REQUIRE(opt3 > opt1);
                REQUIRE(opt3 >= opt1);
#endif
            }
        }
    }

    GIVEN("optionals with and without values") {
        optional<int> empty;
        optional<int> has_value{42};

        WHEN("comparing empty and non-empty optionals") {
            THEN("empty should be less than non-empty") {
                REQUIRE(empty < has_value);
#if !RAINY_HAS_CXX20
                REQUIRE(empty <= has_value);
                REQUIRE(has_value > empty);
                REQUIRE(has_value >= empty);
                REQUIRE(empty != has_value);
#endif
            }
        }

        WHEN("comparing two empty optionals") {
            optional<int> empty2;
            THEN("they should be equal") {
                REQUIRE(empty == empty2);
                REQUIRE_FALSE(empty < empty2);
#if !RAINY_HAS_CXX20
                REQUIRE(empty <= empty2);
                REQUIRE(empty >= empty2);
#endif
            }
        }
    }

    GIVEN("optionals with different types") {
        optional<int> opt_int{42};
        optional<long> opt_long{42L};
        optional<double> opt_double{42.0};

        WHEN("comparing compatible types") {
            THEN("cross-type equality should work") {
                REQUIRE(opt_int == opt_long);
                REQUIRE(opt_int == opt_double);
            }
            THEN("cross-type relational comparisons should work") {
                REQUIRE_FALSE(opt_int < opt_long);
#if !RAINY_HAS_CXX20
                REQUIRE(opt_int <= opt_long);
#endif
            }
        }
    }
}

SCENARIO("optional comparison with nullopt", "[optional][comparison][nullopt]") {
    GIVEN("an empty optional") {
        optional<int> empty;

        WHEN("comparing with nullopt") {
            THEN("it should be equal to nullopt") {
                REQUIRE(empty == nullopt);
#if !RAINY_HAS_CXX20
                REQUIRE(nullopt == empty);
                REQUIRE_FALSE(empty != nullopt);
                REQUIRE_FALSE(nullopt != empty);
#endif
            }
#if !RAINY_HAS_CXX20
            THEN("relational comparisons should work") {
                REQUIRE_FALSE(empty < nullopt);
                REQUIRE_FALSE(nullopt > empty);
                REQUIRE(empty <= nullopt);
                REQUIRE(nullopt <= empty);
                REQUIRE(empty >= nullopt);
                REQUIRE(nullopt >= empty);
            }
#endif
        }
    }

    GIVEN("an optional with value") {
        optional<int> has_value{42};

        WHEN("comparing with nullopt") {
            THEN("it should not be equal to nullopt") {
#if !RAINY_HAS_CXX20
                REQUIRE(has_value != nullopt);
                REQUIRE(nullopt != has_value);
#endif
                REQUIRE_FALSE(has_value == nullopt);
#if !RAINY_HAS_CXX20
                REQUIRE_FALSE(nullopt == has_value);
#endif
            }
#if !RAINY_HAS_CXX20
            THEN("non-empty should be greater than nullopt") {
                REQUIRE(has_value > nullopt);
                REQUIRE(nullopt < has_value);
                REQUIRE(has_value >= nullopt);
                REQUIRE(nullopt <= has_value);
            }
#endif
        }
    }
}

SCENARIO("optional comparison with values", "[optional][comparison][value]") {
    GIVEN("an optional with value") {
        optional<int> opt{42};

        WHEN("comparing with equal value") {
            THEN("equality should hold") {
                REQUIRE(opt == 42);
                REQUIRE(42 == opt);
#if !RAINY_HAS_CXX20
                REQUIRE_FALSE(opt != 42);
                REQUIRE_FALSE(42 != opt);
#endif
            }
        }

        WHEN("comparing with different value") {
            THEN("inequality should hold") {
#if !RAINY_HAS_CXX20
                REQUIRE(opt != 100);
                REQUIRE(100 != opt);
#endif
                REQUIRE_FALSE(opt == 100);
                REQUIRE_FALSE(100 == opt);
            }
            THEN("relational comparisons should work") {
                REQUIRE(opt < 100);
#if !RAINY_HAS_CXX20
                REQUIRE(100 > opt);
                REQUIRE(opt > 10);
#endif
                REQUIRE(10 < opt);
#if !RAINY_HAS_CXX20
                REQUIRE(opt <= 100);
                REQUIRE(opt >= 10);
                REQUIRE(100 >= opt);
                REQUIRE(10 <= opt);
#endif
            }
        }
    }

    GIVEN("an empty optional") {
        optional<int> empty;

        WHEN("comparing with any value") {
            THEN("it should not be equal") {
                REQUIRE_FALSE(empty == 42);
                REQUIRE_FALSE(42 == empty);
#if !RAINY_HAS_CXX20
                REQUIRE(empty != 42);
                REQUIRE(42 != empty);
#endif
            }
            THEN("empty should be less than any value") {
                REQUIRE(empty < 42);
                REQUIRE(empty < -100);
                REQUIRE_FALSE(42 < empty);
#if !RAINY_HAS_CXX20
                REQUIRE(42 > empty);
                REQUIRE(empty <= 42);
                REQUIRE_FALSE(42 <= empty);
#endif
            }
        }
    }

    GIVEN("optional with string") {
        optional<std::string> opt{"hello"};

        WHEN("comparing with string values") {
            THEN("string comparison should work") {
                REQUIRE(opt == "hello");
                REQUIRE("hello" == opt);
                REQUIRE(opt < "world");
                REQUIRE("apple" < opt);
#if !RAINY_HAS_CXX20
                REQUIRE(opt > "apple");
                REQUIRE("world" > opt);
                REQUIRE(opt <= "hello");
                REQUIRE(opt >= "hello");
#endif
            }
        }
    }
}

#if RAINY_HAS_CXX20
SCENARIO("optional three-way comparison (C++20)", "[optional][comparison][spaceship]") {
    GIVEN("optionals with values") {
        optional<int> opt1{42};
        optional<int> opt2{42};
        optional<int> opt3{100};

        WHEN("using spaceship operator") {
            THEN("equal values should have strong equality") {
                REQUIRE((opt1 <=> opt2) == std::strong_ordering::equal);
            }
            THEN("different values should compare correctly") {
                REQUIRE((opt1 <=> opt3) == std::strong_ordering::less);
                REQUIRE((opt3 <=> opt1) == std::strong_ordering::greater);
            }
        }
    }

    GIVEN("empty and non-empty optionals") {
        optional<int> empty;
        optional<int> has_value{42};

        WHEN("using spaceship operator") {
            THEN("empty should be less than non-empty") {
                REQUIRE((empty <=> has_value) == std::strong_ordering::less);
                REQUIRE((has_value <=> empty) == std::strong_ordering::greater);
            }
        }

        WHEN("comparing two empty optionals") {
            optional<int> empty2;
            THEN("they should be equal") {
                REQUIRE((empty <=> empty2) == std::strong_ordering::equal);
            }
        }
    }

    GIVEN("optional and nullopt") {
        optional<int> empty;
        optional<int> has_value{42};

        WHEN("using spaceship with nullopt") {
            THEN("comparisons should work") {
                REQUIRE((empty <=> nullopt) == std::strong_ordering::equal);
                REQUIRE((has_value <=> nullopt) == std::strong_ordering::greater);
            }
        }
    }

    GIVEN("optional and raw value") {
        optional<int> opt{42};
        optional<int> empty;

        WHEN("using spaceship with values") {
            THEN("comparisons should work correctly") {
                REQUIRE((opt <=> 42) == std::strong_ordering::equal);
                REQUIRE((opt <=> 100) == std::strong_ordering::less);
                REQUIRE((opt <=> 10) == std::strong_ordering::greater);
                REQUIRE((empty <=> 42) == std::strong_ordering::less);
            }
        }
    }

    GIVEN("optionals with different comparable types") {
        optional<int> opt_int{42};
        optional<long> opt_long{42L};

        WHEN("using spaceship across types") {
            THEN("cross-type comparison should work") {
                REQUIRE((opt_int <=> opt_long) == std::strong_ordering::equal);
            }
        }
    }
}
#endif
