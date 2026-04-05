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
/**
 * @file pal.hpp
 * @brief 平台抽象层（PAL）线程与同步原语实现接口
 *
 * 本文件提供 rainy foundation 在不同操作系统上的线程与同步设施的
 * 统一抽象实现接口（Platform Abstraction Layer, PAL）。
 *
 * 该接口设计目标：
 *  - 提供接近 C11 threads 语义的跨平台线程能力
 *  - 隐藏 pthread / Win32 Thread 等系统差异
 *  - 允许上层构建 scheduler、executor 或 runtime
 *
 * 注意
 *  - 本命名空间属于 implements 层，为内部实现接口。
 *  - API 行为依赖底层操作系统能力，不保证完全一致的调度行为。
 *
 * 生命周期原则：
 *  - create_* 创建的资源必须由对应 destroy / join / delete 释放
 *  - 未释放资源可能导致系统句柄或内存泄漏
 */
#ifndef RAINY_FOUNDATION_CONCURRENCY_PAL_HPP
#define RAINY_FOUNDATION_CONCURRENCY_PAL_HPP
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>

namespace rainy::foundation::concurrency::implements {
#if !defined(__cpp_lib_is_clock) || __cpp_lib_is_clock < 201907L
    template <typename Ty, typename = void>
    struct is_clock : std::false_type {};

    template <typename Ty>
    struct is_clock<Ty, type_traits::other_trans::void_t<typename Ty::rep, typename Ty::period, typename Ty::duration, typename Ty::time_point,
                                   decltype(Ty::is_steady), decltype(Ty::now())>> : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_clock_v = is_clock<Ty>::value;
#else
    template <typename Ty, typename = void>
    struct is_clock : type_traits::helper::bool_constant<std::chrono::is_clock_v<Ty>> {};
#endif
}

namespace rainy::foundation::concurrency {
    /**
     * @brief Thread operation result codes.
     *        线程操作返回值。
     *
     * This enumeration unifies error states across different platforms.
     * 该枚举用于统一不同平台的错误状态。
     */
    enum class thrd_result : int {
        /**
         * @brief Operation succeeded
         *        操作成功
         *
         * The operation completed successfully.
         * 操作成功完成。
         */
        success,

        /**
         * @brief Insufficient memory or invalid argument
         *        资源不足或参数非法
         *
         * Typically indicates that memory allocation failed or an invalid parameter was provided.
         * Usually accompanied by errno = EINVAL.
         * 通常表示内存分配失败或提供了非法参数，一般伴随 errno = EINVAL。
         */
        nomem,

        /**
         * @brief Operation timed out
         *        超时
         *
         * The operation did not complete within the expected time limit.
         * 操作未在预期时间内完成。
         */
        timed_out,

        /**
         * @brief Resource is busy
         *        资源当前不可用
         *
         * The required resource is temporarily unavailable or locked.
         * 所需资源暂时不可用或被锁定。
         */
        busy,

        /**
         * @brief Unclassified error
         *        未分类错误
         *
         * An unspecified error occurred that does not fit other categories.
         * 发生了不属于其他类别的未指定错误。
         */
        error
    };
}

namespace rainy::foundation::concurrency::implements {
    /**
     * @brief 调度线程句柄
     *
     * handle:
     *      平台相关线程对象句柄（pthread_t / HANDLE 等的抽象表示）
     *
     * tid:
     *      系统线程 ID，用于调试或标识用途。
     *
     * 该结构为轻量值类型，不拥有线程生命周期。
     */
    struct schd_thread_t {
        std::uintptr_t handle;
        std::uint64_t tid;
    };
}

namespace rainy::foundation::concurrency::implements {
    /**
     * @brief 结束当前线程（无返回值版本）等价于调用平台线程退出函数。
     * @attention 不会执行调用栈之后的代码。
     */
    RAINY_TOOLKIT_API void endthread();

    /**
     * @brief 结束当前线程并设置返回码
     * @param return_code 线程退出码
     */
    RAINY_TOOLKIT_API void endthreadex(unsigned int return_code);

