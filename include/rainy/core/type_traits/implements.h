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
#ifndef RAINY_CORE_TYPE_TRAITS_IMPLEMENTS_H
#define RAINY_CORE_TYPE_TRAITS_IMPLEMENTS_H
#include <rainy/core/platform.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4180)
#endif

namespace rainy::type_traits::implements {
    template <typename Ty1, typename Ty2>
    RAINY_CONSTEXPR_BOOL is_same_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_same_v<Ty, Ty> = true;

    template <typename Ty, typename... Types>
    RAINY_CONSTEXPR_BOOL is_any_of_v = (is_same_v<Ty, Types> || ...);

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_reference_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_reference_v<Ty &> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_reference_v<Ty &&> = true;

    template <typename>
    RAINY_CONSTEXPR_BOOL _is_const_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_const_v<const Ty> = true;

    template <typename Ty>
    constexpr bool _is_function_v = !_is_const_v<const Ty> && !_is_reference_v<Ty>;

    template <typename Ty>
    struct remove_reference {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_reference<Ty &> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_reference<Ty &&> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_reference_t = typename remove_reference<Ty>::type;

    template <typename Ty>
    struct remove_volatile {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_volatile<volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<const Ty> {
        using type = Ty;
    };

    template <typename Ty>
    struct remove_cv<const volatile Ty> {
        using type = Ty;
    };

    template <typename Ty>
    using remove_cv_t = typename remove_cv<Ty>::type;

    template <typename Ty>
    using remove_cvref_t = remove_cv_t<remove_reference_t<Ty>>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_integral_v =
        is_any_of_v<remove_cv_t<Ty>, bool, char, signed char, unsigned char, wchar_t,
#if RAINY_HAS_CXX20 && defined(__cpp_lib_char8_t)
                    char8_t,
#endif
                    char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_floating_point_v = is_any_of_v<remove_cv_t<Ty>, float, double, long double>;


    template <typename>
    RAINY_CONSTEXPR_BOOL _is_pointer_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *const> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *volatile> = true;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL _is_pointer_v<Ty *const volatile> = true;
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif