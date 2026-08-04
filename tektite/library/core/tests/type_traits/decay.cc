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
#include <rainy/core/type_traits/decay.hpp>

using namespace rainy::type_traits::other_trans;

template <typename T, typename U>
struct is_same {
    static constexpr bool value = false;
};

template <typename T>
struct is_same<T, T> {
    static constexpr bool value = true;
};

TEST_CASE("decay trait for fundamental types", "[decay]") {
    SECTION("void types") {
        STATIC_REQUIRE(is_same<decay_t<void>, void>::value);
        STATIC_REQUIRE(is_same<decay_t<const void>, void>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile void>, void>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile void>, void>::value);
    }

    SECTION("integral types") {
        STATIC_REQUIRE(is_same<decay_t<int>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<const int>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile int>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile int>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<unsigned int>, unsigned int>::value);
        STATIC_REQUIRE(is_same<decay_t<long long>, long long>::value);
    }

    SECTION("floating point types") {
        STATIC_REQUIRE(is_same<decay_t<float>, float>::value);
        STATIC_REQUIRE(is_same<decay_t<const float>, float>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile float>, float>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile float>, float>::value);
        STATIC_REQUIRE(is_same<decay_t<double>, double>::value);
    }

    SECTION("boolean type") {
        STATIC_REQUIRE(is_same<decay_t<bool>, bool>::value);
        STATIC_REQUIRE(is_same<decay_t<const bool>, bool>::value);
    }
}

TEST_CASE("decay trait for references", "[decay]") {
    SECTION("lvalue references") {
        STATIC_REQUIRE(is_same<decay_t<int &>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<const int &>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile int &>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile int &>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<float &>, float>::value);
    }

    SECTION("rvalue references") {
        STATIC_REQUIRE(is_same<decay_t<int &&>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<const int &&>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile int &&>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile int &&>, int>::value);
        STATIC_REQUIRE(is_same<decay_t<float &&>, float>::value);
    }

    SECTION("reference to reference") {
        using ref_ref = int &&;
        STATIC_REQUIRE(is_same<decay_t<ref_ref>, int>::value);
    }
}

TEST_CASE("decay trait for arrays", "[decay]") {
    SECTION("fixed-size arrays") {
        STATIC_REQUIRE(is_same<decay_t<int[5]>, int *>::value);
        STATIC_REQUIRE(is_same<decay_t<const int[5]>, const int *>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile int[5]>, volatile int *>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile int[5]>, const volatile int *>::value);
    }

    SECTION("multi-dimensional arrays") {
        STATIC_REQUIRE(is_same<decay_t<int[5][3]>, int (*)[3]>::value);
        STATIC_REQUIRE(is_same<decay_t<const int[5][3]>, const int (*)[3]>::value);
        STATIC_REQUIRE(is_same<decay_t<int[5][3][2]>, int (*)[3][2]>::value);
    }

    SECTION("array references") {
        STATIC_REQUIRE(is_same<decay_t<int (&)[5]>, int *>::value);
        STATIC_REQUIRE(is_same<decay_t<const int (&)[5]>, const int *>::value);
        STATIC_REQUIRE(is_same<decay_t<int (&&)[5]>, int *>::value);
    }
}

