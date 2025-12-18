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
#ifndef RAINY_MOON_TUPLE_LIKE_TRAITS_HPP
#define RAINY_MOON_TUPLE_LIKE_TRAITS_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/pair.hpp>
#include <variant>
#include <tuple>

/*
[N4849 13.7.5 Class template partial specializations(temp.class.spec) - 10]:
    The usual access checking rules do not apply to non-dependent names used to specify template arguments of the simple-template-id of
the partial specialization. [Note: The template arguments may be private types or objects that would normally not be accessible.
Dependent names cannot be checked when declaring the partial specialization, but will be checked when substituting into the partial
specialization. —end note]

通常的访问检查规则不适用于用于指定显式实例化的名称。
[注意：
特别是，函数声明符中使用的模板参数和名称（包括参数类型、返回类型和异常规范）可能是通常无法访问的私有类型或对象。
而模板可能是通常无法访问的成员模板或成员函数。
-结束注释]
*/
namespace rainy::meta::moon::implements {
    template <typename Ty>
    struct private_access_tag_t {};

    template <typename Class, auto... Fields>
    struct private_access {
        friend inline constexpr auto get_private_ptrs(const private_access_tag_t<Class>&) {
            return std::make_tuple(Fields...);
        } 
    };

    template <typename Ty>
    constexpr private_access_tag_t<Ty> private_access_tag = {};
}

namespace rainy::meta::moon::implements {
    template <typename Ty, typename = void>
    struct has_get_private_ptrs : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_get_private_ptrs<Ty, type_traits::other_trans::void_t<decltype(get_private_ptrs(private_access_tag<type_traits::cv_modify::remove_cvref_t<Ty>>))>>
        : std::true_type {};

    template <typename Ty, typename = void>
    struct get_private_ptrs_helper {
        static constexpr auto value = std::make_tuple();
    };

    template <typename Ty>
    struct get_private_ptrs_helper<
        Ty, std::void_t<decltype(get_private_ptrs(private_access_tag<type_traits::cv_modify::remove_cvref_t<Ty>>))>> {
        static constexpr auto value = get_private_ptrs(private_access_tag<type_traits::cv_modify::remove_cvref_t<Ty>>);
    };
}

namespace rainy::meta::moon::implements {
    struct any_type {
        template <typename T>
        constexpr operator T() const { return {}; }
    };

    template <typename Ty, typename... Args>
    constexpr rain_fn test_constructible(int) -> decltype(Ty{utility::declval<Args>()..., any_type{}}, type_traits::helper::true_type{}) {
        std::terminate();
        return {};
    }

    template <typename Ty, typename... Args>
    constexpr rain_fn test_constructible(...) -> type_traits::helper::false_type {
        std::terminate();
        return {};
    }

    template <typename T, typename... Args>
    constexpr bool can_construct_with_one_more =
        decltype(test_constructible<T, Args...>(0))::value;

    template <typename T, typename... Args>
    constexpr std::size_t members_count_impl() {
        if constexpr (type_traits::type_properties::is_aggregate_v<T>) {
            if constexpr (can_construct_with_one_more<T, Args...>) {
                return members_count_impl<T, Args..., any_type>();
            } else {
                return sizeof...(Args);
            }
        }else {
            return 0;
        }
    }
}

namespace rainy::meta::moon {
    template <typename Ty, typename = void>
    struct reflectet_for_type {
        static constexpr bool invalid_mark = true;

        static constexpr inline std::size_t count = 0;

        static constexpr auto make() noexcept {
            return std::make_tuple();
        }

        static constexpr auto bind_obj(Ty &) noexcept {
            return std::make_tuple();
        }

        static constexpr auto member_names() noexcept {
            collections::array<std::string_view, 0> empty;
            return empty;
        }
    };

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL is_reflectet_for_type_valid = true;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL
        is_reflectet_for_type_valid<Type, type_traits::other_trans::void_t<decltype(reflectet_for_type<Type>::invalid_mark)>> = false;
}

namespace rainy::meta::moon::implements {
    template <std::size_t N, typename Ty>
    struct refl_to_tuple_impl;

