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
#ifndef RAINY_META_REFLECTION_PROPERTY_HPP
#define RAINY_META_REFLECTION_PROPERTY_HPP
#include <rainy/utility/any.hpp>
#include <rainy/meta/reflection/refl_impl/object_view.hpp>

namespace rainy::meta::reflection {
    enum class property_type {
        enum_property,
        static_property,
        const_static_property,
        volatile_static_property,
        const_volatile_static_property,
        member_property,
        const_member_property,
        volatile_member_property,
        const_volatile_member_property
    };

    enum class property_flags {
        none = 0,
        static_property = 1 << 0,
        const_property = 1 << 1,
        volatile_property = 1 << 2,
        member_property = 1 << 3,
        enum_property = 1 << 4
    };

    RAINY_INLINE constexpr property_flags operator&(property_flags lhs, property_flags rhs) noexcept {
        return static_cast<property_flags>(static_cast<std::underlying_type_t<property_flags>>(lhs) &
                                           static_cast<std::underlying_type_t<property_flags>>(rhs));
    }

    RAINY_INLINE constexpr property_flags operator|(property_flags lhs, property_flags rhs) noexcept {
        return static_cast<property_flags>(static_cast<std::underlying_type_t<property_flags>>(lhs) |
                                           static_cast<std::underlying_type_t<property_flags>>(rhs));
    }

    RAINY_INLINE constexpr property_flags operator^(property_flags lhs, property_flags rhs) noexcept {
        return static_cast<property_flags>(static_cast<std::underlying_type_t<property_flags>>(lhs) ^
                                           static_cast<std::underlying_type_t<property_flags>>(rhs));
    }

    RAINY_INLINE constexpr property_flags operator~(property_flags flags) noexcept {
        return static_cast<property_flags>(~static_cast<std::underlying_type_t<property_flags>>(flags));
    }

    RAINY_INLINE constexpr property_flags &operator&=(property_flags &lhs, property_flags rhs) noexcept {
        lhs = lhs & rhs;
        return lhs;
    }

    RAINY_INLINE constexpr property_flags &operator|=(property_flags &lhs, property_flags rhs) noexcept {
        lhs = lhs | rhs;
        return lhs;
    }

    RAINY_INLINE constexpr property_flags &operator^=(property_flags &lhs, property_flags rhs) noexcept {
        lhs = lhs ^ rhs;
        return lhs;
    }
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename Class>
    static constexpr property_flags deduction_property_type() noexcept {
        property_flags flags = property_flags::none;
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            flags |= property_flags::static_property;
        } else {
            flags |= property_flags::member_property;
        }
        if constexpr (type_traits::type_properties::is_const_v<Type>) {
            flags |= property_flags::const_property;
        }
        if constexpr (type_traits::type_properties::is_volatile_v<Type>) {
            flags |= property_flags::volatile_property;
        }
        if constexpr (type_traits::primary_types::is_enum_v<Type>) {
            flags |= property_flags::enum_property;
        }
        return flags;
    }
}

namespace rainy::meta::reflection {
    class RAINY_TOOLKIT_API property {
    public:
        property() noexcept {
        }

        template <typename Class, typename Type>
        property(Type Class::*property) {
            utility::construct_at(reinterpret_cast<property_accessor_impl<Type, Type Class::*, Class> *>(&property_storage), property);
        }

        template <typename Type>
        property(Type *static_property) {
            utility::construct_at(reinterpret_cast<property_accessor_impl<Type, Type *, void> *>(&property_storage), static_property);
        }

        template <typename Type, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_enum_v<Type>, int> = 0>
        property(Type enum_property) {
            utility::construct_at(reinterpret_cast<enum_property_accessor_impl<Type> *>(&property_storage), enum_property);
        }

        property(const property &right) noexcept;
        property(property &&right) noexcept;

        RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept;

        RAINY_NODISCARD const foundation::ctti::typeinfo &property_ctti_type() const noexcept;
        RAINY_NODISCARD const foundation::ctti::typeinfo &compound_type() const noexcept;

        property &operator=(const property &right) noexcept;
        property &operator=(property &&right) noexcept;

        void set_value(object_view object, const utility::any &val) const;

        template <typename Decayed>
        Decayed get_value(object_view object = non_exists_instance) const {
            return reinterpret_cast<const property_accessor *>(property_storage)->get_property(object).as<Decayed>();
        }

        utility::any get_value(object_view object = non_exists_instance) const;

        template <typename Class, typename Type>
        auto target() const noexcept -> Type Class::* {
            auto ptr = reinterpret_cast<const property_accessor *>(property_storage)->target(rainy_typeid(Type Class::*));
            return *reinterpret_cast<Type Class::**>(ptr);
        }

        property_flags type() const noexcept;

        bool is_const() const noexcept;

        bool is_static() const noexcept {
            return static_cast<bool>(type() | property_flags::static_property);
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
        struct property_accessor {
            virtual ~property_accessor() = default;
            virtual void set_property(object_view object, const utility::any &any) const = 0;
            virtual utility::any get_property(object_view object) noexcept = 0;
            RAINY_NODISCARD virtual const utility::any get_property(object_view object) const noexcept = 0;
            RAINY_NODISCARD virtual property_flags type() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo &compound_ctti() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo &which_belongs() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo &property_ctti_type() const noexcept = 0;
            RAINY_NODISCARD virtual std::uintptr_t target(const foundation::ctti::typeinfo &ctti) const noexcept = 0;
        };

        template <typename CompoundType>
        static const foundation::ctti::typeinfo &compound_type_res() noexcept {
            return rainy_typeid(CompoundType);
        }

        template <typename Type>
        static const foundation::ctti::typeinfo &property_ctti_type_res() noexcept {
            return rainy_typeid(Type);
        }

        template <typename Type, typename CompoundType, typename Class>
        struct property_accessor_impl : property_accessor {
            using compound_type = CompoundType;

            property_accessor_impl(compound_type property) noexcept : property_ptr(property) {
            }

            void set_property(object_view object,const utility::any& any) const override {
                if constexpr (type_traits::primary_types::is_member_object_pointer_v<compound_type>) {
                    if constexpr (type_traits::logical_traits::negation_v<type_traits::type_properties::is_const<Type>>) {
#if RAINY_ENABLE_DEBUG
                        utility::expects(
                            object.type().is_compatible(rainy_typeid(Type)),
                            "We can't set this property because we found the ClassType is not same with your passed instance!");
#else
                        if (!object.type().is_compatible(rainy_typeid(Type))) {
                            return;
                        }
#endif
                        utility::invoke(property_ptr, object.as<Class>()) = any.convert<Type>();
                    }
                } else {
                    if constexpr (type_traits::logical_traits::negation_v<type_traits::type_properties::is_const<Type>>) {
                        *property_ptr = any.convert<Type>();
                    }
                }
            }

            utility::any get_property(object_view object) noexcept override {
                if constexpr (type_traits::type_relations::is_void_v<Class>) {
                    return {*property_ptr};
                } else {
                    return {utility::invoke(property_ptr, object.as<Class>())};
                }
            }

            RAINY_NODISCARD const utility::any get_property(object_view object) const noexcept override {
                if constexpr (type_traits::type_relations::is_void_v<Class>) {
                    return {*property_ptr};
                } else {
                    return {utility::invoke(property_ptr, object.as<Class>())};
                }
            }

            RAINY_NODISCARD property_flags type() const noexcept override {
                return property_type_;
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &compound_ctti() const noexcept override {
                return compound_type_res<compound_type>();
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept override {
                return implements::which_belongs_res<Class>();
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &property_ctti_type() const noexcept override {
                return property_ctti_type_res<Type>();
            }

            RAINY_NODISCARD std::uintptr_t target(const foundation::ctti::typeinfo &ctti) const noexcept override {
                constexpr std::size_t typehash = rainy_typehash(compound_type);
                if (typehash == ctti.hash_code()) {
                    return reinterpret_cast<std::uintptr_t>(
                        const_cast<type_traits::other_trans::decay_t<compound_type> *>(&property_ptr));
                }
                return 0;
            }

            static constexpr property_flags property_type_ = implements::deduction_property_type<Type, Class>();
            compound_type property_ptr;
        };

        template <typename Type>
        struct enum_property_accessor_impl : property_accessor {
            using compound_type = Type;

            enum_property_accessor_impl(compound_type property) noexcept : property_ptr(property) {
            }

            void set_property(object_view object, const utility::any &any) const override {
                utility::expects(object.type() == any.type(), "Type Is Invalid!");
                object.as<Type>() = any.as<Type>();
            }

            utility::any get_property(object_view) noexcept override {
                return property_ptr;
            }

            RAINY_NODISCARD const utility::any get_property(object_view) const noexcept override {
                return property_ptr;
            }

            RAINY_NODISCARD property_flags type() const noexcept override {
                return property_type_;
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &compound_ctti() const noexcept override {
                return compound_type_res<Type>();
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept override {
                return implements::which_belongs_res<void>();
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &property_ctti_type() const noexcept override {
                return property_ctti_type_res<Type>();
            }

            RAINY_NODISCARD std::uintptr_t target(const foundation::ctti::typeinfo &) const noexcept override {
                return 0;
            }

            static constexpr property_flags property_type_ = implements::deduction_property_type<Type, void>();
            compound_type property_ptr;
        };

        enum fake_enum { fake1 };

        static constexpr std::size_t soo_buffer_size =
            sizeof(property_accessor_impl<int, int implements::fake_class::*, implements::fake_class>);

        static_assert(soo_buffer_size >= sizeof(property_accessor_impl<int, int implements::fake_class::*, implements::fake_class>));
        core::byte_t property_storage[soo_buffer_size]{};
    };
}

#endif
