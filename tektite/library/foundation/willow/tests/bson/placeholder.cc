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
#include <catch2/catch_all.hpp>

// TODO(willow-bson): implement BSON binary encode/decode
//
// Planned layout once the bson backend lands under
// include/rainy/foundation/willow/:
//   tests/bson/value.cc      - construction / type predicates on decoded values
//   tests/bson/roundtrip.cc  - encode(decode(x)) == x for scalars and nesting
//   tests/bson/error.cc      - malformed input raises the format's exception
//
// Until then this stub keeps the test target linkable and reserves the
// [willow][bson] Catch2 tag namespace.

TEST_CASE("bson - not implemented yet", "[willow][bson]") {
    // Intentionally empty: backend not ported/implemented yet.
    SUCCEED();
}
