#ifndef RAINY_UTILITY_HPP
#define RAINY_UTILITY_HPP

#include <rainy/base.hpp>
#include <rainy/meta_programming/type_traits.hpp>
#include <random>
#include <bitset>
#include <ostream>

namespace rainy::utility {
    template <typename Ty>
    struct get_first_parameter;

    template <template <class, class...> typename Ty, typename first, typename... rest>
    struct get_first_parameter<Ty<first, rest...>> {
        using type = first;
    };

    template <typename newfirst, typename Ty>
    struct replace_first_parameter;

    template <typename newfirst, template <typename, typename...> typename Ty, typename first, typename... rest>
    struct replace_first_parameter<newfirst, Ty<first, rest...>> {
        using type = Ty<newfirst, rest...>;
    };

    template <typename, typename = void>
    struct get_ptr_difference_type {
        using type = ptrdiff_t;
    };

    template <typename Ty>
    struct get_ptr_difference_type<Ty, foundation::type_traits::primary_types::is_void<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    template <typename Ty, typename other, typename = void>
    struct get_rebind_alias {
        using type = typename replace_first_parameter<other, Ty>::type;
    };

    template <typename Ty, typename other>
    struct get_rebind_alias<Ty, other, foundation::type_traits::other_transformations::void_t<typename Ty::template rebind<other>>> {
        using type = typename Ty::template rebind<other>;
    };
}

namespace rainy::foundation::system::memory {
    template <typename Ty, typename elem>
    struct pointer_traits_base {
        using pointer = Ty;
        using element_type = elem;
        using difference_type = typename utility::get_ptr_difference_type<Ty>::type;

        template <typename other>
        using rebind = typename utility::get_rebind_alias<Ty, other>::type;

        using ref_type = type_traits::other_transformations::conditional_t<type_traits::primary_types::is_void_v<elem>, char, elem> &;

        RAINY_NODISCARD static rainy_constEXPR20 pointer pointer_to(ref_type val) noexcept(noexcept(Ty::pointer_to(val))) {
            return Ty::pointer_to(val);
        }
    };

    template <typename, typename = void, typename = void>
    struct ptr_traits_sfinae_layer {};

    template <typename Ty, typename Uty>
    struct ptr_traits_sfinae_layer<Ty, Uty, type_traits::other_transformations::void_t<typename utility::get_first_parameter<Ty>::type>>
        : pointer_traits_base<Ty, typename utility::get_first_parameter<Ty>::type> {};

    template <typename Ty>
    struct ptr_traits_sfinae_layer<Ty, type_traits::other_transformations::void_t<typename Ty::elementType>, void>
        : pointer_traits_base<Ty, typename Ty::elementType> {};

    template <typename Ty>
    struct pointer_traits : ptr_traits_sfinae_layer<Ty> {};

    template <typename Ty>
    struct pointer_traits<Ty *> {
        using pointer = Ty *;
        using elemen_type = Ty;
        using difference_type = ptrdiff_t;

        template <typename other>
        using rebind = other *;

        using ref_type = type_traits::other_transformations::conditional_t<type_traits::primary_types::is_void_v<Ty>, char, Ty> &;

        RAINY_NODISCARD static rainy_constEXPR20 pointer pointer_to(ref_type val) noexcept {
            return std::addressof(val);
        }
    };
}

namespace rainy::utility {
    template <typename Ty1, typename Ty2>
    struct pair {
        using first_type = Ty1;
        using second_type = Ty2;

        template <
            typename uty1 = Ty1, typename uty2 = Ty2,
            std::enable_if_t<std::conjunction_v<std::is_default_constructible<uty1>, std::is_default_constructible<uty2>>, int> = 0>
        constexpr pair() noexcept(std::is_nothrow_default_constructible_v<Ty1> && std::is_nothrow_default_constructible_v<Ty1>) :
            first(), second() {
        }

        pair(const pair &) = default;

        pair(pair &&) = default;

