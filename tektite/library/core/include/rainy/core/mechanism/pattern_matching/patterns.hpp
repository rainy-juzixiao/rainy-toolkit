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
#ifndef RAINY_CORE_MECHANISM_PATTERN_MATCHING_PATTERNS_HPP
#define RAINY_CORE_MECHANISM_PATTERN_MATCHING_PATTERNS_HPP
#include <cassert>
#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <rainy/core/collections/array.hpp>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/container/variant.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/mechanism/pattern_matching/expression.hpp>
#include <rainy/core/mechanism/pattern_matching/pre.hpp>
#include <rainy/core/type_traits.hpp>

namespace std {
    /// @hide
    template <typename Tyy1, typename Tyy2>
    struct tuple_size<rainy::core::container::pair<Tyy1, Tyy2>> : integral_constant<size_t, 2> {};

    /// @hide
    template <size_t Index, typename Tyy1, typename Tyy2>
    struct tuple_element<Index, rainy::core::container::pair<Tyy1, Tyy2>> {
        static_assert(Index < 2, "pair index is within bounds");
        using type = std::conditional_t<Index == 0, Tyy1, Tyy2>;
    };
}

namespace rainy::core::mechanism::pattern_matching::implements {
    template <typename Iter, typename Sential = Iter>
    class sub_range {
    public:
        constexpr sub_range(Iter const begin, Sential const end) : begin_{begin}, end_{end} {
        }

        constexpr sub_range(sub_range const &other) : begin_{other.begin()}, end_{other.end()} {
        }

        sub_range &operator=(sub_range const &other) {
            begin_ = other.begin();
            end_ = other.end();
            return *this;
        }

        std::size_t size() const {
            return static_cast<std::size_t>(utility::distance(begin_, end_));
        }

        rain_fn begin() const -> auto {
            return begin_;
        }

        rain_fn end() const -> auto {
            return end_;
        }

    private:
        Iter begin_;
        Sential end_;
    };

    template <typename Iter, typename Sential>
    constexpr rain_fn make_subrange(Iter begin, Sential end) -> auto {
        return sub_range<Iter, Sential>{begin, end};
    }

    template <typename RangeTyype>
    struct iter_underlying_type {
        using begin_type = decltype(utility::begin(utility::declval<RangeTyype &>()));
        using end_type = decltype(utility::end(utility::declval<RangeTyype &>()));
    };

    template <typename element_t, std::size_t size>
    struct iter_underlying_type<collections::array<element_t, size>> {
        using begin_type = decltype(&*utility::begin(utility::declval<collections::array<element_t, size> &>()));
        using end_type = begin_type;
    };

    template <typename element_t, std::size_t size>
    struct iter_underlying_type<collections::array<element_t, size> const> {
        using begin_type = decltype(&*utility::begin(utility::declval<collections::array<element_t, size> const &>()));
        using end_type = begin_type;
    };

    template <typename RangeTyype>
    using subrange_t =
        sub_range<typename iter_underlying_type<RangeTyype>::begin_type, typename iter_underlying_type<RangeTyype>::end_type>;

    template <typename Iter, typename Sential>
    bool operator==(sub_range<Iter, Sential> const &left, sub_range<Iter, Sential> const &right) {
        return left.size() == right.size() && algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
    }

    template <typename Ty, typename... Types>
    struct within_types {
        static constexpr auto value = (type_traits::type_relations::is_same_v<Ty, Types> || ...);
    };

    template <typename Ty, typename Tuple>
    struct prepend_unique;

    template <typename Ty, typename... Types>
    struct prepend_unique<Ty, container::tuple<Types...>> {
        static constexpr auto unique = !within_types<Ty, Types...>::value;

        using type = type_traits::other_trans::conditional_t<unique, container::tuple<Ty, Types...>, container::tuple<Types...>>;
    };

    template <typename Ty, typename Tuple>
    using prepend_unique_t = typename prepend_unique<Ty, Tuple>::type;

    template <typename Tuple>
    struct unique;

    template <typename... Types>
    using unique_t = typename unique<container::tuple<Types...>>::type;

    template <>
    struct unique<container::tuple<>> {
        using type = container::tuple<>;
    };

    template <typename Ty, typename... Types>
    struct unique<container::tuple<Ty, Types...>> {
        using type = prepend_unique_t<Ty, unique_t<Types...>>;
    };

    static_assert(type_traits::type_relations::is_same_v<container::tuple<int32_t>, unique_t<int32_t, int32_t>>);
    static_assert(type_traits::type_relations::is_same_v<container::tuple<container::tuple<>, int32_t>,
                                                         unique_t<int32_t, container::tuple<>, int32_t>>);

    template <typename Tyy>
    struct is_pair : type_traits::helper::false_type {};

    template <typename Tyy1, typename Tyy2>
    struct is_pair<container::pair<Tyy1, Tyy2>> : type_traits::helper::true_type {};

    template <typename Tyy>
    struct is_rainy_tuple : type_traits::helper::false_type {};

    template <typename... Types>
    struct is_rainy_tuple<container::tuple<Types...>> : type_traits::helper::true_type {};

    template <std::size_t index, typename Tuple>
    constexpr rain_fn tuple_get(Tuple &&t) noexcept -> decltype(auto) {
        using raw_t = type_traits::modifers::remove_cv_t<type_traits::modifers::remove_reference_t<Tuple>>;
        if constexpr (is_pair<raw_t>::value) {
            if constexpr (index == 0) {
                return static_cast<Tuple &&>(t).first;
            } else {
                return static_cast<Tuple &&>(t).second;
            }
        } else if constexpr (is_rainy_tuple<raw_t>::value) {
            return static_cast<Tuple &&>(t).template get<index>();
        } else {
            using std::get;
            return get<index>(static_cast<Tuple &&>(t));
        }
    }

    template <std::size_t start, typename Tuple, std::size_t... Iter>
    constexpr rain_fn subtuple_impl(Tuple &&t, type_traits::helper::index_sequence<Iter...>) -> decltype(auto) {
        return container::forward_as_tuple(tuple_get<start + Iter>(utility::forward<Tuple>(t))...);
    }

    template <std::size_t start, std::size_t end, typename Tuple>
    constexpr rain_fn subtuple(Tuple &&t) -> decltype(auto) {
        constexpr auto tuple_length = std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>;
        static_assert(start <= end);
        static_assert(end <= tuple_length);
        return subtuple_impl<start>(utility::forward<Tuple>(t), type_traits::helper::make_index_sequence<end - start>{});
    }

    template <std::size_t start, typename Tuple>
    constexpr rain_fn drop(Tuple &&t) -> decltype(auto) {
        constexpr auto tuple_length = std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>;
        static_assert(start <= tuple_length);
        return subtuple<start, tuple_length>(utility::forward<Tuple>(t));
    }

    template <std::size_t len, typename Tuple>
    constexpr rain_fn take(Tuple &&t) -> decltype(auto) {
        constexpr auto tuple_length = std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>;
        static_assert(len <= tuple_length);
        return subtuple<0, len>(utility::forward<Tuple>(t));
    }

    template <typename Fx, typename Tuple, std::size_t... Iter>
    constexpr rain_fn apply_impl(Fx &&f, Tuple &&t, type_traits::helper::index_sequence<Iter...>) -> decltype(auto) {
        return utility::forward<Fx>(f)(tuple_get<Iter>(utility::forward<Tuple>(t))...);
    }

