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
#ifndef RAINY_UTILITY_HPP
#define RAINY_UTILITY_HPP

#include <algorithm>
#include <bitset>
#include <iomanip>
#include <ostream>
#include <rainy/text/format_wrapper.hpp>
#include <random>
#include <sstream>
#include <utility>

#if RAINY_HAS_CXX20
#include <ranges>
#endif

namespace rainy::utility::implements {
    template <typename TypeList>
    struct get_template_arg_helper {};

    template <typename... Types>
    struct get_template_arg_helper<type_traits::other_trans::type_list<Types...>> {
        static constexpr collections::array<foundation::ctti::typeinfo, sizeof...(Types)> arguments = {
            foundation::ctti::typeinfo::create<Types>()...};
    };
}

namespace rainy::utility {
    template <typename Template>
    constexpr decltype(auto) get_template_arguments_ctti() {
        using namespace type_traits::extras::templates;
        using traits = template_traits<Template>;
        if constexpr (is_template_v<Template>) {
            return implements::get_template_arg_helper<typename traits::type>::arguments;
        }
    }

    template <typename Fx>
    constexpr decltype(auto) get_fn_arguments_ctti() {
        using namespace type_traits::primary_types;
        using namespace type_traits::other_trans;
        using traits = function_traits<Fx>;
        if constexpr (traits::valid) {
            return implements::get_template_arg_helper<
                typename tuple_like_to_type_list<typename traits::tuple_like_type>::type>::arguments;
        }
    }

    template <typename Fx>
    constexpr decltype(auto) get_fn_arguments_ctti(Fx) {
        return get_fn_arguments_ctti<Fx>();
    }
}

namespace rainy::utility::implements {
    template <typename Type>
    struct uses_allocator_construction {
        template <typename Allocator, typename... Params>
        static constexpr auto args([[maybe_unused]] const Allocator &allocator, Params &&...params) noexcept {
            if constexpr (!std::uses_allocator_v<Type, Allocator> && std::is_constructible_v<Type, Params...>) {
                return std::forward_as_tuple(std::forward<Params>(params)...);
            } else {
                static_assert(std::uses_allocator_v<Type, Allocator>, "Ill-formed request");

                if constexpr (std::is_constructible_v<Type, std::allocator_arg_t, const Allocator &, Params...>) {
                    return std::tuple<std::allocator_arg_t, const Allocator &, Params &&...>{std::allocator_arg, allocator,
                                                                                             std::forward<Params>(params)...};
                } else {
                    static_assert(std::is_constructible_v<Type, Params..., const Allocator &>, "Ill-formed request");
                    return std::forward_as_tuple(std::forward<Params>(params)..., allocator);
                }
            }
        }
    };

    template <typename Type, typename Other>
    struct uses_allocator_construction<std::pair<Type, Other>> {
        using type = std::pair<Type, Other>;

        template <typename Allocator, typename First, typename Second>
        static constexpr auto args(const Allocator &allocator, std::piecewise_construct_t, First &&first, Second &&second) noexcept {
            return std::make_tuple(
                std::piecewise_construct,
                std::apply(
                    [&allocator](auto &&...curr) {
                        return uses_allocator_construction<Type>::args(allocator, std::forward<decltype(curr)>(curr)...);
                    },
                    std::forward<First>(first)),
                std::apply(
                    [&allocator](auto &&...curr) {
                        return uses_allocator_construction<Other>::args(allocator, std::forward<decltype(curr)>(curr)...);
                    },
                    std::forward<Second>(second)));
        }

        template <typename Allocator>
        static constexpr auto args(const Allocator &allocator) noexcept {
            return uses_allocator_construction<type>::args(allocator, std::piecewise_construct, std::tuple<>{}, std::tuple<>{});
        }

        template <typename Allocator, typename First, typename Second>
        static constexpr auto args(const Allocator &allocator, First &&first, Second &&second) noexcept {
            return uses_allocator_construction<type>::args(allocator, std::piecewise_construct,
                                                           std::forward_as_tuple(std::forward<First>(first)),
                                                           std::forward_as_tuple(std::forward<Second>(second)));
        }

