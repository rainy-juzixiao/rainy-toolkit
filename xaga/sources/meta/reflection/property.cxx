#include <algorithm>
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/property.hpp>

namespace rainy::meta::reflection {
    property::property(const property &right) noexcept {
        if (this == utility::addressof(right)) {
            return;
        }
        core::builtin::copy_memory(property_storage, right.property_storage, soo_buffer_size);
    }

    property::property(property &&right) noexcept {
        if (this == utility::addressof(right)) {
            return;
        }
        core::builtin::copy_memory(property_storage, right.property_storage, soo_buffer_size);
        std::memset(right.property_storage, 0, soo_buffer_size);
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &property::which_belongs() const noexcept {
        return reinterpret_cast<const property_accessor *>(property_storage)->which_belongs();
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &property::property_ctti_type() const noexcept {
        return reinterpret_cast<const property_accessor *>(property_storage)->property_ctti_type();
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &property::compound_type() const noexcept {
        return reinterpret_cast<const property_accessor *>(property_storage)->compound_ctti();
    }

    property &property::operator=(const property &right) noexcept {
        if (this == utility::addressof(right)) {
            return *this;
        }
        core::builtin::copy_memory(property_storage, right.property_storage, soo_buffer_size);
        return *this;
    }

    property &property::operator=(property &&right) noexcept {
        core::builtin::copy_memory(property_storage, right.property_storage, soo_buffer_size);
        std::memset(right.property_storage, 0, soo_buffer_size);
        return *this;
    }

    void property::set_value(object_view object,const utility::any& val) const {
        reinterpret_cast<const property_accessor *>(property_storage)->set_property(object, val);
    }

    utility::any property::get_value(object_view object) const {
        return reinterpret_cast<const property_accessor *>(property_storage)->get_property(object);
    }

    bool property::is_const() const noexcept {
        return static_cast<bool>(type() | property_flags::const_property);
    }

    bool property::is_volatile() const noexcept {
        return static_cast<bool>(type() | property_flags::volatile_property);
    }

    bool property::is_member_pointer() const noexcept {
        return static_cast<bool>(type() | property_flags::member_property);
    }

    bool property::is_pointer() const noexcept {
        return property_ctti_type().has_traits(foundation::ctti::traits::is_pointer);
    }

    bool property::is_array() const noexcept {
        return property_ctti_type().has_traits(foundation::ctti::traits::is_array);
    }

    bool property::is_fundamental() const noexcept {
        return property_ctti_type().has_traits(foundation::ctti::traits::is_fundamental);
    }

    property_flags property::type() const noexcept {
        return reinterpret_cast<const property_accessor *>(property_storage)->type();
    }

    bool property::is_enum() const noexcept {
        return property_ctti_type().has_traits(foundation::ctti::traits::is_enum);
    }

    bool property::is_compound() const noexcept {
        return property_ctti_type().has_traits(foundation::ctti::traits::is_compound);
    }

    void property::clear() noexcept {
        property_storage[0] = '\0';
        std::memset(property_storage, 0, soo_buffer_size);
    }

    bool property::empty() const noexcept {
        return std::all_of(utility::begin(property_storage), utility::end(property_storage), +[](core::byte_t c) { return c == '\0'; });
    }
}