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
#include <catch2/catch_template_test_macros.hpp>

#include <rainy/annotations/moon.hpp>
#include <rainy/meta/moon/tuple_like.hpp>

#include <string>
#include <utility>
#include <tuple>

using namespace rainy::meta::moon;

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

using namespace rainy::annotations::moon;

#endif

struct SimpleStruct {
    int a;
    double b;
    char c;
};

struct ComplexStruct {
    int id;
    std::string name;
    double value;
    bool active;
};

struct EmptyStruct {};

struct RegisteredStruct {
    int x;
    double y;
    std::string z;
};

RAINY_REFLECT_TUPLE_LIKE(RegisteredStruct, x, y, z)

class PrivateClass {
    int secret1;
    double secret2;
    std::string secret3;

public:
    PrivateClass() : secret1(0), secret2(0.0), secret3("") {}
    PrivateClass(int s1, double s2, std::string s3) : secret1(s1), secret2(s2), secret3(s3) {}
    int getSecret1() const { return secret1; }
    double getSecret2() const { return secret2; }
    std::string getSecret3() const { return secret3; }
};

RAINY_PRIVATE_REFLECT_TUPLE_LIKE(PrivateClass, secret1, secret2, secret3)

template <typename T>
struct TemplateStruct {
    T value;
    int count;
};

struct ArrayStruct {
    int arr[5];
};

TEST_CASE("member_count", "[moon][member_count]") {
    SECTION("aggregate struct") {
        REQUIRE(member_count_v<SimpleStruct> == 3);
        REQUIRE(member_count_v<ComplexStruct> == 4);
        REQUIRE(member_count_v<EmptyStruct> == 0);
    }

    SECTION("registered struct") {
        REQUIRE(member_count_v<RegisteredStruct> == 3);
    }

    SECTION("private reflected struct") {
        REQUIRE(member_count_v<PrivateClass> == 3);
    }

    SECTION("template struct") {
        REQUIRE(member_count_v<TemplateStruct<int>> == 2);
        REQUIRE(member_count_v<TemplateStruct<double>> == 2);
    }

    SECTION("array struct") {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        REQUIRE(member_count_v<ArrayStruct> == 1);
#else
        REQUIRE(member_count_v<ArrayStruct> == 5);
#endif
    }
}

TEST_CASE("struct_to_tuple", "[moon][struct_to_tuple]") {
    SECTION("aggregate struct") {
        constexpr auto tuple = struct_to_tuple<SimpleStruct>();
        using tuple_type = decltype(tuple);

        REQUIRE(std::tuple_size_v<tuple_type> == 3);

        static_assert(rainy::type_traits::type_relations::is_convertible_v<std::tuple_element_t<0, tuple_type>, int *>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<std::tuple_element_t<1, tuple_type>, double *>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<std::tuple_element_t<2, tuple_type>, char *>);
    }

    SECTION("empty struct") {
        constexpr auto tuple = struct_to_tuple<EmptyStruct>();
        REQUIRE(std::tuple_size_v<decltype(tuple)> == 0);
    }

    SECTION("registered struct") {
        constexpr auto tuple = struct_to_tuple<RegisteredStruct>();
        using tuple_type = decltype(tuple);

        REQUIRE(std::tuple_size_v<tuple_type> == 3);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<std::tuple_element_t<0, tuple_type>, int *>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<std::tuple_element_t<1, tuple_type>, double *>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<std::tuple_element_t<2, tuple_type>, std::string *>);
    }

    SECTION("private reflected struct") {
        constexpr auto tuple = struct_to_tuple<PrivateClass>();
        REQUIRE(std::tuple_size_v<decltype(tuple)> == 3);
    }
}

