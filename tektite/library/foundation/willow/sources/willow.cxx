/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/foundation/willow/json.hpp>
#include <rainy/foundation/willow/json5.hpp>

namespace rainy::foundation::willow {
    thread_local auto *pool_storage = new std::pmr::unsynchronized_pool_resource{};

    thread_local std::pmr::memory_resource *memory_resource = pool_storage;

    std::pmr::memory_resource *set_memory_resource(std::pmr::memory_resource *memres) noexcept {
        if (!memres) {
            return memory_resource;
        }
        return utility::exchange(memory_resource, memres);
    }

    std::pmr::memory_resource *get_memory_resource() noexcept {
        return memory_resource;
    }
}

namespace rainy::foundation::willow::implements {
    std::uint32_t merge_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) noexcept {
        std::uint32_t code = ((lead_surrogate - unicode_surrogate_lead_begin) << unicode_surrogate_bits);
        code += (trail_surrogate - unicode_surrogate_trail_begin);
        code += unicode_surrogate_base;
        return code;
    }
}
