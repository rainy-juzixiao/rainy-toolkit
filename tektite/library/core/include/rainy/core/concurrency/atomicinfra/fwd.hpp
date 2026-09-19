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
#ifndef RAINY_FOUNDATION_CONCURRENCY_ATOMICINFRA_FWD_HPP
#define RAINY_FOUNDATION_CONCURRENCY_ATOMICINFRA_FWD_HPP
#include <rainy/core/layer.hpp>

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Memory order enumeration used by atomic operations.
     *
     * \lang simp-chinese
     * @brief 原子操作使用的内存顺序枚举。
     */
    using layer::memory_order;

    /**
     * \lang english
     * @brief Acquire-release memory ordering.
     *
     * \lang simp-chinese
     * @brief 获取-释放内存顺序。
     */
    using layer::memory_order_acq_rel;
    /**
     * \lang english
     * @brief Acquire memory ordering.
     *
     * \lang simp-chinese
     * @brief 获取内存顺序。
     */
    using layer::memory_order_acquire;
    /**
     * \lang english
     * @brief Consume memory ordering.
     *
     * \lang simp-chinese
     * @brief 消费内存顺序。
     */
    using layer::memory_order_consume;
    /**
     * \lang english
     * @brief Relaxed memory ordering.
     *
     * \lang simp-chinese
     * @brief 宽松内存顺序。
     */
    using layer::memory_order_relaxed;
    /**
     * \lang english
     * @brief Release memory ordering.
     *
     * \lang simp-chinese
     * @brief 释放内存顺序。
     */
    using layer::memory_order_release;
    /**
     * \lang english
     * @brief Sequentially consistent memory ordering.
     *
     * \lang simp-chinese
     * @brief 顺序一致内存顺序。
     */
    using layer::memory_order_seq_cst;

    /**
     * \lang english
     * @brief Establishes a thread fence with the given memory order.
     *
     * @param order The memory order of the fence.
     *
     * \lang simp-chinese
     * @brief 以指定的内存顺序建立线程栅栏。
     *
     * @param order 栅栏的内存顺序。
     */
    RAINY_INLINE void atomic_thread_fence(memory_order order) noexcept {
        layer::atomic_thread_fence(order);
    }

    /**
     * \lang english
     * @brief Establishes a signal fence with the given memory order.
     *
     * @param order The memory order of the fence.
     *
     * \lang simp-chinese
     * @brief 以指定的内存顺序建立信号栅栏。
     *
     * @param order 栅栏的内存顺序。
     */
    RAINY_INLINE void atomic_signal_fence(memory_order order) noexcept {
        if (order != memory_order::relaxed) {
            rainy_compiler_barrier();
        }
    }
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::core::concurrency::implements {
    template <typename Ty>
    void atomic_wait_impl(const volatile Ty *address, Ty old_val, memory_order /*order*/) noexcept {
        layer::atomic_wait(const_cast<Ty *>(address), &old_val, sizeof(Ty));
    }

    template <typename Ty>
    void atomic_wait_impl_with_callback(const volatile Ty *address, Ty old_val, memory_order /*order*/,
                                        layer::atomic_wait_equal_fn are_equal, void *param = nullptr) noexcept {
        layer::atomic_wait(const_cast<Ty *>(address), &old_val, sizeof(Ty), are_equal, param);
    }

    template <typename Ty>
    void atomic_notify_one_impl(const volatile Ty *address) noexcept {
        layer::atomic_notify_one(const_cast<Ty *>(address), sizeof(Ty));
    }

    template <typename Ty>
    void atomic_notify_all_impl(const volatile Ty *address) noexcept {
        layer::atomic_notify_all(const_cast<Ty *>(address), sizeof(Ty));
    }
}
#endif

#endif
