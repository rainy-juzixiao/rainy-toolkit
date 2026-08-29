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
    using layer::memory_order;

    using layer::memory_order_acq_rel;
    using layer::memory_order_acquire;
    using layer::memory_order_consume;
    using layer::memory_order_relaxed;
    using layer::memory_order_release;
    using layer::memory_order_seq_cst;

    RAINY_INLINE void atomic_thread_fence(memory_order order) noexcept {
        layer::atomic_thread_fence(order);
    }

    RAINY_INLINE void atomic_signal_fence(memory_order order) noexcept {
        if (order != memory_order::relaxed) {
            rainy_compiler_barrier();
        }
    }
}

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