        template <typename Allocator, typename First, typename Second>
        static constexpr auto args(const Allocator &allocator, const std::pair<First, Second> &value) noexcept {
            return uses_allocator_construction<type>::args(allocator, std::piecewise_construct, std::forward_as_tuple(value.first),
                                                           std::forward_as_tuple(value.second));
        }

        template <typename Allocator, typename First, typename Second>
        static constexpr auto args(const Allocator &allocator, std::pair<First, Second> &&value) noexcept {
            return uses_allocator_construction<type>::args(allocator, std::piecewise_construct,
                                                           std::forward_as_tuple(std::move(value.first)),
                                                           std::forward_as_tuple(std::move(value.second)));
        }
    };
}

namespace rainy::utility {
    template <typename Ty, typename Alloc, typename... Types>
    RAINY_NODISCARD constexpr Ty make_obj_using_allocator(const Alloc &allocator, Types &&..._Args) {
        return std::make_from_tuple<Ty>(implements::uses_allocator_construction<Ty>::args(allocator, utility::forward<Types>(_Args)...));
    }
}

namespace rainy::utility {
    class uuid {
    public:
        uuid() : resources() {};

        uuid(const unsigned long data1, const unsigned short data2, const unsigned short data3,
             const collections::array<unsigned char, 8> data4) :
            resources(data1, data2, data3, data4) {
        }

