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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_VERSION_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_VERSION_HPP
#include <rainy/core/platform.hpp>

#define RAINY_WILLOW_BSON_VERSION_MAJOR 1
#define RAINY_WILLOW_BSON_VERSION_MINOR 1
#define RAINY_WILLOW_BSON_VERSION_PATCH 0
#define RAINY_WILLOW_BSON_VERSION \
    ((RAINY_WILLOW_BSON_VERSION_MAJOR) * 1000000 + (RAINY_WILLOW_BSON_VERSION_MINOR) * 1000 + \
     (RAINY_WILLOW_BSON_VERSION_PATCH))

#define RAINY_WILLOW_BSON_HAS_DOCUMENT_CODEC 1
#define RAINY_WILLOW_BSON_HAS_DIRECT_CODEC 1

#define RAINY_WILLOW_BSON_HAS_DOUBLE 1
#define RAINY_WILLOW_BSON_HAS_STRING 1
#define RAINY_WILLOW_BSON_HAS_DOCUMENT 1
#define RAINY_WILLOW_BSON_HAS_ARRAY 1
#define RAINY_WILLOW_BSON_HAS_BINARY 1
#define RAINY_WILLOW_BSON_HAS_UNDEFINED 1
#define RAINY_WILLOW_BSON_HAS_OBJECT_ID 1
#define RAINY_WILLOW_BSON_HAS_BOOLEAN 1
#define RAINY_WILLOW_BSON_HAS_DATETIME 1
#define RAINY_WILLOW_BSON_HAS_NULL 1
#define RAINY_WILLOW_BSON_HAS_REGEX 1
#define RAINY_WILLOW_BSON_HAS_CODE 1
#define RAINY_WILLOW_BSON_HAS_SYMBOL 1
#define RAINY_WILLOW_BSON_HAS_INT32 1
#define RAINY_WILLOW_BSON_HAS_TIMESTAMP 1
#define RAINY_WILLOW_BSON_HAS_INT64 1
#define RAINY_WILLOW_BSON_HAS_DECIMAL128 1
#define RAINY_WILLOW_BSON_HAS_MIN_KEY 1
#define RAINY_WILLOW_BSON_HAS_MAX_KEY 1

#if RAINY_HAS_CXX20
#define RAINY_WILLOW_BSON_HAS_CXX20_FIELDS 1
#else
#define RAINY_WILLOW_BSON_HAS_CXX20_FIELDS 0
#endif

#define RAINY_WILLOW_BSON_AVAILABLE 1

namespace rainy::foundation::willow::bson {
    struct bson_version {
        static constexpr int major = RAINY_WILLOW_BSON_VERSION_MAJOR;
        static constexpr int minor = RAINY_WILLOW_BSON_VERSION_MINOR;
        static constexpr int patch = RAINY_WILLOW_BSON_VERSION_PATCH;
        static constexpr int value = RAINY_WILLOW_BSON_VERSION;
    };
}

#endif