    template <typename Ty>
    struct refl_to_tuple_impl<0, Ty> : std::integral_constant<std::size_t, 0> {
        static constexpr auto make() noexcept {
            return std::make_tuple();
        };

        using type = decltype(make());
    };
}

#define RAINY_DECLARE_TO_TUPLE(N)                                                                                                     \
    template <typename Ty>                                                                                                            \
    struct rainy::meta::moon::implements::refl_to_tuple_impl<N, Ty> : std::integral_constant<std::size_t, N> {                      \
        static constexpr auto make() noexcept {                                                                                       \
            auto &[RAINY_TO_TUPLE_EXPAND_ARGS(N)] =                                                                                   \
                type_traits::helper::get_fake_object<type_traits::cv_modify::remove_cvref_t<Ty>>();                                   \
            auto ref_tup = std::tie(RAINY_TO_TUPLE_EXPAND_ARGS(N));                                                                   \
            auto get_ptrs = [](auto &..._refs) { return std::make_tuple(&_refs...); };                                                \
            return std::apply(get_ptrs, ref_tup);                                                                                     \
        }                                                                                                                             \
        template <typename UTy,                                                                                                       \
                  rainy::type_traits::other_trans::enable_if_t<                                                                       \
                      rainy::type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<UTy>, Ty>, int> = 0>       \
        static constexpr auto make_ptr(UTy &&obj) noexcept {                                                                          \
            auto &[RAINY_TO_TUPLE_EXPAND_ARGS(N)] = obj;                                                                              \
            auto ref_tup = std::tie(RAINY_TO_TUPLE_EXPAND_ARGS(N));                                                                   \
            auto get_ptrs = [](auto &..._refs) { return std::make_tuple(&_refs...); };                                                \
            return std::apply(get_ptrs, ref_tup);                                                                                     \
        }                                                                                                                             \
        using type = decltype(make());                                                                                                \
    };

RAINY_GENERATE_MACRO_FOR_256(RAINY_DECLARE_TO_TUPLE)

#undef RAINY_DECLARE_TO_TUPLE

namespace rainy::meta::moon {
    template <typename Ty>
    struct member_count {
        static constexpr rain_fn eval() noexcept -> std::size_t {
            if constexpr(is_reflectet_for_type_valid<type_traits::cv_modify::remove_cvref_t<Ty>>) {
                return reflectet_for_type<type_traits::cv_modify::remove_cvref_t<Ty>>::count;
            }else {
                return implements::members_count_impl<type_traits::cv_modify::remove_cvref_t<Ty>>();
            }
        }

        static constexpr std::size_t value = eval();
    };

    template <typename Ty>
    static inline constexpr std::size_t member_count_v = member_count<Ty>::value;

    template <typename Ty>
    constexpr auto struct_to_tuple() {
        constexpr std::size_t count = member_count_v<type_traits::cv_modify::remove_cvref_t<Ty>>;
        if constexpr (is_reflectet_for_type_valid<Ty>) {
            return reflectet_for_type<Ty>::make();
        } else if constexpr (count != 0 && type_traits::type_properties::is_aggregate_v<Ty>) {
            return implements::refl_to_tuple_impl<count, Ty>::make();
        } else {
            return std::make_tuple();
        }
    }

    template <std::size_t Idx, typename Ty>
    struct tuple_element {
        using type = std::tuple_element_t<Idx, decltype(struct_to_tuple<Ty>())>;
    };

    template <std::size_t Idx, typename Ty>
    using tuple_element_t = std::tuple_element_t<Idx, decltype(struct_to_tuple<type_traits::cv_modify::remove_cvref_t<Ty>>())>;
}

namespace rainy::utility {
    using meta::moon::member_count;
    using meta::moon::member_count_v;
    using meta::moon::struct_to_tuple;
}

