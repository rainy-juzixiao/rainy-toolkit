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

namespace rainy::core::implements {
    RAINY_TOOLKIT_API void stl_internal_check(bool result) {
        if (result) {
            return;
        }
        constexpr char error_info[] = "we found a error. before make more crushing, we must "
                                      "terminate this program. you can commit a issue in github.";
        (void) std::fwrite(error_info, sizeof(char), sizeof(error_info), stderr);
        std::abort();
    }
}
