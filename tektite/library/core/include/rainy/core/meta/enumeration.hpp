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
#ifndef RAINY_META_MOON_ENUMERATION_HPP
#define RAINY_META_MOON_ENUMERATION_HPP
#include <rainy/core/container/optional.hpp>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/typeinfo.hpp>
#include <rainy/core/functional/functor.hpp>

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
#include <rainy/core/annotations/moon.hpp>
#endif

// 目前C++26的静态反射不再需要ENUM_SCAN_BEGIN/ENUM_SCAN_END宏了，由编译器提供服务
#if !(RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION)
#define ENUM_SCAN_BEGIN -127
#define ENUM_SCAN_END 128
#endif

#if RAINY_USING_CLANG || RAINY_USING_LLVM_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wenum-constexpr-conversion"
#endif

namespace rainy::core::meta::implements {
    enum class enum_subtype {
        common,
        flags
    };

    enum class customize_tag {
        default_tag,
        invalid_tag,
        custom_tag
    };

    template <bool, typename R>
    struct enable_if_enum {};

    template <typename Ty>
    struct enable_if_enum<true, Ty> {
        using type = Ty;
    };

    template <typename Enum, typename Ty, typename Pred = functional::equal<>, typename Decay = type_traits::other_trans::decay_t<Enum>>
    using enable_if_t = typename enable_if_enum<
        type_traits::primary_types::is_enum_v<Decay> && type_traits::properties::is_invocable_r_v<bool, Pred, char, char>, Ty>::type;
}

namespace rainy::core::meta {
    /**
     * \lang english
     * @brief Whether static_cast<E>(V) is valid in a constant expression.
     *
     * Used by the fallback (non-static-reflection) implementation to filter out invalid enumerator values.
     *
     * @tparam E The enum type
     * @tparam V The candidate value
     *
     * \lang simp-chinese
     * @brief 指示 static_cast<E>(V) 在常量表达式中是否有效。
     *
     * 用于后备（非静态反射）实现，以过滤无效的枚举值。
     *
     * @tparam E 枚举类型
     * @tparam V 候选值
     */
#if RAINY_USING_CLANG && __clang_major__ >= 16
    template <typename E, auto V, typename = void>
    inline constexpr bool is_enum_constexpr_static_cast_valid = false;

    template <typename E, auto V>
    inline constexpr bool is_enum_constexpr_static_cast_valid<
        E, V, rainy::type_traits::other_trans::void_t<rainy::type_traits::helper::integral_constant<E, static_cast<E>(V)>>> = true;
#else
    template <typename E, auto V>
    inline constexpr bool is_enum_constexpr_static_cast_valid = true;
#endif

    /**
     * \lang english
     * @brief A pair of a customization tag and an optional name, used to customize enum reflection results.
     *
     * \lang simp-chinese
     * @brief 由定制标签和可选名称组成的类型，用于定制枚举反射结果。
     */
    class customize_t : public container::pair<implements::customize_tag, text::string_view> {
    public:
        /**
         * \lang english
         * @brief Constructs a custom tag with the given name.
         *
         * @param srt The custom name
         *
         * \lang simp-chinese
         * @brief 使用给定的名称构造自定义标签。
         *
         * @param srt 自定义名称
         */
        constexpr customize_t(text::string_view srt) : // NOLINT
            pair{implements::customize_tag::custom_tag, srt} {
        }

        /**
         * \lang english
         * @brief Constructs a custom tag from a C-style string.
         *
         * @param srt The custom name
         *
         * \lang simp-chinese
         * @brief 从C风格字符串构造自定义标签。
         *
         * @param srt 自定义名称
         */
        constexpr customize_t(const char *srt) : customize_t{text::string_view{srt}} { // NOLINT
        }

        /**
         * \lang english
         * @brief Constructs a tag from a predefined customization tag.
         *
         * @param tag The predefined tag, which must not be custom_tag
         *
         * \lang simp-chinese
         * @brief 从预定义的定制标签构造。
         *
         * @param tag 预定义标签，不能是custom_tag
         */
        constexpr customize_t(implements::customize_tag tag) : // NOLINT
            pair{tag, text::string_view{}} {
            assert(tag != implements::customize_tag::custom_tag);
        }
    };

