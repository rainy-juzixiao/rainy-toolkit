#ifndef RAINY_OPTIONAL_H
#define RAINY_OPTIONAL_H
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <optional>

namespace rainy::collections {
    struct nullopt_t {
        struct tag {};
        constexpr explicit nullopt_t(tag) {
        }
    };

    inline constexpr nullopt_t nullopt{nullopt_t::tag{}};

    template <typename Ty>
    class optional;
}

namespace rainy::collections::implements {
    template <typename Ty>
    class optional_base {
    public:
        template <typename U>
        using enable_if_t = type_traits::other_trans::enable_if_t<
            type_traits::type_properties::is_constructible_v<Ty, U &&> &&
                !(type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, utility::placeholder_t> ||
                  type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, utility::placeholder_t>) &&
                !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, optional<Ty>>,
            int>;

        constexpr bool has_value() const noexcept {
            return this->has_value_;
        }

        template <typename UTy, enable_if_t<UTy> = 0>
        constexpr void assign(UTy&& right) {
            if (this->has_value()) {
                static_cast<Ty &>(value_) = utility::forward<UTy>(right);
            } else {
                this->construct_(utility::forward<UTy>(right));
            }
        }

        RAINY_NODISCARD constexpr Ty &operator*() & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr const Ty &operator*() const & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr Ty &&operator*() && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        RAINY_NODISCARD constexpr const Ty &&operator*() const && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        RAINY_NODISCARD constexpr Ty &value() & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr const Ty &value() const & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr Ty &&value() && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        RAINY_NODISCARD constexpr const Ty &&value() const && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

    protected:
        template <typename... Args>
        explicit constexpr optional_base(Args &&...val) : value_{utility::forward<Args>(val)...}, has_value_(true) {
        }

        constexpr ~optional_base() {
            reset_();
        };

        template <typename... Args>
        constexpr Ty &construct_(Args &&...val) {
            utility::expects(!this->has_value_);
            utility::construct_in_place(value_, utility::forward<Args>(val)...);
            has_value_ = true;
            return this->value_;
        }

        constexpr void reset_() {
            if constexpr (!type_traits::type_properties::is_trivially_destructible_v<Ty>) {
                value_.~Ty();
            }
            has_value_ = false;
        }

    private:
        union {
            utility::placeholder_t dummy;
            type_traits::cv_modify::remove_cv_t<Ty> value_;
        };
        bool has_value_;
    };
}


namespace rainy::collections {
    template <typename Ty>
    class optional : public implements::optional_base<Ty> {
    public:
        using base = implements::optional_base<Ty>;

        /*
        is_constructible_v<T,
 U&&> is true, is_same_v<remove_cvref_t<U>, in_place_t> is false, and is_same_v<remove_
cvref_t<U>, optional> is false
        */

        constexpr optional() noexcept = default;

        constexpr explicit optional(std::nullopt_t) noexcept {
        }

        constexpr optional(const optional &right) {
        }

        constexpr optional(optional &&right) noexcept {
        }

        template <typename... Args>
        constexpr explicit optional(utility::placeholder_t, Args &&...) {
        }

        template <typename U, typename... Args>
        constexpr explicit optional(utility::placeholder_t, std::initializer_list<U>, Args &&...) {
        }

        template <typename U = Ty>
        constexpr
#if RAINY_HAS_CXX20
            explicit(!type_traits::type_relations::is_convertible_v<U, Ty>)
#endif
                optional(U &&val) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, U>) :
            base(utility::forward<U>(val)) {
        }

        /*template <class U>
        explicit optional(const optional<U> &) {

        }

        template <class U>
        explicit(see below) optional(optional<U> &&) {

        }*/

        ~optional() = default;

        template <typename UTy, typename = base::enable_if_t<UTy>>
        RAINY_CONSTEXPR20 optional &operator=(UTy &&right) {
            this->assign(utility::forward<UTy>(right));
            return *this;
        }

        RAINY_CONSTEXPR20 void swap(optional &right) noexcept(type_traits::type_properties::is_nothrow_move_constructible_v<Ty> &&
                                                              type_traits::type_properties::is_nothrow_swappable_v<Ty>) {
            if constexpr (type_traits::type_properties::is_move_constructible_v<Ty>) {
                static_assert(type_traits::type_properties::is_swappable_v<Ty>, "optional<T>::swap requires T to be swappable");
            } else {
                static_assert(false, "optional<T>::swap requires T to be move constructible");
            }
            using std::swap;
            const bool engaged = this->has_value();
            if (engaged && right.has_value()) {
                if (engaged) {
                    swap(*(*this), *right);
                }
            } else {
                optional &source = engaged ? *this : right;
                optional &target = engaged ? right : *this;
                target.construct_(utility::move(*source));
                source.reset_();
            }
        }
    };
}

#endif
