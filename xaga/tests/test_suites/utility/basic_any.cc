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
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <rainy/utility/any.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26800 26439)
#endif

#define test_tag "[rainy::utility::basic_any]"

using namespace rainy::utility;

using tuple_t = std::tuple<std::string, std::vector<char>, int>;

struct error_construct_type {
    error_construct_type() {
        throw std::runtime_error("This is a error");
    }

    error_construct_type(int) {
    }

    error_construct_type(const error_construct_type &) {
        throw std::runtime_error("This is a copy error");
    }
};

struct raii_resource {
    raii_resource(bool *has_release, in<std::function<void()>> callback) : has_release{has_release}, callback{callback} {
        // get resources...
    }

    ~raii_resource() {
        // release resources...
        release_resouce();
    }

    void release_resouce() {
        if (*has_release) {
            callback();
        } else {
            *has_release = true;
        }
    }

    bool *has_release{nullptr};
    std::function<void()> callback;
};

SCENARIO("[lifetime]", test_tag) {
    GIVEN("one any variable") {
        any variable{};
        WHEN("construct a value int") {
            construct_in_place(variable, 10);
            THEN("is success for construct value type") {
                REQUIRE(variable.has_value());
                REQUIRE(variable.has_ownership());
            }
            variable.reset();
        }
        WHEN("construct reference int") {
            int integer = 123;
            construct_in_place(variable, std::in_place_type<int &>, integer);
            THEN("is success for construct reference type") {
                REQUIRE(variable.has_value());
                REQUIRE_FALSE(variable.has_ownership());
            }
            variable.reset();
        }
        WHEN("clear this variable") {
            bool status;
            class resource {
            public:
                resource(bool &status) : status{status} {
                    status = false;
                }

                ~resource() {
                    status = true;
                }

            private:
                bool &status;
            };
            static_assert(!std::is_trivial_v<resource>, "Error resource type");
            construct_in_place(variable, std::in_place_type<resource>, status);
            // do something...
            variable.reset();
            THEN("is success for release resource") {
                REQUIRE(status == true);
            }
        }
    }
    GIVEN("two any variable") {
        any variable{10};
        any variable_tuple{tuple_t{"Hello World", {'H', 'e', 'l', 'l', 'o'}, 10}};
        WHEN("we swap two variable") {
            std::swap(variable, variable_tuple);
            THEN("variable_tuple is empty and variable has value") {
                REQUIRE((variable_tuple.has_value() && variable_tuple.is<int>()));
                REQUIRE((variable.has_value() && variable.is<tuple_t>()));
            }
        }
        WHEN("we copy variable") {
            any copy_variable = variable;
            THEN("is success for copy") {
                REQUIRE(copy_variable.has_value());
                REQUIRE(variable.has_value());
            }
        }
        WHEN("we move a variable") {
            any move_variable = std::move(variable);
            THEN("is success for move") {
                REQUIRE(move_variable.has_value());
                REQUIRE(variable.has_value()); // 修正，variable的行为应为被移动后仍可用
            }
        }
    }
}