        template <typename uty1 = Ty1, typename uty2 = Ty2,
                  typename = std::enable_if_t<std::conjunction_v<std::is_copy_constructible<uty1>, std::is_copy_constructible<uty2>>>>
        constexpr pair(const Ty1 &val1, const Ty2 &val2) noexcept(std::is_nothrow_copy_constructible_v<uty1> &&
                                                                  std::is_nothrow_copy_constructible_v<uty2>) :
            first(val1),
            second(val2) {
        }

        template <typename other1, typename other2,
                  typename = std::enable_if_t<
                      std::conjunction_v<std::is_constructible<Ty1, const other1 &>, std::is_constructible<Ty2, const other2 &>>>>
        constexpr pair(const pair<other1, other2> &right) noexcept(std::is_nothrow_constructible_v<Ty1, other1> &&
                                                                   std::is_nothrow_constructible_v<Ty2, other2>) :
            first(right.first),
            second(right.second) {
        }

        template <
            typename other1, typename other2,
            typename = std::enable_if_t<std::conjunction_v<std::is_constructible<Ty1, other1>, std::is_constructible<Ty2, other2>>>>
        constexpr pair(const pair<other1, other2> &&right) noexcept(std::is_nothrow_constructible_v<Ty1, other1> &&
                                                                    std::is_nothrow_constructible_v<Ty2, other2>) :
            first(std::exchange(right.first, {})),
            second(std::exchange(right.second), {}) {
        }

        template <
            typename other1, typename other2,
            typename = std::enable_if_t<std::conjunction_v<std::is_constructible<Ty1, other1>, std::is_constructible<Ty2, other2>>>>
        constexpr pair(other1 &&val1, other2 &&val2) noexcept(std::is_nothrow_constructible_v<Ty1, other1> &&
                                                              std::is_nothrow_constructible_v<Ty2, other2>) :
            first(std::forward<other1>(val1)),
            second(std::forward<other2>(val2)) {
        }

        template <typename... Tuple1, typename... Tuple2, size_t... Indices1, size_t... Indices2>
        constexpr pair(std::tuple<Tuple1...> first_args, std::tuple<Tuple2...> second_args, std::index_sequence<Indices1...>,
                       std::index_sequence<Indices2...>) :
            first(std::get<Indices1>(std::move(first_args))...),
            second(std::get<Indices2>(std::move(second_args))...) {
        }

        template <typename... Args1, typename... Args2>
        constexpr pair(std::piecewise_construct_t, std::tuple<Args1...> first_args, std::tuple<Args2...> second_args) :
            pair(first_args, second_args, std::index_sequence_for<Args1...>{}, std::index_sequence_for<Args2...>{}) {
        }

        pair &operator=(const volatile pair &) = delete;

        template <std::enable_if_t<std::conjunction_v<std::is_copy_assignable<Ty1>, std::is_copy_assignable<Ty2>>, int> = 0>
        pair &operator=(const pair &p) {
            first = p.first;
            second = p.second;
            return *this;
        }

        template <typename U1, typename U2,
                  typename = std::enable_if_t<
                      std::conjunction_v<std::is_assignable<Ty1 &, const U1 &>, std::is_assignable<Ty2 &, const U2 &>>>>
        pair &operator=(const pair<U1, U2> &p) {
            first = p.first;
            second = p.second;
            return *this;
        }

        pair &operator=(pair &&p) noexcept {
            first = p.first;
            second = p.second;
            return *this;
        }

        template <typename U1, typename U2>
        pair &operator=(pair<U1, U2> &&p) {
            first = std::exchange(p.first, {});
            second = std::exchange(p.second, {});
            return *this;
        }

        void swap(pair &p) noexcept(std::is_nothrow_swappable_v<first_type> && std::is_nothrow_swappable_v<second_type>) {
            if (this != std::addressof(p)) {
                std::swap(first, p.first);
                std::swap(second, p.second);
            }
        }

        Ty1 first;
        Ty2 second;
    };
}


