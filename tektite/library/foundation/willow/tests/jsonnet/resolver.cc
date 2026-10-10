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
#include <rainy/foundation/willow/jsonnet.hpp>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::jsonnet;
using rainy::foundation::exceptions::willow::jsonnet::jsonnet_import_error;

namespace rcore = rainy::core;

namespace {
    import_resolver make_resolver() {
        import_resolver resolver;
        resolver.import_source = [](const rcore::text::string &path) -> rcore::text::string {
            if (path == "lib.libsonnet") {
                return "{ answer: 42 }";
            }
            if (path == "nested.libsonnet") {
                return "import \"lib.libsonnet\"";
            }
            return "null";
        };
        resolver.importstr_source = [](const rcore::text::string &path) -> rcore::text::string {
            if (path == "data.txt") {
                return "hello world";
            }
            return "";
        };
        resolver.importbin_source = [](const rcore::text::string &path) -> byte_buffer {
            if (path == "data.bin") {
                return byte_buffer{0x01, 0x02, 0xFF};
            }
            return {};
        };
        return resolver;
    }

    document run_with(const char *source, const import_resolver &resolver) {
        facade<document> parsed = jsonnet::parse(source);
        return evaluate(parsed, resolver);
    }
}

TEST_CASE("jsonnet import - import evaluates the referenced document", "[jsonnet][resolver]") {
    const import_resolver resolver = make_resolver();
    REQUIRE(run_with("(import \"lib.libsonnet\").answer", resolver).as_integer() == 42);
    REQUIRE(run_with("import \"lib.libsonnet\"", resolver).size() == 1);
}

TEST_CASE("jsonnet import - nested imports resolve transitively", "[jsonnet][resolver]") {
    const import_resolver resolver = make_resolver();
    REQUIRE(run_with("(import \"nested.libsonnet\").answer", resolver).as_integer() == 42);
}

TEST_CASE("jsonnet import - importstr returns raw text", "[jsonnet][resolver]") {
    const import_resolver resolver = make_resolver();
    REQUIRE(run_with("importstr \"data.txt\"", resolver).as_string() == "hello world");
}

TEST_CASE("jsonnet import - importbin returns a byte array", "[jsonnet][resolver]") {
    const import_resolver resolver = make_resolver();
    const document bytes = run_with("importbin \"data.bin\"", resolver);
    REQUIRE(bytes.is_array());
    REQUIRE(bytes.size() == 3);
    REQUIRE(bytes[0].as_integer() == 1);
    REQUIRE(bytes[2].as_integer() == 255);
}

TEST_CASE("jsonnet import - a missing resolver raises an import error", "[jsonnet][resolver]") {
    facade<document> parsed = jsonnet::parse("import \"lib.libsonnet\"");
    REQUIRE_THROWS_AS(evaluate(parsed), jsonnet_import_error);
}

TEST_CASE("jsonnet import - an empty callback raises an import error", "[jsonnet][resolver]") {
    import_resolver resolver;
    resolver.import_source = [](const rcore::text::string &) { return rcore::text::string("1"); };
    facade<document> parsed = jsonnet::parse("importstr \"x.txt\"");
    REQUIRE_THROWS_AS(evaluate(parsed, resolver), jsonnet_import_error);
}
