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
#ifndef RAINY_META_REFLECTION_FIELD_HPP
#define RAINY_META_REFLECTION_FIELD_HPP
#include <rainy/utility/any.hpp>
#include <rainy/meta/reflection/refl_impl/object_view.hpp>

namespace rainy::meta::reflection {
    enum class field_type {
        enum_field,
        static_field,
        const_static_field,
        volatile_static_field,
        const_volatile_static_field,
        member_field,
        const_member_field,
        volatile_member_field,
        const_volatile_member_field
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename Class>
    static constexpr field_type deduction_field_type() noexcept {
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            if constexpr (type_traits::type_properties::is_const_v<Type>) {
                if constexpr (type_traits::type_properties::is_volatile_v<Type>) {
                    return field_type::const_volatile_static_field;
                }
                return field_type::const_static_field;
            }
            if constexpr (type_traits::type_properties::is_volatile_v<Type>) {
                return field_type::volatile_static_field;
            }
            return field_type::static_field;
        }
        if constexpr (type_traits::type_properties::is_const_v<Type>) {
            if constexpr (type_traits::type_properties::is_volatile_v<Type>) {
                return field_type::const_volatile_member_field;
            }
            return field_type::const_member_field;
        }
        if constexpr (type_traits::type_properties::is_volatile_v<Type>) {
            return field_type::volatile_member_field;
        }
        return type_traits::primary_types::is_enum_v<Type> ? field_type::enum_field : field_type::member_field;
    }
}

namespace rainy::meta::reflection {
    class RAINY_TOOLKIT_API field {
    public:
        field() noexcept {
        }

        template <typename Class, typename Type>
        field(Type Class::*field) {
            utility::construct_at(reinterpret_cast<field_accessor_impl<Type, Type Class::*, Class> *>(&field_storage), field);
        }

        template <typename Type>
        field(Type *static_field) {
            utility::construct_at(reinterpret_cast<field_accessor_impl<Type, Type *, void> *>(&field_storage), static_field);
        }

        template <typename Type, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<Type>, int> = 0>
        field(Type enum_field) {
            utility::construct_at(reinterpret_cast<enum_field_accessor_impl<Type> *>(&field_storage), enum_field);
        }

        field(const field &right) noexcept;
        field(field &&right) noexcept;

        RAINY_NODISCARD const foundation::rtti::typeinfo &which_belongs() const noexcept;

        RAINY_NODISCARD const foundation::rtti::typeinfo &field_rtti_type() const noexcept;
        RAINY_NODISCARD const foundation::rtti::typeinfo &compound_type() const noexcept;

        field &operator=(const field &right) noexcept;
        field &operator=(field &&right) noexcept;

        const field &set_value(object_view object,const utility::any& val) const;

        template <typename Decayed>
        Decayed get_value(object_view object = non_exists_instance) const {
            return reinterpret_cast<const field_accessor *>(field_storage)->get_field(object).as<Decayed>();
        }

        utility::any get_value(object_view object = non_exists_instance) const;

        template <typename Class, typename Type>
        auto target() const noexcept -> Type Class::* {
            auto ptr = reinterpret_cast<const field_accessor *>(field_storage)->target(rainy_typeid(Type Class::*));
            return *reinterpret_cast<Type Class::**>(ptr);
        }

        field_type type() const noexcept;

        bool is_const() const noexcept;

        bool is_static() const noexcept {
            return type() >= field_type::static_field && type() <= field_type::const_volatile_static_field;
        }

        bool is_volatile() const noexcept;

        bool is_member_pointer() const noexcept;

        bool is_pointer() const noexcept;

        bool is_array() const noexcept;

        bool is_fundamental() const noexcept;

        bool is_enum() const noexcept;

        bool is_compound() const noexcept;

        void clear() noexcept;

        bool empty() const noexcept;

    private:
        struct field_accessor {
            virtual ~field_accessor() = default;
            virtual void set_field(object_view object, const utility::any &any) const = 0;
            virtual utility::any get_field(object_view object) noexcept = 0;
            RAINY_NODISCARD virtual const utility::any get_field(object_view object) const noexcept = 0;
            RAINY_NODISCARD virtual field_type type() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::rtti::typeinfo &compound_rtti() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::rtti::typeinfo &which_belongs() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::rtti::typeinfo &field_rtti_type() const noexcept = 0;
            RAINY_NODISCARD virtual std::uintptr_t target(const foundation::rtti::typeinfo &rtti) const noexcept = 0;
        };

        template <typename CompoundType>
        static const foundation::rtti::typeinfo &compound_type_res() noexcept {
            return rainy_typeid(CompoundType);
        }

        template <typename Type>
        static const foundation::rtti::typeinfo &field_rtti_type_res() noexcept {
            return rainy_typeid(Type);
        }

        template <typename Type, typename CompoundType, typename Class>
        struct field_accessor_impl : field_accessor {
            using compound_type = CompoundType;

            field_accessor_impl(compound_type field) noexcept : field_ptr(field) {
            }

            void set_field(object_view object,const utility::any& any) const override {
                if constexpr (type_traits::primary_types::is_member_object_pointer_v<compound_type>) {
                    if constexpr (type_traits::logical_traits::negation_v<type_traits::type_properties::is_const<Type>>) {
#if RAINY_ENABLE_DEBUG
                        utility::expects(
                            object.rtti().is_compatible(rainy_typeid(Type)),
                            "We can't set this field because we found the ClassType is not same with your passed instance!");
#else
                        if (!object.rtti().is_compatible(rainy_typeid(Type))) {
                            return;
                        }
#endif
                        utility::invoke(field_ptr, object.as<Class>()) = any.convert<Type>();
                    }
                } else {
                    if constexpr (type_traits::logical_traits::negation_v<type_traits::type_properties::is_const<Type>>) {
                        *field_ptr = any.convert<Type>();
                    }
                }
            }

            utility::any get_field(object_view object) noexcept override {
                return {utility::invoke(field_ptr, object.as<Class>())};
            }

            RAINY_NODISCARD const utility::any get_field(object_view object) const noexcept override {
                return {utility::invoke(field_ptr, object.as<Class>())};
            }

            RAINY_NODISCARD field_type type() const noexcept override {
                return field_type_;
            }

            RAINY_NODISCARD const foundation::rtti::typeinfo &compound_rtti() const noexcept override {
                return compound_type_res<compound_type>();
            }

            RAINY_NODISCARD const foundation::rtti::typeinfo &which_belongs() const noexcept override {
                return implements::which_belongs_res<Class>();
            }

            RAINY_NODISCARD const foundation::rtti::typeinfo &field_rtti_type() const noexcept override {
                return field_rtti_type_res<Type>();
            }

            RAINY_NODISCARD std::uintptr_t target(const foundation::rtti::typeinfo &rtti) const noexcept override {
                constexpr std::size_t typehash = rainy_typehash(compound_type);
                if (typehash == rtti.hash_code()) {
                    return reinterpret_cast<std::uintptr_t>(
                        const_cast<type_traits::other_trans::decay_t<compound_type> *>(&field_ptr));
                }
                return 0;
            }

            static constexpr field_type field_type_ = implements::deduction_field_type<Type, Class>();
            compound_type field_ptr;
        };

        template <typename Type>
        struct enum_field_accessor_impl : field_accessor {
            using compound_type = Type;

            enum_field_accessor_impl(compound_type field) noexcept : field_ptr(field) {
            }

            void set_field(object_view object, const utility::any &any) const override {
                utility::expects(object.rtti() == any.type(), "Type Is Invalid!");
                object.as<Type>() = any.as<Type>();
            }

            utility::any get_field(object_view) noexcept override {
                return field_ptr;
            }

            RAINY_NODISCARD const utility::any get_field(object_view) const noexcept override {
                return field_ptr;
            }

            RAINY_NODISCARD field_type type() const noexcept override {
                return field_type_;
            }

            RAINY_NODISCARD const foundation::rtti::typeinfo &compound_rtti() const noexcept override {
                return compound_type_res<Type>();
            }

            RAINY_NODISCARD const foundation::rtti::typeinfo &which_belongs() const noexcept override {
                return implements::which_belongs_res<void>();
            }

            RAINY_NODISCARD const foundation::rtti::typeinfo &field_rtti_type() const noexcept override {
                return field_rtti_type_res<Type>();
            }

            RAINY_NODISCARD std::uintptr_t target(const foundation::rtti::typeinfo &) const noexcept override {
                return 0;
            }

            static constexpr field_type field_type_ = implements::deduction_field_type<Type, void>();
            compound_type field_ptr;
        };

        enum fake_enum { fake1 };

        static constexpr std::size_t soo_buffer_size =
            sizeof(field_accessor_impl<int, int implements::fake_class::*, implements::fake_class>);
        static constexpr std::size_t soo_enum_size = sizeof(enum_field_accessor_impl<fake_enum>);

        static_assert(soo_buffer_size >= sizeof(field_accessor_impl<int, int implements::fake_class::*, implements::fake_class>));
        core::byte_t field_storage[soo_buffer_size]{};
    };
}

#endif
