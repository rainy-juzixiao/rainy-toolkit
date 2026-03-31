/*
 * Copyright 2026 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_CONTAINERvariantIANT_HPP
#define RAINY_FOUNDATION_CONTAINERvariantIANT_HPP
#include <rainy/core/core.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26495 4715)
#endif

#if RAINY_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif

namespace rainy::foundation::container {
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
    class badvariantiant_access : public runtime_error {
    public:
        badvariantiant_access(const diagnostics::source_location &source) : runtime_error{"bad variant access", source} { // NOLINT
        }
    };

    RAINY_INLINE void throw_badvariantiant_access(
        const diagnostics::source_location &source = diagnostics::source_location::current()) {
        throw_exception(badvariantiant_access{source});
    }
}

namespace rainy::foundation::container::implements {
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

        constexpr variant_storage() noexcept { // NOLINT
        }

        RAINY_CONSTEXPR20 ~variant_storage() = default;

        constexpr variant_storage(variant_storage &&) noexcept = default;
        constexpr variant_storage(const variant_storage &) = default;
        constexpr variant_storage &operator=(const variant_storage &) = default;
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

        constexpr variant_storage() noexcept { // NOLINT
        }

        RAINY_CONSTEXPR20 ~variant_storage() noexcept {
        }

        constexpr variant_storage(variant_storage &&) noexcept = default;
        constexpr variant_storage(const variant_storage &) = default;
        constexpr variant_storage &operator=(const variant_storage &) = default;
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

namespace rainy::foundation::container::implements {
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
            return variant_raw_get<Idx - 8>(static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.tail.tail.tail);
        } else if constexpr (Idx < 32) {
            return variant_raw_get<Idx - 16>(
                static_cast<VariantStorage &&>(obj).tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail);
        } else if constexpr (Idx < 64) {
            return variant_raw_get<Idx - 32>(static_cast<VariantStorage &&>(obj)
                                                 .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail
                                                 .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail);
        } else { // Idx >= 64
            return variant_raw_get<Idx - 64>(
                static_cast<VariantStorage &&>(obj)
                    .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail
                    .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail
                    .tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail.tail);
        }
    }
}

namespace rainy::foundation::container::implements {
    // NOLINTBEGIN
    template <std::size_t Index, typename TargetType>
    struct variant_init_single_overload {
        template <typename InitializerType>
        auto operator()(TargetType, InitializerType &&)
            -> type_traits::other_trans::type_list<type_traits::helper::integral_constant<std::size_t, Index>, TargetType> {
        }
    };
    // NOLINTEND

    template <typename Indices, typename... Types>
    struct variant_init_overload_set_;

    template <std::size_t... Indices, typename... Types>
    struct variant_init_overload_set_<std::index_sequence<Indices...>, Types...> : variant_init_single_overload<Indices, Types>... {
        using variant_init_single_overload<Indices, Types>::operator()...;
    };

    template <typename... Types>
    using variant_init_overload_set = variant_init_overload_set_<std::index_sequence_for<Types...>, Types...>;

    // NOLINTBEGIN
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
    //  NOLINTEND

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
            foundation::exceptions::runtime::throw_badvariantiant_access();                                                           \
            break;                                                                                                                    \
            stamper(0, RAINY_VARIANT_CASE);                                                                                           \
        default:                                                                                                                      \
            rainy_assume(false);                                                                                                      \
    }

namespace rainy::foundation::container::implements {
    template <typename Fn, typename VariantStorage>
    using variant_raw_visit_t = decltype(utility::declval<Fn>()(utility::declval<variant_tagged_ref_t<VariantStorage, 0>>()));

    template <typename Fn, typename VariantStorage,
              typename Indices =
                  type_traits::helper::make_index_sequence<type_traits::cv_modify::remove_cvref_t<VariantStorage>::size>>
    constexpr bool variant_raw_visit_noexcept = false;

    template <typename Fn, typename VariantStorage, std::size_t... Idxs>
    constexpr bool variant_raw_visit_noexcept<Fn, VariantStorage, type_traits::helper::index_sequence<Idxs...>> =
        type_traits::logical_traits::conjunction_v<std::is_nothrow_invocable<Fn, variant_tagged<VariantStorage &&, variant_npos>>,
                                                   std::is_nothrow_invocable<Fn, variant_tagged_ref_t<VariantStorage, Idxs>>...>;

    template <std::size_t Idx, typename Fn, typename VariantStorage>
    RAINY_NODISCARD constexpr variant_raw_visit_t<Fn, VariantStorage> variant_raw_visit_dispatch(
        Fn &&func, VariantStorage &&variant) noexcept(std::is_nothrow_invocable_v<Fn, variant_tagged_ref_t<VariantStorage, Idx>>) {
        return static_cast<Fn &&>(func)(
            variant_tagged_ref_t<VariantStorage, Idx>{variant_raw_get<Idx>(static_cast<VariantStorage &&>(variant))});
    }

    template <typename Fn, typename VariantStorage,
              typename Indices =
                  type_traits::helper::make_index_sequence<type_traits::reference_modify::remove_reference_t<VariantStorage>::size>>
    struct variant_raw_dispatch_table;

    // NOLINTBEGIN
    template <typename Fn, typename VariantStorage, std::size_t... Idxs>
    struct variant_raw_dispatch_table<Fn, VariantStorage, type_traits::helper::index_sequence<Idxs...>> {
        using dispatch_t = variant_raw_visit_t<Fn, VariantStorage> (*)(Fn &&, VariantStorage &&) noexcept(
            variant_raw_visit_noexcept<Fn, VariantStorage>);

        static constexpr dispatch_t table[] = {[](Fn &&, VariantStorage &&) -> variant_raw_visit_t<Fn, VariantStorage> {
                                                   exceptions::runtime::throw_badvariantiant_access();
                                               },
                                               &variant_raw_visit_dispatch<Idxs, Fn, VariantStorage>...};
    };
    // NOLINTEND

    template <int Strategy>
    struct variant_raw_visit_impl;

    template <>
    struct variant_raw_visit_impl<-1> {
        template <typename Fn, typename VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fn, VariantStorage> invoke(
            std::size_t idx, Fn &&func, VariantStorage &&obj) noexcept(variant_raw_visit_noexcept<Fn, VariantStorage>) {
            constexpr std::size_t size = type_traits::reference_modify::remove_reference_t<VariantStorage>::size;
            static_assert(size > 64);
            constexpr auto &array = variant_raw_dispatch_table<Fn, VariantStorage>::table;
            return array[idx](static_cast<Fn &&>(func), static_cast<VariantStorage &&>(obj));
        }
    };

    // NOLINTBEGIN
    template <>
    struct variant_raw_visit_impl<1> {
        template <typename Fx, typename VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fx, VariantStorage> invoke(
            std::size_t idx, Fx &&func, VariantStorage &&obj) noexcept(variant_raw_visit_noexcept<Fx, VariantStorage>) {
            RAINY_STAMP(4, RAINY_VARIANT_VISIT_STAMP);
        }
    };

    template <>
    struct variant_raw_visit_impl<2> {
        template <typename Fx, typename VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fx, VariantStorage> invoke(
            const std::size_t idx, Fx &&func, VariantStorage &&obj) noexcept(variant_raw_visit_noexcept<Fx, VariantStorage>) {
            RAINY_STAMP(16, RAINY_VARIANT_VISIT_STAMP);
        }
    };

    template <>
    struct variant_raw_visit_impl<3> {
        template <typename Fx, typename VariantStorage>
        RAINY_NODISCARD static constexpr variant_raw_visit_t<Fx, VariantStorage> invoke(
            std::size_t idx, Fx &&func, VariantStorage &&obj) noexcept(variant_raw_visit_noexcept<Fx, VariantStorage>) {
            RAINY_STAMP(64, RAINY_VARIANT_VISIT_STAMP);
        }
    };
    // NOLINTEND

    template <typename VariantStorage, typename Fn>
    RAINY_NODISCARD constexpr variant_raw_visit_t<Fn, VariantStorage> variant_raw_visit(
        std::size_t idx, VariantStorage &&object, Fn &&func) noexcept(variant_raw_visit_noexcept<Fn, VariantStorage>) {
        constexpr std::size_t size = type_traits::reference_modify::remove_reference_t<VariantStorage>::size;
        constexpr int strategy = size <= 4 ? 1 : size <= 16 ? 2 : size <= 64 ? 3 : -1;
        if (idx == variant_npos) {
            exceptions::runtime::throw_badvariantiant_access();
        }
        ++idx;
        return variant_raw_visit_impl<strategy>::invoke(idx, static_cast<Fn &&>(func), static_cast<VariantStorage &&>(object));
    }
}

#undef RAINY_VARIANT_VISIT_STAMP
#undef RAINY_VARIANT_CASE

namespace rainy::foundation::container::implements {
    template <typename... Types>
    class variant_base : private variant_storage_t<Types...> {
    public:
        RAINY_NODISCARD constexpr bool valueless_by_exception() const noexcept {
            return idx == invalid_index;
        }

        RAINY_NODISCARD constexpr std::size_t index() const noexcept {
            if (idx == invalid_index) {
                return variant_npos; // -1 as index_t -> SIZE_MAX as size_t
            }
            return static_cast<std::size_t>(idx);
        }

    protected:
        static inline constexpr std::size_t schar_max_as_size = static_cast<unsigned char>(-1) / 2;
        static inline constexpr std::size_t short_max_as_size = static_cast<unsigned short>(-1) / 2;

        using index_t = std::conditional_t<(sizeof...(Types) < schar_max_as_size), signed char,
                                           std::conditional_t<(sizeof...(Types) < short_max_as_size), short, int>>;

        static inline constexpr index_t invalid_index = static_cast<index_t>(-1);

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
            if (right.valueless_by_exception()) {
                this->idx = invalid_index;
                return;
            }
            variant_raw_visit(right.index(), right.storage(), [this](auto &&source) { // NOLINT
                using source_t = type_traits::cv_modify::remove_cvref_t<decltype(source)>;
                if constexpr (source_t::idx != variant_npos) {
                    utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, source_t::idx>{},
                                                utility::forward<decltype(source.val)>(source.val));
                    this->idx = source_t::idx;
                }
            });
        }

        RAINY_CONSTEXPR20 void construct_from(variant_base &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...>) {

            if (right.valueless_by_exception()) {
                this->idx = invalid_index;
                return;
            }

            variant_raw_visit(right.index(), utility::move(right).storage(), [&right, this](auto &&source) { // NOLINT
                using source_t = type_traits::cv_modify::remove_cvref_t<decltype(source)>;
                if constexpr (source_t::idx != variant_npos) {
                    utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, source_t::idx>{},
                                                utility::move(source.val));
                    this->idx = source_t::idx;
                    right.template destroy<source_t::idx>();
                    right.idx = invalid_index;
                }
            });
        }

        template <std::size_t Idx>
        RAINY_CONSTEXPR20 void destroy() noexcept {
            using indexed_value_type = type_traits::cv_modify::remove_cv_t<
                typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type>;
            if constexpr (Idx != variant_npos && !type_traits::type_properties::is_trivially_destructible_v<indexed_value_type>) {
                variant_raw_get<Idx>(storage()).~indexed_value_type();
            }
        }

        RAINY_CONSTEXPR20 void destroy() noexcept {
            constexpr bool has_non_trivial_dtor =
                !type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_trivially_destructible<Types>...>;
            if constexpr (has_non_trivial_dtor) {
                if (!valueless_by_exception()) {
                    variant_raw_visit(index(), storage(), [](auto &&ref) noexcept { utility::destroy_at(&ref.val); });
                }
            }
        }

        RAINY_CONSTEXPR20 void swap_with(variant_base &other) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...,
                                                       type_traits::type_properties::is_nothrow_swappable<Types>...>) {
            if (this == &other) {
                return;
            }
            if (valueless_by_exception() && other.valueless_by_exception()) {
                return;
            }
            if (index() == other.index() && !valueless_by_exception()) {
                variant_raw_visit(index(), storage(), [&other](auto &&left_ref) { // NOLINT
                    using tagged = type_traits::cv_modify::remove_cvref_t<decltype(left_ref)>;
                    if constexpr (tagged::idx != variant_npos) {
                        using std::swap;
                        swap(left_ref.val, variant_raw_get<tagged::idx>(other.storage()));
                    }
                });
            } else {
                variant_base tmp;
                tmp.construct_from(utility::move(*this));
                destroy();
                idx = invalid_index;
                construct_from(utility::move(other));
                other.destroy();
                other.idx = invalid_index;
                other.construct_from(utility::move(tmp));
            }
        }

    private:
        template <typename Visitor, typename Variant, std::size_t Index>
        friend constexpr void visit_impl(Visitor &&visitor, Variant &&var);

        template <typename RIndexSeq>
        friend struct variant_dispatcher;

        friend class variant<Types...>;

        template <typename...>
        friend class variant_destroy_layer;

        RAINY_CONSTEXPR20 variant_base() noexcept : storage_t{}, idx(invalid_index) {
        }

        template <
            std::size_t Idx, typename... Args,
            type_traits::other_trans::enable_if_t<
                type_traits::type_properties::is_constructible_v<
                    typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type, Args...>,
                int> = 0>
        constexpr explicit variant_base(std::in_place_index_t<Idx>, Args &&...args) noexcept(
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

        index_t idx{invalid_index};
    };

    template <typename... Types>
    class variant_destroy_layer : public variant_base<Types...> {
    public:
        using base = variant_base<Types...>;
        using base::base;

        RAINY_CONSTEXPR20 ~variant_destroy_layer() noexcept {
            base::destroy();
            this->idx = base::invalid_index;
        }
    };

    template <typename... Types>
    using variant_destroy_layer_t = std::conditional_t<std::conjunction_v<std::is_trivially_destructible<Types>...>,
                                                       variant_base<Types...>, variant_destroy_layer<Types...>>;
}

namespace rainy::foundation::container {
    template <typename... Types>
    class variant : implements::variant_destroy_layer_t<Types...> {
    public:
        using base = implements::variant_destroy_layer_t<Types...>;
        using type_list = type_traits::other_trans::type_list<Types...>;
        using base::storage;

        template <typename First = type_traits::other_trans::type_at_t<0, type_list>,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<First>, int> = 0>
        constexpr variant() : base(std::in_place_index<0>) {
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
        constexpr variant(Ty &&object) noexcept( // NOLINT
            type_traits::type_properties::is_nothrow_constructible_v<implements::variant_init_type<Ty, Types...>, Ty>) :
            base(std::in_place_index<implements::variant_init_index<Ty, Types...>::value>, utility::forward<Ty>(object)) {
        }

        template <typename Ty, typename... Args, typename Idx = type_traits::other_trans::type_find_unique<Ty, type_list>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr explicit variant(std::in_place_type_t<Ty>,
                                   Args &&...args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) :
            base(std::in_place_index<Idx::value>, utility::forward<Args>(args)...) {
        }

        template <std::size_t Idx, typename... Args, typename Type = type_traits::other_trans::type_at_t<Idx, type_list>,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Type, Args...> &&
                          type_traits::other_trans::type_find_unique<Type, type_traits::other_trans::type_list<Types...>>::value !=
                              base::invalid_index /* 防止类型双关 */,
                      int> = 0>
        constexpr explicit variant(std::in_place_index_t<Idx>,
                                   Args &&...args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Type, Args...>) :
            base(std::in_place_index<Idx>, utility::forward<Args>(args)...) {
        }

        template <typename Ty, typename U, typename... Args,
                  typename Idx = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index &&
                          type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>,
                      int> = 0>
        constexpr explicit variant(std::in_place_type_t<Ty>, std::initializer_list<U> il, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, std::initializer_list<U> &, Args...>) :
            base(std::in_place_index<Idx::value>, il, utility::forward<Args>(args)...) {
        }

        template <std::size_t Idx, typename U, typename... Args,
                  typename Ty = type_traits::other_trans::type_at_t<Idx, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>, int> = 0>
        constexpr explicit variant(std::in_place_index_t<Idx>, std::initializer_list<U> il, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, std::initializer_list<U> &, Args...>) :
            base(std::in_place_index<Idx>, il, utility::forward<Args>(args)...) {
        }

        constexpr variant(const variant &right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_copy_constructible<Types>...>) {
            this->construct_from(right);
        }

        constexpr variant(variant &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...>) {
            this->construct_from(utility::move(right));
        }

        ~variant() = default;

        constexpr variant &operator=(const variant &right) {
            if (this == &right) {
                return *this;
            }
            if (right.valueless_by_exception()) {
                base::destroy();
                this->idx = variant_npos;
                return *this;
            }
            if (this->index() == right.index()) {
                implements::variant_raw_visit(right.index(), right.storage(), [this](auto &&right_ref) { // NOLINT
                    using tagged = type_traits::cv_modify::remove_cvref_t<decltype(right_ref)>;
                    if constexpr (tagged::idx != variant_npos) {
                        implements::variant_raw_get<tagged::idx>(this->storage()) = right_ref.val;
                    }
                });
            } else {
                base::destroy();
                this->idx = variant_npos;
                this->construct_from(right);
            }
            return *this;
        }

        constexpr variant &operator=(variant &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...,
                                                       type_traits::type_properties::is_nothrow_move_assignable<Types>...>) {
            if (this == &right) {
                return *this;
            }
            if (right.valueless_by_exception()) {
                base::destroy();
                this->idx = static_cast<base::index_t>(variant_npos);
                return *this;
            }
            if (this->index() == right.index()) {
                implements::variant_raw_visit(right.index(), utility::move(right).storage(), [this](auto &&right_ref) { // NOLINT
                    using tagged = type_traits::cv_modify::remove_cvref_t<decltype(right_ref)>;
                    if constexpr (tagged::idx != variant_npos) {
                        implements::variant_raw_get<tagged::idx>(this->storage()) = utility::move(right_ref.val);
                    }
                });
            } else {
                base::destroy();
                this->idx = static_cast<base::index_t>(variant_npos);
                this->construct_from(utility::move(right));
            }
            return *this;
        }

        template <typename Ty>
        constexpr variant &operator=(Ty &&object) {
            try {
                variant tmp(utility::forward<Ty>(object));
                base::destroy();
                this->idx = static_cast<base::index_t>(variant_npos);
                this->construct_from(utility::move(tmp));
            } catch (...) {
                base::destroy();
                this->idx = static_cast<base::index_t>(variant_npos);
                throw;
            }
            return *this;
        }

        template <typename Ty, typename... Args,
                  typename Idx = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr Ty &emplace(Args &&...args) {
            return emplace<Idx::value>(utility::forward<Args>(args)...);
        }

        template <typename Ty, typename U, typename... Args,
                  typename Idx = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index &&
                          type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>,
                      int> = 0>
        constexpr Ty &emplace(std::initializer_list<U> il, Args &&...args) {
            return emplace<Idx::value>(il, utility::forward<Args>(args)...);
        }

        template <std::size_t Idx, typename... Args,
                  typename Ty = type_traits::other_trans::type_at_t<Idx, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr Ty &emplace(Args &&...args) {
            base::destroy();
            this->idx = static_cast<base::index_t>(variant_npos);
            utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, Idx>{},
                                        utility::forward<Args>(args)...);
            this->set_index(Idx);
            return implements::variant_raw_get<Idx>(this->storage());
        }

        template <std::size_t Idx, typename U, typename... Args,
                  typename Ty = type_traits::other_trans::type_at_t<Idx, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>, int> = 0>
        constexpr Ty &emplace(std::initializer_list<U> il, Args &&...args) {
            base::destroy();
            this->idx = static_cast<base::index_t>(variant_npos);
            utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, Idx>{}, il,
                                        utility::forward<Args>(args)...);
            this->set_index(Idx);
            return implements::variant_raw_get<Idx>(this->storage());
        }

        constexpr void swap(variant &other) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::type_properties::is_nothrow_move_constructible<Types>...,
                                                       type_traits::type_properties::is_nothrow_swappable<Types>...>) {
            this->swap_with(other);
        }

        using base::index;
        using base::valueless_by_exception;
    };
}

namespace rainy::foundation::container {
    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(variant<Types...> &var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_badvariantiant_access();
        }
        return implements::variant_raw_get<Index>(var.storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(const variant<Types...> &var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_badvariantiant_access();
        }
        return implements::variant_raw_get<Index>(var.storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(variant<Types...> &&var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_badvariantiant_access();
        }
        return implements::variant_raw_get<Index>(utility::move(var).storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(const variant<Types...> &&var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            foundation::exceptions::runtime::throw_badvariantiant_access();
        }
        return implements::variant_raw_get<Index>(utility::move(var).storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr auto get_if(variant<Types...> *var) noexcept
        -> decltype(&implements::variant_raw_get<Index>(std::declval<variant<Types...> &>().storage())) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (!var || var->index() != Index) {
            return nullptr;
        }
        return &implements::variant_raw_get<Index>(var->storage());
    }

    template <std::size_t Index, typename... Types>
    constexpr auto get_if(const variant<Types...> *var) noexcept
        -> decltype(&implements::variant_raw_get<Index>(std::declval<const variant<Types...> &>().storage())) {
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

namespace rainy::foundation::container {
    template <typename Ty, typename... Types>
    constexpr Ty &get(variant<Types...> &var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(var);
    }

    template <typename Ty, typename... Types>
    constexpr Ty &&get(variant<Types...> &&var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(utility::move(var));
    }

    template <typename Ty, typename... Types>
    constexpr const Ty &get(const variant<Types...> &var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(var);
    }

    template <typename Ty, typename... Types>
    constexpr const Ty &&get(const variant<Types...> &&var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(utility::move(var));
    }

    template <typename Ty, typename... Types>
    constexpr auto *get_if(variant<Types...> *var) noexcept {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get_if<index>(var);
    }

    template <typename Ty, typename... Types>
    constexpr const auto *get_if(const variant<Types...> *var) noexcept {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get_if<index>(var);
    }
}

namespace rainy::foundation::container::implements {
    // NOLINTBEGIN
    template <typename Visitor, typename Variant>
    constexpr decltype(auto) visit_single_impl(Visitor &&visitor, Variant &&var) {
        if (var.valueless_by_exception()) {
            foundation::exceptions::runtime::throw_badvariantiant_access();
        }
        return variant_raw_visit(var.index(), utility::forward<Variant>(var).storage(),
                                 [&visitor](auto &&tagged_ref) -> decltype(auto) {
                                     using tagged_type = type_traits::reference_modify::remove_reference_t<decltype(tagged_ref)>;
                                     if constexpr (tagged_type::idx != variant_npos) {
                                         return utility::invoke(utility::forward<Visitor>(visitor),
                                                                utility::forward<decltype(tagged_ref.val)>(tagged_ref.val));
                                     } else {
                                         foundation::exceptions::runtime::throw_badvariantiant_access();
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
    // NOLINTEND
}

namespace rainy::foundation::container {
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

namespace rainy::foundation::container {
    template <typename... Types>
    constexpr bool operator==(const variant<Types...> &left, const variant<Types...> &right) {
        if (left.index() != right.index()) {
            return false;
        }
        if (left.valueless_by_exception()) {
            return true; // 两者都 valueless
        }
        bool result = false;
        implements::variant_raw_visit(left.index(), left.storage(), [&](auto &&left_ref) { // NOLINT
            using tagged = type_traits::cv_modify::remove_cvref_t<decltype(left_ref)>;
            if constexpr (tagged::idx != variant_npos) {
                result = (left_ref.val == implements::variant_raw_get<tagged::idx>(right.storage()));
            }
        });
        return result;
    }

    template <typename... Types>
    constexpr bool operator!=(const variant<Types...> &left, const variant<Types...> &right) {
        return !(left == right);
    }

    template <typename... Types>
    constexpr bool operator<(const variant<Types...> &left, const variant<Types...> &right) {
        if (right.valueless_by_exception()) {
            return false;
        }
        if (left.valueless_by_exception()) {
            return true;
        }
        if (left.index() != right.index()) {
            return left.index() < right.index();
        }
        bool result = false;
        implements::variant_raw_visit(left.index(), left.storage(), [&](auto &&left_ref) { // NOLINT
            using tagged = type_traits::cv_modify::remove_cvref_t<decltype(left_ref)>;
            if constexpr (tagged::idx != variant_npos) {
                result = (left_ref.val < implements::variant_raw_get<tagged::idx>(right.storage()));
            }
        });
        return result;
    }

    template <typename... Types>
    constexpr bool operator>(const variant<Types...> &left, const variant<Types...> &right) {
        return right < left;
    }

    template <typename... Types>
    constexpr bool operator<=(const variant<Types...> &left, const variant<Types...> &right) {
        return !(right < left);
    }

    template <typename... Types>
    constexpr bool operator>=(const variant<Types...> &left, const variant<Types...> &right) {
        return !(left < right);
    }
}

namespace std {
    using rainy::foundation::container::get; // NOLINT
}

namespace rainy::utility {
    using foundation::container::get;
    using foundation::container::get_if;
    using foundation::container::holds_alternative;
    using foundation::container::variant;
    using foundation::container::variant_npos;
    using foundation::container::variant_size;
    using foundation::container::variant_size_v;
    using foundation::container::visit;
}

#if RAINY_USING_GCC
#pragma GCC diagnostic pop
#endif

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