namespace rainy::meta::moon {
    /**
     * @brief 尝试获取指定类型中所有成员的名称
     * @tparam Ty 要获取的类型
     * @remark 如果使用RAINY_REFLECT_TUPLE_LIKE或RAINY_PRIVATE_REFLECT_TUPLE_LIKE注册了一个类型，那么此方法将返回注册宏中指定的成员名称
     *
     * @attention get_member_names以及其依赖的函数在IDE中可能会导致错误的结果在constexpr求值中，但实际编译期求值会得到正确的结果
     * @attention 因此，如果对IDE中的结果有洁癖，请尽可能避免获取字符串，除非，你定义了注册，这样才能确保IDE生成正确的结果
     */
    template <typename Ty>
    constexpr auto get_member_names() noexcept {
        if constexpr (is_reflectet_for_type_valid<type_traits::cv_modify::remove_cvref_t<Ty>>) {
            return reflectet_for_type<type_traits::cv_modify::remove_cvref_t<Ty>>::member_names();
        } else {
#if RAINY_HAS_CXX20
            constexpr bool has_get_private_ptrs_v = implements::has_get_private_ptrs<Ty>::value;
            static_assert(member_count_v<Ty> != 0 || has_get_private_ptrs_v, "Failed!");
            if constexpr (!has_get_private_ptrs_v) {
                collections::array<std::string_view, member_count_v<Ty>> array{}; // 创建对应的数组
                constexpr auto tp = struct_to_tuple<Ty>();
                [&array, &tp]<std::size_t... I>(std::index_sequence<I...>) mutable {
                    ((array[I] = foundation::ctti::variable_name<(std::get<I>(tp))>()), ...);
                }(std::make_index_sequence<member_count_v<Ty>>{});
                return array;
            } else {
                constexpr auto tp = implements::get_private_ptrs_helper<Ty>::value; // 使用 helper 获取 `tp`
                constexpr std::size_t tuple_size = std::tuple_size_v<decltype(tp)>;
                collections::array<std::string_view, tuple_size> array{};
                [&array, &tp]<std::size_t... I>(std::index_sequence<I...>) mutable {
                    ((array[I] = foundation::ctti::variable_name<(std::get<I>(tp))>()), ...);
                }(std::make_index_sequence<tuple_size>{});
                return array;
            }
#else
            static_assert(type_traits::implements::always_false<Ty>,
                          "cannot get all the member names of Ty, please use marco RAINY_REFLECT_TUPLE_LIKE() to register");
#endif
        }
    }
}

namespace rainy::meta::moon {
    /**
     * @brief 将对象绑定为指针元组
     * @return 成员指针元组
     */
    template <typename Ty>
    constexpr auto struct_bind_tuple(Ty &obj) {
        constexpr std::size_t count = member_count_v<type_traits::cv_modify::remove_cvref_t<Ty>>;
        if constexpr (is_reflectet_for_type_valid<type_traits::cv_modify::remove_cvref_t<Ty>>) {
            return reflectet_for_type<Ty>::bind_obj(obj);
        } else if constexpr (count != 0 && type_traits::type_properties::is_aggregate_v<Ty>) {
            return implements::refl_to_tuple_impl<count, Ty>::make_ptr(obj);
        } else {
            return std::make_tuple();
        }
    }
}

namespace rainy::utility {
    using meta::moon::struct_bind_tuple;
}

namespace rainy::meta::moon::implements {
    template <typename Tuple>
    struct tuple_traits_impl {
        static inline constexpr bool invalid_mark = true;
    };

    template <typename... Types>
    struct tuple_traits_impl<std::tuple<Types...>> {
        using type = std::tuple<Types...>;

        template <std::size_t Idx>
        using element = std::tuple_element<Idx, type>;

        template <std::size_t Idx>
        using element_t = std::tuple_element<Idx, type>;

        static inline constexpr std::size_t size = sizeof...(Types);
    };

    template <typename Tuple>
    struct pair_traits_impl {
        static inline constexpr bool invalid_mark = true;
    };

    template <typename Ty1, typename Ty2>
    struct pair_traits_impl<std::pair<Ty1, Ty2>> {
        using type = std::pair<Ty1, Ty2>;
        using first_type = Ty1;
        using second_type = Ty2;
        static inline constexpr std::size_t size = 2;
    };

    template <typename Ty1, typename Ty2>
    struct pair_traits_impl<utility::pair<Ty1, Ty2>> {
        using type = utility::pair<Ty1, Ty2>;
        using first_type = Ty1;
        using second_type = Ty2;
        static inline constexpr std::size_t size = 2;
    };
}

