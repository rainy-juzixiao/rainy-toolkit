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

// TODO(willow-cbor): implement CBOR binary encode/decode (RFC 8949)
//
// Planned layout once the cbor backend lands under
// include/rainy/foundation/willow/:
//   tests/cbor/value.cc      - construction / type predicates on decoded values
//   tests/cbor/roundtrip.cc  - encode(decode(x)) == x for scalars and nesting
//   tests/cbor/error.cc      - malformed input raises the format's exception
//
// Until then this stub keeps the test target linkable and reserves the
// [willow][cbor] Catch2 tag namespace.

TEST_CASE("cbor - not implemented yet", "[willow][cbor]") {
    // Intentionally empty: backend not ported/implemented yet.
    SUCCEED();
}