    /**
     * \lang english
     * @brief The default customization tag, meaning the library-provided default behavior is used.
     *
     * \lang simp-chinese
     * @brief 默认定制标签，表示使用库提供的默认行为。
     */
    inline constexpr customize_t default_tag{implements::customize_tag::default_tag};

    /**
     * \lang english
     * @brief The invalid customization tag, meaning no name should be produced.
     *
     * \lang simp-chinese
     * @brief 无效定制标签，表示不产生任何名称。
     */
    inline constexpr customize_t invalid_tag{implements::customize_tag::invalid_tag};

    template <typename Enum>
    constexpr rain_fn customize_for_enum_name(Enum) noexcept -> customize_t {
        return default_tag;
    }

    template <typename>
    constexpr rain_fn customize_for_enum_type_name() noexcept -> customize_t {
        return default_tag;
    }

    template <typename Enum>
    constexpr rain_fn enum_type_name() noexcept -> text::string_view {
        [[maybe_unused]] constexpr auto custom = customize_for_enum_type_name<Enum>();
        static_assert(type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<decltype(custom)>, customize_t>,
                      "oops, so, if you want to customize a name for customize_for_enum_type_name,the return_type of "
                      "customize_for_enum_type_name must be customize_t");
        if constexpr (custom.first == implements::customize_tag::custom_tag) {
            constexpr auto name = custom.second;
            static_assert(!name.empty(), "customize_for_enum_type_name requires not empty string.");
            return name;
        } else if constexpr (custom.first == implements::customize_tag::invalid_tag) {
            return {};
        } else if constexpr (custom.first == implements::customize_tag::default_tag) {
            constexpr text::string_view name = type_name<Enum>();
            return name.substr(name.rfind(':') + 1);
        } else {
            static_assert(rainy::type_traits::implements::always_false<Enum>);
        }
        return {};
    }
}

namespace rainy::core::meta::implements {
    template <typename Op = std::equal_to<>>
    class case_insensitive {
    public:
        template <typename Left, typename Right>
        constexpr auto operator()(Left left, Right right) const noexcept -> type_traits::other_trans::enable_if_t<
            type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Left>, char> &&
                type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Right>, char>,
            bool> {
            return Op{}(to_lower(left), to_lower(right));
        }
    private:
        static constexpr char to_lower(const char c) noexcept {
            return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
        }
    };
}

namespace rainy::core::meta {
    /**
     * \lang english
     * @brief Case-insensitive character comparison predicate.
     *
     * \lang simp-chinese
     * @brief 大小写不敏感的字符比较谓词。
     */
    inline constexpr auto case_insensitive = implements::case_insensitive<>{};
}

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

namespace rainy::core::meta::implements {
    template <typename E, bool NoIgnore = false>
    constexpr rain_fn enum_count_impl() noexcept -> std::size_t {
        std::vector<std::meta::info> vec = std::meta::enumerators_of(^^E);
        std::size_t count = 0;
        for (const auto item: vec) {
            if (NoIgnore) {
                count += 1;
            } else if (const auto mem_anno = annotations::make_member_anno(item); !mem_anno.has<annotations::moon::ignore_tag>()) {
                count += 1;
            }
        }
        return count;
    }

    template <typename E>
    constexpr auto enum_arrays = [] consteval {
        auto members = std::meta::enumerators_of(^^E);
        collections::array<std::meta::info, enum_count_impl<E>()> arr;
        std::size_t idx = 0;
        for (auto member: members) {
            if (const auto mem_anno = annotations::make_member_anno(member); !mem_anno.has<annotations::moon::ignore_tag>()) {
                arr[idx++] = member;
            }
        }
        return std::define_static_array(arr);
    }();

    template <typename E>
    constexpr auto all_enum_array = [] consteval {
        auto members = std::meta::enumerators_of(^^E);
        collections::array<std::meta::info, enum_count_impl<E, true>()> arr;
        std::size_t idx = 0;
        for (auto member: members) {
            arr[idx++] = member;
        }
        return std::define_static_array(arr);
    }();

    template <typename E>
        requires type_traits::primary_types::is_enum_v<E>
    constexpr rain_fn is_enum_value(E value) -> bool {
        if constexpr (enum_count_impl<E>() == 0) {
            return false;
        } else {
            template for (constexpr auto member: enum_arrays<E>) {
                if (value == [:member:]) {
                    return true;
                }
            }
            return false;
        }
    }
}