SCENARIO("[construct]", test_tag) {
    GIVEN("one any variable") {
        any variable{};
        WHEN("we construct int") {
            construct_in_place(variable, std::in_place_type<int>, 10);
            THEN("construct result is valid") {
                REQUIRE(variable.has_value());
                REQUIRE(variable.as<int>() == 10);
            }
            AND_THEN("we'll clean this variable") {
                variable.reset();
                THEN("variable is no longer valid") {
                    REQUIRE_FALSE(variable.has_value());
                }
            }
        }
        WHEN("we construct std::string") {
            construct_in_place(variable, std::in_place_type<std::string>, "Hello World");
            THEN("construct result is valid") {
                REQUIRE(variable.has_value());
                REQUIRE(variable.as<const std::string &>() == "Hello World");
            }
            AND_THEN("we'll clean this variable") {
                variable.reset();
                WHEN("variable is no longer valid") {
                    REQUIRE_FALSE(variable.has_value());
                }
            }
        }
        GIVEN("a integer variable") {
            int integer{42};
            WHEN("we construct int reference") {
                construct_in_place(variable, std::in_place_type<int &>, integer);
                THEN("construct result is valid") {
                    REQUIRE(variable.has_value());
                    REQUIRE(std::addressof(variable.as<int>()) == std::addressof(integer));
                }
                AND_THEN("we'll clean this variable") {
                    variable.reset();
                    THEN("variable is no longer valid") {
                        REQUIRE_FALSE(variable.has_value());
                    }
                    AND_THEN("reference variable is valid") {
                        REQUIRE(integer == 42);
                    }
                }
            }
        }
        GIVEN("a string variable") {
            std::string reference_string = "Hello World";
            WHEN("we construct std::string reference") {
                construct_in_place(variable, std::in_place_type<std::string &>, reference_string);
                THEN("construct result is valid") {
                    REQUIRE(variable.has_value());
                    REQUIRE(std::addressof(variable.as<const std::string &>()) == std::addressof(reference_string));
                }
                AND_THEN("we'll clean this variable") {
                    variable.reset();
                    THEN("variable is no longer valid") {
                        REQUIRE_FALSE(variable.has_value());
                    }
                    AND_THEN("reference variable is valid") {
                        REQUIRE_FALSE(reference_string.empty());
                    }
                }
            }
            AND_WHEN("we use rvalue reference to construct") {
                construct_in_place(variable, std::in_place_type<std::string &&>, std::move(reference_string));
                WHEN("variable has reference") {
                    REQUIRE_FALSE(variable.has_ownership());
                }
                GIVEN("a empty string") {
                    std::string str{};
                    WHEN("use as<Ty&&>() to construct this string") {
                        auto &&ref = std::move(variable.as<std::string &&>());
                        str = std::move(ref);
                        THEN("reference_string is empty") {
                            REQUIRE_FALSE(str.empty());
                            REQUIRE(reference_string.empty());
                        }
                        AND_THEN("we'll clean this variable") {
                            variable.reset();
                            THEN("variable is no longer valid") {
                                REQUIRE_FALSE(variable.has_value());
                            }
                            AND_THEN("reference variable is no longer valid") {
                                REQUIRE(reference_string.empty());
                            }
                        }
                    }
                }
            }
        }
    }
    GIVEN("two any variable") {
        any variable_one{tuple_t{"Hello", {'H', 'e', 'l', 'l', 'o'}, 42}};
        any variable_two{};
        WHEN("use copy semantics to construct variable") {
            THEN("use copy semantics to construct variable") {
                construct_in_place(variable_two, variable_one);
                THEN("check is copy") {
                    REQUIRE(variable_two.has_value());
                    REQUIRE(variable_one.has_value());
                }
            }
        }
        WHEN("use move semantics to construct variable") {
            construct_in_place(variable_two, std::move(variable_one));
            THEN("check is move semantics") {
                REQUIRE(variable_two.has_value());
                REQUIRE_FALSE(variable_one.has_value());
            }
        }
    }
    GIVEN("a might be corrupt variable") {
        any could_corrupt_variable{std::in_place_type<error_construct_type>, 42};
        WHEN("we try to make a copy error") {
            GIVEN("a corrupt_variable") {
                any corrupt_variable{};
                THEN("it should be throw std::runtime_error") {
                    REQUIRE_THROWS_MATCHES((corrupt_variable = could_corrupt_variable, false), std::runtime_error,
                                           Catch::Matchers::Message("This is a copy error"));
                    REQUIRE_FALSE(corrupt_variable.has_value());
                }
            }
        }
        WHEN("we try to make a construct error") {
            GIVEN("a corrupt_variable") {
                any corrupt_variable;
                THEN("it should be throw std::runtime_error") {
                    REQUIRE_THROWS_MATCHES((corrupt_variable.emplace<error_construct_type>(), false), std::runtime_error,
                                           Catch::Matchers::Message("This is a error"));
                    REQUIRE_FALSE(corrupt_variable.has_value());
                }
            }
        }
    }
}

