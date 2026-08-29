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
#include <rainy/foundation/os/dynamic_library/module_context.hpp>

namespace dynamic_library = rainy::foundation::dynamic_library;

namespace {
    constexpr const char *math_library_name =
#if RAINY_USING_WINDOWS
        "ntdll.dll";
#else
        "libm.so.6";
#endif
}

TEST_CASE("module_context loads and releases a real module", "[foundation][os]") {
    dynamic_library::module_context context(math_library_name, false);
    REQUIRE(context.is_loaded());
    REQUIRE(context.native_handle() != nullptr);

    REQUIRE(context.release());
    REQUIRE_FALSE(context.is_loaded());
}

TEST_CASE("module_context resolves exported symbols", "[foundation][os]") {
    dynamic_library::module_context context(math_library_name, false);
    REQUIRE(context.is_loaded());

    SECTION("load_symbol returns a valid address") {
        auto symbol = context.load_symbol(
#if RAINY_USING_WINDOWS
            "RtlUlongByteSwap"
#else
            "cos"
#endif
        );
        REQUIRE(symbol != nullptr);
    }
    SECTION("load_symbol_to binds the function pointer") {
        double (*cosine)(double) = nullptr;
        REQUIRE(context.load_symbol_to(
#if RAINY_USING_WINDOWS
            "RtlUlongByteSwap"
#else
            "cos"
#endif
            , cosine));
        REQUIRE(cosine != nullptr);
    }
    SECTION("missing symbols are reported as null") {
        REQUIRE(context.load_symbol("definitely_not_a_real_symbol_42") == nullptr);
    }
}

TEST_CASE("module_context handles missing modules gracefully", "[foundation][os]") {
    dynamic_library::module_context context("definitely_not_a_real_module_42.so", false);
    REQUIRE_FALSE(context.is_loaded());
    REQUIRE(context.native_handle() == nullptr);

    SECTION("and can load afterwards") {
        REQUIRE_FALSE(context.load("definitely_not_a_real_module_43.so"));
        REQUIRE_FALSE(context.is_loaded());
    }
}

TEST_CASE("module_context observes an already loaded module", "[foundation][os]") {
    dynamic_library::module_context loader(math_library_name, false);
    REQUIRE(loader.is_loaded());
    dynamic_library::module_context observer(math_library_name, true);
    REQUIRE(observer.is_loaded());
}