namespace rainy::core::meta::implements {
    template <typename Ty, std::size_t Size>
    RAINY_CONSTEVAL rain_fn enum_get_member_names_compositor(collections::array<text::string_view, Size> &array) noexcept -> void {
        using namespace std::meta;

        [&]<std::size_t... Idx>(::rainy::type_traits::helper::index_sequence<Idx...>) consteval {
            (
                [&]() consteval {
                    template for (constexpr auto m: all_enum_array<Ty>) {
                        if ([:m:] == [:enum_arrays<Ty>[Idx]:]) {
                            implements::try_apply_rename<m>(array[Idx]);
                            implements::try_apply_name_style<Ty, m>(array[Idx]);
                            implements::try_apply_prefix_and_suffix_tag<Ty, m>(array[Idx]);
                            break;
                        }
                    }
                }(),
                ...);
        }(::rainy::type_traits::helper::make_index_sequence<Size>{});
    }
}

#else

namespace rainy::core::meta::implements {
    template <typename E, E V>
    constexpr rain_fn has_valid_enum_name() noexcept -> bool {
        text::string_view str = variable_name<V>();
        if (str.empty()) {
            return false;
        }
        return str[0] != '('; // NOLINT
    }

    template <typename E, auto V>
    static constexpr rain_fn is_enum_value_helper() noexcept -> bool {
        if constexpr (!type_traits::primary_types::is_enum_v<E>) {
            return false;
        }

        if constexpr (!is_enum_constexpr_static_cast_valid<E, V>) {
            return false;
        }

        constexpr E enum_val = static_cast<E>(V);
        return has_valid_enum_name<E, enum_val>();
    }

    template <typename E, auto V>
    struct is_enum_value_impl {
        static constexpr bool value = is_enum_value_helper<E, V>();
    };

    template <std::size_t Size, typename E, auto Begin, std::size_t... Is>
    constexpr rain_fn enum_values_impl(type_traits::helper::index_sequence<Is...>) -> auto {
        constexpr collections::array<E, Size> arr = []() constexpr {
            collections::array<E, Size> tmp{};
            std::size_t idx = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is)>::value) {
                     tmp[idx++] = static_cast<E>(Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is));
                 }
             }()),
             ...);
            return tmp;
        }();
        return arr;
    }

    template <typename Enum, Enum V>
    constexpr rain_fn enum_name_impl() noexcept -> text::string_view {
        [[maybe_unused]] constexpr auto custom = customize_for_enum_name<Enum>(V);
        static_assert(type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<decltype(custom)>, customize_t>,
                      "oops, so, if you want to customize a name for customize_for_enum_name,the return_type of "
                      "customize_for_enum_type_name must be customize_t");
        if constexpr (custom.first == customize_tag::custom_tag) {
            constexpr auto name = custom.second;
            static_assert(!name.empty(), "magic_enum::customize requires not empty string.");
            return name;
        } else if constexpr (custom.first == customize_tag::invalid_tag) {
            return {};
        } else if constexpr (custom.first == customize_tag::default_tag) {
            constexpr auto name = variable_name<V>();
            return name.substr(name.rfind(':') + 1);
        } else {
            static_assert(type_traits::implements::always_false<Enum>);
        }
        return {};
    }

    template <std::size_t Size, typename E, auto Begin, std::size_t... Is>
    constexpr rain_fn enum_entries_impl(type_traits::helper::index_sequence<Is...>) -> auto {
        constexpr collections::array<container::pair<E, text::string_view>, Size> arr = []() constexpr {
            collections::array<container::pair<E, text::string_view>, Size> tmp{};
            std::size_t idx = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is)>::value) {
                     tmp[idx].first = static_cast<E>(Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is));
                     tmp[idx].second =
                         enum_name_impl<E, static_cast<E>(Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is))>();
                     ++idx;
                 }
             }()),
             ...);
            return tmp;
        }();
        return arr;
    }

    template <std::size_t Size, typename E, auto Begin, std::size_t... Is>
    constexpr rain_fn enum_names_impl(type_traits::helper::index_sequence<Is...>) -> auto {
        constexpr std::array<text::string_view, Size> arr = []() constexpr {
            std::array<text::string_view, Size> tmp{};
            std::size_t idx = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is)>::value) {
                     tmp[idx++] = {
                         enum_name_impl<E, static_cast<E>(Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is))>()};
                 }
             }()),
             ...);
            return tmp;
        }();
        return arr;
    }

    template <typename E, type_traits::other_trans::underlying_type_t<E> Begin, std::size_t... Is>
    constexpr rain_fn enum_count_impl(type_traits::helper::index_sequence<Is...>) -> std::size_t {
        constexpr std::size_t count = []() constexpr {
            std::size_t total = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<type_traits::other_trans::underlying_type_t<E>>(Is)>::value) {
                     ++total;
                 }
             }()),
             ...);
            return total;
        }();
        return count;
    }
}

