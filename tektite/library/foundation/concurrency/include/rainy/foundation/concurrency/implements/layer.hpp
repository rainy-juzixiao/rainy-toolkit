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
#ifndef RAINY_FOUNDATION_CONCURRENCY_IMPLEMENTS_LAYER_HPP
#define RAINY_FOUNDATION_CONCURRENCY_IMPLEMENTS_LAYER_HPP

#include <rainy/core/platform.hpp>
#include <chrono>
#include <rainy/core/layer.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::foundation::concurrency::implements {
    // 将核心库的平台抽象层（PAL）引入实现命名空间，供本模块的封装类型使用。
    using core::layer::thrd_result;
    using core::layer::schd_thread_t;

    using core::layer::create_thread;
    using core::layer::thread_detach;
    using core::layer::thread_hardware_concurrency;
    using core::layer::thread_join;
    using core::layer::thread_sleep_for;
    using core::layer::thread_yield;
    using core::layer::get_thread_id;
    using core::layer::suspend_thread;
    using core::layer::resume_thread;

    using core::layer::mutex_types;
    using core::layer::mtx_t;
    using core::layer::native_mtx_handle;
    using core::layer::native_cnd_handle;
    using core::layer::native_smtx_handle;
    using core::layer::mtx_create;
    using core::layer::mtx_destroy;
    using core::layer::mtx_do_lock;
    using core::layer::mtx_init;
    using core::layer::mtx_lock;
    using core::layer::mtx_timedlock;
    using core::layer::mtx_trylock;
    using core::layer::mtx_unlock;

    using core::layer::cnd_t;
    using core::layer::cnd_broadcast;
    using core::layer::cnd_create;
    using core::layer::cnd_destroy;
    using core::layer::cnd_init;
    using core::layer::cnd_signal;
    using core::layer::cnd_timedwait;
    using core::layer::cnd_wait;

    using core::layer::smtx_t;
    using core::layer::smtx_create;
    using core::layer::smtx_destroy;
    using core::layer::smtx_init;
    using core::layer::smtx_lock;
    using core::layer::smtx_lock_shared;
    using core::layer::smtx_try_lock;
    using core::layer::smtx_try_lock_shared;
    using core::layer::smtx_timed_lock;
    using core::layer::smtx_timed_lock_shared;
    using core::layer::smtx_unlock;
    using core::layer::smtx_unlock_shared;

    using core::layer::tss_create;
    using core::layer::tss_delete;
    using core::layer::tss_get;
    using core::layer::tss_set;
}


namespace rainy::foundation::concurrency::implements {
#if !defined(__cpp_lib_is_clock) || __cpp_lib_is_clock < 201907L
    template <typename Ty, typename = void>
    struct is_clock : std::false_type {};

    template <typename Ty>
    struct is_clock<Ty, type_traits::other_trans::void_t<typename Ty::rep, typename Ty::period, typename Ty::duration,
                                   typename Ty::time_point, decltype(Ty::is_steady), decltype(Ty::now())>>
        : type_traits::helper::true_type {};
#else
    template <typename Ty, typename = void>
    struct is_clock : type_traits::helper::bool_constant<std::chrono::is_clock_v<Ty>> {};
#endif

    template <typename Ty>
    inline constexpr bool is_clock_v = is_clock<Ty>::value;
}

#endif