        bool to_string(std::string &ref, const bool use_capital = true, const bool use_connect = true,
                       const bool use_brace = true) const {
            auto &guid = resources;
            if (is_empty()) {
                return false;
            }
            char connect_char = use_connect ? '-' : 0;
#if RAINY_HAS_CXX20
            ref.resize(40);
            utility::format(ref, "{1:08x}{0}{2:04x}{0}{3:04x}{0}{4:02x}{5:02x}{0}{6:02x}{7:02x}{8:02x}{9:02x}{10:02x}{11:02x}",
                            connect_char, guid.data1, guid.data2, guid.data3, guid.data4[0], guid.data4[1], guid.data4[2],
                            guid.data4[3], guid.data4[4], guid.data4[5], guid.data4[6], guid.data4[7]);
#else
            std::stringstream ss;
            ss << std::hex << std::setfill('0') // 设置为十六进制并填充 0
               << std::setw(8) << guid.data1 << connect_char // 格式化 data1
               << std::setw(4) << guid.data2 << connect_char // 格式化 data2
               << std::setw(4) << guid.data3 << connect_char // 格式化 data3
               << std::setw(2) << static_cast<int>(guid.data4[0]) << connect_char // 格式化 data4[0]
               << std::setw(2) << static_cast<int>(guid.data4[1]) // 格式化 data4[1]
               << std::setw(2) << static_cast<int>(guid.data4[2]) << connect_char // 格式化 data4[2]
               << std::setw(2) << static_cast<int>(guid.data4[3]) // 格式化 data4[3]
               << std::setw(2) << static_cast<int>(guid.data4[4]) // 格式化 data4[4]
               << std::setw(2) << static_cast<int>(guid.data4[5]) << connect_char // 格式化 data4[5]
               << std::setw(2) << static_cast<int>(guid.data4[6]) // 格式化 data4[6]
               << std::setw(2) << static_cast<int>(guid.data4[7]); // 格式化 data4[7]
            ref = ss.str();
#endif
            if (use_capital) {
#if RAINY_HAS_CXX20
                std::ranges::transform(ref.begin(), ref.end(), ref.begin(),
                                       [](const char c) { return static_cast<const char>(std::toupper(static_cast<int>(c))); });
#else
                std::transform(ref.begin(), ref.end(), ref.begin(),
                               [](const char c) { return static_cast<const char>(std::toupper(static_cast<int>(c))); });
#endif
            }
            if (use_brace) {
                ref = '{' + ref + '}';
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
            std::wstringstream ss;
            ss << std::hex << std::setfill(L'0')  // 设置为十六进制并填充 0
               << std::setw(8) << guid.data1 << connect_char  // 格式化 data1
               << std::setw(4) << guid.data2 << connect_char  // 格式化 data2
               << std::setw(4) << guid.data3 << connect_char  // 格式化 data3
               << std::setw(2) << static_cast<int>(guid.data4[0]) << connect_char  // 格式化 data4[0]
               << std::setw(2) << static_cast<int>(guid.data4[1])  // 格式化 data4[1]
               << std::setw(2) << static_cast<int>(guid.data4[2]) << connect_char  // 格式化 data4[2]
               << std::setw(2) << static_cast<int>(guid.data4[3])  // 格式化 data4[3]
               << std::setw(2) << static_cast<int>(guid.data4[4])  // 格式化 data4[4]
               << std::setw(2) << static_cast<int>(guid.data4[5]) << connect_char  // 格式化 data4[5]
               << std::setw(2) << static_cast<int>(guid.data4[6])  // 格式化 data4[6]
               << std::setw(2) << static_cast<int>(guid.data4[7]); // 格式化 data4[7]
            ref = ss.str();
#endif
            if (use_capital) {
#if RAINY_HAS_CXX20
                std::ranges::transform(ref.begin(), ref.end(), ref.begin(),
                                       [](const wchar_t c) { return static_cast<wchar_t>(std::toupper(c)); });
#else
                std::transform(ref.begin(), ref.end(), ref.begin(),
                               [](const wchar_t c) { return static_cast<wchar_t>(std::toupper(static_cast<int>(c))); });
#endif
            }
            if (use_brace) {
                ref = L'{' + ref + L'}';
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

        RAINY_NODISCARD bool is_empty() const {
            const auto &guid = resources;
            const auto &data = guid.data4;
            const bool cond1 = guid.data1 == 0x0 && guid.data2 == 0x0 && guid.data3 == 0x0;
            const bool cond2 = std::all_of(data.cbegin(), data.cend(), [](const unsigned char c) { return c == 0x0; });
            return cond1 && cond2;
        }

        RAINY_NODISCARD bool equal(const uuid &right) const {
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
            std::uniform_int_distribution<uint32_t> dist32(0, (utility::numeric_limits<uint32_t>::max)());
            std::uniform_int_distribution<uint16_t> dist16(0, (utility::numeric_limits<uint16_t>::max)());
            std::uniform_int_distribution<unsigned short> dist8(0, (utility::numeric_limits<unsigned short>::max)());
            unsigned long data1 = dist32(gen);
            unsigned short data2 = dist16(gen);
            unsigned short data3 = dist16(gen);
            data3 &= 0x0FFF;
            data3 |= (4 << 12);
            collections::array<unsigned char, 8> data4{};
            for (int i = 0; i < 8; ++i) {
                data4[i] = static_cast<unsigned char>(dist8(gen));
            }
            data4[0] &= 0x3F;
            data4[0] |= 0x80;
            return uuid{data1, data2, data3, data4};
        }

        auto &get_raw() {
            return resources;
        }

        RAINY_NODISCARD const auto &get_raw() const {
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
        class implements {
        public:
            friend class uuid;

            implements() = default;

            /*
             * 此处要阻止Clion的结构化绑定建议警告
             *
             * 因为uuid使用这个结构体进行管理。Clion认为可以使用结构化绑定替换，但是这样做可能有
             * 可读性较差的情况。因此，我们创建了一个类，并伪装成一个接口。但是，它实际上implements定义的成员函数
             * 永远不可能会被调用，始终为占位。
             */
            implements(const unsigned long data1, const unsigned short data2, const unsigned short data3,
                      const collections::array<unsigned char, 8> &data4) :
                data1(data1),
                data2(data2), data3(data3), data4(data4) {
            }

            RAINY_NODISCARD unsigned long get_data1() const noexcept {
                return data1;
            }

            RAINY_NODISCARD unsigned short get_data2() const noexcept {
                return data2;
            }

            RAINY_NODISCARD unsigned short get_data3() const noexcept {
                return data3;
            }

            collections::array<unsigned char, 8> &get_data4() noexcept {
                return data4;
            }

        private:
            unsigned long data1;
            unsigned short data2;
            unsigned short data3;
            collections::array<unsigned char, 8> data4;
        };

        implements resources;
    };

    RAINY_INLINE uuid make_uuid() {
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

#endif
