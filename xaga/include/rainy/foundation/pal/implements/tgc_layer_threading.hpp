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
#ifndef RAINY_FOUNDATION_PAL_IMPLEMENTS_TGC_LAYER_THREADING_HPP
#define RAINY_FOUNDATION_PAL_IMPLEMENTS_TGC_LAYER_THREADING_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>

#if RAINY_USING_LINUX
#include <pthread.h>
#endif

namespace rainy::foundation::pal::threading {
    enum class thrd_result : int {
        success,
        nomem,
        timed_out,
        busy,
        error
    };
}

namespace rainy::foundation::pal::threading::implements {
    struct schd_thread_t {
        std::uintptr_t handle;
        std::uint64_t tid;
    };
}

namespace rainy::foundation::pal::threading::implements {
    RAINY_TOOLKIT_API void endthread();
    RAINY_TOOLKIT_API void endthreadex(unsigned int return_code);
    RAINY_NODISCARD RAINY_TOOLKIT_API schd_thread_t
    create_thread(foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, unsigned int stack_size,
                  void *arg_list);
    RAINY_NODISCARD RAINY_TOOLKIT_API schd_thread_t
    create_thread(void *security, unsigned int stack_size,
                  foundation::functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, void *arg_list,
                  unsigned int init_flag, std::uint64_t *thrd_addr);
    RAINY_TOOLKIT_API thrd_result thread_join(schd_thread_t thread_handle, std::int64_t *result_receiver) noexcept;
    RAINY_TOOLKIT_API thrd_result thread_detach(schd_thread_t thread_handle) noexcept;
    RAINY_TOOLKIT_API unsigned int thread_hardware_concurrency() noexcept;
    RAINY_TOOLKIT_API std::uint64_t get_thread_id() noexcept;
    RAINY_TOOLKIT_API void thread_yield() noexcept;
    RAINY_TOOLKIT_API void thread_sleep_for(const unsigned long ms) noexcept;
    RAINY_TOOLKIT_API void suspend_thread(schd_thread_t thread_handle) noexcept;
    RAINY_TOOLKIT_API void resume_thread(schd_thread_t thread_handle) noexcept;
}

namespace rainy::foundation::pal::threading::implements {
    struct mutex_types {
        static constexpr int plain_mtx = 0x01;
        static constexpr int try_mtx = 0x02;
        static constexpr int timed_mtx = 0x04;
        static constexpr int recursive_mtx = 0x100;
    };

    using mtx_t = void *;

    /**
     * @brief 锁定互斥锁的具体实现函数。此部分不应当由开发者调用，作为内部实现
     * @brief 将根据mutex锁的handle指向的类型决定。类型是从mutex_types中选定的
     * @brief
     * 如果类型为plain_mtx，将用于普通锁。带有try_mtx可以用于try_lock。如果是timed_mtx，可以用于timedlock，而recursive_mtx将支持递归
     * @param mtx mutex锁的handle
     * @param target 超时时间，如果为nullptr表示无限期锁定
     * @return 如果成功获取锁，则返回success，否则根据情况返回busy或timed_out
     * @return 但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_do_lock(mtx_t *mtx, const ::timespec *target) noexcept;

    /**
     * @brief 初始化mutex锁
     * @attention 此函数需要一个有效的实例句柄。这个句柄指向了implements::mutex_handle实例。如果有必要
     * @attention 考虑调用mtx_create，它将为你自动创建实例并调用此初始化。只需要在不需要它的时候，使用mtx_destory即可
     * @param mtx mutex锁的handle
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_init(mtx_t *mtx, int flags) noexcept;

    /**
     * @brief 创建mutex锁
     * @attention 此函数将会覆写mtx句柄，并且将会从动态存储空间获取一段内存用于存储。且会自动调用mtx_init进行自动的初始化
     * @attention 因此，如果它没有被mtx_destory销毁，可能会导致内存泄漏
     * @param mtx mutex锁的handle，用于接收新的实例
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是如果没有句柄用于接收，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_create(mtx_t *mtx, int flags) noexcept;

    /**
     * @brief 使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_lock(mtx_t *mtx) noexcept;

    /**
     * @brief 尝试使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem。
     * @return 此外，如果互斥锁已被其他线程占用。busy将是此函数的返回值
     */
    RAINY_TOOLKIT_API thrd_result mtx_trylock(mtx_t *mtx) noexcept;

    /**
     * @brief 解锁mutex锁该函数释放互斥锁
     * @brief 如果当前线程持有该锁，则将其解锁
     * @param mtx mutex锁的handle
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_unlock(mtx_t *mtx) noexcept;

    /**
     * @brief 尝试对mutex锁进行锁定，支持超时机制
     * @param mtx mutex锁的handle
     * @param xt 超时参数，表示锁定的最大时间
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     * @return 如果在指定的时间内无法获取锁，则返回timed_out
     */
    RAINY_TOOLKIT_API thrd_result mtx_timedlock(mtx_t *mtx, const ::timespec *xt) noexcept;

    /**
     * @brief 检查当前线程是否已经持有指定的互斥锁
     * @param mtx mutex锁的handle
     * @return bool 如果当前线程持有该互斥锁，返回true，否则返回false
     */
    RAINY_TOOLKIT_API bool mtx_current_owns(mtx_t *mtx) noexcept;

    /**
     * @brief 销毁mutex锁
     * @param mtx mutex锁的handle
     * @return 一般返回success来表示成功。但是，如果
     */
    RAINY_TOOLKIT_API thrd_result mtx_destroy(mtx_t *mtx) noexcept;
    RAINY_TOOLKIT_API void *native_mtx_handle(mtx_t *mtx) noexcept;
}

#endif
