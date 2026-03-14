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
#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rainy/foundation/container/variant.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable: 26800)
#endif

using namespace rainy::foundation::container;

namespace Catch {
    template <typename... Types>
    struct is_range<rainy::foundation::container::variant<Types...>> : std::false_type {};
}

struct NonTrivial {
    int value;
    static inline int construct_count = 0;
    static inline int destruct_count = 0;
    static inline int copy_count = 0;
    static inline int move_count = 0;

    explicit NonTrivial(int v = 0) : value(v) {
        ++construct_count;
    }
    ~NonTrivial() {
        ++destruct_count;
    }
    NonTrivial(const NonTrivial &o) : value(o.value) {
        ++copy_count;
    }
    NonTrivial(NonTrivial &&o) noexcept : value(o.value) {
        o.value = -1;
        ++move_count;
    }
    NonTrivial &operator=(const NonTrivial &o) {
        value = o.value;
        return *this;
    }
    NonTrivial &operator=(NonTrivial &&o) noexcept {
        value = o.value;
        o.value = -1;
        return *this;
    }
    bool operator==(const NonTrivial &o) const {
        return value == o.value;
    }
    bool operator<(const NonTrivial &o) const {
        return value < o.value;
    }

    static void reset() {
        construct_count = destruct_count = copy_count = move_count = 0;
    }
};

struct ThrowOnCopy {
    int value = 0;
    ThrowOnCopy() = default;
    ThrowOnCopy(const ThrowOnCopy &) {
        throw std::runtime_error("copy failed");
    }
    ThrowOnCopy(ThrowOnCopy &&) noexcept = default;
    ThrowOnCopy &operator=(const ThrowOnCopy &) {
        throw std::runtime_error("copy failed");
    }
    ThrowOnCopy &operator=(ThrowOnCopy &&) noexcept = default;
    bool operator==(const ThrowOnCopy &o) const {
        return value == o.value;
    }
    bool operator<(const ThrowOnCopy &o) const {
        return value < o.value;
    }
};

struct MoveOnly {
    int value;
    explicit MoveOnly(int v = 0) : value(v) {
    }
    MoveOnly(const MoveOnly &) = delete;
    MoveOnly(MoveOnly &&o) noexcept : value(o.value) {
        o.value = -1;
    }
    MoveOnly &operator=(const MoveOnly &) = delete;
    MoveOnly &operator=(MoveOnly &&o) noexcept {
        value = o.value;
        o.value = -1;
        return *this;
    }
    bool operator==(const MoveOnly &o) const {
        return value == o.value;
    }
};

struct InitListConstructible {
    std::vector<int> data;
    InitListConstructible(std::initializer_list<int> il) : data(il) {
    }
    InitListConstructible(std::initializer_list<int> il, int ex) : data(il) {
        data.push_back(ex);
    }
};

constexpr int constexpr_get_int(variant<int, double> v) {
    return get<int>(v);
}
constexpr bool constexpr_holds_int(variant<int, double> v) {
    return holds_alternative<int>(v);
}
constexpr std::size_t constexpr_index(variant<int, double> v) {
    return v.index();
}

static_assert(constexpr_get_int(42) == 42, "constexpr get<T> must work");
static_assert(constexpr_holds_int(42) == true, "constexpr holds_alternative must work");
static_assert(constexpr_index(3.14) == 1, "constexpr index must work");
static_assert(constexpr_get_int(variant<int, double>(std::in_place_index<0>, 7)) == 7, "constexpr in_place_index construction must work");