    /**
     * @brief 创建线程
     *
     * @param invoke_function_addr 线程入口函数
     * @param stack_size           栈大小（0 表示使用系统默认）
     * @param arg_list             传递给线程入口的参数
     *
     * @return 新创建线程的句柄
     *
     * 若创建失败，返回的 handle 可能为 0。
     */
    RAINY_NODISCARD RAINY_TOOLKIT_API schd_thread_t create_thread(
        functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr, unsigned int stack_size, void *arg_list);
    /**
     * @brief 创建线程
     *
     * @param security   平台安全属性（可能被忽略）
     * @param stack_size 栈大小
     * @param invoke_function_addr 线程入口
     * @param arg_list   用户参数
     * @param init_flag  初始化标志（平台相关）
     * @param thrd_addr  可选线程ID输出
     *
     * 未使用的参数在某些平台可能被忽略。
     */
    RAINY_NODISCARD RAINY_TOOLKIT_API schd_thread_t
    create_thread(void *security, unsigned int stack_size, functional::function_pointer<unsigned int (*)(void *)> invoke_function_addr,
                  void *arg_list, unsigned int init_flag, std::uint64_t *thrd_addr);

    /**
     * @brief 等待线程结束
     *
     * @param thread_handle 目标线程
     * @param result_receiver 接收线程返回值，可为 nullptr
     *
     * @return
     *  - success     成功等待
     *  - busy        不可等待
     *  - error       等待失败
     *
     * 调用成功后线程资源被回收。
     */
    RAINY_TOOLKIT_API thrd_result thread_join(schd_thread_t thread_handle, std::int64_t *result_receiver) noexcept;

    /**
     * @brief 分离线程
     *
     * 线程结束后由系统自动回收资源。
     * 调用后不可再 join。
     */
    RAINY_TOOLKIT_API thrd_result thread_detach(schd_thread_t thread_handle) noexcept;

    /**
     * @brief 返回系统可用硬件并发数
     *
     * @remark
     * 返回值为系统报告的 CPU 逻辑核心数量。
     * 该值仅供参考。
     */
    RAINY_TOOLKIT_API unsigned int thread_hardware_concurrency() noexcept;

    /**
     * @brief 获取当前线程 ID
     */
    RAINY_TOOLKIT_API std::uint64_t get_thread_id() noexcept;

    /**
     * @brief 主动让出 CPU 时间片
     */
    RAINY_TOOLKIT_API void thread_yield() noexcept;

    /**
     * @brief 当前线程休眠指定毫秒
     * @remark 精度取决于操作系统调度器。
     */
    RAINY_TOOLKIT_API void thread_sleep_for(unsigned long ms) noexcept;

    /**
     * @brief 挂起线程
     *
     * @attention 可能导致死锁风险，仅用于受控环境。
     */
    RAINY_TOOLKIT_API void suspend_thread(schd_thread_t thread_handle) noexcept;

    /**
     * @brief 恢复被挂起线程
     */
    RAINY_TOOLKIT_API void resume_thread(schd_thread_t thread_handle) noexcept;
}

namespace rainy::foundation::concurrency::implements {
    /**
     * @brief mutex 类型标志
     *
     * 可按位组合：
     *
     * plain_mtx      普通互斥锁
     * try_mtx        支持 trylock
     * timed_mtx      支持 timedlock
     * recursive_mtx  递归锁
     */
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
    RAINY_TOOLKIT_API thrd_result mtx_do_lock(mtx_t *const mtx, const ::timespec *target) noexcept;

    /**
     * @brief 初始化mutex锁
     * @attention 此函数需要一个有效的实例句柄。这个句柄指向了implements::mutex_handle实例。如果有必要
     * @attention 考虑调用mtx_create，它将为你自动创建实例并调用此初始化。只需要在不需要它的时候，使用mtx_destory即可
     * @param mtx mutex锁的handle
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_init(mtx_t *const mtx, int flags) noexcept;

    /**
     * @brief 创建mutex锁
     * @attention 此函数将会覆写mtx句柄，并且将会从动态存储空间获取一段内存用于存储。且会自动调用mtx_init进行自动的初始化
     * @attention 因此，如果它没有被mtx_destory销毁，可能会导致内存泄漏
     * @param mtx mutex锁的handle，用于接收新的实例
     * @param flags 从mutex_types中选定，表示该mutex锁的类型
     * @return 一般返回success来表示成功。但是如果没有句柄用于接收，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_create(mtx_t *const mtx, int flags) noexcept;

    /**
     * @brief 使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_lock(mtx_t *const mtx) noexcept;

    /**
     * @brief 尝试使用mutex锁进行上锁操作
     * @param mtx mutex锁的handle，用于上锁操作
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem。
     * @return 此外，如果互斥锁已被其他线程占用。busy将是此函数的返回值
     */
    RAINY_TOOLKIT_API thrd_result mtx_trylock(mtx_t *const mtx) noexcept;

