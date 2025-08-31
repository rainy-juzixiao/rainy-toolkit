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

#ifndef RAINY_META_ENUMERATION_HPP
#define RAINY_META_ENUMERATION_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/pair.hpp>

#define ENUM_SCAN_BEGIN -127
#define ENUM_SCAN_END 128

namespace rainy::meta::enumeration::implements {
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

    template <typename Enum, typename Ty, typename Pred = foundation::functional::equal<>,
              typename Decay = type_traits::other_trans::decay_t<Enum>>
    using enable_if_t = typename enable_if_enum<type_traits::primary_types::is_enum_v<Decay> &&
                                                    type_traits::type_properties::is_invocable_r_v<bool, Pred, char, char>,
                                                Ty>::type;
}

namespace rainy::meta::enumeration {
#if RAINY_USING_CLANG && __clang_major__ >= 16
    template <typename E, auto V, typename = void>
    inline constexpr bool is_enum_constexpr_static_cast_valid = false;

    template <typename E, auto V>
    inline constexpr bool is_enum_constexpr_static_cast_valid<E, V, std::void_t<std::integral_constant<E, static_cast<E>(V)>>> = true;
#else
    template <typename E, auto V>
    inline constexpr bool is_enum_constexpr_static_cast_valid = true;
#endif

    class customize_t : public utility::pair<implements::customize_tag, std::string_view> {
    public:
        constexpr customize_t(std::string_view srt) :
            utility::pair<implements::customize_tag, std::string_view>{implements::customize_tag::custom_tag, srt} {
        }
        constexpr customize_t(const char *srt) : customize_t{std::string_view{srt}} {
        }
        constexpr customize_t(implements::customize_tag tag) :
            utility::pair<implements::customize_tag, std::string_view>{tag, std::string_view{}} {
            assert(tag != implements::customize_tag::custom_tag);
        }
    };

    inline constexpr customize_t default_tag{implements::customize_tag::default_tag};
    inline constexpr customize_t invalid_tag{implements::customize_tag::invalid_tag};

    template <typename Enum>
    constexpr customize_t customize_for_enum_name(Enum) noexcept {
        return default_tag;
    }

    template <typename Enum>
    constexpr customize_t customize_for_enum_type_name() noexcept {
        return default_tag;
    }

    template <typename Enum>
    constexpr std::string_view enum_type_name() noexcept {
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
            constexpr std::string_view name = foundation::ctti::type_name<Enum>();
            return name.substr(name.rfind(':') + 1);
        } else {
            static_assert(type_traits::implements::always_false<Enum>);
        }
    }
}

namespace rainy::meta::enumeration::implements {
    template <typename E, E V>
    constexpr bool has_valid_enum_name() noexcept {
        constexpr std::string_view str = foundation::ctti::variable_name<V>();
        if (str.empty()) {
            return false;
        }
        return str[0] != '(';
    }