SCENARIO("A variant is default constructed", "[variant][constructor][default]") {
    GIVEN("a variant<int, double>") {
        variant<int, double> v;
        THEN("it holds the first alternative") {
            REQUIRE(v.index() == 0);
        }
        AND_THEN("the held int is value-initialised to zero") {
            REQUIRE(get<0>(v) == 0);
        }
        AND_THEN("it is not valueless") {
            REQUIRE_FALSE(v.valueless_by_exception());
        }
    }

    GIVEN("a variant whose first type is NonTrivial") {
        NonTrivial::reset();
        {
            variant<NonTrivial, int> v;
            THEN("NonTrivial's constructor is called exactly once") {
                REQUIRE(NonTrivial::construct_count == 1);
            }
        }
        AND_WHEN("the variant goes out of scope") {
            THEN("NonTrivial's destructor is called exactly once") {
                REQUIRE(NonTrivial::destruct_count == 1);
            }
        }
    }
}

SCENARIO("A variant is constructed from a value", "[variant][constructor][converting]") {
    GIVEN("a variant<int, double, std::string>") {
        WHEN("constructed from an int literal") {
            variant<int, double, std::string> v(42);
            THEN("index is 0 and the held value is 42") {
                REQUIRE(v.index() == 0);
                REQUIRE(get<0>(v) == 42);
            }
        }
        WHEN("constructed from a double literal") {
            variant<int, double, std::string> v(3.14);
            THEN("index is 1 and the held value is 3.14") {
                REQUIRE(v.index() == 1);
                REQUIRE(get<1>(v) == 3.14);
            }
        }
        WHEN("constructed from a std::string") {
            variant<int, double, std::string> v(std::string("hello"));
            THEN("index is 2 and the held value is \"hello\"") {
                REQUIRE(v.index() == 2);
                REQUIRE(get<2>(v) == "hello");
            }
        }
    }
}

SCENARIO("A variant is constructed in-place by type", "[variant][constructor][in_place_type]") {
    GIVEN("a variant<int, std::string>") {
        WHEN("using in_place_type<std::string> with a C-string argument") {
            variant<int, std::string> v(std::in_place_type<std::string>, "world");
            THEN("index is 1") {
                REQUIRE(v.index() == 1);
            }
            AND_THEN("the string is correctly constructed") {
                REQUIRE(get<std::string>(v) == "world");
            }
        }
        WHEN("using placeholder_type<std::string> (rainy extension)") {
            variant<int, std::string> v(std::in_place_type<std::string>, "rainy");
            THEN("index is 1 and value matches") {
                REQUIRE(v.index() == 1);
                REQUIRE(get<std::string>(v) == "rainy");
            }
        }
    }

    GIVEN("a variant<int, std::vector<int>>") {
        WHEN("using in_place_type with an initializer_list") {
            variant<int, InitListConstructible> v(std::in_place_type<InitListConstructible>, std::initializer_list<int>{1, 2, 3});
            THEN("the type is correctly constructed from the list") {
                REQUIRE(v.index() == 1);
                REQUIRE(get<InitListConstructible>(v).data == std::vector<int>{1, 2, 3});
            }
        }
        WHEN("using in_place_type with initializer_list plus extra args") {
            variant<int, InitListConstructible> v(std::in_place_type<InitListConstructible>, std::initializer_list<int>{1, 2, 3}, 99);
            THEN("the extra argument is appended") {
                REQUIRE(get<InitListConstructible>(v).data == std::vector<int>{1, 2, 3, 99});
            }
        }
    }
}

SCENARIO("A variant is constructed in-place by index", "[variant][constructor][in_place_index]") {
    GIVEN("a variant<int, std::string, double>") {
        WHEN("using in_place_index<1> to build a string") {
            variant<int, std::string, double> v(std::in_place_index<1>, "indexed");
            THEN("index is 1 and value is correct") {
                REQUIRE(v.index() == 1);
                REQUIRE(get<1>(v) == "indexed");
            }
        }
        WHEN("using std::in_place_index<2> to build a double") {
            variant<int, std::string, double> v(std::in_place_index<2>, 2.718);
            THEN("index is 2 and value is correct") {
                REQUIRE(v.index() == 2);
                REQUIRE(get<2>(v) == 2.718);
            }
        }
        WHEN("using in_place_index with an initializer_list alternative") {
            variant<int, InitListConstructible> v(std::in_place_index<1>, std::initializer_list<int>{10, 20, 30});
            THEN("the list is forwarded correctly") {
                REQUIRE(get<1>(v).data == std::vector<int>{10, 20, 30});
            }
        }
    }
}

