#ifndef RAINY_COMPONENT_JSON_HPP
#define RAINY_COMPONENT_JSON_HPP
#include <map>
#include <ios>
#include <memory>
#include <cassert>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <rainy/component/willow/utils.hpp>
#include <rainy/component/willow/json_impl.hpp>
#include <rainy/component/willow/basic_json.hpp>

namespace rainy::component::willow {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_basic_json_v = false;

    template <template <class Key, class Ty, class... Args> class ObjectType, template <class Key, class... Args> class ArrayType,
              typename StringType, typename IntegerType, typename FloatingType, typename BooleanType, template <class Ty> class Alloc>
    RAINY_CONSTEXPR_BOOL
        is_basic_json_v<basic_json<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>> = true;

    template <typename Ty>
    struct is_basic_json : type_traits::helper::bool_constant<is_basic_json_v<Ty>> {};

    template <typename Ty, typename BasicJson = basic_json<>, type_traits::other_trans::enable_if_t<is_basic_json_v<BasicJson>, int> = 0>
    struct json_bind;

    template <typename Ty, typename BasicJson = basic_json<>,
               type_traits::other_trans::enable_if_t<is_basic_json_v<BasicJson>, int> = 0>
    inline rain_fn to_json(BasicJson &json, const Ty &value) -> void {
        json_bind<Ty, BasicJson>{}.to_json(json, value);
    }

    template <typename Ty, typename Json = basic_json<>,
              type_traits::other_trans::enable_if_t<is_basic_json_v<Json> && std::is_default_constructible<json_bind<Ty, Json>>::value,
                                                    int> = 0>
    inline rain_fn from_json(const Json &json, Ty &value) -> void {
        json_bind<Ty, Json>{}.from_json(json, value);
    }

    template <typename Ty>
    struct json_bind<std::unique_ptr<Ty>> {
        void to_json(json &json, std::unique_ptr<Ty> const &value) {
            if (value != nullptr) {
                to_json(json, *value);
            } else {
                json = nullptr;
            }
        }

        void from_json(const json &json, std::unique_ptr<Ty> &value) {
            if (json.is_null()) {
                value = nullptr;
            } else {
                if (!value) {
                    value.reset(new Ty);
                }
                from_json(json, *value);
            }
        }
    };

    template <typename Ty>
    struct json_bind<std::shared_ptr<Ty>> {
        void to_json(json &json, std::shared_ptr<Ty> &value) {
            if (value) {
                to_json(json, *value);
            } else {
                json = nullptr;
            }
        }

        void from_json(const json &json, std::shared_ptr<Ty> &value) {
            if (json.is_null()) {
                value = nullptr;
            } else {
                if (!value) {
                    value = std::make_shared<Ty>();
                }
                from_json(json, *value);
            }
        }
    };

    template <typename Ty>
    struct json_bind<std::vector<Ty>> {
        void to_json(json &json, const std::vector<Ty> &value) {
            json = json_type::array;
            for (std::size_t i = 0; i < value.size(); i++) {
                to_json(json[i], value[i]);
            }
        }

        template <typename type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty>, int> = 0>
        void from_json(const json &json, std::vector<Ty> &value) {
            value.resize(json.size());
            for (std::size_t i = 0; i < json.size(); i++) {
                from_json(json[i], value[i]);
            }
        }
    };

    template <typename Ty>
    struct json_bind<std::map<std::string, Ty>> {
        void to_json(json &json, const std::map<std::string, Ty> &value) {
            json = json_type::object;
            for (const auto &p: value) {
                to_json(json[p.first], p.second);
            }
        }

        template <typename type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty>, int> = 0>
        void from_json(const json &json, std::map<std::string, Ty> &value) {
            for (auto iter = json.cbegin(); iter != json.cend(); iter++) {
                Ty item{};
                from_json(iter->value(), item);
                value.insert(std::make_pair(iter->key(), item));
            }
        }
    };

    template <>
    struct json_bind<int> {
        using value_type = int;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<unsigned int> {
        using value_type = unsigned int;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<short> {
        using value_type = short;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<unsigned short> {
        using value_type = unsigned short;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    
    template <>
    struct json_bind<long> {
        using value_type = long;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<unsigned long> {
        using value_type = unsigned long;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<float> {
        using value_type = float;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<double> {
        using value_type = double;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<bool> {
        using value_type = bool;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = static_cast<value_type>(json);
        }
    };

    template <>
    struct json_bind<json> {
        using value_type = json;

        void to_json(json &json, const value_type &value) {
            json = value;
        }
        void from_json(const json &json, value_type &value) {
            value = json;
        }
    };

    template <typename Ty, typename BasicJson = basic_json<>>
    class read_json_wrapper {
    public:
        using char_type = typename BasicJson::char_type;

        read_json_wrapper(const Ty &value) : v_(value) {
        }

        friend std::basic_ostream<char_type> &operator<<(std::basic_ostream<char_type> &out, const read_json_wrapper &wrapper) {
            BasicJson json{};
            to_json(json, wrapper.v_);
            out << json;
            return out;
        }

    private:
        const Ty &v_;
    };

    template <typename Ty, typename BasicJson = basic_json<>>
    class write_json_wrapper : public read_json_wrapper<Ty, BasicJson> {
    public:
        using char_type = typename BasicJson::char_type;

        write_json_wrapper(Ty &value) : read_json_wrapper<Ty, BasicJson>(value), v_(value) {
        }

        friend std::basic_istream<char_type> &operator>>(std::basic_istream<char_type> &in, const write_json_wrapper &wrapper) {
            BasicJson json{};
            in >> json;
            from_json(json, const_cast<Ty &>(wrapper.v_));
            return in;
        }

    private:
        Ty &v_;
    };

    template <typename Ty, typename BasicJson = basic_json<>, typename std::enable_if<is_basic_json_v<BasicJson>, int>::type = 0,
              typename std::enable_if<std::is_default_constructible<json_bind<Ty, BasicJson>>::value, int>::type = 0>
    inline write_json_wrapper<Ty, BasicJson> json_wrap(Ty &value) {
        return write_json_wrapper<Ty, BasicJson>(value);
    }

    template <typename Ty, typename BasicJson = basic_json<>,
              typename std::enable_if<is_basic_json<BasicJson>::value, int>::type = 0,
              typename std::enable_if<std::is_default_constructible<json_bind<Ty, BasicJson>>::value, int>::type = 0>
    inline read_json_wrapper<Ty, BasicJson> json_wrap(const Ty &value) {
        return read_json_wrapper<Ty, BasicJson>(value);
    }

    RAINY_INLINE json operator""_json(const char *str, std::size_t len) {
        std::string_view view{str, len};
        return json::parse(str);
    }

    RAINY_INLINE json64 operator""_json64(const char *str, std::size_t len) {
        std::string_view view{str, len};
        return json64::parse(str);
    }

    RAINY_INLINE wjson operator""_wjson(const wchar_t *str, std::size_t len) {
        std::wstring_view view{str, len};
        return wjson::parse(str);
    }

    RAINY_INLINE wjson64 operator""_wjson64(const wchar_t *str, std::size_t len) {
        std::wstring_view view{str, len};
        return wjson64::parse(str);
    }
}

#if RAINY_HAS_CXX20
namespace std {
    template <>
    struct formatter<rainy::component::willow::json> {
    public:
        using json = rainy::component::willow::json;

        explicit formatter() noexcept {
        }

        auto parse(format_parse_context &ctx) {
            if (ctx.begin() == ctx.end() || *ctx.begin() == '}') {
                need_indent = true;
                return ctx.end();
            }
            switch (*ctx.begin()) {
                case 'r':
                    need_indent = false;
                    break;
                case 'f':
                    need_indent = true;
                    break;
                case 't':
                    use_tab = true;
                    break;
                default:
                    throw std::format_error("Invalid format specification");
            }
            return ctx.begin() + 1;
        }

        template <typename OutputIt>
        auto format(const json &value, std::basic_format_context<OutputIt, char> fc) const noexcept {
            if (use_tab)
                return std::format_to(fc.out(), "{}", value.dump('\t', 1));
            return std::format_to(fc.out(), "{}", need_indent ? value.dump(4, ' ') : value.dump());
        }

    private:
        bool need_indent{};
        bool use_tab{};
    };

    template <>
    struct formatter<rainy::component::willow::wjson> {
    public:
        using wjson = rainy::component::willow::wjson;

        explicit formatter() noexcept {
        }

        auto parse(format_parse_context &ctx) {
            if (ctx.begin() == ctx.end() || *ctx.begin() == L'}') {
                need_indent = true;
                return ctx.end();
            }
            switch (*ctx.begin()) {
                case L'r':
                    need_indent = false;
                    break;
                case L'f':
                    need_indent = true;
                    break;
                case L't':
                    use_tab = true;
                    break;
                default:
                    throw std::format_error("Invalid format specification");
            }
            return ctx.begin() + 1;
        }

        template <typename OutputIt>
        auto format(const wjson &value, std::basic_format_context<OutputIt, wchar_t> fc) const noexcept {
            if (use_tab) {
                return std::format_to(fc.out(), L"{}", value.dump(L'\t', 1));
            }
            return std::format_to(fc.out(), L"{}", need_indent ? value.dump(4, ' ') : value.dump());
        }

    private:
        bool need_indent{};
        bool use_tab{};
    };
}
#endif

#endif