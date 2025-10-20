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
#ifndef RAINY_UTILITY_TUPLE_LIKE_TRAITS_HPP
#define RAINY_UTILITY_TUPLE_LIKE_TRAITS_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/pair.hpp>

/*
[N4849 13.7.5 Class template partial specializations(temp.class.spec) - 10]:
    The usual access checking rules do not apply to non-dependent names used to specify template arguments of the simple-template-id of
the partial specialization. [Note: The template arguments may be private types or objects that would normally not be accessible.
Dependent names cannot be checked when declaring the partial specialization, but will be checked when substituting into the partial
specialization. —end note] 通常的访问检查规则不适用于用于指定显式实例化的名称。
    [注意：特别是，函数声明符中使用的模板参数和名称（包括参数类型、返回类型和异常规范）可能是通常无法访问的私有类型或对象。而模板可能是通常无法访问的成员模板或成员函数。
-结束注释]
*/
namespace rainy::utility {
    template <typename Ty>
    struct private_access_tag_t {};

    template <typename Class, auto... Fields>
    struct private_access {
        friend inline constexpr auto get_private_ptrs(private_access_tag_t<Class>) {
            return std::make_tuple(Fields...);
        }
    };

    template <typename Ty>
    inline constexpr private_access_tag_t<Ty> private_access_tag;
}

#define RAINY_MAKE_PRIVATE_ACCESSIBLE(TYPE, ...)                                                                                      \
    namespace rainy::utility {                                                                                                        \
        template <>                                                                                                                   \
        private_access<TYPE, ##__VA_ARGS__>;                                                                                          \
        inline constexpr auto get_private_ptrs(private_access_tag_t<TYPE>);                                                           \
    }

namespace rainy::type_traits::extras::tuple::implements {
    template <std::size_t N, typename Ty, typename = void>
    struct bind : type_traits::helper::integral_constant<std::size_t, 0> {};

#define RAINY_DECLARE_BIND(N)                                                                                                         \
    template <typename Ty>                                                                                                            \
    struct bind<N, Ty, rainy::type_traits::other_trans::void_t<decltype(Ty{RAINY_INITIALIZER_LIST(N)})>>                              \
        : std::conditional_t<std::is_aggregate_v<type_traits::cv_modify::remove_cvref_t<Ty>>,                                         \
                             rainy::type_traits::helper::integral_constant<std::size_t, N>,                                           \
                             rainy::type_traits::helper::integral_constant<std::size_t, 0>> {}

