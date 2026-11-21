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
 * @file rcu.hpp
 * @module Foundation/Concurrency
 *
 * \lang english
 * @brief
 * This header provides RCU (Read-Copy-Update) synchronization primitives for
 * rainy-toolkit. An rcu_domain manages grace periods, readers enter and
 * leave read-side critical sections via lock()/unlock(), and writers defer
 * reclamation through rcu_synchronize, rcu_barrier, rcu_retire or
 * rcu_obj_base.
 * @brief It is not intended to replace mutex-based synchronization; instead,
 *        it targets read-mostly workloads where readers must be wait-free.
 * @author rainy-juzixiao
 * @date 9/1/2026 Initial annotation added (by rainy-juzixiao)
 *
 * \lang simp-chinese
 * @brief
 * 此头文件为rainy-toolkit提供RCU（Read-Copy-Update）同步原语。
 * rcu_domain负责管理宽限期，读者通过lock()/unlock()进出读侧临界区，而写侧通过
 * rcu_synchronize、rcu_barrier、rcu_retire或rcu_obj_base进行延迟回收
 * @brief 它并非用于取代基于互斥量的同步；而是面向读多写少的场景，
 *        即读者必须无阻塞（wait-free）的场合
 * @author rainy-juzixiao
 * @date 9/1/2026 在此进行添加注释（由rainy-juzixiao添加）
 */
#ifndef RAINY_FOUNDATION_CONCURRENCY_RCU_HPP
#define RAINY_FOUNDATION_CONCURRENCY_RCU_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/concurrency/atomic.hpp>
#include <rainy/core/memory/nebula_ptr.hpp>

namespace rainy::foundation::concurrency::implements {
    struct rcu_domain_state;
}

namespace rainy::foundation::concurrency {
    /**
     * \lang english
     * @brief A read-side protection domain for RCU.
     *        It tracks grace periods: readers mark themselves active through
     *        lock()/unlock(), and deferred reclamation operations are only
     *        allowed to complete once all readers that were active at the
     *        start of a grace period have left their critical sections.
     * @remark Not copyable, not assignable.
     *
     * \lang simp-chinese
     * @brief RCU的读侧保护域。
     *        它负责追踪宽限期（grace period）：读者通过lock()/unlock()将自己标记为活跃，
     *        只有当宽限期开始时活跃的所有读者都离开临界区后，延迟回收操作才被允许完成
     * @remark 不可复制，不可赋值。
     */
    class RAINY_TOOLKIT_API rcu_domain {
    public:
        /**
         * \lang english
         * @brief Constructs an RCU domain.
         *        Creates an independent grace-period tracking structure.
         *
         * \lang simp-chinese
         * @brief 构造RCU域。
         *        创建独立的宽限期追踪结构。
         */
        rcu_domain() noexcept;

        /**
         * \lang english
         * @brief Destroys the RCU domain.
         * @pre No thread may still hold a read-side lock on this domain.
         *
         * \lang simp-chinese
         * @brief 析构RCU域。
         * @pre 不得存在任何仍持有该域读锁的线程。
         */
        ~rcu_domain();

        rcu_domain(const rcu_domain &) = delete;
        rain_fn operator=(const rcu_domain &)->rcu_domain & = delete;

        /**
         * \lang english
         * @brief Enters a read-side critical section.
         *        Marks the current thread as an active reader. Blocking,
         *        calling rcu_synchronize, or any operation that may trigger
         *        grace-period processing is not allowed while inside a
         *        read-side critical section.
         * @note This is normally a lightweight, barrier-level operation and
         *       is never blocked by other readers.
         *
         * \lang simp-chinese
         * @brief 进入RCU读侧临界区。
         *        将当前线程标记为活跃读者。读侧临界区内不允许阻塞、
         *        调用rcu_synchronize或任何可能触发宽限期判定的操作
         * @note 该操作通常是屏障级别的轻量操作，
         *       不会被其他读者阻塞。
         */
        rain_fn lock() noexcept -> void;

        /**
         * \lang english
         * @brief Tries to enter a read-side critical section without blocking.
         * @return Always returns true; the effect is equivalent to lock()
         * @retval true  The read-side critical section was entered
         *
         * \lang simp-chinese
         * @brief 尝试进入读侧临界区。
         * @return 总是返回true；其效果等价于lock()
         * @retval true  成功进入读侧临界区
         */
        RAINY_NODISCARD rain_fn try_lock() noexcept -> bool;

