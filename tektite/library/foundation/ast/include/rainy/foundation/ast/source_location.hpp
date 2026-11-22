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
#ifndef RAINY_FOUNDATION_AST_SOURCE_LOCATION_HPP
#define RAINY_FOUNDATION_AST_SOURCE_LOCATION_HPP
#include <cstdint>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief A position inside a source text.
     *
     * @brief Lines and columns are one-based, offsets are zero-based code unit indices.
     *
     * \lang simp-chinese
     * @brief 源文本中的一个位置。
     *
     * @brief 行与列从 1 开始，偏移量是从 0 开始的码元下标。
     */
    struct source_location {
        std::uint32_t line{1};
        std::uint32_t column{1};
        std::uint32_t offset{0};

        constexpr source_location() noexcept = default;

        constexpr source_location(const std::uint32_t line, const std::uint32_t column, const std::uint32_t offset) noexcept :
            line(line), column(column), offset(offset) {
        }
    };

    constexpr bool operator==(const source_location &left, const source_location &right) noexcept {
        return left.line == right.line && left.column == right.column && left.offset == right.offset;
    }

    constexpr bool operator!=(const source_location &left, const source_location &right) noexcept {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief A half-open range of source positions.
     *
     * \lang simp-chinese
     * @brief 源位置的半开区间。
     */
    struct source_span {
        source_location begin{};
        source_location end{};

        constexpr source_span() noexcept = default;

        constexpr source_span(const source_location begin, const source_location end) noexcept : begin(begin), end(end) {
        }

        constexpr bool empty() const noexcept {
            return begin == end;
        }
    };

    constexpr bool operator==(const source_span &left, const source_span &right) noexcept {
        return left.begin == right.begin && left.end == right.end;
    }

    constexpr bool operator!=(const source_span &left, const source_span &right) noexcept {
        return !(left == right);
    }
}

#endif