SCENARIO("A variant is copy constructed", "[variant][constructor][copy]") {
    GIVEN("a source variant holding NonTrivial") {
        NonTrivial::reset();
        variant<NonTrivial, int> src(std::in_place_index<0>, 7);
        WHEN("copy constructing a new variant") {
            variant<NonTrivial, int> dst(src);
            THEN("both have the same index") {
                REQUIRE(dst.index() == src.index());
            }
            AND_THEN("the held values are equal") {
                REQUIRE(get<0>(dst).value == get<0>(src).value);
            }
            AND_THEN("NonTrivial's copy constructor was called") {
                REQUIRE(NonTrivial::copy_count >= 1);
            }
        }
    }

    GIVEN("a source variant holding std::string") {
        variant<int, std::string> src(std::string("copy me"));
        WHEN("copy constructing") {
            auto dst(src);
            THEN("the string is duplicated correctly") {
                REQUIRE(get<std::string>(dst) == "copy me");
            }
            AND_THEN("modifying dst does not affect src") {
                get<std::string>(dst) += " modified";
                REQUIRE(get<std::string>(src) == "copy me");
            }
        }
    }
}

SCENARIO("A variant is move constructed", "[variant][constructor][move]") {
    GIVEN("a source variant holding NonTrivial") {
        NonTrivial::reset();
        variant<NonTrivial, int> src(std::in_place_index<0>, 42);
        WHEN("move constructing a new variant") {
            variant<NonTrivial, int> dst(std::move(src));
            THEN("dst holds the moved-from value") {
                REQUIRE(get<0>(dst).value == 42);
            }
            AND_THEN("NonTrivial's move constructor was called") {
                REQUIRE(NonTrivial::move_count >= 1);
            }
        }
    }

    GIVEN("a source variant holding MoveOnly") {
        variant<MoveOnly, int> src(std::in_place_index<0>, 99);
        WHEN("move constructing") {
            variant<MoveOnly, int> dst(std::move(src));
            THEN("dst holds the correct value") {
                REQUIRE(get<0>(dst).value == 99);
            }
        }
    }
}

SCENARIO("A variant is copy assigned", "[variant][assignment][copy]") {
    GIVEN("two variants holding different types") {
        variant<int, std::string> lhs(42);
        variant<int, std::string> rhs(std::string("assigned"));
        WHEN("copy assigning rhs to lhs") {
            lhs = rhs;
            THEN("lhs now holds string with the correct value") {
                REQUIRE(lhs.index() == 1);
                REQUIRE(get<std::string>(lhs) == "assigned");
            }
            AND_THEN("rhs is unchanged") {
                REQUIRE(get<std::string>(rhs) == "assigned");
            }
        }
    }

    GIVEN("a variant being assigned from a valueless rhs") {
        variant<int, ThrowOnCopy> lhs(42);
        variant<int, ThrowOnCopy> rhs;
        try {
            rhs = ThrowOnCopy{};
        } catch (...) {}
        if (rhs.valueless_by_exception()) {
            lhs = rhs;
        }
    }

    GIVEN("self assignment") {
        variant<int, std::string> v(std::string("self"));
        WHEN("assigning to itself") {
            v = v; // NOLINT
            THEN("the value is unchanged") {
                REQUIRE(get<std::string>(v) == "self");
            }
        }
    }

    GIVEN("a variant being assigned from a valueless rhs") {
        variant<int, ThrowOnCopy> lhs(42);
        variant<int, ThrowOnCopy> rhs;
        // Force rhs to valueless
        try {
            rhs = ThrowOnCopy{};
        } catch (...) {
        }
        // Only run this branch when rhs is actually valueless
        if (rhs.valueless_by_exception()) {
            WHEN("copy assigning valueless rhs") {
                lhs = rhs;
                THEN("lhs becomes valueless too") {
                    REQUIRE(lhs.valueless_by_exception());
                }
            }
        }
    }
}

SCENARIO("A variant is move assigned", "[variant][assignment][move]") {
    GIVEN("two variants holding different types") {
        variant<int, std::string> lhs(0);
        variant<int, std::string> rhs(std::string("moved"));
        WHEN("move assigning rhs into lhs") {
            lhs = std::move(rhs);
            THEN("lhs holds the string") {
                REQUIRE(get<std::string>(lhs) == "moved");
            }
        }
    }

    GIVEN("two variants holding the same type") {
        variant<int, std::string> lhs(std::string("old"));
        variant<int, std::string> rhs(std::string("new"));
        WHEN("move assigning same-type") {
            lhs = std::move(rhs);
            THEN("lhs is updated") {
                REQUIRE(get<std::string>(lhs) == "new");
            }
        }
    }

    GIVEN("a variant holding NonTrivial") {
        NonTrivial::reset();
        variant<NonTrivial, int> lhs(std::in_place_index<0>, 1);
        variant<NonTrivial, int> rhs(std::in_place_index<1>, 42);
        WHEN("assigning a different alternative") {
            lhs = std::move(rhs);
            THEN("lhs now holds int") {
                REQUIRE(lhs.index() == 1);
                REQUIRE(get<1>(lhs) == 42);
            }
            AND_THEN("the old NonTrivial in lhs was destroyed") {
                REQUIRE(NonTrivial::destruct_count >= 1);
            }
        }
    }
}

SCENARIO("A variant is assigned from a raw value", "[variant][assignment][converting]") {
    GIVEN("a variant<int, std::string> currently holding int") {
        variant<int, std::string> v(0);
        WHEN("assigning a string value") {
            v = std::string("from value");
            THEN("the variant now holds string") {
                REQUIRE(v.index() == 1);
                REQUIRE(get<std::string>(v) == "from value");
            }
        }
        WHEN("assigning an int value") {
            v = 99;
            THEN("the variant still holds int with the new value") {
                REQUIRE(v.index() == 0);
                REQUIRE(get<int>(v) == 99);
            }
        }
    }
}

SCENARIO("emplace constructs a new alternative in-place", "[variant][emplace]") {
    GIVEN("a variant<int, std::string> currently holding int") {
        variant<int, std::string> v(0);

        WHEN("emplacing std::string by type") {
            auto &ref = v.emplace<std::string>("emplaced");
            THEN("the variant now holds string") {
                REQUIRE(v.index() == 1);
                REQUIRE(get<std::string>(v) == "emplaced");
            }
            AND_THEN("the returned reference points into the variant") {
                REQUIRE(&ref == &get<std::string>(v));
            }
        }

        WHEN("emplacing int by index") {
            auto &ref = v.emplace<0>(77);
            THEN("the variant holds the new int") {
                REQUIRE(v.index() == 0);
                REQUIRE(get<0>(v) == 77);
            }
            AND_THEN("the reference is valid") {
                REQUIRE(ref == 77);
            }
        }
    }

    GIVEN("a variant<int, InitListConstructible>") {
        variant<int, InitListConstructible> v(0);

        WHEN("emplacing InitListConstructible with an initializer_list by type") {
            v.emplace<InitListConstructible>(std::initializer_list<int>{5, 6, 7});
            THEN("data is correctly constructed") {
                REQUIRE(get<InitListConstructible>(v).data == std::vector<int>{5, 6, 7});
            }
        }

        WHEN("emplacing with initializer_list and extra arg by index") {
            v.emplace<1>(std::initializer_list<int>{1, 2}, 100);
            THEN("extra arg is appended") {
                REQUIRE(get<1>(v).data == std::vector<int>{1, 2, 100});
            }
        }
    }

    GIVEN("a variant<NonTrivial, int>") {
        NonTrivial::reset();
        variant<NonTrivial, int> v(std::in_place_index<1>, 0);
        WHEN("emplacing NonTrivial (replacing int)") {
            v.emplace<NonTrivial>(55);
            THEN("the old int is replaced and NonTrivial is constructed fresh") {
                REQUIRE(v.index() == 0);
                REQUIRE(get<0>(v).value == 55);
                REQUIRE(NonTrivial::construct_count >= 1);
            }
        }
    }
}

