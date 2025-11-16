/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_MATCHER_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_MATCHER_HPP

#include <rainy/core/core.hpp>
#include <rainy/utility/implements/any/fwd.hpp>

namespace rainy::utility::implements {
    template <typename BasicAny, typename... Handlers>
    class any_matcher {
    public:
        using basic_any = BasicAny;

        explicit any_matcher(Handlers &&...hs) : handlers(utility::forward<Handlers>(hs)...) {
        }

        basic_any invoke(basic_any const &a) const noexcept {
            return invoke_impl(a, std::make_index_sequence<sizeof...(Handlers)>{});
        }

    private:
        template <std::size_t I>
        bool try_to_invoke(basic_any const &this_, basic_any &result) const noexcept {
            using namespace type_traits;
            using namespace type_traits::primary_types;
            using handler_t = std::tuple_element_t<I, std::tuple<Handlers...>>;
            using argument_t = std::tuple_element_t<0, typename function_traits<handler_t>::tuple_like_type>;
            if constexpr (type_relations::is_same_v<argument_t, any_default_match>) {
                return false; // 最后处理
            } else {
                auto &handler = std::get<I>(handlers);
                if (this_.template is<argument_t>()) {
                    if constexpr (type_relations::is_void_v<function_return_type<handler_t>>) {
                        utility::invoke(handler, this_.template as<argument_t>());
                    } else {
                        result = utility::invoke(handler, this_.template as<argument_t>());
                    }
                    return true;
                }
                if (this_.template is_convertible<argument_t>()) {
                    if constexpr (type_relations::is_void_v<function_return_type<handler_t>>) {
                        utility::invoke(handler, this_.template convert<argument_t>());
                    } else {
                        result = utility::invoke(handler, this_.template convert<argument_t>());
                    }
                    return true;
                }
                return false;
            }
        }

        template <std::size_t... Idx>
        static constexpr std::size_t eval_for_default_match_index(std::index_sequence<Idx...>) {
            using namespace type_traits;
            using namespace type_traits::other_trans;
            using type_list = type_list<
                typename type_at<0, typename tuple_like_to_type_list<primary_types::param_list_in_tuple<Handlers>>::type>::type...>;
            std::size_t index{};
            ((type_relations::is_same_v<cv_modify::remove_cvref_t<typename type_at<Idx, type_list>::type>, any_default_match>
              ? index = Idx,
              void() : void()),
             ...);
            return index;
        }

        template <std::size_t... Idx>
        static constexpr std::size_t eval_for_default_match(std::index_sequence<Idx...>) {
            using namespace type_traits;
            using namespace type_traits::other_trans;
            using type_list = type_list<
                typename type_at<0, typename tuple_like_to_type_list<primary_types::param_list_in_tuple<Handlers>>::type>::type...>;
            std::size_t count{};
            ((count += type_relations::is_same_v<cv_modify::remove_cvref_t<typename type_at<Idx, type_list>::type>, any_default_match>),
             ...);
            return count;
        }

        template <std::size_t... I>
        basic_any invoke_impl(basic_any const &a, std::index_sequence<I...>) const noexcept {
            using namespace type_traits;
            basic_any result{};
            if (!a.has_value()) {
                return result;
            }
            const bool matched = (try_to_invoke<I>(a, result) || ...);
            constexpr std::size_t default_match_count = eval_for_default_match(std::make_index_sequence<sizeof...(Handlers)>{});
            static_assert(default_match_count <= 1, "You can't add 2 and more default_match!");
            if (!matched) {
                if constexpr (default_match_count == 1) {
                    constexpr std::size_t index = eval_for_default_match_index(std::make_index_sequence<sizeof...(Handlers)>{});
                    auto &handler = std::get<index>(handlers);
                    using handler_t = decltype(handler);
                    static constexpr any_default_match default_match_obj{};
                    using type_list = typename other_trans::tuple_like_to_type_list<
                        primary_types::param_list_in_tuple<cv_modify::remove_cvref_t<handler_t>>>::type;
                    if constexpr (constexpr std::size_t arity = other_trans::type_list_size_v<type_list>; arity == 1) {
                        handler(default_match_obj);
                    } else if constexpr (arity == 2) {
                        static_assert(type_traits::type_relations::is_convertible_v<const basic_any &,
                                                                                    typename other_trans::type_at<1, type_list>::type>,
                                      "param(2) not match");
                        handler(default_match_obj, a);
                    } else {
                        static_assert(type_traits::implements::always_false<handler_t>, "handler param lists not match!");
                    }
                }
            }
            return result;
        }

        std::tuple<Handlers...> handlers;
    };
}

#endif