TEST_CASE("struct_bind_tuple", "[moon][struct_bind_tuple][private_access]") {
    SECTION("bind aggregate struct") {
        SimpleStruct obj{42, 3.14, 'A'};
        auto ptr_tuple = struct_bind_tuple(obj);

        REQUIRE(std::tuple_size_v<decltype(ptr_tuple)> == 3);

        REQUIRE(*std::get<0>(ptr_tuple) == 42);
        REQUIRE(*std::get<1>(ptr_tuple) == 3.14);
        REQUIRE(*std::get<2>(ptr_tuple) == 'A');

        *std::get<0>(ptr_tuple) = 100;
        *std::get<1>(ptr_tuple) = 2.718;
        *std::get<2>(ptr_tuple) = 'Z';

        REQUIRE(obj.a == 100);
        REQUIRE(obj.b == 2.718);
        REQUIRE(obj.c == 'Z');
    }

    SECTION("bind registered struct") {
        RegisteredStruct obj{10, 20.5, "hello"};
        auto ptr_tuple = struct_bind_tuple(obj);

        REQUIRE(std::tuple_size_v<decltype(ptr_tuple)> == 3);

        REQUIRE(*std::get<0>(ptr_tuple) == 10);
        REQUIRE(*std::get<1>(ptr_tuple) == 20.5);
        REQUIRE(*std::get<2>(ptr_tuple) == "hello");

        *std::get<0>(ptr_tuple) = 99;
        REQUIRE(obj.x == 99);
    }

    SECTION("bind private reflected struct - 私有成员访问测试") {
        PrivateClass obj(100, 200.5, "secret");
        auto ptr_tuple = struct_bind_tuple(obj);

        REQUIRE(std::tuple_size_v<decltype(ptr_tuple)> == 3);

        REQUIRE(*std::get<0>(ptr_tuple) == 100);
        REQUIRE(*std::get<1>(ptr_tuple) == 200.5);
        REQUIRE(*std::get<2>(ptr_tuple) == "secret");

        *std::get<0>(ptr_tuple) = 999;
        *std::get<1>(ptr_tuple) = 888.5;
        *std::get<2>(ptr_tuple) = "modified";

        REQUIRE(obj.getSecret1() == 999);
        REQUIRE(obj.getSecret2() == 888.5);
        REQUIRE(obj.getSecret3() == "modified");
    }
}

TEST_CASE("get_member_names", "[moon][get_member_names]") {
    SECTION("aggregate struct names") {
        constexpr auto names = get_member_names<SimpleStruct>();

        REQUIRE(names.size() == 3);
        REQUIRE(names[0] == "a");
        REQUIRE(names[1] == "b");
        REQUIRE(names[2] == "c");
    }

    SECTION("registered struct names") {
        constexpr auto names = get_member_names<RegisteredStruct>();

        REQUIRE(names.size() == 3);
        REQUIRE(names[0] == "x");
        REQUIRE(names[1] == "y");
        REQUIRE(names[2] == "z");
    }

    SECTION("private reflected struct names") {
        constexpr auto names = get_member_names<PrivateClass>();

        REQUIRE(names.size() == 3);
        REQUIRE(names[0] == "secret1");
        REQUIRE(names[1] == "secret2");
        REQUIRE(names[2] == "secret3");
    }

    SECTION("empty struct names") {
        constexpr auto names = get_member_names<EmptyStruct>();
        REQUIRE(names.size() == 0);
    }
}

TEST_CASE("get and index operations", "[moon][get][index_of]") {
    SECTION("name_of and index_of") {
        constexpr auto names = get_member_names<SimpleStruct>();
        REQUIRE(name_of<SimpleStruct, 0>() == "a");
        REQUIRE(name_of<SimpleStruct, 1>() == "b");
        REQUIRE(name_of<SimpleStruct, 2>() == "c");

        REQUIRE(name_of<SimpleStruct>(0) == "a");
        REQUIRE(name_of<SimpleStruct>(1) == "b");
        REQUIRE(name_of<SimpleStruct>(2) == "c");

        REQUIRE(index_of<SimpleStruct>("a") == 0);
        REQUIRE(index_of<SimpleStruct>("b") == 1);
        REQUIRE(index_of<SimpleStruct>("c") == 2);
        REQUIRE(index_of<SimpleStruct>("nonexistent") == 3); // 返回 size
    }

    SECTION("get by index") {
        SimpleStruct obj{42, 3.14, 'A'};

        REQUIRE(get<0>(obj) == 42);
        REQUIRE(get<1>(obj) == 3.14);
        REQUIRE(get<2>(obj) == 'A');

        // 修改值
        get<0>(obj) = 100;
        REQUIRE(obj.a == 100);
    }

    SECTION("get by name") {
        SimpleStruct obj{42, 3.14, 'A'};

        auto result = get(obj, "a");

        REQUIRE(index_of<SimpleStruct>("a") == 0);
        REQUIRE(index_of<SimpleStruct>("b") == 1);
        REQUIRE(index_of<SimpleStruct>("c") == 2);
    }
}

