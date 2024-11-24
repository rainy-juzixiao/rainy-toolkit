#ifndef RAINY_UTILITY_HPP
#define RAINY_UTILITY_HPP

#include <rainy/base.hpp>
#include <rainy/meta/type_traits.hpp>
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
    struct get_ptr_difference_type<Ty, type_traits::primary_types::is_void<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    template <typename Ty, typename other, typename = void>
    struct get_rebind_alias {
        using type = typename replace_first_parameter<other, Ty>::type;
    };

    template <typename Ty, typename other>
    struct get_rebind_alias<Ty, other, type_traits::other_trans::void_t<typename Ty::template rebind<other>>> {
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

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<elem>, char, elem> &;

        RAINY_NODISCARD static RAINY_CONSTEXPR20 pointer pointer_to(ref_type val) noexcept(noexcept(Ty::pointer_to(val))) {
            return Ty::pointer_to(val);
        }
    };

    template <typename, typename = void, typename = void>
    struct ptr_traits_sfinae_layer {};

    template <typename Ty, typename Uty>
    struct ptr_traits_sfinae_layer<Ty, Uty, type_traits::other_trans::void_t<typename utility::get_first_parameter<Ty>::type>>
        : pointer_traits_base<Ty, typename utility::get_first_parameter<Ty>::type> {};

    template <typename Ty>
    struct ptr_traits_sfinae_layer<Ty, type_traits::other_trans::void_t<typename Ty::elementType>, void>
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

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Ty>, char, Ty> &;

        RAINY_NODISCARD static RAINY_CONSTEXPR20 pointer pointer_to(ref_type val) noexcept {
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
    static_assert(rainy::type_traits::concepts::formattable_with<Ty1, std::format_context>, "Ty1 Is A unsupported type");
    static_assert(rainy::type_traits::concepts::formattable_with<Ty2, std::format_context>, "Ty2 Is A unsupported type");

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
             const containers::array<unsigned char, 8> data4) :
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
            containers::array<unsigned char, 8> data4{};
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
                      const containers::array<unsigned char, 8> &data4) :
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

            containers::array<unsigned char, 8> &get_data4() noexcept {
                return data4;
            }

        private:
            unsigned long data1;
            unsigned short data2;
            unsigned short data3;
            containers::array<unsigned char, 8> data4;
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