namespace rainy::meta::moon {
    template <typename Tuple>
    struct tuple_traits : implements::tuple_traits_impl<type_traits::cv_modify::remove_cvref_t<Tuple>> {};

    template <typename Tuple, typename = void>
    RAINY_CONSTEXPR_BOOL is_tuple_v = true;

    template <typename Tuple>
    RAINY_CONSTEXPR_BOOL is_tuple_v<Tuple, type_traits::other_trans::void_t<decltype(tuple_traits<Tuple>::invalid_mark)>> = false;

    template <typename Tuple>
    struct is_tuple : type_traits::helper::bool_constant<is_tuple_v<Tuple>> {};

    template <typename Pair>
    struct pair_traits : implements::pair_traits_impl<type_traits::cv_modify::remove_cvref_t<Pair>> {};

    template <typename Pair, typename = void>
    RAINY_CONSTEXPR_BOOL is_pair_v = true;

    template <typename Pair>
    RAINY_CONSTEXPR_BOOL is_pair_v<Pair, type_traits::other_trans::void_t<decltype(pair_traits<Pair>::invalid_mark)>> = false;

    template <typename Pair>
    struct is_pair : type_traits::helper::bool_constant<is_pair_v<Pair>> {};
}

namespace rainy::type_traits::primary_types {
    using meta::moon::is_pair;
    using meta::moon::is_pair_v;
    using meta::moon::is_tuple;
    using meta::moon::is_tuple_v;
    using meta::moon::pair_traits;
    using meta::moon::tuple_traits;
}

namespace rainy::meta::moon {
    template <typename Ty1, typename Ty2>
    struct reflectet_for_type<std::pair<Ty1, Ty2>> {
        static constexpr inline std::size_t count = 2;

        static constexpr auto make() noexcept {
            constexpr auto pair = type_traits::helper::get_fake_object<std::pair<Ty1, Ty2>>();
            return std::make_tuple(&pair.first, &pair.second);
        }

        static constexpr auto bind_obj(std::pair<Ty1, Ty2> &obj) noexcept {
            return std::make_tuple(&obj.first, &obj.second);
        }

        static constexpr auto member_names() noexcept {
            collections::array<std::string_view, 2> names = {"first", "second"};
            return names;
        }
    };

    template <typename Ty1, typename Ty2>
    struct reflectet_for_type<utility::pair<Ty1, Ty2>> {
        static constexpr inline std::size_t count = 2;

        static constexpr auto make() noexcept {
            constexpr auto pair = type_traits::helper::get_fake_object<utility::pair<Ty1, Ty2>>();
            return std::make_tuple(&pair.first, &pair.second);
        }

        static constexpr auto bind_obj(utility::pair<Ty1, Ty2> &obj) noexcept {
            return std::make_tuple(&obj.first, &obj.second);
        }

        static constexpr auto member_names() noexcept {
            collections::array<std::string_view, 2> names = {"first", "second"};
            return names;
        }
    };

    template <typename Tuple>
    struct reflectet_for_type<Tuple, type_traits::other_trans::enable_if_t<is_tuple_v<Tuple>>> {
        static constexpr inline std::size_t count = tuple_traits<type_traits::cv_modify::remove_cvref_t<Tuple>>::size;

        static constexpr auto make() noexcept {
            return implements::refl_to_tuple_impl<count, Tuple>::make();
        }

        static constexpr auto bind_obj(Tuple &obj) noexcept {
            return implements::refl_to_tuple_impl<count, Tuple>::make_ptr(obj);
        }
    };

    template <typename Ty>
    struct reflectet_for_type<Ty, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_array_v<Ty>>> {
        static constexpr inline std::size_t count = type_traits::primary_types::array_size_v<Ty>;

        static constexpr auto make() noexcept {
            return implements::refl_to_tuple_impl<count, Ty>::make();
        }

        static constexpr auto bind_obj(Ty &obj) noexcept {
            return implements::refl_to_tuple_impl<count, Ty>::make_ptr(obj);
        }

        static constexpr auto member_names() noexcept {
            collections::array<std::string_view, 0> empty;
            return empty;
        }
    };

