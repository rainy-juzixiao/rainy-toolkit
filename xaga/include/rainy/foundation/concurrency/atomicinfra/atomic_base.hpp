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
#ifndef RAINY_FOUNDATION_CONCURRENCY_IMPLEMENTS_ATOMIC_BASE_HPP
#define RAINY_FOUNDATION_CONCURRENCY_IMPLEMENTS_ATOMIC_BASE_HPP
#include <rainy/foundation/concurrency/atomicinfra/atomic_ops.hpp>

namespace rainy::foundation::concurrency::implements {
    template <typename Ty, typename Ops>
    class atomic_base {
    public:
        using value_type = Ty;

        static constexpr bool is_always_lock_free = true;

        /**
         * @brief 默认构造函数
         *
         * @note 默认构造时会初始化 storage_ 成员为 Ty 类型的默认值。
         */
        atomic_base() noexcept(std::is_nothrow_default_constructible_v<Ty>) : storage_(Ty{}) {
        }

        /**
         * @brief 使用指定值构造
         *
         * @param desired 初始化值。
         */
        atomic_base(Ty desired) noexcept : storage_(desired) {
        }

        /**
         * @brief 判断是否为无锁类型
         *
         * @return 返回 true，表示该类型始终是无锁的。
         */
        rain_fn is_lock_free() const noexcept -> bool {
            return true;
        }

        /**
         * @brief 判断是否为无锁类型（对 volatile 类型的支持）
         *
         * @return 返回 true，表示该类型始终是无锁的。
         */
        rain_fn is_lock_free() const volatile noexcept -> bool {
            return true;
        }

        atomic_base(const atomic_base &) = delete;
        atomic_base &operator=(const atomic_base &) = delete;
        atomic_base &operator=(const atomic_base &) volatile = delete;

        /**
         * @brief 存储值
         *
         * @param desired 要存储的值。
         * @param order 存储的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) noexcept -> void {
            Ops::store(ptr(), desired, order);
        }

        /**
         * @brief 存储值（对 volatile 类型的支持）
         *
         * @param desired 要存储的值。
         * @param order 存储的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> void {
            Ops::store(ptr(), desired, order);
        }

        /**
         * @brief 赋值操作符
         *
         * @param desired 要赋的值。
         * @return 返回赋值后的值。
         */
        rain_fn operator=(Ty desired) noexcept -> Ty {
            store(desired);
            return desired;
        }

        /**
         * @brief 赋值操作符（对 volatile 类型的支持）
         *
         * @param desired 要赋的值。
         * @return 返回赋值后的值。
         */
        rain_fn operator=(Ty desired) volatile noexcept -> Ty {
            store(desired);
            return desired;
        }

        /**
         * @brief 加载值
         *
         * @param order 加载的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加载的值。
         */
        rain_fn load(memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return Ops::load(ptr(), order);
        }

        /**
         * @brief 加载值（对 volatile 类型的支持）
         *
         * @param order 加载的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加载的值。
         */
        rain_fn load(memory_order order = memory_order::seq_cst) const volatile noexcept -> Ty {
            return Ops::load(ptr(), order);
        }

        /**
         * @brief 类型转换操作符
         *
         * @return 返回当前值。
         */
        operator Ty() const noexcept {
            return load();
        }

        /**
         * @brief 类型转换操作符（对 volatile 类型的支持）
         *
         * @return 返回当前值。
         */
        operator Ty() const volatile noexcept {
            return load();
        }

