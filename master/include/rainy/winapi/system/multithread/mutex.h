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
#ifndef RAINY_WINAPI_SYSTEM_MUTITHREAD_MUTEX_H
#define RAINY_WINAPI_SYSTEM_MUTITHREAD_MUTEX_H
#include <rainy/core/core.hpp>
#include <rainy/winapi/api_core.h>
#include <rainy/winapi/system/multithread/thread.h>
#include <chrono>
#include <ctime>

namespace rainy::winapi::system::multithread {
    struct mutex_types {
        static constexpr int plain_mtx = 0x01;
        static constexpr int try_mtx = 0x02;
        static constexpr int timed_mtx = 0x04;
        static constexpr int recursive_mtx = 0x100;
    };

    using mtx_t = void *;
}

namespace rainy::winapi::system::multithread::internals {
    struct mutex_handle {
        void *handle{nullptr};
        int type{0};
        unsigned int thread_id{0};
        int count{0};
    };

    /**
     * @brief 从mutex锁的句柄中获取原生锁的handle
     * @param mtx mutex锁的handle
     * @return 返回mutex锁handle中对应的原生锁handle。但是，如果mtx为空，errno将被设置为EINVAL。并返回nullptr
     */
    RAINY_HPP_INLINE PSRWLOCK get_srw_lock(mtx_t *mtx) noexcept;

    /**
     * @brief 锁定互斥锁的具体实现函数。此部分不应当由开发者调用，作为内部实现
     * @brief 将根据mutex锁的handle指向的类型决定。类型是从mutex_types中选定的
     * @brief 如果类型为plain_mtx，将用于普通锁。带有try_mtx可以用于try_lock。如果是timed_mtx，可以用于timedlock，而recursive_mtx将支持递归
     * @param mtx mutex锁的handle
     * @param target 超时时间，如果为nullptr表示无限期锁定
     * @return 如果成功获取锁，则返回success，否则根据情况返回busy或timed_out
     * @return 但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_HPP_INLINE win32_thread_result mtx_do_lock(mtx_t *mtx, const ::timespec *target) noexcept;
}

namespace rainy::winapi::system::multithread {
    /**
     * @brief 初始化mutex锁
     * @attention 此函数需要一个有效的实例句柄。这个句柄指向了internals::mutex_handle实例。如果有必要
     * @attention 考虑调用mtx_create，它将为你自动创建实例并调用此初始化。只需要在不需要它的时候，使用mtx_destory即可
     * @param mtx mutex锁的handle
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_HPP_INLINE win32_thread_result mtx_init(mtx_t *mtx, int flags) noexcept;

    /**
     * @brief 创建mutex锁
     * @attention 此函数将会覆写mtx句柄，并且将会从动态存储空间获取一段内存用于存储。且会自动调用mtx_init进行自动的初始化
     * @attention 因此，如果它没有被mtx_destory销毁，可能会导致内存泄漏
     * @param mtx mutex锁的handle，用于接收新的实例
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是如果没有句柄用于接收，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_HPP_INLINE win32_thread_result mtx_create(mtx_t *mtx, int flags) noexcept;
    
    /**
     * @brief 使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_HPP_INLINE win32_thread_result mtx_lock(mtx_t *mtx) noexcept;
    
    /**
     * @brief 尝试使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem。
     * @return 此外，如果互斥锁已被其他线程占用。busy将是此函数的返回值
     */
    RAINY_HPP_INLINE win32_thread_result mtx_trylock(mtx_t *mtx) noexcept;
    
    /**
     * @brief 解锁mutex锁该函数释放互斥锁
     * @brief 如果当前线程持有该锁，则将其解锁
     * @param mtx mutex锁的handle
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_HPP_INLINE win32_thread_result mtx_unlock(mtx_t *mtx) noexcept;
    
    /**
     * @brief 尝试对mutex锁进行锁定，支持超时机制
     * @param mtx mutex锁的handle
     * @param xt 超时参数，表示锁定的最大时间
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     * @return 如果在指定的时间内无法获取锁，则返回timed_out
     */
    RAINY_HPP_INLINE win32_thread_result mtx_timedlock(mtx_t *mtx, const ::timespec *xt) noexcept;
    
    /**
     * @brief 检查当前线程是否已经持有指定的互斥锁
     * @param mtx mutex锁的handle
     * @return bool 如果当前线程持有该互斥锁，返回true，否则返回false
     */
    RAINY_HPP_INLINE bool mtx_current_owns(mtx_t *mtx) noexcept;
    
    /**
     * @brief 销毁mutex锁
     * @param mtx mutex锁的handle
     * @param release 是否释放内存，若为`true`则释放内存。如果是mtx_create创建的，请务必使其为true
     * @return 一般返回success来表示成功。但是，如果
     */
    RAINY_HPP_INLINE win32_thread_result mtx_destroy(mtx_t *mtx, bool release = true) noexcept;
}

#if !RAINY_USING_SOURCESFILE_BUILD
#include <rainy_cxxfiles/winapi/system/multithread/mutex.cxx>
#endif

#endif