    template <template <typename Ty, std::size_t N> typename ArrayTemplate, typename Ty, std::size_t N>
    struct reflectet_for_type<
        ArrayTemplate<Ty, N>,
        type_traits::other_trans::void_t<
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_aggregate_v<ArrayTemplate<Ty, N>>>,
            typename ArrayTemplate<Ty, N>::value_type, typename ArrayTemplate<Ty, N>::iterator>> {
        static constexpr inline std::size_t count = N;

        static constexpr auto make() noexcept {
            return implements::refl_to_tuple_impl<count, ArrayTemplate<Ty, N>>::make();
        }

        static constexpr auto bind_obj(ArrayTemplate<Ty, N> &obj) noexcept {
            return implements::refl_to_tuple_impl<count, ArrayTemplate<Ty, N>>::make_ptr(obj);
        }

        static constexpr auto member_names() noexcept {
            collections::array<std::string_view, 0> empty;
            return empty;
        }
    };

    template <template <typename Ty, std::size_t N> typename ArrayTemplate, typename Ty, std::size_t N>
    struct reflectet_for_type<ArrayTemplate<Ty, N>, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<
                                                        ArrayTemplate<Ty, N>, collections::array<Ty, N>>>> {
        static constexpr inline std::size_t count = N;

        static constexpr auto make() noexcept {
            if constexpr (count <= 256) {
                return implements::refl_to_tuple_impl<count, Ty[count]>::make();
            } else {
                return std::make_tuple();
            }
        }

        static constexpr auto bind_obj(ArrayTemplate<Ty, N> &obj) noexcept {
            if constexpr (count <= 256) {
                return implements::refl_to_tuple_impl<count, Ty[count]>::make_ptr(obj.access_carrays());
            } else {
                return std::make_tuple();
            }
        }

        static constexpr auto member_names() noexcept {
            collections::array<std::string_view, 0> empty;
            return empty;
        }
    };
}

#define RAINY_PTR_EACH(obj, member) &obj.member
#define RAINY_NAME_EACH(obj, member) #member

#define RAINY_REFLECT_TUPLE_LIKE(STRUCT, ...)                                                                                         \
    template <>                                                                                                                       \
    struct rainy::meta::moon::reflectet_for_type<STRUCT> {                                                                            \
        static constexpr inline std::size_t count = RAINY_ARG_COUNT(__VA_ARGS__);                                                     \
                                                                                                                                      \
        static constexpr auto make() noexcept {                                                                                       \
            auto &obj = type_traits::helper::get_fake_object<STRUCT>();                                                               \
            return std::make_tuple(RAINY_FE_FOR_EACH(RAINY_PTR_EACH, obj, __VA_ARGS__));                                              \
        }                                                                                                                             \
                                                                                                                                      \
        static constexpr auto bind_obj(STRUCT &obj) noexcept {                                                                        \
            return std::make_tuple(RAINY_FE_FOR_EACH(RAINY_PTR_EACH, obj, __VA_ARGS__));                                              \
        }                                                                                                                             \
                                                                                                                                      \
        static constexpr auto member_names() noexcept {                                                                               \
            collections::array<std::string_view, count> names = {RAINY_FE_FOR_EACH(RAINY_NAME_EACH, dummy, __VA_ARGS__)};             \
            return names;                                                                                                             \
        }                                                                                                                             \
    };

#define RAINY_MEMPTR_EACH(TYPE, member) &TYPE::member
#define RAINY_PRIVATE_PTR_EACH(obj, ptr) (obj.*ptr)