SCENARIO("[type-inspect]", test_tag) {
    GIVEN("a empty variable") {
        any variable{};
        WHEN("emplace integer") {
            variable.emplace<int>(42);
            THEN("check the type is int") {
                REQUIRE(variable.is<int>());
                REQUIRE_FALSE(variable.is<char>());
                REQUIRE_FALSE(variable.is<std::int64_t>());
                REQUIRE_FALSE(variable.is<std::size_t>());
            }
            THEN("check same type reference convert") {
                REQUIRE(variable.is_convertible<int &>());
                REQUIRE(variable.is_convertible<int &&>());
            }
            AND_THEN("make sure int variable won't convert to reference of char") {
                REQUIRE_FALSE(variable.is_convertible<char &>());
                REQUIRE_FALSE(variable.is_convertible<char &&>());
            }
        }
        WHEN("emplace a char type") {
            variable.emplace<char>('c');
            THEN("check the type from char to others type") {
                REQUIRE(variable.is_convertible<int>());
                REQUIRE(variable.is_convertible<char>());
                REQUIRE(variable.is_convertible<std::int64_t>());
            }
            THEN("check char type won't convert to std::string") {
                REQUIRE_FALSE(variable.is_convertible<std::string>());
            }
        }
        WHEN("emplace a std::string") {
            variable.emplace<std::string>("Hello World");
            THEN("check the type from std::string to std::string_view") {
                REQUIRE(variable.is_convertible<std::string_view>());
            }
        }
    }
    GIVEN("a empty variable for multi-type-insepect and a test_func") {
        any variable{};
        static auto is_one_of = [&variable]() -> bool { return variable.is_one_of<int, int &, std::vector<char>>(); };
        static auto is_one_of_convertible = [&variable]() -> bool {
            return variable.is_one_of_convertible<int, int &, std::vector<char>>();
        };
        WHEN("variable is int") {
            variable.emplace<int>(42);
            THEN("check the type with is_one_of") {
                REQUIRE(is_one_of());
            }
            THEN("check the type with is_one_of_convertible") {
                REQUIRE(is_one_of_convertible());
            }
        }
        GIVEN("a integer for reference") {
            int integer{42};
            WHEN("variable is int reference") {
                variable.emplace<int &>(integer);
                THEN("check the type with is_one_of") {
                    REQUIRE(is_one_of());
                }
                THEN("check the type with is_one_of_convertible") {
                    REQUIRE(is_one_of_convertible());
                }
            }
        }
        WHEN("variable is std::vector<T>") {
            WHEN("T is char") {
                variable.emplace<std::vector<char>>();
                THEN("check the type with is_one_of") {
                    REQUIRE(is_one_of());
                }
                THEN("check the type with is_one_of_convertible") {
                    REQUIRE(is_one_of_convertible());
                }
            }
            AND_WHEN("T is int") {
                variable.emplace<std::vector<int>>();
                THEN("check the type with is_one_of") {
                    REQUIRE_FALSE(is_one_of());
                }
                THEN("check the type with is_one_of_convertible") {
                    REQUIRE_FALSE(is_one_of_convertible());
                }
            }
        }
    }
}

