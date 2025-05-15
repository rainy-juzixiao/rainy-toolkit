#ifndef RAINY_META_REFLECTION_TYPE_HPP
#define RAINY_META_REFLECTION_TYPE_HPP
#include <rainy/foundation/system/memory/nebula_ptr.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/field.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/meta_method.hpp>
#include <rainy/collections/string.hpp>
#include <sstream>
#include <numeric> 
#include <unordered_map>

namespace rainy::meta::reflection {
    struct type;
    class enumeration;
    class union_t;
    class class_t;
    class shared_object;
}

namespace rainy::meta::reflection::implements {
    static const std::unordered_map<std::string_view, foundation::system::memory::nebula_ptr<type>> empty_types;
    static const std::unordered_map<std::string_view, field> empty_fields;
    static const std::unordered_map<std::string_view, function> empty_methods;
    static const field empty_field;
    static const function empty_method;

    template <typename Type, typename... Args>
    constexpr auto make_ctor_name() {
        using namespace foundation::rtti;
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
            constexpr std::size_t args_len = [] {
                std::size_t len = 0;
                for (auto &arg: arg_names)
                    len += arg.size();
                len += 2 * (sizeof...(Args) - 1); // ", "
                return len;
            }();
            constexpr std::size_t total_len = type_str.size() + 1 + args_len + 1 + 1; // '(' + args + ')' + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            auto append = [&](std::string_view s) {
                for (char c: s)
                    result[pos++] = c;
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

    std::size_t eval_hash_from_paramlist(collections::views::array_view<utility::any> view) {
        return core::accumulate(view.begin(), view.end(), std::size_t{0},
                               [](std::size_t acc, const utility::any &arg) { return acc + arg.type().hash_code(); });
    }
}

namespace rainy::meta::reflection {
    enum class basic_type {
        enumeration,
        class_t,
        interface_t,
        union_t
    };

    struct RAINY_TOOLKIT_API type {
        virtual ~type() = default;
        virtual std::string_view name() const noexcept = 0;
        virtual const foundation::rtti::typeinfo &get_typeinfo() const noexcept = 0;
        virtual bool is_type(basic_type type) const noexcept = 0;
        virtual std::size_t fields_count() const noexcept = 0;
        virtual std::size_t methods_count() const noexcept = 0;
        virtual const field &get_field(std::string_view name) const noexcept = 0;
        virtual const function &get_method(std::string_view name) const noexcept = 0;
        virtual const std::unordered_map<std::string_view, field> &get_fields() const noexcept = 0;
        virtual const std::unordered_map<std::string_view, function> &get_methods() const noexcept = 0;
        virtual utility::any create(rainy::collections::views::array_view<utility::any> paramlist = {}) const noexcept = 0;
        enumeration *as_enumeration() noexcept;
        const enumeration *as_enumeration() const noexcept;
        union_t *as_union() noexcept;
        const union_t *as_union() const noexcept;
    };
}

namespace rainy::meta::reflection {
    class enumeration : public type {
    public:
        /* 下面的成员函数，默认进行final处理，为枚举类进行特化 */
        bool is_type(basic_type type) const noexcept {
            return type == basic_type::enumeration;
        }
        const std::unordered_map<std::string_view, function> &get_methods() const noexcept final { // NOLINT
            return implements::empty_methods;
        }

        const function &get_method(std::string_view) const noexcept final { // NOLINT
            return implements::empty_method;
        }

        std::size_t methods_count() const noexcept final { // NOLINT
            return 0;
        }

        virtual const foundation::rtti::typeinfo &underlying_type() const noexcept = 0;
        virtual field &get_field(std::string_view name) noexcept = 0;

        template <typename Type, type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Type, field>>>
        enumeration &add_field(std::string_view name, Type enums) {
            static_assert(type_traits::primary_types::is_enum_v<Type>, "Type must be a enumeration type!");
            utility::expects(rainy_typeid(Type) == this->get_typeinfo(),
                             "Type must be a enumeration type and be same with target type!");
            add_field_impl(name, field{enums});
            return *this;
        }

        enumeration& add_field(std::string_view name, field enum_field) {
            utility::expects(enum_field.is_enum() && enum_field.field_rtti_type() == this->get_typeinfo(),
                             "Type must be a enumeration type and be same with target type!");
            add_field_impl(name, enum_field);
            return *this;
        }

    private:
        virtual void add_field_impl(std::string_view name, field enum_field) = 0;
        virtual void remove_field_impl(std::string_view name, field enum_field) = 0;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type>
    class enumeration_impl : public enumeration {
    public:
        static_assert(type_traits::primary_types::is_enum_v<Type>, "Type must be a enumeration type!");

        static constexpr std::string_view type_name = foundation::rtti::type_name<Type>();
        static constexpr foundation::rtti::typeinfo type_info = foundation::rtti::typeinfo::create<Type>();

        std::string_view name() const noexcept override {
            return type_name;
        }

        const foundation::rtti::typeinfo &get_typeinfo() const noexcept override {
            return type_info;
        }

        const field &get_field(std::string_view name) const noexcept override {
            return fields.at(name);
        }

        const std::unordered_map<std::string_view, field> &get_fields() const noexcept override {
            return fields;
        }

        const foundation::rtti::typeinfo &underlying_type() const noexcept override {
            return rainy_typeid(std::underlying_type_t<Type>);
        }

        std::size_t fields_count() const noexcept override {
            return fields.size();
        }

        field &get_field(std::string_view name) noexcept override {
            return fields.at(name);
        }

        utility::any create(rainy::collections::views::array_view<utility::any> paramlist) const noexcept override {
            if (!paramlist.empty()) {
                auto &param_init = paramlist[0];
                if (param_init.type() == rainy_typeid(Type)) {
                    return Type{param_init.as<Type>()};
                }
            }
            return Type{};
        }

    private:
        void add_field_impl(std::string_view name, field enum_field) override {
            if (enum_field.empty() || name.empty()) {
                return;
            }
            (void) fields.try_emplace(name, enum_field);
        }

        void remove_field_impl(std::string_view name, field enum_field) override {
            if (enum_field.empty() || name.empty()) {
                return;
            }
            fields.erase(name);
        }

        std::unordered_map<std::string_view, field> fields{};
    };
}

namespace rainy::meta::reflection {
    class union_t : public type {
    public:
        bool is_type(basic_type type) const noexcept override {
            return type == basic_type::union_t;
        }

        virtual const std::unordered_map<std::string_view, field> &get_fields() const noexcept override = 0;
        virtual const std::unordered_map<std::string_view, function> &get_methods() const noexcept override = 0;
        virtual std::size_t fields_count() const noexcept override = 0;
        virtual std::size_t methods_count() const noexcept override = 0;
        virtual const function &get_method(std::string_view name) const noexcept override = 0;
        virtual const field &get_field(std::string_view name) const noexcept override = 0;

        template <typename Field,
                  type_traits::other_trans::enable_if_t<type_traits::primary_types::is_member_object_pointer_v<Field>, int> = 0>
        union_t &add_field(std::string_view name, Field field) {
            add_field_impl(name, field);
            return *this;
        }

        union_t &add_field(std::string_view name, field field) {
            add_field_impl(name, field);
            return *this;
        }

    protected:
        virtual void add_field_impl(std::string_view name, const field &field) = 0;
        virtual void add_method_impl(std::string_view name, const function &func) = 0;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type>
    class union_impl : public union_t {
    public:
        static_assert(type_traits::primary_types::is_union_v<Type>, "Type must be a enumeration type!");

        static constexpr std::string_view type_name = foundation::rtti::type_name<Type>();
        static constexpr foundation::rtti::typeinfo type_info = foundation::rtti::typeinfo::create<Type>();

        std::string_view name() const noexcept override {
            return type_name;
        }

        const foundation::rtti::typeinfo &get_typeinfo() const noexcept override {
            return type_info;
        }

        const std::unordered_map<std::string_view, field> &get_fields() const noexcept override {
            return fields;
        }

        const std::unordered_map<std::string_view, function> &get_methods() const noexcept override {
            return methods;
        }

        const field &get_field(std::string_view name) const noexcept override {
            return fields.at(name);
        }

        const function &get_method(std::string_view name) const noexcept override {
            return methods.at(name);
        }

        std::size_t fields_count() const noexcept override {
            return fields.size();
        }

        std::size_t methods_count() const noexcept override {
            return methods.size();
        }

        utility::any create(collections::views::array_view<utility::any> paramlist = {}) const noexcept override {
            if constexpr (type_traits::type_properties::is_default_constructible_v<Type>) {
                if (paramlist.empty()) {
                    return Type{};
                }
            }
            auto ctor = std::find_if(ctors.begin(), ctors.end(), [&paramlist](const auto &ctor) {
                const function &fn = ctor.second;
                return fn.is_invocable();
            });
            return ctor->second.invoke_variadic(non_exists_instance, paramlist);
        }

        template <typename... Args>
        union_impl &add_ctor() {
            static constexpr auto ctor_name = implements::make_ctor_name<Type, Args...>();
            add_ctor_impl({ctor_name.data(), ctor_name.size()}, method::get_ctor_fn<Type, Args...>());
            return *this;
        }

    protected:
        void add_field_impl(std::string_view name, const field &field) override {
            if (field.empty() || name.empty()) {
                return;
            }
            (void) fields.try_emplace(name, field);
        }

        void add_method_impl(std::string_view name, const function &func) override {
            if (func.empty() || name.empty()) {
                return;
            }
            (void) methods.try_emplace(name, func);
        }

