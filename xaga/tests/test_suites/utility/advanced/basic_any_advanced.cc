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
#pragma warning(disable : 26800 26439 26495)
#endif

#define test_tag "[rainy::utility::basic_any -- advanced usage]"

using namespace rainy;
using namespace rainy::utility;

struct three_member_structure {
    int member1;
    char member2;
    std::string member3;
};

SCENARIO("[value-cast]", test_tag) {
    GIVEN("a empty variable") {
        any a;
        WHEN("the variable is integer 10") {
            a.emplace<int>(10);
            THEN("check for different type cast") {
                REQUIRE_NOTHROW(a.convert<float>() == 1.0f);
                REQUIRE_NOTHROW(a.convert<double>() == 1.0);
                REQUIRE_NOTHROW(a.convert<std::int64_t>() == 10ll);
            }
        }
        WHEN("the variable is string") {
            AND_WHEN("use const char* literal") {
                a.emplace<const char *>("Hello World");
                THEN("cast to std::string_view") {
                    REQUIRE_NOTHROW(a.convert<std::string_view>() == std::string_view{"Hello World"});
                }
            }
            AND_WHEN("use const char*") {
                const char *str = "Hello World";
                a.emplace<const char *>(str);
                THEN("cast to std::string_view") {
                    REQUIRE_NOTHROW(a.convert<std::string_view>() == std::string_view{"Hello World"});
                }
            }
            AND_WHEN("use const char* from a constant-string") {
                a = "Hello World";
                THEN("cast to std::string_view") {
                    REQUIRE_NOTHROW(a.convert<std::string_view>() == std::string_view{"Hello World"});
                }
            }
            AND_WHEN("a is std::string") {
                std::string str = "Hello World";
                a.emplace<std::string &>(str);
                THEN("cast to std::string_view") {
                    REQUIRE_NOTHROW(a.convert<std::string_view>() == std::string_view{"Hello World"});
                }
                WHEN("cast to std::string_view by using value") {
                    a.emplace<std::string>("Hello World");
                    THEN("cast to std::string_view") {
                        REQUIRE_NOTHROW(a.convert<std::string_view>() == std::string_view{"Hello World"});
                    }
                }
            }
        }
    }
}

SCENARIO("[match/match_for]", test_tag) {
    GIVEN("a empty variable") {
        any a;
        WHEN("do a change to integer and do match") {
            a.emplace<int>(10);
            THEN("it should be match for int") {
                // clang-format off
                bool flag{false};
                a.match(
                    [&flag](int) /* int 分支 */ {
                        flag = true;
                    }, 
                    [](char) /* char分支可能会匹配，但是会因为优先级上，优先检查同类型再检查是否可能转换 */ { 
                        REQUIRE(false);
                    },
                    [](std::string) /* 不应该匹配的分支 */ { 
                        REQUIRE(false);
                    });
                // clang-format on
                REQUIRE(flag);
            }
            AND_THEN("receive a variable after do match to check") {
                // clang-format off
                bool flag{false};
                any match_ret = a.match(
                    [&flag](int value) /* int 分支 */ { 
                        flag = true; 
                        return value * 3;
                    },
                    [](char) /* char分支可能会匹配，但是会因为优先级上，优先检查同类型再检查是否可能转换 */ { 
                        REQUIRE(false); 
                    },
                    [](std::string) /* 不应该匹配的分支 */ { 
                        REQUIRE(false);
                    });
                // clang-format on
                REQUIRE(flag);
                AND_THEN("it should be a int variable") {
                    REQUIRE(match_ret.has_value());
                    REQUIRE(match_ret.type() == rainy_typeid(int));
                    REQUIRE(match_ret.as<int>() == 30);
                }
            }
        }
        AND_WHEN("do a change to const char* and try to match") {
            a = "Hello World";
            THEN("try to match std::string_view") {
                bool flag{false};
                a.match([&flag](std::string_view) { flag = true; });
                REQUIRE(flag);
            }
        }
        AND_WHEN("do default match") {
            THEN("use default_match to mark a default branch") {
                a = std::vector<int>{};
                bool flag{false};
                // clang-format off
                a.match(
                    [](int) {
                        REQUIRE(false);
                    },
                    [](char) { 
                        REQUIRE(false); 
                    },
                    [](std::string) {
                        REQUIRE(false); 
                    },
                    [&flag](any::default_match) /* 默认匹配的分支 */{
                        flag = true;
                    });
                // clang-format on
                REQUIRE(flag);
            }
            AND_THEN("make sure that no default_match branch for invoke") {
                bool flag{true};
                // clang-format off
                a.match(
                [&](int) {
                    flag = false; 
                },
                [&](char) { 
                    flag = false; 
                },
                [&](std::string) {
                    flag = false; 
                });
                // clang-format on
                THEN("check for flag") {
                    REQUIRE(flag);
                }
            }
        }
    }
    AND_GIVEN("a empty variable and some variable for test") {
        any a;
        int value = 42;
        WHEN("a is integer") {
            a.emplace<int>(10);
            THEN("we do match_for without auto_deduce flag") {
                auto variant = a.match_for<std::string, int>([](int value) { return std::to_string(value); },
                                                             [](const std::string &value) { return std::stoi(value); });
                static_assert(std::is_same_v<decltype(variant), std::variant<std::string, int>>, "Compile Time Test failed!");
                std::visit(
                    [](auto &&val) {
                        if constexpr (std::is_same_v<rainy::type_traits::cv_modify::remove_cvref_t<decltype(val)>, std::string>) {
                            REQUIRE(val == "10");
                        } else {
                            REQUIRE(false);
                        }
                    },
                    variant);
            }
            AND_THEN("we use auto_deduce flag to do match_for") {
                // clang-format off
                auto variant = a.match_for(
                    auto_deduce,
                    [](int value) -> std::string { 
                        return std::to_string(value);
                    },
                    [](const std::string &value) -> int { 
                        return std::stoi(value); 
                    });
                // clang-format on
                static_assert(std::is_same_v<decltype(variant), std::variant<std::string, int>>, "Compile Time Test failed!");
                std::visit(
                    [](auto &&val) {
                        if constexpr (std::is_same_v<rainy::type_traits::cv_modify::remove_cvref_t<decltype(val)>, std::string>) {
                            REQUIRE(val == "10");
                        } else {
                            REQUIRE(false);
                        }
                    },
                    variant);
            }
        }
        WHEN("a is reference") {
            a.emplace<int &>(value);
            THEN("we do match_for without auto_deduce flag") {
                // clang-format off
                auto variant = a.match_for<int, std::string>(
                    [](int value) { 
                        return value * 100; 
                    },
                    [](const std::string &value) { return value; }
                );
                // clang-format on
                static_assert(std::is_same_v<decltype(variant), std::variant<int, std::string>>, "Compile Time Test failed!");
                std::visit(
                    [](auto &&val) {
                        if constexpr (std::is_same_v<rainy::type_traits::cv_modify::remove_cvref_t<decltype(val)>, int>) {
                            REQUIRE(val == 4200);
                        } else {
                            REQUIRE(false);
                        }
                    },
                    variant);
            }
            AND_THEN("we do match_for without auto_deduce flag") {
                // clang-format off
                auto variant = a.match_for(
                    auto_deduce,
                    [](int value) { 
                        return value * 100; 
                    },
                    [](const std::string &value) { return value; }
                );
                // clang-format on
                static_assert(std::is_same_v<decltype(variant), std::variant<int, std::string>>, "Compile Time Test failed!");
                std::visit(
                    [](auto &&val) {
                        if constexpr (std::is_same_v<rainy::type_traits::cv_modify::remove_cvref_t<decltype(val)>, int>) {
                            REQUIRE(val == 4200);
                        } else {
                            REQUIRE(false);
                        }
                    },
                    variant);
            }
        }
    }
}