    template <typename E, auto V>
    static constexpr bool is_enum_value_helper() noexcept {
        if constexpr (!std::is_enum_v<E>) {
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

    template <typename E, std::underlying_type_t<E> Begin, std::size_t... Is>
    constexpr bool enum_range_checker_impl(std::index_sequence<Is...>) {
        return ((is_enum_value_impl<E, Begin + static_cast<std::underlying_type_t<E>>(Is)>::value) || ...);
    }

    template <std::size_t Size, typename E, auto Begin, std::size_t... Is>
    constexpr auto enum_values_impl(std::index_sequence<Is...>) {
        constexpr collections::array<E, Size> arr = [] constexpr {
            collections::array<E, Size> tmp{};
            std::size_t idx = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<std::underlying_type_t<E>>(Is)>::value) {
                     tmp[idx++] = static_cast<E>(Begin + static_cast<std::underlying_type_t<E>>(Is));
                 }
             }()),
             ...);
            return tmp;
        }();
        return arr;
    }

    template <typename Enum, Enum V>
    constexpr auto enum_name_impl() noexcept {
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
            constexpr auto name = foundation::ctti::variable_name<V>();
            return name.substr(name.rfind(':') + 1);
        } else {
            static_assert(type_traits::implements::always_false<Enum>);
        }
    }

    template <std::size_t Size, typename E, auto Begin, std::size_t... Is>
    constexpr auto enum_entries_impl(std::index_sequence<Is...>) {
        constexpr collections::array<utility::pair<E, std::string_view>, Size> arr = [] constexpr {
            collections::array<utility::pair<E, std::string_view>, Size> tmp{};
            std::size_t idx = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<std::underlying_type_t<E>>(Is)>::value) {
                     tmp[idx++] = {static_cast<E>(Begin + static_cast<std::underlying_type_t<E>>(Is)),
                                   enum_name_impl<E, static_cast<E>(Begin + static_cast<std::underlying_type_t<E>>(Is))>()};
                 }
             }()),
             ...);
            return tmp;
        }();
        return arr;
    }

    template <std::size_t Size, typename E, auto Begin, std::size_t... Is>
    constexpr auto enum_names_impl(std::index_sequence<Is...>) {
        constexpr collections::array<std::string_view, Size> arr = [] constexpr {
            collections::array<std::string_view, Size> tmp{};
            std::size_t idx = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<std::underlying_type_t<E>>(Is)>::value) {
                     tmp[idx++] = {enum_name_impl<E, static_cast<E>(Begin + static_cast<std::underlying_type_t<E>>(Is))>()};
                 }
             }()),
             ...);
            return tmp;
        }();
        return arr;
    }

    template <typename E, std::underlying_type_t<E> Begin, std::size_t... Is>
    constexpr std::size_t enum_count_impl(std::index_sequence<Is...>) {
        constexpr std::size_t count = [] constexpr {
            std::size_t total = 0;
            (([&] {
                 if constexpr (is_enum_value_impl<E, Begin + static_cast<std::underlying_type_t<E>>(Is)>::value) {
                     ++total;
                 }
             }()),
             ...);
            return total;
        }();
        return count;
    }
}

namespace rainy::meta::enumeration {
    template <typename E, auto V>
    RAINY_CONSTEXPR_BOOL is_enum_value_v = implements::is_enum_value_impl<E, V>::value;

    template <typename E, auto V>
    struct is_enum_value : type_traits::helper::bool_constant<is_enum_value_v<E, V>> {};

    template <typename E>
    constexpr std::size_t enum_count() noexcept {
        static_assert(std::is_enum_v<E>, "must be enum");
        constexpr int begin = ENUM_SCAN_BEGIN;
        constexpr int end = ENUM_SCAN_END;
        constexpr std::size_t n = static_cast<std::size_t>(end - begin + 1);
        static_assert(implements::enum_range_checker_impl<E, begin>(std::make_index_sequence<n>{}),
                      "Enum has no valid values in scan range!");
        return implements::enum_count_impl<E, begin>(std::make_index_sequence<n>{});
    }

    template <typename E>
    constexpr auto enum_values() noexcept {
        constexpr int begin = ENUM_SCAN_BEGIN;
        constexpr int end = ENUM_SCAN_END;
        constexpr std::size_t n = static_cast<std::size_t>(end - begin + 1);
        return implements::enum_values_impl<enum_count<E>(), E, begin>(std::make_index_sequence<n>{});
    }

    template <typename E, std::size_t Idx>
    constexpr auto enum_value() noexcept -> E {
        constexpr auto values = enum_values<E>();
        return values[Idx];
    }

    template <typename E>
    constexpr auto enum_value(std::size_t idx) noexcept
        -> type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<E>, E> {
        constexpr auto values = enum_values<E>();
        return values[idx];
    }

    template <typename E>
    constexpr auto enum_entries() {
        constexpr int begin = ENUM_SCAN_BEGIN;
        constexpr int end = ENUM_SCAN_END;
        constexpr std::size_t n = static_cast<std::size_t>(end - begin + 1);
        return implements::enum_entries_impl<enum_count<E>(), E, begin>(std::make_index_sequence<n>{});
    }

    template <typename Enum>
    constexpr auto enum_name(Enum EnumValue)
        -> type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<Enum>, std::string_view> {
        constexpr auto entries = enum_entries<Enum>();
        for (const auto &[enum_value, enum_name]: entries) {
            if (enum_value == EnumValue) {
                return enum_name;
            }
        }
        return {};
    }

