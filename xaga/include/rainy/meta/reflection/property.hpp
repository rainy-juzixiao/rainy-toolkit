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
#ifndef RAINY_META_REFLECTION_field_HPP
#define RAINY_META_REFLECTION_field_HPP
#include <rainy/utility/any.hpp>
#include <rainy/meta/reflection/object_view.hpp>
#include <rainy/meta/reflection/metadata.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace rainy::meta::reflection {
    enum class field_flags {
        none = 0,
        static_field = 1 << 0,
        const_field = 1 << 1,
        volatile_field = 1 << 2,
        member_field = 1 << 3,
        enum_field = 1 << 4
    };
    
    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(field_flags);
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename Class>
    static constexpr field_flags deduction_field_type() noexcept {
        field_flags flags = field_flags::none;
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            flags |= field_flags::static_field;
        } else {
            flags |= field_flags::member_field;
        }
        if constexpr (type_traits::type_properties::is_const_v<Type>) {
            flags |= field_flags::const_field;
        }
        if constexpr (type_traits::type_properties::is_volatile_v<Type>) {
            flags |= field_flags::volatile_field;
        }
        if constexpr (type_traits::primary_types::is_enum_v<Type>) {
            flags |= field_flags::enum_field;
        }
        return flags;
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

        field(const field &right) noexcept;
        field(field &&right) noexcept;

        RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept;

        RAINY_NODISCARD const foundation::ctti::typeinfo &field_ctti_type() const noexcept;
        RAINY_NODISCARD const foundation::ctti::typeinfo &compound_type() const noexcept;

        field &operator=(const field &right) noexcept;
        field &operator=(field &&right) noexcept;

        utility::any::reference operator()(object_view object) {
            return get_value(object);
        }

        void set_value(object_view object, const utility::any &val) const;

        template <typename Decayed>
        Decayed get_value(object_view object = non_exists_instance) const {
            return reinterpret_cast<const field_accessor *>(field_storage)->get_field(object).as<Decayed>();
        }

        utility::any::reference get_value(object_view object = non_exists_instance) const;

        template <typename Class, typename Type>
        auto target() const noexcept -> Type Class::* {
            auto ptr = reinterpret_cast<const field_accessor *>(field_storage)->target(rainy_typeid(Type Class::*));
            return *reinterpret_cast<Type Class::**>(ptr);
        }

        field_flags type() const noexcept;

        bool is_const() const noexcept;

        bool is_static() const noexcept {
            return static_cast<bool>(type() | field_flags::static_field);
        }

        bool is_volatile() const noexcept;

        bool is_member_pointer() const noexcept;

        bool is_pointer() const noexcept;

        bool is_array() const noexcept;

        bool is_fundamental() const noexcept;

        bool is_compound() const noexcept;

        void clear() noexcept;

        bool empty() const noexcept;

    private:
        struct field_accessor {
            virtual ~field_accessor() = default;
            virtual void set_field(object_view object, const utility::any &any) const = 0;
            RAINY_NODISCARD virtual utility::any::reference get_field(object_view object) noexcept = 0;
            RAINY_NODISCARD virtual const utility::any::reference get_field(object_view object) const noexcept = 0;
            RAINY_NODISCARD virtual field_flags type() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo &compound_ctti() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo &which_belongs() const noexcept = 0;
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo &field_ctti_type() const noexcept = 0;
            RAINY_NODISCARD virtual std::uintptr_t target(const foundation::ctti::typeinfo &ctti) const noexcept = 0;
        };

        template <typename CompoundType>
        static const foundation::ctti::typeinfo &compound_type_res() noexcept {
            return rainy_typeid(CompoundType);
        }

        template <typename Type>
        static const foundation::ctti::typeinfo &field_ctti_type_res() noexcept {
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
                            object.type().is_compatible(rainy_typeid(Class)),
                            "We can't set this field because we found the ClassType is not same with your passed instance!");
#else
                        if (!object.type().is_compatible(rainy_typeid(Type))) {
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

            utility::any::reference get_field(object_view object) noexcept override {
                if constexpr (type_traits::type_relations::is_void_v<Class>) {
                    return {*field_ptr};
                } else {
                    return {utility::invoke(field_ptr, object.as<Class>())};
                }
            }

            RAINY_NODISCARD const utility::any::reference get_field(object_view object) const noexcept override {
                if constexpr (type_traits::type_relations::is_void_v<Class>) {
                    return {*field_ptr};
                } else {
                    return {utility::invoke(field_ptr, object.as<Class>())};
                }
            }

            RAINY_NODISCARD field_flags type() const noexcept override {
                return field_type_;
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &compound_ctti() const noexcept override {
                return compound_type_res<compound_type>();
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept override {
                return implements::which_belongs_res<Class>();
            }

            RAINY_NODISCARD const foundation::ctti::typeinfo &field_ctti_type() const noexcept override {
                return field_ctti_type_res<Type>();
            }

            RAINY_NODISCARD std::uintptr_t target(const foundation::ctti::typeinfo &ctti) const noexcept override {
                constexpr std::size_t typehash = rainy_typehash(compound_type);
                if (typehash == ctti.hash_code()) {
                    return reinterpret_cast<std::uintptr_t>(
                        const_cast<type_traits::other_trans::decay_t<compound_type> *>(&field_ptr));
                }
                return 0;
            }

            static constexpr field_flags field_type_ = implements::deduction_field_type<Type, Class>();
            compound_type field_ptr;
        };

        static constexpr std::size_t soo_buffer_size =
            sizeof(field_accessor_impl<int, int implements::fake_class::*, implements::fake_class>);

        static_assert(soo_buffer_size >= sizeof(field_accessor_impl<int, int implements::fake_class::*, implements::fake_class>));
        core::byte_t field_storage[soo_buffer_size]{};
    };
}

namespace rainy::meta::reflection {
    class RAINY_TOOLKIT_API property : public field {
    public:
        property() noexcept = default;

        template <typename Field, typename... Args, std::size_t N = 0,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<field, Field>, int> = 0>
        static property make(std::string_view name, Field &&field, collections::array<metadata, N> &metadatas) {
            return property{name, field, metadatas};
        }

        property(property &&right) noexcept : field(utility::move(right)), ptr{utility::move(right.ptr)} {
        }

        property(const property &right) noexcept : field(right), ptr(right.ptr) {
        }

        property &operator=(const property &right) {
            field::operator=(right);
            this->ptr = right.ptr;
            return *this;
        }

        property &operator=(property &right) {
            field::operator=(utility::move(right));
            this->ptr = utility::move(right.ptr);
            return *this;
        }

        std::string_view get_name() const noexcept;

        const metadata &get_metadata(const utility::any &key) const noexcept;

        collections::views::array_view<metadata> get_metadatas() const noexcept;

    private:
        template <typename Field, typename... Args, std::size_t N = 0,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<field, Field>, int> = 0>
        property(std::string_view name, Field &&field, collections::array<metadata, N> &metadatas) noexcept :
            field(utility::forward<Field>(field)), ptr(std::make_shared<data>(utility::move(name))) {
            if constexpr (N != 0) {
                for (metadata &meta: metadatas) {
                    ptr->metadata.emplace_back(utility::move(meta));
                }
            }
        }

        struct data {
            std::string_view name;
            std::vector<reflection::metadata> metadata;
        };

        std::shared_ptr<data> ptr;
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
