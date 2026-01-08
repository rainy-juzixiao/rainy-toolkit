/*
 * Copyright 2025 rainy-juzixiao
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
/**
 * @file bind.hpp
 * @brief 可调用对象绑定库
 */
#ifndef RAINY_FOUNDATION_FUNCTIONAL_BIND_HPP
#define RAINY_FOUNDATION_FUNCTIONAL_BIND_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::functional {
    template <typename Fx, typename... BoundArgs>
    class binder;
}

namespace rainy::foundation::functional::implements {
    template <std::size_t Number>
    struct placeholder_t {};

    template <typename Ty>
    struct is_binder : type_traits::helper::false_type {};

    template <typename Fx, typename... Args>
    struct is_binder<binder<Fx, Args...>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_binder_v = is_binder<type_traits::other_trans::decay_t<Ty>>::value;

    template <typename Ty, typename = void>
    struct binder_arg {
        template <typename UTy, typename CallTuple>
        static constexpr decltype(auto) adjust(UTy &&v, CallTuple &) noexcept {
            return utility::forward<Ty>(v);
        }
    };

    template <std::size_t ArgNumber>
    struct binder_arg<placeholder_t<ArgNumber>> {
        template <typename CallTuple>
        static constexpr decltype(auto) adjust(implements::placeholder_t<ArgNumber>, CallTuple &call_args) noexcept {
            return call_args.template get<ArgNumber - 1>();
        }
    };

    template <typename Ty>
    struct binder_arg<Ty, type_traits::other_trans::enable_if_t<is_binder_v<Ty>>> {
        template <typename Binder, typename CallTuple, std::size_t... I>
        static constexpr decltype(auto) apply_binder(Binder &&binder_obj, CallTuple &call_args,
                                                     type_traits::helper::index_sequence<I...>) noexcept {
            return utility::forward<Binder>(binder_obj)(call_args.template get<I>()...);
        }

        template <typename UTy, typename CallTuple>
        static constexpr decltype(auto) adjust(UTy &&binder_obj, CallTuple &call_args) {
            return apply_binder(utility::forward<UTy>(binder_obj), call_args,
                                type_traits::helper::make_index_sequence<
                                    std::tuple_size_v<type_traits::reference_modify::remove_reference_t<CallTuple>>>{});
        }
    };

    template <typename Fx, typename BoundTuple, typename CallTuple, std::size_t... I>
    constexpr rain_fn call_binder_impl(Fx &f, BoundTuple &bound, CallTuple &&call, type_traits::helper::index_sequence<I...>) noexcept(
        noexcept(utility::invoke(
            utility::declval<Fx &>(),
            binder_arg<type_traits::cv_modify::remove_cvref_t<decltype(utility::declval<BoundTuple &>().template get<I>())>>::adjust(
                utility::declval<BoundTuple &>().template get<I>(), utility::declval<CallTuple &>())...))) -> decltype(auto) {
        return utility::invoke(f, binder_arg<type_traits::cv_modify::remove_cvref_t<decltype(bound.template get<I>())>>::adjust(
                                      bound.template get<I>(), call)...);
    }

#if !RAINY_HAS_CXX20
    template <typename F, typename BoundTuple, typename CallTuple, typename IndexSeq>
    concept can_call_binder = requires(F &f, BoundTuple &bound, CallTuple &&call, IndexSeq seq) {
        { call_binder_impl(f, bound, utility::forward<CallTuple>(call), seq) };
    };
#else
    template <typename Fx, typename BoundTuple, typename CallTuple, typename IndexSeq, typename = void>
    RAINY_CONSTEXPR_BOOL can_call_binder = false;