#endif

namespace rainy::core::meta {
    /**
     * \lang english
     * @brief Whether V is a valid enumerator of the enum type E.
     *
     * @tparam E The enum type
     * @tparam V The candidate value
     *
     * \lang simp-chinese
     * @brief 指示 V 是否为枚举类型 E 的有效枚举值。
     *
     * @tparam E 枚举类型
     * @tparam V 候选值
     */
    template <typename E, auto V>
    RAINY_CONSTEXPR_BOOL is_enum_value_v =
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        implements::is_enum_value(static_cast<E>(V))
#else
        implements::is_enum_value_impl<E, V>::value
#endif
        ;

    template <typename E, auto V>
    struct is_enum_value : type_traits::helper::bool_constant<is_enum_value_v<E, V>> {};

    template <typename E>
    constexpr rain_fn enum_count() noexcept -> std::size_t {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION // 自C++26开始，如果可能，将启用static reflection的语法，对enum类型进行反射
        return implements::enum_count_impl<E>();
#else
        static_assert(type_traits::primary_types::is_enum_v<E>, "must be enum");
        constexpr rainy_let begin = ENUM_SCAN_BEGIN;
        constexpr rainy_let end = ENUM_SCAN_END;
        constexpr rainy_let n = static_cast<std::size_t>(end - begin + 1); // NOLINT
        return implements::enum_count_impl<E, type_traits::other_trans::underlying_type_t<E>(begin)>(
            type_traits::helper::make_index_sequence<n>{});
#endif
    }

    template <typename E>
    constexpr rain_fn enum_values() noexcept -> auto {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        if constexpr (enum_count<E>() == 0) {
            collections::array<E, 0> enums_arr = {};
            return enums_arr;
        } else {
            collections::array<E, enum_count<E>()> enums_arr = {};
            template for (std::size_t idx = 0; constexpr auto member: implements::enum_arrays<E>) {
                if constexpr (constexpr auto member_anno = annotations::make_member_anno(member);
                              !member_anno.template has<annotations::moon::ignore_tag>()) {
                    enums_arr[idx++] = {static_cast<E>([:member:])};
                }
            }
            return enums_arr;
        }
#else
        if constexpr (enum_count<E>() == 0) {
            collections::array<E, 0> enums_arr = {};
            return enums_arr;
        } else {
            constexpr rainy_let begin = ENUM_SCAN_BEGIN;
            constexpr rainy_let end = ENUM_SCAN_END;
            constexpr rainy_let n = static_cast<std::size_t>(end - begin + 1); // NOLINT
            return implements::enum_values_impl<enum_count<E>(), E, type_traits::other_trans::underlying_type_t<E>(begin)>(
                type_traits::helper::make_index_sequence<n>{});
        }
#endif
    }

    /**
     * \lang english
     * @brief Returns the Idx-th enumerator value of the enum type.
     *
     * @tparam E The enum type
     * @tparam Idx The index of the enumerator
     * @return The enumerator value
     *
     * \lang simp-chinese
     * @brief 返回枚举类型的第 Idx 个枚举值。
     *
     * @tparam E 枚举类型
     * @tparam Idx 枚举值的索引
     * @return 枚举值
     */
    template <typename E, std::size_t Idx>
    constexpr rain_fn enum_value() noexcept -> E {
        constexpr auto values = enum_values<E>();
        return values[Idx];
    }

