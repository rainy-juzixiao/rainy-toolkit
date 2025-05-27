#ifndef RAINY_META_REFLECTION_REFL_IMPL_OBJECT_VIEW_HPP
#define RAINY_META_REFLECTION_REFL_IMPL_OBJECT_VIEW_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::meta::reflection::implements {
    struct fake_class {};

    static const auto static_type = foundation::rtti::typeinfo::create_typeinfo_by_name("static[no-definite-class-type]");

    template <typename Class>
    static const foundation::rtti::typeinfo &which_belongs_res() noexcept {
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            return static_type;
        } else {
            return rainy_typeid(Class);
        }
    }
}

namespace rainy::meta::reflection::implements {
    enum class object_view_rep {
        normal,
        pointer,
        reference,
        pointer_with_reference
    };
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
        object_view(Ty& object) noexcept :
            object_{const_cast<void *>(static_cast<const void *>(utility::addressof(object)))}, rtti_{&rainy_typeid(Ty)} {
            if constexpr (type_traits::composite_types::is_reference_v<Ty> || type_traits::primary_types::is_pointer_v<Ty>) {
                this->rep = implements::object_view_rep::pointer_with_reference;
            } else if constexpr (type_traits::composite_types::is_reference_v<Ty>) {
                this->rep = implements::object_view_rep::reference;
            } else if constexpr (type_traits::primary_types::is_pointer_v<Ty>) {
                this->rep = implements::object_view_rep::pointer;
            } else {
                this->rep = implements::object_view_rep::normal;
            }
        }

        template <typename Ty,
                  std::enable_if_t<!type_traits::type_relations::is_same_v<std::decay_t<Ty>, object_view> &&
                      !type_traits::type_relations::is_same_v<std::decay_t<Ty>, non_exists_instance_t> &&
                      std::is_rvalue_reference_v<Ty &&> && !std::is_lvalue_reference_v<Ty>,
                  int> = 0>
        object_view(Ty &&object) :
            object_{const_cast<void *>(static_cast<const void *>(utility::addressof(object)))}, rtti_{&rainy_typeid(Ty &&)} {
            if constexpr (type_traits::composite_types::is_reference_v<Ty> || type_traits::primary_types::is_pointer_v<Ty>) {
                this->rep = implements::object_view_rep::pointer_with_reference;
            } else if constexpr (type_traits::composite_types::is_reference_v<Ty>) {
                this->rep = implements::object_view_rep::reference;
            } else if constexpr (type_traits::primary_types::is_pointer_v<Ty>) {
                this->rep = implements::object_view_rep::pointer;
            } else {
                this->rep = implements::object_view_rep::normal;
            }
        }

        object_view(void *const object, const foundation::rtti::typeinfo &rtti, bool is_any = false) noexcept :
            object_{object}, rtti_{&rtti}, rep{implements::object_view_rep::normal}, is_any_{is_any} {
            using namespace foundation::rtti;
            if (rtti.has_traits(traits::is_pointer) && (rtti.has_traits(traits::is_lref) || rtti.has_traits(traits::is_rref))) {
                this->rep = implements::object_view_rep::pointer_with_reference;
            } else if (rtti.has_traits(traits::is_pointer)) {
                this->rep = implements::object_view_rep::pointer;
            } else if (rtti.has_traits(traits::is_lref) || rtti.has_traits(traits::is_rref)) {
                this->rep = implements::object_view_rep::reference;
            } else {
                this->rep = implements::object_view_rep::normal;
            }
        }

        object_view(non_exists_instance_t) noexcept : object_(nullptr), rtti_(&rainy_typeid(void)),rep(implements::object_view_rep::normal) {
        }

        object_view(object_view &&other) noexcept :
            object_(other.object_), rtti_(other.rtti_), rep{implements::object_view_rep::normal} {
            other.object_ = nullptr;
            other.rtti_ = &rainy_typeid(void);
            other.rep = implements::object_view_rep::normal;
        }

        object_view &operator=(object_view &&other) noexcept {
            if (this != &other) {
                object_ = utility::exchange(other.object_, nullptr);
                rtti_ = utility::exchange(other.rtti_, &rainy_typeid(void));
                rep = implements::object_view_rep::normal;
            }
            return *this;
        }

        object_view(const object_view &) = default;
        object_view &operator=(const object_view &) = default;

        object_view() = delete;
        object_view(std::nullptr_t) = delete;
        object_view &operator=(std::nullptr_t) = delete;

        template <typename Decayed, enable_if_t<Decayed> = 0>
        RAINY_NODISCARD Decayed *cast_to_pointer() noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return const_cast<remove_ref_t *>(static_cast<const object_view *>(this)->cast_to_pointer<Decayed>());
        }

        template <typename Decayed, enable_if_t<Decayed> = 0>
        RAINY_NODISCARD const Decayed *cast_to_pointer() const noexcept {
            using namespace foundation::rtti;
            static constexpr typeinfo target_type = typeinfo::create<Decayed>();
            return rtti().is_compatible(target_type) ? reinterpret_cast<const Decayed *>(target_as_void_ptr())
                                                                      : nullptr;
        }

        template <typename Type, enable_if_t<Type> = 0>
        RAINY_NODISCARD auto as() -> decltype(auto) {
#if RAINY_ENABLE_DEBUG
            rainy_let ptr = cast_to_pointer<type_traits::other_trans::decay_t<Type>>();
#else
            rainy_let ptr = static_cast<type_traits::other_trans::decay_t<Type> *>(const_cast<void *>(target_as_void_ptr()));
#endif
            if (!ptr) {
                std::terminate();
            }
            if constexpr (type_traits::primary_types::is_lvalue_reference_v<Type>) {
                if constexpr (type_traits::type_properties::is_const_v<std::remove_reference_t<Type>>) {
                    return *static_cast<const std::remove_reference_t<Type> *>(ptr);
                } else {
                    return *static_cast<std::remove_reference_t<Type> *>(ptr);
                }
            } else if constexpr (std::is_rvalue_reference_v<Type>) {
                if constexpr (std::is_const_v<std::remove_reference_t<Type>>) {
                    return *static_cast<const std::remove_reference_t<Type> *>(ptr);
                } else {
                    return utility::move(*static_cast<std::remove_reference_t<Type> *>(ptr));
                }
            } else {
                return *static_cast<std::remove_reference_t<Type> *>(ptr);
            }
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

        RAINY_NODISCARD const foundation::rtti::typeinfo &rtti() const noexcept {
            rainy_assume(rtti_ != nullptr);
            return *rtti_;
        }

        RAINY_NODISCARD bool valid() const noexcept {
            rainy_assume(rtti_ != nullptr);
            return !rtti_->is_same(rainy_typeid(void));
        }

        RAINY_NODISCARD void *get_pointer() noexcept {
            return valid() ? object_ : nullptr;
        }

        RAINY_NODISCARD const void *get_pointer() const noexcept {
            return valid() ? object_ : nullptr;
        }

        RAINY_NODISCARD const void *target_as_void_ptr() const noexcept {
            using implements::object_view_rep;
            switch (rep) {
                case object_view_rep::normal: 
                    return object_;
                case object_view_rep::pointer:
                    return &object_;
                case object_view_rep::reference:
                    return object_;
                case object_view_rep::pointer_with_reference:
                    return object_;
            }
            return nullptr;
        }

        RAINY_NODISCARD bool is_any() const noexcept {
            return is_any_;
        }

    private:
        void *object_;
        implements::object_view_rep rep{implements::object_view_rep::normal};
        const foundation::rtti::typeinfo *rtti_{&rainy_typeid(void)};
        bool is_any_{false};
    };
}

#endif