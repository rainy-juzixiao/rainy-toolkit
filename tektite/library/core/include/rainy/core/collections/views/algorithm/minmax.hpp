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
#ifndef RAINY_COLLECTIONS_VIEWS_ALGORITHM_MINMAX_HPP
#define RAINY_COLLECTIONS_VIEWS_ALGORITHM_MINMAX_HPP

#include <rainy/core/algorithm/minmax.hpp>
#include <rainy/core/collections/views/views_interface.hpp>

namespace rainy::core::collections::views {
    /**
     * @brief Finds the smallest element of a range.
     *        查找范围中最小的元素。
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto min_element(Range &&range) {
        return core::algorithm::min_element(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Finds the greatest element of a range.
     *        查找范围中最大的元素。
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto max_element(Range &&range) {
        return core::algorithm::max_element(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Finds both the smallest and greatest elements of a range.
     *        同时查找范围中最小和最大的元素。
     */
    template <typename Range>
    RAINY_NODISCARD constexpr auto minmax_element(Range &&range) {
        return core::algorithm::minmax_element(utility::begin(range), utility::end(range));
    }

    /**
     * @brief Returns the smaller of two values.
     *        返回两个值中较小的一个。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr auto min(const Ty &left, const Ty &right) -> const Ty & {
        return core::algorithm::min(left, right);
    }

    /**
     * @brief Returns the greater of two values.
     *        返回两个值中较大的一个。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr auto max(const Ty &left, const Ty &right) -> const Ty & {
        return core::algorithm::max(left, right);
    }

    /**
     * @brief Clamps a value into the [low, high] range.
     *        将值限制在[low, high]范围内。
     */
    template <typename Ty>
    RAINY_NODISCARD constexpr auto clamp(const Ty &value, const Ty &low, const Ty &high) -> const Ty & {
        return core::algorithm::clamp(value, low, high);
    }
}

namespace rainy::collections::views {
    using core::collections::views::clamp;
    using core::collections::views::max;
    using core::collections::views::max_element;
    using core::collections::views::min;
    using core::collections::views::min_element;
    using core::collections::views::minmax_element;
}

#endif
