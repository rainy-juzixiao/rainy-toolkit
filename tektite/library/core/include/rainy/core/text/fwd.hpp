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
    /**
     * \lang english
     * @brief Character traits type used by the text classes.
     *
     * @tparam Elem The character element type
     *
     * \lang simp-chinese
     * @brief 供文本类使用的字符 traits 类型。
     *
     * @tparam Elem 字符元素类型
     */
    template <typename Elem>
    struct char_traits;

    /**
     * \lang english
     * @brief A dynamically-sized character string with small-string optimization.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Allocator The allocator type
     *
     * \lang simp-chinese
     * @brief 带有短字符串优化的动态字符字符串。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Allocator 分配器类型
     */
    template <typename CharType, typename Traits = char_traits<CharType>, typename Allocator = memory::allocator<CharType>>
    class basic_string;

    /**
     * \lang english
     * @brief A non-owning view over a character sequence.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 字符序列的非拥有视图。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     */
    template <typename CharType, typename Traits = char_traits<CharType>>
    class basic_string_view;
}

#endif
