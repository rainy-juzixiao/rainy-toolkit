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
#ifndef RAINY_CORE_IMPLEMENTS_ARM64_INTRIN_HPP
#define RAINY_CORE_IMPLEMENTS_ARM64_INTRIN_HPP
#include <rainy/core/platform.hpp>

#if RAINY_IS_ARM64
namespace rainy::core::pal::implements {
    RAINY_TOOLKIT_API long interlocked_increment_arm64_explicit(volatile long *value, memory_order order);
}
#endif

#endif