    /**
     * \lang english
     * @brief Returns the idx-th enumerator value of the enum type.
     *
     * @tparam E The enum type
     * @param idx The index of the enumerator
     * @return The enumerator value
     *
     * \lang simp-chinese
     * @brief 返回枚举类型的第 idx 个枚举值。
     *
     * @tparam E 枚举类型
     * @param idx 枚举值的索引
     * @return 枚举值
     */
    template <typename E>
    constexpr rain_fn enum_value(std::size_t idx) noexcept
        -> type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<E>, E> {
        constexpr auto values = enum_values<E>();
        return values[idx];
    }

    template <typename E>
    constexpr rain_fn enum_entries() -> auto {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        collections::array<text::string_view, enum_count<E>()> name_arr;
        template for (std::size_t idx = 0; constexpr auto member: implements::enum_arrays<E>) {
            name_arr[idx++] = {std::meta::identifier_of(member)};
        }
        implements::enum_get_member_names_compositor<E, enum_count<E>()>(name_arr);

        collections::array<container::pair<E, text::string_view>, enum_count<E>()> arr;
        template for (std::size_t idx = 0; constexpr auto member: implements::enum_arrays<E>) {
            if constexpr (constexpr auto member_anno = annotations::make_member_anno(member);
                          !member_anno.template has<annotations::moon::ignore_tag>()) {
                arr[idx] = {static_cast<E>([:member:]), name_arr[idx]};
                ++idx;
            }
        }
        return arr;
#else
        constexpr rainy_let begin = ENUM_SCAN_BEGIN;
        constexpr rainy_let end = ENUM_SCAN_END;
        constexpr rainy_let n = static_cast<std::size_t>(end - begin + 1); // NOLINT
        return implements::enum_entries_impl<enum_count<E>(), E, begin>(type_traits::helper::make_index_sequence<n>{});
#endif
    }

    /**
     * \lang english
     * @brief Returns the name of the given enumerator value.
     *
     * @tparam Enum The enum type
     * @param EnumValue The enumerator value
     * @return The name of the enumerator, or an empty string_view if not found
     *
     * \lang simp-chinese
     * @brief 返回给定枚举值的名称。
     *
     * @tparam Enum 枚举类型
     * @param EnumValue 枚举值
     * @return 枚举值的名称，未找到时返回空string_view
     */
    template <typename Enum>
    constexpr rain_fn enum_name(Enum EnumValue)
        -> type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<Enum>, text::string_view> {
        for (constexpr auto entries = enum_entries<Enum>(); const auto &[enum_value, enum_name]: entries) {
            if (enum_value == EnumValue) {
                return enum_name;
            }
        }
        return {};
    }

    /**
     * \lang english
     * @brief Returns the name of the enumerator given as a template argument.
     *
     * @tparam Enum The enum type
     * @tparam EnumValue The enumerator value
     * @return The name of the enumerator, or an empty string_view if not found
     *
     * \lang simp-chinese
     * @brief 返回以模板参数给定的枚举值的名称。
     *
     * @tparam Enum 枚举类型
     * @tparam EnumValue 枚举值
     * @return 枚举值的名称，未找到时返回空string_view
     */
    template <typename Enum, Enum EnumValue>
    constexpr rain_fn enum_name()
        -> type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<Enum>, text::string_view> {
        return enum_name<Enum>(EnumValue);
    }

    /**
     * \lang english
     * @brief Returns an array of all enumerator names.
     *
     * @tparam Enum The enum type
     * @return An array of enumerator names
     *
     * \lang simp-chinese
     * @brief 返回所有枚举值名称组成的数组。
     *
     * @tparam Enum 枚举类型
     * @return 枚举值名称数组
     */
    template <typename Enum>
    constexpr rain_fn enum_names() noexcept -> auto {
#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
        collections::array<text::string_view, enum_count<Enum>()> arr;
        if constexpr (enum_count<Enum>() == 0) {
            return arr;
        } else {
            template for (std::size_t idx = 0; constexpr auto member: implements::enum_arrays<Enum>) {
                auto s = std::meta::identifier_of(member);
                arr[idx++] = s;
            }
            implements::enum_get_member_names_compositor<Enum, enum_count<Enum>()>(arr);
            return arr;
        }
#else
        constexpr int begin = ENUM_SCAN_BEGIN;
        constexpr int end = ENUM_SCAN_END;
        constexpr std::size_t n = end - begin + 1;
        return implements::enum_names_impl<enum_count<Enum>(), Enum, begin>(type_traits::helper::make_index_sequence<n>{});
#endif
    }

