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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_EXECUTOR_TRAIT_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_EXECUTOR_TRAIT_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::net::implements {
    struct executor_memfns_base {
        void context();
        void on_work_started();
        void on_work_finished();
        void dispatch();
        void post();
        void defer();
    };

    template <typename Ty>
    struct executor_memfns_derived : Ty, executor_memfns_base {};

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_context_memfn = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_context_memfn<Ty, type_traits::other_trans::void_t<decltype(&executor_memfns_derived<Ty>::context)>> =
        type_traits::type_relations::is_same_v<decltype(&executor_memfns_derived<Ty>::context), void (executor_memfns_base::*)()>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_on_work_started_memfn = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_on_work_started_memfn<Ty, type_traits::other_trans::void_t<decltype(&executor_memfns_derived<Ty>::on_work_started)>> =
        type_traits::type_relations::is_same_v<decltype(&executor_memfns_derived<Ty>::on_work_started), void (executor_memfns_base::*)()>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_on_work_finished_memfn = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_on_work_finished_memfn<Ty, type_traits::other_trans::void_t<decltype(&executor_memfns_derived<Ty>::on_work_finished)>> =
        type_traits::type_relations::is_same_v<decltype(&executor_memfns_derived<Ty>::on_work_finished), void (executor_memfns_base::*)()>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_dispatch_memfn = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_dispatch_memfn<Ty, type_traits::other_trans::void_t<decltype(&executor_memfns_derived<Ty>::dispatch)>> =
        type_traits::type_relations::is_same_v<decltype(&executor_memfns_derived<Ty>::dispatch), void (executor_memfns_base::*)()>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_post_memfn = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_post_memfn<Ty, type_traits::other_trans::void_t<decltype(&executor_memfns_derived<Ty>::post)>> =
        type_traits::type_relations::is_same_v<decltype(&executor_memfns_derived<Ty>::post), void (executor_memfns_base::*)()>;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_defer_memfn = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_defer_memfn<Ty, type_traits::other_trans::void_t<decltype(&executor_memfns_derived<Ty>::defer)>> =
        type_traits::type_relations::is_same_v<decltype(&executor_memfns_derived<Ty>::defer), void (executor_memfns_base::*)()>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_executor_class_v =
        has_context_memfn<Ty> &&
        has_on_work_started_memfn<Ty> &&
        has_on_work_finished_memfn<Ty> &&
        has_dispatch_memfn<Ty> &&
        has_post_memfn<Ty> &&
        has_defer_memfn<Ty>;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_executor_v =
        type_traits::primary_types::is_class_v<Ty> && is_executor_class_v<Ty>;

    template <typename Ty>
    struct is_executor : type_traits::helper::bool_constant<is_executor_v<Ty>> {};
}

namespace rainy::foundation::io::net {
    template <typename Ty>
    struct is_executor : implements::is_executor<Ty> {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_executor_v = is_executor<Ty>::value;

    struct executor_arg_t {};

    constexpr auto executor_arg = executor_arg_t();

    template <typename Ty, typename Executor>
    struct uses_executor : type_traits::helper::false_type {};

    template <typename Ty, typename Executor>
    RAINY_CONSTEXPR_BOOL uses_executor_v = uses_executor<Ty, Executor>::value;
}

#endif