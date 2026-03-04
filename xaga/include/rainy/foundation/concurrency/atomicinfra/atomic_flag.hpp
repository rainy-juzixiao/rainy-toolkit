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
#ifndef RAINY_FOUNDATION_CONCURRENCY_IMPLEMENTS_ATOMIC_FLAG_HPP
#define RAINY_FOUNDATION_CONCURRENCY_IMPLEMENTS_ATOMIC_FLAG_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/atomicinfra/atomic_ops.hpp>

namespace rainy::foundation::concurrency::implements {
    class atomic_flag_storage {
    public:
        using storage_type = std::int8_t;
        using ops = atomic_ops<storage_type>;

        static constexpr storage_type clear_value = 0;
        static constexpr storage_type set_value = 1;

        constexpr atomic_flag_storage() noexcept : storage_(clear_value) {
        }

        bool test(memory_order order) const noexcept {
            return ops::load(ptr(), order) != clear_value;
        }
        
        bool test(memory_order order) const volatile noexcept {
            return ops::load(ptr(), order) != clear_value;
        }

        bool test_and_set(memory_order order) noexcept {
            return ops::exch(ptr(), set_value, order) != clear_value;
        }
        
        bool test_and_set(memory_order order) volatile noexcept {
            return ops::exch(ptr(), set_value, order) != clear_value;
        }

        void clear(memory_order order) noexcept {
            ops::store(ptr(), clear_value, order);
        }
        
        void clear(memory_order order) volatile noexcept {
            ops::store(ptr(), clear_value, order);
        }

        void wait(bool old, memory_order order) const noexcept {
            storage_type old_val = old ? set_value : clear_value;
            atomic_wait_impl(ptr(), old_val, order);
        }

        void wait(bool old, memory_order order) const volatile noexcept {
            storage_type old_val = old ? set_value : clear_value;
            atomic_wait_impl(ptr(), old_val, order);
        }

        void notify_one() noexcept {
            atomic_notify_one_impl(ptr());
        }

        void notify_one() volatile noexcept {
            atomic_notify_one_impl(ptr());
        }

        void notify_all() noexcept {
            atomic_notify_all_impl(ptr());
        }

        void notify_all() volatile noexcept {
            atomic_notify_all_impl(ptr());
        }

    protected:
        volatile storage_type *ptr() noexcept {
            return &storage_;
        }

        volatile storage_type *ptr() volatile noexcept {
            return &storage_;
        }

        const volatile storage_type *ptr() const volatile noexcept {
            return &storage_;
        }

    private:
        alignas(sizeof(storage_type)) volatile storage_type storage_;
    };
}

#endif