#if RAINY_HAS_CXX20
template <typename Ty1,typename Ty2>
class std::formatter<rainy::utility::pair<Ty1, Ty2>, char> {
public:
    static_assert(rainy::foundation::type_traits::concepts::formattable_with<Ty1, std::format_context>, "Ty1 Is A unsupported type");
    static_assert(rainy::foundation::type_traits::concepts::formattable_with<Ty2, std::format_context>, "Ty2 Is A unsupported type");

    explicit formatter() noexcept {
    }

    auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    auto format(const rainy::utility::pair<Ty1,Ty2> &value, std::format_context fc) const noexcept {
        return std::format_to(fc.out(), "{}:{}", value.first, value.second);
    }
};
#endif

namespace rainy::utility {
    class uuid {
    public:
        constexpr uuid() : resources(){};

        uuid(const unsigned long data1, const unsigned short data2, const unsigned short data3,
             const foundation::containers::array<unsigned char, 8> data4) :
            resources(data1, data2, data3, data4) {
        }

        bool to_string(std::string &ref, const bool use_capital = true, const bool use_connect = true,
                       const bool use_brace = true) const {
            auto &guid = resources;
            if (is_empty()) {
                return false;
            }
            const char *connect_char = use_connect ? "-" : "";
            ref.resize(40);
#if RAINY_HAS_CXX20
            utility::format(ref, "{1:08x}{0}{2:04x}{0}{3:04x}{0}{4:02x}{5:02x}{0}{6:02x}{7:02x}{8:02x}{9:02x}{10:02x}{11:02x}",
                            connect_char, guid.data1, guid.data2, guid.data3, guid.data4[0], guid.data4[1], guid.data4[2],
                            guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
#else
            utility::format(ref, "%08x%s%04x%s%04x%s%02x%02x%s%02x%02x%02x%02x%02x%02x", guid.data1, connect_char, guid.data2,
                            connect_char, guid.data3, connect_char, guid.data4[0], guid.data4[1], connect_char, guid.data4[2],
                            guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
#endif
            if (use_capital) {
#if RAINY_HAS_CXX20
                std::ranges::transform(ref.begin(), ref.end(), ref.begin(), [](const wchar_t c) { return std::toupper(c); });
#else
                std::transform(ref.begin(), ref.end(), ref.begin(), [](const wchar_t c) { return std::toupper(c); });
#endif
            }
            if (use_brace) {
                ref = "{" + ref + "}";
            }
            return true;
        }

        bool to_wstring(std::wstring &ref, const bool use_capital = true, const bool use_connect = true,
                        const bool use_brace = true) const {
            auto &guid = resources;
            if (is_empty()) {
                return false;
            }
            const wchar_t *connect_char = use_connect ? L"-" : L"";
#if RAINY_HAS_CXX20
            utility::format(ref, L"{1:08x}{0}{2:04x}{0}{3:04x}{0}{4:02x}{5:02x}{0}{6:02x}{7:02x}{8:02x}{9:02x}{10:02x}{11:02x}",
                            connect_char, guid.data1, guid.data2, guid.data3, guid.data4[0], guid.data4[1], guid.data4[2],
                            guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
#else
            utility::format(ref, L"%08x%s%04x%s%04x%s%02x%02x%s%02x%02x%02x%02x%02x%02x", guid.data1, connect_char, guid.data2,
                            connect_char, guid.data3, connect_char, guid.data4[0], guid.data4[1], connect_char, guid.data4[2],
                            guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
#endif
            if (use_capital) {
#if RAINY_HAS_CXX20
                std::ranges::transform(ref.begin(), ref.end(), ref.begin(), [](const wchar_t c) { return std::toupper(c); });
#else
                std::transform(ref.begin(), ref.end(), ref.begin(), [](const wchar_t c) { return std::toupper(c); });
#endif
            }
            if (use_brace) {
                ref = L"{" + ref + L"}";
            }
            return true;
        }

        RAINY_NODISCARD std::string to_string(const bool use_capital = true, const bool use_connect = true,
                                                 const bool use_brace = true) const {
            if (std::string out{}; this->to_string(out, use_capital, use_connect, use_brace)) {
                return out;
            }
            return {};
        }

        RAINY_NODISCARD std::wstring to_wstring(const bool use_capital = true, const bool use_connect = true,
                                                   const bool use_brace = true) const {
            if (std::wstring out{}; this->to_wstring(out, use_capital, use_connect, use_brace)) {
                return out;
            }
            return {};
        }

        bool is_empty() const {
            const auto &guid = resources;
            const auto &data = guid.data4;
            const bool cond1 = guid.data1 == 0x0 && guid.data2 == 0x0 && guid.data3 == 0x0;
            const bool cond2 = std::all_of(data.cbegin(), data.cend(), [](const unsigned char c) { return c == 0x0; });
            return cond1 && cond2;
        }

        bool equal(const uuid &right) const {
            const auto &this_resource = resources;
            const auto &right_resource = right.resources;
            return this_resource.data1 == right_resource.data1 && this_resource.data2 == right_resource.data2 &&
                   this_resource.data3 == right_resource.data3 &&
                   std::memcmp(&this_resource.data4[0], &right_resource.data4[0], sizeof(unsigned char) * 8) == 0;
        }

        explicit operator bool() const {
            return !is_empty();
        }

        friend bool operator==(const uuid &left, const uuid &right) {
            return left.equal(right);
        }

        friend bool operator!=(const uuid &left, const uuid &right) {
            return !left.equal(right);
        }

        static uuid generate() noexcept {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint32_t> dist32(0, (std::numeric_limits<uint32_t>::max)());
            std::uniform_int_distribution<uint16_t> dist16(0, (std::numeric_limits<uint16_t>::max)());
            std::uniform_int_distribution<unsigned short> dist8(0, (std::numeric_limits<unsigned short>::max)());
            unsigned long data1 = dist32(gen);
            unsigned short data2 = dist16(gen);
            unsigned short data3 = dist16(gen);
            data3 &= 0x0FFF;
            data3 |= (4 << 12);
            foundation::containers::array<unsigned char, 8> data4{};
            for (int i = 0; i < 8; ++i) {
                data4[i] = dist8(gen);
            }
            data4[0] &= 0x3F;
            data4[0] |= 0x80;
            return uuid{data1, data2, data3, data4};
        }

        auto &get_raw() {
            return resources;
        }

        const auto &get_raw() const {
            return resources;
        }

        friend std::ostream &operator<<(std::ostream &ostream, const uuid &uid) {
            ostream << uid.to_string();
            return ostream;
        }

        friend std::wostream &operator<<(std::wostream &wostream, const uuid &uid) {
            wostream << uid.to_wstring();
            return wostream;
        }

    private:
        class internals {
        public:
            friend class uuid;

            internals() = default;

            /*
             * 此处要阻止Clion的结构化绑定建议警告
             *
             * 因为uuid使用这个结构体进行管理。Clion认为可以使用结构化绑定替换，但是这样做可能有
             * 可读性较差的情况。因此，我们创建了一个类，并伪装成一个接口。但是，它实际上internals定义的成员函数
             * 永远不可能会被调用，始终为占位。
             */
            internals(const unsigned long data1, const unsigned short data2, const unsigned short data3,
                      const foundation::containers::array<unsigned char, 8> &data4) :
                data1(data1),
                data2(data2), data3(data3), data4(data4) {
            }

            unsigned long get_data1() const noexcept {
                return data1;
            }

            unsigned short get_data2() const noexcept {
                return data2;
            }

            unsigned short get_data3() const noexcept {
                return data3;
            }

            foundation::containers::array<unsigned char, 8> &get_data4() noexcept {
                return data4;
            }

        private:
            unsigned long data1;
            unsigned short data2;
            unsigned short data3;
            foundation::containers::array<unsigned char, 8> data4;
        };

        internals resources;
    };

    uuid make_uuid() {
        return uuid::generate();
    }
}

#if RAINY_HAS_CXX20
template <>
class std::formatter<rainy::utility::uuid, char> {
public:
    explicit formatter() noexcept {
    }

    auto parse(format_parse_context &ctx) const noexcept {
        return ctx.begin();
    }

    auto format(const rainy::utility::uuid &value, std::format_context fc) const noexcept {
        return std::format_to(fc.out(), "{}", value.to_string());
    }
};
#endif

namespace rainy::utility::internals {
    template <std::size_t N, typename Ty, typename = void>
    struct bind : std::integral_constant<std::size_t, 0> {};

    template <typename Ty>
    struct bind<1, Ty, std::void_t<decltype(Ty{{}})>> : std::integral_constant<std::size_t, 1> {};

    template <typename Ty>
    struct bind<2, Ty, std::void_t<decltype(Ty{{}, {}})>> : std::integral_constant<std::size_t, 2> {};

    template <typename Ty>
    struct bind<3, Ty, std::void_t<decltype(Ty{{}, {}, {}})>> : std::integral_constant<std::size_t, 3> {};

    template <typename Ty>
    struct bind<4, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}})>> : std::integral_constant<std::size_t, 4> {};