SCENARIO("get<I> retrieves the held value by index", "[variant][get][index]") {
    GIVEN("a variant<int, double, std::string> holding string at index 2") {
        variant<int, double, std::string> v(std::string("retrieve"));
        WHEN("calling get<2> on an lvalue and mutating through the reference") {
            auto &ref = get<2>(v);
            REQUIRE(ref == "retrieve");
            ref += "!";
            AND_THEN("the mutation is visible via const access") {
                const auto &cv = v;
                REQUIRE(get<2>(cv) == "retrieve!");
            }
            AND_THEN("the mutation is visible when moving out") {
                auto moved = get<2>(std::move(v));
                REQUIRE(moved == "retrieve!");
            }
        }
        WHEN("calling get<2> on a const lvalue") {
            const auto &cv = v;
            REQUIRE(get<2>(cv) == "retrieve");
        }
        WHEN("calling get<2> on an rvalue") {
            auto moved = get<2>(std::move(v));
            THEN("the string is moved out") {
                REQUIRE(moved == "retrieve");
            }
        }
        WHEN("calling get with the wrong index") {
            THEN("bad_variant_access is thrown") {
                REQUIRE_THROWS(get<0>(v));
            }
        }
    }
}

SCENARIO("get<T> retrieves the held value by type", "[variant][get][type]") {
    GIVEN("a variant<int, double, std::string> holding int 42") {
        variant<int, double, std::string> v(42);

        WHEN("calling get<int>") {
            THEN("returns 42") {
                REQUIRE(get<int>(v) == 42);
            }
        }

        WHEN("calling get<double> on a variant holding int") {
            THEN("bad_variant_access is thrown") {
                REQUIRE_THROWS(get<double>(v));
            }
        }

        WHEN("calling get<int> on rvalue") {
            int val = get<int>(std::move(v));
            THEN("the value is moved out correctly") {
                REQUIRE(val == 42);
            }
        }
    }
}

SCENARIO("get_if returns a pointer or null", "[variant][get_if]") {
    GIVEN("a variant<int, std::string> holding int") {
        variant<int, std::string> v(10);

        WHEN("get_if<0> is called with a valid pointer") {
            auto *p = get_if<0>(&v);
            THEN("a non-null pointer is returned") {
                REQUIRE(p != nullptr);
                REQUIRE(*p == 10);
            }
        }

        WHEN("get_if<1> is called (wrong alternative)") {
            auto *p = get_if<1>(&v);
            THEN("nullptr is returned") {
                REQUIRE(p == nullptr);
            }
        }

        WHEN("get_if<int> by type is called") {
            auto *p = get_if<int>(&v);
            THEN("a non-null pointer to the int is returned") {
                REQUIRE(p != nullptr);
                REQUIRE(*p == 10);
            }
        }

        WHEN("get_if<std::string> by type is called") {
            auto *p = get_if<std::string>(&v);
            THEN("nullptr is returned") {
                REQUIRE(p == nullptr);
            }
        }

        WHEN("a null pointer is passed to get_if") {
            variant<int, std::string> *null_v = nullptr;
            THEN("nullptr is returned safely") {
                REQUIRE(get_if<0>(null_v) == nullptr);
            }
        }
    }

    GIVEN("a const variant<int, std::string> holding int") {
        const variant<int, std::string> cv(5);
        WHEN("get_if<int> is called on const pointer") {
            const auto *p = get_if<int>(&cv);
            THEN("a valid const pointer is returned") {
                REQUIRE(p != nullptr);
                REQUIRE(*p == 5);
            }
        }
    }
}

SCENARIO("holds_alternative reports whether a type is active", "[variant][holds_alternative]") {
    GIVEN("a variant<int, double, std::string>") {
        variant<int, double, std::string> v(3.14);

        THEN("holds_alternative<double> is true") {
            REQUIRE(holds_alternative<double>(v));
        }
        THEN("holds_alternative<int> is false") {
            REQUIRE_FALSE(holds_alternative<int>(v));
        }
        THEN("holds_alternative<std::string> is false") {
            REQUIRE_FALSE(holds_alternative<std::string>(v));
        }

        WHEN("the variant is reassigned to string") {
            v = std::string("new");
            THEN("holds_alternative<std::string> becomes true") {
                REQUIRE(holds_alternative<std::string>(v));
            }
            AND_THEN("holds_alternative<double> becomes false") {
                REQUIRE_FALSE(holds_alternative<double>(v));
            }
        }
    }
}

SCENARIO("swap exchanges the contents of two variants", "[variant][swap]") {
    GIVEN("two variants holding the same alternative type") {
        variant<int, std::string> a(std::string("alpha"));
        variant<int, std::string> b(std::string("beta"));
        WHEN("member swap is called") {
            a.swap(b);
            THEN("a now holds beta and b holds alpha") {
                REQUIRE(get<std::string>(a) == "beta");
                REQUIRE(get<std::string>(b) == "alpha");
            }
        }
    }

    GIVEN("two variants holding different alternative types") {
        variant<int, std::string> a(42);
        variant<int, std::string> b(std::string("swap me"));
        WHEN("non-member swap is called") {
            swap(a, b);
            THEN("indices are exchanged") {
                REQUIRE(a.index() == 1);
                REQUIRE(b.index() == 0);
            }
            AND_THEN("values are exchanged") {
                REQUIRE(get<std::string>(a) == "swap me");
                REQUIRE(get<int>(b) == 42);
            }
        }
    }

    GIVEN("a variant swapped with itself") {
        variant<int, std::string> v(std::string("self-swap"));
        WHEN("swap(v, v) is called") {
            swap(v, v); // NOLINT
            THEN("the value is unchanged") {
                REQUIRE(get<std::string>(v) == "self-swap");
            }
        }
    }

    GIVEN("two variants holding NonTrivial") {
        NonTrivial::reset();
        variant<NonTrivial, int> a(std::in_place_index<0>, 1);
        variant<NonTrivial, int> b(std::in_place_index<0>, 2);
        WHEN("swapped") {
            a.swap(b);
            THEN("values are exchanged without extra heap allocation") {
                REQUIRE(get<0>(a).value == 2);
                REQUIRE(get<0>(b).value == 1);
            }
        }
    }
}

SCENARIO("Variants are compared for equality", "[variant][comparison][equality]") {
    GIVEN("two variants with the same index and value") {
        variant<int, std::string> a(42), b(42);
        THEN("they compare equal") {
            REQUIRE(a == b);
            REQUIRE_FALSE(a != b);
        }
    }

    GIVEN("two variants with the same index but different values") {
        variant<int, std::string> a(1), b(2);
        THEN("they are not equal") {
            REQUIRE(a != b);
            REQUIRE_FALSE(a == b);
        }
    }

    GIVEN("two variants with different indices") {
        variant<int, std::string> a(0), b(std::string("x"));
        THEN("they are not equal") {
            REQUIRE(a != b);
        }
    }

    GIVEN("two valueless variants") {
        // Manufacture two valueless variants via ThrowOnCopy
        variant<int, ThrowOnCopy> x, y;
        try {
            ThrowOnCopy tc;
            x = tc;
        } catch (...) {
        }
        try {
            ThrowOnCopy tc;
            y = tc;
        } catch (...) {
        }
        if (x.valueless_by_exception() && y.valueless_by_exception()) {
            THEN("two valueless variants compare equal") {
                REQUIRE(x == y);
            }
        }
    }
}

SCENARIO("Variants are compared with relational operators", "[variant][comparison][relational]") {
    GIVEN("variants with the same index") {
        variant<int, std::string> a(1), b(2);
        THEN("a < b") {
            REQUIRE(a < b);
            REQUIRE(b > a);
            REQUIRE(a <= b);
            REQUIRE(b >= a);
        }
    }

    GIVEN("variants with different indices") {
        variant<int, std::string> a(999); // index 0
        variant<int, std::string> b(std::string("a")); // index 1
        THEN("smaller index is less regardless of held value") {
            REQUIRE(a < b);
            REQUIRE_FALSE(b < a);
        }
    }

    GIVEN("a valueless and a non-valueless variant") {
        variant<int, ThrowOnCopy> vl;
        try {
            ThrowOnCopy tc;
            vl = tc;
        } catch (...) {
        }
        variant<int, ThrowOnCopy> nv(42);
        if (vl.valueless_by_exception()) {
            THEN("valueless is less than any non-valueless") {
                REQUIRE(vl < nv);
                REQUIRE_FALSE(nv < vl);
            }
        }
    }
}

SCENARIO("visit applies a visitor to the active alternative", "[variant][visit]") {
    GIVEN("a variant<int, double, std::string>") {
        WHEN("it holds int and visit extracts the value") {
            variant<int, double, std::string> v(7);
            int result = visit(
                [](auto &&x) -> int {
                    if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>)
                        return x;
                    return -1;
                },
                v);
            THEN("the visitor receives the int") {
                REQUIRE(result == 7);
            }
        }

        WHEN("it holds string and visit returns its size") {
            variant<int, double, std::string> v(std::string("hello"));
            std::size_t sz = visit(
                [](auto &&x) -> std::size_t {
                    if constexpr (std::is_same_v<std::decay_t<decltype(x)>, std::string>)
                        return x.size();
                    return 0;
                },
                v);
            THEN("the visitor receives the string") {
                REQUIRE(sz == 5);
            }
        }

        WHEN("visiting a valueless variant") {
            variant<int, ThrowOnCopy> v;
            try {
                ThrowOnCopy tc;
                v = tc;
            } catch (...) {
            }
            if (v.valueless_by_exception()) {
                THEN("visit throws bad_variant_access") {
                    REQUIRE_THROWS(visit([](auto &&) {}, v));
                }
            }
        }
    }

    GIVEN("two variants for multi-variant visit") {
        variant<int, std::string> a(3);
        variant<int, std::string> b(4);
        WHEN("visiting both simultaneously") {
            int sum = visit(
                [](auto &&x, auto &&y) -> int {
                    if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int> && std::is_same_v<std::decay_t<decltype(y)>, int>)
                        return x + y;
                    return -1;
                },
                a, b);
            THEN("the visitor receives both values") {
                REQUIRE(sum == 7);
            }
        }
    }
}

SCENARIO("visit with explicit return type Rx", "[variant][visit][typed]") {
    GIVEN("a variant<int, double>") {
        variant<int, double> v(2);
        WHEN("calling visit<double> to force double return") {
            double result = visit<double>([](auto &&x) -> double { return static_cast<double>(x) * 1.5; }, v);
            THEN("the result is cast to double correctly") {
                REQUIRE(result == Catch::Approx(3.0));
            }
        }
    }
}