    /**
     * @brief 解锁mutex锁该函数释放互斥锁
     * @brief 如果当前线程持有该锁，则将其解锁
     * @param mtx mutex锁的handle
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     */
    RAINY_TOOLKIT_API thrd_result mtx_unlock(mtx_t *const mtx) noexcept;

    /**
     * @brief 尝试对mutex锁进行锁定，支持超时机制
     * @param mtx mutex锁的handle
     * @param xt 超时参数，表示锁定的最大时间
     * @return 一般返回success来表示成功。但是，如果mtx为空，errno将被设置为EINVAL，并返回nomem
     * @return 如果在指定的时间内无法获取锁，则返回timed_out
     */
    RAINY_TOOLKIT_API thrd_result mtx_timedlock(mtx_t *const mtx, const ::timespec *xt) noexcept;

    /**
     * @brief 检查当前线程是否已经持有指定的互斥锁
     * @param mtx mutex锁的handle
     * @return bool 如果当前线程持有该互斥锁，返回true，否则返回false
     */
    RAINY_TOOLKIT_API bool mtx_current_owns(mtx_t *const mtx) noexcept;

    /**
     * @brief 销毁 mutex 锁
     *
     * 释放互斥锁关联的系统资源。
     *
     * @param mtx mutex锁的handle
     *
     * @return
     *  - success 成功销毁
     *  - nomem   mtx 为空或无效（errno = EINVAL）
     *
     * @warning
     *  - 若仍有线程持有该锁，行为未定义。
     *  - destroy 后 mtx 不再可用，必须重新初始化。
     */
    RAINY_TOOLKIT_API thrd_result mtx_destroy(mtx_t *const mtx) noexcept;

    /**
     * @brief 获取底层平台 mutex 句柄
     *
     * 返回值类型依赖平台：
     *  - Linux: pthread_mutex_t*
     *  - Windows: CRITICAL_SECTION / HANDLE 等
     *
     * 仅用于底层集成或调试用途。
     */
    RAINY_TOOLKIT_API void *native_mtx_handle(mtx_t *const mtx) noexcept;
}

namespace rainy::foundation::concurrency::implements {
    /**
     * @brief 创建线程局部存储键（TSS Key）
     *
     * 每个线程可为该 key 存储独立的指针值。
     *
     * @return 有效 handle，失败时返回无效值。
     */
    RAINY_TOOLKIT_API core::handle tss_create();

    /**
     * @brief 获取当前线程的 TSS 值
     *
     * 若未设置返回 nullptr。
     */
    RAINY_TOOLKIT_API void *tss_get(core::handle tss_key);

    /**
     * @brief 设置当前线程的 TSS 值
     *
     * @return true 成功，否则失败
     */
    RAINY_TOOLKIT_API bool tss_set(core::handle tss_key, const void *value);

    /**
     * @brief 删除 TSS 键
     *
     * 删除后所有线程上的关联值失效。
     * 不负责释放 value 指向的内存。
     */
    RAINY_TOOLKIT_API bool tss_delete(core::handle tss_key);
}

namespace rainy::foundation::concurrency::implements {
    using cnd_t = void *;

    /**
     * @brief 条件变量初始化
     *
     * @param cnd 条件变量句柄
     * @return
     *  - success 成功
     *  - nomem  cnd 无效
     */
    RAINY_TOOLKIT_API thrd_result cnd_init(cnd_t *const cnd) noexcept;

    /**
     * @brief 创建条件变量
     *
     * @param cnd 条件变量句柄
     * @return
     *  - success 成功
     *  - nomem  内存不足或句柄无效
     */
    RAINY_TOOLKIT_API thrd_result cnd_create(cnd_t *cnd) noexcept;

    /**
     * @brief 条件变量等待
     *
     * 调用该函数时，必须先锁定 mutex。
     * 调用后 mutex 会被自动释放，等待被唤醒后重新获取 mutex。
     *
     * @param cnd 条件变量
     * @param mtx 与之关联的 mutex
     * @return
     *  - success 成功
     *  - nomem  无效参数
     *  - timed_out 超时
     */
    RAINY_TOOLKIT_API thrd_result cnd_wait(cnd_t *const cnd, mtx_t *const mtx) noexcept;

    /**
     * @brief 条件变量等待，带超时
     *
     * @param cnd 条件变量
     * @param mtx mutex
     * @param timeout 绝对时间（timespec）
     * @return
     *  - success 成功
     *  - timed_out 超时
     *  - nomem 无效参数
     */
    RAINY_TOOLKIT_API thrd_result cnd_timedwait(cnd_t *const cnd, mtx_t *const mtx, const ::timespec *timeout) noexcept;

