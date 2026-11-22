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
#ifndef RAINY_CORE_MECHANISM_PATTERN_MATCHING_PRE_HPP
#define RAINY_CORE_MECHANISM_PATTERN_MATCHING_PRE_HPP
#include <rainy/core/container/tuple.hpp>

namespace rainy::core::mechanism::pattern_matching::implements {
    template <typename Ty, bool IsRef>
    struct value_type {
        using type = Ty;
    };

    template <typename Ty>
    struct value_type<Ty, true> {
        using type = Ty &&;
    };

    template <typename Ty, typename... Patterns>
    constexpr auto match_patterns(Ty &&value, const Patterns &...patterns);

    template <typename Ty, bool IsRef>
    class match_helper {
    public:
        using type = typename value_type<Ty, IsRef>::type;
        using reference_type = type &&;

        template <typename Arg>
        constexpr explicit match_helper(Arg &&value) : value(utility::forward<Arg>(value)) {
        }

        template <typename... pattern_pair>
        constexpr auto operator()(pattern_pair const &...patterns) -> decltype(auto) {
            return match_patterns(utility::forward<reference_type>(this->value), patterns...);
        }

    private:
        type value;
    };
}

namespace rainy::core::mechanism::pattern_matching {
    /**
     * \lang english
     * @brief Starts a match expression over a single value.
     *
     * The returned helper is callable with a list of `pattern = handler` arms; the first arm whose pattern accepts the value
     * is executed.
     *
     * \lang simp-chinese
     * @brief 对单个值开始一次模式匹配。
     *
     * 返回的辅助对象可以接收若干个 `pattern = handler` 分支，第一个接受该值的分支会被执行。
     */
    template <typename Value>
    constexpr rain_fn match(Value &&value) -> auto {
        return implements::match_helper<Value, true>{utility::forward<Value>(value)};
    }

    /**
     * \lang english
     * @brief Starts a match expression over several values, packed into a tuple of references.
     *
     * The packed tuple is what `ds` destructures, so a multi-value match is written as a single tuple pattern.
     *
     * \lang simp-chinese
     * @brief 对若干个值开始一次模式匹配，这些值会被打包成引用元组。
     *
     * 打包出的元组正是 `ds` 解构的对象，因此多值匹配写成一个元组模式即可。
     */
    template <typename First, typename... Values>
    constexpr rain_fn match(First &&first, Values &&...values) -> auto {
        auto result = utility::container::forward_as_tuple(utility::forward<First>(first), utility::forward<Values>(values)...);
        return implements::match_helper<decltype(result), false>{utility::forward<decltype(result)>(result)};
    }
}

#endif