        /**
         * @brief 交换值
         *
         * @param desired 要交换的值。
         * @param order 交换的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return Ops::exch(ptr(), desired, order);
        }

        /**
         * @brief 交换值（对 volatile 类型的支持）
         *
         * @param desired 要交换的值。
         * @param order 交换的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return Ops::exch(ptr(), desired, order);
        }

        /**
         * @brief 弱比较并交换
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param success 成功的内存顺序。
         * @param failure 失败的内存顺序。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        /**
         * @brief 弱比较并交换（对 volatile 类型的支持）
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param success 成功的内存顺序。
         * @param failure 失败的内存顺序。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        /**
         * @brief 弱比较并交换（带默认内存顺序）
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param order 默认的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept -> bool {
            return compare_exchange_weak(expected, desired, order, order);
        }

        /**
         * @brief 弱比较并交换（对 volatile 类型的支持，带默认内存顺序）
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param order 默认的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> bool {
            return compare_exchange_weak(expected, desired, order, order);
        }

        /**
         * @brief 强比较并交换
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param success 成功的内存顺序。
         * @param failure 失败的内存顺序。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        /**
         * @brief 强比较并交换（对 volatile 类型的支持）
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param success 成功的内存顺序。
         * @param failure 失败的内存顺序。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept
            -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        /**
         * @brief 强比较并交换（带默认内存顺序）
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param order 默认的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept -> bool {
            return compare_exchange_strong(expected, desired, order, order);
        }

        /**
         * @brief 强比较并交换（对 volatile 类型的支持，带默认内存顺序）
         *
         * @param expected 期望的值。
         * @param desired 要交换的目标值。
         * @param order 默认的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回是否交换成功。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept
            -> bool {
            return compare_exchange_strong(expected, desired, order, order);
        }

        /**
         * @brief 等待值发生变化
         *
         * @param old 当前值。
         * @param order 等待的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept -> void {
            atomic_wait_impl(ptr(), old, order);
        }

        /**
         * @brief 等待值发生变化（对 volatile 类型的支持）
         *
         * @param old 当前值。
         * @param order 等待的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const volatile noexcept -> void {
            atomic_wait_impl(ptr(), old, order);
        }

        /**
         * @brief 通知一个等待的线程
         */
        rain_fn notify_one() noexcept -> void {
            atomic_notify_one_impl(ptr());
        }

        /**
         * @brief 通知一个等待的线程（对 volatile 类型的支持）
         */
        rain_fn notify_one() volatile noexcept -> void {
            atomic_notify_one_impl(ptr());
        }

        /**
         * @brief 通知所有等待的线程
         */
        rain_fn notify_all() noexcept -> void {
            atomic_notify_all_impl(ptr());
        }

        /**
         * @brief 通知所有等待的线程（对 volatile 类型的支持）
         */
        rain_fn notify_all() volatile noexcept -> void {
            atomic_notify_all_impl(ptr());
        }

    protected:
        volatile Ty *ptr() noexcept {
            return &storage_;
        }

        const volatile Ty *ptr() const noexcept {
            return &storage_;
        }

    private:
        alignas(sizeof(Ty)) volatile Ty storage_;
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    class atomic_integral : public implements::atomic_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_base<Ty, implements::atomic_ops<Ty>>;
        using ops = implements::atomic_ops<Ty>;

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        using base::operator=;

        /**
         * @brief 执行加法操作
         *
         * @param arg 增加的值。
         * @param order 加法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的值。
         */
        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::add(this->ptr(), arg, order);
        }