    RAINY_DECLARE_BIND(1);
    RAINY_DECLARE_BIND(2);
    RAINY_DECLARE_BIND(3);
    RAINY_DECLARE_BIND(4);
    RAINY_DECLARE_BIND(5);
    RAINY_DECLARE_BIND(6);
    RAINY_DECLARE_BIND(7);
    RAINY_DECLARE_BIND(8);
    RAINY_DECLARE_BIND(9);
    RAINY_DECLARE_BIND(10);
    RAINY_DECLARE_BIND(11);
    RAINY_DECLARE_BIND(12);
    RAINY_DECLARE_BIND(13);
    RAINY_DECLARE_BIND(14);
    RAINY_DECLARE_BIND(15);
    RAINY_DECLARE_BIND(16);
    RAINY_DECLARE_BIND(17);
    RAINY_DECLARE_BIND(18);
    RAINY_DECLARE_BIND(19);
    RAINY_DECLARE_BIND(20);
    RAINY_DECLARE_BIND(21);
    RAINY_DECLARE_BIND(22);
    RAINY_DECLARE_BIND(23);
    RAINY_DECLARE_BIND(24);
    RAINY_DECLARE_BIND(25);
    RAINY_DECLARE_BIND(26);
    RAINY_DECLARE_BIND(27);
    RAINY_DECLARE_BIND(28);
    RAINY_DECLARE_BIND(29);
    RAINY_DECLARE_BIND(30);
    RAINY_DECLARE_BIND(31);
    RAINY_DECLARE_BIND(32);
    RAINY_DECLARE_BIND(33);
    RAINY_DECLARE_BIND(34);
    RAINY_DECLARE_BIND(35);
    RAINY_DECLARE_BIND(36);
    RAINY_DECLARE_BIND(37);
    RAINY_DECLARE_BIND(38);
    RAINY_DECLARE_BIND(39);
    RAINY_DECLARE_BIND(40);
    RAINY_DECLARE_BIND(41);
    RAINY_DECLARE_BIND(42);
    RAINY_DECLARE_BIND(43);
    RAINY_DECLARE_BIND(44);
    RAINY_DECLARE_BIND(45);
    RAINY_DECLARE_BIND(46);
    RAINY_DECLARE_BIND(47);
    RAINY_DECLARE_BIND(48);
    RAINY_DECLARE_BIND(49);
    RAINY_DECLARE_BIND(50);
    RAINY_DECLARE_BIND(51);
    RAINY_DECLARE_BIND(52);
    RAINY_DECLARE_BIND(53);
    RAINY_DECLARE_BIND(54);
    RAINY_DECLARE_BIND(55);
    RAINY_DECLARE_BIND(56);
    RAINY_DECLARE_BIND(57);
    RAINY_DECLARE_BIND(58);
    RAINY_DECLARE_BIND(59);
    RAINY_DECLARE_BIND(60);
    RAINY_DECLARE_BIND(61);
    RAINY_DECLARE_BIND(62);
    RAINY_DECLARE_BIND(63);
    RAINY_DECLARE_BIND(64);
    RAINY_DECLARE_BIND(65);
    RAINY_DECLARE_BIND(66);
    RAINY_DECLARE_BIND(67);
    RAINY_DECLARE_BIND(68);
    RAINY_DECLARE_BIND(69);
    RAINY_DECLARE_BIND(70);
    RAINY_DECLARE_BIND(71);
    RAINY_DECLARE_BIND(72);
    RAINY_DECLARE_BIND(73);
    RAINY_DECLARE_BIND(74);
    RAINY_DECLARE_BIND(75);
    RAINY_DECLARE_BIND(76);
    RAINY_DECLARE_BIND(77);
    RAINY_DECLARE_BIND(78);
    RAINY_DECLARE_BIND(79);
    RAINY_DECLARE_BIND(80);
#undef RAINY_DECLARE_BIND

    template <std::size_t N, typename Ty>
    inline static constexpr auto bind_v = bind<N, Ty>::value;

    template <typename Ty>
    static inline constexpr std::size_t eval_member_count =
        (std::max)({implements::bind_v<0, Ty>,  implements::bind_v<1, Ty>,  implements::bind_v<2, Ty>,  implements::bind_v<3, Ty>,
                    implements::bind_v<4, Ty>,  implements::bind_v<5, Ty>,  implements::bind_v<6, Ty>,  implements::bind_v<7, Ty>,
                    implements::bind_v<8, Ty>,  implements::bind_v<9, Ty>,  implements::bind_v<10, Ty>, implements::bind_v<11, Ty>,
                    implements::bind_v<12, Ty>, implements::bind_v<13, Ty>, implements::bind_v<14, Ty>, implements::bind_v<15, Ty>,
                    implements::bind_v<16, Ty>, implements::bind_v<17, Ty>, implements::bind_v<18, Ty>, implements::bind_v<19, Ty>,
                    implements::bind_v<20, Ty>, implements::bind_v<21, Ty>, implements::bind_v<22, Ty>, implements::bind_v<23, Ty>,
                    implements::bind_v<24, Ty>, implements::bind_v<25, Ty>, implements::bind_v<26, Ty>, implements::bind_v<27, Ty>,
                    implements::bind_v<28, Ty>, implements::bind_v<29, Ty>, implements::bind_v<30, Ty>, implements::bind_v<31, Ty>,
                    implements::bind_v<32, Ty>, implements::bind_v<33, Ty>, implements::bind_v<34, Ty>, implements::bind_v<35, Ty>,
                    implements::bind_v<36, Ty>, implements::bind_v<37, Ty>, implements::bind_v<38, Ty>, implements::bind_v<39, Ty>,
                    implements::bind_v<40, Ty>, implements::bind_v<41, Ty>, implements::bind_v<42, Ty>, implements::bind_v<43, Ty>,
                    implements::bind_v<44, Ty>, implements::bind_v<45, Ty>, implements::bind_v<46, Ty>, implements::bind_v<47, Ty>,
                    implements::bind_v<48, Ty>, implements::bind_v<49, Ty>, implements::bind_v<50, Ty>, implements::bind_v<51, Ty>,
                    implements::bind_v<52, Ty>, implements::bind_v<53, Ty>, implements::bind_v<54, Ty>, implements::bind_v<55, Ty>,
                    implements::bind_v<56, Ty>, implements::bind_v<57, Ty>, implements::bind_v<58, Ty>, implements::bind_v<59, Ty>,
                    implements::bind_v<60, Ty>, implements::bind_v<61, Ty>, implements::bind_v<62, Ty>, implements::bind_v<63, Ty>,
                    implements::bind_v<64, Ty>, implements::bind_v<65, Ty>, implements::bind_v<66, Ty>, implements::bind_v<67, Ty>,
                    implements::bind_v<68, Ty>, implements::bind_v<69, Ty>, implements::bind_v<70, Ty>, implements::bind_v<71, Ty>,
                    implements::bind_v<72, Ty>, implements::bind_v<73, Ty>, implements::bind_v<74, Ty>, implements::bind_v<75, Ty>,
                    implements::bind_v<76, Ty>, implements::bind_v<77, Ty>, implements::bind_v<78, Ty>, implements::bind_v<79, Ty>});
}

