#ifndef RAINY_META_META_METHOD_HPP
#define RAINY_META_META_METHOD_HPP
#include <rainy/core/core.hpp>

namespace rainy::meta::method::internals {
    template <typename Ty, typename... Args>
    struct ctor_impl {
        static constexpr Ty invoke(Args... args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) {
            return Ty(utility::forward<Args>(args)...);
        }
    };

    template <typename Ty>
    struct dtor_impl {
        static RAINY_CONSTEXPR20 void invoke(const Ty *object) noexcept(std::is_nothrow_destructible_v<Ty>) {
            if (object) {
                object->~Ty();
            }
        }
    };
}

namespace rainy::meta::method {
    template <typename Ty, typename... Args>
    struct ctor : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>,
                                                        internals::ctor_impl<Ty, Args...>> {};

    template <typename Ty>
    struct dtor
        : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, internals::dtor_impl<Ty>> {};
}

namespace rainy::meta::method {
    template <typename Ty, typename... Args,
              typename type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
    constexpr auto get_ctor_fn() {
        return &ctor<Ty, Args...>::invoke;
    }

    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, int> = 0>
    constexpr auto get_dtor_fn() {
        return &dtor<Ty>::invoke;
    }
}

namespace rainy::meta::method::internals {
    template <typename Ty, typename Assign>
    struct assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            Assign assign) noexcept(type_traits::type_properties::is_nothrow_assignable_v<Ty, Assign>) {
            return (this_ = assign);
        }
    };

    template <typename Ty>
    struct copy_assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            type_traits::reference_modify::add_rvalue_reference_t<Ty>
                &&rvalue) noexcept(type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>) {
            return (this_ = rvalue);
        }
    };

    template <typename Ty>
    struct move_assign_impl {
        static constexpr type_traits::reference_modify::add_lvalue_reference_t<Ty> invoke(
            type_traits::reference_modify::add_lvalue_reference_t<Ty> this_,
            type_traits::reference_modify::add_rvalue_reference_t<Ty>
                rvalue) noexcept(type_traits::type_properties::is_nothrow_move_assignable_v<Ty>) {
            return (this_ = utility::move(rvalue));
        }
    };
}

namespace rainy::meta::method {
    template <typename Ty, typename Assign>
    struct assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                              !type_traits::type_properties::is_const_v<Ty> &&
                                                              !type_traits::composite_types::is_reference_v<Ty>,
                                                          internals::assign_impl<Ty, Assign>> {};

    template <typename Ty>
    struct copy_assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                               internals::copy_assign_impl<Ty>> {};

    template <typename Ty>
    struct move_assign : type_traits::other_trans::conditional_t<
                             type_traits::type_properties::is_move_assignable_v<Ty>, internals::move_assign_impl<Ty>,
                             type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                                   internals::copy_assign_impl<Ty>>> {};
}

namespace rainy::meta::method {
    template <typename Ty, typename Assign,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty, Assign> &&
                                                        !type_traits::type_properties::is_const_v<Ty> &&
                                                        !type_traits::composite_types::is_reference_v<Ty>,
                                                    int> = 0>
    constexpr auto get_assign() {
        return &assign<Ty, Assign>::invoke;
    }

    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty> ||
                                                                     type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>,
                                                                 int> = 0>
    constexpr auto get_move_assign() {
        return &move_assign<Ty>::invoke;
    }

    template <typename Ty,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>, int> = 0>
    constexpr auto get_copy_assign() {
        return &copy_assign<Ty>::invoke;
    }
}

#endif