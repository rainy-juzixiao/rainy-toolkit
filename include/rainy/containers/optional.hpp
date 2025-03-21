#ifndef RAINY_OPTIONAL_H
#define RAINY_OPTIONAL_H
#include <rainy/core/core.hpp>
#include <rainy/meta/type_traits.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <optional>

namespace rainy::containers {
    struct nullopt_t {
        struct tag {};
        constexpr explicit nullopt_t(tag) {
        }
    };

    inline constexpr nullopt_t nullopt{nullopt_t::tag{}};

    template <typename Ty>
    class optional;
}

namespace rainy::containers::internals {
    template <typename Ty>
    class optional_base {
    protected:
        using std_type = std::optional<Ty>;

        constexpr optional_base() noexcept : dummy({}), has_value_(false) {
        }

        template <typename U>
        using _allow_conv = type_traits::other_trans::enable_if_t<
            type_traits::type_properties::is_constructible_v<Ty, U &&> &&
            !(type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, utility::placeholder> ||
              type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, utility::placeholder>) &&!type_traits::
                type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, optional<Ty>> &&
            !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<U>, std_type>>;

        template <typename... Args>
        explicit optional_base(Args &&...val) : value(utility::forward<Args>(val)...), has_value_(true) {
        }

        ~optional_base() {
            reset_();
        };

        template <typename... Args>
        Ty &construct_(Args &&...val) {
            utility::expects(!this->has_value_);
            utility::construct_in_place(value, utility::forward<Args>(val)...);
            has_value_ = true;
            return this->value;
        }

        void reset_() {
            if (has_value_) {
                value.~Ty();
                has_value_ = false;
            }
        }

    private:
        union {
            utility::placeholder dummy;
            type_traits::cv_modify::remove_cv_t<Ty> value;
        };
        bool has_value_;
    };
}


namespace rainy::containers {
    template <typename Ty>
    class optional : internals::optional_base<Ty> {
    public:
        using base_ = internals::optional_base<Ty>;
        using std_type = std::optional<Ty>;

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
        constexpr explicit optional(utility::placeholder, Args &&...) {
        }

        template <typename U, typename... Args>
        constexpr explicit optional(utility::placeholder, std::initializer_list<U>, Args &&...) {
        }

        template <typename U = Ty>
        constexpr
#if RAINY_HAS_CXX20
            explicit(!type_traits::type_relations::is_convertible_v<U, Ty>)
#endif
                optional(U &&val) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, U>) :
            base_(utility::forward<U>(val)) {
        }

        /*template <class U>
        explicit optional(const optional<U> &) {

        }

        template <class U>
        explicit(see below) optional(optional<U> &&) {

        }*/

        ~optional() = default;
    };
}


#endif