#define RAINY_PRIVATE_REFLECT_TUPLE_LIKE(TYPE, ...)                                                                                   \
    namespace rainy::meta::moon::implements {                                                                                         \
        template struct private_access<TYPE, RAINY_FE_FOR_EACH(RAINY_MEMPTR_EACH, TYPE, __VA_ARGS__)>;                                \
        inline constexpr auto get_private_ptrs(const private_access_tag_t<TYPE> &);                                                   \
    }                                                                                                                                 \
                                                                                                                                      \
    template <>                                                                                                                       \
    struct rainy::meta::moon::reflectet_for_type<TYPE> {                                                                              \
        static constexpr inline std::size_t count = RAINY_ARG_COUNT(__VA_ARGS__);                                                     \
                                                                                                                                      \
        static constexpr auto make() noexcept {                                                                                       \
            using Ty = TYPE;                                                                                                          \
            auto &obj = type_traits::helper::get_fake_object<Ty>();                                                                   \
            constexpr auto ptrs = implements::get_private_ptrs_helper<TYPE>::value;                                                   \
            return std::apply([&](auto... ptr) { return std::make_tuple(utility::addressof(obj.*ptr)...); }, ptrs);                   \
        }                                                                                                                             \
                                                                                                                                      \
        static constexpr auto bind_obj(TYPE &obj) noexcept {                                                                          \
            using Ty = TYPE;                                                                                                          \
            constexpr auto ptrs = implements::get_private_ptrs_helper<TYPE>::value;                                                   \
            return std::apply([&](auto... ptr) { return std::make_tuple(utility::addressof(obj.*ptr)...); }, ptrs);                   \
        }                                                                                                                             \
                                                                                                                                      \
        static constexpr auto member_names() noexcept {                                                                               \
            collections::array<std::string_view, count> names = {RAINY_FE_FOR_EACH(RAINY_NAME_EACH, dummy, __VA_ARGS__)};             \
            return names;                                                                                                             \
        }                                                                                                                             \
    };

namespace rainy::meta::moon::implements {
    template <typename Tuple, typename NamesArray, typename Fx, std::size_t... Idx>
    constexpr rain_fn for_each_impl(Tuple &&tuple, NamesArray &&names, Fx &&func, type_traits::helper::index_sequence<Idx...>)
        -> void {
        (utility::invoke(utility::forward<Fx>(func), *std::get<Idx>(utility::forward<Tuple>(tuple)),
                         std::get<Idx>(utility::forward<NamesArray>(names)), Idx),
         ...);
    }

    template <typename Tuple, typename Fx, std::size_t... Idx>
    constexpr rain_fn visit_members_impl(Tuple &&tuple, Fx &&func, type_traits::helper::index_sequence<Idx...>) -> void {
        utility::invoke(utility::forward<Fx>(func), *std::get<Idx>(utility::forward<Tuple>(tuple))...);
    }
}

namespace rainy::meta::moon::implements {
    template <typename Ty, typename Tuple, size_t... Is>
    auto get_member_offset_arr_impl(Ty &t, Tuple &tp, type_traits::helper::index_sequence<Is...>) {
        collections::array<std::size_t, sizeof...(Is)> arr;
        ((arr[Is] = static_cast<std::size_t>(reinterpret_cast<const char *>(std::get<Is>(tp)) - reinterpret_cast<char *>(&t))), ...);
        return arr;
    }
}

namespace rainy::meta::moon {
    template <typename Struct>
    constexpr rain_fn tuple_to_variant() noexcept {
        using tuple_type_list = typename type_traits::other_trans::tuple_like_to_type_list<decltype(struct_to_tuple<Struct>())>::type;
        using unique_type_list = type_traits::other_trans::unique_type_list_t<tuple_type_list>;
        using variant_type = typename type_traits::other_trans::type_list_to_tuple_like<unique_type_list, std::variant>::type;
        return variant_type{};
    }

    template <typename Ty>
    rain_fn get_member_offset_arr(Ty &&t)
        -> annotations::lifetime::static_read_only<collections::array<std::size_t, member_count_v<Ty>>> {
        constexpr size_t count = member_count_v<Ty>;
        auto tp = struct_bind_tuple(utility::forward<Ty>(t));
        static collections::array<size_t, count> arr =
            implements::get_member_offset_arr_impl(t, tp, type_traits::helper::make_index_sequence<count>{});
        return arr;
    };

    template <typename Ty>
    RAINY_INLINE rain_fn get_member_offset_arr()
        -> annotations::lifetime::static_read_only<collections::array<std::size_t, member_count_v<Ty>>> {
        return get_member_offset_arr(type_traits::helper::get_fake_object<Ty>());
    }

