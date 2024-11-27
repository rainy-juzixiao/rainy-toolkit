#ifndef RAINY_REFLECTION_CORE_TYPE_HPP
#define RAINY_REFLECTION_CORE_TYPE_HPP
#include <rainy/internals/reflection_core/field.hpp>
#include <rainy/internals/reflection_core/method.hpp>
#include <rainy/system/nebula_ptr.hpp>

namespace rainy::foundation::reflection::internals {
    enum class type {
        enum_type,
        class_type,
        union_type,
        interface_type
    };

    enum class data_type {
        field,
        method,
        dynamic_type,
        enum_field,
        unknown
    };

    struct data_collection {
        data_type type;
        void *resouces;
    };

    struct dynamic_type {
        dynamic_type() = default;
        virtual ~dynamic_type() = default;
        dynamic_type(const dynamic_type &) = default;
        dynamic_type(dynamic_type &&) = default;
        dynamic_type &operator=(const dynamic_type &) = default;
        dynamic_type &operator=(dynamic_type &&) = default;

        RAINY_NODISCARD virtual const utility::dynamic_type_info &info() const noexcept = 0;
        RAINY_NODISCARD virtual type get_type() const noexcept = 0;
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        RAINY_NODISCARD virtual data_collection get_data(std::string_view name) const noexcept = 0;
        RAINY_NODISCARD virtual std::size_t hash_code() const noexcept = 0;
    };

    template <typename Ty>
    class enum_type final : public dynamic_type {
    public:
        enum_type() : _info(utility::type_info::create<Ty>()) {
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept override {
            return _info;
        }

        RAINY_NODISCARD type get_type() const noexcept override {
            return type::enum_type;
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return _info.name();
        }

        RAINY_NODISCARD data_collection get_data(const std::string_view name) const noexcept override {
            if (name == "field") {
                return {data_type::enum_field, static_cast<void *>(&field)};
            }
            return {data_type::unknown, nullptr};
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return _info.hash_code();
        }

    private:
        utility::type_info _info{};
        std::vector<enum_field> field;
    };

    template <typename Ty>
    class union_type final : public dynamic_type {
    public:
        union_type() : _info(utility::type_info::create<Ty>()) {
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept override {
            return _info;
        }

        RAINY_NODISCARD type get_type() const noexcept override {
            return type::union_type;
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return _info.name();
        }

        RAINY_NODISCARD data_collection get_data(const std::string_view name) const noexcept override {
            if (name == "field") {
                return {data_type::field, static_cast<void *>(&_field)};
            } else if (name == "method") {
                return {data_type::method, static_cast<void *>(&_field)};
            }
            return {data_type::unknown, nullptr};
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return _info.hash_code();
        }

    private:
        utility::type_info _info;
        std::vector<field> _field;
        std::vector<method> _method;
    };

    template <typename Ty>
    class base_type : public dynamic_type {
    public:
        base_type(const type type_kind) : _info(utility::type_info::create<Ty>()), _type_kind(type_kind) {
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept override {
            return _info;
        }

        RAINY_NODISCARD type get_type() const noexcept override {
            return _type_kind;
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return _info.name();
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return _info.hash_code();
        }

    protected:
        utility::type_info _info;
        type _type_kind;
        std::vector<field> _field;
        std::vector<method> _method;
        std::vector<dynamic_type *> derived;
    };

    template <typename Ty>
    class class_type final : public base_type<Ty> {
    public:
        class_type() : base_type<Ty>(type::class_type) {
        }

        data_collection get_data(const std::string_view name) const noexcept override {
            if (name == "field") {
                return {data_type::field, static_cast<void *>(&this->_field)};
            }
            if (name == "method") {
                return {data_type::method, static_cast<void *>(&this->_method)};
            }
            if (name == "base") {
                return {data_type::dynamic_type, static_cast<void *>(&base)};
            }
            if (name == "derived") {
                return {data_type::dynamic_type, static_cast<void *>(&this->derived)};
            }
            return {data_type::unknown, nullptr};
        }

    private:
        std::vector<dynamic_type *> base;
    };

    template <typename Ty>
    class interface_type final : public base_type<Ty> {
    public:
        interface_type() : base_type<Ty>(type::interface_type) {
        }

        data_collection get_data(const std::string_view name) const override {
            if (name == "field") {
                return {data_type::field, static_cast<void *>(&this->_field)};
            }
            if (name == "method") {
                return {data_type::method, static_cast<void *>(&this->_method)};
            }
            if (name == "derived") {
                return {data_type::dynamic_type, static_cast<void *>(&this->derived)};
            }
            return {data_type::unknown, nullptr};
        }
    };

    template <typename EnumType, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<EnumType>, int> = 0>
    auto make_enum_type() {
        return system::memory::make_nebula<dynamic_type<EnumType>>();
    }

    template <typename UnionType, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_union_v<UnionType>, int> = 0>
    auto make_union_type() {
        return system::memory::make_nebula<dynamic_type<UnionType>>();
    }

    template <typename ClassType, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_class_v<ClassType>, int> = 0>
    auto make_class_type() {
        return system::memory::make_nebula<dynamic_type<ClassType>>();
    }

    template <typename InterfaceType,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_abstract_v<InterfaceType>, int> = 0>
    auto make_interface_type() {
        return system::memory::make_nebula<dynamic_type<ClassType>>();
    }

    class const_abstract_type {
    public:
        template <typename Class>
        const_abstract_type() {
        
        }

        const std::vector<field> &try_get_field() const {
            utility::expects(!data.empty());
            data_collection collection = data->get_data("field");
            utility::ensures(collection.type == data_type::field);
            rainy_let get = static_cast<std::vector<field> *>(collection.resouces);
            utility::ensures(static_cast<bool>(get));
            return *get;
        }

        const std::vector<method> &try_get_method() const {
            utility::expects(!data.empty());
            data_collection collection = data->get_data("method");
            utility::ensures(collection.type == data_type::field);
            rainy_let get = static_cast<std::vector<method> *>(collection.resouces);
            utility::ensures(static_cast<bool>(get));
            return *get;
        }

    private:
        system::memory::nebula_ptr<dynamic_type> data;
    };
}

#endif