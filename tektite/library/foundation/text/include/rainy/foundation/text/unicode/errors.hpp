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
#ifndef RAINY_FOUNDATION_TEXT_UNICODE_ERRORS_HPP
#define RAINY_FOUNDATION_TEXT_UNICODE_ERRORS_HPP

namespace rainy::foundation::text::unicode {
    /**
     * \lang english
     * @brief The reason a transmutation step failed.
     *
     * \lang simp-chinese
     * @brief 一次施法步骤失败的原因。
     */
    enum class encoding_error {
        /**
         * \lang english
         * @brief No error occurred.
         *
         * \lang simp-chinese
         * @brief 没有发生错误。
         */
        none,
        /**
         * \lang english
         * @brief An unexpected byte or code unit was found where a leading element was required.
         *
         * \lang simp-chinese
         * @brief 在需要首元素的位置发现了意外的字节或码元。
         */
        invalid_leading_element,
        /**
         * \lang english
         * @brief A continuation element was missing or malformed.
         *
         * \lang simp-chinese
         * @brief 延续元素缺失或格式错误。
         */
        invalid_continuation_element,
        /**
         * \lang english
         * @brief The sequence ended in the middle of a multi element code point.
         *
         * \lang simp-chinese
         * @brief 序列在多元素码点的中间结束。
         */
        truncated_sequence,
        /**
         * \lang english
         * @brief The decoded code point is a surrogate or outside the Unicode range.
         *
         * \lang simp-chinese
         * @brief 解码出的码点是代理项或超出 Unicode 范围。
         */
        invalid_code_point,
        /**
         * \lang english
         * @brief The sequence encodes a code point with more elements than necessary.
         *
         * \lang simp-chinese
         * @brief 序列用多于必要的元素编码了一个码点。
         */
        overlong_sequence
    };
}

namespace rainy::text {
    using foundation::text::unicode::encoding_error;
}

#endif
