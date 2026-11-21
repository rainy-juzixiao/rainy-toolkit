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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RING_BUFFER_FWD_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RING_BUFFER_FWD_HPP

#include <cstddef>
#include <cstdint>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief Selects how a ring_buffer behaves when it is full.
     *
     * \lang simp-chinese
     * @brief 选择 ring_buffer 满载时的行为。
     */
    enum class overflow_policy : std::uint8_t {
        /**
         * \lang english
         * @brief Pushing onto a full buffer throws a length_error.
         *
         * \lang simp-chinese
         * @brief 向已满缓冲压入时抛出 length_error。
         */
        throw_on_full,
        /**
         * \lang english
         * @brief Pushing onto a full buffer overwrites the oldest element.
         *
         * \lang simp-chinese
         * @brief 向已满缓冲压入时覆盖最旧的元素。
         */
        overwrite_oldest
    };
}

namespace rainy::foundation::collections::implements {
    template <typename BufferPtr, typename Value>
    class ring_iterator;
}

#endif