SCENARIO("[swap]", test_tag) {
    using tuple_t = std::tuple<std::string, std::vector<char>, int>;
    GIVEN("two variable") {
        any variable1{};
        any variable2{};
        WHEN("var1 gives 42 and var2 gives 'c'") {
            variable1 = 42;
            variable2 = 'c';
            WHEN("swap var1 and var2") {
                swap(variable1, variable2);
                THEN("var1 should be 'c' and var2 should be 42") {
                    REQUIRE((variable1.is<char>() && variable2.is<int>()));
                    REQUIRE(variable1.as<char>() == 'c');
                    REQUIRE(variable2.as<int>() == 42);
                }
            }
        }
        WHEN("var1 and var2 is tuple") {
            variable1 = tuple_t{"Hello", {'H', 'e', 'l', 'l', 'o'}, 5};
            variable2 = tuple_t{"The world", {'T', 'h', 'e', ' ', 'w', 'o', 'r', 'l', 'd'}, 9};
            WHEN("swap var1 and var2") {
                swap(variable1, variable2);
                THEN("check var1 and var2 contents") {
                    const auto &tuple1 = variable1.as<tuple_t>();
                    const auto &tuple2 = variable2.as<tuple_t>();
                    {
                        REQUIRE_THAT(std::get<0>(tuple1), Catch::Matchers::Equals("The world"));
                        REQUIRE_THAT(std::get<0>(tuple2), Catch::Matchers::Equals("Hello"));
                    }
                    {
                        REQUIRE_THAT(std::get<1>(tuple1),
                                     Catch::Matchers::UnorderedEquals(std::vector<char>{'T', 'h', 'e', ' ', 'w', 'o', 'r', 'l', 'd'}));
                        REQUIRE_THAT(std::get<1>(tuple2),
                                     Catch::Matchers::UnorderedEquals(std::vector<char>{'H', 'e', 'l', 'l', 'o'}));
                    }
                    {
                        REQUIRE(std::get<2>(tuple1) == 9);
                        REQUIRE(std::get<2>(tuple2) == 5);
                    }
                }
            }
        }
    }
}