namespace rainy::type_traits::extras::tuple {
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
    };

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL is_reflectet_for_type_valid = true;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL
        is_reflectet_for_type_valid<Type, type_traits::other_trans::void_t<decltype(reflectet_for_type<Type>::invalid_mark)>> = false;
}

namespace rainy::type_traits::extras::tuple::implements {
    template <std::size_t N, typename Ty>
    struct refl_to_tuple_impl;

    template <typename Ty>
    struct refl_to_tuple_impl<0, Ty> : std::integral_constant<std::size_t, 0> {
        static constexpr auto make() noexcept {
            return std::make_tuple();
        };

        using type = decltype(make());
    };

#define RAINY_DECLARE_TO_TUPLE(N)                                                                                                     \
    template <typename Ty>                                                                                                            \
    struct refl_to_tuple_impl<N, Ty> : std::integral_constant<std::size_t, N> {                                                       \
        static constexpr auto make() noexcept {                                                                                       \
            auto &[RAINY_TO_TUPLE_EXPAND_ARGS(N)] =                                                                                   \
                type_traits::helper::get_fake_object<type_traits::cv_modify::remove_cvref_t<Ty>>();                                   \
            auto ref_tup = std::tie(RAINY_TO_TUPLE_EXPAND_ARGS(N));                                                                   \
            auto get_ptrs = [](auto &..._refs) { return std::make_tuple(&_refs...); };                                                \
            return std::apply(get_ptrs, ref_tup);                                                                                     \
        }                                                                                                                             \
        static constexpr auto make_ptr(Ty &obj) noexcept {                                                                            \
            auto &[RAINY_TO_TUPLE_EXPAND_ARGS(N)] = obj;                                                                              \
            auto ref_tup = std::tie(RAINY_TO_TUPLE_EXPAND_ARGS(N));                                                                   \
            auto get_ptrs = [](auto &..._refs) { return std::make_tuple(&_refs...); };                                                \
            return std::apply(get_ptrs, ref_tup);                                                                                     \
        }                                                                                                                             \
        using type = decltype(make());                                                                                                \
    }

    RAINY_DECLARE_TO_TUPLE(1);
    RAINY_DECLARE_TO_TUPLE(2);
    RAINY_DECLARE_TO_TUPLE(3);
    RAINY_DECLARE_TO_TUPLE(4);
    RAINY_DECLARE_TO_TUPLE(5);
    RAINY_DECLARE_TO_TUPLE(6);
    RAINY_DECLARE_TO_TUPLE(7);
    RAINY_DECLARE_TO_TUPLE(8);
    RAINY_DECLARE_TO_TUPLE(9);
    RAINY_DECLARE_TO_TUPLE(10);
    RAINY_DECLARE_TO_TUPLE(11);
    RAINY_DECLARE_TO_TUPLE(12);
    RAINY_DECLARE_TO_TUPLE(13);
    RAINY_DECLARE_TO_TUPLE(14);
    RAINY_DECLARE_TO_TUPLE(15);
    RAINY_DECLARE_TO_TUPLE(16);
    RAINY_DECLARE_TO_TUPLE(17);
    RAINY_DECLARE_TO_TUPLE(18);
    RAINY_DECLARE_TO_TUPLE(19);
    RAINY_DECLARE_TO_TUPLE(20);
    RAINY_DECLARE_TO_TUPLE(21);
    RAINY_DECLARE_TO_TUPLE(22);
    RAINY_DECLARE_TO_TUPLE(23);
    RAINY_DECLARE_TO_TUPLE(24);
    RAINY_DECLARE_TO_TUPLE(25);
    RAINY_DECLARE_TO_TUPLE(26);
    RAINY_DECLARE_TO_TUPLE(27);
    RAINY_DECLARE_TO_TUPLE(28);
    RAINY_DECLARE_TO_TUPLE(29);
    RAINY_DECLARE_TO_TUPLE(30);
    RAINY_DECLARE_TO_TUPLE(31);
    RAINY_DECLARE_TO_TUPLE(32);
    RAINY_DECLARE_TO_TUPLE(33);
    RAINY_DECLARE_TO_TUPLE(34);
    RAINY_DECLARE_TO_TUPLE(35);
    RAINY_DECLARE_TO_TUPLE(36);
    RAINY_DECLARE_TO_TUPLE(37);
    RAINY_DECLARE_TO_TUPLE(38);
    RAINY_DECLARE_TO_TUPLE(39);
    RAINY_DECLARE_TO_TUPLE(40);
    RAINY_DECLARE_TO_TUPLE(41);
    RAINY_DECLARE_TO_TUPLE(42);
    RAINY_DECLARE_TO_TUPLE(43);
    RAINY_DECLARE_TO_TUPLE(44);
    RAINY_DECLARE_TO_TUPLE(45);
    RAINY_DECLARE_TO_TUPLE(46);
    RAINY_DECLARE_TO_TUPLE(47);
    RAINY_DECLARE_TO_TUPLE(48);
    RAINY_DECLARE_TO_TUPLE(49);
    RAINY_DECLARE_TO_TUPLE(50);
    RAINY_DECLARE_TO_TUPLE(51);
    RAINY_DECLARE_TO_TUPLE(52);
    RAINY_DECLARE_TO_TUPLE(53);
    RAINY_DECLARE_TO_TUPLE(54);
    RAINY_DECLARE_TO_TUPLE(55);
    RAINY_DECLARE_TO_TUPLE(56);
    RAINY_DECLARE_TO_TUPLE(57);
    RAINY_DECLARE_TO_TUPLE(58);
    RAINY_DECLARE_TO_TUPLE(59);
    RAINY_DECLARE_TO_TUPLE(60);
    RAINY_DECLARE_TO_TUPLE(61);
    RAINY_DECLARE_TO_TUPLE(62);
    RAINY_DECLARE_TO_TUPLE(63);
    RAINY_DECLARE_TO_TUPLE(64);
    RAINY_DECLARE_TO_TUPLE(65);
    RAINY_DECLARE_TO_TUPLE(66);
    RAINY_DECLARE_TO_TUPLE(67);
    RAINY_DECLARE_TO_TUPLE(68);
    RAINY_DECLARE_TO_TUPLE(69);
    RAINY_DECLARE_TO_TUPLE(70);
    RAINY_DECLARE_TO_TUPLE(71);
    RAINY_DECLARE_TO_TUPLE(72);
    RAINY_DECLARE_TO_TUPLE(73);
    RAINY_DECLARE_TO_TUPLE(74);
    RAINY_DECLARE_TO_TUPLE(75);
    RAINY_DECLARE_TO_TUPLE(76);
    RAINY_DECLARE_TO_TUPLE(77);
    RAINY_DECLARE_TO_TUPLE(78);
    RAINY_DECLARE_TO_TUPLE(79);
    RAINY_DECLARE_TO_TUPLE(80);
#undef RAINY_DECLARE_TO_TUPLE
}

