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

// TODO(willow-ini): implement INI / profile style key-value sections
//
// Planned layout once the ini backend lands under
// include/rainy/foundation/willow/:
//   tests/ini/value.cc      - construction / type predicates on decoded values
//   tests/ini/roundtrip.cc  - encode(decode(x)) == x for scalars and nesting
//   tests/ini/error.cc      - malformed input raises the format's exception
//
// Until then this stub keeps the test target linkable and reserves the
// [willow][ini] Catch2 tag namespace.

TEST_CASE("ini - not implemented yet", "[willow][ini]") {
    // Intentionally empty: backend not ported/implemented yet.
    SUCCEED();
}
