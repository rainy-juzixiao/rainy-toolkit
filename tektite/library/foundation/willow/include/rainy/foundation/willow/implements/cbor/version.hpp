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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_VERSION_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_VERSION_HPP
#include <rainy/core/platform.hpp>

#define RAINY_WILLOW_CBOR_VERSION_MAJOR 1
#define RAINY_WILLOW_CBOR_VERSION_MINOR 0
#define RAINY_WILLOW_CBOR_VERSION_PATCH 0
#define RAINY_WILLOW_CBOR_VERSION \
    ((RAINY_WILLOW_CBOR_VERSION_MAJOR) * 1000000 + (RAINY_WILLOW_CBOR_VERSION_MINOR) * 1000 + \
     (RAINY_WILLOW_CBOR_VERSION_PATCH))

#define RAINY_WILLOW_CBOR_RFC 8949

#define RAINY_WILLOW_CBOR_HAS_DOCUMENT_CODEC 1
#define RAINY_WILLOW_CBOR_HAS_DIRECT_CODEC 1

#define RAINY_WILLOW_CBOR_HAS_UNSIGNED_INT 1
#define RAINY_WILLOW_CBOR_HAS_NEGATIVE_INT 1
#define RAINY_WILLOW_CBOR_HAS_BYTE_STRING 1
#define RAINY_WILLOW_CBOR_HAS_TEXT_STRING 1
#define RAINY_WILLOW_CBOR_HAS_ARRAY 1
#define RAINY_WILLOW_CBOR_HAS_MAP 1
#define RAINY_WILLOW_CBOR_HAS_TAG 1
#define RAINY_WILLOW_CBOR_HAS_SIMPLE 1
#define RAINY_WILLOW_CBOR_HAS_HALF_FLOAT 1
#define RAINY_WILLOW_CBOR_HAS_SINGLE_FLOAT 1
#define RAINY_WILLOW_CBOR_HAS_DOUBLE_FLOAT 1
#define RAINY_WILLOW_CBOR_HAS_INDEFINITE 1

#if RAINY_HAS_CXX20
#define RAINY_WILLOW_CBOR_HAS_CXX20_FIELDS 1
#else
#define RAINY_WILLOW_CBOR_HAS_CXX20_FIELDS 0
#endif

#define RAINY_WILLOW_CBOR_AVAILABLE 1

namespace rainy::foundation::willow::cbor {
    struct cbor_version {
        static constexpr int major = RAINY_WILLOW_CBOR_VERSION_MAJOR;
        static constexpr int minor = RAINY_WILLOW_CBOR_VERSION_MINOR;
        static constexpr int patch = RAINY_WILLOW_CBOR_VERSION_PATCH;
        static constexpr int value = RAINY_WILLOW_CBOR_VERSION;
        static constexpr int rfc = RAINY_WILLOW_CBOR_RFC;
    };
}

#endif
