#ifndef RAINY_VARIANT_HPP
#define RAINY_VARIANT_HPP
#include <rainy/core/core.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26495 4715)
#endif

namespace rainy::utility {
    template <typename... Types>
    class variant; // 前置声明

    RAINY_INLINE_CONSTEXPR std::size_t variant_npos = type_traits::other_trans::type_list_npos;

    template <typename Ty>
    struct variant_size; // undefined

    template <typename Ty>
    struct variant_size<const Ty> : variant_size<Ty>::type {};

    template <typename... Types>
    struct variant_size<variant<Types...>> : type_traits::helper::integral_constant<std::size_t, sizeof...(Types)> {};

    template <typename Ty>
    constexpr std::size_t variant_size_v = variant_size<Ty>::value;
}

namespace rainy::foundation::exceptions::runtime {
    class bad_variant_access : public runtime_error {
    public:
        bad_variant_access(const diagnostics::source_location &source) : runtime_error{"bad variant access", source} {
        }
    };

    void throw_bad_variant_access(const diagnostics::source_location &source = diagnostics::source_location::current()) {
        throw_exception(bad_variant_access{source});
    }
}

namespace rainy::utility::implements {
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

        template <std::size_t Idx, typename... Types, type_traits::other_trans::enable_if_t<Idx != 0, int> = 0>
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

        template <std::size_t Idx, typename... Types, type_traits::other_trans::enable_if_t<Idx != 0, int> = 0>
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
}

namespace rainy::utility::implements {
    template <std::size_t Idx, typename VariantStorage>
    constexpr decltype(auto) variant_raw_get(VariantStorage &&obj) noexcept {
        if constexpr (Idx == 0) {
            return static_cast<VariantStorage &&>(obj).get();
        } else if constexpr (Idx == 1) {
            return static_cast<VariantStorage &&>(obj).tail.get();
        } else if constexpr (Idx == 2) {
            return static_cast<VariantStorage &&>(obj).tail.tail.get();
        } else if constexpr (Idx == 3) {
            return static_cast<VariantStorage &&>(obj).tail.tail.tail.get();
        } else if constexpr (Idx == 4) {
            return static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.get();
        } else if constexpr (Idx == 5) {
            return static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.get();
        } else if constexpr (Idx == 6) {
            return static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.tail.get();
        } else if constexpr (Idx == 7) {
            return static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.tail.tail.get();
        } else if constexpr (Idx < 16) {
            return variant_rawget<Idx - 8>(static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.tail.tail.tail);
        } else if constexpr (Idx < 32) {
            return variant_rawget<Idx - 16>(
                static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail);
        } else if constexpr (Idx < 64) {
            return variant_rawget<Idx - 32>(static_cast<VariantStorage &&>(obj)
                                                .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail
                                                .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail);
        } else { // Idx >= 64
            return variant_rawget<Idx - 64>(
                static_cast<VariantStorage &&>(obj)
                    .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail
                    .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail
                    .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail);
        }
    }
}

namespace rainy::utility::implements {
#if RAINY_HAS_CXX20
    template <std::size_t Index, class TargetType>
    auto construct_array(TargetType (&&)[1])
        -> type_traits::other_trans::type_list<type_traits::helper::integral_constant<std::size_t, Index>, TargetType> {
    }

    template <std::size_t Index, class TargetType, class InitializerType>
    using variant_type_resolver = decltype(construct_array<Index, TargetType>({utility::declval<InitializerType>()}));
#endif

    template <std::size_t Index, typename TargetType>
    struct variant_init_single_overload {
#if RAINY_HAS_CXX20
        template <typename InitializerType>
        auto operator()(TargetType, InitializerType &&) -> variant_type_resolver<Index, TargetType, InitializerType> {
        }
#else
        template <typename InitializerType>
        auto operator()(TargetType, InitializerType &&)
            -> type_traits::other_trans::type_list<type_traits::helper::integral_constant<std::size_t, Index>, TargetType> {
        }
#endif
    };

    template <typename Indices, typename... Types>
    struct variant_init_overload_set_;

    template <std::size_t... Indices, typename... Types>
    struct variant_init_overload_set_<std::index_sequence<Indices...>, Types...> : variant_init_single_overload<Indices, Types>... {
        using variant_init_single_overload<Indices, Types>::operator()...;
    };

    template <typename... Types>
    using variant_init_overload_set = variant_init_overload_set_<std::index_sequence_for<Types...>, Types...>;

    template <typename Enable, typename Ty, typename... Types>
    struct variant_init_helper {};

    template <typename Ty, typename... Types>
    struct variant_init_helper<
        std::void_t<decltype(variant_init_overload_set<Types...>{}(utility::declval<Ty>(), utility::declval<Ty>()))>, Ty, Types...> {

        using type = decltype(variant_init_overload_set<Types...>{}(utility::declval<Ty>(), utility::declval<Ty>()));
    };

    template <typename Ty, typename... Types>
    using variant_init_type = typename type_traits::other_trans::type_list_front<
        typename type_traits::other_trans::type_list_pop_front<typename variant_init_helper<void, Ty, Types...>::type>::type>::type;

    template <typename Ty, typename... Types>
    using variant_init_index =
        typename type_traits::other_trans::type_list_front<typename variant_init_helper<void, Ty, Types...>::type>::type;

    template <typename Ty, std::size_t Tag>
    struct variant_tagged {
        static constexpr std::size_t idx = Tag;
        Ty val;
    };

    template <typename Storage, std::size_t Idx>
    using variant_tagged_ref_t = variant_tagged<decltype(variant_raw_get<Idx>(utility::declval<Storage>())) &&, Idx>;
}

#define RAINY_VARIANT_CASE(n)                                                                                                         \
    case (n) + 1:                                                                                                                     \
        if constexpr ((n) < size) {                                                                                                   \
            return static_cast<Fx &&>(func)(                                                                                          \
                variant_tagged_ref_t<VariantStorage, (n)>{variant_raw_get<(n)>(static_cast<VariantStorage &&>(obj))});                \
        }                                                                                                                             \
        rainy_assume(false);                                                                                                          \
        [[fallthrough]]

#define RAINY_VARIANT_VISIT_STAMP(stamper, n)                                                                                         \
    constexpr std::size_t size = ::rainy::type_traits::reference_modify::remove_reference_t<VariantStorage>::size;                    \
    static_assert(((n) == 4 || size > (n) / 4) && size <= (n));                                                                       \
    switch (idx) {                                                                                                                    \
        case 0:                                                                                                                       \
            foundation::exceptions::runtime::throw_bad_variant_access();                                                              \
            break;                                                                                                                    \
            stamper(0, RAINY_VARIANT_CASE);                                                                                           \
        default:                                                                                                                      \
            rainy_assume(false);                                                                                                      \
    }

namespace rainy::utility::implements {
    template <typename _Fn, class VariantStorage>
    using variant_raw_visit_t = decltype(utility::declval<_Fn>()(utility::declval<variant_tagged_ref_t<VariantStorage, 0>>()));

    template <class _Fn, class VariantStorage,
              class _Indices = type_traits::helper::make_index_sequence<type_traits::cv_modify::remove_cvref_t<VariantStorage>::size>>
    constexpr bool variant_raw_visit_noexcept = false;

    template <class _Fn, class VariantStorage, std::size_t... _Idxs>
    constexpr bool variant_raw_visit_noexcept<_Fn, VariantStorage, type_traits::helper::index_sequence<_Idxs...>> =
        type_traits::logical_traits::conjunction_v<std::is_nothrow_invocable<_Fn, variant_tagged<VariantStorage &&, variant_npos>>,
                                                   std::is_nothrow_invocable<_Fn, variant_tagged_ref_t<VariantStorage, _Idxs>>...>;

    template <std::size_t _Idx, class _Fn, class VariantStorage>
    RAINY_NODISCARD constexpr variant_raw_visit_t<_Fn, VariantStorage> variant_raw_visit_dispatch(_Fn &&_Func, VariantStorage &&_Var) noexcept(
        std::is_nothrow_invocable_v<_Fn, variant_tagged_ref_t<VariantStorage, _Idx>>) {
        return static_cast<_Fn &&>(_Func)(
            variant_tagged_ref_t<VariantStorage, _Idx>{variant_raw_get<_Idx>(static_cast<VariantStorage &&>(_Var))});
    }

    template <class _Fn, class VariantStorage>
    RAINY_NODISCARD constexpr variant_raw_visit_t<_Fn, VariantStorage> variant_raw_visit_valueless(_Fn &&_Func, VariantStorage &&_Obj) noexcept(
        std::is_nothrow_invocable_v<_Fn, variant_tagged<VariantStorage &&, variant_npos>>) {
        return static_cast<_Fn &&>(_Func)(variant_tagged<VariantStorage &&, variant_npos>{static_cast<VariantStorage &&>(_Obj)});
    }

    template <class _Fn, class VariantStorage,
              class _Indices = type_traits::helper::make_index_sequence<type_traits::reference_modify::remove_reference_t<VariantStorage>::size>>
    struct variant_raw_dispatch_table; // undefined

    template <class _Fn, class VariantStorage, std::size_t... _Idxs>
    struct variant_raw_dispatch_table<_Fn, VariantStorage, type_traits::helper::index_sequence<_Idxs...>> {
        // map from canonical index to visitation target
        using dispatch_t = variant_raw_visit_t<_Fn, VariantStorage> (*)(_Fn &&,VariantStorage &&) noexcept(variant_raw_visit_noexcept<_Fn, VariantStorage>);
        static constexpr dispatch_t table[] = {&variant_raw_visit_valueless<_Fn, VariantStorage>,
                                                 &variant_raw_visit_dispatch<_Idxs, _Fn, VariantStorage>...};
    };

    template <int Strategy>
    struct variant_raw_visit_impl;

    template <>
    struct variant_raw_visit_impl<-1> {
        template <class _Fn, class VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<_Fn, VariantStorage> invoke(std::size_t idx, _Fn &&func, VariantStorage &&obj) noexcept(
            variant_raw_visit_noexcept<_Fn, VariantStorage>) {
            constexpr std::size_t size = type_traits::reference_modify::remove_reference_t<VariantStorage>::size;
            static_assert(size > 64);
            constexpr auto &array = variant_raw_dispatch_table<_Fn, VariantStorage>::table;
            return array[idx](static_cast<_Fn &&>(func), static_cast<VariantStorage &&>(obj));
        }
    };

    template <>
    struct variant_raw_visit_impl<1> {
        template <typename Fx, class VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fx, VariantStorage> invoke(std::size_t idx, Fx &&func, VariantStorage &&obj) noexcept(
            variant_raw_visit_noexcept<Fx, VariantStorage>) {
            RAINY_STAMP(4, RAINY_VARIANT_VISIT_STAMP);
        }
    };

    template <>
    struct variant_raw_visit_impl<2> {
        template <class Fx, class VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fx, VariantStorage> invoke(std::size_t idx, Fx &&func, VariantStorage &&obj) noexcept(
            variant_raw_visit_noexcept<Fx, VariantStorage>) {
            RAINY_STAMP(16, RAINY_VARIANT_VISIT_STAMP);
        }
    };

    template <>
    struct variant_raw_visit_impl<3> {
        template <class Fx, class VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fx, VariantStorage> invoke(std::size_t idx, Fx &&func, VariantStorage &&obj) noexcept(
            variant_raw_visit_noexcept<Fx, VariantStorage>) {
            RAINY_STAMP(64, RAINY_VARIANT_VISIT_STAMP);
        }
    };

    template <class VariantStorage, class _Fn>
    RAINY_NODISCARD constexpr variant_raw_visit_t<_Fn, VariantStorage> variant_raw_visit(
        std::size_t idx, VariantStorage &&_Obj, _Fn &&_Func) noexcept(variant_raw_visit_noexcept<_Fn, VariantStorage>) {
        constexpr std::size_t size = type_traits::reference_modify::remove_reference_t<VariantStorage>::size;
        constexpr int strategy = size <= 4 ? 1 : size <= 16 ? 2 : size <= 64 ? 3 : -1;
        ++idx;
        return variant_raw_visit_impl<strategy>::invoke(idx, static_cast<_Fn &&>(_Func), static_cast<VariantStorage &&>(_Obj));
    }
}

#undef RAINY_VARIANT_VISIT_STAMP
#undef RAINY_VARIANT_CASE

namespace rainy::utility::implements {
    template <typename... Types>
    class variant_base : private variant_storage_t<Types...> {
    public:
        RAINY_NODISCARD constexpr bool valueless_by_exception() const noexcept {
            return idx == variant_npos;
        }

        RAINY_NODISCARD constexpr std::size_t index() const noexcept {
            return static_cast<std::size_t>(idx);
        }

    protected:
        using storage_t = variant_storage_t<Types...>;

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

        RAINY_CONSTEXPR20 void construct_from(const variant_base &right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_copy_constructible<Types>...>) {
            variant_raw_visit(right.index(), right.storage(), [this](auto source) {
                using source_t = decltype(source);
                if constexpr (source_t::idx != variant_npos) {
                    utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, source_t::idx>{},
                                                utility::forward<decltype(source.val)>(source.val));
                    this->idx = source_t::idx;
                }
            });
        }

        RAINY_CONSTEXPR20 void construct_from(variant_base &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...>) {
            variant_raw_visit(right.index(), utility::move(right).storage(), [&right, this](auto source) {
                using source_t = decltype(source);
                if constexpr (source_t::idx != variant_npos) {
                    utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, source_t::idx>{},
                                                utility::move(source.val));
                    this->idx = source_t::idx;
                    right.idx = 0;
                }
            });
        }

        template <std::size_t Idx>
        RAINY_CONSTEXPR20 void destroy() noexcept {
            using indexed_value_type = type_traits::cv_modify::remove_cv_t<
                typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type, Idx>;
            if constexpr (Idx != variant_npos && !type_traits::type_properties::is_trivially_destructible_v<indexed_value_type>) {
                variant_raw_get<Idx>(storage()).~indexed_value_type();
            }
        }

        RAINY_CONSTEXPR20 void destroy() noexcept {
            constexpr bool is_trivially_destructible =
                !type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_destructible<Types>...>;
            if constexpr (is_trivially_destructible) {
                if (index() != variant_npos) {
                    variant_raw_visit(index(), storage(), [](auto ref) noexcept {
                        using indexed_value_type = type_traits::cv_modify::remove_cvref_t<decltype(ref.val)>;
                        ref.val.~indexed_value_type();
                    });
                }
            }
        }
    private:
        template <typename Visitor, typename Variant, std::size_t Index>
        friend constexpr void visit_impl(Visitor &&visitor, Variant &&var);

        static inline constexpr std::size_t schar_max_as_size = static_cast<unsigned char>(-1) / 2;
        static inline constexpr std::size_t short_max_as_size = static_cast<unsigned short>(-1) / 2;

        using index_t = std::conditional_t<(sizeof...(Types) < schar_max_as_size), signed char,
                                           std::conditional_t<(sizeof...(Types) < short_max_as_size), short, int>>;

        template <typename RIndexSeq>
        friend struct variant_dispatcher;

        friend class variant<Types...>;
        
        template <typename... Types>
        friend class variant_destroy_layer;

        static inline constexpr std::size_t invalid_index = static_cast<index_t>(-1);

        RAINY_CONSTEXPR20 variant_base() noexcept : storage_t{}, idx(invalid_index) {
        }

        template <
            std::size_t Idx, typename... Args,
            type_traits::other_trans::enable_if_t<
                type_traits::type_properties::is_constructible_v<
                    typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type, Args...>,
                int> = 0>
        constexpr explicit variant_base(utility::in_place_index_t<Idx>, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<
                typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type, Args...>) :
            storage_t(type_traits::helper::integral_constant<std::size_t, Idx>{}, utility::forward<Args>(args)...) {
            this->idx = Idx;
        }

        template <std::size_t Idx, typename VariantStorage>
        friend constexpr decltype(auto) variant_raw_get(VariantStorage &&obj) noexcept;

        RAINY_CONSTEXPR20 void set_index(const std::size_t index) noexcept {
            this->idx = static_cast<index_t>(index);
        }

        std::size_t idx{variant_npos};
    };

    template <typename... Types>
    struct variant_destroy_layer : implements::variant_base<Types...> {
        using base = variant_base<Types...>;
        using base::base;

        RAINY_CONSTEXPR20 ~variant_destroy_layer() noexcept { // Destroy contained value, if any
            base::destroy();
        }

        variant_destroy_layer() = default;
        variant_destroy_layer(const variant_destroy_layer &) = default;
        variant_destroy_layer(variant_destroy_layer &&) = default;
        variant_destroy_layer &operator=(const variant_destroy_layer &) = default;
        variant_destroy_layer &operator=(variant_destroy_layer &&) = default;
    };

    template <typename... Types>
    using variant_destroy_layer_t = std::conditional_t<std::conjunction_v<std::is_trivially_destructible<Types>...>, variant_base<Types...>,
                                                 variant_destroy_layer<Types...>>;
}

