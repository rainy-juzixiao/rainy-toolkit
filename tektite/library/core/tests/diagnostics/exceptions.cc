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
#include <rainy/core/diagnostics/exceptions.hpp>
#include <string>

using namespace rainy::core::exceptions;
using rainy::core::diagnostics::source_location;

void test_handler() {
}

TEST_CASE("exception - basic construction", "[exceptions][construction]") {
    exception ex("test error");
    std::string msg = ex.what();
    REQUIRE(msg.find("test error") != std::string::npos);
}

TEST_CASE("exception - copy semantics", "[exceptions][copy]") {
    exception ex1("original");
    exception ex2{ex1};
    std::string msg1 = ex1.what();
    std::string msg2 = ex2.what();
    REQUIRE(msg1 == msg2);
}

TEST_CASE("exception - move semantics", "[exceptions][move]") {
    exception ex1("movable");
    std::string before = ex1.what();
    exception ex2{std::move(ex1)};
    REQUIRE(ex2.what() == before);
}

TEST_CASE("exception - assignment", "[exceptions][assignment]") {
    exception ex1("first");
    exception ex2("second");
    ex2 = ex1;
    REQUIRE(std::string(ex2.what()) == std::string(ex1.what()));
}

TEST_CASE("logic_error hierarchy", "[exceptions][logic]") {
    logic::logic_error le("logic failure");
    std::string msg = le.what();
    REQUIRE(msg.find("logic failure") != std::string::npos);
}

TEST_CASE("runtime_error hierarchy", "[exceptions][runtime]") {
    runtime::runtime_error re("runtime failure");
    REQUIRE(std::string(re.what()).find("runtime failure") != std::string::npos);
}

TEST_CASE("specific exception types", "[exceptions][types]") {
    SECTION("out_of_range") {
        logic::out_of_range oor("index out of bounds");
        std::string msg = oor.what();
        REQUIRE((msg.find("out_of_range") != std::string::npos ||
                 msg.find("index out of bounds") != std::string::npos));
    }

    SECTION("nullpointer_exception") {
        runtime::nullpointer_exception npe("null ptr");
        std::string msg = npe.what();
        REQUIRE((msg.find("nullpointer") != std::string::npos ||
                 msg.find("null ptr") != std::string::npos));
    }

    SECTION("bad_alloc") {
        runtime::bad_alloc ba;
        REQUIRE(std::string(ba.what()).find("bad allocation") != std::string::npos);
    }

    SECTION("overflow_error") {
        runtime::overflow_error oe("overflow");
        REQUIRE(std::string(oe.what()).find("overflow") != std::string::npos);
    }

    SECTION("underflow_error") {
        runtime::underflow_error ue("underflow");
        REQUIRE(std::string(ue.what()).find("underflow") != std::string::npos);
    }
}

TEST_CASE("uncaught_exceptions", "[exceptions][utility]") {
    auto count = uncaught_exceptions();
    REQUIRE(count >= 0);
}

TEST_CASE("exception_semantic enum", "[exceptions][semantic]") {
    auto enforce = exception_semantic::enforce;
    auto observe = exception_semantic::observe;
    REQUIRE(enforce != observe);
}

TEST_CASE("with_this_exception tag", "[exceptions][tag]") {
    auto tag = with_this_exception<logic::logic_error>;
    (void)tag;
    SUCCEED("with_this_exception_t compiles and constructs");
}

TEST_CASE("exception - source_location integration", "[exceptions][source]") {
    exception ex("located error", source_location::current());
    std::string msg = ex.what();
    REQUIRE(msg.find("located error") != std::string::npos);
}

TEST_CASE("global_exception_handler - set and get", "[exceptions][handler]") {
    auto old = global_exception_handler(nullptr);
    REQUIRE(old != nullptr);

    auto current = global_exception_handler(nullptr);
    REQUIRE(current == old);
}

TEST_CASE("global_exception_handler - set custom handler", "[exceptions][handler]") {
    bool called = false;
    auto old = global_exception_handler(test_handler);
    auto current = global_exception_handler(nullptr);
    REQUIRE(current == test_handler);
    global_exception_handler(old);
}