    template <typename Fx, typename BoundTuple, typename CallTuple, typename IndexSeq>
    RAINY_CONSTEXPR_BOOL
        can_call_binder<Fx, BoundTuple, CallTuple, IndexSeq,
                        type_traits::other_trans::void_t<decltype(call_binder_impl(
                            utility::declval<Fx &>(), utility::declval<BoundTuple &>(),
                            utility::forward<CallTuple>(utility::declval<CallTuple &&>()), utility::declval<IndexSeq>()))>> = true;
#endif
    template <typename Fx, typename BoundTuple, typename CallTuple, typename IndexSeq>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_of_call_binder =
        noexcept(call_binder_impl(utility::declval<Fx &>(), utility::declval<BoundTuple &>(),
                                  utility::forward<CallTuple>(utility::declval<CallTuple &&>()), utility::declval<IndexSeq>()));
}

namespace rainy::foundation::functional::placeholders {
    inline constexpr implements::placeholder_t<1> _1{};
    inline constexpr implements::placeholder_t<2> _2{};
    inline constexpr implements::placeholder_t<3> _3{};
    inline constexpr implements::placeholder_t<4> _4{};
    inline constexpr implements::placeholder_t<5> _5{};
    inline constexpr implements::placeholder_t<6> _6{};
    inline constexpr implements::placeholder_t<7> _7{};
    inline constexpr implements::placeholder_t<8> _8{};
    inline constexpr implements::placeholder_t<9> _9{};
    inline constexpr implements::placeholder_t<10> _10{};
    inline constexpr implements::placeholder_t<11> _11{};
    inline constexpr implements::placeholder_t<12> _12{};
    inline constexpr implements::placeholder_t<13> _13{};
    inline constexpr implements::placeholder_t<14> _14{};
    inline constexpr implements::placeholder_t<15> _15{};
    inline constexpr implements::placeholder_t<16> _16{};
    inline constexpr implements::placeholder_t<17> _17{};
    inline constexpr implements::placeholder_t<18> _18{};
    inline constexpr implements::placeholder_t<19> _19{};
    inline constexpr implements::placeholder_t<20> _20{};
}

namespace rainy::foundation::functional {
    template <typename Fx, typename... BoundArgs>
    class binder {
    public:
        using impl_bound_seq = type_traits::helper::index_sequence_for<BoundArgs...>;

        using func_type = Fx;
        using bound_args = utility::tuple<type_traits::other_trans::decay_t<BoundArgs>...>;

        constexpr binder(Fx func, BoundArgs... args) : pair{func, utility::make_tuple(args...)} {
        }

        constexpr binder(const binder &) = default;
        constexpr binder(binder &&) = default;

        constexpr binder &operator=(const binder &) = default;
        constexpr binder &operator=(binder &&) = default;

#if RAINY_HAS_CXX20
        template <typename... CallArgs>
            requires implements::can_call_binder<func_type, bound_args, utility::tuple<CallArgs...>, impl_bound_seq>
        constexpr decltype(auto) operator()(CallArgs &&...args) noexcept(
            implements::is_nothrow_invocable_of_call_binder<func_type, bound_args, utility::tuple<CallArgs...>, impl_bound_seq>) {
            auto &func = pair.get_first();
            auto &bound = pair.get_second();
            return implements::call_binder_impl(func, bound, utility::forward_as_tuple(utility::forward<CallArgs>(args)...),
                                                impl_bound_seq{});
        }
#else
        template <typename... CallArgs,
                  type_traits::other_trans::enable_if_t<
                      implements::can_call_binder<func_type, bound_args, utility::tuple<CallArgs...>, impl_bound_seq>, int> = 0>
        constexpr decltype(auto) operator()(CallArgs &&...args) noexcept(
            implements::is_nothrow_invocable_of_call_binder<func_type, bound_args, utility::tuple<CallArgs...>, impl_bound_seq>) {
            auto &func = pair.get_first();
            auto &bound = pair.get_second();
            return implements::call_binder_impl(func, bound, utility::forward_as_tuple(utility::forward<CallArgs>(args)...),
                                                impl_bound_seq{});
        }
#endif

    private:
        compressed_pair<Fx, utility::tuple<type_traits::other_trans::decay_t<BoundArgs>...>> pair;
    };

    template <typename Fx, typename... Args>
    binder(Fx, Args...) -> binder<Fx, Args...>;

    /**
     * @brief 将函数和多个参数进行绑定
     * @tparam Fx 由参数推导，函数的类型
     * @tparam Args 要绑定的参数的类型
     * @param fx 要绑定的目标函数
     * @param args 要绑定的参数
     * @return 返回一个函数对象用于调用绑定的表达式
     */
    template <typename Fx, typename... Args>
    rain_fn bind(Fx fx, Args &&...args) -> binder<Fx, Args...> {
        return binder<Fx, Args...>(fx, args...);
    }
}

namespace rainy::foundation::functional::implements {
    template <typename Fx, typename BoundTuple, std::size_t... I, typename... Args>
    constexpr rain_fn call_front_binder_impl(
        Fx &f, BoundTuple &bound, type_traits::helper::index_sequence<I...>,
        Args &&...args) noexcept(noexcept(utility::invoke(utility::declval<Fx &>(),
                                                          utility::declval<BoundTuple &>().template get<I>()...,
                                                          utility::declval<Args>()...))) -> decltype(auto) {
        return utility::invoke(f, bound.template get<I>()..., utility::forward<Args>(args)...);
    }

    template <typename Fx, typename BoundTuple, typename IndexSeq, typename TypeList, typename = void>
    RAINY_CONSTEXPR_BOOL can_call_front_binder = false;

    template <typename Fx, typename BoundTuple, typename IndexSeq, typename... Args>
    RAINY_CONSTEXPR_BOOL can_call_front_binder<
        Fx, BoundTuple, IndexSeq, type_traits::other_trans::type_list<Args...>,
        type_traits::other_trans::void_t<decltype(call_front_binder_impl(
            utility::declval<Fx &>(), utility::declval<BoundTuple &>(), utility::declval<IndexSeq>(), utility::declval<Args>()...))>> =
        true;

    template <typename Fx, typename BoundTuple, typename IndexSeq, typename... Args>
    RAINY_CONSTEXPR_BOOL is_nothrow_invocable_of_call_front_binder = noexcept(call_front_binder_impl(
        utility::declval<Fx &>(), utility::declval<BoundTuple &>(), utility::declval<IndexSeq>(), utility::declval<Args>()...));
}

namespace rainy::foundation::functional {
    template <typename Fx, typename... BoundArgs>
    class binder_front {
    public:
        using impl_bound_seq = type_traits::helper::index_sequence_for<BoundArgs...>;

        using func_type = Fx;
        using bound_args = utility::tuple<type_traits::other_trans::decay_t<BoundArgs>...>;

        template <typename Fx2, typename... BoundArgs2>
        constexpr binder_front(Fx2 &&f, BoundArgs2 &&...bound_args) :
            pair(utility::forward<Fx2>(f), utility::make_tuple(utility::forward<BoundArgs2>(bound_args)...)) {
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<
                      implements::can_call_front_binder<Fx, bound_args, impl_bound_seq, type_traits::other_trans::type_list<Args...>>,
                      int> = 0>
        constexpr auto operator()(Args &&...args) & noexcept(
            implements::is_nothrow_invocable_of_call_front_binder<Fx, bound_args, impl_bound_seq, Args...>) -> decltype(auto) {
            return implements::call_front_binder_impl(pair.get_first(), pair.get_second(), impl_bound_seq{},
                                                      utility::forward<Args>(args)...);
        }

    private:
        utility::compressed_pair<Fx, bound_args> pair;
    };

    template <typename Fx, typename... BoundArgs>
    binder_front(Fx, BoundArgs...) -> binder_front<Fx, BoundArgs...>;

    template <typename Fx, typename... Args>
    constexpr rain_fn bind_front(Fx &&f, Args &&...args) -> binder_front<Fx, Args...> {
        return binder_front<Fx, Args...>(utility::forward<Fx>(f), utility::forward<Args>(args)...);
    }
}

#endif