        /**
         * \lang english
         * @brief Leaves the read-side critical section.
         *        Clears the active-reader mark of the current thread and may
         *        advance grace-period processing. The current thread must
         *        hold the read-side lock of this domain.
         *
         * \lang simp-chinese
         * @brief 离开读侧临界区。
         *        清除当前线程的活跃读者标记，并可能推动宽限期前进。
         *        当前线程必须持有该域的读锁。
         */
        rain_fn unlock() noexcept -> void;

    private:
        implements::rcu_domain_state *state_;

        friend RAINY_TOOLKIT_API rain_fn rcu_synchronize(rcu_domain &dom) noexcept -> void;
        friend RAINY_TOOLKIT_API rain_fn rcu_barrier(rcu_domain &dom) noexcept -> void;
    };

    /**
     * \lang english
     * @brief Returns the default global RCU domain.
     *        All RCU operations that are not given an explicit domain operate
     *        on this domain.
     * @return A reference to the default rcu_domain, whose lifetime covers
     *         the whole program
     *
     * \lang simp-chinese
     * @brief 获取默认的全局RCU域。
     *        所有未显式指定域的RCU操作均作用于该域
     * @return 一个指向默认rcu_domain的引用，其生命周期覆盖整个程序
     */
    RAINY_TOOLKIT_API rain_fn rcu_default_domain() noexcept -> rcu_domain &;

    /**
     * \lang english
     * @brief Blocks until the end of the current grace period.
     *        Blocks the calling thread until all readers that had called
     *        lock() before the call to rcu_synchronize have left their
     *        read-side critical sections. Writers use this to synchronize
     *        safely after updating shared data.
     * @param dom The target RCU domain, defaults to rcu_default_domain()
     * @note It does not wait for readers that enter their critical sections
     *       after the call to rcu_synchronize.
     * @warning Calling this inside a read-side critical section leads to a
     *          deadlock.
     *
     * \lang simp-chinese
     * @brief 阻塞等待当前宽限期结束。
     *        阻塞调用线程，直到在调用rcu_synchronize之前已调用lock()的所有读者
     *        均离开读侧临界区。写侧在更新共享数据后，用它来安全地同步
     * @param dom 目标RCU域，默认为rcu_default_domain()
     * @note 它不会等待在调用rcu_synchronize之后才进入临界区的读者
     * @warning 在读侧临界区内调用会导致死锁
     */
    RAINY_TOOLKIT_API rain_fn rcu_synchronize(rcu_domain &dom = rcu_default_domain()) noexcept -> void;

    /**
     * \lang english
     * @brief Waits until all deferred reclamation operations queued in the
     *        domain have completed.
     *        Blocks until every previously queued retire operation in the
     *        domain (whether queued through rcu_retire or
     *        rcu_obj_base::retire) has run to completion. It behaves like a
     *        cross-thread destruction barrier.
     * @param dom The target RCU domain, defaults to rcu_default_domain()
     * @note After rcu_barrier returns, it is guaranteed that all previously
     *       queued deleters have run; this is commonly used before unloading
     *       a module to ensure no dangling destruction.
     *
     * \lang simp-chinese
     * @brief 等待域中所有已排队的延迟回收操作完成。
     *        阻塞直到该域内所有先前排队的retire操作（无论是通过rcu_retire
     *        还是通过rcu_obj_base::retire排队的）全部执行完毕。
     *        其行为类似一个跨线程的析构屏障
     * @param dom 目标RCU域，默认为rcu_default_domain()
     * @note rcu_barrier返回后，可保证之前排队的删除器已运行完毕；
     *       常用于模块卸载前确保无悬垂析构
     */
    RAINY_TOOLKIT_API rain_fn rcu_barrier(rcu_domain &dom = rcu_default_domain()) noexcept -> void;

    /**
     * \lang english
     * @brief Places an object into the deferred reclamation queue.
     *        Registers @p p together with the deleter @p d in the given
     *        domain; once the grace period ends, the domain invokes d(p) at
     *        an appropriate time. After calling, the caller must immediately
     *        give up all access to @p p.
     * @tparam Ty The complete object type to be reclaimed
     * @tparam Dx The deleter type
     * @param p Pointer to the object to be reclaimed, must not be null
     * @param d Deleter invoked with p after the grace period ends
     * @param dom The target RCU domain, defaults to rcu_default_domain()
     * @pre @p p is not null, and *p is no longer accessed by the calling
     *      thread afterwards.
     * @throws Any exception thrown by the initialization of the deleter, or
     *         an allocation failure; if it exits via an exception, no
     *         evaluation is scheduled.
     *
     * \lang simp-chinese
     * @brief 将对象排入延迟回收队列。
     *        将@p p与删除器@d一并登记到指定域；待宽限期结束后，
     *        由域在合适的时机调用d(p)完成回收。调用后调用方应立刻
     *        放弃对@p p的所有访问
     * @tparam Ty 被回收对象的类型，须为完整对象类型
     * @tparam Dx 删除器类型
     * @param p 待回收对象指针，不得为空
     * @param d 删除器，宽限期结束后以p为参数调用
     * @param dom 目标RCU域，默认为rcu_default_domain()
     * @pre @p p非空，且此后*p不再被当前线程访问。
     * @throws 由删除器初始化抛出的异常，或分配失败产生的异常；
     *         若经由异常退出，则不会有任何求值被排入队列。
     */
    template <typename Ty, typename Dx = core::memory::default_deleter<Ty>>
    rain_fn rcu_retire(Ty *p, Dx d = Dx(), rcu_domain &dom = rcu_default_domain()) -> void;