    template <typename Ty>
    struct bind<5, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}})>> : std::integral_constant<std::size_t, 5> {};

    template <typename Ty>
    struct bind<6, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}})>> : std::integral_constant<std::size_t, 6> {};

    template <typename Ty>
    struct bind<7, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}})>> : std::integral_constant<std::size_t, 7> {};

    template <typename Ty>
    struct bind<8, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}})>> : std::integral_constant<std::size_t, 8> {};

    template <typename Ty>
    struct bind<9, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}})>> : std::integral_constant<std::size_t, 9> {};

    template <typename Ty>
    struct bind<10, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>> : std::integral_constant<std::size_t, 10> {
    };

    template <std::size_t N, typename Ty>
    inline static constexpr auto bind_v = bind<N, Ty>::value;

    template <std::size_t N, typename Ty>
    struct to_tuple;

    template <typename Ty>
    struct to_tuple<0, Ty> : std::integral_constant<std::size_t, 0> {
        static constexpr auto make = [](auto &&x) noexcept { return std::make_tuple(); };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<1, Ty> : std::integral_constant<std::size_t, 1> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1] = x;
            return std::make_tuple(_1);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<2, Ty> : std::integral_constant<std::size_t, 2> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2] = x;
            return std::make_tuple(_1, _2);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<3, Ty> : std::integral_constant<std::size_t, 3> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3] = x;
            return std::make_tuple(_1, _2, _3);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<4, Ty> : std::integral_constant<std::size_t, 4> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4] = x;
            return std::make_tuple(_1, _2, _3);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<5, Ty> : std::integral_constant<std::size_t, 5> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5] = x;
            return std::make_tuple(_1, _2, _3, _4, _5);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<6, Ty> : std::integral_constant<std::size_t, 6> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<7, Ty> : std::integral_constant<std::size_t, 7> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<8, Ty> : std::integral_constant<std::size_t, 8> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<9, Ty> : std::integral_constant<std::size_t, 9> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<10, Ty> : std::integral_constant<std::size_t, 10> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