    /**
     * \lang english
     * @brief Converts a name to an optional enum value.
     *
     * @tparam Enum The enum type
     * @tparam Pred The comparison predicate type
     * @param name The name to convert
     * @param pred The comparison predicate
     * @return The matching enumerator value, or std::nullopt if no match
     *
     * \lang simp-chinese
     * @brief 将名称转换为可选的枚举值。
     *
     * @tparam Enum 枚举类型
     * @tparam Pred 比较谓词类型
     * @param name 要转换的名称
     * @param pred 比较谓词
     * @return 匹配的枚举值，未匹配时返回std::nullopt
     */
    template <typename Enum, typename Pred = std::equal_to<>>
    constexpr rain_fn enum_cast(const text::string_view name, Pred pred = {}) noexcept
        -> implements::enable_if_t<Enum, container::optional<Enum>, Pred> {
        for (constexpr auto entries = enum_entries<Enum>(); const auto &[enum_value, enum_name]: entries) {
            if (algorithm::equal(name.begin(), name.end(), enum_name.begin(), enum_name.end(), pred)) {
                return enum_value;
            }
        }
        return std::nullopt;
    }

    template <typename Enum>
    constexpr rain_fn enum_cast(type_traits::other_trans::underlying_type_t<Enum> value) noexcept
        -> implements::enable_if_t<Enum, container::optional<Enum>> {
        constexpr auto entries = enum_entries<Enum>();
        using underlying = type_traits::other_trans::underlying_type_t<Enum>;
        for (const auto &[enum_value, enum_name]: entries) {
            if (static_cast<underlying>(enum_value) == value) {
                return enum_value;
            }
            (void) enum_name;
        }
        return std::nullopt;
    }

    template <typename Enum>
    RAINY_NODISCARD constexpr rain_fn enum_integer(Enum value) noexcept
        -> implements::enable_if_t<Enum, type_traits::other_trans::underlying_type_t<Enum>> {
        return static_cast<type_traits::other_trans::underlying_type_t<Enum>>(value);
    }

    /**
     * \lang english
     * @brief Returns the underlying integer value of the enumerator (alias of enum_integer).
     *
     * @tparam Enum The enum type
     * @param value The enumerator value
     * @return The underlying integer value
     *
     * \lang simp-chinese
     * @brief 返回枚举值的底层整数值（enum_integer的别名）。
     *
     * @tparam Enum 枚举类型
     * @param value 枚举值
     * @return 底层整数值
     */
    template <typename Enum>
    RAINY_NODISCARD constexpr rain_fn enum_underlying(Enum value) noexcept
        -> implements::enable_if_t<Enum, type_traits::other_trans::underlying_type_t<Enum>> {
        return static_cast<type_traits::other_trans::underlying_type_t<Enum>>(value);
    }

    /**
     * \lang english
     * @brief Returns the index of the enumerator within enum_values.
     *
     * @tparam Enum The enum type
     * @param value The enumerator value
     * @return The index, or std::nullopt if the value is not an enumerator
     *
     * \lang simp-chinese
     * @brief 返回枚举值在enum_values中的索引。
     *
     * @tparam Enum 枚举类型
     * @param value 枚举值
     * @return 索引，若该值不是枚举值则返回std::nullopt
     */
    template <typename Enum>
    RAINY_NODISCARD constexpr rain_fn enum_index(Enum value) noexcept
        -> implements::enable_if_t<Enum, container::optional<std::size_t>> {
        constexpr auto values = enum_values<Enum>();
        std::size_t idx{};
        for (const auto &item: values) {
            if (item == value) {
                return idx;
            }
            ++idx;
        }
        return std::nullopt;
    }

    template <typename Enum, Enum EnumValue>
    RAINY_NODISCARD constexpr rain_fn enum_index() noexcept -> implements::enable_if_t<Enum, container::optional<std::size_t>> {
        return enum_index<Enum>(EnumValue);
    }

