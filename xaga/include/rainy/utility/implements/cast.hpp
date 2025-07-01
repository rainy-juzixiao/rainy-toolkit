#ifndef RAINY_UTILITY_IMPLEMENTS_CAST_HPP
#define RAINY_UTILITY_IMPLEMENTS_CAST_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/utility/implements/any_bad_cast.hpp>

namespace rainy::utility::implements {
    template <typename Type>
    RAINY_NODISCARD RAINY_INLINE auto as_impl(const void *target_pointer, const foundation::ctti::typeinfo &type) noexcept -> decltype(auto) {
        using namespace foundation::ctti;
        void *ptr = const_cast<void *>(target_pointer);
        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Type>) {
            if constexpr (type_traits::type_properties::is_const_v<type_traits::reference_modify::remove_reference_t<Type>>) {
                // 返回 const lvalue 引用
                return *static_cast<const type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
            } else {
                // 返回非 const lvalue 引用
                assert(!type.is_const() && "Cannot cast const value to non-const rvalue reference");
                return *static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
            }
        } else if constexpr (type_traits::primary_types::is_rvalue_reference_v<Type>) {
            if constexpr (type_traits::type_properties::is_const_v<std::remove_reference_t<Type>>) {
                // 返回 const rvalue 引用
                return utility::move(*static_cast<const type_traits::reference_modify::remove_reference_t<Type> *>(ptr));
            } else {
                // 返回非 const rvalue 引用
                assert(!type.is_const() && "Cannot cast const value to non-const rvalue reference");
                return utility::move(*static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr));
            }
        } else {
            if (type.is_reference()) {
                if (type.is_lvalue_reference()) {
                    constexpr auto ctti =
                        foundation::ctti::typeinfo::create<type_traits::reference_modify::add_lvalue_reference_t<Type>>();
                    if (ctti == type) {
                        // 一致的类型，不需要进一步检查，直接返回
                        return *static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
                    }
                    if (!ctti.is_const()) {
                        // 检查是否const
                        assert(type.is_const() && "Cannot directly cast a non-const value to reference.");
                    }
                } else {
                    constexpr auto ctti =
                        foundation::ctti::typeinfo::create<type_traits::reference_modify::add_rvalue_reference_t<Type>>();
                    if (ctti == type) {
                        // 一致的类型，不需要进一步检查，直接返回
                        return *static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
                    }
                    if (!ctti.is_const()) {
                        // 检查是否const
                        assert(type.is_const() && "Cannot directly cast a non-const value to reference.");
                    }
                }
            }
            return *static_cast<type_traits::reference_modify::remove_reference_t<Type> *>(ptr);
        }
    }

    template <typename Type>
    RAINY_INLINE bool is_as_runnable(const foundation::ctti::typeinfo &type) {
        using namespace foundation::ctti;
        using namespace type_traits;
        using namespace foundation::exceptions::cast;
        static const auto target_type = typeinfo::create<Type>();
        if (!target_type.is_compatible(type)) {
            return false;
        }
        if constexpr (primary_types::is_lvalue_reference_v<Type>) {
            if constexpr (type_properties::is_const_v<reference_modify::remove_reference_t<Type>>) {
                return true;
            } else {
                return !type.has_traits(traits::is_const);
            }
        } else if constexpr (primary_types::is_rvalue_reference_v<Type>) {
            if constexpr (type_properties::is_const_v<std::remove_reference_t<Type>>) {
                return true;
            } else {
                return !type.has_traits(traits::is_const);
            }
        } else {
            if (type.is_reference()) {
                if (type.is_lvalue_reference()) {
                    constexpr auto ctti =
                        foundation::ctti::typeinfo::create<type_traits::reference_modify::add_lvalue_reference_t<Type>>();
                    if (ctti == type) {
                        return true;
                    }
                    if (!ctti.is_const()) {
                        return type.is_const();
                    }
                } else {
                    constexpr auto ctti =
                        foundation::ctti::typeinfo::create<type_traits::reference_modify::add_rvalue_reference_t<Type>>();
                    if (ctti == type) {
                        return true;
                    }
                    if (!ctti.is_const()) {
                        return type.is_const();
                    }
                }
            }
            return true;
        }
    }
}

#endif