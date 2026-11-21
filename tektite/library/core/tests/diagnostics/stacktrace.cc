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
#include <rainy/core/diagnostics/stacktrace.hpp>

using namespace rainy::foundation::diagnostics;

TEST_CASE("stacktrace_entry - default construction", "[stacktrace][entry]") {
    stacktrace_entry entry;
    REQUIRE(entry.native_handle() == nullptr);
    REQUIRE_FALSE(static_cast<bool>(entry));
}

TEST_CASE("stacktrace_entry - copy semantics", "[stacktrace][entry]") {
    stacktrace_entry e1;
    stacktrace_entry e2{e1};
    REQUIRE(e2.native_handle() == e1.native_handle());

    stacktrace_entry e3;
    e3 = e1;
    REQUIRE(e3.native_handle() == e1.native_handle());
}

TEST_CASE("stacktrace_entry - comparison operators", "[stacktrace][entry]") {
    stacktrace_entry a;
    stacktrace_entry b;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("basic_stacktrace - default construction", "[stacktrace][basic]") {
    stacktrace st;
    REQUIRE(st.empty());
    REQUIRE(st.size() == 0);
    REQUIRE(st.begin() == st.end());
}

TEST_CASE("basic_stacktrace - copy semantics", "[stacktrace][basic]") {
    stacktrace st1;
    stacktrace st2{st1};
    REQUIRE(st2.empty());
    REQUIRE(st2.size() == st1.size());

    stacktrace st3;
    st3 = st1;
    REQUIRE(st3.empty());
}

TEST_CASE("basic_stacktrace - move semantics", "[stacktrace][basic]") {
    stacktrace st1;
    stacktrace st2{std::move(st1)};
    REQUIRE(st2.empty());

    stacktrace st3;
    stacktrace st4;
    st4 = std::move(st3);
    REQUIRE(st4.empty());
}

TEST_CASE("basic_stacktrace - comparison", "[stacktrace][basic]") {
    stacktrace a;
    stacktrace b;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("basic_stacktrace - swap", "[stacktrace][basic]") {
    stacktrace a;
    stacktrace b;
    a.swap(b);
    REQUIRE(a.empty());
    REQUIRE(b.empty());
}

TEST_CASE("basic_stacktrace - iterator empty range", "[stacktrace][iterator]") {
    const stacktrace st;
    REQUIRE(st.begin() == st.end());
    REQUIRE(st.cbegin() == st.cend());
    REQUIRE(st.rbegin() == st.rend());
    REQUIRE(st.crbegin() == st.crend());
}

TEST_CASE("basic_stacktrace - get_allocator", "[stacktrace][allocator]") {
    stacktrace st;
    auto alloc = st.get_allocator();
    (void)alloc;
    SUCCEED("get_allocator compiles and returns a valid allocator");
}

TEST_CASE("stacktrace_entry - bool conversion", "[stacktrace][entry]") {
    stacktrace_entry entry;
    REQUIRE_FALSE(entry);
}

TEST_CASE("basic_stacktrace - max_size", "[stacktrace][capacity]") {
    stacktrace st;
    REQUIRE(st.max_size() > 0);
}