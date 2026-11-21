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
#include <rainy/foundation/willow/implements/common/operations.hpp>
#include <rainy/foundation/willow/json.hpp>

#include <string>

using namespace rainy::foundation::willow;
namespace ops = rainy::foundation::willow::implements::operations;

namespace {
    document nested_array(const std::size_t depth) {
        document current(document_type::array);
        for (std::size_t i = 0; i < depth; ++i) {
            document next(document_type::array);
            next.push_back(current);
            current = rainy::utility::move(next);
        }
        return current;
    }
}

TEST_CASE("operations - deep equals rejects documents past the nesting limit", "[willow][operations][depth]") {
    const document left = nested_array(300);
    const document right = nested_array(300);
    REQUIRE_THROWS(ops::equals(left, right));
    REQUIRE_THROWS(ops::equals(left, right, 0));
}

TEST_CASE("operations - deep equals accepts shallow documents", "[willow][operations][depth]") {
    const document left = nested_array(4);
    const document right = nested_array(4);
    REQUIRE(ops::equals(left, right));
}
