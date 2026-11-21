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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_RESOLVER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_RESOLVER_HPP
#include <cstdint>
#include <rainy/core/functional/delegate.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/willow/implements/jsonnet/version.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet {
    using byte_buffer = core::collections::vector<std::uint8_t>;

    /**
     * \lang english
     * @brief Supplies source text or bytes for @c import, @c importstr and @c importbin.
     *
     * \lang simp-chinese
     * @brief 为 @c import、@c importstr 与 @c importbin 提供源文本或字节。
     */
    struct import_resolver {
        functional::delegate<core::text::string(const core::text::string &)> import_source{};
        functional::delegate<core::text::string(const core::text::string &)> importstr_source{};
        functional::delegate<byte_buffer(const core::text::string &)> importbin_source{};
    };
}

#endif

#endif
