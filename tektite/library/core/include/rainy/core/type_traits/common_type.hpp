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
#ifndef RAINY_CORE_TYPE_TRAITS_COMMON_TYPE_HPP
#define RAINY_CORE_TYPE_TRAITS_COMMON_TYPE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/decay.hpp>

namespace rainy::type_traits::primary_types {
    template <typename...>
    struct common_type {};

    template <typename Ty>
    struct common_type<Ty> : common_type<Ty, Ty> {};
}

namespace rainy::type_traits::implemennts {
    template <typename Ty1, typename Ty2>
    using conditional_result_t = decltype(false ? utility::declval<Ty1>() : utility::declval<Ty2>());

    template <typename, typename, typename = void>
    struct decay_conditional_result {};
    template <typename Ty1, typename Ty2>
    struct decay_conditional_result<Ty1, Ty2, other_trans::void_t<conditional_result_t<Ty1, Ty2>>>
        : other_trans::decay<conditional_result_t<Ty1, Ty2>> {};

    template <typename Ty1, typename Ty2, typename = void>
    struct common_type_2_impl : decay_conditional_result<const Ty1 &, const Ty2 &> {};

    template <typename Ty1, typename Ty2>
    struct common_type_2_impl<Ty1, Ty2, other_trans::void_t<conditional_result_t<Ty1, Ty2>>> : decay_conditional_result<Ty1, Ty2> {};

    template <typename AlwaysVoid, typename Ty1, typename Ty2, typename... R>
    struct common_type_multi_impl {};
    template <typename Ty1, typename Ty2, typename... R>
    struct common_type_multi_impl<other_trans::void_t<typename primary_types::common_type<Ty1, Ty2>::type>, Ty1, Ty2, R...>
        : primary_types::common_type<typename primary_types::common_type<Ty1, Ty2>::type, R...> {};
}

namespace rainy::type_traits::primary_types {
    template <typename Ty1, typename Ty2>
    struct common_type<Ty1, Ty2>
        : other_trans::conditional_t<
              type_relations::is_same_v<Ty1, other_trans::decay_t<Ty1>> && type_relations::is_same_v<Ty2, other_trans::decay_t<Ty2>>,
              implemennts::common_type_2_impl<Ty1, Ty2>, common_type<other_trans::decay_t<Ty1>, typename other_trans::decay_t<Ty2>>> {
    };

    template <typename Ty1, typename Ty2, typename... R>
    struct common_type<Ty1, Ty2, R...> : implemennts::common_type_multi_impl<void, Ty1, Ty2, R...> {};

    template <typename... Types>
    using common_type_t = typename common_type<Types...>::type;
};

#endif