    /**
     * \lang english
     * @brief A mixin base that embeds RCU reclamation capability into an
     *        object.
     *        Objects derived from this base can queue themselves for
     *        deferred reclamation via the member retire(), so the caller
     *        does not need to keep a separate pointer.
     * @tparam Ty The complete type of the object to be reclaimed
     * @tparam Dx The deleter type, defaults to core::memory::default_deleter<Ty>
     *
     * \lang simp-chinese
     * @brief 一个将RCU回收能力内嵌到对象中的混入基类。
     *        继承该基类的对象可通过成员retire()将自身排入延迟回收队列，
     *        调用方无需再持有独立指针
     * @tparam Ty 被回收对象的完整类型
     * @tparam Dx 删除器类型，默认为core::memory::default_deleter<Ty>
     */
    template <typename Ty, typename Dx = core::memory::default_deleter<Ty>>
    class rcu_obj_base {
    public:
        /**
         * \lang english
         * @brief Queues the object pointed to by this for deferred
         *        reclamation.
         *        Equivalent to rcu_retire(static_cast<Ty *>(this), d, dom).
         *        The object must not be accessed after the call.
         * @param d The deleter, default-constructed Dx
         * @param dom The target RCU domain, defaults to rcu_default_domain()
         *
         * \lang simp-chinese
         * @brief 将this指向的对象排入延迟回收队列。
         *        等价于rcu_retire(static_cast<Ty *>(this), d, dom)。
         *        调用后不得再访问对象成员
         * @param d 删除器，默认构造Dx
         * @param dom 目标RCU域，默认为rcu_default_domain()
         */
        rain_fn retire(Dx d = Dx(), rcu_domain &dom = rcu_default_domain()) noexcept -> void {
            rcu_retire(static_cast<Ty *>(this), utility::move(d), dom);
        }

    protected:
        /**
         * \lang english
         * @brief Default-constructs the base class.
         *
         * \lang simp-chinese
         * @brief 默认构造基类。
         */
        rcu_obj_base() = default;

        /**
         * \lang english
         * @brief Copy-constructs the base class.
         * @note No base state participates in copying; a copy of a derived
         *       object never carries over reclamation registration.
         *
         * \lang simp-chinese
         * @brief 拷贝构造基类。
         * @note 基类状态不参与拷贝；派生对象的拷贝不会继承回收登记
         */
        rcu_obj_base(const rcu_obj_base &) = default;

        /**
         * \lang english
         * @brief Move-constructs the base class.
         * @note No base state participates in the move.
         *
         * \lang simp-chinese
         * @brief 移动构造基类。
         * @note 基类状态不参与移动
         */
        rcu_obj_base(rcu_obj_base &&) = default;

        /**
         * \lang english
         * @brief Copy-assigns the base class.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值基类。
         */
        rain_fn operator=(const rcu_obj_base &)->rcu_obj_base & = default;

        /**
         * \lang english
         * @brief Move-assigns the base class.
         *
         * \lang simp-chinese
         * @brief 移动赋值基类。
         */
        rain_fn operator=(rcu_obj_base &&)->rcu_obj_base & = default;

        /**
         * \lang english
         * @brief Destroys the base class.
         * @warning Destroying an object after retiring it (double
         *          reclamation) is undefined behavior; the caller must
         *          ensure retire and delete never both happen.
         *
         * \lang simp-chinese
         * @brief 析构基类。
         * @warning 若对象retire后又被析构（双重回收），行为未定义；
         *          调用方须保证retire与delete不会重复发生
         */
        ~rcu_obj_base() = default;
    };

    template <typename Ty, typename Dx>
    rain_fn rcu_retire(Ty *p, Dx d, rcu_domain &dom) -> void {
        rcu_synchronize(dom);
        d(p);
    }
}

#endif
