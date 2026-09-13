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
#include <rainy/core/layer.hpp>
#include <rainy/core/concurrency/atomicinfra/atomic_ops.hpp>

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::concurrency::implements {
    /**
     * \lang english
     * @brief Storage class implementing the atomic_flag semantics.
     *
     *  Holds a single byte used as a boolean flag and provides the low-level
     *  test / test_and_set / clear operations together with the wait/notify
     *  interface. This class is an implementation detail; the public interface
     *  is exposed through atomic_flag.
     *
     * \lang simp-chinese
     * @brief 实现 atomic_flag 语义的存储类。
     *
     *  持有一个用作布尔标志的单字节，并提供底层的 test / test_and_set / clear
     *  操作以及 wait/notify 接口。本类为实现细节；公开接口通过 atomic_flag 暴露。
     */
    class atomic_flag_storage {
    public:
        /**
         * \lang english
         * @brief The underlying storage type used to represent the flag.
         *
         * \lang simp-chinese
         * @brief 用于表示该标志的底层存储类型。
         */
        using storage_type = std::int8_t;

        /**
         * \lang english
         * @brief The operation set used to perform the underlying atomic primitives.
         *
         * \lang simp-chinese
         * @brief 用于执行底层原子原语的操作集。
         */
        using ops = atomic_ops<storage_type>;

        /**
         * \lang english
         * @brief The storage value representing the cleared (false) state.
         *
         * \lang simp-chinese
         * @brief 表示已清除（false）状态的存储值。
         */
        static constexpr storage_type clear_value = 0;

        /**
         * \lang english
         * @brief The storage value representing the set (true) state.
         *
         * \lang simp-chinese
         * @brief 表示已设置（true）状态的存储值。
         */
        static constexpr storage_type set_value = 1;

        /**
         * \lang english
         * @brief Default constructor. Initializes the flag to the cleared state.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。将标志初始化为已清除状态。
         */
        constexpr atomic_flag_storage() noexcept : storage_(clear_value) {
        }

        /**
         * \lang english
         * @brief Atomically reads the current value of the flag.
         *
         * @param order The memory order for the load.
         * @return true if the flag is set, false otherwise.
         *
         * \lang simp-chinese
         * @brief 原子地读取该标志的当前值。
         *
         * @param order 加载的内存顺序。
         * @return 若标志已设置则返回 true，否则返回 false。
         */
        bool test(memory_order order) const noexcept {
            return ops::load(ptr(), order) != clear_value;
        }

        /**
         * \lang english
         * @brief Atomically reads the current value of the flag (volatile overload).
         *
         * @param order The memory order for the load.
         * @return true if the flag is set, false otherwise.
         *
         * \lang simp-chinese
         * @brief 原子地读取该标志的当前值（对 volatile 类型的支持）。
         *
         * @param order 加载的内存顺序。
         * @return 若标志已设置则返回 true，否则返回 false。
         */
        bool test(memory_order order) const volatile noexcept {
            return ops::load(ptr(), order) != clear_value;
        }

        /**
         * \lang english
         * @brief Atomically sets the flag to true and returns its previous value.
         *
         * @param order The memory order for the exchange.
         * @return The value of the flag before the operation.
         *
         * \lang simp-chinese
         * @brief 原子地将标志设置为 true 并返回其先前的值。
         *
         * @param order 交换的内存顺序。
         * @return 操作前该标志的值。
         */
        bool test_and_set(memory_order order) noexcept {
            return ops::exch(ptr(), set_value, order) != clear_value;
        }

        /**
         * \lang english
         * @brief Atomically sets the flag to true and returns its previous value
         *        (volatile overload).
         *
         * @param order The memory order for the exchange.
         * @return The value of the flag before the operation.
         *
         * \lang simp-chinese
         * @brief 原子地将标志设置为 true 并返回其先前的值（对 volatile 类型的支持）。
         *
         * @param order 交换的内存顺序。
         * @return 操作前该标志的值。
         */
        bool test_and_set(memory_order order) volatile noexcept {
            return ops::exch(ptr(), set_value, order) != clear_value;
        }

        /**
         * \lang english
         * @brief Atomically sets the flag to false.
         *
         * @param order The memory order for the store.
         *
         * \lang simp-chinese
         * @brief 原子地将标志设置为 false。
         *
         * @param order 存储的内存顺序。
         */
        void clear(memory_order order) noexcept {
            ops::store(ptr(), clear_value, order);
        }

        /**
         * \lang english
         * @brief Atomically sets the flag to false (volatile overload).
         *
         * @param order The memory order for the store.
         *
         * \lang simp-chinese
         * @brief 原子地将标志设置为 false（对 volatile 类型的支持）。
         *
         * @param order 存储的内存顺序。
         */
        void clear(memory_order order) volatile noexcept {
            ops::store(ptr(), clear_value, order);
        }

        /**
         * \lang english
         * @brief Blocks until notified and the flag no longer equals old.
         *
         * @param old The value to compare against.
         * @param order The memory order for the wait.
         *
         * \lang simp-chinese
         * @brief 阻塞直到被通知且该标志不再等于 old。
         *
         * @param old 用于比较的值。
         * @param order 等待的内存顺序。
         */
        void wait(bool old, memory_order order) const noexcept {
            storage_type old_val = old ? set_value : clear_value;
            atomic_wait_impl(ptr(), old_val, order);
        }

        /**
         * \lang english
         * @brief Blocks until notified and the flag no longer equals old
         *        (volatile overload).
         *
         * @param old The value to compare against.
         * @param order The memory order for the wait.
         *
         * \lang simp-chinese
         * @brief 阻塞直到被通知且该标志不再等于 old（对 volatile 类型的支持）。
         *
         * @param old 用于比较的值。
         * @param order 等待的内存顺序。
         */
        void wait(bool old, memory_order order) const volatile noexcept {
            storage_type old_val = old ? set_value : clear_value;
            atomic_wait_impl(ptr(), old_val, order);
        }

        /**
         * \lang english
         * @brief Notifies at least one thread blocked in wait().
         *
         * \lang simp-chinese
         * @brief 通知至少一个阻塞在 wait() 中的线程。
         */
        void notify_one() noexcept {
            atomic_notify_one_impl(ptr());
        }

        /**
         * \lang english
         * @brief Notifies at least one thread blocked in wait() (volatile overload).
         *
         * \lang simp-chinese
         * @brief 通知至少一个阻塞在 wait() 中的线程（对 volatile 类型的支持）。
         */
        void notify_one() volatile noexcept {
            atomic_notify_one_impl(ptr());
        }

        /**
         * \lang english
         * @brief Notifies all threads blocked in wait().
         *
         * \lang simp-chinese
         * @brief 通知所有阻塞在 wait() 中的线程。
         */
        void notify_all() noexcept {
            atomic_notify_all_impl(ptr());
        }

        /**
         * \lang english
         * @brief Notifies all threads blocked in wait() (volatile overload).
         *
         * \lang simp-chinese
         * @brief 通知所有阻塞在 wait() 中的线程（对 volatile 类型的支持）。
         */
        void notify_all() volatile noexcept {
            atomic_notify_all_impl(ptr());
        }

    protected:
        /**
         * \lang english
         * @brief Returns a pointer to the underlying storage.
         *
         * \lang simp-chinese
         * @brief 返回指向底层存储的指针。
         */
        volatile storage_type *ptr() noexcept {
            return &storage_;
        }

        /**
         * \lang english
         * @brief Returns a pointer to the underlying storage (volatile overload).
         *
         * \lang simp-chinese
         * @brief 返回指向底层存储的指针（对 volatile 类型的支持）。
         */
        volatile storage_type *ptr() volatile noexcept {
            return &storage_;
        }

        /**
         * \lang english
         * @brief Returns a pointer to the underlying storage (const volatile overload).
         *
         * \lang simp-chinese
         * @brief 返回指向底层存储的指针（const volatile 重载）。
         */
        const volatile storage_type *ptr() const volatile noexcept {
            return &storage_;
        }

    private:
        alignas(sizeof(storage_type)) volatile storage_type storage_;
    };
}

#endif

#endif