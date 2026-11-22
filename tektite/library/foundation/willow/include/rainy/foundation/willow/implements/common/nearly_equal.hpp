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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_NEARLY_EQUAL_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_NEARLY_EQUAL_HPP
#include <cmath>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/limits.hpp>

namespace rainy::foundation::willow::implements {
    template <typename Ty>
    bool nearly_equal(Ty a, Ty b) noexcept {
        return std::fabs(a - b) < utility::numeric_limits<Ty>::epsilon();
    }
}

#endif
