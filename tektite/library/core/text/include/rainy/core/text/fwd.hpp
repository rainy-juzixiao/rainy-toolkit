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
#ifndef RAINY_CORE_TEXT_FWD_HPP
#define RAINY_CORE_TEXT_FWD_HPP

#include <rainy/core/memory/allocator.hpp>

namespace rainy::core::text {
    template <typename Elem>
    struct char_traits;

    template <typename CharType, typename Traits = char_traits<CharType>, typename Allocator = memory::allocator<CharType>>
    class basic_string;

    template <typename CharType, typename Traits = char_traits<CharType>>
    class basic_string_view;
}

#endif