namespace rainy::type_traits::extras::tuple {
    template <typename Ty>
    struct member_count {
        static inline constexpr std::size_t value =
            is_reflectet_for_type_valid<Ty> ? reflectet_for_type<Ty>::count : implements::eval_member_count<Ty>;
    };

    template <typename Ty>
    static constexpr std::size_t member_count_v = member_count<Ty>::value;

    template <typename Ty>
    constexpr auto struct_to_tuple() {
        constexpr std::size_t count = member_count_v<type_traits::cv_modify::remove_cvref_t<Ty>>;
        if constexpr (is_reflectet_for_type_valid<Ty>) {
            return reflectet_for_type<Ty>::make();
        } else if constexpr (count != 0) {
            return implements::refl_to_tuple_impl<count, Ty>::make();
        } else {
            return std::make_tuple();
        }
    }
}

namespace rainy::utility {
    using type_traits::extras::tuple::member_count;
    using type_traits::extras::tuple::member_count_v;
    using type_traits::extras::tuple::struct_to_tuple;
}

namespace rainy::utility::implements {
    template <typename Ty, typename = void>
    struct has_get_private_ptrs : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_get_private_ptrs<Ty, type_traits::other_trans::void_t<decltype(get_private_ptrs(private_access_tag<Ty>))>>
        : std::true_type {};

    template <typename Ty, typename = void>
    struct get_private_ptrs_helper {
        static constexpr auto value = std::make_tuple();
    };

    template <typename Ty>
    struct get_private_ptrs_helper<Ty, std::void_t<decltype(get_private_ptrs(private_access_tag<Ty>))>> {
        static constexpr auto value = get_private_ptrs(private_access_tag<Ty>);
    };
}

namespace rainy::utility {
#if RAINY_HAS_CXX20 && !RAINY_USING_GCC
    template <typename Ty>
    constexpr auto get_member_names() {
        constexpr bool has_get_private_ptrs_v = implements::has_get_private_ptrs<Ty>::value;
        static_assert(member_count_v<Ty> != 0 || has_get_private_ptrs_v, "Failed!");
        if constexpr (!has_get_private_ptrs_v) {
            collections::array<std::string_view, member_count_v<Ty>> array{}; // 创建对应的数组
            constexpr auto tp = struct_to_tuple<Ty>();
            [&array, &tp]<std::size_t... I>(std::index_sequence<I...>) mutable {
                ((array[I] = variable_name<(std::get<I>(tp))>()), ...);
            }(std::make_index_sequence<member_count_v<Ty>>{});
            return array;
        } else {
            constexpr auto tp = implements::get_private_ptrs_helper<Ty>::value; // 使用 helper 获取 `tp`
            constexpr std::size_t tuple_size = std::tuple_size_v<decltype(tp)>;
            collections::array<std::string_view, tuple_size> array{};
            [&array, &tp]<std::size_t... I>(std::index_sequence<I...>) mutable {
                ((array[I] = variable_name<(std::get<I>(tp))>()), ...);
            }(std::make_index_sequence<tuple_size>{});
            return array;
        }
    }
#endif
}

namespace rainy::type_traits::extras::tuple {
    template <typename Ty>
    constexpr auto struct_bind_tuple(Ty &obj) {
        constexpr bool has_private = utility::implements::has_get_private_ptrs<Ty>::value;
        if constexpr (!has_private) {
            constexpr std::size_t count = member_count_v<type_traits::cv_modify::remove_cvref_t<Ty>>;
            if constexpr (is_reflectet_for_type_valid<Ty>) {
                return reflectet_for_type<Ty>::bind_obj(obj);
            } else if constexpr (count != 0) {
                return implements::refl_to_tuple_impl<count, Ty>::make_ptr(obj);
            } else {
                return std::make_tuple();
            }
        } else {
            constexpr auto ptrs = utility::implements::get_private_ptrs_helper<Ty>::value;
            if constexpr (type_traits::type_relations::is_same_v<std::tuple<>, decltype(ptrs)>) {
                return std::make_tuple();
            } else {
                return std::apply([&](auto... mp) { return std::make_tuple(&(obj.*mp)...); }, ptrs);
            }
        }
    }
}

