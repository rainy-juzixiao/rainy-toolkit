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
#ifndef RAINY_META_REFLECTION_REFL_IMPL_HPP
#define RAINY_META_REFLECTION_REFL_IMPL_HPP
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::meta::reflection {
    class type;
    class method; // 方法类
    class property; // 属性类
    class enumeration; // 枚举类
    class constructor; // 构造器
    class metadata; // 元数据 
    class fundmental;
}

namespace rainy::meta::reflection::implements {
    struct type_accessor;
    struct enumeration_accessor;
    struct fundmental_type_accessor;

    template <typename Enum>
    struct enumeration_accessor_impl;

    template <typename Type>
    struct fundmental_type_accessor_impl;

    template <typename Type>
    rain_fn new_fundmental_accessor_instance(type_accessor *type) noexcept -> fundmental_type_accessor * {
        static fundmental_type_accessor_impl<Type> instance{type};
        return &instance;
    }

    template <typename EnumType>
    rain_fn new_enum_type_storage_instance(type_accessor *type) noexcept -> enumeration_accessor_impl<EnumType>* {
        static implements::enumeration_accessor_impl<EnumType> enum_type_storage{type};
        return &enum_type_storage;
    }
}

namespace rainy::meta::reflection::implements {
    class RAINY_TOOLKIT_API register_table {
    public:
        static void register_type(std::string_view name, type_accessor *type);

        static type_accessor *get_accessor(annotations::lifetime::in<foundation::ctti::typeinfo> type);

        static void unregister(std::string_view name, foundation::ctti::typeinfo ctti);

        static type_accessor *get_accessor_by_name(std::string_view name);

        static bool has_register(annotations::lifetime::in<foundation::ctti::typeinfo> type) noexcept;

    private:
        class myimpl;

        static register_table &instance();

        myimpl *global_ptr;
    };
}

namespace rainy::meta::reflection::implements {
    class RAINY_TOOLKIT_API injector {
    public:
        static void register_type(std::string_view name, type_accessor *type);

        static void unregister_all();

    private:
        struct registration_entry {
            std::string_view name;
            foundation::ctti::typeinfo ctti;
        };

        class myimpl;

        static injector &instance();

        myimpl *global_ptr;
    };
}

namespace rainy::meta::reflection::implements {
    using method_storage_t = std::unordered_multimap<std::string_view, method>;
    using property_storage_t = std::unordered_map<std::string_view, property>;
    using enumeration_storage_t = std::unordered_map<std::string_view, enumeration>;
    using ctor_storage_t = std::vector<constructor>;
}

namespace rainy::meta::reflection::implements {
    struct type_accessor {
        virtual ~type_accessor() = default;
        /* 类型名称标记 */
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        /* 类型信息 */
        virtual foundation::ctti::typeinfo &typeinfo() noexcept = 0;
        /* 方法、静态方法、重载运算符 */
        virtual method_storage_t &methods() noexcept = 0;
        /* 构造函数集合 */
        virtual ctor_storage_t &ctors() noexcept = 0;
        /* 属性、静态属性 */
        virtual property_storage_t &properties() noexcept = 0;
        /* 基类集合 */
        virtual std::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept = 0;
        /* 派生类集合，用于未来设计dynamic_cast */
        virtual std::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept = 0;
        // 基类/派生类返回的是一张表，其中的指针指向位于反射系统内部的实例（其实是静态示例）
        // const在此部分非一等公民，由面向用户的接口进行const属性添加
        // 不考虑过多性能
        virtual std::vector<metadata> &metadatas() noexcept = 0;
        /* 如果是枚举信息，则此方法有效 */
        virtual enumeration &as_enumeration() noexcept = 0;
        /* 如果是fundmental类型，则此方法有效 */
        virtual fundmental &as_fundmental() noexcept = 0;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type>
    class type_accessor_impl_class final : public type_accessor {
    public:
        explicit type_accessor_impl_class(const std::string_view name) noexcept :
            name_(name), typeinfo_(foundation::ctti::typeinfo::create<Type>()) {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        foundation::ctti::typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            return methods_;
        }

        ctor_storage_t &ctors() noexcept override {
            return ctors_;
        }

        property_storage_t &properties() noexcept override {
            return properties_;
        }

        std::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept override {
            return bases_;
        }

        std::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept override {
            return deriveds_;
        }

        std::vector<metadata> &metadatas() noexcept override {
            return metadatas_;
        }

        enumeration &as_enumeration() noexcept {
            static enumeration empty;
            return empty;
        }

