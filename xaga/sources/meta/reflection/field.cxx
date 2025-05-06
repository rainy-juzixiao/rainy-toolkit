#include <algorithm>
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/field.hpp>

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

    RAINY_NODISCARD const foundation::rtti::typeinfo &field::which_belongs() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->which_belongs();
    }

    RAINY_NODISCARD const foundation::rtti::typeinfo &field::field_rtti_type() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->field_rtti_type();
    }

    RAINY_NODISCARD const foundation::rtti::typeinfo &field::compound_type() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->compound_rtti();
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

    const field &field::set_value(object_view object,const utility::any& val) const {
        reinterpret_cast<const field_accessor *>(field_storage)->set_field(object, val);
        return *this;
    }

    utility::any field::get_value(object_view object) const {
        return reinterpret_cast<const field_accessor *>(field_storage)->get_field(object);
    }

    bool field::is_const() const noexcept {
        switch (type()) {
            case field_type::const_member_field:
            case field_type::const_static_field:
            case field_type::const_volatile_member_field:
            case field_type::const_volatile_static_field:
                return true;
            default:
                return false;
        }
    }

    bool field::is_volatile() const noexcept {
        switch (type()) {
            case field_type::volatile_member_field:
            case field_type::volatile_static_field:
            case field_type::const_volatile_member_field:
            case field_type::const_volatile_static_field:
                return true;
            default:
                return false;
        }
    }

    bool field::is_member_pointer() const noexcept {
        return type() >= field_type::member_field && type() <= field_type::const_volatile_member_field;
    }

    bool field::is_pointer() const noexcept {
        return field_rtti_type().has_traits(foundation::rtti::traits::is_pointer);
    }

    bool field::is_array() const noexcept {
        return field_rtti_type().has_traits(foundation::rtti::traits::is_array);
    }

    bool field::is_fundamental() const noexcept {
        return field_rtti_type().has_traits(foundation::rtti::traits::is_fundamental);
    }

    field_type field::type() const noexcept {
        return reinterpret_cast<const field_accessor *>(field_storage)->type();
    }

    bool field::is_enum() const noexcept {
        return field_rtti_type().has_traits(foundation::rtti::traits::is_enum);
    }

    bool field::is_compound() const noexcept {
        return field_rtti_type().has_traits(foundation::rtti::traits::is_compound);
    }

    void field::clear() noexcept {
        field_storage[0] = '\0';
        std::memset(field_storage, 0, soo_buffer_size);
    }

    bool field::empty() const noexcept {
        return std::all_of(utility::begin(field_storage), utility::end(field_storage), +[](core::byte_t c) { return c == '\0'; });
    }
}