/*
[N4849 13.7.5 Class template partial specializations(temp.class.spec) - 10]:
    The usual access checking rules do not apply to non-dependent names used to specify template arguments of the simple-template-id of the partial specialization. 
    [Note: The template arguments may be private types or objects that would normally not be accessible. Dependent names cannot be checked when declaring the partial specialization, but will be checked when substituting into the partial specialization. —end note] 
    通常的访问检查规则不适用于用于指定显式实例化的名称。
    [注意：特别是，函数声明符中使用的模板参数和名称（包括参数类型、返回类型和异常规范）可能是通常无法访问的私有类型或对象。而模板可能是通常无法访问的成员模板或成员函数。 -结束注释]
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

// 更新后的宏定义
#define RAINY_MAKE_PRIVATE_ACCESSIBLE(TYPE,...)                                                                                               \
    namespace rainy::utility {\
    template <> private_access<TYPE, ##__VA_ARGS__>; \
    inline constexpr auto get_private_ptrs(private_access_tag_t<TYPE>);\
    }


namespace rainy::type_traits::extras::tuple::internals {
    template <std::size_t N, typename Ty, typename = void>
    struct bind : type_traits::helper::integral_constant<std::size_t, 0> {};

#define RAINY_DECLARE_BIND(N)                                                                                                         \
    template <typename Ty>                                                                                                            \
    struct bind<N, Ty, rainy::type_traits::other_trans::void_t<decltype(Ty{RAINY_INITIALIZER_LIST(N)})>>                    \
        : rainy::type_traits::helper::integral_constant<std::size_t, N> {}

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
}

namespace rainy::type_traits::extras::tuple::internals {
    template <std::size_t N, typename Ty>
    inline static constexpr auto bind_v = bind<N, Ty>::value;

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
    struct refl_to_tuple_impl<N, Ty> : std::integral_constant<std::size_t, N> {                                                            \
        static constexpr auto make() noexcept {                                                                                       \
            auto &[RAINY_TO_TUPLE_EXPAND_ARGS(N)] =                                                                                   \
                type_traits::helper::get_fake_object<type_traits::cv_modify::remove_cvref_t<Ty>>();                                   \
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

    template <typename Ty>
    static inline constexpr std::size_t eval_member_count =
        (std::max)({internals::bind_v<0, Ty>,  internals::bind_v<1, Ty>,  internals::bind_v<2, Ty>,  internals::bind_v<3, Ty>,
                    internals::bind_v<4, Ty>,  internals::bind_v<5, Ty>,  internals::bind_v<6, Ty>,  internals::bind_v<7, Ty>,
                    internals::bind_v<8, Ty>,  internals::bind_v<9, Ty>,  internals::bind_v<10, Ty>, internals::bind_v<11, Ty>,
                    internals::bind_v<12, Ty>, internals::bind_v<13, Ty>, internals::bind_v<14, Ty>, internals::bind_v<15, Ty>,
                    internals::bind_v<16, Ty>, internals::bind_v<17, Ty>, internals::bind_v<18, Ty>, internals::bind_v<19, Ty>,
                    internals::bind_v<20, Ty>, internals::bind_v<21, Ty>, internals::bind_v<22, Ty>, internals::bind_v<23, Ty>,
                    internals::bind_v<24, Ty>, internals::bind_v<25, Ty>, internals::bind_v<26, Ty>, internals::bind_v<27, Ty>,
                    internals::bind_v<28, Ty>, internals::bind_v<29, Ty>, internals::bind_v<30, Ty>, internals::bind_v<31, Ty>,
                    internals::bind_v<32, Ty>, internals::bind_v<33, Ty>, internals::bind_v<34, Ty>, internals::bind_v<35, Ty>,
                    internals::bind_v<36, Ty>, internals::bind_v<37, Ty>, internals::bind_v<38, Ty>, internals::bind_v<39, Ty>,
                    internals::bind_v<40, Ty>, internals::bind_v<41, Ty>, internals::bind_v<42, Ty>, internals::bind_v<43, Ty>,
                    internals::bind_v<44, Ty>, internals::bind_v<45, Ty>, internals::bind_v<46, Ty>, internals::bind_v<47, Ty>,
                    internals::bind_v<48, Ty>, internals::bind_v<49, Ty>, internals::bind_v<50, Ty>, internals::bind_v<51, Ty>,
                    internals::bind_v<52, Ty>, internals::bind_v<53, Ty>, internals::bind_v<54, Ty>, internals::bind_v<55, Ty>,
                    internals::bind_v<56, Ty>, internals::bind_v<57, Ty>, internals::bind_v<58, Ty>, internals::bind_v<59, Ty>,
                    internals::bind_v<60, Ty>, internals::bind_v<61, Ty>, internals::bind_v<62, Ty>, internals::bind_v<63, Ty>,
                    internals::bind_v<64, Ty>, internals::bind_v<65, Ty>, internals::bind_v<66, Ty>, internals::bind_v<67, Ty>,
                    internals::bind_v<68, Ty>, internals::bind_v<69, Ty>, internals::bind_v<70, Ty>, internals::bind_v<71, Ty>,
                    internals::bind_v<72, Ty>, internals::bind_v<73, Ty>, internals::bind_v<74, Ty>, internals::bind_v<75, Ty>,
                    internals::bind_v<76, Ty>, internals::bind_v<77, Ty>, internals::bind_v<78, Ty>, internals::bind_v<79, Ty>});
}

namespace rainy::type_traits::extras::tuple {
    template <typename Ty>
    struct member_count {
        static inline constexpr std::size_t value = internals::eval_member_count<Ty>;
    };

    template <typename Ty>
    static constexpr std::size_t member_count_v = member_count<Ty>::value;

    template <typename Ty>
    inline static constexpr auto struct_to_tuple = internals::refl_to_tuple_impl<member_count_v<Ty>, Ty>::make;
}

namespace rainy::utility {
    using type_traits::extras::tuple::member_count_v;
    using type_traits::extras::tuple::member_count;
    using type_traits::extras::tuple::struct_to_tuple;
}

namespace rainy::utility::internals {
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
#if RAINY_HAS_CXX20
    template <typename Ty>
    constexpr auto get_member_names() {
        constexpr bool has_get_private_ptrs_v = internals::has_get_private_ptrs<Ty>::value;
        static_assert(member_count_v<Ty> != 0 || has_get_private_ptrs_v, "Failed!");
        if constexpr (!has_get_private_ptrs_v) {
            containers::array<std::string_view, member_count_v<Ty>> array{}; // 创建对应的数组
            constexpr auto tp = struct_to_tuple<Ty>();
            [&array, &tp]<std::size_t... I>(std::index_sequence<I...>) mutable {
                ((array[I] = variable_name<(std::get<I>(tp))>()), ...);
            }(std::make_index_sequence<member_count_v<Ty>>{});
            return array;
        } else {
            constexpr auto tp = internals::get_private_ptrs_helper<Ty>::value; // 使用 helper 获取 `tp`
            constexpr size_t tuple_size = std::tuple_size_v<decltype(tp)>;
            containers::array<std::string_view, tuple_size> array{};
            [&array, &tp]<std::size_t... I>(std::index_sequence<I...>) mutable {
                ((array[I] = variable_name<(std::get<I>(tp))>()), ...);
            }(std::make_index_sequence<tuple_size>{});
            return array;
        }
    }
#endif
}

#endif
