#ifndef RAINY_VARIANT_HPP
#define RAINY_VARIANT_HPP
#include <rainy/base.hpp>
#include <rainy/meta/type_traits.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
/*
此处拒绝MSVC的C26495警告-> Variable 'variable' is uninitialized. Always initialize a member variable (type.6).
https://learn.microsoft.com/en-us/cpp/code-quality/c26495
*/
#endif

namespace rainy::containers {
    template <typename... Types>
    class variant; // 前置声明
}

namespace rainy::containers::internals {
    template <bool TrivialTestruction, typename... Types>
    struct variant_storage {};

    template <typename First, typename... Rest>
    struct variant_storage<true, First, Rest...> {
        template <typename... Types>
        using variant_storage_type =
            variant_storage<type_traits::logical_traits::conjunction_v<std::is_trivially_destructible<Types>...>, Types...>;

        using first_t = First;

        template <std::size_t Idx>
        using idx_t = type_traits::helper::integral_constant<std::size_t, Idx>;

        static constexpr std::size_t size = 1 + sizeof...(Rest);

        constexpr variant_storage() noexcept {
        }

        RAINY_CONSTEXPR20 ~variant_storage() noexcept {
        }

        constexpr variant_storage(variant_storage &&) noexcept = default;
        constexpr variant_storage(const variant_storage &) noexcept = default;
        constexpr variant_storage &operator=(const variant_storage &) noexcept = default;
        constexpr variant_storage &operator=(variant_storage &&) noexcept = default;

        template <typename... Types>
        constexpr explicit variant_storage(idx_t<0>, Types &&...args) : head(utility::forward<Types>(args)...) {
        }

        template <std::size_t Idx, typename... Types, type_traits::other_trans::enable_if_t<(Idx > 0), int> = 0>
        constexpr explicit variant_storage(idx_t<Idx>, Types &&...args) : tail(idx_t<Idx - 1>{}, utility::forward<Types>(args)...) {
        }

        constexpr first_t &get() noexcept {
            return head;
        }

        constexpr const first_t &get() const noexcept {
            return head;
        }

        template <std::size_t Idx, typename VariantStorage>
        friend constexpr decltype(auto) variant_raw_get(VariantStorage &&obj) noexcept;

        union {
            type_traits::cv_modify::remove_cv_t<first_t> head;
            variant_storage_type<Rest...> tail;
        };
    };

    template <typename First, typename... Rest>
    struct variant_storage<false, First, Rest...> {
        template <typename... Types>
        using variant_storage_type =
            variant_storage<type_traits::logical_traits::conjunction_v<std::is_trivially_destructible<Types>...>, Types...>;

        using first_t = First;

        template <std::size_t Idx>
        using idx_t = type_traits::helper::integral_constant<std::size_t, Idx>;

        template <typename... Types>
        class variant_base;

        static constexpr std::size_t size = 1 + sizeof...(Rest);

        constexpr variant_storage() noexcept {
        }

        RAINY_CONSTEXPR20 ~variant_storage() noexcept {
        }

        constexpr variant_storage(variant_storage &&) noexcept = default;
        constexpr variant_storage(const variant_storage &) noexcept = default;
        constexpr variant_storage &operator=(const variant_storage &) noexcept = default;
        constexpr variant_storage &operator=(variant_storage &&) noexcept = default;

        template <typename... Types>
        constexpr explicit variant_storage(idx_t<0>, Types &&...args) : head(utility::forward<Types>(args)...) {
        }

        template <std::size_t Idx, typename... Types, type_traits::other_trans::enable_if_t<(Idx > 0), int> = 0>
        constexpr explicit variant_storage(idx_t<Idx>, Types &&...args) : tail(idx_t<Idx - 1>{}, utility::forward<Types>(args)...) {
        }

        constexpr first_t &get() & noexcept {
            return head;
        }

        constexpr const first_t &get() const & noexcept {
            return head;
        }

        constexpr first_t &&get() && noexcept {
            return utility::move(head);
        }

        constexpr const first_t &&get() const && noexcept {
            return utility::move(head);
        }

        template <std::size_t Idx, typename VariantStorage>
        friend constexpr decltype(auto) variant_raw_get(VariantStorage &&obj) noexcept;

        union {
            type_traits::cv_modify::remove_cv_t<first_t> head;
            variant_storage_type<Rest...> tail;
        };
    };

    template <typename... Types>
    using variant_storage_t =
        variant_storage<type_traits::logical_traits::conjunction_v<std::is_trivially_destructible<Types>...>, Types...>;