namespace rainy::utility {
    template <typename... Types>
    class variant : public implements::variant_destroy_layer_t<Types...> {
    public:
        using base = implements::variant_destroy_layer_t<Types...>;
        using type_list = type_traits::other_trans::type_list<Types...>;
        using base::storage;

        template <typename First = typename type_traits::other_trans::type_at<0, type_list>::type,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<First>, int> = 0>
        constexpr variant() : base(utility::in_place_index<0>) {
        }

        template <typename Ty,
                  type_traits::other_trans::enable_if_t<
                      sizeof...(Types) != 0 &&
                          !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Ty>, type_list> &&
                          !type_traits::primary_types::is_specialization_v<type_traits::cv_modify::remove_cvref_t<Ty>,
                                                                           utility::placeholder_type_t> &&
                          !utility::is_in_place_index_specialization<type_traits::cv_modify::remove_cvref_t<Ty>> &&
                          type_traits::type_properties::is_constructible_v<implements::variant_init_type<Ty, Types...>, Ty>,
                      int> = 0>
        constexpr variant(Ty &&object) noexcept(std::is_nothrow_constructible_v<implements::variant_init_type<Ty, Types...>, Ty>) :
            base(utility::in_place_index<implements::variant_init_index<Ty, Types...>::value>, object) {
        }

        template <typename Ty, typename... Args, typename Idx = type_traits::other_trans::type_find_unique<Ty, type_list>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr explicit variant(std::in_place_type_t<Ty>, Args &&...args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        }

        template <typename Ty, typename... Args, typename Idx = type_traits::other_trans::type_find_unique<Ty, type_list>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr explicit variant(utility::placeholder_type_t<Ty>,
                                   Args &&...args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) :
            base(utility::in_place_index<Idx::value>, utility::forward<Args>(args)...) {
        }

        template <std::size_t Idx, typename... Args, typename Type = typename type_traits::other_trans::type_at<Idx, type_list>::type,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type, Args...> &&
                                                            type_traits::other_trans::type_find_unique<Type, variant>::value !=
                                                                base::invalid_index /* 防止类型双关 */,
                                                        int> = 0>
        constexpr explicit variant(std::in_place_index_t<Idx>,
                                   Args &&...args) noexcept(std::is_nothrow_constructible_v<Type, Args...>) :
            base(utility::in_place_index<Idx>, utility::forward<Args>(args)...) {
        }

        template <std::size_t Idx, typename... Args, typename Type = typename type_traits::other_trans::type_at<Idx, type_list>::type,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type, Args...> &&
                                                            type_traits::other_trans::type_find_unique<Type, variant>::value !=
                                                                base::invalid_index /* 防止类型双关 */,
                                                        int> = 0>
        constexpr explicit variant(utility::in_place_index_t<Idx>,
                                   Args &&...args) noexcept(std::is_nothrow_constructible_v<Type, Args...>) :
            base(utility::in_place_index<Idx>, utility::forward<Args>(args)...) {
        }