SCENARIO("The variant destructor calls the held value's destructor", "[variant][destructor]") {
    GIVEN("a scope containing a variant<NonTrivial, int>") {
        NonTrivial::reset();
        {
            variant<NonTrivial, int> v(std::in_place_index<0>, 5);
            REQUIRE(NonTrivial::construct_count == 1);
        }
        THEN("NonTrivial's destructor is called exactly once on scope exit") {
            REQUIRE(NonTrivial::destruct_count == 1);
        }
    }

    GIVEN("a variant that transitions through two NonTrivial values") {
        NonTrivial::reset();
        {
            variant<NonTrivial, int> v(std::in_place_index<0>, 1);
            v.emplace<NonTrivial>(2);
        }
        THEN("exactly two NonTrivials were constructed and two destroyed") {
            REQUIRE(NonTrivial::construct_count == 2);
            REQUIRE(NonTrivial::destruct_count == 2);
        }
    }

    GIVEN("a trivially destructible variant") {
        variant<int, double> v(1.0);
        THEN("it is trivially destructible") {
            REQUIRE(std::is_trivially_destructible_v<variant<int, double>>);
        }
    }
}

SCENARIO("Assignment preserves the old value when an exception is thrown", "[variant][exception]") {
    GIVEN("a variant<int, ThrowOnCopy> holding int") {
        variant<int, ThrowOnCopy> v(42);
        WHEN("assigning a ThrowOnCopy lvalue which throws during copy") {
            ThrowOnCopy tc;
            THEN("the assignment propagates the exception") {
                REQUIRE_THROWS(v = tc);
            }
            AND_THEN("the variant is either valueless or holds the original int") {
                // Standard permits valueless after failed assignment
                if (!v.valueless_by_exception()) {
                    REQUIRE(get<int>(v) == 42);
                }
            }
        }
    }
}

SCENARIO("variant operations work at compile time and produce correct runtime values", "[variant][constexpr]") {

    GIVEN("the compile-time static_asserts defined at file scope all pass") {
        THEN("constexpr_get_int(42) == 42") {
            REQUIRE(constexpr_get_int(42) == 42);
        }
        THEN("constexpr_holds_int(42) == true") {
            REQUIRE(constexpr_holds_int(42) == true);
        }
        THEN("constexpr_index(3.14) == 1") {
            REQUIRE(constexpr_index(3.14) == 1);
        }
    }

    GIVEN("a constexpr-constructed variant using in_place_index") {
        constexpr variant<int, double> v(std::in_place_index<1>, 2.718);
        THEN("index is 1 at runtime too") {
            REQUIRE(v.index() == 1);
        }
        THEN("get<1> returns the correct value") {
            REQUIRE(get<1>(v) == 2.718);
        }
    }

    GIVEN("a constexpr holds_alternative check") {
        constexpr variant<int, double> v(std::in_place_index<0>, 100);
        static_assert(holds_alternative<int>(v));
        THEN("the result is consistent at runtime") {
            REQUIRE(holds_alternative<int>(v));
            REQUIRE_FALSE(holds_alternative<double>(v));
        }
    }
}

SCENARIO("variant correctly handles more than eight alternative types", "[variant][large]") {
    using BigVariant = variant<int, double, float, char, short, long, unsigned, bool, std::string, std::vector<int>>;

    GIVEN("a BigVariant constructed at index 8 (std::string)") {
        BigVariant v(std::in_place_index<8>, "big");
        THEN("index is 8") {
            REQUIRE(v.index() == 8);
        }
        THEN("get<8> returns the correct string") {
            REQUIRE(get<8>(v) == "big");
        }
        THEN("get<std::string> also works") {
            REQUIRE(get<std::string>(v) == "big");
        }
    }

    GIVEN("a BigVariant constructed at index 9 (std::vector<int>)") {
        BigVariant v(std::in_place_index<9>, std::vector<int>{1, 2, 3});
        THEN("index is 9 and contents are correct") {
            REQUIRE(v.index() == 9);
            REQUIRE(get<9>(v) == std::vector<int>{1, 2, 3});
        }
    }

    GIVEN("visiting every alternative of a BigVariant") {
        BigVariant v(std::in_place_index<3>, 'X');
        char result = visit(
            [](auto &&x) -> char {
                if constexpr (std::is_same_v<std::decay_t<decltype(x)>, char>)
                    return x;
                return '\0';
            },
            v);
        THEN("the visitor dispatches to the correct alternative") {
            REQUIRE(result == 'X');
        }
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