    template <std::size_t Idx, typename VariantStorage>
    constexpr decltype(auto) variant_raw_get(VariantStorage &&obj) noexcept {
        if constexpr (Idx == 0) {
            return static_cast<VariantStorage &&>(obj).get();
        } else {
            return variant_raw_get<Idx - 1>(static_cast<VariantStorage &&>(obj).tail);
        }
    }

#if RAINY_HAS_CXX20
    template <std::size_t Index, class TargetType>
    auto construct_array(TargetType (&&)[1])
        -> type_traits::extras::tuple_like::meta_list<type_traits::helper::integral_constant<std::size_t, Index>, TargetType> {
    }

    // Resolves the variant type based on the initializer type.
    template <std::size_t Index, class TargetType, class InitializerType>
    using variant_type_resolver = decltype(construct_array<Index, TargetType>({utility::declval<InitializerType>()}));
#endif

    template <std::size_t Index, typename TargetType>
    struct variant_init_single_overload {
#if RAINY_HAS_CXX20
        // Operator for handling the initialization with an initializer type when C++20 is enabled.
        template <typename InitializerType>
        auto operator()(TargetType, InitializerType &&) -> variant_type_resolver<Index, TargetType, InitializerType> {
        }
#else // C++20 not enabled
      // Operator for handling the initialization without C++20.
        template <typename InitializerType>
        auto operator()(TargetType, InitializerType &&)
            -> type_traits::extras::tuple_like::meta_list<type_traits::helper::integral_constant<size_t, Index>, TargetType> {
        }
#endif
    };

    template <typename Indices, typename... Types>
    struct variant_init_overload_set_;

    template <size_t... Indices, typename... Types>
    struct variant_init_overload_set_<std::index_sequence<Indices...>, Types...> : variant_init_single_overload<Indices, Types>... {
        using variant_init_single_overload<Indices, Types>::operator()...;
    };

    template <typename... Types>
    using variant_init_overload_set = variant_init_overload_set_<std::index_sequence_for<Types...>, Types...>;

    template <typename Enable, typename Ty, typename... Types>
    struct variant_init_helper {}; // failure case (has no member "type")

    template <typename Ty, typename... Types>
    struct variant_init_helper<std::void_t<decltype(variant_init_overload_set<Types...>{}(utility::declval<Ty>(), utility::declval<Ty>()))>,
                               Ty, Types...> {

        using type = decltype(variant_init_overload_set<Types...>{}(utility::declval<Ty>(), utility::declval<Ty>()));
    };

    template <typename Ty, typename... Types>
    using variant_init_type =
        type_traits::extras::tuple_like::meta_front<type_traits::extras::tuple_like::meta_pop_front<typename variant_init_helper<void, Ty, Types...>::type>>;

    template <typename Ty, typename... Types>
    using variant_init_index = type_traits::extras::tuple_like::meta_front<typename variant_init_helper<void, Ty, Types...>::type>;

    template <typename... Types>
    class variant_base : private variant_storage_t<Types...> {
    public:
        RAINY_NODISCARD constexpr bool valueless_by_exception() const noexcept {
            return idx < 0;
        }

        RAINY_NODISCARD constexpr std::size_t index() const noexcept {
            return static_cast<std::size_t>(idx);
        }

    private:
        using storage_t = variant_storage_t<Types...>;

        template <typename Visitor, typename Variant, std::size_t Index>
        friend constexpr void visit_impl(Visitor &&visitor, Variant &&var);

        static inline constexpr size_t schar_max_as_size = static_cast<unsigned char>(-1) / 2;
        static inline constexpr size_t short_max_as_size = static_cast<unsigned short>(-1) / 2;

        using index_t = std::conditional_t<(sizeof...(Types) < schar_max_as_size), signed char,
                                           std::conditional_t<(sizeof...(Types) < short_max_as_size), short, int>>;

        friend class variant<Types...>;

        static inline constexpr std::size_t invalid_index = static_cast<index_t>(-1);

        RAINY_CONSTEXPR20 variant_base() noexcept : storage_t{}, idx(invalid_index) {
        }

        template <std::size_t Idx, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<
                          type_traits::extras::tuple_like::meta_at_c_t<Idx, variant_base<Types...>>, Args...>,
                      int> = 0>
        constexpr explicit variant_base(utility::placeholder_index_t<Idx>, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<
                type_traits::extras::tuple_like::meta_at_c_t<Idx, variant_base<Types...>>, Args...>) :
            storage_t(type_traits::helper::integral_constant<size_t, Idx>{}, utility::forward<Args>(args)...),
            idx(static_cast<index_t>(Idx)) {
        }

        template <std::size_t Idx, typename VariantStorage>
        friend constexpr decltype(auto) variant_raw_get(VariantStorage &&obj) noexcept;

        RAINY_CONSTEXPR20 void set_index(const std::size_t index) noexcept {
            this->idx = static_cast<index_t>(index);
        }

        RAINY_NODISCARD constexpr storage_t &storage() & noexcept {
            return *this;
        }

        RAINY_NODISCARD constexpr const storage_t &storage() const & noexcept {
            return *this;
        }