        /**
         * @brief 执行加法操作（对 volatile 类型的支持）
         *
         * @param arg 增加的值。
         * @param order 加法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的值。
         */
        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::add(this->ptr(), arg, order);
        }

        /**
         * @brief 执行减法操作
         *
         * @param arg 减少的值。
         * @param order 减法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的值。
         */
        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::sub(this->ptr(), arg, order);
        }

        /**
         * @brief 执行减法操作（对 volatile 类型的支持）
         *
         * @param arg 减少的值。
         * @param order 减法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的值。
         */
        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::sub(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位与操作
         *
         * @param arg 与操作数。
         * @param order 按位与的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位与操作前的值。
         */
        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::band(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位与操作（对 volatile 类型的支持）
         *
         * @param arg 与操作数。
         * @param order 按位与的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位与操作前的值。
         */
        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::band(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位或操作
         *
         * @param arg 或操作数。
         * @param order 按位或的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位或操作前的值。
         */
        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::bor(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位或操作（对 volatile 类型的支持）
         *
         * @param arg 或操作数。
         * @param order 按位或的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位或操作前的值。
         */
        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::bor(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位异或操作
         *
         * @param arg 异或操作数。
         * @param order 按位异或的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位异或操作前的值。
         */
        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::bxor(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位异或操作（对 volatile 类型的支持）
         *
         * @param arg 异或操作数。
         * @param order 按位异或的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位异或操作前的值。
         */
        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::bxor(this->ptr(), arg, order);
        }

        /**
         * @brief 执行最大值比较并交换操作
         *
         * @param arg 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        /**
         * @brief 执行最大值比较并交换操作（对 volatile 类型的支持）
         *
         * @param arg 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        /**
         * @brief 执行最小值比较并交换操作
         *
         * @param arg 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        /**
         * @brief 执行最小值比较并交换操作（对 volatile 类型的支持）
         *
         * @param arg 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        /**
         * @brief 后置自增操作
         *
         * @return 返回自增前的值。
         */
        rain_fn operator++(int) noexcept -> Ty {
            return fetch_add(Ty{1});
        }

        /**
         * @brief 后置自增操作（对 volatile 类型的支持）
         *
         * @return 返回自增前的值。
         */
        rain_fn operator++(int) volatile noexcept -> Ty {
            return fetch_add(Ty{1});
        }

        /**
         * @brief 后置自减操作
         *
         * @return 返回自减前的值。
         */
        rain_fn operator--(int) noexcept -> Ty {
            return fetch_sub(Ty{1});
        }

        /**
         * @brief 后置自减操作（对 volatile 类型的支持）
         *
         * @return 返回自减前的值。
         */
        rain_fn operator--(int) volatile noexcept -> Ty {
            return fetch_sub(Ty{1});
        }

        /**
         * @brief 前置自增操作
         *
         * @return 返回自增后的值。
         */
        rain_fn operator++() noexcept -> Ty {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        /**
         * @brief 前置自增操作（对 volatile 类型的支持）
         *
         * @return 返回自增后的值。
         */
        rain_fn operator++() volatile noexcept -> Ty {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        /**
         * @brief 前置自减操作
         *
         * @return 返回自减后的值。
         */
        rain_fn operator--() noexcept -> Ty {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        /**
         * @brief 前置自减操作（对 volatile 类型的支持）
         *
         * @return 返回自减后的值。
         */
        rain_fn operator--() volatile noexcept -> Ty {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        /**
         * @brief 执行加法并赋值操作
         *
         * @param arg 增加的值。
         * @return 返回加法操作后的值。
         */
        rain_fn operator+=(Ty arg) noexcept -> Ty {
            return fetch_add(arg) + arg;
        }

        /**
         * @brief 执行加法并赋值操作（对 volatile 类型的支持）
         *
         * @param arg 增加的值。
         * @return 返回加法操作后的值。
         */
        rain_fn operator+=(Ty arg) volatile noexcept -> Ty {
            return fetch_add(arg) + arg;
        }

        /**
         * @brief 执行减法并赋值操作
         *
         * @param arg 减少的值。
         * @return 返回减法操作后的值。
         */
        rain_fn operator-=(Ty arg) noexcept -> Ty {
            return fetch_sub(arg) - arg;
        }

        /**
         * @brief 执行减法并赋值操作（对 volatile 类型的支持）
         *
         * @param arg 减少的值。
         * @return 返回减法操作后的值。
         */
        rain_fn operator-=(Ty arg) volatile noexcept -> Ty {
            return fetch_sub(arg) - arg;
        }

        /**
         * @brief 执行按位与并赋值操作
         *
         * @param arg 与操作数。
         * @return 返回按位与操作后的值。
         */
        rain_fn operator&=(Ty arg) noexcept -> Ty {
            return fetch_and(arg) & arg;
        }

        /**
         * @brief 执行按位与并赋值操作（对 volatile 类型的支持）
         *
         * @param arg 与操作数。
         * @return 返回按位与操作后的值。
         */
        rain_fn operator&=(Ty arg) volatile noexcept -> Ty {
            return fetch_and(arg) & arg;
        }

        /**
         * @brief 执行按位或并赋值操作
         *
         * @param arg 或操作数。
         * @return 返回按位或操作后的值。
         */
        rain_fn operator|=(Ty arg) noexcept -> Ty {
            return fetch_or(arg) | arg;
        }

        /**
         * @brief 执行按位或并赋值操作（对 volatile 类型的支持）
         *
         * @param arg 或操作数。
         * @return 返回按位或操作后的值。
         */
        rain_fn operator|=(Ty arg) volatile noexcept -> Ty {
            return fetch_or(arg) | arg;
        }

        /**
         * @brief 执行按位异或并赋值操作
         *
         * @param arg 异或操作数。
         * @return 返回按位异或操作后的值。
         */
        rain_fn operator^=(Ty arg) noexcept -> Ty {
            return fetch_xor(arg) ^ arg;
        }

        /**
         * @brief 执行按位异或并赋值操作（对 volatile 类型的支持）
         *
         * @param arg 异或操作数。
         * @return 返回按位异或操作后的值。
         */
        rain_fn operator^=(Ty arg) volatile noexcept -> Ty {
            return fetch_xor(arg) ^ arg;
        }
    };
}


namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    class atomic_floating : public implements::atomic_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_base<Ty, implements::atomic_ops<Ty>>;
        using ops = implements::atomic_ops<Ty>;

        static_assert(!std::is_const_v<Ty>, "atomic_floating<Ty>: Ty must not be const");
        static_assert(!std::is_volatile_v<Ty>, "atomic_floating<Ty>: Ty must not be volatile");

        using value_type = Ty;
        using difference_type = Ty;

        using base::operator=;

        using base::base;

        /**
         * @brief 执行加法操作
         *
         * @param operand 增加的值。
         * @param order 加法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的值。
         */
        rain_fn fetch_add(Ty operand, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        /**
         * @brief 执行加法操作（对 volatile 类型的支持）
         *
         * @param operand 增加的值。
         * @param order 加法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的值。
         */
        rain_fn fetch_add(Ty operand, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_add(operand, order);
        }

        /**
         * @brief 执行减法操作
         *
         * @param operand 减少的值。
         * @param order 减法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的值。
         */
        rain_fn fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        /**
         * @brief 执行减法操作（对 volatile 类型的支持）
         *
         * @param operand 减少的值。
         * @param order 减法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的值。
         */
        rain_fn fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand, order);
        }

        /**
         * @brief 执行加法并赋值操作
         *
         * @param operand 增加的值。
         * @return 返回加法操作后的值。
         */
        rain_fn operator+=(Ty operand) noexcept -> Ty {
            return fetch_add(operand) + operand;
        }

        /**
         * @brief 执行加法并赋值操作（对 volatile 类型的支持）
         *
         * @param operand 增加的值。
         * @return 返回加法操作后的值。
         */
        rain_fn operator+=(Ty operand) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_add(operand) + operand;
        }

        /**
         * @brief 执行减法并赋值操作
         *
         * @param operand 减少的值。
         * @return 返回减法操作后的值。
         */
        rain_fn operator-=(Ty operand) noexcept -> Ty {
            return fetch_sub(operand) - operand;
        }

        /**
         * @brief 执行减法并赋值操作（对 volatile 类型的支持）
         *
         * @param operand 减少的值。
         * @return 返回减法操作后的值。
         */
        rain_fn operator-=(Ty operand) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand) - operand;
        }
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    class atomic_pointer : public implements::atomic_base<Ty *, implements::atomic_ops<Ty *>> {
    public:
        using base = implements::atomic_base<Ty *, implements::atomic_ops<Ty *>>;
        using ops = implements::atomic_ops<Ty *>;

        static_assert(!std::is_function_v<Ty>, "atomic_pointer<Ty>: Ty must not be a function type");

        using value_type = Ty *;
        using difference_type = std::ptrdiff_t;

        using base::operator=;

        using base::base;

        /**
         * @brief 执行指针加法操作
         *
         * @param n 要增加的指针差值。
         * @param order 加法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的指针值。
         */
        rain_fn fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return ops::ptr_add(this->ptr(), n, order);
        }

        /**
         * @brief 执行指针加法操作（对 volatile 类型的支持）
         *
         * @param n 要增加的指针差值。
         * @param order 加法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的指针值。
         */
        rain_fn fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_add(n, order);
        }

        /**
         * @brief 执行指针减法操作
         *
         * @param n 要减少的指针差值。
         * @param order 减法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的指针值。
         */
        rain_fn fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return ops::ptr_sub(this->ptr(), n, order);
        }

        /**
         * @brief 执行指针减法操作（对 volatile 类型的支持）
         *
         * @param n 要减少的指针差值。
         * @param order 减法的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的指针值。
         */
        rain_fn fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_sub(n, order);
        }

        /**
         * @brief 执行指针最大值比较并交换操作
         *
         * @param val 比较的指针值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的指针值。
         */
        rain_fn fetch_max(Ty *val, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return implements::atomic_ptr_fetch_max(this->ptr(), val, order);
        }

        /**
         * @brief 执行指针最大值比较并交换操作（对 volatile 类型的支持）
         *
         * @param val 比较的指针值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的指针值。
         */
        rain_fn fetch_max(Ty *val, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_max(val, order);
        }

        /**
         * @brief 执行指针最小值比较并交换操作
         *
         * @param val 比较的指针值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的指针值。
         */
        rain_fn fetch_min(Ty *val, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return implements::atomic_ptr_fetch_min(this->ptr(), val, order);
        }

        /**
         * @brief 执行指针最小值比较并交换操作（对 volatile 类型的支持）
         *
         * @param val 比较的指针值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的指针值。
         */
        rain_fn fetch_min(Ty *val, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_min(val, order);
        }

        /**
         * @brief 后置自增操作
         *
         * @return 返回自增前的指针值。
         */
        rain_fn operator++(int) noexcept -> Ty * {
            return fetch_add(1);
        }

        /**
         * @brief 后置自增操作（对 volatile 类型的支持）
         *
         * @return 返回自增前的指针值。
         */
        rain_fn operator++(int) volatile noexcept -> Ty * {
            return fetch_add(1);
        }

        /**
         * @brief 后置自减操作
         *
         * @return 返回自减前的指针值。
         */
        rain_fn operator--(int) noexcept -> Ty * {
            return fetch_sub(1);
        }

        /**
         * @brief 后置自减操作（对 volatile 类型的支持）
         *
         * @return 返回自减前的指针值。
         */
        rain_fn operator--(int) volatile noexcept -> Ty * {
            return fetch_sub(1);
        }

        /**
         * @brief 前置自增操作
         *
         * @return 返回自增后的指针值。
         */
        rain_fn operator++() noexcept -> Ty * {
            return fetch_add(1) + 1;
        }

        /**
         * @brief 前置自增操作（对 volatile 类型的支持）
         *
         * @return 返回自增后的指针值。
         */
        rain_fn operator++() volatile noexcept -> Ty * {
            return fetch_add(1) + 1;
        }

        /**
         * @brief 前置自减操作
         *
         * @return 返回自减后的指针值。
         */
        rain_fn operator--() noexcept -> Ty * {
            return fetch_sub(1) - 1;
        }

        /**
         * @brief 前置自减操作（对 volatile 类型的支持）
         *
         * @return 返回自减后的指针值。
         */
        rain_fn operator--() volatile noexcept -> Ty * {
            return fetch_sub(1) - 1;
        }

        /**
         * @brief 执行指针加法并赋值操作
         *
         * @param n 增加的指针差值。
         * @return 返回加法操作后的指针值。
         */
        rain_fn operator+=(std::ptrdiff_t n) noexcept -> Ty * {
            return fetch_add(n) + n;
        }

        /**
         * @brief 执行指针加法并赋值操作（对 volatile 类型的支持）
         *
         * @param n 增加的指针差值。
         * @return 返回加法操作后的指针值。
         */
        rain_fn operator+=(std::ptrdiff_t n) volatile noexcept -> Ty * {
            return fetch_add(n) + n;
        }

        /**
         * @brief 执行指针减法并赋值操作
         *
         * @param n 减少的指针差值。
         * @return 返回减法操作后的指针值。
         */
        rain_fn operator-=(std::ptrdiff_t n) noexcept -> Ty * {
            return fetch_sub(n) - n;
        }

        /**
         * @brief 执行指针减法并赋值操作（对 volatile 类型的支持）
         *
         * @param n 减少的指针差值。
         * @return 返回减法操作后的指针值。
         */
        rain_fn operator-=(std::ptrdiff_t n) volatile noexcept -> Ty * {
            return fetch_sub(n) - n;
        }
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename TVal, typename Ty>
    struct atomic_base_selector {
    private:
        using type = std::conditional_t<
            // 1. bool 独立路由：只有基础操作，无算术/位运算
            //    直接使用 atomic_base，不进入 atomic_integral
            std::is_same_v<Ty, bool>, atomic_base<bool, atomic_ops<bool>>,
            std::conditional_t<
                // 2. 整数（已排除 bool）
                std::is_integral_v<Ty>, atomic_integral<Ty>,
                std::conditional_t<
                    // 3. 浮点
                    std::is_floating_point_v<Ty>, atomic_floating<Ty>,
                    std::conditional_t<
                        // 4. 指针
                        std::is_pointer_v<Ty>, atomic_pointer<std::remove_pointer_t<Ty>>,
                        // 5. 通用回退（用户自定义 trivially copyable 类型）
                        atomic_base<Ty, atomic_ops<Ty>>>>>>;

    public:
        static_assert(std::is_trivially_copyable_v<Ty>, "atomic<Ty>: Ty must be TriviallyCopyable (ISO C++ [atomics.types.generic])");

        using result = type;
    };

    template <typename TVal, typename Ty = TVal>
    using select_atomic_base_t = typename implements::atomic_base_selector<TVal, Ty>::result;
}

#endif
