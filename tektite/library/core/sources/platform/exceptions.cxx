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
#include <rainy/core/platform.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>

namespace rainy::core::implements {
    void throw_exception_out_of_range(const char *msg) {
        exceptions::logic::throw_out_of_range(msg);
    }

    void throw_exception_length_error(const char *msg) {
        exceptions::logic::throw_length_error(msg);
    }
}
