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
#ifndef RAINY_FOUNDATION_TEXT_BAD_LEXICAL_CAST_HPP
#define RAINY_FOUNDATION_TEXT_BAD_LEXICAL_CAST_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/format.hpp>
#include <rainy/core/text/string_view.hpp>

namespace rainy::foundation::exceptions::logic {
    /**
     * \lang english
     * @brief Thrown when a text sequence cannot be interpreted as the requested target type.
     *
     * @remark This type deliberately deviates from `boost::bad_lexical_cast`: the toolkit unifies every
     *         exception under its own hierarchy, so this one derives from `core::exceptions::logic::logic_error`
     *         instead of `std::bad_cast` and does not carry `type_info` pairs.
     *
     * \lang simp-chinese
     * @brief 文本序列无法被解释为请求的目标类型时抛出。
     *
     * @remark 本类型有意偏离 `boost::bad_lexical_cast`：工具包将所有异常统一到自有异常体系，
     *         因此它派生自 `core::exceptions::logic::logic_error` 而非 `std::bad_cast`，
     *         也不携带 `type_info` 对。
     */
    class bad_lexical_cast : public core::exceptions::logic::logic_error {
    public:
        using base = core::exceptions::logic::logic_error;

        /**
         * \lang english
         * @brief Constructs the exception.
         * @param operation The name of the conversion that failed
         * @param message The reason of the failure
         * @param location The source location where the failure is reported
         *
         * \lang simp-chinese
         * @brief 构造该异常。
         * @param operation 失败的转换名称
         * @param message 失败原因
         * @param location 报告失败处的源码位置
         */
        RAINY_NODISCARD explicit bad_lexical_cast(core::text::string_view operation, core::text::string_view message,
                                                  const source &location = core::diagnostics::source_location::current()) :
            base(core::text::format("bad lexical cast in '{}': {}", operation, message).c_str(), location) {
        }
    };

    /**
     * \lang english
     * @brief Throws bad_lexical_cast.
     * @param operation The name of the conversion that failed
     * @param message The reason of the failure
     *
     * \lang simp-chinese
     * @brief 抛出 bad_lexical_cast。
     * @param operation 失败的转换名称
     * @param message 失败原因
     */
    RAINY_INLINE void throw_bad_lexical_cast(core::text::string_view operation, core::text::string_view message) {
        core::exceptions::throw_exception_if<bad_lexical_cast>(false, operation, message);
    }
}

#endif