    template <typename Type, typename Fx>
    constexpr rain_fn for_each(Fx &&func) -> void {
        if constexpr (type_traits::type_properties::is_invocable_v<Fx, std::string_view, std::size_t>) {
            auto names = get_member_names<Type>();
            for (std::size_t idx = 0; idx < names.size(); ++idx) {
                utility::invoke(utility::forward<Fx>(func), names[idx], idx);
            }
        } else if constexpr (type_traits::type_properties::is_invocable_v<Fx, std::string_view>) {
            for (const auto &item: get_member_names<Type>()) {
                utility::invoke(utility::forward<Fx>(func), item);
            }
        } else {
            static_assert(type_traits::implements::always_false<Fx>, "Cannot accept this func");
        }
    }

    template <typename Object, typename Fx>
    constexpr rain_fn for_each(Object &&object, Fx &&func) -> void {
        auto tuple = struct_bind_tuple(object);
        implements::for_each_impl(tuple, get_member_names<Object>(), utility::forward<Fx>(func),
                                  type_traits::helper::make_index_sequence<member_count_v<Object>>{});
    }

    template <typename Object, typename Visitor>
    constexpr rain_fn visit_members(Object &&object, Visitor &&visitor) -> void {
        auto tuple = struct_bind_tuple(object);
        implements::visit_members_impl(tuple, utility::forward<Visitor>(visitor),
                                       type_traits::helper::make_index_sequence<member_count_v<Object>>{});
    }

    template <typename Ty, std::size_t Idx>
    constexpr rain_fn name_of() noexcept -> std::string_view {
        auto names = get_member_names<Ty>();
        return std::get<Idx>(names);
    }

    template <typename Ty>
    constexpr rain_fn name_of(std::size_t index) noexcept -> std::string_view {
        auto names = get_member_names<Ty>();
        if (index < names.size()) {
            return names[index];
        }
        return {};
    }

    template <typename Ty>
    constexpr rain_fn index_of(std::string_view name) noexcept -> std::size_t {
        auto names = get_member_names<Ty>();
        std::size_t idx{0};
        for (const std::string_view &item: names) {
            if (name == item) {
                return idx;
            }
            ++idx;
        }
        return idx;
    }

    template <typename Ty, type_traits::helper::basic_constexpr_string String>
    constexpr rain_fn index_of() noexcept -> std::size_t {
        return index_of<Ty>({String.data(), String.length()});
    }

    template <std::size_t Idx, typename Ty>
    constexpr rain_fn get(Ty &&object) noexcept -> decltype(auto) {
        return (*std::get<Idx>(struct_bind_tuple(object)));
    }

    template <typename Ty>
    constexpr rain_fn get(Ty &&object, std::size_t idx) -> decltype(tuple_to_variant<type_traits::cv_modify::remove_cvref_t<Ty>>()) {
        auto tuple = struct_bind_tuple(object);
        auto ret = tuple_to_variant<type_traits::cv_modify::remove_cvref_t<Ty>>();
        constexpr auto find_fn = [](auto &&ret, const std::size_t index_, auto &&extract) {
            // NOLINT BEGIN
            std::apply(
                [&](auto &&...elems) { // NOLINT
                    std::size_t i{0};
                    (void) ((i++ == index_ ? ret = elems, true : false) || ...);
                },
                extract);
            // NOLINT END
        };
        find_fn(ret, idx, tuple);
        return ret;
    }

    template <typename Ty>
    constexpr rain_fn get(Ty &&object, std::string_view name)
        -> decltype(tuple_to_variant<type_traits::cv_modify::remove_cvref_t<Ty>>()) {
        constexpr std::size_t index = index_of<Ty>(name);
        return get<Ty>(utility::forward<Ty>(object), index);
    }

#if RAINY_HAS_CXX20
    template <type_traits::helper::basic_constexpr_string String, typename Ty>
    constexpr rain_fn get(Ty &&object) noexcept -> decltype(auto) {
        constexpr std::size_t index = index_of<Ty, String>();
        return get<index>(utility::forward<Ty>(object));
    }
#endif
}

namespace rainy::meta::moon {
    template <typename Ty>
    struct tuple_size : type_traits::helper::integral_constant<std::size_t, member_count_v<Ty>> {};

    template <typename Ty>
    static inline constexpr std::size_t tuple_size_v = tuple_size<Ty>::value;
}

#endif
