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
#ifndef RAINY_META_REFLECTION_REFL_IMPL_OBJECT_VIEW_HPP
#define RAINY_META_REFLECTION_REFL_IMPL_OBJECT_VIEW_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/utility/implements/cast.hpp>

namespace rainy::meta::reflection::implements {
    struct fake_class {};

    static const auto static_type = foundation::ctti::typeinfo::create_typeinfo_by_name("static[no-definite-class-type]");

    template <typename Class>
    static const foundation::ctti::typeinfo &which_belongs_res() noexcept {
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            return static_type;
        } else {
            return rainy_typeid(Class);
        }
    }

    struct as_array {};
    struct as_reference {};
}

namespace rainy::meta::reflection {
    // 用于表示不存在的实例
    struct non_exists_instance_t {};

    static constexpr inline non_exists_instance_t non_exists_instance;

    class object_view {
    public:
        template <typename Ty>
        using enable_if_t = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Ty, object_view> && !type_traits::type_relations::is_same_v<Ty,non_exists_instance_t>, int>;

        template <typename Ty, enable_if_t<Ty> = 0>
        object_view(Ty &object) noexcept : // NOLINT
            object_{const_cast<void *>(static_cast<const void *>(utility::addressof(object)))}, ctti_{&rainy_typeid(Ty)} {
        }

        template <typename Ty,
                  std::enable_if_t<!type_traits::type_relations::is_same_v<std::decay_t<Ty>, object_view> &&
                      !type_traits::type_relations::is_same_v<std::decay_t<Ty>, non_exists_instance_t> &&
                      std::is_rvalue_reference_v<Ty &&> && !std::is_lvalue_reference_v<Ty>,
                  int> = 0>
        object_view(Ty &&object) : // NOLINT
            object_{const_cast<void *>(static_cast<const void *>(utility::addressof(object)))}, ctti_{&rainy_typeid(Ty &&)} {
        }

        object_view(void *const object, const foundation::ctti::typeinfo &ctti) noexcept :
            object_{object}, ctti_{&ctti} {
        }

        object_view(implements::as_array, void *const object, const foundation::ctti::typeinfo &ctti) noexcept :
            object_{nullptr}, ctti_{&ctti} {
            object_holder_ = object;
            object_ = static_cast<void *>(&object_holder_);
        }

        object_view(implements::as_reference, void *const object, const foundation::ctti::typeinfo &ctti) noexcept :
            object_{object}, ctti_{&ctti} {
        }

        object_view(non_exists_instance_t) noexcept :
            object_(nullptr), ctti_(&rainy_typeid(void)) {
        }

        object_view(object_view &&other) noexcept :
            object_(other.object_), ctti_(other.ctti_) {
            other.object_ = nullptr;
            other.ctti_ = &rainy_typeid(void);
        }

        object_view &operator=(object_view &&other) noexcept {
            if (this != &other) {
                object_ = utility::exchange(other.object_, nullptr);
                ctti_ = utility::exchange(other.ctti_, &rainy_typeid(void));
            }
            return *this;
        }

        object_view(const object_view &) = default;
        object_view &operator=(const object_view &) = default;
        object_view() = default;
        object_view(std::nullptr_t) = delete;
        object_view &operator=(std::nullptr_t) = delete;

        template <typename Decayed, enable_if_t<Decayed> = 0>
        RAINY_NODISCARD RAINY_INLINE Decayed *cast_to_pointer() noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return const_cast<remove_ref_t *>(static_cast<const object_view *>(this)->cast_to_pointer<Decayed>());
        }

        template <typename Decayed, enable_if_t<Decayed> = 0>
        RAINY_NODISCARD RAINY_INLINE const Decayed *cast_to_pointer() const noexcept {
            using namespace foundation::ctti;
            static constexpr typeinfo target_type = typeinfo::create<Decayed>();
            return ctti().is_compatible(target_type) ? reinterpret_cast<const Decayed *>(target_as_void_ptr())
                                                                      : nullptr;
        }

        template <typename Type>
        RAINY_NODISCARD auto as() noexcept -> decltype(auto) {
            return utility::implements::as_impl<Type>(target_as_void_ptr(), ctti());
        }
        
        template <typename Type, enable_if_t<Type> = 0>
        RAINY_NODISCARD auto as() const -> decltype(auto) {
            using namespace type_traits::cv_modify;
            using ret_type = decltype(utility::declval<object_view &>().template as<Type>());
            rainy_let nonconst = const_cast<object_view *>(this);
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ret_type>) {
                return nonconst->as<type_traits::reference_modify::add_const_rvalue_ref_t<Type>>();
            } else {
                return nonconst->as<type_traits::reference_modify::add_const_lvalue_ref_t<Type>>();
            }
        }

        RAINY_NODISCARD explicit operator bool() const noexcept {
            return valid();
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &ctti() const noexcept {
            rainy_assume(ctti_ != nullptr);
            return *ctti_;
        }

        RAINY_NODISCARD bool valid() const noexcept {
            rainy_assume(ctti_ != nullptr);
            return !ctti_->is_same(rainy_typeid(void));
        }

        RAINY_NODISCARD void *get_pointer() noexcept {
            return object_;
        }

        RAINY_NODISCARD const void *get_pointer() const noexcept {
            return object_;
        }

        RAINY_NODISCARD RAINY_INLINE const void *target_as_void_ptr() const noexcept {
            return object_;
        }

    private:
        void *object_;
        const foundation::ctti::typeinfo *ctti_{&rainy_typeid(void)};
        void *object_holder_{};
    };
}

#endif