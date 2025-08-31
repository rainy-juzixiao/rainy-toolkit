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
    
    // namespace traits {
    //     template <typename>
    //     struct is_basic_json : std::false_type {};

    //    template <template <class Key, class Ty, class... Args> class ObjectType, template <class Key, class... Args> class
    //    ArrayType,
    //              typename StringType, typename IntegerType, typename FloatingType, typename BooleanType,
    //              template <class Ty> class Alloc>
    //    struct is_basic_json<basic_json<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>
    //        : std::true_type {};

    //    template <typename Ty, typename basicJson = basic_json<>,
    //              typename std::enable_if<is_basic_json<basicJson>::value, int>::type = 0>
    //    struct json_bind;

    //    template <typename Ty, typename basicJson = basic_json<>,
    //              typename std::enable_if<is_basic_json<basicJson>::value, int>::type = 0,
    //              typename std::enable_if<std::is_default_constructible<json_bind<Ty, basicJson>>::value, int>::type = 0>
    //    inline void to_json(basicJson &j, const Ty &value) {
    //        json_bind<Ty, basicJson>().to_json(j, value);
    //    }

    //    template <typename Ty, typename basicJson = basic_json<>,
    //              typename std::enable_if<is_basic_json<basicJson>::value, int>::type = 0,
    //              typename std::enable_if<std::is_default_constructible<json_bind<Ty, basicJson>>::value, int>::type = 0>
    //    void from_json(const basicJson &j, Ty &value) {
    //        json_bind<Ty, basicJson>().from_json(j, value);
    //    }

    //    template <typename Ty>
    //    struct json_bind<std::unique_ptr<Ty>> {
    //        void to_json(json &j, std::unique_ptr<Ty> const &v) {
    //            if (v != nullptr) {
    //                traits::to_json(j, *v);
    //            } else {
    //                j = nullptr;
    //            }
    //        }

    //        void from_json(const json &j, std::unique_ptr<Ty> &v) {
    //            if (j.is_null()) {
    //                v = nullptr;
    //            } else {
    //                if (v == nullptr) {
    //                    v.reset(new Ty);
    //                }
    //                traits::from_json(j, *v);
    //            }
    //        }
    //    };

    //    template <typename Ty>
    //    struct json_bind<std::shared_ptr<Ty>> {
    //        void to_json(json &j, std::shared_ptr<Ty> const &v) {
    //            if (v != nullptr) {
    //                traits::to_json(j, *v);
    //            } else {
    //                j = nullptr;
    //            }
    //        }

    //        void from_json(const json &j, std::shared_ptr<Ty> &v) {
    //            if (j.is_null()) {
    //                v = nullptr;
    //            } else {
    //                if (v == nullptr) {
    //                    v = std::make_shared<Ty>();
    //                }
    //                traits::from_json(j, *v);
    //            }
    //        }
    //    };

    //    template <typename Ty>
    //    struct json_bind<std::vector<Ty>> {
    //        void to_json(json &j, const std::vector<Ty> &v) {
    //            j = json_type::array;
    //            for (std::size_t i = 0; i < v.size(); i++) {
    //                traits::to_json(j[i], v[i]);
    //            }
    //        }

    //        template <typename std::enable_if<std::is_default_constructible<Ty>::value, int>::type = 0>
    //        void from_json(const json &j, std::vector<Ty> &v) {
    //            v.resize(j.size());
    //            for (std::size_t i = 0; i < j.size(); i++) {
    //                traits::from_json(j[i], v[i]);
    //            }
    //        }
    //    };

    //    template <typename Ty>
    //    struct json_bind<std::map<std::string, Ty>> {
    //        void to_json(json &j, const std::map<std::string, Ty> &v) {
    //            j = json_type::object;
    //            for (const auto &p: v) {
    //                traits::to_json(j[p.first], p.second);
    //            }
    //        }

    //        template <typename std::enable_if<std::is_default_constructible<Ty>::value, int>::type = 0>
    //        void from_json(const json &j, std::map<std::string, Ty> &v) {
    //            for (auto iter = j.cbegin(); iter != j.cend(); iter++) {
    //                Ty item{};
    //                traits::from_json(iter->value(), item);
    //                v.insert(std::make_pair(iter->key(), item));
    //            }
    //        }
    //    };

    //    template <>
    //    struct json_bind<int> {
    //        using value_type = int;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<unsigned int> {
    //        using value_type = unsigned int;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<short> {
    //        using value_type = short;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<unsigned short> {
    //        using value_type = unsigned short;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<long> {
    //        using value_type = long;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<unsigned long> {
    //        using value_type = unsigned long;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<float> {
    //        using value_type = float;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<double> {
    //        using value_type = double;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<bool> {
    //        using value_type = bool;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = static_cast<value_type>(j);
    //        }
    //    };

    //    template <>
    //    struct json_bind<json> {
    //        using value_type = json;

    //        void to_json(json &j, const value_type &v) {
    //            j = v;
    //        }
    //        void from_json(const json &j, value_type &v) {
    //            v = j;
    //        }
    //    };

    //    template <typename Ty, typename basicJson = basic_json<>>
    //    class read_json_wrapper {
    //    public:
    //        using char_type = typename basicJson::char_type;

    //        read_json_wrapper(const Ty &v) : v_(v) {
    //        }

    //        friend std::basic_ostream<char_type> &operator<<(std::basic_ostream<char_type> &out, const read_json_wrapper &wrapper) {
    //            basicJson j{};
    //            traits::to_json(j, wrapper.v_);
    //            out << j;
    //            return out;
    //        }

    //    private:
    //        const Ty &v_;
    //    };

    //    template <typename Ty, typename basicJson = basic_json<>>
    //    class write_json_wrapper : public read_json_wrapper<Ty, basicJson> {
    //    public:
    //        using char_type = typename basicJson::char_type;

    //        write_json_wrapper(Ty &v) : read_json_wrapper<Ty, basicJson>(v), v_(v) {
    //        }

    //        friend std::basic_istream<char_type> &operator>>(std::basic_istream<char_type> &in, const write_json_wrapper &wrapper) {
    //            basicJson j{};
    //            in >> j;
    //            traits::from_json(j, const_cast<Ty &>(wrapper.v_));
    //            return in;
    //        }

    //    private:
    //        Ty &v_;
    //    };

    //    template <typename Ty, typename basicJson = basic_json<>,
    //              typename std::enable_if<traits::is_basic_json<basicJson>::value, int>::type = 0,
    //              typename std::enable_if<std::is_default_constructible<traits::json_bind<Ty, basicJson>>::value, int>::type = 0>
    //    inline write_json_wrapper<Ty, basicJson> json_wrap(Ty &v) {
    //        return write_json_wrapper<Ty, basicJson>(v);
    //    }

    //    template <typename Ty, typename basicJson = basic_json<>,
    //              typename std::enable_if<traits::is_basic_json<basicJson>::value, int>::type = 0,
    //              typename std::enable_if<std::is_default_constructible<traits::json_bind<Ty, basicJson>>::value, int>::type = 0>
    //    inline read_json_wrapper<Ty, basicJson> json_wrap(const Ty &v) {
    //        return read_json_wrapper<Ty, basicJson>(v);
    //    }
    //}

    json operator""_json(const char *str, std::size_t len) {
        std::string_view view{str, len};
        return json::parse(str);
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