SCENARIO("[ownership-check]", test_tag) {
    GIVEN("a double release flag and a trigger func") {
        bool has_cause_double_release{false};
        auto double_release_fn = [&has_cause_double_release]() { has_cause_double_release = true; };
        GIVEN("a release flag checker and a variable") {
            bool has_release{false};
            any variable{};
            WHEN("we use value construct") {
                THEN("construct variable") {
                    variable.emplace<raii_resource>(&has_release, double_release_fn);
                    THEN("check has ownership") {
                        REQUIRE(variable.has_ownership());
                    }
                    AND_THEN("making release") {
                        variable.as<raii_resource>().release_resouce();
                        variable.reset();
                        THEN("check has release") {
                            REQUIRE(has_release);
                        }
                        THEN("check has double release") {
                            REQUIRE(has_cause_double_release);
                        }
                    }
                }
            }
            WHEN("we use reference construct") {
                GIVEN("a resource") {
                    raii_resource resource{&has_release, double_release_fn};
                    variable.emplace<raii_resource &>(resource);
                    THEN("check variable not had ownership") {
                        REQUIRE_FALSE(variable.has_ownership());
                    }
                    AND_THEN("making release") {
                        resource.release_resouce();
                        variable.reset();
                        THEN("check has release") {
                            REQUIRE(has_release);
                        }
                        THEN("check has no double release") {
                            REQUIRE_FALSE(has_cause_double_release);
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("[emplace]", test_tag) {
    GIVEN("a variable") {
        any variable{'c'};
        WHEN("for value emplace construct") {
            auto &val = variable.emplace<int>(10);
            THEN("test for type is int") {
                REQUIRE(val == 10);
                REQUIRE(variable.is<int>());
            }
        }
        GIVEN("a integer for reference") {
            int integer = 42;
            WHEN("for lvalue-reference construct") {
                auto &val = variable.emplace<int &>(integer);
                REQUIRE(variable.type().is_reference());
                REQUIRE(variable.type().is_lvalue_reference());
                REQUIRE(std::addressof(val) == std::addressof(integer));
            }
            WHEN("for rvalue-reference construct") {
                auto &&val = variable.emplace<int &&>(std::move(integer));
                REQUIRE(variable.type().is_reference());
                REQUIRE(variable.type().is_rvalue_reference());
                REQUIRE(std::addressof(val) == std::addressof(integer));
            }
        }
    }
}

SCENARIO("[as-cast]", test_tag) {
    GIVEN("a variable with integer") {
        any variable{42};
        WHEN("try to cast int") {
            THEN("checking for cast") {
                REQUIRE_NOTHROW(variable.as<int>() == 42);
            }
            AND_THEN("checking for error-cast") {
                REQUIRE_THROWS_AS(variable.as<char>(), rainy::foundation::exceptions::cast::bad_any_cast);
            }
        }
    }
    GIVEN("a variable with a integer for reference") {
        int integer{42};
        any variable{std::in_place_type<const int &>, integer};
        WHEN("try to cast int") {
            THEN("checking for cast") {
                REQUIRE_NOTHROW(variable.as<const int>() == 42); // 仅允许在引用对象时以const形式
            }
            AND_THEN("checking for error-cast") {
                REQUIRE_THROWS_AS(variable.as<int>(), rainy::foundation::exceptions::cast::bad_any_cast);
                REQUIRE_THROWS_AS(variable.as<char>(), rainy::foundation::exceptions::cast::bad_any_cast);
            }
        }
        WHEN("variable use int& not const int&") {
            variable.emplace<int &>(integer);
            THEN("checking for cast") {
                REQUIRE_NOTHROW(variable.as<int>() == 42);
                REQUIRE_NOTHROW(variable.as<volatile int>() == 42);
                REQUIRE_NOTHROW(variable.as<const int>() == 42);
            }
            AND_THEN("checking for error-cast") {
                REQUIRE_THROWS_AS(variable.as<char>(), rainy::foundation::exceptions::cast::bad_any_cast);
            }
        }
    }
}

SCENARIO("[convert-cast]", test_tag) {
    GIVEN("a variable with integer") {
        any variable{42};
        WHEN("convert using reference cast") {
            REQUIRE_NOTHROW(variable.convert<int &>() == 42);
            REQUIRE_NOTHROW(variable.convert<volatile int &>() == 42);
            REQUIRE_NOTHROW(variable.convert<const int &>() == 42);
            REQUIRE_NOTHROW(variable.convert<const volatile int &>() == 100);
        }
        WHEN("convert using value cast") {
            THEN("chcecking for value-cast when type is int") {
                REQUIRE_NOTHROW(variable.convert<double>() == 42.);
                REQUIRE_NOTHROW(variable.convert<float>() == 42.f);
                REQUIRE_NOTHROW(variable.convert<std::int64_t>() == 42ll);
            }
            WHEN("type is std::size_t") {
                THEN("checking for overflow value-cast when") {
                    variable.emplace<std::size_t>(-1);
                    REQUIRE_NOTHROW(variable.convert<int>() == -1);
                }
            }
        }
    }
    GIVEN("some variable for string_view value-cast") {
        any variable{};
        const char *mystring = "Hello World";
        std::string string = "Hello World";
        WHEN("use const char*") {
            WHEN("use literal") {
                {
                    variable = "Hello World";
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
                {
                    variable.emplace<const char *>("Hello World");
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
                {
                    variable = {std::in_place_type<const char *>, "Hello World"};
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
            }
            WHEN("use reference") {
                {
                    variable.emplace<const char *&>(mystring);
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
                {
                    auto &reference = mystring;
                    variable.emplace<const char *&>(reference);
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
                {
                    const auto &reference = mystring;
                    variable.emplace<const char *const &>(reference);
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
            }
            WHEN("use std::string") {
                {
                    variable.emplace<std::string>("Hello World");
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
                {
                    auto &reference = string;
                    variable.emplace<std::string &>(reference);
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
                {
                    const auto &reference = string;
                    variable.emplace<const std::string &>(reference);
                    REQUIRE(variable.convert<std::string_view>() == "Hello World");
                }
            }
        }
    }
}

SCENARIO("[target_as_void_ptr]", test_tag) {
    GIVEN("a empty variable and some variable for test") {
        int integer{42};
        char str[] = "Hello World";
        any variable{};
        WHEN("variable is int reference") {
            variable.emplace<int &>(integer);
            THEN("get target_as_void_ptr() for check") {
                REQUIRE(variable.target_as_void_ptr() == &integer);
                REQUIRE(*static_cast<const int *>(variable.target_as_void_ptr()) == 42);
            }
        }
        WHEN("variable is int") {
            variable.emplace<int>(42);
            THEN("get target_as_void_ptr() for check") {
                REQUIRE(*static_cast<const int *>(variable.target_as_void_ptr()) == 42);
            }
        }
        WHEN("variable is const char* as reference") {
            variable.emplace<const char *>(str);
            THEN("get target_as_void_ptr() for check") {
                REQUIRE(std::char_traits<char>::compare(*static_cast<const char *const *>(variable.target_as_void_ptr()),
                                                        "Hello World", sizeof(str) - 1) == 0);
            }
        }
    }
}

SCENARIO("[hash_code]", test_tag) {
    GIVEN("a empty variable") {
        any variable{};
        WHEN("variable is 42") {
            variable.emplace<int>(42);
            THEN("check by using std::hash") {
                REQUIRE(std::hash<any>{}(variable) == std::hash<int>{}(42));
            }
            THEN("check by using hash_code() method") {
                REQUIRE(variable.hash_code() == std::hash<int>{}(42));
            }
        }
        WHEN("variable is 3.14f") {
            variable.emplace<float>(3.14f);
            THEN("check by using std::hash") {
                REQUIRE(std::hash<any>{}(variable) == std::hash<float>{}(3.14f));
            }
            THEN("check by using hash_code() method") {
                REQUIRE(variable.hash_code() == std::hash<float>{}(3.14f));
            }
        }
        WHEN("variable is 3.14") {
            variable.emplace<double>(3.14);
            THEN("check by using std::hash") {
                REQUIRE(std::hash<any>{}(variable) == std::hash<double>{}(3.14));
            }
            THEN("check by using hash_code() method") {
                REQUIRE(variable.hash_code() == std::hash<double>{}(3.14));
            }
        }
        WHEN("variable is string") {
            variable.emplace<std::string>("Hello World");
            THEN("check by using std::hash") {
                REQUIRE(std::hash<any>{}(variable) == std::hash<std::string>{}("Hello World"));
            }
            THEN("check by using hash_code() method") {
                REQUIRE(variable.hash_code() == std::hash<std::string>{}("Hello World"));
            }
        }
        WHEN("variable is string view") {
            variable.emplace<std::string_view>("Hello World");
            THEN("check by using std::hash") {
                REQUIRE(std::hash<any>{}(variable) == std::hash<std::string_view>{}("Hello World"));
            }
            THEN("check by using hash_code() method") {
                REQUIRE(variable.hash_code() == std::hash<std::string_view>{}("Hello World"));
            }
        }
    }
}

SCENARIO("[cast_to_pointer]", test_tag) {
    GIVEN("a empty variable and some variable for test") {
        any variable;
        int integer{42};
        WHEN("variable is int") {
            variable = 10;
            THEN("check cast_to_pointer by using value-type") {
                int *int_ptr = variable.cast_to_pointer<int>();
                REQUIRE(*int_ptr == 10);
            }
            THEN("check cast_to_pointer by using lvalue-ref-type") {
                int *int_ptr = variable.cast_to_pointer<int &>();
                REQUIRE(*int_ptr == 10);
            }
            THEN("check cast_to_pointer by using rvalue-ref-type") {
                int *int_ptr = variable.cast_to_pointer<int>();
                REQUIRE(*int_ptr == 10);
            }
        }
        WHEN("variable is int reference") {
            variable.emplace<int &>(integer);
            THEN("check cast_to_pointer by using value-type") {
                int *int_ptr = variable.cast_to_pointer<int>();
                REQUIRE(*int_ptr == integer);
            }
            THEN("check cast_to_pointer by using lvalue-ref-type") {
                int *int_ptr = variable.cast_to_pointer<int &>();
                REQUIRE(*int_ptr == integer);
            }
            THEN("check cast_to_pointer by using rvalue-ref-type") {
                int *int_ptr = variable.cast_to_pointer<int>();
                REQUIRE(*int_ptr == integer);
            }
        }
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
