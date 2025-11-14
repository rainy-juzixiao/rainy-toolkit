#ifndef RAINY_CORE_TMP_META_METHOD_HPP
#define RAINY_CORE_TMP_META_METHOD_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/tmp/implements.hpp>

namespace rainy::type_traits::extras::meta_method {
    template <typename Ty, typename = void>
    struct try_to_invoke_begin {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find begin method! "
                                                                             "rainy::utility::begin"
                                                                             "only support begin() in Container Type"
                                                                             "please add begin() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_begin<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().begin())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(Ty &container) noexcept(noexcept(container.begin())) -> decltype(container.begin()) {
            return container.begin();
        }
    };

    template <typename, typename = void>
    struct try_to_invoke_end {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find end method! "
                                                                             "rainy::utility::end"
                                                                             "only support end() in Container Type"
                                                                             "please add end() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_end<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().end())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(Ty &container) noexcept(noexcept(container.end())) -> decltype(container.end()) {
            return container.end();
        }
    };

    template <typename Ty, typename = void>
    struct try_to_invoke_cbegin {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find begin method! "
                                                                             "rainy::utility::cbegin"
                                                                             "only support cbegin() in Container Type"
                                                                             "please add cbegin() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_cbegin<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().cbegin())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(const Ty &container) noexcept(noexcept(container.cbegin())) -> decltype(container.cbegin()) {
            return container.cbegin();
        }
    };

    template <typename, typename = void>
    struct try_to_invoke_cend {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find end method! "
                                                                             "rainy::utility::cend"
                                                                             "only support cend() in Container Type"
                                                                             "please add cend() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_cend<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().cend())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(const Ty &container) noexcept(noexcept(container.cend())) -> decltype(container.cend()) {
            return container.cend();
        }
    };
    
    template <typename Ty, typename = void>
    struct try_to_invoke_size {
        static RAINY_CONSTEXPR_BOOL value = false;

        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find begin method! "
                                                                             "rainy::utility::begin"
                                                                             "only support begin() in Container Type"
                                                                             "please add begin() method in Container Definition");
        }
    };

    template <typename Ty>
    struct try_to_invoke_size<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().size())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        static auto invoke(const Ty &container) noexcept(noexcept(container.size())) -> decltype(container.size()) {
            return container.size();
        }
    };
}

namespace rainy::utility {
    template <typename Container>
    RAINY_NODISCARD constexpr auto begin(Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto begin(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont);
    }