    template <typename Enum, implements::enable_if_t<Enum, int> = 0>
    RAINY_NODISCARD constexpr rain_fn enum_contains(Enum value) noexcept -> auto {
        return static_cast<bool>(enum_cast<Enum>(static_cast<type_traits::other_trans::underlying_type_t<Enum>>(value)).has_value());
    }

    /**
     * \lang english
     * @brief Checks whether the template-argument value is a valid enumerator.
     *
     * @tparam Enum The enum type
     * @tparam Value The value to check
     * @return true if the value is an enumerator, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查以模板参数给定的值是否为有效枚举值。
     *
     * @tparam Enum 枚举类型
     * @tparam Value 要检查的值
     * @return 如果该值是枚举值则为true，否则为false
     */
    template <typename Enum, Enum Value, implements::enable_if_t<Enum, int> = 0>
    RAINY_NODISCARD constexpr rain_fn enum_contains() noexcept -> bool {
        return enum_contains<Enum>(Value);
    }

    /**
     * \lang english
     * @brief Checks whether the given underlying value corresponds to an enumerator.
     *
     * @tparam Enum The enum type
     * @param value The underlying value to check
     * @return true if the value is an enumerator, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查给定的底层整数值是否对应某个枚举值。
     *
     * @tparam Enum 枚举类型
     * @param value 要检查的底层整数值
     * @return 如果该值对应枚举值则为true，否则为false
     */
    template <typename Enum>
    RAINY_NODISCARD constexpr rain_fn enum_contains(type_traits::other_trans::underlying_type_t<Enum> value) noexcept -> bool {
        return static_cast<bool>(enum_cast<Enum>(value).has_value());
    }

    /**
     * \lang english
     * @brief Checks whether the given name matches an enumerator.
     *
     * @tparam Enum The enum type
     * @tparam Pred The comparison predicate type
     * @param name The name to check
     * @param pred The comparison predicate
     * @return true if the name matches an enumerator, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查给定的名称是否匹配某个枚举值。
     *
     * @tparam Enum 枚举类型
     * @tparam Pred 比较谓词类型
     * @param name 要检查的名称
     * @param pred 比较谓词
     * @return 如果名称匹配某个枚举值则为true，否则为false
     */
    template <typename Enum, typename Pred = implements::case_insensitive<>>
    RAINY_NODISCARD constexpr rain_fn enum_contains(text::string_view name, Pred pred = {}) noexcept -> bool {
        return static_cast<bool>(enum_cast<Enum>(name, pred).has_value());
    }
}

namespace rainy::core::meta {
    /**
     * \lang english
     * @brief Builds a combined name string for a bitwise-OR combination of flag enumerators.
     *
     * @tparam E The enum type
     * @param value The combined flag value
     * @param sep The separator between flag names
     * @return The combined name string, or an empty string if the value is not a valid combination
     *
     * \lang simp-chinese
     * @brief 为标志枚举值的按位或组合构建拼接后的名称字符串。
     *
     * @tparam E 枚举类型
     * @param value 组合后的标志值
     * @param sep 标志名称之间的分隔符
     * @return 拼接后的名称字符串，若该值不是有效的组合则返回空字符串
     */
    template <typename E>
    RAINY_NODISCARD rain_fn enum_flags_name(E value, const char sep = '|') -> implements::enable_if_t<E, text::string> {
        using D = type_traits::other_trans::decay_t<E>;
        using U = type_traits::other_trans::underlying_type_t<D>;
        text::string name;
        auto check_value = U{0};
        constexpr auto names = enum_names<E>();
        for (std::size_t i = 0; i < enum_count<E>(); ++i) {
            const auto v = static_cast<U>(enum_value<D>(i));
            // 跳过 0 和非 2 的幂次（复合 flag）
            if (v == 0 || (v & (v - 1)) != 0) {
                continue;
            }
            if ((static_cast<U>(value) & v) != 0) {
                if (const auto n = names[i]; !n.empty()) {
                    check_value |= v;
                    if (!name.empty()) {
                        name.append(1, sep);
                    }
                    name.append(n.data(), n.size());
                } else {
                    return {};
                }
            }
        }
        if (check_value != 0 && check_value == static_cast<U>(value)) {
            return name;
        }
        return {};
    }
}

#if RAINY_USING_CLANG || RAINY_USING_LLVM_GCC
#pragma GCC diagnostic pop
#endif

#endif