    template <typename Fx, typename Tuple>
    constexpr rain_fn apply_(Fx &&f, Tuple &&t) -> decltype(auto) {
        constexpr auto tuple_length = std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>;
        return apply_impl(utility::forward<Fx>(f), utility::forward<Tuple>(t),
                          type_traits::helper::make_index_sequence<tuple_length>{});
    }

    template <typename Fx, typename... Args>
    constexpr rain_fn invoke_(Fx &&f, Args &&...args) noexcept(type_traits::properties::is_nothrow_invocable_v<Fx, Args...>)
        -> type_traits::properties::invoke_result_t<Fx, Args...> {
        return utility::forward<Fx>(f)(utility::forward<Args>(args)...);
    }

    constexpr rain_fn tuple_cat_impl() -> auto {
        return container::tuple<>{};
    }

    template <typename Tuple>
    constexpr rain_fn tuple_cat_impl(Tuple &&t) -> auto {
        return utility::forward<Tuple>(t);
    }

    template <typename First, typename... Rest>
    constexpr rain_fn tuple_cat_impl(First &&first, Rest &&...rest) -> auto {
        if constexpr (std::tuple_size_v<type_traits::modifers::remove_reference_t<First>> == 0) {
            return tuple_cat_impl(utility::forward<Rest>(rest)...);
        } else if constexpr (std::tuple_size_v<type_traits::modifers::remove_reference_t<typename type_traits::other_trans::decay_t<
                                 decltype(tuple_cat_impl(utility::forward<Rest>(rest)...))>>> == 0) {
            return utility::forward<First>(first);
        } else {
            return utility::forward<First>(first).concat(tuple_cat_impl(utility::forward<Rest>(rest)...));
        }
    }

    template <typename Ty>
    struct decay_array {
        using type = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_array_v<type_traits::modifers::remove_reference_t<Ty>>,
            type_traits::modifers::remove_extent_t<type_traits::modifers::remove_reference_t<Ty>> *, Ty>;
    };

    template <typename Ty>
    using decay_array_t = typename decay_array<Ty>::type;

    static_assert(type_traits::type_relations::is_same_v<decay_array_t<int32_t[]>, int32_t *>);
    static_assert(type_traits::type_relations::is_same_v<decay_array_t<int32_t const[]>, int32_t const *>);
    static_assert(type_traits::type_relations::is_same_v<decay_array_t<int32_t const &>, int32_t const &>);

    template <typename Ty>
    struct add_const_to_pointer {
        using type = type_traits::other_trans::conditional_t<
            !type_traits::primary_types::is_pointer_v<Ty>, Ty,
            type_traits::modifers::add_pointer_t<type_traits::modifers::add_const_t<type_traits::modifers::remove_pointer_t<Ty>>>>;
    };

    template <typename Ty>
    using add_const_to_pointer_t = typename add_const_to_pointer<Ty>::type;

    static_assert(type_traits::type_relations::is_same_v<add_const_to_pointer_t<void *>, void const *>);
    static_assert(type_traits::type_relations::is_same_v<add_const_to_pointer_t<int32_t>, int32_t>);

    template <typename Pattern>
    using internal_pattern_t = type_traits::modifers::remove_reference_t<add_const_to_pointer_t<decay_array_t<Pattern>>>;

    template <typename Pattern>
    struct pattern_traits;

    template <typename... pattern_pairs>
    struct pattern_pairs_ret_type {
        using ret_type = type_traits::primary_types::common_type_t<typename pattern_pairs::ret_type...>;
    };

    enum class id_process : int32_t {
        cancel,
        confirm
    };

    template <typename Pattern>
    constexpr void process_id(Pattern const &pattern, int32_t depth, id_process id_process_arg) {
        pattern_traits<Pattern>::process_id_impl(pattern, depth, id_process_arg);
    }

    template <typename Tuple>
    struct variant_of;

    template <typename Ty, typename... Types>
    struct variant_of<container::tuple<Ty, Types...>> {
        using type = container::variant<utility::monostate, Ty, Types...>;
    };

    template <typename... Types>
    using uniq_variant = typename variant_of<unique_t<Types...>>::type;

    template <typename... Types>
    struct context {
        using element_t = uniq_variant<Types...>;
        using container_t = collections::array<element_t, sizeof...(Types)>;
        container_t mem_holder_;
        std::size_t size_ = 0;

        template <typename Ty>
        constexpr void emplace_back(Ty &&t) {
            mem_holder_[size_] = utility::forward<Ty>(t);
            ++size_;
        }

        constexpr rain_fn back() -> element_t & {
            return mem_holder_[size_ - 1];
        }
    };

    template <>
    struct context<> {};

    template <typename Ty>
    struct context_trait;

    template <typename... Types>
    struct context_trait<container::tuple<Types...>> {
        using context_t = context<Types...>;
    };

    template <typename Value, typename Pattern, typename context_t>
    constexpr rain_fn match_pattern(Value &&value, Pattern const &pattern, int32_t depth, context_t &context) -> auto {
        auto const result = pattern_traits<Pattern>::match_pattern_impl(utility::forward<Value>(value), pattern, depth, context);
        auto const process = result ? id_process::confirm : id_process::cancel;
        process_id(pattern, depth, process);
        return result;
    }

    template <typename Pattern, typename Func>
    class pattern_pair {
    public:
        using ret_type = type_traits::properties::invoke_result_t<Func>;
        using pattern_t = Pattern;

        constexpr pattern_pair(Pattern const &pattern, Func const &func) : pattern_{pattern}, handler_{func} {
        }
        template <typename Value, typename context_t>
        constexpr bool match_value(Value &&value, context_t &context) const {
            return match_pattern(utility::forward<Value>(value), pattern_, /*depth*/ 0, context);
        }

        constexpr rain_fn execute() const -> auto {
            return handler_();
        }

    private:
        Pattern const pattern_;
        type_traits::other_trans::conditional_t<type_traits::primary_types::is_function_v<Func>, Func const &, Func const> handler_;
    };

    template <typename Pattern, typename Pred>
    class post_check;

    template <typename Pred>
    class when_t {
    public:
        Pred pred_;
    };

    template <typename Pred>
    constexpr rain_fn when(Pred &&pred) -> auto {
        auto p = to_nullary(pred);
        return when_t<decltype(p)>{p};
    }

    template <typename Pattern>
    class pattern_helper {
    public:
        constexpr explicit pattern_helper(Pattern const &pattern) : pattern_{pattern} {
        }

        template <typename Func>
        constexpr rain_fn operator=(Func &&func)->auto {
            auto f = to_nullary(func);
            return pattern_pair<Pattern, decltype(f)>{pattern_, f};
        }

        template <typename Pred>
        constexpr rain_fn operator|(when_t<Pred> const &w)->auto {
            return pattern_helper<post_check<Pattern, Pred>>(post_check(pattern_, w.pred_));
        }

    private:
        Pattern const pattern_;
    };

    template <typename... Patterns>
    class ds_t;

    template <typename... Patterns>
    constexpr rain_fn ds(Patterns const &...patterns) -> ds_t<Patterns...>;

    template <typename Pattern>
    class ooo_binder;

    class pattern_pipable {
    public:
        template <typename Pattern>
        constexpr rain_fn operator|(Pattern const &p) const->auto {
            return pattern_helper<Pattern>{p};
        }