    template <typename Enum, Enum EnumValue>
    constexpr auto enum_name()
        -> type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<Enum>, std::string_view> {
        return enum_name<Enum, EnumValue>();
    }

    template <typename Enum>
    constexpr auto enum_names() noexcept {
        constexpr int begin = ENUM_SCAN_BEGIN;
        constexpr int end = ENUM_SCAN_END;
        constexpr std::size_t n = static_cast<std::size_t>(end - begin + 1);
        return implements::enum_names_impl<enum_count<Enum>(), Enum, begin>(std::make_index_sequence<n>{});
    }

    template <typename Enum, typename Pred = foundation::functional::equal<>>
    constexpr auto enum_cast(std::string_view name, Pred pred = {}) noexcept
        -> implements::enable_if_t<Enum, std::optional<Enum>, Pred> {
        constexpr auto entries = enum_entries<Enum>();
        for (const auto &[enum_value, enum_name]: entries) {
            if (std::equal(name.begin(), name.end(), enum_name.begin(), enum_name.end(), pred)) {
                return enum_value;
            }
        }
        return std::nullopt;
    }

    template <typename Enum>
    constexpr auto enum_cast(type_traits::other_trans::underlying_type_t<Enum> value) noexcept
        -> implements::enable_if_t<Enum, std::optional<Enum>> {
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
    RAINY_NODISCARD constexpr auto enum_integer(Enum value) noexcept
        -> implements::enable_if_t<Enum, type_traits::other_trans::underlying_type_t<Enum>> {
        return static_cast<type_traits::other_trans::underlying_type_t<Enum>>(value);
    }

    template <typename Enum>
    RAINY_NODISCARD constexpr auto enum_underlying(Enum value) noexcept
        -> implements::enable_if_t<Enum, type_traits::other_trans::underlying_type_t<Enum>> {
        return static_cast<type_traits::other_trans::underlying_type_t<Enum>>(value);
    }

    template <typename Enum>
    RAINY_NODISCARD constexpr auto enum_index(Enum value) noexcept -> implements::enable_if_t<Enum, std::optional<std::size_t>> {
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
    RAINY_NODISCARD constexpr auto enum_index() noexcept -> implements::enable_if_t<Enum, std::optional<std::size_t>> {
        return enum_index<Enum>(EnumValue);
    }

    template <typename Enum, implements::enable_if_t<Enum, int> = 0>
    RAINY_NODISCARD constexpr auto enum_contains(Enum value) noexcept {
        if (enum_cast<Enum>(value).has_value()) {
            return true;
        }
        return false;
    }

    template <typename Enum, Enum Value, implements::enable_if_t<Enum, int> = 0>
    RAINY_NODISCARD constexpr bool enum_contains() noexcept {
        return enum_contains<Enum>(Value);
    }

    template <typename Enum>
    RAINY_NODISCARD constexpr bool enum_contains(type_traits::other_trans::underlying_type_t<Enum> value) noexcept {
        if (enum_cast<Enum>(value).has_value()) {
            return true;
        }
        return false;
    }

    template <typename Enum, typename Pred = foundation::functional::equal<Enum>>
    RAINY_NODISCARD constexpr bool enum_contains(std::string_view name, Pred pred = {}) noexcept {
        if (enum_cast<Enum>(name, pred).has_value()) {
            return true;
        }
        return false;
    }
}

namespace rainy::meta::enumeration {
    template <typename E>
    RAINY_NODISCARD auto enum_flags_name(E value, char sep = static_cast<char>('|')) -> implements::enable_if_t<E, std::string> {
        using D = std::decay_t<E>;
        using U = type_traits::other_trans::underlying_type_t<D>;
        std::string name;
        auto check_value = U{0};
        constexpr auto names = enum_names<E>();
        for (std::size_t i = 0; i < enum_count<E>(); ++i) {
            if (const auto v = static_cast<U>(enum_value<D>(i)); (static_cast<U>(value) & v) != 0) {
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

#endif