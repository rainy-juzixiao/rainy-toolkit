#ifndef RAINY_META_META_METHOD_HPP
#define RAINY_META_META_METHOD_HPP
#include <rainy/core/core.hpp>

namespace rainy::meta::method::implements {
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
                                                        implements::ctor_impl<Ty, Args...>> {};

    template <typename Ty>
    struct dtor
        : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_destructible_v<Ty>, implements::dtor_impl<Ty>> {};
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

namespace rainy::meta::method::implements {
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
                                                          implements::assign_impl<Ty, Assign>> {};

    template <typename Ty>
    struct copy_assign : type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                               implements::copy_assign_impl<Ty>> {};

    template <typename Ty>
    struct move_assign : type_traits::other_trans::conditional_t<
                             type_traits::type_properties::is_move_assignable_v<Ty>, implements::move_assign_impl<Ty>,
                             type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_assignable_v<Ty>,
                                                                   implements::copy_assign_impl<Ty>>> {};
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

namespace rainy::meta::method::cpp_methods {
    static constexpr std::string_view method_operator_add = "operator+";
    static constexpr std::string_view method_operator_sub = "operator-";
    static constexpr std::string_view method_operator_mul = "operator*";
    static constexpr std::string_view method_operator_div = "operator/";
    static constexpr std::string_view method_operator_mod = "operator%";
    static constexpr std::string_view method_operator_eq = "operator==";
    static constexpr std::string_view method_operator_neq = "operator!=";
    static constexpr std::string_view method_operator_lt = "operator<";
    static constexpr std::string_view method_operator_gt = "operator>";
    static constexpr std::string_view method_operator_le = "operator<=";
    static constexpr std::string_view method_operator_ge = "operator>=";
    static constexpr std::string_view method_operator_assign = "operator=";
    static constexpr std::string_view method_operator_index = "operator[]";
    static constexpr std::string_view method_operator_call = "operator()";
    static constexpr std::string_view method_operator_arrow = "operator->";
    static constexpr std::string_view method_operator_deref = "operator*";
    static constexpr std::string_view method_operator_addr = "operator&";
    static constexpr std::string_view method_operator_preinc = "operator++";
    static constexpr std::string_view method_operator_postinc = "operator++(int)";
    static constexpr std::string_view method_operator_predec = "operator--";
    static constexpr std::string_view method_operator_postdec = "operator--(int)";
    static constexpr std::string_view method_operator_or = "operator||";
    static constexpr std::string_view method_operator_and = "operator&&";
    static constexpr std::string_view method_operator_not = "operator!";
    static constexpr std::string_view method_operator_bit_or = "operator|";
    static constexpr std::string_view method_operator_bit_and = "operator&";
    static constexpr std::string_view method_operator_bit_xor = "operator^";
    static constexpr std::string_view method_operator_bit_not = "operator~";
    static constexpr std::string_view method_operator_shift_l = "operator<<";
    static constexpr std::string_view method_operator_shift_r = "operator>>";
    static constexpr std::string_view method_begin = "begin";
    static constexpr std::string_view method_end = "end";
    static constexpr std::string_view method_cbegin = "cbegin";
    static constexpr std::string_view method_cend = "cend";
    static constexpr std::string_view method_rbegin = "rbegin";
    static constexpr std::string_view method_rend = "rend";
    static constexpr std::string_view method_size = "size";
    static constexpr std::string_view method_empty = "empty";
    static constexpr std::string_view method_clear = "clear";
    static constexpr std::string_view method_push_back = "push_back";
    static constexpr std::string_view method_pop_back = "pop_back";
    static constexpr std::string_view method_length = "length";
    static constexpr std::string_view method_insert = "insert";
    static constexpr std::string_view method_erase = "erase";
    static constexpr std::string_view method_find = "find";
    static constexpr std::string_view method_resize = "resize";
    static constexpr std::string_view method_swap = "swap";
    static constexpr std::string_view method_at = "at";
    static constexpr std::string_view method_front = "front";
    static constexpr std::string_view method_back = "back";
    static constexpr std::string_view method_append = "append";
}

#endif