        void add_ctor_impl(std::string_view name, const function &func) {
            if (func.empty() || name.empty()) {
                return;
            }
            (void) ctors.try_emplace(name, func);
        }

        std::unordered_map<std::string_view, field> fields{};
        std::unordered_map<std::string_view, function> methods{};
        std::unordered_map<std::string_view, function> ctors{};
        function dtor;
    };
}

namespace rainy::meta::reflection {
    class interface_t : public union_t {
    public:
        using type_collection = std::unordered_map<std::string_view, foundation::system::memory::nebula_ptr<type>>;

        bool is_type(basic_type type) const noexcept override {
            return type == basic_type::interface_t;
        }

        virtual const type_collection &get_bases() const noexcept {
            return implements::empty_types;
        }

        virtual const type_collection &get_dervieds() const noexcept = 0;
    };

    class class_t : public interface_t {
    public:
        bool is_type(basic_type type) const noexcept override {
            return type == basic_type::class_t;
        }

        virtual const std::unordered_map<std::string_view, foundation::system::memory::nebula_ptr<type>> &get_bases()
            const noexcept override = 0;
        virtual const std::unordered_map<std::string_view, foundation::system::memory::nebula_ptr<type>> &get_dervieds()
            const noexcept override = 0;

    private:
        virtual class_t &add_base_impl(std::string_view name, type *base) = 0;
    };
}

namespace rainy::meta::reflection {
    class type_register {
    public:
        type_register(const type_register &) = delete;
        type_register(type_register &&) = delete;
        type_register &operator=(const type_register &) = delete;
        type_register &operator=(type_register &&) = delete;

        template <typename Type>
        static bool register_type() {
            using namespace foundation::rtti;
            auto &this_ = instance();
            constexpr core::let<typeinfo> rtti = typeinfo::create<Type>();
            if constexpr (rtti->has_traits(traits::is_enum)) {
                static implements::enumeration_impl<Type> typedata;
                auto pair = this_.storage.try_emplace(rtti->hash_code(), &typedata);
                return pair.second;
            } else if constexpr (rtti->has_traits(traits::is_union)) {
                static implements::union_impl<Type> typedata;
                auto pair = this_.storage.try_emplace(rtti->hash_code(), &typedata);
                return pair.second;
            } 
            return false;
        }

        template <typename Type>
        static type &get() {
            return get(foundation::rtti::typeinfo::create<Type>());
        }

        static type &get(const foundation::rtti::typeinfo& rtti) {
            using namespace foundation::rtti;
            auto &this_ = instance();
            auto iter = this_.storage.find(rtti.hash_code());
            if (iter != this_.storage.end()) {
                return iter->second.as_reference();
            }
            std::terminate();
        }

        template <typename Type>
        static type_register &unregister() {
            using namespace foundation::rtti;
            auto &this_ = instance();
            constexpr core::let<foundation::rtti::typeinfo> rtti = foundation::rtti::typeinfo::create<Type>();
            auto iter = this_.storage.find(rtti->hash_code());
            if (iter != this_.storage.end()) {
                this_.storage.erase(iter);
            }
            return instance();
        }

    private:
        type_register() = default;

        static type_register &instance() {
            static type_register instance;
            return instance;
        }

        using smart_pointer_t = foundation::system::memory::nebula_ptr<type, foundation::system::memory::no_delete<type>>;
        // 因为我们引用的是静态对象，智能指针只是方便我们明确所有权

        std::unordered_map<std::size_t, smart_pointer_t> storage{};
    };
}

namespace rainy::meta::reflection {
    class shared_object {
    public:

    private:
        struct object_accessor {
            virtual utility::any invoke(std::string_view method_name,rainy::collections::views::array_view<utility::any> paramlist) const = 0;
        };

        template <typename Type>
        class object_accessor_impl : public object_accessor {
        public:
            object_accessor_impl(rainy::collections::views::array_view<utility::any> paramlist) {
                
            }

            utility::any invoke(std::string_view method_name, rainy::collections::views::array_view<utility::any> paramlist) const override {
                using namespace core;
                type* type = get();
                if (type->is_type(basic_type::enumeration)) {
                    auto enumeration = type->as_enumeration();
                    field& f = enumeration->get_field(method_name);
                    if (!paramlist.empty()) {
                        // 具有设置语义
                        f.set_value(object, paramlist[0]);
                        return object;
                    } else {
                        return f.get_value();
                    }
                }
                return {};
            }

        private:
            type *get() const noexcept {
                return type_register::get<Type>();
            }

            union {
                Type object;
                std::max_align_t dummy;
            };
        };

        std::shared_ptr<object_accessor> object_ptr_;
        foundation::rtti::typeinfo typeinfo_;
    };
}

#endif