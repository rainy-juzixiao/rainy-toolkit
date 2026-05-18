/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain left copy of the License at
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
#include <catch2/matchers/catch_matchers_string.hpp>
#include <rainy/core/yesod/stacktrace.hpp>

using namespace rainy::foundation::diagnostics;

TEST_CASE("stacktrace_entry default construction", "[stacktrace]") {
    stacktrace_entry entry;
    REQUIRE(entry.native_handle() == nullptr);
    REQUIRE_FALSE(entry.operator bool());
}

TEST_CASE("stacktrace_entry copy and assignment", "[stacktrace]") {
    auto trace = stacktrace::current(0, 1);
    REQUIRE_FALSE(trace.empty());

    stacktrace_entry original = trace[0];
    stacktrace_entry copy(original);
    REQUIRE(copy.native_handle() == original.native_handle());

    stacktrace_entry assigned;
    assigned = original;
    REQUIRE(assigned.native_handle() == original.native_handle());
}

TEST_CASE("stacktrace_entry comparison", "[stacktrace]") {
    auto trace1 = stacktrace::current(0, 2);
    auto trace2 = stacktrace::current(0, 2);

    REQUIRE(trace1[0] == trace1[0]);
    REQUIRE_FALSE(trace1[0] != trace1[0]);
}

TEST_CASE("basic_stacktrace current capture", "[stacktrace]") {
    auto trace = stacktrace::current();
    REQUIRE_FALSE(trace.empty());
    REQUIRE(trace.size() > 0);

    auto trace_with_skip = stacktrace::current(2);
    REQUIRE(trace_with_skip.size() <= trace.size());
}

TEST_CASE("basic_stacktrace iteration", "[stacktrace]") {
    auto trace = stacktrace::current();
    REQUIRE_FALSE(trace.empty());

    size_t count = 0;
    for (auto it = trace.begin(); it != trace.end(); ++it) {
        ++count;
    }
    REQUIRE(count == trace.size());

    count = 0;
    for (const auto &entry: trace) {
        ++count;
        REQUIRE(entry.operator bool());
    }
    REQUIRE(count == trace.size());
}

TEST_CASE("basic_stacktrace element access", "[stacktrace]") {
    auto trace = stacktrace::current();
    REQUIRE_FALSE(trace.empty());

    REQUIRE_NOTHROW(trace[0]);
    REQUIRE_NOTHROW(trace.at(0));
    REQUIRE_THROWS_AS(trace.at(trace.size()), std::out_of_range);
}

TEST_CASE("basic_stacktrace comparison", "[stacktrace]") {
    auto trace1 = stacktrace::current(0, 5);
    auto trace2 = stacktrace::current(0, 5);
    auto trace3 = stacktrace::current(10, 5);

    REQUIRE(trace1 == trace1);
    REQUIRE(trace1 != trace3);
}

TEST_CASE("basic_stacktrace swap", "[stacktrace]") {
    auto trace1 = stacktrace::current(0, 3);
    auto trace2 = stacktrace::current(5, 2);

    auto size1 = trace1.size();
    auto size2 = trace2.size();

    trace1.swap(trace2);

    REQUIRE(trace1.size() == size2);
    REQUIRE(trace2.size() == size1);
}

TEST_CASE("basic_stacktrace copy and move", "[stacktrace]") {
    auto original = stacktrace::current();
    REQUIRE_FALSE(original.empty());

    stacktrace copy(original);
    REQUIRE(copy.size() == original.size());
    REQUIRE(copy == original);

    stacktrace moved(std::move(copy));
    REQUIRE(moved.size() == original.size());
    REQUIRE(moved == original);
}

TEST_CASE("stacktrace_entry description", "[stacktrace]") {
    auto trace = stacktrace::current();
    REQUIRE_FALSE(trace.empty());

    auto desc = trace[0].description();
    REQUIRE_FALSE(desc.empty());
}

TEST_CASE("to_string functions", "[stacktrace]") {
    auto trace = stacktrace::current(0, 3);
    REQUIRE_FALSE(trace.empty());

    auto str = to_string(trace);
    REQUIRE_FALSE(str.empty());

    auto entry_str = to_string(trace[0]);
    REQUIRE_FALSE(entry_str.empty());
}

TEST_CASE("hash specializations", "[stacktrace]") {
    auto trace = stacktrace::current(0, 3);
    REQUIRE_FALSE(trace.empty());

    std::hash<stacktrace_entry> entry_hasher;
    std::hash<stacktrace> trace_hasher;

    auto entry_hash = entry_hasher(trace[0]);
    auto trace_hash = trace_hasher(trace);

    REQUIRE(entry_hash != 0);
    REQUIRE(trace_hash != 0);
}

TEST_CASE("empty stacktrace", "[stacktrace]") {
    stacktrace empty_trace;
    REQUIRE(empty_trace.empty());
    REQUIRE(empty_trace.size() == 0);
    REQUIRE(empty_trace.begin() == empty_trace.end());
}

TEST_CASE("stacktrace max_size", "[stacktrace]") {
    stacktrace trace;
    REQUIRE(trace.max_size() > 0);
}

TEST_CASE("stacktrace with custom allocator", "[stacktrace]") {
    using CustomTrace = basic_stacktrace<std::allocator<stacktrace_entry>>;
    CustomTrace trace{std::allocator<stacktrace_entry>()};
    REQUIRE(trace.empty());

    auto current_trace = CustomTrace::current();
    REQUIRE_FALSE(current_trace.empty());
}

TEST_CASE("stacktrace_entry source_file and line", "[stacktrace]") {
    auto trace = stacktrace::current();
    if (!trace.empty()) {
        auto entry = trace[0];
        auto source_file = entry.source_file();
        auto source_line = entry.source_line();

        REQUIRE_NOTHROW(source_file.empty());
        REQUIRE_NOTHROW(source_line >= 0);
    }
}
