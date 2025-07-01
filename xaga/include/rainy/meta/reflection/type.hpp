#ifndef RAINY_META_REFLECTION_TYPE_HPP
#define RAINY_META_REFLECTION_TYPE_HPP
#include <rainy/collections/string.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/meta_method.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/property.hpp>
#include <unordered_map>

namespace rainy::meta::reflection::implements {
    using method_storage_t = std::unordered_multimap<std::string_view, method>;
    using property_storage_t = std::unordered_map<std::string_view, property>;
    using ctor_storage_t = std::unordered_multimap<std::string_view, method>;
}

namespace rainy::meta::reflection::implements {
    template <typename TypeList>
    struct template_argument_generater {};

    template <typename... Types>
    struct template_argument_generater<type_traits::other_trans::type_list<Types...>> {
        static collections::views::array_view<foundation::ctti::typeinfo> get() noexcept {
            static collections::array<foundation::ctti::typeinfo, sizeof...(Types)> list = {
                foundation::ctti::typeinfo::create<Types>()...};
            return list;
        }
    };

    class dyn_typeinfo {
    public:
        template <typename Ty>
        static dyn_typeinfo create() noexcept {
            dyn_typeinfo info;
            if constexpr (type_traits::type_relations::is_void_v<Ty>) {
                info.sizeof_ = 0;
            } else {
                info.sizeof_ = sizeof(Ty);
            }
            if constexpr (type_traits::primary_types::template_traits<type_traits::cv_modify::remove_cvref_t<Ty>>::value) {
                info.template_arguemnts_ = template_argument_generater<
                    typename type_traits::primary_types::template_traits<type_traits::cv_modify::remove_cvref_t<Ty>>::type>::get();
            }
            info.type_ = foundation::ctti::typeinfo::create<Ty>();
            return info;
        }

        RAINY_NODISCARD std::size_t get_sizeof() const noexcept {
            return sizeof_;
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo& typeinfo() const noexcept {
            return type_;
        }

        RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> template_arguemnts() const noexcept {
            return template_arguemnts_;
        }

    private:
        dyn_typeinfo() noexcept = default;

        collections::views::array_view<foundation::ctti::typeinfo> template_arguemnts_;
        foundation::ctti::typeinfo type_;
        std::size_t sizeof_{0};
    };
}

namespace rainy::meta::reflection::implements {
    struct type_accessor {
        virtual ~type_accessor() = default;
        /* 类型名称标记 */
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        /* 类型信息 */
        virtual dyn_typeinfo &type() noexcept = 0;
        /* 方法、静态方法、重载运算符 */
        virtual method_storage_t &methods() noexcept = 0;
        /* 构造函数集合 */
        virtual ctor_storage_t &ctors() noexcept = 0;
        /* 属性、静态属性 */
        virtual std::unordered_map<std::string_view, property> &properties() noexcept = 0;
        /* 基类集合 */
        virtual std::unordered_map<std::string_view, type_accessor *> &bases() noexcept = 0;
        /* 派生类集合，用于未来设计dynamic_cast */
        virtual std::unordered_map<std::string_view, type_accessor *> &deriveds() noexcept = 0;
        // 基类/派生类返回的是一张表，其中的指针指向位于反射系统内部的实例（其实是静态示例）
        // const在此部分非一等公民，由面向用户的接口进行const属性添加
        // 不考虑过多性能
    };

    template <typename Type>
    class type_accessor_impl_class final : public type_accessor {
    public:
        explicit type_accessor_impl_class(const std::string_view name) noexcept : name_(name), typeinfo_(dyn_typeinfo::create<Type>()) {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        dyn_typeinfo &type() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            return methods_;
        }

        ctor_storage_t &ctors() noexcept override {
            return ctors_;
        }

        std::unordered_map<std::string_view, property> &properties() noexcept override {
            return properties_;
        }

        std::unordered_map<std::string_view, type_accessor *> &bases() noexcept override {
            return bases_;
        }

        std::unordered_map<std::string_view, type_accessor *> &deriveds() noexcept override {
            return deriveds_;
        }

    private:
        std::string_view name_;
        dyn_typeinfo typeinfo_;
        method_storage_t methods_;
        ctor_storage_t ctors_;
        std::unordered_map<std::string_view, property> properties_;
        std::unordered_map<std::string_view, type_accessor *> bases_;
        std::unordered_map<std::string_view, type_accessor *> deriveds_;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename... Args>
    constexpr auto make_ctor_name() {
        using namespace foundation::ctti;
        constexpr auto type_str = type_name<Type>();
        if constexpr (sizeof...(Args) == 0) {
            constexpr std::size_t total_len = type_str.size() + 2 + 1; // "Type()" + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            for (char c: type_str) {
                result[pos++] = c;
            }
            result[pos++] = '(';
            result[pos++] = ')';
            result[pos++] = '\0';

            return result;
        } else {
            constexpr collections::array<std::string_view, sizeof...(Args)> arg_names = {type_name<Args>()...};
            constexpr std::size_t args_len = [&arg_names] {
                std::size_t len = 0;
                for (auto &arg: arg_names) {
                    len += arg.size();
                }
                len += 2 * (sizeof...(Args) - 1); // ", "
                return len;
            }();
            constexpr std::size_t total_len = type_str.size() + 1 + args_len + 1 + 1; // '(' + args + ')' + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            auto append = [&](const std::string_view &s) {
                for (char c: s) {
                    result[pos++] = c;
                }
            };
            append(type_str);
            result[pos++] = '(';
            for (std::size_t i = 0; i < arg_names.size(); ++i) {
                append(arg_names[i]);
                if (i < arg_names.size() - 1) {
                    result[pos++] = ',';
                    result[pos++] = ' ';
                }
            }
            result[pos++] = ')';
            result[pos++] = '\0';
            return result;
        }
    }

    RAINY_INLINE std::size_t eval_hash_from_paramlist(collections::views::array_view<utility::any> view) {
        return core::accumulate(view.begin(), view.end(), std::size_t{0},
                                [right = std::size_t{1}](const std::size_t acc, const utility::any &arg) mutable {
                                    return acc + (arg.type().hash_code() * right++);
                                });
    }
}

#endif