namespace rainy::utility {
    using type_traits::extras::tuple::struct_bind_tuple;
}

namespace rainy::type_traits::extras::tuple {
    template <typename Ty1, typename Ty2>
    struct reflectet_for_type<std::pair<Ty1, Ty2>> {
        static constexpr inline std::size_t count = 2;

        static constexpr auto make() noexcept {
            constexpr auto pair = helper::get_fake_object<std::pair<Ty1, Ty2>>();
            return std::make_tuple(&pair.first, &pair.second);
        }

        static constexpr auto bind_obj(std::pair<Ty1, Ty2> &obj) noexcept {
            return std::make_tuple(&obj.first, &obj.second);
        }
    };

    template <typename Ty1, typename Ty2>
    struct reflectet_for_type<utility::pair<Ty1, Ty2>> {
        static constexpr inline std::size_t count = 2;

        static constexpr auto make() noexcept {
            constexpr auto pair = helper::get_fake_object<utility::pair<Ty1, Ty2>>();
            return std::make_tuple(&pair.first, &pair.second);
        }

        static constexpr auto bind_obj(utility::pair<Ty1, Ty2> &obj) noexcept {
            return std::make_tuple(&obj.first, &obj.second);
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
    };

    template <template <typename Ty, std::size_t N> typename ArrayTemplate,typename Ty, std::size_t N>
    struct reflectet_for_type<ArrayTemplate<Ty, N>,
        type_traits::other_trans::void_t< 
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_aggregate_v<ArrayTemplate<Ty, N>>>, 
            typename ArrayTemplate<Ty, N>::value_type,
            typename ArrayTemplate<Ty, N>::iterator
        >
    > {
        static constexpr inline std::size_t count = N;

        static constexpr auto make() noexcept {
            return implements::refl_to_tuple_impl<count, ArrayTemplate<Ty, N>>::make();
        }

        static constexpr auto bind_obj(ArrayTemplate<Ty, N> &obj) noexcept {
            return implements::refl_to_tuple_impl<count, ArrayTemplate<Ty, N>>::make_ptr(obj);
        }
    };
}

namespace rainy::type_traits::extras::tuple::implements {
    template <typename Tuple>
    struct tuple_traits_impl {
        static inline constexpr bool invalid_mark = true;
    };

    template <typename... Types>
    struct tuple_traits_impl<std::tuple<Types...>> {
        using type = std::tuple<Types...>;

        template <std::size_t Idx>
        using element = std::tuple_element<Idx,type>;

        template <std::size_t Idx>
        using element_t = std::tuple_element<Idx,type>;

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

namespace rainy::type_traits::extras::tuple {
    template <typename Tuple>
    struct tuple_traits : implements::tuple_traits_impl<type_traits::cv_modify::remove_cvref_t<Tuple>> {};

    template <typename Tuple, typename = void>
    RAINY_CONSTEXPR_BOOL is_tuple_v = true;

    template <typename Tuple>
    RAINY_CONSTEXPR_BOOL is_tuple_v<Tuple, other_trans::void_t<decltype(tuple_traits<Tuple>::invalid_mark)>> = false;

    template <typename Tuple>
    struct is_tuple : helper::bool_constant<is_tuple_v<Tuple>> {};

    template <typename Pair>
    struct pair_traits : implements::pair_traits_impl<type_traits::cv_modify::remove_cvref_t<Pair>> {};

    template <typename Pair, typename = void>
    RAINY_CONSTEXPR_BOOL is_pair_v = true;

    template <typename Pair>
    RAINY_CONSTEXPR_BOOL is_pair_v<Pair, other_trans::void_t<decltype(pair_traits<Pair>::invalid_mark)>> = false;

    template <typename Pair>
    struct is_pair : helper::bool_constant<is_pair_v<Pair>> {};
}

namespace rainy::type_traits::primary_types {
    using extras::tuple::tuple_traits;
    using extras::tuple::pair_traits;
    using extras::tuple::is_tuple_v;
    using extras::tuple::is_pair_v;
    using extras::tuple::is_tuple;
    using extras::tuple::is_pair;
}

#endif