#ifndef RAINY_BIND_STRUCT_MORE_MEMBERS
#define RAINY_BIND_STRUCT_MORE_MEMBERS 0
#endif

/* 我在思考哪个神经病没事要那么多成员？ */
#if RAINY_BIND_STRUCT_MORE_MEMBERS
    template <typename Ty>
    struct bind<11, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 11> {};

    template <typename Ty>
    struct bind<12, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 12> {};

    template <typename Ty>
    struct bind<13, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 13> {};

    template <typename Ty>
    struct bind<14, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 14> {};

    template <typename Ty>
    struct bind<15, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 15> {};

    template <typename Ty>
    struct bind<16, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 16> {};

    template <typename Ty>
    struct bind<17, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 17> {};

    template <typename Ty>
    struct bind<18, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 18> {};

    template <typename Ty>
    struct bind<19, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 19> {};

    template <typename Ty>
    struct bind<20, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 20> {};

    template <typename Ty>
    struct bind<21, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 21> {};

    template <typename Ty>
    struct bind<22, Ty,
                std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 22> {};

    template <typename Ty>
    struct bind<23, Ty,
                std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 23> {};

    template <typename Ty>
    struct bind<24, Ty, std::void_t<decltype(Ty{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
                                                {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}})>>
        : std::integral_constant<std::size_t, 24> {};

    template <typename Ty>
    struct to_tuple<11, Ty> : std::integral_constant<std::size_t, 11> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<12, Ty> : std::integral_constant<std::size_t, 12> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<13, Ty> : std::integral_constant<std::size_t, 13> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<14, Ty> : std::integral_constant<std::size_t, 14> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14);
        };

        using type = decltype(make(std::declval<Ty>()));
    };


    template <typename Ty>
    struct to_tuple<15, Ty> : std::integral_constant<std::size_t, 15> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<16, Ty> : std::integral_constant<std::size_t, 16> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<17, Ty> : std::integral_constant<std::size_t, 17> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<18, Ty> : std::integral_constant<std::size_t, 18> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<19, Ty> : std::integral_constant<std::size_t, 19> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<20, Ty> : std::integral_constant<std::size_t, 20> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<21, Ty> : std::integral_constant<std::size_t, 21> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<22, Ty> : std::integral_constant<std::size_t, 22> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21,
                                   _22);
        };

        using type = decltype(make(std::declval<Ty>()));
    };


    template <typename Ty>
    struct to_tuple<23, Ty> : std::integral_constant<std::size_t, 23> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22,
                                   _23);
        };

        using type = decltype(make(std::declval<Ty>()));
    };

    template <typename Ty>
    struct to_tuple<24, Ty> : std::integral_constant<std::size_t, 24> {
        static constexpr auto make = [](auto &&x) noexcept {
            auto [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24] = x;
            return std::make_tuple(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22,
                                   _23, _24);
        };

        using type = decltype(make(std::declval<Ty>()));
    };
