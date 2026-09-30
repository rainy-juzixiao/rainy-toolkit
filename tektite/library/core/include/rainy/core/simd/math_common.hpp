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
#ifndef RAINY_CORE_SIMD_MATH_COMMON_HPP
#define RAINY_CORE_SIMD_MATH_COMMON_HPP

#include <rainy/core/simd/deduced_vec.hpp>
#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/type_traits/common_type.hpp>

namespace rainy::core::implements {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_math_floating_deduced_v = is_math_floating_point_v<typename deduced_vec<Ty>::type>;

    struct common_type_never {};

    template <typename Ty>
    using common_type_operand_t =
        type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Ty>, common_type_never, Ty>;

    template <typename A, typename B, typename = void>
    struct common_type2 {};

    template <typename A, typename B>
    struct common_type2<A, B,
                        type_traits::implements::void_t<
                            type_traits::primary_types::common_type_t<common_type_operand_t<A>, common_type_operand_t<B>>>> {
        using type = type_traits::primary_types::common_type_t<common_type_operand_t<A>, common_type_operand_t<B>>;
    };

    template <typename A, typename B, typename C, typename = void>
    struct common_type3 {};

    template <typename A, typename B, typename C>
    struct common_type3<A, B, C,
                        type_traits::implements::void_t<type_traits::primary_types::common_type_t<
                            common_type_operand_t<A>, common_type_operand_t<B>, common_type_operand_t<C>>>> {
        using type = type_traits::primary_types::common_type_t<common_type_operand_t<A>, common_type_operand_t<B>,
                                                               common_type_operand_t<C>>;
    };

    template <typename Ty0, typename Ty1, bool BothMath, bool Math0>
    struct math_common_selector2 {
        using sane = common_type2<Ty0, typename deduced_vec<Ty1>::type>;
    };

    template <typename Ty0, typename Ty1>
    struct math_common_selector2<Ty0, Ty1, false, true> {
        using sane = common_type2<typename deduced_vec<Ty0>::type, Ty1>;
    };

    template <typename Ty0, typename Ty1>
    struct math_common_selector2<Ty0, Ty1, true, true> {
        using sane = common_type2<typename deduced_vec<Ty0>::type, typename deduced_vec<Ty1>::type>;
    };

    template <typename Ty0, typename Ty1>
    using math_common_selector2_t = math_common_selector2<Ty0, Ty1, is_math_floating_deduced_v<Ty0> && is_math_floating_deduced_v<Ty1>,
                                                          is_math_floating_deduced_v<Ty0>>;

    template <typename Ty0, typename Ty1, typename = void>
    struct math_common_vec_impl2 {};

    template <typename Ty0, typename Ty1>
    struct math_common_vec_impl2<Ty0, Ty1, type_traits::implements::void_t<typename math_common_selector2_t<Ty0, Ty1>::sane::type>> {
        using type = typename math_common_selector2_t<Ty0, Ty1>::sane::type;
    };

    template <typename Ty0, typename Ty1, typename = void>
    struct has_math_common2 : type_traits::helper::false_type {};

    template <typename Ty0, typename Ty1>
    struct has_math_common2<Ty0, Ty1, type_traits::implements::void_t<typename math_common_vec_impl2<Ty0, Ty1>::type>>
        : type_traits::helper::true_type {};

    template <typename Ty0, typename Ty1, typename Ty2, bool PairValid>
    struct math_common_selector3 {
        using sane = common_type3<typename deduced_vec<Ty2>::type, Ty0, Ty1>;
    };

    template <typename Ty0, typename Ty1, typename Ty2>
    struct math_common_selector3<Ty0, Ty1, Ty2, true> {
        using sane = common_type2<typename math_common_vec_impl2<Ty0, Ty1>::type, Ty2>;
    };

    template <typename Ty0, typename Ty1, typename Ty2>
    using math_common_selector3_t = math_common_selector3<Ty0, Ty1, Ty2, has_math_common2<Ty0, Ty1>::value>;

    template <typename Ty0, typename Ty1, typename Ty2, typename = void>
    struct math_common_vec_impl3 {};

    template <typename Ty0, typename Ty1, typename Ty2>
    struct math_common_vec_impl3<Ty0, Ty1, Ty2,
                                  type_traits::implements::void_t<typename math_common_selector3_t<Ty0, Ty1, Ty2>::sane::type>> {
        using type = typename math_common_selector3_t<Ty0, Ty1, Ty2>::sane::type;
    };

    template <typename... Vs>
    struct math_common_vec {};

    template <typename Ty0>
    struct math_common_vec<Ty0> {
        using type = typename deduced_vec<Ty0>::type;
    };

    template <typename Ty0, typename Ty1>
    struct math_common_vec<Ty0, Ty1> : math_common_vec_impl2<Ty0, Ty1> {};

    template <typename Ty0, typename Ty1, typename Ty2>
    struct math_common_vec<Ty0, Ty1, Ty2> : math_common_vec_impl3<Ty0, Ty1, Ty2> {};
}

#endif