        variant(const variant &right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_copy_constructible<Types>...>) {
            this->construct_from(right);
        }

        variant(variant &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...>) {
            this->construct_from(utility::move(right));
        }

        variant &operator=(const variant & right) {
            base::destroy();
            this->idx = variant_npos;
            this->construct_from(right);
            return *this;
        }

        variant &operator=(variant&& right) {
            base::destroy();
            this->idx = variant_npos;
            this->construct_from(utility::move(right));
            return *this;
        }

        template <typename Ty>
        variant &operator=(Ty&& object) {
            base::destroy();
            this->idx = variant_npos;
            this->construct_from(variant{object});
            return *this;
        }
    };
}

namespace rainy::utility {
    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(variant<Types...> &var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(var.storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(const variant<Types...> &var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(var.storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(variant<Types...> &&var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(std::move(var).storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(const variant<Types...> &&var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(std::move(var).storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr auto *get_if(variant<Types...> *var) noexcept {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (!var || var->index() != Index) {
            return nullptr;
        }
        return &implements::variant_raw_get<Index>(var->storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr auto *get_if(const variant<Types...> *var) noexcept {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (!var || var->index() != Index) {
            return nullptr;
        }
        return &implements::variant_raw_get<Index>(var->storage());
    }

    template <typename Ty, typename... Types>
    constexpr bool holds_alternative(const variant<Types...> &var) noexcept {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return var.index() == index;
    }
}

namespace rainy::utility::implements {
    template <typename Visitor, typename Variant>
    constexpr decltype(auto) visit_single_impl(Visitor &&visitor, Variant &&var) {
        using variant_type = std::remove_reference_t<Variant>;
        if (var.valueless_by_exception()) {
            foundation::exceptions::runtime::throw_bad_variant_access();
        }
        return variant_raw_visit(var.index(), utility::forward<Variant>(var).storage(),
                                 [&visitor](auto &&tagged_ref) -> decltype(auto) {
                                     using tagged_type = std::remove_reference_t<decltype(tagged_ref)>;
                                     if constexpr (tagged_type::idx != variant_npos) {
                                         return utility::invoke(utility::forward<Visitor>(visitor),
                                                                utility::forward<decltype(tagged_ref.val)>(tagged_ref.val));
                                     }
                                 });
    }

    template <typename Visitor, typename FirstVariant, typename... RestVariants>
    constexpr decltype(auto) visit_multi_impl(Visitor &&visitor, FirstVariant &&first, RestVariants &&...rest) {
        return visit_single_impl(
            [&visitor, &rest...](auto &&first_val) -> decltype(auto) {
                if constexpr (sizeof...(RestVariants) == 0) {
                    return utility::invoke(utility::forward<Visitor>(visitor), utility::forward<decltype(first_val)>(first_val));
                } else {
                    return visit_multi_impl(
                        [&visitor, &first_val](auto &&...rest_vals) -> decltype(auto) {
                            return utility::invoke(utility::forward<Visitor>(visitor),
                                                   utility::forward<decltype(first_val)>(first_val),
                                                   utility::forward<decltype(rest_vals)>(rest_vals)...);
                        },
                        utility::forward<RestVariants>(rest)...);
                }
            },
            utility::forward<FirstVariant>(first));
    }
}

namespace rainy::utility {
    template <typename Visitor, typename... Variants>
    constexpr decltype(auto) visit(Visitor &&visitor, Variants &&...variants) {
        if constexpr (sizeof...(Variants) == 1) {
            return implements::visit_single_impl(utility::forward<Visitor>(visitor), utility::forward<Variants>(variants)...);
        } else {
            return implements::visit_multi_impl(utility::forward<Visitor>(visitor), utility::forward<Variants>(variants)...);
        }
    }

    template <typename Rx, typename Visitor, typename... Variants>
    constexpr Rx visit(Visitor &&visitor, Variants &&...variants) {
        return static_cast<Rx>(visit(utility::forward<Visitor>(visitor), utility::forward<Variants>(variants)...));
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif