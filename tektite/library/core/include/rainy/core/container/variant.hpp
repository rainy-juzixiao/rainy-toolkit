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
#ifndef RAINY_CORE_CONTAINER_VARIANT_HPP
#define RAINY_CORE_CONTAINER_VARIANT_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/diagnostics/source_location.hpp>
#include <rainy/core/type_traits.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26495 4715)
#endif

#if RAINY_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A type-safe union that stores a value of exactly one of the listed types.
     *
     * @tparam Types The alternative types the variant may hold
     *
     * \lang simp-chinese
     * @brief 一种类型安全的联合，恰好存储所列类型之一的值。
     *
     * @tparam Types variant 可能持有的备选类型
     */
    template <typename... Types>
    class variant; // 前置声明

    /**
     * \lang english
     * @brief A special index value indicating that the variant holds no value.
     *
     * \lang simp-chinese
     * @brief 用于指示 variant 不持有任何值的特殊索引值。
     */
    RAINY_INLINE_CONSTEXPR std::size_t variant_npos = type_traits::other_trans::type_list_npos;

    /**
     * \lang english
     * @brief Provides the number of alternative types of a variant.
     *
     * @tparam Ty The type to inspect
     *
     * \lang simp-chinese
     * @brief 提供 variant 备选类型的数量。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    struct variant_size; // undefined

    /**
     * \lang english
     * @brief Specialization of variant_size for const-qualified variants.
     *
     * @tparam Ty The variant type
     *
     * \lang simp-chinese
     * @brief variant_size 针对 const 限定 variant 的特化。
     *
     * @tparam Ty variant 类型
     */
    template <typename Ty>
    struct variant_size<const Ty> : variant_size<Ty>::type {};

    /**
     * \lang english
     * @brief Specialization of variant_size for the variant class template.
     *
     * @tparam Types The alternative types of the variant
     *
     * \lang simp-chinese
     * @brief variant_size 针对 variant 类模板的特化。
     *
     * @tparam Types variant 的备选类型
     */
    template <typename... Types>
    struct variant_size<variant<Types...>> : type_traits::helper::integral_constant<std::size_t, sizeof...(Types)> {};

    /**
     * \lang english
     * @brief The number of alternative types of a variant.
     *
     * @tparam Ty The type to inspect
     *
     * \lang simp-chinese
     * @brief variant 备选类型的数量。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    constexpr std::size_t variant_size_v = variant_size<Ty>::value;
}

namespace rainy::core::exceptions::runtime {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bad_variant_access, core::exceptions::runtime::runtime_error, "bad variant access",
                                      throw_bad_variant_access);
}

// @NODOCBEGIN
namespace rainy::core::container::implements {
    template <bool TrivialTestruction, typename... Types>
    struct variant_storage {};

    template <typename First, typename... Rest>
    struct variant_storage<true, First, Rest...> {
        template <typename... Types>
        using variant_storage_type =
            variant_storage<type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...>,
                            Types...>;

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
            type_traits::modifers::remove_cv_t<first_t> head;
            variant_storage_type<Rest...> tail;
        };
    };

    template <typename First, typename... Rest>
    struct variant_storage<false, First, Rest...> {
        template <typename... Types>
        using variant_storage_type =
            variant_storage<type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...>,
                            Types...>;

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
            type_traits::modifers::remove_cv_t<first_t> head;
            variant_storage_type<Rest...> tail;
        };
    };

    template <typename... Types>
    using variant_storage_t =
        variant_storage<type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...>,
                        Types...>;
}

namespace rainy::core::container::implements {
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

namespace rainy::core::container::implements {
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
    struct variant_init_overload_set_<type_traits::helper::index_sequence<Indices...>, Types...>
        : variant_init_single_overload<Indices, Types>... {
        using variant_init_single_overload<Indices, Types>::operator()...;
    };

    template <typename... Types>
    using variant_init_overload_set = variant_init_overload_set_<type_traits::helper::index_sequence_for<Types...>, Types...>;

    // NOLINTBEGIN
    template <typename Enable, typename Ty, typename... Types>
    struct variant_init_helper {};

    template <typename Ty, typename... Types>
    struct variant_init_helper<type_traits::other_trans::void_t<decltype(variant_init_overload_set<Types...>{}(
                                   utility::declval<Ty>(), utility::declval<Ty>()))>,
                               Ty, Types...> {

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
    constexpr std::size_t size = ::rainy::type_traits::modifers::remove_reference_t<VariantStorage>::size;                            \
    static_assert(((n) == 4 || size > (n) / 4) && size <= (n));                                                                       \
    switch (idx) {                                                                                                                    \
        case 0:                                                                                                                       \
            core::exceptions::runtime::throw_bad_variant_access();                                                                    \
            break;                                                                                                                    \
            stamper(0, RAINY_VARIANT_CASE);                                                                                           \
        default:                                                                                                                      \
            rainy_assume(false);                                                                                                      \
    }

namespace rainy::core::container::implements {
    template <typename Fn, typename VariantStorage>
    using variant_raw_visit_t = decltype(utility::declval<Fn>()(utility::declval<variant_tagged_ref_t<VariantStorage, 0>>()));

    template <typename Fn, typename VariantStorage,
              typename Indices = type_traits::helper::make_index_sequence<type_traits::modifers::remove_cvref_t<VariantStorage>::size>>
    constexpr bool variant_raw_visit_noexcept = false;

    template <typename Fn, typename VariantStorage, std::size_t... Idxs>
    constexpr bool variant_raw_visit_noexcept<Fn, VariantStorage, type_traits::helper::index_sequence<Idxs...>> =
        type_traits::logical_traits::conjunction_v<
            type_traits::properties::is_nothrow_invocable<Fn, variant_tagged<VariantStorage &&, variant_npos>>,
            type_traits::properties::is_nothrow_invocable<Fn, variant_tagged_ref_t<VariantStorage, Idxs>>...>;

    template <std::size_t Idx, typename Fn, typename VariantStorage>
    RAINY_NODISCARD constexpr variant_raw_visit_t<Fn, VariantStorage> variant_raw_visit_dispatch(
        Fn &&func,
        VariantStorage
            &&variant) noexcept(type_traits::properties::is_nothrow_invocable_r_v<Fn, variant_tagged_ref_t<VariantStorage, Idx>>) {
        return static_cast<Fn &&>(func)(
            variant_tagged_ref_t<VariantStorage, Idx>{variant_raw_get<Idx>(static_cast<VariantStorage &&>(variant))});
    }

    template <typename Fn, typename VariantStorage,
              typename Indices =
                  type_traits::helper::make_index_sequence<type_traits::modifers::remove_reference_t<VariantStorage>::size>>
    struct variant_raw_dispatch_table;

    // NOLINTBEGIN
    template <typename Fn, typename VariantStorage, std::size_t... Idxs>
    struct variant_raw_dispatch_table<Fn, VariantStorage, type_traits::helper::index_sequence<Idxs...>> {
        using dispatch_t = variant_raw_visit_t<Fn, VariantStorage> (*)(Fn &&, VariantStorage &&) noexcept(
            variant_raw_visit_noexcept<Fn, VariantStorage>);

        static constexpr dispatch_t table[] = {[](Fn &&, VariantStorage &&) -> variant_raw_visit_t<Fn, VariantStorage> {
                                                   core::exceptions::runtime::throw_bad_variant_access();
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
            constexpr std::size_t size = type_traits::modifers::remove_reference_t<VariantStorage>::size;
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
        constexpr std::size_t size = type_traits::modifers::remove_reference_t<VariantStorage>::size;
        constexpr int strategy = size <= 4 ? 1 : size <= 16 ? 2 : size <= 64 ? 3 : -1;
        if (idx == variant_npos) {
            core::exceptions::runtime::throw_bad_variant_access();
        }
        ++idx;
        return variant_raw_visit_impl<strategy>::invoke(idx, static_cast<Fn &&>(func), static_cast<VariantStorage &&>(object));
    }
}

#undef RAINY_VARIANT_VISIT_STAMP
#undef RAINY_VARIANT_CASE

namespace rainy::core::container::implements {
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

        using index_t = type_traits::other_trans::conditional_t<
            (sizeof...(Types) < schar_max_as_size), signed char,
            type_traits::other_trans::conditional_t<(sizeof...(Types) < short_max_as_size), short, int>>;

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
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_copy_constructible<Types>...>) {
            if (right.valueless_by_exception()) {
                this->idx = invalid_index;
                return;
            }
            variant_raw_visit(right.index(), right.storage(), [this](auto &&source) { // NOLINT
                using source_t = type_traits::modifers::remove_cvref_t<decltype(source)>;
                if constexpr (source_t::idx != variant_npos) {
                    utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, source_t::idx>{},
                                                utility::forward<decltype(source.val)>(source.val));
                    this->idx = source_t::idx;
                }
            });
        }

        RAINY_CONSTEXPR20 void construct_from(variant_base &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_move_constructible<Types>...>) {

            if (right.valueless_by_exception()) {
                this->idx = invalid_index;
                return;
            }

            variant_raw_visit(right.index(), utility::move(right).storage(), [&right, this](auto &&source) { // NOLINT
                using source_t = type_traits::modifers::remove_cvref_t<decltype(source)>;
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
            using indexed_value_type = type_traits::modifers::remove_cv_t<
                typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type>;
            if constexpr (Idx != variant_npos && !type_traits::properties::is_trivially_destructible_v<indexed_value_type>) {
                variant_raw_get<Idx>(storage()).~indexed_value_type();
            }
        }

        RAINY_CONSTEXPR20 void destroy() noexcept {
            constexpr bool has_non_trivial_dtor =
                !type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...>;
            if constexpr (has_non_trivial_dtor) {
                if (!valueless_by_exception()) {
                    variant_raw_visit(index(), storage(), [](auto &&ref) noexcept { utility::destroy_at(&ref.val); });
                }
            }
        }

        RAINY_CONSTEXPR20 void swap_with(variant_base &other) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_move_constructible<Types>...,
                                                       type_traits::properties::is_nothrow_swappable<Types>...>) {
            if (this == &other) {
                return;
            }
            if (valueless_by_exception() && other.valueless_by_exception()) {
                return;
            }
            if (index() == other.index() && !valueless_by_exception()) {
                variant_raw_visit(index(), storage(), [&other](auto &&left_ref) { // NOLINT
                    using tagged = type_traits::modifers::remove_cvref_t<decltype(left_ref)>;
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
                type_traits::properties::is_constructible_v<
                    typename type_traits::other_trans::type_at<Idx, type_traits::other_trans::type_list<Types...>>::type, Args...>,
                int> = 0>
        constexpr explicit variant_base(std::in_place_index_t<Idx>, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<
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
    using variant_destroy_layer_t = type_traits::other_trans::conditional_t<
        type_traits::logical_traits::conjunction_v<type_traits::properties::is_trivially_destructible<Types>...>,
        variant_base<Types...>, variant_destroy_layer<Types...>>;
}
// @NODOCEND

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A type-safe union that stores a value of exactly one of the listed types.
     *         The currently held alternative is identified by an index; the variant may also be valueless.
     *
     * @tparam Types The alternative types the variant may hold
     *
     * \lang simp-chinese
     * @brief 一种类型安全的联合，恰好存储所列类型之一的值。
     *         当前持有的备选类型由索引标识；variant 也可能处于无值状态。
     *
     * @tparam Types variant 可能持有的备选类型
     */
    template <typename... Types>
    class variant : implements::variant_destroy_layer_t<Types...> {
    public:
        using base = implements::variant_destroy_layer_t<Types...>;
        using type_list = type_traits::other_trans::type_list<Types...>;
        using base::storage;

        /**
         * \lang english
         * @brief Default constructor. Constructs the variant holding the first alternative.
         *
         * @tparam First The first alternative type
         *
         * \lang simp-chinese
         * @brief 默认构造函数。构造持有第一个备选类型的 variant。
         *
         * @tparam First 第一个备选类型
         */
        template <typename First = type_traits::other_trans::type_at_t<0, type_list>,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<First>, int> = 0>
        constexpr variant() : base(std::in_place_index<0>) {
        }

        /**
         * \lang english
         * @brief Converting constructor. Constructs the variant holding the alternative that best matches the given value.
         *
         * @tparam Ty The type of the value
         * @param object The value used to initialize the held alternative
         *
         * \lang simp-chinese
         * @brief 转换构造函数。构造持有与给定值最匹配的备选类型的 variant。
         *
         * @tparam Ty 值的类型
         * @param object 用于初始化所持备选值的值
         */
        template <typename Ty, type_traits::other_trans::enable_if_t<
                                   sizeof...(Types) != 0 &&
                                       !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<Ty>, type_list> &&
                                       !type_traits::primary_types::is_specialization_v<type_traits::modifers::remove_cvref_t<Ty>,
                                                                                        utility::placeholder_type_t> &&
                                       !utility::is_in_place_index_specialization<type_traits::modifers::remove_cvref_t<Ty>> &&
                                       type_traits::properties::is_constructible_v<implements::variant_init_type<Ty, Types...>, Ty>,
                                   int> = 0>
        constexpr variant(Ty &&object) noexcept( // NOLINT
            type_traits::properties::is_nothrow_constructible_v<implements::variant_init_type<Ty, Types...>, Ty>) :
            base(std::in_place_index<implements::variant_init_index<Ty, Types...>::value>, utility::forward<Ty>(object)) {
        }

        /**
         * \lang english
         * @brief In-place constructor by type. Constructs the alternative of type Ty from the given arguments.
         *
         * @tparam Ty The alternative type to construct
         * @tparam Args The types of the arguments forwarded to the constructor of Ty
         * @param args The arguments forwarded to the constructor of Ty
         *
         * \lang simp-chinese
         * @brief 按类型就地构造函数。使用给定实参构造类型为 Ty 的备选值。
         *
         * @tparam Ty 要构造的备选类型
         * @tparam Args 转发给 Ty 构造函数的实参类型
         * @param args 转发给 Ty 构造函数的实参
         */
        template <typename Ty, typename... Args, typename Idx = type_traits::other_trans::type_find_unique<Ty, type_list>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr explicit variant(std::in_place_type_t<Ty>,
                                   Args &&...args) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) :
            base(std::in_place_index<Idx::value>, utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief In-place constructor by index. Constructs the alternative at index Idx from the given arguments.
         *
         * @tparam Idx The index of the alternative to construct
         * @tparam Args The types of the arguments forwarded to the constructor of the alternative
         * @param args The arguments forwarded to the constructor of the alternative
         *
         * \lang simp-chinese
         * @brief 按索引就地构造函数。使用给定实参构造索引为 Idx 的备选值。
         *
         * @tparam Idx 要构造的备选值的索引
         * @tparam Args 转发给备选值构造函数的实参类型
         * @param args 转发给备选值构造函数的实参
         */
        template <std::size_t Idx, typename... Args, typename Type = type_traits::other_trans::type_at_t<Idx, type_list>,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Type, Args...> &&
                          type_traits::other_trans::type_find_unique<Type, type_traits::other_trans::type_list<Types...>>::value !=
                              base::invalid_index /* 防止类型双关 */,
                      int> = 0>
        constexpr explicit variant(std::in_place_index_t<Idx>,
                                   Args &&...args) noexcept(type_traits::properties::is_nothrow_constructible_v<Type, Args...>) :
            base(std::in_place_index<Idx>, utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief In-place constructor by type with an initializer list.
         *
         * @tparam Ty The alternative type to construct
         * @tparam U The type of the elements of the initializer list
         * @tparam Args The types of the remaining arguments forwarded to the constructor of Ty
         * @param il The initializer list passed to the constructor of Ty
         * @param args The remaining arguments forwarded to the constructor of Ty
         *
         * \lang simp-chinese
         * @brief 带初始化器列表的按类型就地构造函数。
         *
         * @tparam Ty 要构造的备选类型
         * @tparam U 初始化器列表元素的类型
         * @tparam Args 转发给 Ty 构造函数的其余实参类型
         * @param il 传递给 Ty 构造函数的初始化器列表
         * @param args 转发给 Ty 构造函数的其余实参
         */
        template <typename Ty, typename U, typename... Args,
                  typename Idx = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index &&
                          type_traits::properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>,
                      int> = 0>
        constexpr explicit variant(std::in_place_type_t<Ty>, std::initializer_list<U> il, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, std::initializer_list<U> &, Args...>) :
            base(std::in_place_index<Idx::value>, il, utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief In-place constructor by index with an initializer list.
         *
         * @tparam Idx The index of the alternative to construct
         * @tparam U The type of the elements of the initializer list
         * @tparam Args The types of the remaining arguments forwarded to the constructor of the alternative
         * @param il The initializer list passed to the constructor of the alternative
         * @param args The remaining arguments forwarded to the constructor of the alternative
         *
         * \lang simp-chinese
         * @brief 带初始化器列表的按索引就地构造函数。
         *
         * @tparam Idx 要构造的备选值的索引
         * @tparam U 初始化器列表元素的类型
         * @tparam Args 转发给备选值构造函数的其余实参类型
         * @param il 传递给备选值构造函数的初始化器列表
         * @param args 转发给备选值构造函数的其余实参
         */
        template <std::size_t Idx, typename U, typename... Args,
                  typename Ty = type_traits::other_trans::type_at_t<Idx, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>, int> = 0>
        constexpr explicit variant(std::in_place_index_t<Idx>, std::initializer_list<U> il, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, std::initializer_list<U> &, Args...>) :
            base(std::in_place_index<Idx>, il, utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief Copy constructor. Copies the held alternative from the source variant.
         *
         * @param right The source variant to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。从源 variant 拷贝所持有的备选值。
         *
         * @param right 要拷贝的源 variant
         */
        constexpr variant(const variant &right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_copy_constructible<Types>...>) {
            this->construct_from(right);
        }

        /**
         * \lang english
         * @brief Move constructor. Moves the held alternative from the source variant.
         *
         * @param right The source variant to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。从源 variant 移动所持有的备选值。
         *
         * @param right 要移动的源 variant
         */
        constexpr variant(variant &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_move_constructible<Types>...>) {
            this->construct_from(utility::move(right));
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the held alternative if present.
         *
         * \lang simp-chinese
         * @brief 析构函数。若存在所持有的备选值则销毁它。
         */
        ~variant() = default;

        /**
         * \lang english
         * @brief Copy assignment operator. Copies the held alternative from the source variant.
         *
         * @param right The source variant to copy from
         * @return Reference to this variant
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。从源 variant 拷贝所持有的备选值。
         *
         * @param right 要拷贝的源 variant
         * @return 此 variant 的引用
         */
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
                    using tagged = type_traits::modifers::remove_cvref_t<decltype(right_ref)>;
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

        /**
         * \lang english
         * @brief Move assignment operator. Moves the held alternative from the source variant.
         *
         * @param right The source variant to move from
         * @return Reference to this variant
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。从源 variant 移动所持有的备选值。
         *
         * @param right 要移动的源 variant
         * @return 此 variant 的引用
         */
        constexpr variant &operator=(variant &&right) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_move_constructible<Types>...,
                                                       type_traits::properties::is_nothrow_move_assignable<Types>...>) {
            if (this == &right) {
                return *this;
            }
            if (right.valueless_by_exception()) {
                base::destroy();
                this->idx = static_cast<typename base::index_t>(variant_npos);
                return *this;
            }
            if (this->index() == right.index()) {
                implements::variant_raw_visit(right.index(), utility::move(right).storage(), [this](auto &&right_ref) { // NOLINT
                    using tagged = type_traits::modifers::remove_cvref_t<decltype(right_ref)>;
                    if constexpr (tagged::idx != variant_npos) {
                        implements::variant_raw_get<tagged::idx>(this->storage()) = utility::move(right_ref.val);
                    }
                });
            } else {
                base::destroy();
                this->idx = static_cast<typename base::index_t>(variant_npos);
                this->construct_from(utility::move(right));
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Converting assignment operator. Assigns a value to the variant, replacing the held alternative.
         *
         * @tparam Ty The type of the value
         * @param object The value to assign
         * @return Reference to this variant
         *
         * \lang simp-chinese
         * @brief 转换赋值运算符。将值赋给 variant，替换所持有的备选值。
         *
         * @tparam Ty 值的类型
         * @param object 要赋的值
         * @return 此 variant 的引用
         */
        template <typename Ty>
        constexpr variant &operator=(Ty &&object) {
            try {
                variant tmp(utility::forward<Ty>(object));
                base::destroy();
                this->idx = static_cast<typename base::index_t>(variant_npos);
                this->construct_from(utility::move(tmp));
            } catch (...) {
                base::destroy();
                this->idx = static_cast<typename base::index_t>(variant_npos);
                throw;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Constructs a new alternative of type Ty in place, destroying the previous one.
         *
         * @tparam Ty The alternative type to construct
         * @tparam Args The types of the arguments forwarded to the constructor of Ty
         * @param args The arguments forwarded to the constructor of Ty
         * @return A reference to the newly constructed alternative
         *
         * \lang simp-chinese
         * @brief 就地构造类型为 Ty 的新备选值，销毁先前持有的值。
         *
         * @tparam Ty 要构造的备选类型
         * @tparam Args 转发给 Ty 构造函数的实参类型
         * @param args 转发给 Ty 构造函数的实参
         * @return 新构造备选值的引用
         */
        template <typename Ty, typename... Args,
                  typename Idx = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index && type_traits::properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr Ty &emplace(Args &&...args) {
            return emplace<Idx::value>(utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Constructs a new alternative of type Ty in place from an initializer list and arguments.
         *
         * @tparam Ty The alternative type to construct
         * @tparam U The type of the elements of the initializer list
         * @tparam Args The types of the remaining arguments forwarded to the constructor of Ty
         * @param il The initializer list passed to the constructor of Ty
         * @param args The remaining arguments forwarded to the constructor of Ty
         * @return A reference to the newly constructed alternative
         *
         * \lang simp-chinese
         * @brief 使用初始化器列表和实参就地构造类型为 Ty 的新备选值。
         *
         * @tparam Ty 要构造的备选类型
         * @tparam U 初始化器列表元素的类型
         * @tparam Args 转发给 Ty 构造函数的其余实参类型
         * @param il 传递给 Ty 构造函数的初始化器列表
         * @param args 转发给 Ty 构造函数的其余实参
         * @return 新构造备选值的引用
         */
        template <typename Ty, typename U, typename... Args,
                  typename Idx = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      Idx::value != base::invalid_index &&
                          type_traits::properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>,
                      int> = 0>
        constexpr Ty &emplace(std::initializer_list<U> il, Args &&...args) {
            return emplace<Idx::value>(il, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Constructs a new alternative at index Idx in place, destroying the previous one.
         *
         * @tparam Idx The index of the alternative to construct
         * @tparam Args The types of the arguments forwarded to the constructor of the alternative
         * @param args The arguments forwarded to the constructor of the alternative
         * @return A reference to the newly constructed alternative
         *
         * \lang simp-chinese
         * @brief 就地构造索引为 Idx 的新备选值，销毁先前持有的值。
         *
         * @tparam Idx 要构造的备选值的索引
         * @tparam Args 转发给备选值构造函数的实参类型
         * @param args 转发给备选值构造函数的实参
         * @return 新构造备选值的引用
         */
        template <std::size_t Idx, typename... Args,
                  typename Ty = type_traits::other_trans::type_at_t<Idx, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...>, int> = 0>
        constexpr Ty &emplace(Args &&...args) {
            base::destroy();
            this->idx = static_cast<typename base::index_t>(variant_npos);
            utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, Idx>{},
                                        utility::forward<Args>(args)...);
            this->set_index(Idx);
            return implements::variant_raw_get<Idx>(this->storage());
        }

        /**
         * \lang english
         * @brief Constructs a new alternative at index Idx in place from an initializer list and arguments.
         *
         * @tparam Idx The index of the alternative to construct
         * @tparam U The type of the elements of the initializer list
         * @tparam Args The types of the remaining arguments forwarded to the constructor of the alternative
         * @param il The initializer list passed to the constructor of the alternative
         * @param args The remaining arguments forwarded to the constructor of the alternative
         * @return A reference to the newly constructed alternative
         *
         * \lang simp-chinese
         * @brief 使用初始化器列表和实参就地构造索引为 Idx 的新备选值。
         *
         * @tparam Idx 要构造的备选值的索引
         * @tparam U 初始化器列表元素的类型
         * @tparam Args 转发给备选值构造函数的其余实参类型
         * @param il 传递给备选值构造函数的初始化器列表
         * @param args 转发给备选值构造函数的其余实参
         * @return 新构造备选值的引用
         */
        template <std::size_t Idx, typename U, typename... Args,
                  typename Ty = type_traits::other_trans::type_at_t<Idx, type_traits::other_trans::type_list<Types...>>,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, std::initializer_list<U> &, Args...>, int> = 0>
        constexpr Ty &emplace(std::initializer_list<U> il, Args &&...args) {
            base::destroy();
            this->idx = static_cast<typename base::index_t>(variant_npos);
            utility::construct_in_place(this->storage(), type_traits::helper::integral_constant<std::size_t, Idx>{}, il,
                                        utility::forward<Args>(args)...);
            this->set_index(Idx);
            return implements::variant_raw_get<Idx>(this->storage());
        }

        /**
         * \lang english
         * @brief Swaps the held alternatives of two variants.
         *
         * @param other The variant to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个 variant 所持有的备选值。
         *
         * @param other 要与之交换的 variant
         */
        constexpr void swap(variant &other) noexcept(
            type_traits::logical_traits::conjunction_v<type_traits::properties::is_nothrow_move_constructible<Types>...,
                                                       type_traits::properties::is_nothrow_swappable<Types>...>) {
            this->swap_with(other);
        }

        using base::index;
        using base::valueless_by_exception;
    };
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Accesses the alternative at the given index. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Index The index of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A reference to the alternative at the given index
     *
     * \lang simp-chinese
     * @brief 访问给定索引处的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Index 要访问的备选值的索引
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 给定索引处备选值的引用
     */
    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(variant<Types...> &var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            core::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(var.storage());
    }

    /**
     * \lang english
     * @brief Accesses the alternative at the given index of a const variant. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Index The index of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A const reference to the alternative at the given index
     *
     * \lang simp-chinese
     * @brief 访问 const variant 中给定索引处的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Index 要访问的备选值的索引
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 给定索引处备选值的常量引用
     */
    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(const variant<Types...> &var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            core::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(var.storage());
    }

    /**
     * \lang english
     * @brief Accesses the alternative at the given index of an rvalue variant. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Index The index of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return An rvalue reference to the alternative at the given index
     *
     * \lang simp-chinese
     * @brief 访问右值 variant 中给定索引处的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Index 要访问的备选值的索引
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 给定索引处备选值的右值引用
     */
    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(variant<Types...> &&var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            core::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(utility::move(var).storage());
    }

    /**
     * \lang english
     * @brief Accesses the alternative at the given index of a const rvalue variant. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Index The index of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A const rvalue reference to the alternative at the given index
     *
     * \lang simp-chinese
     * @brief 访问 const 右值 variant 中给定索引处的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Index 要访问的备选值的索引
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 给定索引处备选值的常量右值引用
     */
    template <std::size_t Index, typename... Types>
    constexpr decltype(auto) get(const variant<Types...> &&var) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (var.index() != Index) {
            core::exceptions::runtime::throw_bad_variant_access();
        }
        return implements::variant_raw_get<Index>(utility::move(var).storage());
    }

    /**
     * \lang english
     * @brief Returns a pointer to the alternative at the given index, or nullptr if the variant holds a different alternative.
     *
     * @tparam Index The index of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A pointer to the alternative, or nullptr
     *
     * \lang simp-chinese
     * @brief 返回指向给定索引处备选值的指针；若 variant 持有不同的备选值则返回 nullptr。
     *
     * @tparam Index 要访问的备选值的索引
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 指向备选值的指针；若不符合则返回 nullptr
     */
    template <std::size_t Index, typename... Types>
    constexpr auto get_if(variant<Types...> *var) noexcept
        -> decltype(&implements::variant_raw_get<Index>(std::declval<variant<Types...> &>().storage())) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (!var || var->index() != Index) {
            return nullptr;
        }
        return &implements::variant_raw_get<Index>(var->storage());
    }

    /**
     * \lang english
     * @brief Returns a const pointer to the alternative at the given index, or nullptr if the variant holds a different alternative.
     *
     * @tparam Index The index of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A const pointer to the alternative, or nullptr
     *
     * \lang simp-chinese
     * @brief 返回指向 const variant 中给定索引处备选值的常量指针；若 variant 持有不同的备选值则返回 nullptr。
     *
     * @tparam Index 要访问的备选值的索引
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 指向备选值的常量指针；若不符合则返回 nullptr
     */
    template <std::size_t Index, typename... Types>
    constexpr auto get_if(const variant<Types...> *var) noexcept
        -> decltype(&implements::variant_raw_get<Index>(std::declval<const variant<Types...> &>().storage())) {
        static_assert(Index < sizeof...(Types), "Index out of bounds");
        if (!var || var->index() != Index) {
            return nullptr;
        }
        return &implements::variant_raw_get<Index>(var->storage());
    }

    /**
     * \lang english
     * @brief Checks whether the variant currently holds the alternative of the given type.
     *
     * @tparam Ty The type to check for
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return true if the variant holds the alternative of type Ty, false otherwise
     *
     * \lang simp-chinese
     * @brief 检查 variant 当前是否持有给定类型的备选值。
     *
     * @tparam Ty 要检查的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 若 variant 持有类型为 Ty 的备选值则返回 true，否则返回 false
     */
    template <typename Ty, typename... Types>
    constexpr bool holds_alternative(const variant<Types...> &var) noexcept {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return var.index() == index;
    }
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Accesses the alternative of the given type. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Ty The type of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A reference to the alternative of type Ty
     *
     * \lang simp-chinese
     * @brief 访问给定类型的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Ty 要访问的备选值的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 类型为 Ty 的备选值的引用
     */
    template <typename Ty, typename... Types>
    constexpr Ty &get(variant<Types...> &var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(var);
    }

    /**
     * \lang english
     * @brief Accesses the alternative of the given type of an rvalue variant. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Ty The type of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return An rvalue reference to the alternative of type Ty
     *
     * \lang simp-chinese
     * @brief 访问右值 variant 中给定类型的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Ty 要访问的备选值的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 类型为 Ty 的备选值的右值引用
     */
    template <typename Ty, typename... Types>
    constexpr Ty &&get(variant<Types...> &&var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(utility::move(var));
    }

    /**
     * \lang english
     * @brief Accesses the alternative of the given type of a const variant. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Ty The type of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A const reference to the alternative of type Ty
     *
     * \lang simp-chinese
     * @brief 访问 const variant 中给定类型的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Ty 要访问的备选值的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 类型为 Ty 的备选值的常量引用
     */
    template <typename Ty, typename... Types>
    constexpr const Ty &get(const variant<Types...> &var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(var);
    }

    /**
     * \lang english
     * @brief Accesses the alternative of the given type of a const rvalue variant. Throws bad_variant_access if the variant holds a different alternative.
     *
     * @tparam Ty The type of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A const rvalue reference to the alternative of type Ty
     *
     * \lang simp-chinese
     * @brief 访问 const 右值 variant 中给定类型的备选值。若 variant 持有不同的备选值则抛出 bad_variant_access。
     *
     * @tparam Ty 要访问的备选值的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 类型为 Ty 的备选值的常量右值引用
     */
    template <typename Ty, typename... Types>
    constexpr const Ty &&get(const variant<Types...> &&var) {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get<index>(utility::move(var));
    }

    /**
     * \lang english
     * @brief Returns a pointer to the alternative of the given type, or nullptr if the variant holds a different alternative.
     *
     * @tparam Ty The type of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A pointer to the alternative, or nullptr
     *
     * \lang simp-chinese
     * @brief 返回指向给定类型备选值的指针；若 variant 持有不同的备选值则返回 nullptr。
     *
     * @tparam Ty 要访问的备选值的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 指向备选值的指针；若不符合则返回 nullptr
     */
    template <typename Ty, typename... Types>
    constexpr auto *get_if(variant<Types...> *var) noexcept {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get_if<index>(var);
    }

    /**
     * \lang english
     * @brief Returns a const pointer to the alternative of the given type, or nullptr if the variant holds a different alternative.
     *
     * @tparam Ty The type of the alternative to access
     * @tparam Types The alternative types of the variant
     * @param var The variant
     * @return A const pointer to the alternative, or nullptr
     *
     * \lang simp-chinese
     * @brief 返回指向 const variant 中给定类型备选值的常量指针；若 variant 持有不同的备选值则返回 nullptr。
     *
     * @tparam Ty 要访问的备选值的类型
     * @tparam Types variant 的备选类型
     * @param var variant
     * @return 指向备选值的常量指针；若不符合则返回 nullptr
     */
    template <typename Ty, typename... Types>
    constexpr const auto *get_if(const variant<Types...> *var) noexcept {
        constexpr auto index = type_traits::other_trans::type_find_unique<Ty, type_traits::other_trans::type_list<Types...>>::value;
        static_assert(index != variant_npos, "Type not found in variant");
        return get_if<index>(var);
    }
}

namespace rainy::core::container::implements {
    // NOLINTBEGIN
    template <typename Visitor, typename Variant>
    constexpr decltype(auto) visit_single_impl(Visitor &&visitor, Variant &&var) {
        if (var.valueless_by_exception()) {
            core::exceptions::runtime::throw_bad_variant_access();
        }
        return variant_raw_visit(var.index(), utility::forward<Variant>(var).storage(),
                                 [&visitor](auto &&tagged_ref) -> decltype(auto) {
                                     using tagged_type = type_traits::modifers::remove_reference_t<decltype(tagged_ref)>;
                                     if constexpr (tagged_type::idx != variant_npos) {
                                         return utility::invoke(utility::forward<Visitor>(visitor),
                                                                utility::forward<decltype(tagged_ref.val)>(tagged_ref.val));
                                     } else {
                                         core::exceptions::runtime::throw_bad_variant_access();
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

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Invokes a callable with the currently held alternative of each variant.
     *
     * @tparam Visitor The type of the callable
     * @tparam Variants The types of the variants to visit
     * @param visitor The callable invoked with the held alternatives
     * @param variants The variants to visit
     * @return The result of invoking the callable
     *
     * \lang simp-chinese
     * @brief 使用每个 variant 当前持有的备选值调用可调用对象。
     *
     * @tparam Visitor 可调用对象的类型
     * @tparam Variants 要访问的 variant 的类型
     * @param visitor 以持有的备选值调用的可调用对象
     * @param variants 要访问的 variant
     * @return 调用可调用对象的结果
     */
    template <typename Visitor, typename... Variants>
    constexpr decltype(auto) visit(Visitor &&visitor, Variants &&...variants) {
        if constexpr (sizeof...(Variants) == 1) {
            return implements::visit_single_impl(utility::forward<Visitor>(visitor), utility::forward<Variants>(variants)...);
        } else {
            return implements::visit_multi_impl(utility::forward<Visitor>(visitor), utility::forward<Variants>(variants)...);
        }
    }

    /**
     * \lang english
     * @brief Invokes a callable with the currently held alternative of each variant and casts the result to Rx.
     *
     * @tparam Rx The return type to cast the result to
     * @tparam Visitor The type of the callable
     * @tparam Variants The types of the variants to visit
     * @param visitor The callable invoked with the held alternatives
     * @param variants The variants to visit
     * @return The result of invoking the callable, cast to Rx
     *
     * \lang simp-chinese
     * @brief 使用每个 variant 当前持有的备选值调用可调用对象，并将结果转换为 Rx。
     *
     * @tparam Rx 结果要转换成的返回类型
     * @tparam Visitor 可调用对象的类型
     * @tparam Variants 要访问的 variant 的类型
     * @param visitor 以持有的备选值调用的可调用对象
     * @param variants 要访问的 variant
     * @return 调用可调用对象的结果，转换为 Rx
     */
    template <typename Rx, typename Visitor, typename... Variants>
    constexpr Rx visit(Visitor &&visitor, Variants &&...variants) {
        return static_cast<Rx>(visit(utility::forward<Visitor>(visitor), utility::forward<Variants>(variants)...));
    }
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Compares two variants for equality.
     *
     * @tparam Types The alternative types of the variants
     * @param left The left variant
     * @param right The right variant
     * @return true if both variants are valueless, or hold the same alternative and their values are equal
     *
     * \lang simp-chinese
     * @brief 比较两个 variant 是否相等。
     *
     * @tparam Types variant 的备选类型
     * @param left 左侧 variant
     * @param right 右侧 variant
     * @return 若两个 variant 都无值，或持有相同的备选值且值相等则返回 true
     */
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
            using tagged = type_traits::modifers::remove_cvref_t<decltype(left_ref)>;
            if constexpr (tagged::idx != variant_npos) {
                result = (left_ref.val == implements::variant_raw_get<tagged::idx>(right.storage()));
            }
        });
        return result;
    }

    /**
     * \lang english
     * @brief Compares two variants for inequality.
     *
     * @tparam Types The alternative types of the variants
     * @param left The left variant
     * @param right The right variant
     * @return true if the variants are not equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较两个 variant 是否不相等。
     *
     * @tparam Types variant 的备选类型
     * @param left 左侧 variant
     * @param right 右侧 variant
     * @return 若两个 variant 不相等则返回 true，否则返回 false
     */
    template <typename... Types>
    constexpr bool operator!=(const variant<Types...> &left, const variant<Types...> &right) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Compares two variants lexicographically with operator<.
     *
     * @tparam Types The alternative types of the variants
     * @param left The left variant
     * @param right The right variant
     * @return true if left compares less than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator< 比较两个 variant。
     *
     * @tparam Types variant 的备选类型
     * @param left 左侧 variant
     * @param right 右侧 variant
     * @return 若 left 小于 right 则返回 true，否则返回 false
     */
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
            using tagged = type_traits::modifers::remove_cvref_t<decltype(left_ref)>;
            if constexpr (tagged::idx != variant_npos) {
                result = (left_ref.val < implements::variant_raw_get<tagged::idx>(right.storage()));
            }
        });
        return result;
    }

    /**
     * \lang english
     * @brief Compares two variants lexicographically with operator>.
     *
     * @tparam Types The alternative types of the variants
     * @param left The left variant
     * @param right The right variant
     * @return true if left compares greater than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator> 比较两个 variant。
     *
     * @tparam Types variant 的备选类型
     * @param left 左侧 variant
     * @param right 右侧 variant
     * @return 若 left 大于 right 则返回 true，否则返回 false
     */
    template <typename... Types>
    constexpr bool operator>(const variant<Types...> &left, const variant<Types...> &right) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares two variants lexicographically with operator<=.
     *
     * @tparam Types The alternative types of the variants
     * @param left The left variant
     * @param right The right variant
     * @return true if left compares less than or equal to right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator<= 比较两个 variant。
     *
     * @tparam Types variant 的备选类型
     * @param left 左侧 variant
     * @param right 右侧 variant
     * @return 若 left 小于等于 right 则返回 true，否则返回 false
     */
    template <typename... Types>
    constexpr bool operator<=(const variant<Types...> &left, const variant<Types...> &right) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares two variants lexicographically with operator>=.
     *
     * @tparam Types The alternative types of the variants
     * @param left The left variant
     * @param right The right variant
     * @return true if left compares greater than or equal to right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator>= 比较两个 variant。
     *
     * @tparam Types variant 的备选类型
     * @param left 左侧 variant
     * @param right 右侧 variant
     * @return 若 left 大于等于 right 则返回 true，否则返回 false
     */
    template <typename... Types>
    constexpr bool operator>=(const variant<Types...> &left, const variant<Types...> &right) {
        return !(left < right);
    }
}

namespace std {
    using rainy::core::container::get; // NOLINT
}

namespace rainy::utility::container {
    using rainy::core::container::visit;
}

namespace rainy::utility::container {
    using rainy::core::container::get;
    using rainy::core::container::get_if;
    using rainy::core::container::variant;
    using rainy::core::container::variant_npos;
    using rainy::core::container::variant_size;
    using rainy::core::container::variant_size_v;
}

#if RAINY_USING_GCC
#pragma GCC diagnostic pop
#endif

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

namespace rainy::container {
    using rainy::core::container::variant;
    using rainy::core::container::variant_size;
    using rainy::core::container::variant_size_v;
    using rainy::core::container::variant_npos;
    using rainy::core::container::get_if;
    using rainy::core::container::holds_alternative;
    using rainy::core::container::get;
    using rainy::core::container::visit;
    using rainy::core::container::operator==;
    using rainy::core::container::operator!=;
    using rainy::core::container::operator<;
    using rainy::core::container::operator>;
    using rainy::core::container::operator<=;
    using rainy::core::container::operator>=;
}


#endif