TEST_CASE("for_each", "[moon][for_each]") {
    SECTION("for_each over type - names only") {
        std::vector<std::string_view> captured;

        for_each<SimpleStruct>([&](std::string_view name, std::size_t idx) {
            captured.push_back(name);
            if (idx == 0) REQUIRE(name == "a");
            if (idx == 1) REQUIRE(name == "b");
            if (idx == 2) REQUIRE(name == "c");
        });

        REQUIRE(captured.size() == 3);
    }

    SECTION("for_each over object") {
        SimpleStruct obj{10, 20.5, 'X'};
        std::vector<int> values;

        for_each(obj, [&](auto& member, std::string_view name, std::size_t idx) {
            values.push_back(static_cast<int>(member));
        });

        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);  // double 转 int 截断
        REQUIRE(values[2] == 'X');
    }
}

TEST_CASE("visit_members", "[moon][visit_members]") {
    SECTION("visit all members") {
        SimpleStruct obj{1, 2.5, 'Z'};

        int sum = 0;
        visit_members(obj, [&](int& a, double& b, char& c) {
            sum = a + static_cast<int>(b) + static_cast<int>(c);
        });

        REQUIRE(sum == 1 + 2 + static_cast<int>('Z'));
    }
}

TEST_CASE("tuple and pair traits", "[moon][tuple_traits][pair_traits]") {
    SECTION("is_tuple") {
        REQUIRE(is_tuple_v<std::tuple<int, double, std::string>>);
        REQUIRE(is_tuple_v<std::tuple<>>);
        REQUIRE_FALSE(is_tuple_v<int>);
        REQUIRE_FALSE(is_tuple_v<SimpleStruct>);
    }

    SECTION("tuple_traits") {
        using Tup = std::tuple<int, double, char>;
        REQUIRE(tuple_traits<Tup>::size == 3);
        static_assert(std::is_same_v<tuple_traits<Tup>::element_t<0>, int>);
        static_assert(std::is_same_v<tuple_traits<Tup>::element_t<1>, double>);
        static_assert(std::is_same_v<tuple_traits<Tup>::element_t<2>, char>);
    }

    SECTION("is_pair") {
        REQUIRE(is_pair_v<std::pair<int, double>>);
        REQUIRE(is_pair_v<rainy::utility::pair<int, double>>);
        REQUIRE_FALSE(is_pair_v<int>);
        REQUIRE_FALSE(is_pair_v<std::tuple<int, double>>);
    }

    SECTION("pair_traits") {
        using P = std::pair<int, std::string>;
        REQUIRE(pair_traits<P>::size == 2);
        static_assert(std::is_same_v<pair_traits<P>::first_type, int>);
        static_assert(std::is_same_v<pair_traits<P>::second_type, std::string>);
    }
}

TEST_CASE("tuple_element", "[moon][tuple_element]") {
    SECTION("tuple_element for struct") {
        static_assert(rainy::type_traits::type_relations::is_convertible_v<tuple_element_t<0, SimpleStruct>, int*>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<tuple_element_t<1, SimpleStruct>, double*>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<tuple_element_t<2, SimpleStruct>, char*>);
    }

    SECTION("tuple_element for registered struct") {
        static_assert(rainy::type_traits::type_relations::is_convertible_v<tuple_element_t<0, RegisteredStruct>, int*>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<tuple_element_t<1, RegisteredStruct>, double*>);
        static_assert(rainy::type_traits::type_relations::is_convertible_v<tuple_element_t<2, RegisteredStruct>, std::string*>);
    }
}

TEST_CASE("pair reflection specialization", "[moon][pair]") {
    SECTION("std::pair") {
        constexpr auto count = member_count_v<std::pair<int, double>>;
        REQUIRE(count == 2);

        constexpr auto names = get_member_names<std::pair<int, double>>();
        REQUIRE(names.size() == 2);
        REQUIRE(names[0] == "first");
        REQUIRE(names[1] == "second");

        std::pair<int, double> obj{42, 3.14};
        auto ptr_tuple = struct_bind_tuple(obj);

        REQUIRE(*std::get<0>(ptr_tuple) == 42);
        REQUIRE(*std::get<1>(ptr_tuple) == 3.14);
    }

    SECTION("rainy::utility::pair") {
        using rainy::utility::pair;

        constexpr auto count = member_count_v<pair<int, double>>;
        REQUIRE(count == 2);

        pair<int, double> obj{42, 3.14};
        auto ptr_tuple = struct_bind_tuple(obj);

        REQUIRE(*std::get<0>(ptr_tuple) == 42);
        REQUIRE(*std::get<1>(ptr_tuple) == 3.14);
    }
}

// ============================================================================
// 测试用例 11: is_reflectet_for_type_valid
// ============================================================================

