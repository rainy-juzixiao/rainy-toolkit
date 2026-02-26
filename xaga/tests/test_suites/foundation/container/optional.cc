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