#endif
}

namespace rainy::utility {
#if RAINY_BIND_STRUCT_MORE_MEMBERS
    template <typename Ty>
    static constexpr auto member_count =
        (std::max)({internals::bind_v<0, Ty>,  internals::bind_v<1, Ty>,  internals::bind_v<2, Ty>,  internals::bind_v<3, Ty>,
                    internals::bind_v<4, Ty>,  internals::bind_v<5, Ty>,  internals::bind_v<6, Ty>,  internals::bind_v<7, Ty>,
                    internals::bind_v<8, Ty>,  internals::bind_v<9, Ty>,  internals::bind_v<10, Ty>, internals::bind_v<11, Ty>,
                    internals::bind_v<12, Ty>, internals::bind_v<13, Ty>, internals::bind_v<14, Ty>, internals::bind_v<15, Ty>,
                    internals::bind_v<16, Ty>, internals::bind_v<17, Ty>, internals::bind_v<18, Ty>, internals::bind_v<19, Ty>,
                    internals::bind_v<20, Ty>, internals::bind_v<21, Ty>, internals::bind_v<22, Ty>, internals::bind_v<23, Ty>,
                    internals::bind_v<24, Ty>});
#else
    template <typename Ty>
    static constexpr auto member_count =
        (std::max)({internals::bind_v<0, Ty>, internals::bind_v<1, Ty>, internals::bind_v<2, Ty>, internals::bind_v<3, Ty>,
                    internals::bind_v<4, Ty>, internals::bind_v<5, Ty>, internals::bind_v<6, Ty>, internals::bind_v<7, Ty>,
                    internals::bind_v<8, Ty>, internals::bind_v<9, Ty>, internals::bind_v<10, Ty>});
#endif

    template <typename Ty>
    inline static constexpr auto make_to_tuple = internals::to_tuple<member_count<Ty>, Ty>::make;
}

#endif