        template <typename Ty>
        constexpr rain_fn operator|(Ty const *p) const->auto {
            return pattern_helper<Ty const *>{p};
        }

        template <typename Pattern>
        constexpr rain_fn operator|(ooo_binder<Pattern> const &p) const->auto {
            return operator|(ds(p));
        }
    };

    constexpr pattern_pipable pattern{};

    template <typename Pattern>
    struct pattern_traits {
        template <typename Value>
        using app_result_tuple = container::tuple<>;

        static constexpr auto nb_id_v = 0;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, Pattern const &pattern, int32_t /* depth */,
                                                    context_t & /*context*/) -> auto {
            return pattern == utility::forward<Value>(value);
        }
        static constexpr void process_id_impl(Pattern const &, int32_t /*depth*/, id_process) {
        }
    };

    template <>
    struct pattern_traits<wildcard> {
        using Pattern = wildcard;

        template <typename Value>
        using app_result_tuple = container::tuple<>;

        static constexpr auto nb_id_v = 0;

        template <typename Value, typename context_t>
        static constexpr bool match_pattern_impl(Value &&, Pattern const &, int32_t, context_t &) {
            return true;
        }
        static constexpr void process_id_impl(Pattern const &, int32_t /*depth*/, id_process) {
        }
    };

    template <typename... Patterns>
    class or_t {
    public:
        constexpr explicit or_t(Patterns const &...patterns) : patterns_{patterns...} {
        }

        constexpr rain_fn patterns() const -> const auto & {
            return patterns_;
        }

    private:
        container::tuple<internal_pattern_t<Patterns>...> patterns_;
    };

    template <typename... Patterns>
    constexpr rain_fn or_(Patterns const &...patterns) -> auto {
        return or_t<Patterns...>{patterns...};
    }

    template <typename... Patterns>
    struct pattern_traits<or_t<Patterns...>> {
        template <typename Value>
        using app_result_tuple = decltype(tuple_cat_impl(typename pattern_traits<Patterns>::template app_result_tuple<Value>{}...));

        static constexpr auto nb_id_v = (pattern_traits<Patterns>::nb_id_v + ... + 0);

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, or_t<Patterns...> const &or_pattern, int32_t depth,
                                                    context_t &context) -> auto {
            constexpr auto pattern_size = sizeof...(Patterns);
            return apply_([&value, depth,
                           &context](auto const &...patterns) { return (match_pattern(value, patterns, depth + 1, context) || ...); },
                          take<pattern_size - 1>(or_pattern.patterns())) ||
                   match_pattern(utility::forward<Value>(value), tuple_get<pattern_size - 1>(or_pattern.patterns()), depth + 1,
                                 context);
        }
        static constexpr void process_id_impl(or_t<Patterns...> const &or_pattern, int32_t depth, id_process id_process_arg) {
            return apply_(
                [depth, id_process_arg](Patterns const &...patterns) { return (process_id(patterns, depth, id_process_arg), ...); },
                or_pattern.patterns());
        }
    };

    template <typename Pred>
    class meet_t : public Pred {
    public:
        using Pred::operator();
    };

    template <typename Pred>
    constexpr rain_fn meet(Pred const &pred) -> auto {
        return meet_t<Pred>{pred};
    }

    template <typename Pred>
    struct pattern_traits<meet_t<Pred>> {
        template <typename Value>
        using app_result_tuple = container::tuple<>;

        static constexpr auto nb_id_v = 0;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, meet_t<Pred> const &meet_pattern, int32_t /* depth */, context_t &)
            -> auto {
            return meet_pattern(utility::forward<Value>(value));
        }
        static constexpr void process_id_impl(meet_t<Pred> const &, int32_t /*depth*/, id_process) {
        }
    };

    template <typename Tyy>
    struct pattern_traits<nullary_t<Tyy>> {
        template <typename Value>
        using app_result_tuple = container::tuple<>;

        static constexpr auto nb_id_v = 0;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, nullary_t<Tyy> const &nullary_pattern, int32_t /* depth */,
                                                    context_t &) -> auto {
            return static_cast<bool>(nullary_pattern(utility::forward<Value>(value)));
        }

        static constexpr void process_id_impl(nullary_t<Tyy> const &, int32_t /*depth*/, id_process) {
        }
    };

    template <typename Unary, typename Pattern>
    class app_t {
    public:
        constexpr app_t(Unary &&unary, Pattern const &pattern) : unary_{utility::forward<Unary>(unary)}, pattern_{pattern} {
        }

        constexpr rain_fn unary() const -> const auto & {
            return unary_;
        }

        constexpr rain_fn pattern() const -> const auto & {
            return pattern_;
        }

    private:
        Unary const unary_;
        internal_pattern_t<Pattern> const pattern_;
    };

    template <typename Unary, typename Pattern>
    constexpr rain_fn app(Unary &&unary, Pattern const &pattern) -> auto {
        return app_t<Unary, Pattern>{utility::forward<Unary>(unary), pattern};
    }

    constexpr auto probe_value = 1;
    static_assert(container::holds_alternative<int32_t const *>(container::variant<utility::monostate, const int32_t *>{
        &probe_value}));

    template <typename Unary, typename Pattern>
    struct pattern_traits<app_t<Unary, Pattern>> {
        template <typename Value>
        using app_result = type_traits::properties::invoke_result_t<Unary, Value>;

#ifdef RAINY_NO_SCALAR_REFERENCES_USED_IN_PATTERNS
        template <typename Value>
        using app_result_cur_tuple =
            type_traits::other_trans::conditional_t<type_traits::primary_types::is_lvalue_reference_v<app_result<Value>> ||
                                                        type_traits::composite_types::is_scalar_v<app_result<Value>>,
                                                    container::tuple<>,
                                                    container::tuple<type_traits::other_trans::decay_t<app_result<Value>>>>;
#else
        template <typename Value>
        using app_result_cur_tuple =
            type_traits::other_trans::conditional_t<type_traits::primary_types::is_lvalue_reference_v<app_result<Value>>,
                                                    container::tuple<>,
                                                    container::tuple<type_traits::other_trans::decay_t<app_result<Value>>>>;
#endif

        template <typename Value>
        using app_result_tuple = decltype(tuple_cat_impl(
            utility::declval<app_result_cur_tuple<Value>>(),
            utility::declval<typename pattern_traits<Pattern>::template app_result_tuple<app_result<Value>>>()));

        static constexpr rain_fn nb_id_v = pattern_traits<Pattern>::nb_id_v;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, app_t<Unary, Pattern> const &app_pattern, int32_t depth,
                                                    context_t &context) -> auto {
            if constexpr (type_traits::type_relations::is_same_v<app_result_cur_tuple<Value>, container::tuple<>>) {
                return match_pattern(utility::forward<app_result<Value>>(invoke_(app_pattern.unary(), value)), app_pattern.pattern(),
                                     depth + 1, context);
            } else {
                context.emplace_back(invoke_(app_pattern.unary(), value));
                decltype(auto) result = container::get<type_traits::other_trans::decay_t<app_result<Value>>>(context.back());
                return match_pattern(utility::forward<app_result<Value>>(result), app_pattern.pattern(), depth + 1, context);
            }
        }
        static constexpr void process_id_impl(app_t<Unary, Pattern> const &app_pattern, int32_t depth, id_process id_process_arg) {
            return process_id(app_pattern.pattern(), depth, id_process_arg);
        }
    };

    template <typename... Patterns>
    class and_t {
    public:
        constexpr explicit and_t(Patterns const &...patterns) : patterns_{patterns...} {
        }

        constexpr rain_fn patterns() const -> const auto & {
            return patterns_;
        }

    private:
        container::tuple<internal_pattern_t<Patterns>...> patterns_;
    };

    template <typename... Patterns>
    constexpr rain_fn and_(Patterns const &...patterns) -> auto {
        return and_t<Patterns...>{patterns...};
    }

    template <typename Tuple>
    struct nb_id_in_tuple;

    template <typename... Patterns>
    struct nb_id_in_tuple<container::tuple<Patterns...>> {
        static constexpr auto nb_id_v = (pattern_traits<type_traits::other_trans::decay_t<Patterns>>::nb_id_v + ... + 0);
    };

    template <typename... Patterns>
    struct pattern_traits<and_t<Patterns...>> {
        template <typename Value>
        using app_result_tuple =
            decltype(tuple_cat_impl(utility::declval<typename pattern_traits<Patterns>::template app_result_tuple<Value>>()...));

        static constexpr auto nb_id_v = (pattern_traits<Patterns>::nb_id_v + ... + 0);

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, and_t<Patterns...> const &and_pattern, int32_t depth,
                                                    context_t &context) -> auto {
            constexpr auto pattern_size = sizeof...(Patterns);
            auto const except_last =
                apply_([&value, depth,
                        &context](auto const &...patterns) { return (match_pattern(value, patterns, depth + 1, context) && ...); },
                       take<pattern_size - 1>(and_pattern.patterns()));

            if constexpr (nb_id_in_tuple<
                              type_traits::other_trans::decay_t<decltype(take<pattern_size - 1>(and_pattern.patterns()))>>::nb_id_v ==
                          0) {
                return except_last && match_pattern(utility::forward<Value>(value),
                                                    tuple_get<pattern_size - 1>(and_pattern.patterns()), depth + 1, context);
            } else {
                return except_last && match_pattern(value, tuple_get<pattern_size - 1>(and_pattern.patterns()), depth + 1, context);
            }
        }

        static constexpr void process_id_impl(and_t<Patterns...> const &and_pattern, int32_t depth, id_process id_process_arg) {
            return apply_(
                [depth, id_process_arg](Patterns const &...patterns) { return (process_id(patterns, depth, id_process_arg), ...); },
                and_pattern.patterns());
        }
    };

    template <typename Pattern>
    class not_t {
    public:
        explicit not_t(Pattern const &pattern) : pattern_{pattern} {
        }
        auto const &pattern() const {
            return pattern_;
        }

    private:
        internal_pattern_t<Pattern> pattern_;
    };

    template <typename Pattern>
    constexpr rain_fn not_(Pattern const &pattern) -> auto {
        return not_t<Pattern>{pattern};
    }

    template <typename Pattern>
    struct pattern_traits<not_t<Pattern>> {
        template <typename Value>
        using app_result_tuple = typename pattern_traits<Pattern>::template app_result_tuple<Value>;

        static constexpr auto nb_id_v = pattern_traits<Pattern>::nb_id_v;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, not_t<Pattern> const &not_pattern, int32_t depth,
                                                    context_t &context) -> auto {
            return !match_pattern(utility::forward<Value>(value), not_pattern.pattern(), depth + 1, context);
        }

        static constexpr void process_id_impl(not_t<Pattern> const &not_pattern, int32_t depth, id_process id_process_arg) {
            process_id(not_pattern.pattern(), depth, id_process_arg);
        }
    };

    template <typename Ptr, typename Value, typename = type_traits::other_trans::void_t<>>
    struct store_pointer : type_traits::helper::false_type {};

    template <typename Tyype>
    using value_variant = type_traits::other_trans::conditional_t<
        type_traits::primary_types::is_lvalue_reference_v<Tyype>, uniq_variant<type_traits::modifers::remove_reference_t<Tyype> *>,
        type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_rvalue_reference_v<Tyype>,
            uniq_variant<type_traits::modifers::remove_reference_t<Tyype>, type_traits::modifers::remove_reference_t<Tyype> *>,
            type_traits::other_trans::conditional_t<
                type_traits::properties::is_abstract_v<type_traits::modifers::remove_reference_t<Tyype>>,
                uniq_variant<type_traits::modifers::remove_reference_t<Tyype> *,
                             type_traits::modifers::remove_reference_t<Tyype> const *>,
                uniq_variant<type_traits::modifers::remove_reference_t<Tyype>, type_traits::modifers::remove_reference_t<Tyype> *,
                             type_traits::modifers::remove_reference_t<Tyype> const *>>>>;

    template <typename Tyype, typename Value>
    struct store_pointer<
        Tyype, Value,
        type_traits::other_trans::void_t<decltype(utility::declval<value_variant<Tyype> &>() = &utility::declval<Value>())>>
        : type_traits::composite_types::is_reference<Value> {};

    static_assert(!store_pointer<char, char>::value);
    static_assert(store_pointer<char, char &>::value);
    static_assert(store_pointer<const char, const char &>::value);
    static_assert(store_pointer<const char, char &>::value);
    static_assert(store_pointer<container::tuple<int32_t &, int32_t &> const, container::tuple<int32_t &, int32_t &> const &>::value);

    template <typename... Types>
    class overload_t : public Types... {
    public:
        using Types::operator()...;
    };

    template <typename... Types>
    constexpr rain_fn overload(Types &&...ts) -> auto {
        return overload_t<Types...>{ts...};
    }

    struct ooo_t;

    template <typename Tyype>
    class id_traits {
    public:
        static constexpr rain_fn
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
            __attribute__((no_sanitize_address))
#endif
#endif
            equal(Tyype const &left, Tyype const &right) -> auto {
            return left == right;
        }
    };

    template <typename Tyype>
    class id_block_base {
    public:
        constexpr id_block_base() : depth_{}, variant_{} {
        }

        constexpr rain_fn variant() -> auto & {
            return variant_;
        }

        constexpr void reset(int32_t depth) {
            if (depth_ - depth >= 0) {
                variant_ = value_variant<Tyype>{utility::monostate{}};
                depth_ = depth;
            }
        }
        constexpr void confirm(const int32_t depth) {
            if (depth_ > depth || depth_ == 0) {
                assert(depth == depth_ - 1 || depth == depth_ || depth_ == 0);
                depth_ = depth;
            }
        }

    protected:
        value_variant<Tyype> variant_;

    private:
        int32_t depth_;
    };

    constexpr id_block_base<int> dummy;

    template <typename Tyype>
    class id_block : public id_block_base<Tyype> {
    public:
        constexpr rain_fn has_value() const -> auto {
            return container::visit(overload([](Tyype const &) { return true; }, [](Tyype const *) { return true; },
                                             [](utility::monostate const &) { return false; }),
                                    id_block_base<Tyype>::variant_);
        }

        constexpr rain_fn get() const -> decltype(auto) {
            return container::visit(overload([](Tyype const &v) -> Tyype const & { return v; },
                                             [](Tyype const *p) -> Tyype const & { return *p; },
                                             [](Tyype *p) -> Tyype const & { return *p; },
                                             [](utility::monostate const &) -> Tyype const & {
                                                 exceptions::throw_exception(exceptions::logic::logic_error{"invalid state!"});
                                             }),
                                    id_block_base<Tyype>::variant_);
        }
    };

    template <typename Tyype>
    class id_block<Tyype const &> : public id_block<Tyype> {};

    template <typename Tyype>
    class id_block<Tyype &> : public id_block_base<Tyype &> {
    public:
        constexpr rain_fn has_value() const -> auto {
            return container::visit(overload([](Tyype *) { return true; }, [](utility::monostate const &) { return false; }),
                                    id_block_base<Tyype &>::variant_);
        }

        constexpr rain_fn get() -> decltype(auto) {
            return container::visit(overload(
                                        [](Tyype *v) -> Tyype & {
                                            if (v == nullptr) {
                                                exceptions::runtime::throw_runtime_error("Trying to dereference a nullptr!");
                                            }
                                            return *v;
                                        },
                                        [](utility::monostate &) -> Tyype & {
                                            exceptions::throw_exception(exceptions::logic::logic_error{"Invalid state!"});
                                        }),
                                    id_block_base<Tyype &>::variant_);
        }
    };

    template <typename Tyype>
    class id_block<Tyype &&> : public id_block_base<Tyype &&> {
    public:
        constexpr rain_fn has_value() const -> auto {
            return container::visit(overload([](Tyype const &) { return true; }, [](Tyype *) { return true; },
                                             [](utility::monostate const &) { return false; }),
                                    id_block_base<Tyype &&>::variant_);
        }

        constexpr rain_fn get() -> decltype(auto) {
            return container::visit(overload([](Tyype &v) -> Tyype & { return v; },
                                             [](Tyype *v) -> Tyype & {
                                                 if (v == nullptr) {
                                                     exceptions::runtime::throw_runtime_error("Trying to dereference a nullptr!");
                                                 }
                                                 return *v;
                                             },
                                             [](utility::monostate &) -> Tyype & {
                                                 exceptions::throw_exception(exceptions::logic::logic_error{"Invalid state!"});
                                             }),
                                    id_block_base<Tyype &&>::variant_);
        }
    };

    template <typename Tyype>
    class id_util {
    public:
        template <typename Value>
        static constexpr rain_fn bind_value(value_variant<Tyype> &v, Value &&value,
                                            type_traits::helper::false_type /* store_pointer */) -> auto {
            v = value_variant<Tyype>{utility::forward<Value>(value)};
        }
        template <typename Value>
        static constexpr rain_fn bind_value(value_variant<Tyype> &v, Value &&value, type_traits::helper::true_type /* store_pointer */)
            -> auto {
            v = value_variant<Tyype>{&value};
        }
    };

    template <typename Tyype>
    class id {
    public:
        using block_t = id_block<Tyype>;
        using block_vt = container::variant<block_t, block_t *>;

        constexpr id() = default;

        constexpr id(id const &id) {
            block_ = block_vt{&id.block()};
        }

        template <typename Pattern>
        constexpr rain_fn at(Pattern &&pattern) -> auto {
            return and_(pattern, *this);
        }

        constexpr rain_fn at(ooo_t const &) -> auto {
            return ooo_binder<Tyype>{*this};
        }

        constexpr block_t &block() const {
            return container::visit(overload([](block_t &v) -> block_t & { return v; }, [](block_t *p) -> block_t & { return *p; }),
                                    const_cast<block_vt &>(block_));
        }

        template <typename Value>
        constexpr rain_fn match_value(Value &&v) const -> auto {
            if (has_value()) {
                return id_traits<type_traits::other_trans::decay_t<Tyype>>::equal(internal_value(), v);
            }
            id_util<Tyype>::bind_value(block().variant(), utility::forward<Value>(v), store_pointer<Tyype, Value>{});
            return true;
        }

        constexpr void reset(int32_t depth) const {
            return block().reset(depth);
        }

        constexpr void confirm(int32_t depth) const {
            return block().confirm(depth);
        }

        constexpr bool has_value() const {
            return block().has_value();
        }

        constexpr rain_fn get() -> decltype(auto) {
            return block().get();
        }

        constexpr rain_fn operator*()->decltype(auto) {
            return get();
        }

    private:
        constexpr rain_fn internal_value() const -> decltype(auto) {
            return block().get();
        }

        block_vt block_{};
    };

    template <typename Tyype>
    struct pattern_traits<id<Tyype>> {
        template <typename Value>
        using app_result_tuple = container::tuple<>;

        static constexpr auto nb_id_v = true;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, id<Tyype> const &id_pattern, int32_t /* depth */, context_t &)
            -> auto {
            return id_pattern.match_value(utility::forward<Value>(value));
        }
        static constexpr void process_id_impl(id<Tyype> const &id_pattern, int32_t depth, id_process id_process_arg) {
            switch (id_process_arg) {
                case id_process::cancel:
                    id_pattern.reset(depth);
                    break;

                case id_process::confirm:
                    id_pattern.confirm(depth);
                    break;
            }
        }
    };

    template <typename... Patterns>
    class ds_t {
    public:
        constexpr explicit ds_t(Patterns const &...patterns) : patterns_{patterns...} {
        }

        constexpr rain_fn patterns() const -> const auto & {
            return patterns_;
        }

        using pattern_tuple = container::tuple<internal_pattern_t<Patterns>...>;

    private:
        pattern_tuple patterns_;
    };

    template <typename... Patterns>
    constexpr rain_fn ds(Patterns const &...patterns) -> ds_t<Patterns...> {
        return ds_t<Patterns...>{patterns...};
    }

    template <typename Ty>
    class ooo_binder {
    public:
        ooo_binder(id<Ty> const &id) : id_{id} {
        }

        rain_fn binder() const -> decltype(auto) {
            return id_;
        }

    private:
        id<Ty> id_;
    };

    struct ooo_t {
        template <typename Ty>
        constexpr rain_fn operator()(id<Ty> id) const->auto {
            return ooo_binder<Ty>{id};
        }
    };

    constexpr ooo_t ooo;

    template <>
    struct pattern_traits<ooo_t> {
        template <typename Value>
        using app_result_tuple = container::tuple<>;

        static constexpr auto nb_id_v = false;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&, ooo_t, int32_t /*depth*/, context_t &) -> auto {
            return true;
        }

        static constexpr void process_id_impl(ooo_t, int32_t /*depth*/, id_process) {
        }
    };

    template <typename Pattern>
    struct pattern_traits<ooo_binder<Pattern>> {
        template <typename Value>
        using app_result_tuple = typename pattern_traits<Pattern>::template app_result_tuple<Value>;

        static constexpr auto nb_id_v = pattern_traits<Pattern>::nb_id_v;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, ooo_binder<Pattern> const &ooo_binder_pattern, int32_t depth,
                                                    context_t &context) -> auto {
            return match_pattern(utility::forward<Value>(value), ooo_binder_pattern.binder(), depth + 1, context);
        }

        static constexpr void process_id_impl(ooo_binder<Pattern> const &ooo_binder_pattern, int32_t depth,
                                              id_process id_process_arg) {
            process_id(ooo_binder_pattern.binder(), depth, id_process_arg);
        }
    };

    template <typename Ty>
    struct is_ooo : public type_traits::helper::false_type {};

    template <>
    struct is_ooo<ooo_t> : public type_traits::helper::true_type {};

    template <typename Ty>
    struct is_ooo_binder : public type_traits::helper::false_type {};

    template <typename Ty>
    struct is_ooo_binder<ooo_binder<Ty>> : public type_traits::helper::true_type {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_ooo_binder_v = is_ooo_binder<type_traits::other_trans::decay_t<Ty>>::value;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_ooo_or_binder_v = is_ooo<type_traits::other_trans::decay_t<Ty>>::value || is_ooo_binder_v<Ty>;

    template <typename... Patterns>
    constexpr auto nb_ooo_or_binder_v = ((is_ooo_or_binder_v<Patterns> ? 1 : 0) + ... + 0);

    static_assert(nb_ooo_or_binder_v<int32_t &, ooo_t const &, char const *, wildcard, ooo_t const> == 2);

    template <typename Tuple, std::size_t... Iter>
    constexpr std::size_t find_ooo_index_impl(type_traits::helper::index_sequence<Iter...>) {
        return ((is_ooo_or_binder_v<decltype(tuple_get<Iter>(utility::declval<Tuple>()))> ? Iter : 0) + ...);
    }

    template <typename Tuple>
    constexpr std::size_t find_ooo_index() {
        return find_ooo_index_impl<Tuple>(
            type_traits::helper::make_index_sequence<std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>>{});
    }

    static_assert(is_ooo_or_binder_v<ooo_t>);
    static_assert(is_ooo_or_binder_v<ooo_binder<int32_t>>);
    static_assert(find_ooo_index<container::tuple<int32_t, ooo_binder<int32_t>, const char *>>() == 1);
    static_assert(find_ooo_index<container::tuple<int32_t, ooo_t, const char *>>() == 1);

    template <typename Iterator>
    constexpr Iterator next_iter(Iterator it, std::ptrdiff_t n) {
        utility::advance(it, n);
        return it;
    }

    template <std::size_t value_start, std::size_t pattern_start, std::size_t... Iter, typename value_tuple_type,
              typename pattern_tuple_type, typename context_t>
    constexpr rain_fn match_pattern_multiple_impl(value_tuple_type &&value_tuple, pattern_tuple_type &&pattern_tuple_arg,
                                                  const int32_t depth, context_t &context,
                                                  type_traits::helper::index_sequence<Iter...>) -> decltype(auto) {
        auto const func = [&](auto &&value, auto &&pattern) { // NOLINT
            return match_pattern(utility::forward<decltype(value)>(value), pattern, depth + 1, context);
        };
        static_cast<void>(func);
        return (func(tuple_get<Iter + value_start>(value_tuple), tuple_get<Iter + pattern_start>(pattern_tuple_arg)) && ...);
    }

    template <std::size_t value_start, std::size_t pattern_start, std::size_t size, typename value_tuple_type,
              typename pattern_tuple_type, typename context_t>
    constexpr rain_fn match_pattern_multiple(value_tuple_type &&value_tuple, pattern_tuple_type &&pattern_tuple_arg, int32_t depth,
                                             context_t &context) -> decltype(auto) {
        return match_pattern_multiple_impl<value_start, pattern_start>(utility::forward<value_tuple_type>(value_tuple),
                                                                       pattern_tuple_arg, depth, context,
                                                                       type_traits::helper::make_index_sequence<size>{});
    }

    template <std::size_t pattern_start, std::size_t... Iter, typename range_begin_type, typename pattern_tuple_type,
              typename context_t>
    constexpr rain_fn match_pattern_range_impl(range_begin_type &&range_begin, pattern_tuple_type &&pattern_tuple_arg, int32_t depth,
                                               context_t &context, type_traits::helper::index_sequence<Iter...>) -> decltype(auto) {
        auto const func = [&](auto &&value, auto &&pattern) { // NOLINT
            return match_pattern(utility::forward<decltype(value)>(value), pattern, depth + 1, context);
        };
        static_cast<void>(func);
        return (func(*next_iter(range_begin, static_cast<std::ptrdiff_t>(Iter)), tuple_get<Iter + pattern_start>(pattern_tuple_arg)) &&
                ...);
    }

    template <std::size_t pattern_start, std::size_t size, typename value_range_begin, typename pattern_tuple_type, typename context_t>
    constexpr rain_fn match_pattern_range(value_range_begin &&begin_iter, pattern_tuple_type &&pattern_tuple_arg, int32_t depth,
                                          context_t &context) -> decltype(auto) {
        return match_pattern_range_impl<pattern_start>(utility::forward<value_range_begin>(begin_iter), pattern_tuple_arg, depth,
                                                       context, type_traits::helper::make_index_sequence<size>{});
    }

    template <std::size_t start, typename indices, typename Tuple>
    struct indexed_types;

    template <typename Tuple, std::size_t start, std::size_t... Iter>
    struct indexed_types<start, type_traits::helper::index_sequence<Iter...>, Tuple> {
        using type =
            container::tuple<type_traits::other_trans::decay_t<decltype(tuple_get<start + Iter>(utility::declval<Tuple>()))>...>;
    };

    template <std::size_t start, std::size_t end, typename Tuple>
    struct sub_types {
        static constexpr auto tuple_length = std::tuple_size_v<type_traits::modifers::remove_reference_t<Tuple>>;
        static_assert(start <= end);
        static_assert(end <= tuple_length);

        using indices = type_traits::helper::make_index_sequence<end - start>;

        using type = typename indexed_types<start, indices, Tuple>::type;
    };

    template <std::size_t start, std::size_t end, typename Tuple>
    using sub_types_t = typename sub_types<start, end, Tuple>::type;

    static_assert(type_traits::type_relations::is_same_v<container::tuple<std::nullptr_t>,
                                                         sub_types_t<3, 4, container::tuple<char, bool, int32_t, std::nullptr_t>>>);
    static_assert(type_traits::type_relations::is_same_v<container::tuple<char>,
                                                         sub_types_t<0, 1, container::tuple<char, bool, int32_t, std::nullptr_t>>>);
    static_assert(type_traits::type_relations::is_same_v<container::tuple<>,
                                                         sub_types_t<1, 1, container::tuple<char, bool, int32_t, std::nullptr_t>>>);
    static_assert(type_traits::type_relations::is_same_v<container::tuple<int32_t, std::nullptr_t>,
                                                         sub_types_t<2, 4, container::tuple<char, bool, int32_t, std::nullptr_t>>>);

    template <typename value_tuple_type>
    struct is_array : public type_traits::helper::false_type {};

    template <typename Ty, std::size_t s>
    struct is_array<collections::array<Ty, s>> : public type_traits::helper::true_type {};

    template <typename value_tuple_type>
    RAINY_CONSTEXPR_BOOL is_array_v = is_array<type_traits::other_trans::decay_t<value_tuple_type>>::value;

    template <typename Value, typename = type_traits::other_trans::void_t<>>
    struct is_tuple_like : type_traits::helper::false_type {};

    template <typename Value>
    struct is_tuple_like<Value, type_traits::other_trans::void_t<decltype(std::tuple_size<Value>::value)>>
        : type_traits::helper::true_type {};

    template <typename value_tuple_type>
    RAINY_CONSTEXPR_BOOL is_tuple_like_v = is_tuple_like<type_traits::other_trans::decay_t<value_tuple_type>>::value;

    static_assert(is_tuple_like_v<container::pair<int32_t, char>>);
    static_assert(!is_tuple_like_v<bool>);

    template <typename Value, typename = type_traits::other_trans::void_t<>>
    struct is_range : type_traits::helper::false_type {};

    template <typename Value>
    struct is_range<Value, type_traits::other_trans::void_t<decltype(utility::begin(utility::declval<Value>())),
                                                            decltype(utility::end(utility::declval<Value>()))>>
        : type_traits::helper::true_type {};

    template <typename value_tuple_type>
    RAINY_CONSTEXPR_BOOL is_range_v = is_range<type_traits::other_trans::decay_t<value_tuple_type>>::value;

    static_assert(!is_range_v<container::pair<int32_t, char>>);
    static_assert(is_range_v<const collections::array<int32_t, 5>>);

    template <typename... Patterns>
    struct pattern_traits<ds_t<Patterns...>> {
        static constexpr auto nb_ooo_or_binder = nb_ooo_or_binder_v<Patterns...>;
        static_assert(nb_ooo_or_binder == 0 || nb_ooo_or_binder == 1);

        template <typename PsTuple, typename VsTuple>
        struct pair_pv;

        template <typename... Ps, typename... Vs>
        struct pair_pv<container::tuple<Ps...>, container::tuple<Vs...>> {
        public:
            using type = decltype(tuple_cat_impl(utility::declval<typename pattern_traits<Ps>::template app_result_tuple<Vs>>()...));
        };

        template <std::size_t nb_ooo, typename value_tuple_type>
        struct app_result_for_tuple_helper;

        template <typename... Values>
        struct app_result_for_tuple_helper<0, container::tuple<Values...>> {
            using type =
                decltype(tuple_cat_impl(utility::declval<typename pattern_traits<Patterns>::template app_result_tuple<Values>>()...));
        };

        template <typename... Values>
        struct app_result_for_tuple_helper<1, container::tuple<Values...>> {
            static constexpr auto ooo_index = find_ooo_index<typename ds_t<Patterns...>::pattern_tuple>();
            using patterns_head = sub_types_t<0, ooo_index, container::tuple<Patterns...>>;
            using values_head = sub_types_t<0, ooo_index, container::tuple<Values...>>;
            static RAINY_CONSTEXPR_BOOL is_binder =
                is_ooo_binder_v<container::tuple_element_t<ooo_index, container::tuple<Patterns...>>>;
            using element_t =
                container::tuple_element_t<0, container::tuple<type_traits::modifers::remove_reference_t<Values>..., int32_t>>;
            static constexpr int64_t diff = static_cast<int64_t>(sizeof...(Values) - sizeof...(Patterns));
            static constexpr std::size_t clipped_diff = static_cast<std::size_t>(diff > 0 ? diff : 0);
            using ooo_result_tuple = typename type_traits::other_trans::conditional<
                is_binder, container::tuple<subrange_t<collections::array<element_t, clipped_diff>>>, container::tuple<>>::type;
            using first_half_tuple = typename pair_pv<patterns_head, values_head>::type;
            using patterns_tail = sub_types_t<ooo_index + 1, sizeof...(Patterns), container::tuple<Patterns...>>;
            static constexpr auto second_half_start = static_cast<std::size_t>(static_cast<int64_t>(ooo_index) + 1 + diff);
            using values_tail = sub_types_t<second_half_start, sizeof...(Values), container::tuple<Values...>>;
            using second_half_tuple = typename pair_pv<patterns_tail, values_tail>::type;

            using type = decltype(tuple_cat_impl(utility::declval<first_half_tuple>(), utility::declval<ooo_result_tuple>(),
                                                 utility::declval<second_half_tuple>()));
        };

        template <typename Tuple>
        using app_result_for_tuple =
            typename app_result_for_tuple_helper<nb_ooo_or_binder, decltype(drop<0>(utility::declval<Tuple>()))>::type;

        template <typename RangeTyype>
        using range_tuple = type_traits::other_trans::conditional_t<nb_ooo_or_binder == 1, container::tuple<subrange_t<RangeTyype>>,
                                                                    container::tuple<>>;

        template <typename RangeTyype>
        using app_result_for_range_type = decltype(tuple_cat_impl(
            utility::declval<range_tuple<RangeTyype>>(), utility::declval<typename pattern_traits<Patterns>::template app_result_tuple<
                                                             decltype(*utility::begin(utility::declval<RangeTyype>()))>>()...));

        template <typename Value, typename = int>
        struct app_result_helper;

        template <typename Value>
        struct app_result_helper<Value, type_traits::other_trans::enable_if_t<is_tuple_like_v<Value>, int>> {
            using type = app_result_for_tuple<Value>;
        };

        template <typename RangeTyype>
        struct app_result_helper<RangeTyype,
                                 type_traits::other_trans::enable_if_t<!is_tuple_like_v<RangeTyype> && is_range_v<RangeTyype>, int>> {
            using type = app_result_for_range_type<RangeTyype>;
        };

        template <typename Value>
        using app_result_tuple = typename app_result_helper<Value>::type;

        static constexpr auto nb_id_v = (pattern_traits<Patterns>::nb_id_v + ... + 0);

        template <typename value_tuple_type, typename context_t>
        static constexpr rain_fn match_pattern_impl(value_tuple_type &&value_tuple, ds_t<Patterns...> const &ds_pattern, int32_t depth,
                                                    context_t &context)
            -> type_traits::other_trans::enable_if_t<is_tuple_like_v<value_tuple_type>, bool> {
            if constexpr (nb_ooo_or_binder == 0) {
                return apply_(
                    [&value_tuple, depth, &context](auto const &...patterns) {
                        return apply_(
                            [depth, &context, &patterns...](auto &&...values) constexpr { // NOLINT
                                static_assert(sizeof...(patterns) == sizeof...(values));
                                return (match_pattern(utility::forward<decltype(values)>(values), patterns, depth + 1, context) &&
                                        ...);
                            },
                            value_tuple);
                    },
                    ds_pattern.patterns());
            } else if constexpr (nb_ooo_or_binder == 1) {
                constexpr auto ooo_index = find_ooo_index<typename ds_t<Patterns...>::pattern_tuple>();
                constexpr bool is_binder = is_ooo_binder_v<container::tuple_element_t<ooo_index, container::tuple<Patterns...>>>;
                constexpr auto value_is_array = is_array_v<value_tuple_type>;
                auto result = match_pattern_multiple<0, 0, ooo_index>(utility::forward<value_tuple_type>(value_tuple),
                                                                      ds_pattern.patterns(), depth, context);
                constexpr auto value_length = std::tuple_size<type_traits::other_trans::decay_t<value_tuple_type>>::value;
                constexpr auto pattern_length = sizeof...(Patterns);
                if constexpr (value_is_array) {
                    if constexpr (is_binder) {
                        auto const range_size = static_cast<long>(value_length - (pattern_length - 1));
                        context.emplace_back(make_subrange(&value_tuple[ooo_index], &value_tuple[ooo_index] + range_size));
                        using type = decltype(make_subrange(&value_tuple[ooo_index], &value_tuple[ooo_index] + range_size));
                        result = result && match_pattern(container::get<type>(context.back()),
                                                         tuple_get<ooo_index>(ds_pattern.patterns()), depth, context);
                    }
                } else {
                    static_assert(!is_binder);
                }
                return result && match_pattern_multiple<value_length - pattern_length + ooo_index + 1, ooo_index + 1,
                                                        pattern_length - ooo_index - 1>(
                                     utility::forward<value_tuple_type>(value_tuple), ds_pattern.patterns(), depth, context);
            }
            return false;
        }

        template <typename value_range_type, typename context_t>
        static constexpr rain_fn match_pattern_impl(value_range_type &&value_range, ds_t<Patterns...> const &ds_pattern, int32_t depth,
                                                    context_t &context)
            -> type_traits::other_trans::enable_if_t<!is_tuple_like_v<value_range_type> && is_range_v<value_range_type>, bool> {
            static_assert(nb_ooo_or_binder == 0 || nb_ooo_or_binder == 1);
            constexpr auto nb_pattern = sizeof...(Patterns);

            if constexpr (nb_ooo_or_binder == 0) {
                if (value_range.size() != nb_pattern) {
                    return false;
                }
                return match_pattern_range<0, nb_pattern>(utility::begin(value_range), ds_pattern.patterns(), depth, context);
            } else if constexpr (nb_ooo_or_binder == 1) {
                if (value_range.size() < nb_pattern - 1) {
                    return false;
                }
                constexpr auto ooo_index = find_ooo_index<typename ds_t<Patterns...>::pattern_tuple>();
                constexpr bool is_binder = is_ooo_binder_v<container::tuple_element_t<ooo_index, container::tuple<Patterns...>>>;
                auto result = match_pattern_range<0, ooo_index>(utility::begin(value_range), ds_pattern.patterns(), depth, context);
                auto const value_length = value_range.size();
                constexpr auto pattern_length = sizeof...(Patterns);
                auto const ooo_begin = next_iter(utility::begin(value_range), static_cast<std::ptrdiff_t>(ooo_index));
                if constexpr (is_binder) {
                    auto const range_size = static_cast<long>(value_length - (pattern_length - 1));
                    auto const end = next_iter(ooo_begin, range_size);
                    context.emplace_back(make_subrange(ooo_begin, end));
                    using type = decltype(make_subrange(ooo_begin, end));
                    result = result && match_pattern(container::get<type>(context.back()), tuple_get<ooo_index>(ds_pattern.patterns()),
                                                     depth, context);
                }
                auto const begin_after_ooo = next_iter(ooo_begin, static_cast<std::ptrdiff_t>(value_length - pattern_length + 1));
                return result && match_pattern_range<ooo_index + 1, pattern_length - ooo_index - 1>(
                                     begin_after_ooo, ds_pattern.patterns(), depth, context);
            }
            return false;
        }

        static constexpr void process_id_impl(ds_t<Patterns...> const &ds_pattern, int32_t depth, id_process id_process_arg) {
            return apply_([depth, id_process_arg](auto &&...patterns) { return (process_id(patterns, depth, id_process_arg), ...); },
                          ds_pattern.patterns());
        }
    };

    static_assert(type_traits::type_relations::is_same_v<
                  typename pattern_traits<ds_t<ooo_binder<subrange_t<const collections::array<int32_t, 2>>>>>::app_result_tuple<
                      const collections::array<int32_t, 2>>,
                  container::tuple<sub_range<const int32_t *, const int32_t *>>>);

    static_assert(type_traits::type_relations::is_same_v<
                  typename pattern_traits<ds_t<ooo_binder<sub_range<int32_t *, int32_t *>>, id<int32_t>>>::app_result_tuple<
                      const collections::array<int32_t, 3>>,
                  container::tuple<sub_range<const int32_t *, const int32_t *>>>);

    static_assert(type_traits::type_relations::is_same_v<
                  typename pattern_traits<ds_t<ooo_binder<sub_range<int32_t *, int32_t *>>, id<int32_t>>>::app_result_tuple<
                      collections::array<int32_t, 3>>,
                  container::tuple<sub_range<int32_t *, int32_t *>>>);

    template <typename Pattern, typename Pred>
    class post_check {
    public:
        constexpr explicit post_check(Pattern const &pattern, Pred const &pred) : pattern_{pattern}, pred_{pred} {
        }

        constexpr bool check() const {
            return pred_();
        }

        constexpr rain_fn pattern() const -> const auto & {
            return pattern_;
        }

    private:
        Pattern const pattern_;
        Pred const pred_;
    };

    template <typename Pattern, typename Pred>
    struct pattern_traits<post_check<Pattern, Pred>> {
        template <typename Value>
        using app_result_tuple = typename pattern_traits<Pattern>::template app_result_tuple<Value>;

        template <typename Value, typename context_t>
        static constexpr rain_fn match_pattern_impl(Value &&value, post_check<Pattern, Pred> const &post_check, const int32_t depth,
                                                    context_t &context) -> auto {
            return match_pattern(utility::forward<Value>(value), post_check.pattern(), depth + 1, context) && post_check.check();
        }
        static constexpr void process_id_impl(post_check<Pattern, Pred> const &post_check, int32_t depth, id_process id_process_arg) {
            process_id(post_check.pattern(), depth, id_process_arg);
        }
    };

    static_assert(
        type_traits::type_relations::is_same_v<pattern_traits<wildcard>::template app_result_tuple<int32_t>, container::tuple<>>);
    static_assert(
        type_traits::type_relations::is_same_v<pattern_traits<int32_t>::template app_result_tuple<int32_t>, container::tuple<>>);
    constexpr auto identity_probe = [](auto &&t) { return t; };
    static_assert(type_traits::type_relations::is_same_v<
                  pattern_traits<app_t<decltype(identity_probe), wildcard>>::template app_result_tuple<collections::array<int32_t, 3>>,
                  container::tuple<collections::array<int32_t, 3>>>);

    static_assert(pattern_traits<and_t<app_t<decltype(identity_probe), wildcard>>>::nb_id_v == 0);
    static_assert(pattern_traits<and_t<app_t<decltype(identity_probe), id<int32_t>>>>::nb_id_v == 1);
    static_assert(pattern_traits<and_t<id<int32_t>, id<float>>>::nb_id_v == 2);
    static_assert(pattern_traits<or_t<id<int32_t>, id<float>>>::nb_id_v == 2);
    static_assert(pattern_traits<or_t<wildcard, float>>::nb_id_v == 0);

    template <typename Value, typename... pattern_pairs>
    constexpr rain_fn match_patterns(Value &&value, pattern_pairs const &...patterns) -> auto {
        using ret_type = typename pattern_pairs_ret_type<pattern_pairs...>::ret_type;
        using type_tuple = decltype(tuple_cat_impl(
            utility::declval<typename pattern_traits<typename pattern_pairs::pattern_t>::template app_result_tuple<Value>>()...));

        if constexpr (!type_traits::type_relations::is_same_v<ret_type, void>) {
            constexpr auto func = [](auto const &pattern, auto &&value_, ret_type &result) constexpr -> bool {
                if (auto context = typename context_trait<type_tuple>::context_t{};
                    pattern.match_value(utility::forward<Value>(value_), context)) {
                    result = pattern.execute();
                    process_id(pattern, 0, id_process::cancel);
                    return true;
                }
                return false;
            };
            ret_type result{};
            bool const matched = (func(patterns, value, result) || ...);
            if (!matched) {
                exceptions::throw_exception(exceptions::logic::logic_error{"Error: no patterns got matched!"});
            }
            static_cast<void>(matched);
            return result;
        } else {
            auto const func = [](auto const &pattern, auto &&value_) -> bool {
                if (auto context = typename context_trait<type_tuple>::context_t{};
                    pattern.match_value(utility::forward<Value>(value_), context)) {
                    pattern.execute();
                    process_id(pattern, 0, id_process::cancel);
                    return true;
                }
                return false;
            };
            bool const matched = (func(patterns, value) || ...);
            static_cast<void>(matched);
        }
    }

}
#endif