        RAINY_NODISCARD constexpr storage_t &&storage() && noexcept {
            return utility::move(*this);
        }

        RAINY_NODISCARD constexpr const storage_t &&storage() const && noexcept {
            return utility::move(*this);
        }

        constexpr void destroy() {
        }

        index_t idx;
    };

    template <typename Visitor, typename Variant, std::size_t Index = 0>
    constexpr void visit_impl(Visitor &&visitor, Variant &&var) {
        if constexpr (Index == std::decay_t<Variant>::storage_t::size) {
            foundation::system::exceptions::logic::throw_logic_error("Invalid index for variant visitation");
        } else {
            if (var.index() == Index) {
                std::forward<Visitor>(visitor)(variant_raw_get<Index>(std::forward<Variant>(var)));
            } else {
                visit_impl<Visitor, Variant, Index + 1>(std::forward<Visitor>(visitor), std::forward<Variant>(var));
            }
        }
    }
}

namespace rainy::containers {
    template <typename... Types>
    class variant : public internals::variant_base<Types...> {
    public:
        using base = internals::variant_base<Types...>; // 引用，此处忽略
        template <typename Ty>
        using cv_modify_helper = type_traits::cv_modify::cv_modify_helper<Ty>; // 引用，此处忽略

        template <typename First = type_traits::extras::tuple_like::meta_at_c_t<0, variant>,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<First>, int> = 0>
        constexpr variant() : base(utility::placeholder_index<0>) {
        }

        template <
            typename Ty,
            type_traits::other_trans::enable_if_t<
                sizeof...(Types) != 0 && !type_traits::type_relations::is_same_v<typename cv_modify_helper<Ty>::remove_cvref, variant> &&
                    !type_traits::primary_types::is_specialization_v<typename cv_modify_helper<Ty>::remove_cvref,
                                                                     utility::placeholder_type_t> &&
                    !type_traits::extras::tuple_like::is_in_placeholder_index_specialization<typename cv_modify_helper<Ty>::remove_cvref> &&
                    type_traits::type_properties::is_constructible_v<internals::variant_init_type<Ty, Types...>, Ty>,
                int> = 0>
        constexpr variant(Ty &&object) noexcept(std::is_nothrow_constructible_v<internals::variant_init_type<Ty, Types...>, Ty>) :
            base(utility::placeholder_index<internals::variant_init_index<Ty, Types...>::value>, object) {
        }

        template <typename Ty, typename... Args, typename Idx = type_traits::extras::tuple_like::meta_find_unique_index<Ty, variant>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr explicit variant(std::in_place_type_t<Ty>, Args &&...args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        }

        template <typename Ty, typename... Args, typename Idx = type_traits::extras::tuple_like::meta_find_unique_index<Ty, variant>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr explicit variant(utility::placeholder_type_t<Ty>, Args &&...args) noexcept(std::is_nothrow_constructible_v<Ty,Args...>) :
            base(utility::placeholder_index<Idx::value>, utility::forward<Args>(args)...) {
        }

        template <std::size_t Idx, typename... Args, typename Type = type_traits::extras::tuple_like::meta_at_c_t<Idx, variant>,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type, Args...> &&
                                                                      type_traits::extras::tuple_like::meta_find_unique_index<
                                                                          Type, variant>::value != base::invalid_index /* 防止类型双关 */,
                                                                  int> = 0>
        constexpr explicit variant(std::in_place_index_t<Idx>, Args &&...args) noexcept(std::is_nothrow_constructible_v<Type, Args...>) :
            base(utility::placeholder_index<Idx>, utility::forward<Args>(args)...) {
        }

        template <std::size_t Idx, typename... Args, typename Type = type_traits::extras::tuple_like::meta_at_c_t<Idx, variant>,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type, Args...> &&
                                                                      type_traits::extras::tuple_like::meta_find_unique_index<
                                                                          Type, variant>::value != base::invalid_index /* 防止类型双关 */,
                                                                  int> = 0>
        constexpr explicit variant(utility::placeholder_index_t<Idx>,
                                   Args &&...args) noexcept(std::is_nothrow_constructible_v<Type, Args...>) :
            base(utility::placeholder_index<Idx>, utility::forward<Args>(args)...) {
        }
    };

    template <typename Ty>
    struct variant_size; // undefined

    template <typename Ty>
    struct variant_size<const Ty> : variant_size<Ty>::type {};

    template <typename... Types>
    struct variant_size<variant<Types...>> : type_traits::helper::integral_constant<std::size_t, sizeof...(Types)> {};

    template <typename Ty>
    constexpr std::size_t variant_size_v = variant_size<Ty>::value;

    template <typename Visitor, typename... Variants>
    constexpr void visit(Visitor &&vis, Variants &&...vars) {
        (internals::visit_impl(vis, utility::forward<Variants>(vars)), ...);
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif