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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_VERSION_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_VERSION_HPP
#include <rainy/core/platform.hpp>

#define RAINY_WILLOW_MSGPACK_VERSION_MAJOR 1
#define RAINY_WILLOW_MSGPACK_VERSION_MINOR 0
#define RAINY_WILLOW_MSGPACK_VERSION_PATCH 0
#define RAINY_WILLOW_MSGPACK_VERSION \
    ((RAINY_WILLOW_MSGPACK_VERSION_MAJOR) * 1000000 + (RAINY_WILLOW_MSGPACK_VERSION_MINOR) * 1000 + \
     (RAINY_WILLOW_MSGPACK_VERSION_PATCH))

#define RAINY_WILLOW_MSGPACK_HAS_EXT 1
#define RAINY_WILLOW_MSGPACK_HAS_TIMESTAMP 1
#define RAINY_WILLOW_MSGPACK_HAS_BIN 1
#define RAINY_WILLOW_MSGPACK_HAS_STR8 1

#define RAINY_WILLOW_MSGPACK_AVAILABLE 1

namespace rainy::foundation::willow::msgpack {
    struct msgpack_version {
        static constexpr int major = RAINY_WILLOW_MSGPACK_VERSION_MAJOR;
        static constexpr int minor = RAINY_WILLOW_MSGPACK_VERSION_MINOR;
        static constexpr int patch = RAINY_WILLOW_MSGPACK_VERSION_PATCH;
        static constexpr int value = RAINY_WILLOW_MSGPACK_VERSION;
    };
}

#endif
