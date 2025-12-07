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
#include <algorithm>
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/property.hpp>

namespace rainy::meta::reflection {
    field::field(const field &right) noexcept {
        if (this == utility::addressof(right)) {
            return;
        }
        core::builtin::copy_memory(field_storage, right.field_storage, soo_buffer_size);
    }

    field::field(field &&right) noexcept {
        if (this == utility::addressof(right)) {
            return;
        }
        core::builtin::copy_memory(field_storage, right.field_storage, soo_buffer_size);
        std::memset(right.field_storage, 0, soo_buffer_size);
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &field::which_belongs() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->which_belongs();
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &field::field_ctti_type() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->field_ctti_type();
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &field::compound_type() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->compound_ctti();
    }

    field &field::operator=(const field &right) noexcept {
        if (this == utility::addressof(right)) {
            return *this;
        }
        core::builtin::copy_memory(field_storage, right.field_storage, soo_buffer_size);
        return *this;
    }

    field &field::operator=(field &&right) noexcept {
        core::builtin::copy_memory(field_storage, right.field_storage, soo_buffer_size);
        std::memset(right.field_storage, 0, soo_buffer_size);
        return *this;
    }

    void field::set_value(annotations::lifetime::in<object_view> object,const utility::any& val) const {
        reinterpret_cast<const field_accessor *>(field_storage)->set_field(object, val);
    }

    utility::any::reference field::get_value(annotations::lifetime::in<object_view> object) {
        return reinterpret_cast<field_accessor *>(field_storage)->get_field(object);
    }

    utility::any::reference field::get_value(annotations::lifetime::in<object_view> object) const {
        return reinterpret_cast<const field_accessor *>(field_storage)->get_field(object);
    }

    bool field::is_const() const noexcept {
        return static_cast<bool>(type() | field_flags::const_field);
    }

    bool field::is_volatile() const noexcept {
        return static_cast<bool>(type() | field_flags::volatile_field);
    }

    bool field::is_member_pointer() const noexcept {
        return static_cast<bool>(type() | field_flags::member_field);
    }

    bool field::is_pointer() const noexcept {
        return field_ctti_type().has_traits(foundation::ctti::traits::is_pointer);
    }

    bool field::is_array() const noexcept {
        return field_ctti_type().has_traits(foundation::ctti::traits::is_array);
    }

    bool field::is_fundamental() const noexcept {
        return field_ctti_type().has_traits(foundation::ctti::traits::is_fundamental);
    }

    field_flags field::type() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->type();
    }

    bool field::is_compound() const noexcept {
        return field_ctti_type().has_traits(foundation::ctti::traits::is_compound);
    }

    void field::clear() noexcept {
        field_storage[0] = '\0';
        std::memset(field_storage, 0, soo_buffer_size);
    }

    bool field::empty() const noexcept {
        return std::all_of(utility::begin(field_storage), utility::end(field_storage), +[](const core::byte_t c) { return c == '\0'; });
    }
}

namespace rainy::meta::reflection {
    std::string_view property::get_name() const noexcept {
        return ptr->name;
    }

    const metadata &property::get_metadata(const utility::any &key) const noexcept {
        return implements::find_metadata(ptr->metadata, key);
    }

    collections::views::array_view<metadata> property::get_metadatas() const noexcept {
        return ptr->metadata;
    }
}