SCENARIO("[destructure]", test_tag) {
    GIVEN("a empty variable") {
        any a;
        WHEN("a is a three member structure") {
            a = three_member_structure{42, 'c', "Hello World"};
            THEN("we use lambda handler to make destructure") {
                a.destructure([](int member1, char member2, const std::string &member3) {
                    REQUIRE(member1 == 42);
                    REQUIRE(member2 == 'c');
                    REQUIRE(member3 == "Hello World");
                });
            }
            AND_THEN("use a another structure to receive the result") {
                struct receiver_t {
                    int m1;
                    char m2;
                    std::string_view m3;
                };
                receiver_t receiver;
                a.destructure(receiver);
                REQUIRE(receiver.m1 == 42);
                REQUIRE(receiver.m2 == 'c');
                REQUIRE(receiver.m3 == "Hello World");
            }
            AND_THEN("we use placeholder_t tag to ingnore some member pass") {
                a.destructure([](int member1, char member2, utility::placeholder_t) {
                    REQUIRE(member1 == 42);
                    REQUIRE(member2 == 'c');
                });
            }
        }
        AND_WHEN("a is std::pair<int, std::string>") {
            a.emplace<std::pair<int, std::string>>(42, "Hello World");
            THEN("use std::pair and rainy::utility::pair to receive the result") {
                std::pair<int, std::string_view> std_pair;
                utility::pair<int, std::string_view> utility_pair;
                a.destructure(std_pair);
                a.destructure(utility_pair);
                {
                    CHECK(std_pair.first == 42);
                    CHECK(std_pair.second == "Hello World");
                }
                {
                    CHECK(utility_pair.first == 42);
                    CHECK(utility_pair.second == "Hello World");
                }
            }
            AND_THEN("and then we use placeholder_t tag to ingnore some member pass") {
                std::pair<int, utility::placeholder_t> std_pair;
                utility::pair<int, utility::placeholder_t> utility_pair;
                a.destructure(std_pair);
                a.destructure(utility_pair);
                {
                    CHECK(std_pair.first == 42);
                    CHECK(utility_pair.first == 42);
                }
            }
        }
        AND_WHEN("a is std::tuple") {
            a = std::make_tuple(42, 'c', "Hello World", std::vector<int>{1, 2, 3, 4, 5});
            auto tuple = std::make_tuple(42, 'c', "Hello World", std::vector<int>{1, 2, 3, 4, 5});
            utility::member_count_v<type_traits::cv_modify::remove_cvref_t<decltype(tuple)>>;
            THEN("use a another structure to receive the result") {
                struct receiver_t {
                    int m1;
                    char m2;
                    std::string_view m3;
                    std::vector<int> m4;
                };
                receiver_t receiver;
                a.destructure(receiver);
                {
                    REQUIRE(receiver.m1 == 42);
                    REQUIRE(receiver.m2 == 'c');
                    REQUIRE(receiver.m3 == "Hello World");
                    REQUIRE(receiver.m4 == std::vector<int>{1, 2, 3, 4, 5});
                }
            }
            AND_THEN("use struture binding like api to destructure") {
                auto&& [m1, m2, m3, m4] = a.destructure<int, char, std::string_view, std::vector<int>>();
                {
                    REQUIRE(m1 == 42);
                    REQUIRE(m2 == 'c');
                    REQUIRE(m3 == "Hello World");
                    REQUIRE(m4 == std::vector<int>{1, 2, 3, 4, 5});
                }
            }
        }
    }
}

SCENARIO("[transform]", test_tag) {
    GIVEN("a empty variable") {
        any a;
        WHEN("a is int reference") {
            int integer = 42;
            a.emplace<int &>(integer);
            THEN("try to transform to a double") {
                auto &this_from_a = a.transform<double>();
                REQUIRE(utility::addressof(this_from_a) == utility::addressof(a));
                REQUIRE(this_from_a.type() == rainy_typeid(double));
                REQUIRE(this_from_a.as<double>() == 42.0);
            }
            THEN("try to transform to a error_type") {
                auto &this_from_a = a.transform<std::string_view>();
                REQUIRE(utility::addressof(this_from_a) == utility::addressof(a));
                REQUIRE(this_from_a.type() == rainy_typeid(int &));
            }
            THEN("use a handler to transform") {
                auto &this_from_a = a.transform([](const int &value) -> std::string { return std::to_string(value); });
                REQUIRE(utility::addressof(this_from_a) == utility::addressof(a));
                REQUIRE(this_from_a.type() == rainy_typeid(std::string));
                AND_THEN("try to convert std::string_view") {
                    REQUIRE(this_from_a.convert<std::string_view>() == "42");
                }
            }
        }
        AND_WHEN("a is std::string") {
            a.emplace<std::string>("Hello World");
            THEN("try to transform to a error_type") {
                auto &this_from_a = a.transform<double>();
                REQUIRE(utility::addressof(this_from_a) == utility::addressof(a));
                REQUIRE(this_from_a.type() == rainy_typeid(std::string));
            }
            THEN("try to transform to a std::string_view") {
                auto &this_from_a = a.transform<std::string_view>();
                REQUIRE(utility::addressof(this_from_a) == utility::addressof(a));
                REQUIRE(this_from_a.type() == rainy_typeid(std::string));
                REQUIRE(this_from_a.convert<std::string_view>() == "Hello World");
            }
            THEN("use a handler to transform") {
                auto &this_from_a = a.transform([](const std::string &value) -> int { return std::stoi(value); });
                REQUIRE(utility::addressof(this_from_a) == utility::addressof(a));
                REQUIRE(this_from_a.type() == rainy_typeid(int));
                AND_THEN("try to convert double") {
                    REQUIRE(this_from_a.convert<double>() == 42.0);
                }
            }
        }
    }
}
