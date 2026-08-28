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
#include <rainy/core/type_traits/primary_types.hpp>

using namespace rainy::type_traits::primary_types;
using rainy::type_traits::type_relations::is_same_v;

namespace testing {
    enum plain_enum {};
    enum class scoped_enum {};
    union test_union {
        int x;
        double y;
    };
    struct test_struct {};
    class test_class {};
    void free_function(int);
    struct member_holder {
        int value;
        void method();
    };
    template <typename...>
    struct variadic_template {};
} // namespace testing

TEST_CASE("fundamental type categories", "[type_traits][primary_types]") {
    STATIC_REQUIRE(is_void_v<void>);
    STATIC_REQUIRE_FALSE(is_void_v<int>);

    STATIC_REQUIRE(is_integral_v<int>);
    STATIC_REQUIRE(is_integral_v<const unsigned long long>);
    STATIC_REQUIRE_FALSE(is_integral_v<double>);
    STATIC_REQUIRE_FALSE(is_integral_v<bool> == false);

    STATIC_REQUIRE(is_floating_point_v<float>);
    STATIC_REQUIRE(is_floating_point_v<const long double>);
    STATIC_REQUIRE_FALSE(is_floating_point_v<int>);

    STATIC_REQUIRE(is_null_pointer_v<std::nullptr_t>);
    STATIC_REQUIRE_FALSE(is_null_pointer_v<void *>);
}

TEST_CASE("composite type categories", "[type_traits][primary_types]") {
    SECTION("enum / union / class") {
        STATIC_REQUIRE(is_enum_v<testing::plain_enum>);
        STATIC_REQUIRE(is_enum_v<testing::scoped_enum>);
        STATIC_REQUIRE_FALSE(is_enum_v<int>);

        STATIC_REQUIRE(is_union_v<testing::test_union>);
        STATIC_REQUIRE_FALSE(is_union_v<testing::test_struct>);

        STATIC_REQUIRE(is_class_v<testing::test_struct>);
        STATIC_REQUIRE(is_class_v<testing::test_class>);
        STATIC_REQUIRE_FALSE(is_class_v<testing::test_union>);
        STATIC_REQUIRE_FALSE(is_class_v<int>);
    }
    SECTION("references / pointers / arrays / functions") {
        STATIC_REQUIRE(is_lvalue_reference_v<int &>);
        STATIC_REQUIRE_FALSE(is_lvalue_reference_v<int &&>);
        STATIC_REQUIRE(is_rvalue_reference_v<int &&>);
        STATIC_REQUIRE_FALSE(is_rvalue_reference_v<int &>);

        STATIC_REQUIRE(is_pointer_v<int *>);
        STATIC_REQUIRE(is_pointer_v<const char **>);
        STATIC_REQUIRE_FALSE(is_pointer_v<int>);

        STATIC_REQUIRE(is_array_v<int[2]>);
        STATIC_REQUIRE(is_array_v<int[]>);
        STATIC_REQUIRE_FALSE(is_array_v<int *>);

        STATIC_REQUIRE(is_function_v<void(int)>);
        STATIC_REQUIRE_FALSE(is_function_v<void (*)(int)>);
    }
    SECTION("member pointers") {
        STATIC_REQUIRE(is_member_object_pointer_v<int testing::member_holder::*>);
        STATIC_REQUIRE(is_member_function_pointer_v<void (testing::member_holder::*)()>);
        STATIC_REQUIRE_FALSE(is_member_object_pointer_v<void (testing::member_holder::*)()>);
        STATIC_REQUIRE_FALSE(is_member_function_pointer_v<int testing::member_holder::*>);
    }
}

TEST_CASE("type identity and specialization detection", "[type_traits][primary_types]") {
    STATIC_REQUIRE(is_same_v<type_identity_t<int>, int>);
    STATIC_REQUIRE(is_same_v<type_identity<int>::type, int>);

    STATIC_REQUIRE(is_specialization_v<testing::variadic_template<int, double>, testing::variadic_template>);
    STATIC_REQUIRE_FALSE(is_specialization_v<int, testing::variadic_template>);
}

TEST_CASE("array_size", "[type_traits][primary_types]") {
    STATIC_REQUIRE(array_size_v<int[4]> == 4);
    STATIC_REQUIRE(array_size_v<int[4][8]> == 4);
    STATIC_REQUIRE(array_size<int[10]>::value == 10);
}