TEST_CASE("decay trait for function types", "[decay]") {
    SECTION("function types decay to function pointers") {
        STATIC_REQUIRE(is_same<decay_t<void()>, void (*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<int(int)>, int (*)(int)>::value);
        STATIC_REQUIRE(is_same<decay_t<float(double, char)>, float (*)(double, char)>::value);
        STATIC_REQUIRE(is_same<decay_t<void(int, float)>, void (*)(int, float)>::value);
    }

    SECTION("function pointer types") {
        STATIC_REQUIRE(is_same<decay_t<void (*)()>, void (*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<int (*)(int)>, int (*)(int)>::value);
        STATIC_REQUIRE(is_same<decay_t<const void (*)()>, const void (*)()>::value);
    }

    SECTION("function references") {
        STATIC_REQUIRE(is_same<decay_t<void (&)()>, void (*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<int (&)(int)>, int (*)(int)>::value);
        STATIC_REQUIRE(is_same<decay_t<void (&&)()>, void (*)()>::value);
    }

    SECTION("function types with cv-qualifiers on pointer") {
        // cv-qualifiers on function pointer types are valid
        STATIC_REQUIRE(is_same<decay_t<void (*const)()>, void (*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<void (*volatile)()>, void (*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<void (*const volatile)()>, void (*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<int (*const)(int)>, int (*)(int)>::value);
    }

    SECTION("function types with ref-qualifiers") {
        // Ref-qualifiers are only valid for non-static member functions
        // We need to test through member function types
        struct TestStruct {
            void lvalue_func() & {
            }
            void rvalue_func() && {
            }
            void const_func() const {
            }
            void volatile_func() volatile {
            }
            void cv_func() const volatile {
            }
        };

        // Test member function pointer types with ref-qualifiers
        STATIC_REQUIRE(is_same<decay_t<void (TestStruct::*)() &>, void (TestStruct::*)() &>::value);
        STATIC_REQUIRE(is_same<decay_t<void (TestStruct::*)() &&>, void (TestStruct::*)() &&>::value);
        STATIC_REQUIRE(is_same<decay_t<void (TestStruct::*)() const>, void (TestStruct::*)() const>::value);
        STATIC_REQUIRE(is_same<decay_t<void (TestStruct::*)() volatile>, void (TestStruct::*)() volatile>::value);
        STATIC_REQUIRE(is_same<decay_t<void (TestStruct::*)() const volatile>, void (TestStruct::*)() const volatile>::value);
    }

    SECTION("function types with noexcept") {
#if __cplusplus >= 201703L
        STATIC_REQUIRE(is_same<decay_t<void() noexcept>, void (*)() noexcept>::value);
        STATIC_REQUIRE(is_same<decay_t<int(int) noexcept>, int (*)(int) noexcept>::value);
#endif
    }
}

TEST_CASE("decay trait for complex types", "[decay]") {
    SECTION("class and struct types") {
        struct TestStruct {};
        class TestClass {};

        STATIC_REQUIRE(is_same<decay_t<TestStruct>, TestStruct>::value);
        STATIC_REQUIRE(is_same<decay_t<const TestStruct>, TestStruct>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile TestStruct>, TestStruct>::value);
        STATIC_REQUIRE(is_same<decay_t<const volatile TestStruct>, TestStruct>::value);
        STATIC_REQUIRE(is_same<decay_t<TestClass>, TestClass>::value);
        STATIC_REQUIRE(is_same<decay_t<TestStruct &>, TestStruct>::value);
        STATIC_REQUIRE(is_same<decay_t<TestStruct &&>, TestStruct>::value);
    }

    SECTION("enum types") {
        enum TestEnum {
            VALUE1,
            VALUE2
        };
        enum class TestEnumClass {
            VALUE1,
            VALUE2
        };

        STATIC_REQUIRE(is_same<decay_t<TestEnum>, TestEnum>::value);
        STATIC_REQUIRE(is_same<decay_t<const TestEnum>, TestEnum>::value);
        STATIC_REQUIRE(is_same<decay_t<volatile TestEnum>, TestEnum>::value);
        STATIC_REQUIRE(is_same<decay_t<TestEnumClass>, TestEnumClass>::value);
        STATIC_REQUIRE(is_same<decay_t<TestEnum &>, TestEnum>::value);
        STATIC_REQUIRE(is_same<decay_t<TestEnum &&>, TestEnum>::value);
    }

    SECTION("pointer types") {
        STATIC_REQUIRE(is_same<decay_t<int *>, int *>::value);
        STATIC_REQUIRE(is_same<decay_t<const int *>, const int *>::value);
        STATIC_REQUIRE(is_same<decay_t<int *const>, int *>::value);
        STATIC_REQUIRE(is_same<decay_t<int *volatile>, int *>::value);
        STATIC_REQUIRE(is_same<decay_t<int *const volatile>, int *>::value);
        STATIC_REQUIRE(is_same<decay_t<int **>, int **>::value);
        STATIC_REQUIRE(is_same<decay_t<const int *const>, const int *>::value);
    }

    SECTION("pointer to member") {
        struct S {
            int member;
            void func() {
            }
            void const_func() const {
            }
            void volatile_func() volatile {
            }
            void cv_func() const volatile {
            }
        };

        STATIC_REQUIRE(is_same<decay_t<int S::*>, int S::*>::value);
        STATIC_REQUIRE(is_same<decay_t<const int S::*>, const int S::*>::value);
        STATIC_REQUIRE(is_same<decay_t<void (S::*)()>, void (S::*)()>::value);
        STATIC_REQUIRE(is_same<decay_t<void (S::*)() const>, void (S::*)() const>::value);
        STATIC_REQUIRE(is_same<decay_t<void (S::*)() volatile>, void (S::*)() volatile>::value);
        STATIC_REQUIRE(is_same<decay_t<void (S::*)() const volatile>, void (S::*)() const volatile>::value);
    }
}

TEST_CASE("decay trait for mixed cases", "[decay]") {
    SECTION("arrays of function pointers") {
        using array_type = int (*[5])(int);
        STATIC_REQUIRE(is_same<decay_t<array_type>, int (**)(int)>::value);
    }

    SECTION("reference to array of function pointer") {
        using ref_type = int (*(&) [5])(int);
        STATIC_REQUIRE(is_same<decay_t<ref_type>, int (**)(int)>::value);
    }

    SECTION("function returning function pointer") {
        using func_type = int (*)(int); // Function returning int*
        STATIC_REQUIRE(is_same<decay_t<func_type>, int (*)(int)>::value);
    }
}

// Runtime tests to verify the decay_t type works with actual variables
TEST_CASE("decay trait runtime behavior", "[decay]") {
    SECTION("decay of array variable") {
        int arr[5] = {1, 2, 3, 4, 5};
        decay_t<decltype(arr)> ptr = arr; // Should decay to int*
        REQUIRE(ptr == arr);
        REQUIRE(ptr[0] == 1);
        REQUIRE(ptr[2] == 3);
    }

    SECTION("decay of const array variable") {
        const int arr[3] = {10, 20, 30};
        decay_t<decltype(arr)> ptr = arr; // Should decay to const int*
        REQUIRE(ptr == arr);
        REQUIRE(ptr[1] == 20);
    }

    SECTION("decay of function variable") {
        auto func = [](int x) -> int { return x * 2; };
        decay_t<decltype(func)> f = func;
        REQUIRE(f(5) == 10);
    }

    SECTION("decay of function pointer variable") {
        int (*func_ptr)(int) = [](int x) -> int { return x * 3; };
        decay_t<decltype(func_ptr)> f = func_ptr;
        REQUIRE(f(4) == 12);
    }

    SECTION("decay of member function pointer") {
        struct S {
            int value;
            int get_value() const {
                return value;
            }
        };

        int (S::*mem_func)() const = &S::get_value;
        decay_t<decltype(mem_func)> f = mem_func;
        S s{42};
        REQUIRE((s.*f)() == 42);
    }
}