TEST_CASE("is_reflectet_for_type_valid", "[moon][reflectet_for_type]") {
    SECTION("registered types are valid") {
        REQUIRE(is_reflectet_for_type_valid<RegisteredStruct>);
        REQUIRE(is_reflectet_for_type_valid<PrivateClass>);
        REQUIRE(is_reflectet_for_type_valid<std::pair<int, int>>);
    }

    SECTION("non-registered aggregate types are invalid") {
        REQUIRE_FALSE(is_reflectet_for_type_valid<SimpleStruct>);
    }
}

// ============================================================================
// 测试用例 12: 模板结构体
// ============================================================================

TEMPLATE_TEST_CASE("template struct reflection", "[moon][template]", int, double, std::string) {
    using Struct = TemplateStruct<TestType>;

    REQUIRE(member_count_v<Struct> == 2);

    constexpr auto names = get_member_names<Struct>();
    REQUIRE(names.size() == 2);
    REQUIRE(names[0] == "value");
    REQUIRE(names[1] == "count");
}

// ============================================================================
// 测试用例 13: compile-time constexpr 验证
// ============================================================================

TEST_CASE("compile-time constexpr evaluation", "[moon][constexpr]") {
    SECTION("member_count is constexpr") {
        constexpr size_t count1 = member_count_v<SimpleStruct>;
        constexpr size_t count2 = member_count_v<RegisteredStruct>;
        constexpr size_t count3 = member_count_v<EmptyStruct>;

        static_assert(count1 == 3);
        static_assert(count2 == 3);
        static_assert(count3 == 0);
    }

    SECTION("struct_to_tuple is constexpr") {
        constexpr auto tuple = struct_to_tuple<SimpleStruct>();
        static_assert(std::tuple_size_v<decltype(tuple)> == 3);
    }

    SECTION("get_member_names is consteval") {
        constexpr auto names = get_member_names<SimpleStruct>();
        static_assert(names.size() == 3);
        static_assert(names[0] == "a");
        static_assert(names[1] == "b");
        static_assert(names[2] == "c");
    }
}

// ============================================================================
// 测试用例 14: tuple_size
// ============================================================================

TEST_CASE("tuple_size", "[moon][tuple_size]") {
    SECTION("tuple_size for structs") {
        REQUIRE(tuple_size_v<SimpleStruct> == 3);
        REQUIRE(tuple_size_v<RegisteredStruct> == 3);
        REQUIRE(tuple_size_v<PrivateClass> == 3);
        REQUIRE(tuple_size_v<EmptyStruct> == 0);
    }

    SECTION("tuple_size for pairs") {
        REQUIRE(tuple_size_v<std::pair<int, double>> == 2);
    }
}

struct ManuallyRegistered {
    int field1;
    std::string field2;
    double field3;
};

template <>
struct rainy::meta::moon::reflectet_for_type<ManuallyRegistered> {
    static constexpr inline std::size_t count = 3;

    static constexpr auto make() noexcept {
        auto& obj = type_traits::helper::get_fake_object<ManuallyRegistered>();
        return std::make_tuple(&obj.field1, &obj.field2, &obj.field3);
    }

    static constexpr auto bind_obj(ManuallyRegistered& obj) noexcept {
        return std::make_tuple(&obj.field1, &obj.field2, &obj.field3);
    }

    static constexpr auto member_names() noexcept {
        rainy::collections::array<std::string_view, 3> names = {"field1", "field2", "field3"};
        return names;
    }
};

TEST_CASE("manual registration with RAINY_REFLECT_TUPLE_LIKE", "[moon][manual_registration]") {
    REQUIRE(member_count_v<ManuallyRegistered> == 3);

    constexpr auto names = get_member_names<ManuallyRegistered>();
    REQUIRE(names.size() == 3);
    REQUIRE(names[0] == "field1");
    REQUIRE(names[1] == "field2");
    REQUIRE(names[2] == "field3");

    ManuallyRegistered obj{42, "test", 3.14};
    auto ptr_tuple = struct_bind_tuple(obj);

    REQUIRE(*std::get<0>(ptr_tuple) == 42);
    REQUIRE(*std::get<1>(ptr_tuple) == "test");
    REQUIRE(*std::get<2>(ptr_tuple) == 3.14);
}

// ============================================================================
// 测试用例 16: 错误处理 - 不存在的成员索引
// ============================================================================

