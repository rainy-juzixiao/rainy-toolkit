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

namespace rainy::meta::reflection {
    // 用于表示不存在的实例
    struct non_exists_instance_t {};

    static constexpr inline non_exists_instance_t non_exists_instance;

    class object_view {
    public:
        template <typename Ty>
        using enable_if_t = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Ty, object_view>, int>;

        template <typename Ty, enable_if_t<Ty> = 0>
        object_view(Ty *object) noexcept :
            object_{const_cast<type_traits::cv_modify::remove_cv_t<Ty> *>(object)}, rtti_{&rainy_typeid(Ty *)} {
            utility::expects(object != nullptr, "object cannot be a null pointer!");
        }

        template <typename Ty, enable_if_t<Ty> = 0>
        object_view(Ty &object) noexcept :
            object_{utility::addressof(const_cast<type_traits::cv_modify::remove_cv_t<Ty> &>(object))}, rtti_{&rainy_typeid(Ty)} {
        }

        object_view(const non_exists_instance_t &) noexcept : object_(nullptr), rtti_(&rainy_typeid(void)) {
        }

        object_view(object_view &&other) noexcept : object_(other.object_), rtti_(other.rtti_) {
            other.object_ = nullptr;
            other.rtti_ = &rainy_typeid(void);
        }

        object_view &operator=(object_view &&other) noexcept {
            if (this != &other) {
                object_ = utility::exchange(other.object_, nullptr);
                rtti_ = utility::exchange(other.rtti_, &rainy_typeid(void));
            }
            return *this;
        }

        object_view(const object_view &right) = default;
        object_view &operator=(const object_view &) = default;

        object_view() = delete;
        object_view(std::nullptr_t) = delete;
        object_view &operator=(std::nullptr_t) = delete;

        template <typename Ty, enable_if_t<Ty> = 0>
        RAINY_NODISCARD Ty *cast_to_pointer() noexcept {
            using TypeNoRef = std::remove_reference_t<Ty>;
            return const_cast<TypeNoRef *>(static_cast<const object_view *>(this)->cast_to_pointer<TypeNoRef>());
        }

        template <typename Ty, enable_if_t<Ty> = 0>
        RAINY_NODISCARD const Ty *cast_to_pointer() const noexcept {
            using TypeNoRef = std::remove_reference_t<Ty>;
            rainy_assume(rtti_ != nullptr);
            if (!valid()) {
                return nullptr;
            }
            return static_cast<const TypeNoRef *>(object_);
        }

        template <typename Ty, enable_if_t<Ty> = 0>
        RAINY_NODISCARD auto as() -> std::conditional_t<std::is_const_v<Ty>, const Ty &, Ty &> {
            using TypeNoRef = std::remove_reference_t<Ty>;
            const TypeNoRef *ptr = cast_to_pointer<TypeNoRef>();
            utility::ensures(ptr != nullptr, "Cannot dereference an invalid instance");

            if constexpr (std::is_const_v<Ty>) {
                return *ptr; // const返回const引用
            } else {
                return *const_cast<TypeNoRef *>(ptr); // 非const返回非const引用
            }
        }

        template <typename Ty, enable_if_t<Ty> = 0>
        RAINY_NODISCARD auto as() const -> std::conditional_t<std::is_const_v<Ty>, const Ty &, Ty &> {
            using TypeNoRef = std::remove_reference_t<Ty>;
            const TypeNoRef *ptr = cast_to_pointer<TypeNoRef>();
            utility::ensures(ptr != nullptr, "Cannot dereference an invalid instance");

            if constexpr (std::is_const_v<Ty>) {
                return *ptr; // const返回const引用
            } else {
                return *const_cast<TypeNoRef *>(ptr); // 非const返回非const引用
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

    private:
        void *object_;
        const foundation::rtti::typeinfo *rtti_{&rainy_typeid(void)};
    };
#undef RAINY_OBJECT_VIEW_CAST_TO_DECLARATION
}

#endif