    template <typename Container, std::size_t N>
    static auto begin(Container (&container)[N]) noexcept {
        return container;
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto end(Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto end(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont);
    }

    template <typename Container, std::size_t N>
    static auto end(Container (&container)[N]) noexcept {
        return container + N;
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto cbegin(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto cend(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont);
    }

    template <typename Container>
    RAINY_NODISCARD constexpr auto size(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_size<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_size<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_size<const Container>::invoke(cont);
    }
}

/* 元方法Trait */
namespace rainy::type_traits::extras::meta_method {
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_iterator_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_iterator_v<
        Ty,
        type_traits::other_trans::void_t<decltype(utility::begin(utility::declval<Ty &>()) != utility::end(utility::declval<Ty &>()),
                                                  ++utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>(),
                                                  --utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>(),
                                                  (void) utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>()++,
                                                  (void) utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>()--,
                                                  *utility::begin(utility::declval<Ty &>()))>> = true;
    template <typename Ty>
    struct has_iterator : helper::bool_constant<has_iterator_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_add_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_add_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() + utility::declval<const Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_add : helper::bool_constant<has_operator_add_v<Ty>> {};
    
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_addition_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_addition_v<Ty, other_trans::void_t<decltype(utility::declval<Ty &>() + utility::declval<std::ptrdiff_t>())>> = true;

    template <typename Ty>
    struct has_operator_addition : helper::bool_constant<has_operator_addition_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_subtraction_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_subtraction_v<Ty, other_trans::void_t<decltype(utility::declval<Ty &>() - utility::declval<std::ptrdiff_t>())>> =
            true;

    template <typename Ty>
    struct has_operator_subtraction : helper::bool_constant<has_operator_subtraction_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_plus_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_plus_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() += utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_plus_equal : helper::bool_constant<has_operator_plus_equal_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_sub_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_sub_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() - utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_sub : helper::bool_constant<has_operator_sub_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_sub_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_sub_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() -= utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_sub_equal : helper::bool_constant<has_operator_plus_equal_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mul_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_mul_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() * utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_mul : helper::bool_constant<has_operator_mul_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mul_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_mul_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() *= utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_mul_equal : helper::bool_constant<has_operator_mul_equal_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_div_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_div_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() / utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_div : helper::bool_constant<has_operator_div_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_div_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_div_equal_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() /= utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_div_equal : helper::bool_constant<has_operator_div_equal_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mod_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_mod_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() % utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_mod : helper::bool_constant<has_operator_mod_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mod_equal_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_mod_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() %= utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_mod_equal : helper::bool_constant<has_operator_mod_equal_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_eq_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_eq_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() == utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_eq : helper::bool_constant<has_operator_eq_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_neq_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_neq_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() != utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_neq : helper::bool_constant<has_operator_neq_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_lt_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_lt_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() < utility::declval<const Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_lt : helper::bool_constant<has_operator_lt_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_gt_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_gt_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() > utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_gt : helper::bool_constant<has_operator_gt_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_le_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_le_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() <= utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_le : helper::bool_constant<has_operator_le_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_ge_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_ge_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() >= utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_ge : helper::bool_constant<has_operator_ge_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_assign_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_assign_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() = utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_assign : helper::bool_constant<has_operator_assign_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_index_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_index_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()[std::declval<std::size_t>()])>> =
            true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_index_for_key_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_index_for_key_v<
        Ty, type_traits::other_trans::void_t<typename Ty::key_type,
                                             decltype(utility::declval<Ty &>()[std::declval<typename Ty::key_type>()])>> =
        true;
    
    template <typename Ty>
    struct has_operator_index : helper::bool_constant<has_operator_index_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_call_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_call_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()(std::declval<typename Ty::size_type>()))>> = true;

    template <typename Ty>
    struct has_operator_call : helper::bool_constant<has_operator_call_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_arrow_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_arrow_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().operator->())>> =
        true;

    template <typename Ty>
    struct has_operator_arrow : helper::bool_constant<has_operator_arrow_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_three_way_compare_v = false;

#if RAINY_HAS_CXX20
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_three_way_compare_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() <=> utility::declval<Ty &>())>> = true;
#endif

    template <typename Ty>
    struct has_operator_three_way_compare : helper::bool_constant<has_operator_three_way_compare_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_deref_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_deref_v<Ty, type_traits::other_trans::void_t<decltype(*utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_deref : helper::bool_constant<has_operator_deref_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_addr_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_addr_v<Ty, type_traits::other_trans::void_t<decltype(&utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_addr : helper::bool_constant<has_operator_addr_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_preinc_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_preinc_v<Ty, type_traits::other_trans::void_t<decltype(++utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_preinc : helper::bool_constant<has_operator_preinc_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_predec_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_predec_v<Ty, type_traits::other_trans::void_t<decltype(--utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_predec : helper::bool_constant<has_operator_predec_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_postinc_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_postinc_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()++)>> = true;

    template <typename Ty>
    struct has_operator_postinc : helper::bool_constant<has_operator_preinc_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_postdec_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_postdec_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()--)>> = true;

    template <typename Ty>
    struct has_operator_postdec : helper::bool_constant<has_operator_postdec_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_or_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_or_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() || utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_or : helper::bool_constant<has_operator_or_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_and_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_and_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() && utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_and : helper::bool_constant<has_operator_and_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_not_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_not_v<Ty, type_traits::other_trans::void_t<decltype(!utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_not : helper::bool_constant<has_operator_not_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_or_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_bit_or_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() | utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_bit_or : helper::bool_constant<has_operator_bit_or_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_and_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_bit_and_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() & utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_bit_and : helper::bool_constant<has_operator_bit_and_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_xor_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_bit_xor_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() ^ utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_bit_xor : helper::bool_constant<has_operator_bit_xor_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_not_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_bit_not_v<Ty, type_traits::other_trans::void_t<decltype(~utility::declval<Ty &>())>> = true;

    template <typename Ty>
    struct has_operator_bit_not : helper::bool_constant<has_operator_bit_not_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_shift_l_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_shift_l_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() << utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_shift_l : helper::bool_constant<has_operator_shift_l_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_shift_r_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_shift_r_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() >> utility::declval<Ty &>())>> =
            true;

    template <typename Ty>
    struct has_operator_shift_r : helper::bool_constant<has_operator_shift_r_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_begin_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_begin_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().begin())>> = true;

    template <typename Ty>
    struct has_begin : helper::bool_constant<has_begin_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_end_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_end_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().end())>> = true;

    template <typename Ty>
    struct has_end : helper::bool_constant<has_end_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_cbegin_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_cbegin_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().cbegin())>> = true;

    template <typename Ty>
    struct has_cbegin : helper::bool_constant<has_cbegin_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_cend_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_cend_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().cend())>> = true;

    template <typename Ty>
    struct has_cend : helper::bool_constant<has_cend_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_rbegin_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_rbegin_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().rbegin())>> = true;

    template <typename Ty>
    struct has_rbegin : helper::bool_constant<has_rbegin_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_rend_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_rend_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().rend())>> = true;

    template <typename Ty>
    struct has_rend : helper::bool_constant<has_rend_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_size_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_size_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().size())>> = true;

    template <typename Ty>
    struct has_size : helper::bool_constant<has_size_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_empty_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_empty_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().empty())>> = true;

    template <typename Ty>
    struct has_empty : helper::bool_constant<has_empty_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_clear_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_clear_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().clear())>> = true;

    template <typename Ty>
    struct has_clear : helper::bool_constant<has_clear_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_push_back_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_push_back_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().push_back(std::declval<typename Ty::value_type>()))>> =
        true;

    template <typename Ty>
    struct has_push_back : helper::bool_constant<has_push_back_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_emplace_back_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_emplace_back_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().emplace_back(std::declval<typename Ty::value_type>()))>> =
        true;

    template <typename Ty>
    struct has_emplace_back : helper::bool_constant<has_emplace_back_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_pop_back_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_pop_back_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().pop_back())>> = true;

    template <typename Ty>
    struct has_pop_back : helper::bool_constant<has_pop_back_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_pop_front_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_pop_front_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().pop_front())>> = true;

    template <typename Ty>
    struct has_pop_front : helper::bool_constant<has_pop_front_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_insert_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_insert_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().insert(
                             std::declval<typename Ty::const_iterator>(), std::declval<typename Ty::value_type>()))>> = true;
    template <typename Ty>
    struct has_insert : helper::bool_constant<has_insert_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_erase_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_erase_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().erase(std::declval<typename Ty::const_iterator>()))>> =
        true;
    template <typename Ty>
    struct has_erase : helper::bool_constant<has_erase_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_find_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_find_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().find(std::declval<typename Ty::value_type>()))>> = true;

    template <typename Ty>
    struct has_find : helper::bool_constant<has_find_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_resize_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_resize_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().resize(std::declval<typename Ty::size_type>()))>> =
        true;

    template <typename Ty>
    struct has_resize : helper::bool_constant<has_resize_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_front_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_front_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().front())>> = true;

    template <typename Ty>
    struct has_front : helper::bool_constant<has_front_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_back_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_back_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().back())>> = true;

    template <typename Ty>
    struct has_back : helper::bool_constant<has_back_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v<Ty, type_traits::other_trans::void_t<typename Ty::mapped_type>> = true;

    template <typename Ty>
    struct has_mapped_type : type_traits::helper::bool_constant<has_mapped_type_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_max_size_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_max_size_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().max_size())>> =
        true;

    template <typename Ty>
    struct has_max_size : type_traits::helper::bool_constant<has_max_size_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_data_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_data_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().data())>> = true;

    template <typename Ty>
    struct has_data : type_traits::helper::bool_constant<has_data_v<Ty>> {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_length_v = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_length_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().length())>> = true;

    template <typename Ty>
    struct has_length : type_traits::helper::bool_constant<has_length_v<Ty>> {};
}

#endif