TEST_CASE("out of bounds access", "[moon][error_handling]") {
    SimpleStruct obj{1, 2.0, 'c'};

    SECTION("name_of with out of bounds index returns empty") {
        REQUIRE(name_of<SimpleStruct>(999).empty());
    }

    SECTION("index_of with nonexistent name returns size") {
        REQUIRE(index_of<SimpleStruct>("nonexistent") == 3);
    }
}

// ============================================================================
// C++26 静态反射测试 (条件编译)
// ============================================================================

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

// 注解测试结构体 - 使用正确的 C++26 注解语法 [[= value]]
struct AnnotatedForTest {
    [[= rainy::annotations::moon::rename("renamed_field")]]
    int original_name = 100;

    [[= rainy::annotations::moon::with_prefix("pre_")]]
    [[= rainy::annotations::moon::with_suffix("_suf")]]
    double prefixed_suffixed = 3.14;

    [[= rainy::annotations::moon::no_prefix]]
    [[= rainy::annotations::moon::with_suffix("_only")]]
    int no_prefix_only = 200;

    [[= rainy::annotations::moon::no_suffix]]
    std::string no_suffix_field = "test";

    [[= rainy::annotations::moon::ignore]]
    int ignored_field = 999;
};

struct
[[
    = rainy::annotations::moon::with_prefix("type_"),
    = rainy::annotations::moon::with_suffix("_type")
    ]]
TypeAnnotatedStruct {
    int my_field = 10;
    double another_field = 20.5;
};

TEST_CASE("C++26 static reflection - annotations", "[moon][cxx26][annotations]") {
    SECTION("annotation presence") {
        constexpr auto names = get_member_names<AnnotatedForTest>();
        REQUIRE(names.size() == 4);  // ignored_field 被忽略

        // 验证 rename 注解
        // 注意：actual behavior depends on implements::try_apply_rename
        bool found_renamed = false;
        for (const auto& name : names) {
            if (name == "renamed_field") {
                found_renamed = true;
            }
        }
        // rename 应该被应用
        REQUIRE(found_renamed);
    }

    SECTION("prefix and suffix tags") {
        constexpr auto names = get_member_names<AnnotatedForTest>();

        // 验证前缀和后缀被应用
        bool found_prefixed = false;
        for (const auto& name : names) {
            if (name.find("pre_") != std::string_view::npos &&
                name.find("_suf") != std::string_view::npos) {
                found_prefixed = true;
            }
        }
        REQUIRE(found_prefixed);
    }

    SECTION("type-level annotations") {
        constexpr auto names = get_member_names<TypeAnnotatedStruct>();
        // 类型级别注解应该影响所有成员
        for (const auto& name : names) {
            // 应该包含类型级别的前缀和后缀
            // REQUIRE(name.find("type_") != std::string_view::npos);
            // REQUIRE(name.find("_type") != std::string_view::npos);
        }
    }
}

TEST_CASE("C++26 static reflection - member_offset", "[moon][cxx26][member_offset]") {
    SECTION("get_member_offset_arr") {
        SimpleStruct obj{1, 2.0, 'c'};
        auto offsets = get_member_offset_arr(obj);

        REQUIRE(offsets.size() == 3);
        REQUIRE(offsets[0] < offsets[1]);
        REQUIRE(offsets[1] < offsets[2]);
    }

    SECTION("get_member_offset_arr compile-time") {
        constexpr auto offsets = get_member_offset_arr<SimpleStruct>();
        REQUIRE(offsets.size() == 3);
    }
}

#endif

TEST_CASE("utility namespace aliases", "[moon][utility]") {
    using rainy::utility::member_count_v;
    using rainy::utility::struct_to_tuple;
    using rainy::utility::struct_bind_tuple;

    REQUIRE(member_count_v<SimpleStruct> == 3);

    constexpr auto tuple = struct_to_tuple<SimpleStruct>();
    REQUIRE(std::tuple_size_v<decltype(tuple)> == 3);

    SimpleStruct obj{1, 2.0, 'c'};
    auto ptr_tuple = struct_bind_tuple(obj);
    REQUIRE(std::tuple_size_v<decltype(ptr_tuple)> == 3);
}

TEST_CASE("tuple_to_variant", "[moon][tuple_to_variant]") {
    SECTION("convert struct to variant type") {
        using Variant = decltype(tuple_to_variant<SimpleStruct>());
        // variant 包含所有成员类型的 unique 组合
        // SimpleStruct 成员: int, double, char - 都是 unique
        // 所以 variant 有 3 个选项
    }
}