    /**
     * @brief 唤醒一个等待线程
     *
     * @param cnd 条件变量
     * @return success / nomem
     */
    RAINY_TOOLKIT_API thrd_result cnd_signal(cnd_t *const cnd) noexcept;

    /**
     * @brief 唤醒所有等待线程
     *
     * @param cnd 条件变量
     * @return success / nomem
     */
    RAINY_TOOLKIT_API thrd_result cnd_broadcast(cnd_t *const cnd) noexcept;

    /**
     * @brief 条件变量销毁
     *
     * @param cnd 条件变量
     * @return
     *  - success 成功
     *  - nomem 无效句柄
     *
     * @warning 未释放等待线程的条件变量销毁行为未定义。
     */
    RAINY_TOOLKIT_API thrd_result cnd_destroy(cnd_t *cnd) noexcept;

    /**
     * @brief 获取底层平台条件变量原生句柄
     */
    RAINY_TOOLKIT_API void *native_cnd_handle(cnd_t *const cnd) noexcept;
}

namespace rainy::foundation::concurrency::implements {
    using smtx_t = void *;

    /**
     * @brief 初始化 shared_mutex
     *
     * @attention
     * 需要一个有效的实例句柄，该句柄必须指向
     * implements::shared_mutex_handle。
     *
     * 如需自动分配实例，请使用 smtx_create。
     *
     * @param smtx shared_mutex 句柄
     *
     * @return
     *  - success 初始化成功
     *  - nomem   smtx 为空（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_init(smtx_t *const smtx) noexcept;

    /**
     * @brief 创建 shared_mutex
     *
     * 从动态存储空间分配实例，并自动调用 smtx_init。
     *
     * @warning
     * 若未调用 smtx_destroy，可能导致内存泄漏。
     *
     * @param smtx 用于接收实例的句柄
     *
     * @return
     *  - success 创建成功
     *  - nomem   无效句柄（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_create(smtx_t *const smtx) noexcept;

    /**
     * @brief 以独占模式锁定 shared_mutex
     *
     * 阻塞直到获得独占访问权限。
     *
     * 独占锁与：
     *  - 所有 shared 锁互斥
     *  - 其他 exclusive 锁互斥
     *
     * @param smtx shared_mutex 句柄
     */
    RAINY_TOOLKIT_API thrd_result smtx_lock(smtx_t *const smtx) noexcept;

    /**
     * @brief 以共享模式锁定 shared_mutex
     *
     * 多个线程可同时持有 shared 锁，
     * 但会与 exclusive 锁互斥。
     *
     * @param smtx shared_mutex 句柄
     */
    RAINY_TOOLKIT_API thrd_result smtx_lock_shared(smtx_t *const smtx) noexcept;

    /**
     * @brief 尝试获取独占锁
     *
     * @return
     *  - success 获取成功
     *  - busy    当前无法获取
     *  - nomem   smtx 无效（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_try_lock(smtx_t *const smtx) noexcept;

    /**
     * @brief 尝试获取共享锁
     *
     * @return
     *  - success 获取成功
     *  - busy    当前无法获取
     *  - nomem   smtx 无效（errno = EINVAL）
     */
    RAINY_TOOLKIT_API thrd_result smtx_try_lock_shared(smtx_t *const smtx) noexcept;

    /**
     * @brief 释放独占锁
     *
     * 当前线程必须持有 exclusive 锁。
     */
    RAINY_TOOLKIT_API thrd_result smtx_unlock(smtx_t *const smtx) noexcept;

    /**
     * @brief 释放共享锁
     *
     * 当前线程必须持有 shared 锁之一。
     */
    RAINY_TOOLKIT_API thrd_result smtx_unlock_shared(smtx_t *const smtx) noexcept;

    /**
     * @brief 销毁 shared_mutex
     *
     * 释放关联的系统资源。
     *
     * @warning
     * 若仍有线程持有锁，行为未定义。
     */
    RAINY_TOOLKIT_API thrd_result smtx_destroy(smtx_t *const smtx) noexcept;

    /**
     * @brief 获取底层平台 shared_mutex 句柄
     *
     * 返回类型依赖平台：
     *  - Windows : SRWLOCK*
     *  - Linux   : pthread_rwlock_t*
     *
     * 仅用于底层集成或调试。
     */
    RAINY_TOOLKIT_API void *native_smtx_handle(smtx_t *const smtx) noexcept;
}

#endif