        fundmental &as_fundmental() noexcept {
            static fundmental empty;
            return empty;
        }

    private:
        std::string_view name_;
        foundation::ctti::typeinfo typeinfo_;
        method_storage_t methods_;
        ctor_storage_t ctors_;
        property_storage_t properties_;
        std::unordered_map<foundation::ctti::typeinfo, type> bases_;
        std::unordered_map<foundation::ctti::typeinfo, type> deriveds_;
        std::vector<metadata> metadatas_;
    };

    template <typename Type>
    class type_accessor_impl_enumeration final : public type_accessor {
    public:
        explicit type_accessor_impl_enumeration(const std::string_view name) noexcept :
            name_(name), typeinfo_(foundation::ctti::typeinfo::create<Type>()),
            enumeration{core::internal_construct_tag, this, new_enum_type_storage_instance<Type>(this)} {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        foundation::ctti::typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            static method_storage_t empty;
            return empty;
        }

        ctor_storage_t &ctors() noexcept override {
            static ctor_storage_t ctors_;
            static std::once_flag flag;
            std::call_once(flag, []() {
                static collections::array<metadata, 0> empty{};
                static std::tuple<> a;
                static constexpr auto underlying_ctor_name = make_ctor_name<Type, type_traits::other_trans::underlying_type_t<Type>>();
                static constexpr auto ctor_name = make_ctor_name<Type, Type>();
                static constexpr auto default_ctor_name = make_ctor_name<Type>();
                ctors_.emplace_back(constructor::make(
                    {underlying_ctor_name.data(), underlying_ctor_name.size()},
                    [](type_traits::other_trans::underlying_type_t<Type> value) { return Type{value}; }, a, empty));
                ctors_.emplace_back(
                    constructor::make({ctor_name.data(), ctor_name.size()}, [](Type value) { return Type{value}; }, a, empty));
                ctors_.emplace_back(
                    constructor::make({default_ctor_name.data(), default_ctor_name.size()}, []() { return Type{}; }, a, empty));
            });
            return ctors_;
        }

        property_storage_t &properties() noexcept override {
            static property_storage_t empty;
            return empty;
        }

        std::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept override {
            static std::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        std::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept override {
            static std::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        std::vector<metadata> &metadatas() noexcept override {
            static std::vector<metadata> empty;
            return empty;
        }

        enumeration &as_enumeration() noexcept override {
            return enumeration;
        }

        fundmental &as_fundmental() noexcept {
            static fundmental empty;
            return empty;
        }

    private:
        std::string_view name_;
        foundation::ctti::typeinfo typeinfo_;
        enumeration enumeration;
    };

    template <typename Type>
    class type_accessor_impl_fundmental_type final : public type_accessor {
    public:
        explicit type_accessor_impl_fundmental_type(const std::string_view name) noexcept :
            name_(name), typeinfo_(foundation::ctti::typeinfo::create<Type>()),
            fundmental_type(core::internal_construct_tag, this, new_fundmental_accessor_instance<Type>(this)) {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        foundation::ctti::typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            static method_storage_t empty;
            return empty;
        }

        ctor_storage_t &ctors() noexcept override {
            static ctor_storage_t ctors_;
            static std::once_flag flag;
            std::call_once(flag, []() {
                static collections::array<metadata, 0> empty{};
                static std::tuple<> a;
                if constexpr (!type_traits::primary_types::is_void_v<Type>) {
                    static constexpr auto ctor_name = make_ctor_name<Type, Type>();
                    static constexpr auto default_ctor_name = make_ctor_name<Type>();
                    ctors_.emplace_back(
                        constructor::make({ctor_name.data(), ctor_name.size()}, [](Type value) { return Type{value}; }, a, empty));
                    ctors_.emplace_back(
                        constructor::make({default_ctor_name.data(), default_ctor_name.size()}, []() { return Type{}; }, a, empty));
                }
            });
            return ctors_;
        }

        property_storage_t &properties() noexcept override {
            static property_storage_t empty;
            return empty;
        }

        std::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept override {
            static std::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        std::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept override {
            static std::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        std::vector<metadata> &metadatas() noexcept override {
            return metadatas_;
        }

        enumeration &as_enumeration() noexcept override {
            static enumeration empty;
            return empty;
        }

        fundmental &as_fundmental() noexcept {
            return fundmental_type;
        }

    private:
        std::string_view name_;
        foundation::ctti::typeinfo typeinfo_;
        fundmental fundmental_type;
        std::vector<metadata> metadatas_;
    };
}

#endif