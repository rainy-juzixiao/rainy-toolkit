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
#include <rainy/foundation/os/process.hpp>

namespace rainy::foundation::os {
    rain_fn current_process_id() noexcept -> std::uint64_t {
        return implements::current_process_id();
    }

    rain_fn process_list() noexcept -> core::collections::vector<process_entry> {
        return implements::query_process_list();
    }

    rain_fn current_priority() noexcept -> int {
        return implements::current_priority();
    }

    rain_fn set_current_priority(int priority) noexcept -> bool {
        return implements::set_current_priority(priority);
    }
}
