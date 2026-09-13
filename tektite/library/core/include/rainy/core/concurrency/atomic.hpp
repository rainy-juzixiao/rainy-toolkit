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
#ifndef RAINY_CORE_CONCURRENCY_ATOMIC_HPP
#define RAINY_CORE_CONCURRENCY_ATOMIC_HPP
#include <rainy/core/annotations/lifetime_annotation.hpp>
#include <rainy/core/concurrency/atomicinfra/atomic_base.hpp>
#include <rainy/core/concurrency/atomicinfra/atomic_flag.hpp>
#include <rainy/core/concurrency/atomicinfra/atomic_ref.hpp>

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Atomic type template class.
     *
     *  Describes an object that performs atomic operations on the stored value of type Ty.
     *
     * @tparam Ty The stored type.
     *
     * \lang simp-chinese
     * @brief 原子类型模板类
     *
     *  描述对 Ty 类型的存储值执行 atomic 操作的对象。
     *
     * @tparam Ty 存储类型。
     */
    template <typename Ty>
    class atomic : public implements::select_atomic_base_t<Ty> {
    public:
        using implements::select_atomic_base_t<Ty>::select_atomic_base_t;
        using implements::select_atomic_base_t<Ty>::operator=;
    };
}

#else // fake class declare

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Atomic type template class.
     *
     *  Describes an object that performs atomic operations on the stored value of type Ty.
     *  The primary template applies to any trivially copyable type Ty. Members that are
     *  only available for certain categories of types are annotated per-member with an
     *  @note tag describing the applicable types.
     *
     * @tparam Ty The stored type. Must be TriviallyCopyable
     *            (ISO C++ [atomics.types.generic]).
     *
     * \lang simp-chinese
     * @brief 原子类型模板类
     *
     *  描述对 Ty 类型的存储值执行 atomic 操作的对象。
     *  主模板适用于任何可平凡复制类型 Ty。仅对特定类别类型可用的成员，
     *  会通过每个成员的 @note 标注说明其适用类型。
     *
     * @tparam Ty 存储类型。必须为可平凡复制类型
     *            （ISO C++ [atomics.types.generic]）。
     */
    template <typename Ty>
    class atomic {
    public:
        using value_type = Ty;
        using difference_type = Ty;

        static constexpr bool is_always_lock_free = true;

        /**
         * \lang english
         * @brief Checks whether the atomic object's operations are lock-free.
         *
         * @return true if the operations are lock-free.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 检查该原子对象的操作是否为无锁的。
         *
         * @return 若为无锁操作则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn is_lock_free() const noexcept -> bool;

        /**
         * \lang english
         * @brief Checks whether the atomic object's operations are lock-free
         *        (volatile overload).
         *
         * @return true if the operations are lock-free.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 检查该原子对象的操作是否为无锁的（对 volatile 类型的支持）。
         *
         * @return 若为无锁操作则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn is_lock_free() const volatile noexcept -> bool;

        /**
         * \lang english
         * @brief Default constructor. The contained value is value-initialized.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。所含值被值初始化。
         *
         * @note 适用类型：所有类型。
         */
        constexpr atomic() noexcept;

        /**
         * \lang english
         * @brief Constructs the atomic object with the given value.
         *
         * @param desired The initial value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用给定值构造原子对象。
         *
         * @param desired 初始值。
         *
         * @note 适用类型：所有类型。
         */
        constexpr atomic(Ty desired) noexcept;

        atomic(const atomic &) = delete;
        atomic &operator=(const atomic &) = delete;
        atomic &operator=(const atomic &) volatile = delete;

        /**
         * \lang english
         * @brief Atomically stores a value.
         *
         * @param desired The value to store.
         * @param order The memory order (defaults to memory_order::seq_cst).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地存储一个值。
         *
         * @param desired 要存储的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) noexcept -> void;

        /**
         * \lang english
         * @brief Atomically stores a value (volatile overload).
         *
         * @param desired The value to store.
         * @param order The memory order (defaults to memory_order::seq_cst).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地存储一个值（对 volatile 类型的支持）。
         *
         * @param desired 要存储的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> void;

        /**
         * \lang english
         * @brief Atomically loads the stored value.
         *
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The loaded value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地加载所存储的值。
         *
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 加载得到的值。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn load(memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically loads the stored value (volatile overload).
         *
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The loaded value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地加载所存储的值（对 volatile 类型的支持）。
         *
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 加载得到的值。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn load(memory_order order = memory_order::seq_cst) const volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Conversion operator to the underlying value type.
         *
         * @return The current value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 到所含值类型的类型转换操作符。
         *
         * @return 当前值。
         *
         * @note 适用类型：所有类型。
         */
        operator Ty() const noexcept;

        /**
         * \lang english
         * @brief Conversion operator to the underlying value type (volatile overload).
         *
         * @return The current value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 到所含值类型的类型转换操作符（对 volatile 类型的支持）。
         *
         * @return 当前值。
         *
         * @note 适用类型：所有类型。
         */
        operator Ty() const volatile noexcept;

        /**
         * \lang english
         * @brief Atomically replaces the stored value and returns the previous value.
         *
         * @param desired The value to store.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the exchange.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地替换所存储的值并返回先前的值。
         *
         * @param desired 要存储的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 交换前的值。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value and returns the previous value
         *        (volatile overload).
         *
         * @param desired The value to store.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the exchange.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地替换所存储的值并返回先前的值（对 volatile 类型的支持）。
         *
         * @param desired 要存储的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 交换前的值。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Weak compare-and-exchange. May fail spuriously.
         *
         * @param expected Reference to the expected value; updated with the actual
         *                 value on failure.
         * @param desired The desired value to store on success.
         * @param success The memory order on success.
         * @param failure The memory order on failure.
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 弱比较并交换。可能虚假失败。
         *
         * @param expected 期望值的引用；失败时被更新为实际值。
         * @param desired 成功时要存储的目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept -> bool;

        /**
         * \lang english
         * @brief Weak compare-and-exchange (volatile overload).
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param success The memory order on success.
         * @param failure The memory order on failure.
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 弱比较并交换（对 volatile 类型的支持）。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept -> bool;

        /**
         * \lang english
         * @brief Weak compare-and-exchange with a single memory order.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用单一内存顺序的弱比较并交换。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept -> bool;

        /**
         * \lang english
         * @brief Weak compare-and-exchange with a single memory order (volatile overload).
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用单一内存顺序的弱比较并交换（对 volatile 类型的支持）。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> bool;

        /**
         * \lang english
         * @brief Strong compare-and-exchange. No spurious failure.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param success The memory order on success.
         * @param failure The memory order on failure.
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 强比较并交换。不会虚假失败。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept -> bool;

        /**
         * \lang english
         * @brief Strong compare-and-exchange (volatile overload).
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param success The memory order on success.
         * @param failure The memory order on failure.
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 强比较并交换（对 volatile 类型的支持）。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept -> bool;

        /**
         * \lang english
         * @brief Strong compare-and-exchange with a single memory order.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用单一内存顺序的强比较并交换。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept -> bool;

        /**
         * \lang english
         * @brief Strong compare-and-exchange with a single memory order (volatile overload).
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用单一内存顺序的强比较并交换（对 volatile 类型的支持）。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> bool;

        /**
         * \lang english
         * @brief Atomically adds a value and returns the previous value.
         *
         * @param arg The value to add.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the addition.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地加上一个值并返回先前的值。
         *
         * @param arg 要增加的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 加法操作前的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically adds a value and returns the previous value (volatile overload).
         *
         * @param arg The value to add.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the addition.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地加上一个值并返回先前的值（对 volatile 类型的支持）。
         *
         * @param arg 要增加的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 加法操作前的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically subtracts a value and returns the previous value.
         *
         * @param arg The value to subtract.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the subtraction.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地减去一个值并返回先前的值。
         *
         * @param arg 要减少的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 减法操作前的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically subtracts a value and returns the previous value
         *        (volatile overload).
         *
         * @param arg The value to subtract.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the subtraction.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地减去一个值并返回先前的值（对 volatile 类型的支持）。
         *
         * @param arg 要减少的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 减法操作前的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the result of a bitwise AND,
         *        and returns the previous value.
         *
         * @param arg The AND operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为按位与的结果，并返回先前的值。
         *
         * @param arg 与操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the result of a bitwise AND,
         *        and returns the previous value (volatile overload).
         *
         * @param arg The AND operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为按位与的结果，并返回先前的值
         *        （对 volatile 类型的支持）。
         *
         * @param arg 与操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the result of a bitwise OR,
         *        and returns the previous value.
         *
         * @param arg The OR operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为按位或的结果，并返回先前的值。
         *
         * @param arg 或操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the result of a bitwise OR,
         *        and returns the previous value (volatile overload).
         *
         * @param arg The OR operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为按位或的结果，并返回先前的值
         *        （对 volatile 类型的支持）。
         *
         * @param arg 或操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the result of a bitwise XOR,
         *        and returns the previous value.
         *
         * @param arg The XOR operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为按位异或的结果，并返回先前的值。
         *
         * @param arg 异或操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the result of a bitwise XOR,
         *        and returns the previous value (volatile overload).
         *
         * @param arg The XOR operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为按位异或的结果，并返回先前的值
         *        （对 volatile 类型的支持）。
         *
         * @param arg 异或操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the maximum of the stored
         *        value and arg, and returns the previous value.
         *
         * @param arg The value to compare.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为它与 arg 的最大值，并返回先前的值。
         *
         * @param arg 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the maximum of the stored
         *        value and arg, and returns the previous value (volatile overload).
         *
         * @param arg The value to compare.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为它与 arg 的最大值，并返回先前的值
         *        （对 volatile 类型的支持）。
         *
         * @param arg 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the minimum of the stored
         *        value and arg, and returns the previous value.
         *
         * @param arg The value to compare.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为它与 arg 的最小值，并返回先前的值。
         *
         * @param arg 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the stored value with the minimum of the stored
         *        value and arg, and returns the previous value (volatile overload).
         *
         * @param arg The value to compare.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地将所存储的值替换为它与 arg 的最小值，并返回先前的值
         *        （对 volatile 类型的支持）。
         *
         * @param arg 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Post-increment operation.
         *
         * @return The value before the increment.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 后置自增操作。
         *
         * @return 自增前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator++(int) noexcept -> Ty;

        /**
         * \lang english
         * @brief Post-increment operation (volatile overload).
         *
         * @return The value before the increment.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 后置自增操作（对 volatile 类型的支持）。
         *
         * @return 自增前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator++(int) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Post-decrement operation.
         *
         * @return The value before the decrement.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 后置自减操作。
         *
         * @return 自减前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator--(int) noexcept -> Ty;

        /**
         * \lang english
         * @brief Post-decrement operation (volatile overload).
         *
         * @return The value before the decrement.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 后置自减操作（对 volatile 类型的支持）。
         *
         * @return 自减前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator--(int) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Pre-increment operation.
         *
         * @return The value after the increment.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 前置自增操作。
         *
         * @return 自增后的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator++() noexcept -> Ty;

        /**
         * \lang english
         * @brief Pre-increment operation (volatile overload).
         *
         * @return The value after the increment.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 前置自增操作（对 volatile 类型的支持）。
         *
         * @return 自增后的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator++() volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Pre-decrement operation.
         *
         * @return The value after the decrement.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 前置自减操作。
         *
         * @return 自减后的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator--() noexcept -> Ty;

        /**
         * \lang english
         * @brief Pre-decrement operation (volatile overload).
         *
         * @return The value after the decrement.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 前置自减操作（对 volatile 类型的支持）。
         *
         * @return 自减后的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator--() volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs addition and assignment.
         *
         * @param arg The value to add.
         * @return The value after the addition.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 执行加法并赋值操作。
         *
         * @param arg 要增加的值。
         * @return 加法操作后的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn operator+=(Ty arg) noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs addition and assignment (volatile overload).
         *
         * @param arg The value to add.
         * @return The value after the addition.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 执行加法并赋值操作（对 volatile 类型的支持）。
         *
         * @param arg 要增加的值。
         * @return 加法操作后的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn operator+=(Ty arg) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs subtraction and assignment.
         *
         * @param arg The value to subtract.
         * @return The value after the subtraction.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 执行减法并赋值操作。
         *
         * @param arg 要减少的值。
         * @return 减法操作后的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn operator-=(Ty arg) noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs subtraction and assignment (volatile overload).
         *
         * @param arg The value to subtract.
         * @return The value after the subtraction.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 执行减法并赋值操作（对 volatile 类型的支持）。
         *
         * @param arg 要减少的值。
         * @return 减法操作后的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn operator-=(Ty arg) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise AND and assignment.
         *
         * @param arg The AND operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位与并赋值操作。
         *
         * @param arg 与操作数。
         * @return 按位与操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator&=(Ty arg) noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise AND and assignment (volatile overload).
         *
         * @param arg The AND operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位与并赋值操作（对 volatile 类型的支持）。
         *
         * @param arg 与操作数。
         * @return 按位与操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator&=(Ty arg) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise OR and assignment.
         *
         * @param arg The OR operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位或并赋值操作。
         *
         * @param arg 或操作数。
         * @return 按位或操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator|=(Ty arg) noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise OR and assignment (volatile overload).
         *
         * @param arg The OR operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位或并赋值操作（对 volatile 类型的支持）。
         *
         * @param arg 或操作数。
         * @return 按位或操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator|=(Ty arg) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise XOR and assignment.
         *
         * @param arg The XOR operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位异或并赋值操作。
         *
         * @param arg 异或操作数。
         * @return 按位异或操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator^=(Ty arg) noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise XOR and assignment (volatile overload).
         *
         * @param arg The XOR operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位异或并赋值操作（对 volatile 类型的支持）。
         *
         * @param arg 异或操作数。
         * @return 按位异或操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator^=(Ty arg) volatile noexcept -> Ty;

        /**
         * \lang english
         * @brief Blocks until notified and the stored value changes.
         *
         * @param old The value to compare against.
         * @param order The memory order (defaults to memory_order::seq_cst).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 阻塞直到被通知且所存储的值发生变化。
         *
         * @param old 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept -> void;

        /**
         * \lang english
         * @brief Blocks until notified and the stored value changes (volatile overload).
         *
         * @param old The value to compare against.
         * @param order The memory order (defaults to memory_order::seq_cst).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 阻塞直到被通知且所存储的值发生变化（对 volatile 类型的支持）。
         *
         * @param old 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const volatile noexcept -> void;

        /**
         * \lang english
         * @brief Notifies at least one thread blocked in wait().
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 通知至少一个阻塞在 wait() 中的线程。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn notify_one() noexcept -> void;

        /**
         * \lang english
         * @brief Notifies at least one thread blocked in wait() (volatile overload).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 通知至少一个阻塞在 wait() 中的线程（对 volatile 类型的支持）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn notify_one() volatile noexcept -> void;

        /**
         * \lang english
         * @brief Notifies all threads blocked in wait().
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 通知所有阻塞在 wait() 中的线程。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn notify_all() noexcept -> void;

        /**
         * \lang english
         * @brief Notifies all threads blocked in wait() (volatile overload).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 通知所有阻塞在 wait() 中的线程（对 volatile 类型的支持）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn notify_all() volatile noexcept -> void;
    };
}

#endif

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Atomic type alias for `bool`.
     *
     * \lang simp-chinese
     * @brief `bool` 的原子类型别名。
     */
    using atomic_bool = atomic<bool>;
    /**
     * \lang english
     * @brief Atomic type alias for `char`.
     *
     * \lang simp-chinese
     * @brief `char` 的原子类型别名。
     */
    using atomic_char = atomic<char>;
    /**
     * \lang english
     * @brief Atomic type alias for `signed char`.
     *
     * \lang simp-chinese
     * @brief `signed char` 的原子类型别名。
     */
    using atomic_schar = atomic<signed char>;
    /**
     * \lang english
     * @brief Atomic type alias for `unsigned char`.
     *
     * \lang simp-chinese
     * @brief `unsigned char` 的原子类型别名。
     */
    using atomic_uchar = atomic<unsigned char>;
    /**
     * \lang english
     * @brief Atomic type alias for `short`.
     *
     * \lang simp-chinese
     * @brief `short` 的原子类型别名。
     */
    using atomic_short = atomic<short>;
    /**
     * \lang english
     * @brief Atomic type alias for `unsigned short`.
     *
     * \lang simp-chinese
     * @brief `unsigned short` 的原子类型别名。
     */
    using atomic_ushort = atomic<unsigned short>;
    /**
     * \lang english
     * @brief Atomic type alias for `int`.
     *
     * \lang simp-chinese
     * @brief `int` 的原子类型别名。
     */
    using atomic_int = atomic<int>;
    /**
     * \lang english
     * @brief Atomic type alias for `unsigned int`.
     *
     * \lang simp-chinese
     * @brief `unsigned int` 的原子类型别名。
     */
    using atomic_uint = atomic<unsigned int>;
    /**
     * \lang english
     * @brief Atomic type alias for `long`.
     *
     * \lang simp-chinese
     * @brief `long` 的原子类型别名。
     */
    using atomic_long = atomic<long>;
    /**
     * \lang english
     * @brief Atomic type alias for `unsigned long`.
     *
     * \lang simp-chinese
     * @brief `unsigned long` 的原子类型别名。
     */
    using atomic_ulong = atomic<unsigned long>;
    /**
     * \lang english
     * @brief Atomic type alias for `long long`.
     *
     * \lang simp-chinese
     * @brief `long long` 的原子类型别名。
     */
    using atomic_llong = atomic<long long>;
    /**
     * \lang english
     * @brief Atomic type alias for `unsigned long long`.
     *
     * \lang simp-chinese
     * @brief `unsigned long long` 的原子类型别名。
     */
    using atomic_ullong = atomic<unsigned long long>;
#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Atomic type alias for `char8_t`.
     *
     * \lang simp-chinese
     * @brief `char8_t` 的原子类型别名。
     */
    using atomic_char8_t = atomic<char8_t>;
#endif
    /**
     * \lang english
     * @brief Atomic type alias for `char16_t`.
     *
     * \lang simp-chinese
     * @brief `char16_t` 的原子类型别名。
     */
    using atomic_char16_t = atomic<char16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `char32_t`.
     *
     * \lang simp-chinese
     * @brief `char32_t` 的原子类型别名。
     */
    using atomic_char32_t = atomic<char32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `wchar_t`.
     *
     * \lang simp-chinese
     * @brief `wchar_t` 的原子类型别名。
     */
    using atomic_wchar_t = atomic<wchar_t>;

    /**
     * \lang english
     * @brief Atomic type alias for `std::int8_t`.
     *
     * \lang simp-chinese
     * @brief `std::int8_t` 的原子类型别名。
     */
    using atomic_int8_t = atomic<std::int8_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint8_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint8_t` 的原子类型别名。
     */
    using atomic_uint8_t = atomic<std::uint8_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int16_t`.
     *
     * \lang simp-chinese
     * @brief `std::int16_t` 的原子类型别名。
     */
    using atomic_int16_t = atomic<std::int16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint16_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint16_t` 的原子类型别名。
     */
    using atomic_uint16_t = atomic<std::uint16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int32_t`.
     *
     * \lang simp-chinese
     * @brief `std::int32_t` 的原子类型别名。
     */
    using atomic_int32_t = atomic<std::int32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint32_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint32_t` 的原子类型别名。
     */
    using atomic_uint32_t = atomic<std::uint32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int64_t`.
     *
     * \lang simp-chinese
     * @brief `std::int64_t` 的原子类型别名。
     */
    using atomic_int64_t = atomic<std::int64_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint64_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint64_t` 的原子类型别名。
     */
    using atomic_uint64_t = atomic<std::uint64_t>;

    /**
     * \lang english
     * @brief Atomic type alias for `std::int_least8_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_least8_t` 的原子类型别名。
     */
    using atomic_int_least8_t = atomic<std::int_least8_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_least8_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_least8_t` 的原子类型别名。
     */
    using atomic_uint_least8_t = atomic<std::uint_least8_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int_least16_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_least16_t` 的原子类型别名。
     */
    using atomic_int_least16_t = atomic<std::int_least16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_least16_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_least16_t` 的原子类型别名。
     */
    using atomic_uint_least16_t = atomic<std::uint_least16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int_least32_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_least32_t` 的原子类型别名。
     */
    using atomic_int_least32_t = atomic<std::int_least32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_least32_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_least32_t` 的原子类型别名。
     */
    using atomic_uint_least32_t = atomic<std::uint_least32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int_least64_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_least64_t` 的原子类型别名。
     */
    using atomic_int_least64_t = atomic<std::int_least64_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_least64_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_least64_t` 的原子类型别名。
     */
    using atomic_uint_least64_t = atomic<std::uint_least64_t>;

    /**
     * \lang english
     * @brief Atomic type alias for `std::int_fast8_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_fast8_t` 的原子类型别名。
     */
    using atomic_int_fast8_t = atomic<std::int_fast8_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_fast8_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_fast8_t` 的原子类型别名。
     */
    using atomic_uint_fast8_t = atomic<std::uint_fast8_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int_fast16_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_fast16_t` 的原子类型别名。
     */
    using atomic_int_fast16_t = atomic<std::int_fast16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_fast16_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_fast16_t` 的原子类型别名。
     */
    using atomic_uint_fast16_t = atomic<std::uint_fast16_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int_fast32_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_fast32_t` 的原子类型别名。
     */
    using atomic_int_fast32_t = atomic<std::int_fast32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_fast32_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_fast32_t` 的原子类型别名。
     */
    using atomic_uint_fast32_t = atomic<std::uint_fast32_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::int_fast64_t`.
     *
     * \lang simp-chinese
     * @brief `std::int_fast64_t` 的原子类型别名。
     */
    using atomic_int_fast64_t = atomic<std::int_fast64_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uint_fast64_t`.
     *
     * \lang simp-chinese
     * @brief `std::uint_fast64_t` 的原子类型别名。
     */
    using atomic_uint_fast64_t = atomic<std::uint_fast64_t>;

    /**
     * \lang english
     * @brief Atomic type alias for `std::intptr_t`.
     *
     * \lang simp-chinese
     * @brief `std::intptr_t` 的原子类型别名。
     */
    using atomic_intptr_t = atomic<std::intptr_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uintptr_t`.
     *
     * \lang simp-chinese
     * @brief `std::uintptr_t` 的原子类型别名。
     */
    using atomic_uintptr_t = atomic<std::uintptr_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::size_t`.
     *
     * \lang simp-chinese
     * @brief `std::size_t` 的原子类型别名。
     */
    using atomic_size_t = atomic<std::size_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::ptrdiff_t`.
     *
     * \lang simp-chinese
     * @brief `std::ptrdiff_t` 的原子类型别名。
     */
    using atomic_ptrdiff_t = atomic<std::ptrdiff_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::intmax_t`.
     *
     * \lang simp-chinese
     * @brief `std::intmax_t` 的原子类型别名。
     */
    using atomic_intmax_t = atomic<std::intmax_t>;
    /**
     * \lang english
     * @brief Atomic type alias for `std::uintmax_t`.
     *
     * \lang simp-chinese
     * @brief `std::uintmax_t` 的原子类型别名。
     */
    using atomic_uintmax_t = atomic<std::uintmax_t>;
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Atomic type reference template class.
     *
     *  Describes a reference that performs atomic operations on the stored value of type Ty.
     *
     * @tparam Ty The referenced type.
     *
     * \lang simp-chinese
     * @brief 原子类型引用模板类
     *
     *  描述对 Ty 类型的存储值执行 atomic 操作的引用。
     *
     * @tparam Ty 存储类型。
     */
    template <typename Ty>
    class atomic_ref : public implements::select_atomic_ref_base_t<Ty> {
    public:
        using implements::select_atomic_ref_base_t<Ty>::select_atomic_ref_base_t;
        using implements::select_atomic_ref_base_t<Ty>::operator=;
    };
}

#else

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Atomic type reference template class.
     *
     *  Describes a reference that performs atomic operations on the stored value of type Ty.
     *  The referenced object must outlive the atomic_ref, and no other atomic_ref may
     *  reference the same object concurrently. The referenced object must satisfy the
     *  alignment requirement reported by required_alignment.
     *
     * @tparam Ty The referenced type. Must be TriviallyCopyable
     *            (ISO C++ [atomics.ref.generic]).
     *
     * \lang simp-chinese
     * @brief 原子类型引用模板类
     *
     *  描述对 Ty 类型的存储值执行 atomic 操作的引用。
     *  被引用对象的生存期必须长于 atomic_ref，且同一对象上不得同时存在其它
     *  atomic_ref。被引用对象必须满足 required_alignment 所报告的 Alignment 要求。
     *
     * @tparam Ty 被引用类型。必须为可平凡复制类型
     *            （ISO C++ [atomics.ref.generic]）。
     */
    template <typename Ty>
    class atomic_ref {
    public:
        /**
         * \lang english
         * @brief The type of the value referenced by this atomic_ref.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 此 atomic_ref 所引用值的类型。
         *
         * @note 适用类型：所有类型。
         */
        using value_type = Ty;

        /**
         * \lang english
         * @brief The alignment required for the referenced object.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 被引用对象所需的 Alignment。
         *
         * @note 适用类型：所有类型。
         */
        static constexpr std::size_t required_alignment = /* implementation-defined */;

        /**
         * \lang english
         * @brief Indicates whether the atomic operations are always lock-free.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 指示原子操作是否始终是无锁的。
         *
         * @note 适用类型：所有类型。
         */
        static constexpr bool is_always_lock_free = true;

        /**
         * \lang english
         * @brief Checks whether the atomic operations are lock-free.
         *
         * @return true if the operations are lock-free.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 检查原子操作是否为无锁的。
         *
         * @return 若为无锁操作则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn is_lock_free() const noexcept -> bool;

        /**
         * \lang english
         * @brief Constructs an atomic_ref that references the given object.
         *
         * @param obj The object to reference.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 构造一个引用给定对象的 atomic_ref。
         *
         * @param obj 要引用的对象。
         *
         * @note 适用类型：所有类型。
         */
        explicit atomic_ref(Ty &obj) noexcept;

        atomic_ref(const atomic_ref &) noexcept;
        atomic_ref &operator=(const atomic_ref &) = delete;

        /**
         * \lang english
         * @brief Atomically stores a value into the referenced object.
         *
         * @param desired The value to store.
         * @param order The memory order (defaults to memory_order::seq_cst).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地将一个值存储到被引用对象中。
         *
         * @param desired 要存储的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> void;

        /**
         * \lang english
         * @brief Atomically loads the value from the referenced object.
         *
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The loaded value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地从被引用对象加载值。
         *
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 加载得到的值。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn load(memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Conversion operator to the referenced value type.
         *
         * @return The current value.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 到被引用值类型的类型转换操作符。
         *
         * @return 当前值。
         *
         * @note 适用类型：所有类型。
         */
        operator Ty() const noexcept;

        /**
         * \lang english
         * @brief Atomically replaces the referenced value and returns the previous value.
         *
         * @param desired The value to store.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the exchange.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 原子地替换被引用值并返回先前的值。
         *
         * @param desired 要存储的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 交换前的值。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Weak compare-and-exchange. May fail spuriously.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param success The memory order on success.
         * @param failure The memory order on failure.
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 弱比较并交换。可能虚假失败。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept -> bool;

        /**
         * \lang english
         * @brief Weak compare-and-exchange with a single memory order.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用单一内存顺序的弱比较并交换。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> bool;

        /**
         * \lang english
         * @brief Strong compare-and-exchange. No spurious failure.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param success The memory order on success.
         * @param failure The memory order on failure.
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 强比较并交换。不会虚假失败。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept -> bool;

        /**
         * \lang english
         * @brief Strong compare-and-exchange with a single memory order.
         *
         * @param expected Reference to the expected value.
         * @param desired The desired value.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return true if the exchange succeeded.
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 使用单一内存顺序的强比较并交换。
         *
         * @param expected 期望值的引用。
         * @param desired 目标值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 若交换成功则返回 true。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> bool;

        /**
         * \lang english
         * @brief Atomically adds a value and returns the previous value.
         *
         * @param arg The value to add.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the addition.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地加上一个值并返回先前的值。
         *
         * @param arg 要增加的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 加法操作前的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically subtracts a value and returns the previous value.
         *
         * @param arg The value to subtract.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the subtraction.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地减去一个值并返回先前的值。
         *
         * @param arg 要减少的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 减法操作前的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the referenced value with the result of a
         *        bitwise AND, and returns the previous value.
         *
         * @param arg The AND operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将被引用值替换为按位与的结果，并返回先前的值。
         *
         * @param arg 与操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the referenced value with the result of a
         *        bitwise OR, and returns the previous value.
         *
         * @param arg The OR operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将被引用值替换为按位或的结果，并返回先前的值。
         *
         * @param arg 或操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the referenced value with the result of a
         *        bitwise XOR, and returns the previous value.
         *
         * @param arg The XOR operand.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 原子地将被引用值替换为按位异或的结果，并返回先前的值。
         *
         * @param arg 异或操作数。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the referenced value with the maximum of the
         *        referenced value and arg, and returns the previous value.
         *
         * @param arg The value to compare.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地将被引用值替换为它与 arg 的最大值，并返回先前的值。
         *
         * @param arg 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Atomically replaces the referenced value with the minimum of the
         *        referenced value and arg, and returns the previous value.
         *
         * @param arg The value to compare.
         * @param order The memory order (defaults to memory_order::seq_cst).
         * @return The value before the operation.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 原子地将被引用值替换为它与 arg 的最小值，并返回先前的值。
         *
         * @param arg 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         * @return 操作前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Post-increment operation.
         *
         * @return The value before the increment.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 后置自增操作。
         *
         * @return 自增前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator++(int) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Post-decrement operation.
         *
         * @return The value before the decrement.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 后置自减操作。
         *
         * @return 自减前的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator--(int) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Pre-increment operation.
         *
         * @return The value after the increment.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 前置自增操作。
         *
         * @return 自增后的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator++() const noexcept -> Ty;

        /**
         * \lang english
         * @brief Pre-decrement operation.
         *
         * @return The value after the decrement.
         *
         * @note Applies to: integral and pointer types only.
         *
         * \lang simp-chinese
         * @brief 前置自减操作。
         *
         * @return 自减后的值。
         *
         * @note 适用类型：仅整数类型与指针类型。
         */
        rain_fn operator--() const noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs addition and assignment.
         *
         * @param arg The value to add.
         * @return The value after the addition.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 执行加法并赋值操作。
         *
         * @param arg 要增加的值。
         * @return 加法操作后的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn operator+=(Ty arg) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs subtraction and assignment.
         *
         * @param arg The value to subtract.
         * @return The value after the subtraction.
         *
         * @note Applies to: integral, floating-point, and pointer types only.
         *
         * \lang simp-chinese
         * @brief 执行减法并赋值操作。
         *
         * @param arg 要减少的值。
         * @return 减法操作后的值。
         *
         * @note 适用类型：仅整数类型、浮点类型与指针类型。
         */
        rain_fn operator-=(Ty arg) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise AND and assignment.
         *
         * @param arg The AND operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位与并赋值操作。
         *
         * @param arg 与操作数。
         * @return 按位与操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator&=(Ty arg) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise OR and assignment.
         *
         * @param arg The OR operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位或并赋值操作。
         *
         * @param arg 或操作数。
         * @return 按位或操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator|=(Ty arg) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Performs bitwise XOR and assignment.
         *
         * @param arg The XOR operand.
         * @return The value after the operation.
         *
         * @note Applies to: integral types only.
         *
         * \lang simp-chinese
         * @brief 执行按位异或并赋值操作。
         *
         * @param arg 异或操作数。
         * @return 按位异或操作后的值。
         *
         * @note 适用类型：仅整数类型。
         */
        rain_fn operator^=(Ty arg) const noexcept -> Ty;

        /**
         * \lang english
         * @brief Blocks until notified and the referenced value changes.
         *
         * @param old The value to compare against.
         * @param order The memory order (defaults to memory_order::seq_cst).
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 阻塞直到被通知且被引用值发生变化。
         *
         * @param old 用于比较的值。
         * @param order 内存顺序（默认为 memory_order::seq_cst）。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept -> void;

        /**
         * \lang english
         * @brief Notifies at least one thread blocked in wait().
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 通知至少一个阻塞在 wait() 中的线程。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn notify_one() const noexcept -> void;

        /**
         * \lang english
         * @brief Notifies all threads blocked in wait().
         *
         * @note Applies to: all types.
         *
         * \lang simp-chinese
         * @brief 通知所有阻塞在 wait() 中的线程。
         *
         * @note 适用类型：所有类型。
         */
        rain_fn notify_all() const noexcept -> void;
    };
}

#endif

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Checks whether the atomic type is lock-free.
     *
     * @tparam Ty The atomic type to check.
     * @param obj The atomic object to check.
     * @return Returns `true` if the atomic object is lock-free, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 检查原子类型是否为无锁
     *
     * @tparam Ty 要检查的原子类型。
     * @param obj 要检查的原子对象。
     * @return 如果原子对象为无锁类型，返回 `true`，否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_is_lock_free(const volatile atomic<Ty> *obj) noexcept -> bool {
        return obj->is_lock_free();
    }

    /**
     * \lang english
     * @brief Checks whether the atomic type is lock-free (const-qualified).
     *
     * @tparam Ty The atomic type to check.
     * @param obj The atomic object to check.
     * @return Returns `true` if the atomic object is lock-free, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 检查原子类型是否为无锁（const 类型）
     *
     * @tparam Ty 要检查的原子类型。
     * @param obj 要检查的原子对象。
     * @return 如果原子对象为无锁类型，返回 `true`，否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_is_lock_free(const atomic<Ty> *obj) noexcept -> bool {
        return obj->is_lock_free();
    }

    /**
     * \lang english
     * @brief Explicitly stores an atomic value.
     *
     * @tparam Ty The atomic type to store.
     * @param obj The atomic object to store the value into.
     * @param desired The value to store.
     * @param order The memory order of the operation.
     *
     * \lang simp-chinese
     * @brief 显式存储原子值
     *
     * @tparam Ty 要存储的原子类型。
     * @param obj 要存储值的原子对象。
     * @param desired 期望存储的值。
     * @param order 操作的内存顺序。
     */
    template <typename Ty>
    rain_fn atomic_store_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept
        -> void {
        obj->store(desired, order);
    }

    /**
     * \lang english
     * @brief Explicitly stores an atomic value (const-qualified).
     *
     * @tparam Ty The atomic type to store.
     * @param obj The atomic object to store the value into.
     * @param desired The value to store.
     * @param order The memory order of the operation.
     *
     * \lang simp-chinese
     * @brief 显式存储原子值（const 类型）
     *
     * @tparam Ty 要存储的原子类型。
     * @param obj 要存储值的原子对象。
     * @param desired 期望存储的值。
     * @param order 操作的内存顺序。
     */
    template <typename Ty>
    rain_fn atomic_store_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept -> void {
        obj->store(desired, order);
    }

    /**
     * \lang english
     * @brief Atomically stores a value (using the default memory order).
     *
     *  Stores the value using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to store.
     * @param obj The atomic object in which to store the value.
     * @param desired The value to store.
     *
     * \lang simp-chinese
     * @brief 存储原子值（使用默认顺序）
     *
     *  使用默认的内存顺序 `memory_order::seq_cst` 存储值。
     *
     * @tparam Ty 要存储的原子类型。
     * @param obj 要存储值的原子对象。
     * @param desired 期望存储的值。
     */
    template <typename Ty>
    rain_fn atomic_store(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept -> void {
        atomic_store_explicit(obj, desired, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically stores a value (using the default memory order) (const-qualified).
     *
     *  Stores the value using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to store.
     * @param obj The atomic object in which to store the value.
     * @param desired The value to store.
     *
     * \lang simp-chinese
     * @brief 存储原子值（使用默认顺序）（const 类型）
     *
     *  使用默认的内存顺序 `memory_order::seq_cst` 存储值。
     *
     * @tparam Ty 要存储的原子类型。
     * @param obj 要存储值的原子对象。
     * @param desired 期望存储的值。
     */
    template <typename Ty>
    rain_fn atomic_store(atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept -> void {
        atomic_store_explicit(obj, desired, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically loads a value with an explicit memory order.
     *
     * @tparam Ty The atomic type to load.
     * @param obj The atomic object from which to load.
     * @param order The memory order of the operation.
     * @return Returns the loaded atomic value.
     *
     * \lang simp-chinese
     * @brief 显式加载原子值
     *
     * @tparam Ty 要加载的原子类型。
     * @param obj 要加载的原子对象。
     * @param order 操作的内存顺序。
     * @return 返回加载的原子值。
     */
    template <typename Ty>
    rain_fn atomic_load_explicit(const volatile atomic<Ty> *obj, memory_order order) noexcept -> Ty {
        return obj->load(order);
    }

    /**
     * \lang english
     * @brief Atomically loads a value with an explicit memory order (const-qualified).
     *
     * @tparam Ty The atomic type to load.
     * @param obj The atomic object from which to load.
     * @param order The memory order of the operation.
     * @return Returns the loaded atomic value.
     *
     * \lang simp-chinese
     * @brief 显式加载原子值（const 类型）
     *
     * @tparam Ty 要加载的原子类型。
     * @param obj 要加载的原子对象。
     * @param order 操作的内存顺序。
     * @return 返回加载的原子值。
     */
    template <typename Ty>
    rain_fn atomic_load_explicit(const atomic<Ty> *obj, memory_order order) noexcept -> Ty {
        return obj->load(order);
    }

    /**
     * \lang english
     * @brief Atomically loads a value (using the default memory order).
     *
     * @tparam Ty The atomic type to load.
     * @param obj The atomic object from which to load.
     * @return Returns the loaded atomic value.
     *
     * \lang simp-chinese
     * @brief 显式加载原子值（使用默认顺序）
     *
     * @tparam Ty 要加载的原子类型。
     * @param obj 要加载的原子对象。
     * @return 返回加载的原子值。
     */
    template <typename Ty>
    rain_fn atomic_load(const volatile atomic<Ty> *obj) noexcept -> Ty {
        return atomic_load_explicit(obj, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically loads a value (using the default memory order) (const-qualified).
     *
     * @tparam Ty The atomic type to load.
     * @param obj The atomic object from which to load.
     * @return Returns the loaded atomic value.
     *
     * \lang simp-chinese
     * @brief 显式加载原子值（使用默认顺序）（const 类型）
     *
     * @tparam Ty 要加载的原子类型。
     * @param obj 要加载的原子对象。
     * @return 返回加载的原子值。
     */
    template <typename Ty>
    rain_fn atomic_load(const atomic<Ty> *obj) noexcept -> Ty {
        return atomic_load_explicit(obj, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Explicitly exchanges an atomic value.
     *
     *  Performs an atomic exchange operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param desired The new value to set.
     * @param order The memory order of the operation.
     * @return Returns the atomic value before the exchange.
     *
     * \lang simp-chinese
     * @brief 显式交换原子值
     *
     *  该函数执行一个原子的交换操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param desired 期望设置的新值。
     * @param order 操作的内存顺序。
     * @return 返回交换之前的原子值。
     */
    template <typename Ty>
    rain_fn atomic_exchange_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept
        -> Ty {
        return obj->exchange(desired, order);
    }

    /**
     * \lang english
     * @brief Atomically exchanges a value with an explicit memory order (const-qualified).
     *
     *  Performs an atomic exchange operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param desired The new value to set.
     * @param order The memory order of the operation.
     * @return Returns the atomic value before the exchange.
     *
     * \lang simp-chinese
     * @brief 显式交换原子值（const 类型）
     *
     *  该函数执行一个原子的交换操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param desired 期望设置的新值。
     * @param order 操作的内存顺序。
     * @return 返回交换之前的原子值。
     */
    template <typename Ty>
    rain_fn atomic_exchange_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept -> Ty {
        return obj->exchange(desired, order);
    }

    /**
     * \lang english
     * @brief Atomically exchanges a value (using the default memory order).
     *
     *  Performs the exchange using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param desired The new value to set.
     * @return Returns the atomic value before the exchange.
     *
     * \lang simp-chinese
     * @brief 交换原子值（使用默认顺序）
     *
     *  使用默认的内存顺序 `memory_order::seq_cst` 执行交换操作。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param desired 期望设置的新值。
     * @return 返回交换之前的原子值。
     */
    template <typename Ty>
    rain_fn atomic_exchange(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept -> Ty {
        return atomic_exchange_explicit(obj, desired, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically exchanges a value (using the default memory order) (const-qualified).
     *
     *  Performs the exchange using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param desired The new value to set.
     * @return Returns the atomic value before the exchange.
     *
     * \lang simp-chinese
     * @brief 交换原子值（使用默认顺序）（const 类型）
     *
     *  使用默认的内存顺序 `memory_order::seq_cst` 执行交换操作。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param desired 期望设置的新值。
     * @return 返回交换之前的原子值。
     */
    template <typename Ty>
    rain_fn atomic_exchange(atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept -> Ty {
        return atomic_exchange_explicit(obj, desired, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a weak compare-and-exchange with explicit memory orders.
     *
     *  Attempts an atomic compare-and-exchange operation (weak form) using the explicitly specified memory orders.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @param success The memory order for the successful case.
     * @param failure The memory order for the failed case.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 显式弱比较交换原子值
     *
     *  该函数尝试进行原子比较和交换操作（弱版本），并使用显式指定的内存顺序。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @param success 操作成功时的内存顺序。
     * @param failure 操作失败时的内存顺序。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_weak_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                                  typename atomic<Ty>::value_type desired, memory_order success,
                                                  memory_order failure) noexcept -> bool {
        return obj->compare_exchange_weak(*expected, desired, success, failure);
    }

    /**
     * \lang english
     * @brief Atomically performs a weak compare-and-exchange with explicit memory orders (const-qualified).
     *
     *  Attempts an atomic compare-and-exchange operation (weak form) using the explicitly specified memory orders.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @param success The memory order for the successful case.
     * @param failure The memory order for the failed case.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 显式弱比较交换原子值（const 类型）
     *
     *  该函数尝试进行原子比较和交换操作（弱版本），并使用显式指定的内存顺序。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @param success 操作成功时的内存顺序。
     * @param failure 操作失败时的内存顺序。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_weak_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                                  typename atomic<Ty>::value_type desired, memory_order success,
                                                  memory_order failure) noexcept -> bool {
        return obj->compare_exchange_weak(*expected, desired, success, failure);
    }

    /**
     * \lang english
     * @brief Atomically performs a weak compare-and-exchange (using the default memory order).
     *
     *  Attempts an atomic compare-and-exchange operation (weak version) using the default
     *  memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected atomic value, updated to the old value of the atomic object on success.
     * @param desired The new value to set.
     * @return Returns `true` if the exchange succeeds, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 弱比较交换原子值（使用默认顺序）
     *
     *  该函数尝试进行原子比较和交换操作（弱版本），并使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_weak(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                         typename atomic<Ty>::value_type desired) noexcept -> bool {
        return atomic_compare_exchange_weak_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a weak compare-and-exchange (using the default memory order) (const-qualified).
     *
     *  Attempts an atomic compare-and-exchange operation (weak form) using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 弱比较交换原子值（使用默认顺序）（const 类型）
     *
     *  该函数尝试进行原子比较和交换操作（弱版本），并使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_weak(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                         typename atomic<Ty>::value_type desired) noexcept -> bool {
        return atomic_compare_exchange_weak_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a strong compare-and-exchange with explicit memory orders.
     *
     *  Attempts an atomic compare-and-exchange operation (strong form) using the explicitly specified memory orders.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @param success The memory order for the successful case.
     * @param failure The memory order for the failed case.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 显式强比较交换原子值
     *
     *  该函数尝试进行原子比较和交换操作（强版本），并使用显式指定的内存顺序。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @param success 操作成功时的内存顺序。
     * @param failure 操作失败时的内存顺序。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_strong_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                                    typename atomic<Ty>::value_type desired, memory_order success,
                                                    memory_order failure) noexcept -> bool {
        return obj->compare_exchange_strong(*expected, desired, success, failure);
    }

    /**
     * \lang english
     * @brief Atomically performs a strong compare-and-exchange with explicit memory orders (const-qualified).
     *
     *  Attempts an atomic compare-and-exchange operation (strong form) using the explicitly specified memory orders.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @param success The memory order for the successful case.
     * @param failure The memory order for the failed case.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 显式强比较交换原子值（const 类型）
     *
     *  该函数尝试进行原子比较和交换操作（强版本），并使用显式指定的内存顺序。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @param success 操作成功时的内存顺序。
     * @param failure 操作失败时的内存顺序。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_strong_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                                    typename atomic<Ty>::value_type desired, memory_order success,
                                                    memory_order failure) noexcept -> bool {
        return obj->compare_exchange_strong(*expected, desired, success, failure);
    }

    /**
     * \lang english
     * @brief Atomically performs a strong compare-and-exchange (using the default memory order).
     *
     *  Attempts an atomic compare-and-exchange operation (strong form) using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 强比较交换原子值（使用默认顺序）
     *
     *  该函数尝试进行原子比较和交换操作（强版本），并使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_strong(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                           typename atomic<Ty>::value_type desired) noexcept -> bool {
        return atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a strong compare-and-exchange (using the default memory order) (const-qualified).
     *
     *  Attempts an atomic compare-and-exchange operation (strong form) using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type to exchange.
     * @param obj The atomic object to exchange.
     * @param expected The expected value, updated to the old value of the atomic object when the exchange succeeds.
     * @param desired The new value to set.
     * @return Returns `true` if the exchange succeeded, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 强比较交换原子值（使用默认顺序）（const 类型）
     *
     *  该函数尝试进行原子比较和交换操作（强版本），并使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要交换的原子类型。
     * @param obj 要交换的原子对象。
     * @param expected 期望的原子值，在交换成功时更新为原子对象的旧值。
     * @param desired 期望设置的新值。
     * @return 如果成功交换，返回 `true`；否则返回 `false`。
     */
    template <typename Ty>
    rain_fn atomic_compare_exchange_strong(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                           typename atomic<Ty>::value_type desired) noexcept -> bool {
        return atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs an addition with an explicit memory order.
     *
     *  Performs an atomic addition operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the addition.
     * @param obj The atomic object on which to perform the addition.
     * @param arg The amount to add.
     * @param order The memory order.
     * @return Returns the value before the atomic addition.
     *
     * \lang simp-chinese
     * @brief 显式执行原子加法操作
     *
     *  该函数执行一个原子的加法操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行加法操作的原子类型。
     * @param obj 要执行加法操作的原子对象。
     * @param arg 加法的增量。
     * @param order 内存顺序。
     * @return 返回原子加法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_add_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_add(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs an addition with an explicit memory order (const-qualified).
     *
     *  Performs an atomic addition operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the addition.
     * @param obj The atomic object on which to perform the addition.
     * @param arg The amount to add.
     * @param order The memory order.
     * @return Returns the value before the atomic addition.
     *
     * \lang simp-chinese
     * @brief 显式执行原子加法操作（const 类型）
     *
     *  该函数执行一个原子的加法操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行加法操作的原子类型。
     * @param obj 要执行加法操作的原子对象。
     * @param arg 加法的增量。
     * @param order 内存顺序。
     * @return 返回原子加法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_add_explicit(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_add(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs an addition (using the default memory order).
     *
     *  Performs an atomic addition operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the addition.
     * @param obj The atomic object on which to perform the addition.
     * @param arg The amount to add.
     * @return Returns the value before the atomic addition.
     *
     * \lang simp-chinese
     * @brief 执行原子加法操作（使用默认顺序）
     *
     *  该函数执行一个原子的加法操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行加法操作的原子类型。
     * @param obj 要执行加法操作的原子对象。
     * @param arg 加法的增量。
     * @return 返回原子加法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_add(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept -> Ty {
        return atomic_fetch_add_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs an addition (using the default memory order) (const-qualified).
     *
     *  Performs an atomic addition operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to add.
     * @param obj The atomic object on which to add.
     * @param arg The increment of the addition.
     * @return Returns the value before the atomic addition.
     *
     * \lang simp-chinese
     * @brief 执行原子加法操作（使用默认顺序）（const 类型）
     *
     *  该函数执行一个原子的加法操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行加法操作的原子类型。
     * @param obj 要执行加法操作的原子对象。
     * @param arg 加法的增量。
     * @return 返回原子加法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_add(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept -> Ty {
        return atomic_fetch_add_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a subtraction with an explicit memory order.
     *
     *  Performs an atomic subtraction operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the subtraction.
     * @param obj The atomic object on which to perform the subtraction.
     * @param arg The amount to subtract.
     * @param order The memory order.
     * @return Returns the value before the atomic subtraction.
     *
     * \lang simp-chinese
     * @brief 显式执行原子减法操作
     *
     *  该函数执行一个原子的减法操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行减法操作的原子类型。
     * @param obj 要执行减法操作的原子对象。
     * @param arg 减法的减量。
     * @param order 内存顺序。
     * @return 返回原子减法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_sub_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_sub(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a subtraction with an explicit memory order (const-qualified).
     *
     *  Performs an atomic subtraction operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the subtraction.
     * @param obj The atomic object on which to perform the subtraction.
     * @param arg The amount to subtract.
     * @param order The memory order.
     * @return Returns the value before the atomic subtraction.
     *
     * \lang simp-chinese
     * @brief 显式执行原子减法操作（const 类型）
     *
     *  该函数执行一个原子的减法操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行减法操作的原子类型。
     * @param obj 要执行减法操作的原子对象。
     * @param arg 减法的减量。
     * @param order 内存顺序。
     * @return 返回原子减法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_sub_explicit(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_sub(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a subtraction (using the default memory order).
     *
     *  Performs an atomic subtraction operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the subtraction.
     * @param obj The atomic object on which to perform the subtraction.
     * @param arg The amount to subtract.
     * @return Returns the value before the atomic subtraction.
     *
     * \lang simp-chinese
     * @brief 执行原子减法操作（使用默认顺序）
     *
     *  该函数执行一个原子的减法操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行减法操作的原子类型。
     * @param obj 要执行减法操作的原子对象。
     * @param arg 减法的减量。
     * @return 返回原子减法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_sub(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept -> Ty {
        return atomic_fetch_sub_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a subtraction (using the default memory order) (const-qualified).
     *
     *  Performs an atomic subtraction operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the subtraction.
     * @param obj The atomic object on which to perform the subtraction.
     * @param arg The amount to subtract.
     * @return Returns the value before the atomic subtraction.
     *
     * \lang simp-chinese
     * @brief 执行原子减法操作（使用默认顺序）（const 类型）
     *
     *  该函数执行一个原子的减法操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行减法操作的原子类型。
     * @param obj 要执行减法操作的原子对象。
     * @param arg 减法的减量。
     * @return 返回原子减法操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_sub(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept -> Ty {
        return atomic_fetch_sub_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND with an explicit memory order.
     *
     *  Performs an atomic AND operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the AND.
     * @param obj The atomic object on which to perform the AND.
     * @param arg The operand of the AND operation.
     * @param order The memory order.
     * @return Returns the value before the atomic AND.
     *
     * \lang simp-chinese
     * @brief 显式执行原子与操作
     *
     *  该函数执行原子的与操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行与操作的原子类型。
     * @param obj 要执行与操作的原子对象。
     * @param arg 与操作的参数。
     * @param order 内存顺序。
     * @return 返回原子与操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_and_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_and(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND with an explicit memory order (const-qualified).
     *
     *  Performs an atomic AND operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the AND.
     * @param obj The atomic object on which to perform the AND.
     * @param arg The operand of the AND operation.
     * @param order The memory order.
     * @return Returns the value before the atomic AND.
     *
     * \lang simp-chinese
     * @brief 显式执行原子与操作（const 类型）
     *
     *  该函数执行原子的与操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行与操作的原子类型。
     * @param obj 要执行与操作的原子对象。
     * @param arg 与操作的参数。
     * @param order 内存顺序。
     * @return 返回原子与操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_and_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_and(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND (using the default memory order).
     *
     *  Performs an atomic bitwise AND operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to AND.
     * @param obj The atomic object on which to AND.
     * @param arg The argument of the bitwise AND operation.
     * @return Returns the value before the atomic bitwise AND.
     *
     * \lang simp-chinese
     * @brief 执行原子与操作（使用默认顺序）
     *
     *  该函数执行原子的与操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行与操作的原子类型。
     * @param obj 要执行与操作的原子对象。
     * @param arg 与操作的参数。
     * @return 返回原子与操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_and(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_and_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise AND (using the default memory order) (const-qualified).
     *
     *  Performs an atomic bitwise AND operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to AND.
     * @param obj The atomic object on which to AND.
     * @param arg The argument of the bitwise AND operation.
     * @return Returns the value before the atomic bitwise AND.
     *
     * \lang simp-chinese
     * @brief 执行原子与操作（使用默认顺序）（const 类型）
     *
     *  该函数执行原子的与操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行与操作的原子类型。
     * @param obj 要执行与操作的原子对象。
     * @param arg 与操作的参数。
     * @return 返回原子与操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_and(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_and_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Explicitly performs an atomic bitwise OR operation.
     *
     *  Performs an atomic bitwise OR operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to OR.
     * @param obj The atomic object on which to OR.
     * @param arg The argument of the bitwise OR operation.
     * @param order The memory order.
     * @return Returns the value before the atomic bitwise OR.
     *
     * \lang simp-chinese
     * @brief 显式执行原子或操作
     *
     *  该函数执行原子的或操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行或操作的原子类型。
     * @param obj 要执行或操作的原子对象。
     * @param arg 或操作的参数。
     * @param order 内存顺序。
     * @return 返回原子或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_or_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_or(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR with an explicit memory order (const-qualified).
     *
     *  Performs an atomic OR operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the OR.
     * @param obj The atomic object on which to perform the OR.
     * @param arg The operand of the OR operation.
     * @param order The memory order.
     * @return Returns the value before the atomic OR.
     *
     * \lang simp-chinese
     * @brief 显式执行原子或操作（const 类型）
     *
     *  该函数执行原子的或操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行或操作的原子类型。
     * @param obj 要执行或操作的原子对象。
     * @param arg 或操作的参数。
     * @param order 内存顺序。
     * @return 返回原子或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_or_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_or(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR (using the default memory order).
     *
     *  Performs an atomic OR operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the OR.
     * @param obj The atomic object on which to perform the OR.
     * @param arg The operand of the OR operation.
     * @return Returns the value before the atomic OR.
     *
     * \lang simp-chinese
     * @brief 执行原子或操作（使用默认顺序）
     *
     *  该函数执行原子的或操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行或操作的原子类型。
     * @param obj 要执行或操作的原子对象。
     * @param arg 或操作的参数。
     * @return 返回原子或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_or(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_or_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise OR (using the default memory order) (const-qualified).
     *
     *  Performs an atomic OR operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the OR.
     * @param obj The atomic object on which to perform the OR.
     * @param arg The operand of the OR operation.
     * @return Returns the value before the atomic OR.
     *
     * \lang simp-chinese
     * @brief 执行原子或操作（使用默认顺序）（const 类型）
     *
     *  该函数执行原子的或操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行或操作的原子类型。
     * @param obj 要执行或操作的原子对象。
     * @param arg 或操作的参数。
     * @return 返回原子或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_or(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_or_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR with an explicit memory order.
     *
     *  Performs an atomic XOR operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the XOR.
     * @param obj The atomic object on which to perform the XOR.
     * @param arg The operand of the XOR operation.
     * @param order The memory order.
     * @return Returns the value before the atomic XOR.
     *
     * \lang simp-chinese
     * @brief 显式执行原子异或操作
     *
     *  该函数执行原子的异或操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行异或操作的原子类型。
     * @param obj 要执行异或操作的原子对象。
     * @param arg 异或操作的参数。
     * @param order 内存顺序。
     * @return 返回原子异或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_xor_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_xor(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR with an explicit memory order (const-qualified).
     *
     *  Performs an atomic XOR operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the XOR.
     * @param obj The atomic object on which to perform the XOR.
     * @param arg The operand of the XOR operation.
     * @param order The memory order.
     * @return Returns the value before the atomic XOR.
     *
     * \lang simp-chinese
     * @brief 显式执行原子异或操作（const 类型）
     *
     *  该函数执行原子的异或操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行异或操作的原子类型。
     * @param obj 要执行异或操作的原子对象。
     * @param arg 异或操作的参数。
     * @param order 内存顺序。
     * @return 返回原子异或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_xor_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_xor(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR (using the default memory order).
     *
     *  Performs an atomic XOR operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the XOR.
     * @param obj The atomic object on which to perform the XOR.
     * @param arg The operand of the XOR operation.
     * @return Returns the value before the atomic XOR.
     *
     * \lang simp-chinese
     * @brief 执行原子异或操作（使用默认顺序）
     *
     *  该函数执行一个原子的异或操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行异或操作的原子类型。
     * @param obj 要执行异或操作的原子对象。
     * @param arg 异或操作的参数。
     * @return 返回原子异或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_xor(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_xor_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a bitwise XOR (using the default memory order) (const-qualified).
     *
     *  Performs an atomic XOR operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the XOR.
     * @param obj The atomic object on which to perform the XOR.
     * @param arg The operand of the XOR operation.
     * @return Returns the value before the atomic XOR.
     *
     * \lang simp-chinese
     * @brief 执行原子异或操作（使用默认顺序）（const 类型）
     *
     *  该函数执行一个原子的异或操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行异或操作的原子类型。
     * @param obj 要执行异或操作的原子对象。
     * @param arg 异或操作的参数。
     * @return 返回原子异或操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_xor(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_xor_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a maximum operation with an explicit memory order.
     *
     *  Performs an atomic maximum operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the maximum.
     * @param obj The atomic object on which to perform the maximum.
     * @param arg The operand of the maximum operation.
     * @param order The memory order.
     * @return Returns the value before the atomic maximum.
     *
     * \lang simp-chinese
     * @brief 显式执行原子最大值操作
     *
     *  该函数执行原子的最大值操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行最大值操作的原子类型。
     * @param obj 要执行最大值操作的原子对象。
     * @param arg 最大值操作的参数。
     * @param order 内存顺序。
     * @return 返回原子最大值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_max_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_max(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a maximum operation with an explicit memory order (const-qualified).
     *
     *  Performs an atomic maximum operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the maximum.
     * @param obj The atomic object on which to perform the maximum.
     * @param arg The operand of the maximum operation.
     * @param order The memory order.
     * @return Returns the value before the atomic maximum.
     *
     * \lang simp-chinese
     * @brief 显式执行原子最大值操作（const 类型）
     *
     *  该函数执行原子的最大值操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行最大值操作的原子类型。
     * @param obj 要执行最大值操作的原子对象。
     * @param arg 最大值操作的参数。
     * @param order 内存顺序。
     * @return 返回原子最大值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_max_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_max(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a maximum operation (using the default memory order).
     *
     *  Performs an atomic maximum operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the maximum.
     * @param obj The atomic object on which to perform the maximum.
     * @param arg The operand of the maximum operation.
     * @return Returns the value before the atomic maximum.
     *
     * \lang simp-chinese
     * @brief 执行原子最大值操作（使用默认顺序）
     *
     *  该函数执行一个原子的最大值操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行最大值操作的原子类型。
     * @param obj 要执行最大值操作的原子对象。
     * @param arg 最大值操作的参数。
     * @return 返回原子最大值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_max(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_max_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a maximum operation (using the default memory order) (const-qualified).
     *
     *  Performs an atomic maximum operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to perform the maximum.
     * @param obj The atomic object on which to perform the maximum.
     * @param arg The operand of the maximum operation.
     * @return Returns the value before the atomic maximum.
     *
     * \lang simp-chinese
     * @brief 执行原子最大值操作（使用默认顺序）（const 类型）
     *
     *  该函数执行一个原子的最大值操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行最大值操作的原子类型。
     * @param obj 要执行最大值操作的原子对象。
     * @param arg 最大值操作的参数。
     * @return 返回原子最大值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_max(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_max_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a minimum operation with an explicit memory order.
     *
     *  Performs an atomic minimum operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the minimum.
     * @param obj The atomic object on which to perform the minimum.
     * @param arg The operand of the minimum operation.
     * @param order The memory order.
     * @return Returns the value before the atomic minimum.
     *
     * \lang simp-chinese
     * @brief 显式执行原子最小值操作
     *
     *  该函数执行原子的最小值操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行最小值操作的原子类型。
     * @param obj 要执行最小值操作的原子对象。
     * @param arg 最小值操作的参数。
     * @param order 内存顺序。
     * @return 返回原子最小值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_min_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept
        -> Ty {
        return obj->fetch_min(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a minimum operation with an explicit memory order (const-qualified).
     *
     *  Performs an atomic minimum operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to perform the minimum.
     * @param obj The atomic object on which to perform the minimum.
     * @param arg The operand of the minimum operation.
     * @param order The memory order.
     * @return Returns the value before the atomic minimum.
     *
     * \lang simp-chinese
     * @brief 显式执行原子最小值操作（const 类型）
     *
     *  该函数执行原子的最小值操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行最小值操作的原子类型。
     * @param obj 要执行最小值操作的原子对象。
     * @param arg 最小值操作的参数。
     * @param order 内存顺序。
     * @return 返回原子最小值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_min_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept -> Ty {
        return obj->fetch_min(arg, order);
    }

    /**
     * \lang english
     * @brief Atomically performs a minimum operation (using the default memory order).
     *
     *  Performs an atomic minimum operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to take the minimum.
     * @param obj The atomic object on which to take the minimum.
     * @param arg The argument of the minimum operation.
     * @return Returns the value before the atomic minimum operation.
     *
     * \lang simp-chinese
     * @brief 执行原子最小值操作（使用默认顺序）
     *
     *  该函数执行一个原子的最小值操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行最小值操作的原子类型。
     * @param obj 要执行最小值操作的原子对象。
     * @param arg 最小值操作的参数。
     * @return 返回原子最小值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_min(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_min_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically performs a minimum operation (using the default memory order) (const-qualified).
     *
     *  Performs an atomic minimum operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to take the minimum.
     * @param obj The atomic object on which to take the minimum.
     * @param arg The argument of the minimum operation.
     * @return Returns the value before the atomic minimum operation.
     *
     * \lang simp-chinese
     * @brief 执行原子最小值操作（使用默认顺序）（const 类型）
     *
     *  该函数执行一个原子的最小值操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行最小值操作的原子类型。
     * @param obj 要执行最小值操作的原子对象。
     * @param arg 最小值操作的参数。
     * @return 返回原子最小值操作之前的值。
     */
    template <typename Ty>
    rain_fn atomic_fetch_min(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept -> Ty {
        return atomic_fetch_min_explicit(obj, arg, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Explicitly waits for an atomic change.
     *
     *  Performs an atomic wait operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to wait.
     * @param obj The atomic object on which to wait.
     * @param old The old value to compare against.
     * @param order The memory order.
     *
     * \lang simp-chinese
     * @brief 显式等待原子操作
     *
     *  该函数执行原子的等待操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行等待操作的原子类型。
     * @param obj 要执行等待操作的原子对象。
     * @param old 比较的旧值。
     * @param order 内存顺序。
     */
    template <typename Ty>
    void atomic_wait_explicit(const volatile atomic<Ty> *obj, typename atomic<Ty>::value_type old, memory_order order) {
        obj->wait(old, order);
    }

    /**
     * \lang english
     * @brief Atomically waits for a change with an explicit memory order (const-qualified).
     *
     *  Performs an atomic wait operation using the explicitly specified memory order.
     *
     * @tparam Ty The atomic type on which to wait.
     * @param obj The atomic object on which to wait.
     * @param old The old value to compare against.
     * @param order The memory order.
     *
     * \lang simp-chinese
     * @brief 显式等待原子操作（const 类型）
     *
     *  该函数执行原子的等待操作，使用显式指定的内存顺序。
     *
     * @tparam Ty 要执行等待操作的原子类型。
     * @param obj 要执行等待操作的原子对象。
     * @param old 比较的旧值。
     * @param order 内存顺序。
     */
    template <typename Ty>
    void atomic_wait_explicit(const atomic<Ty> *obj, typename atomic<Ty>::value_type old, memory_order order) {
        obj->wait(old, order);
    }

    /**
     * \lang english
     * @brief Atomically waits for a change (using the default memory order).
     *
     *  Performs an atomic wait operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to wait.
     * @param obj The atomic object on which to wait.
     * @param old The old value to compare against.
     *
     * \lang simp-chinese
     * @brief 执行原子等待操作（使用默认顺序）
     *
     *  该函数执行一个原子的等待操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行等待操作的原子类型。
     * @param obj 要执行等待操作的原子对象。
     * @param old 比较的旧值。
     */
    template <typename Ty>
    void atomic_wait(const volatile atomic<Ty> *obj, typename atomic<Ty>::value_type old) {
        atomic_wait_explicit(obj, old, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically waits for a change (using the default memory order) (const-qualified).
     *
     *  Performs an atomic wait operation using the default memory order `memory_order::seq_cst`.
     *
     * @tparam Ty The atomic type on which to wait.
     * @param obj The atomic object on which to wait.
     * @param old The old value to compare against.
     *
     * \lang simp-chinese
     * @brief 执行原子等待操作（使用默认顺序）（const 类型）
     *
     *  该函数执行一个原子的等待操作，使用默认的内存顺序 `memory_order::seq_cst`。
     *
     * @tparam Ty 要执行等待操作的原子类型。
     * @param obj 要执行等待操作的原子对象。
     * @param old 比较的旧值。
     */
    template <typename Ty>
    void atomic_wait(const atomic<Ty> *obj, typename atomic<Ty>::value_type old) {
        atomic_wait_explicit(obj, old, memory_order::seq_cst);
    }

    /**
     * \lang english
     * @brief Atomically notifies a single waiting thread.
     *
     *  Performs an atomic notification, waking only one waiting thread.
     *
     * @tparam Ty The atomic type on which to notify.
     * @param obj The atomic object on which to notify.
     *
     * \lang simp-chinese
     * @brief 执行原子通知操作（单个）
     *
     *  该函数执行原子的通知操作，仅通知一个等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_one(volatile atomic<Ty> *obj) {
        obj->notify_one();
    }

    /**
     * \lang english
     * @brief Atomically notifies a single waiting thread (const-qualified).
     *
     *  Performs an atomic notification, waking only one waiting thread.
     *
     * @tparam Ty The atomic type on which to notify.
     * @param obj The atomic object on which to notify.
     *
     * \lang simp-chinese
     * @brief 执行原子通知操作（单个）（const 类型）
     *
     *  该函数执行原子的通知操作，仅通知一个等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_one(atomic<Ty> *obj) {
        obj->notify_one();
    }

    /**
     * \lang english
     * @brief Atomically notifies all waiting threads.
     *
     *  Performs an atomic notification, waking all waiting threads.
     *
     * @tparam Ty The atomic type on which to notify.
     * @param obj The atomic object on which to notify.
     *
     * \lang simp-chinese
     * @brief 执行原子通知操作（全部）
     *
     *  该函数执行原子的通知操作，通知所有等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_all(volatile atomic<Ty> *obj) {
        obj->notify_all();
    }

    /**
     * \lang english
     * @brief Atomically notifies all waiting threads (const-qualified).
     *
     *  Performs an atomic notification, waking all waiting threads.
     *
     * @tparam Ty The atomic type on which to notify.
     * @param obj The atomic object on which to notify.
     *
     * \lang simp-chinese
     * @brief 执行原子通知操作（全部）（const 类型）
     *
     *  该函数执行原子的通知操作，通知所有等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_all(atomic<Ty> *obj) {
        obj->notify_all();
    }
}

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Atomic flag type.
     *
     *  A simple atomic flag supporting test, set, clear, wait, and notify operations.
     *
     * \lang simp-chinese
     * @brief 原子标志类型。
     *
     *  支持测试、设置、清除、等待和通知操作的简单原子标志。
     */
    class atomic_flag : public implements::atomic_flag_storage {
    public:
        /**
         * \lang english
         * @brief Default constructor.
         *
         *  Initializes an `atomic_flag` object.
         *
         * \lang simp-chinese
         * @brief 默认构造函数
         *
         *  初始化一个 `atomic_flag` 对象。
         */
        constexpr atomic_flag() noexcept : implements::atomic_flag_storage() {
        }

        /**
         * \lang english
         * @brief Copy constructor is deleted.
         *
         * \lang simp-chinese
         * @brief 禁止拷贝构造
         */
        atomic_flag(const atomic_flag &) = delete;

        /**
         * \lang english
         * @brief Copy assignment operator is deleted.
         *
         * \lang simp-chinese
         * @brief 禁止拷贝赋值
         */
        atomic_flag &operator=(const atomic_flag &) = delete;

        /**
         * \lang english
         * @brief Copy assignment operator (volatile-qualified) is deleted.
         *
         * \lang simp-chinese
         * @brief 禁止拷贝赋值（volatile 修饰）
         */
        atomic_flag &operator=(const atomic_flag &) volatile = delete;

        /**
         * \lang english
         * @brief Tests the state of the flag.
         *
         * @param order The memory order of the operation (defaults to memory_order::seq_cst).
         * @return Returns `true` if the flag is set, otherwise `false`.
         *
         * \lang simp-chinese
         * @brief 测试标志位的状态
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test(memory_order order = memory_order::seq_cst) const noexcept -> bool {
            return atomic_flag_storage::test(order);
        }

        /**
         * \lang english
         * @brief Tests the state of the flag (volatile-qualified).
         *
         * @param order The memory order of the operation (defaults to memory_order::seq_cst).
         * @return Returns `true` if the flag is set, otherwise `false`.
         *
         * \lang simp-chinese
         * @brief 测试标志位的状态（volatile 修饰）
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test(memory_order order = memory_order::seq_cst) const volatile noexcept -> bool {
            return atomic_flag_storage::test(order);
        }

        /**
         * \lang english
         * @brief Tests and sets the flag.
         *
         * @param order The memory order of the operation (defaults to memory_order::seq_cst).
         * @return Returns `true` if the flag was already set, otherwise `false`.
         *
         * \lang simp-chinese
         * @brief 测试并设置标志位
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test_and_set(memory_order order = memory_order::seq_cst) noexcept -> bool {
            return atomic_flag_storage::test_and_set(order);
        }

        /**
         * \lang english
         * @brief Tests and sets the flag (volatile-qualified).
         *
         * @param order The memory order of the operation (defaults to memory_order::seq_cst).
         * @return Returns `true` if the flag was already set, otherwise `false`.
         *
         * \lang simp-chinese
         * @brief 测试并设置标志位（volatile 修饰）
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test_and_set(memory_order order = memory_order::seq_cst) volatile noexcept -> bool {
            return atomic_flag_storage::test_and_set(order);
        }

        /**
         * \lang english
         * @brief Clears the flag.
         *
         * @param order The memory order of the operation (defaults to memory_order::seq_cst).
         *
         * \lang simp-chinese
         * @brief 清除标志位
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn clear(memory_order order = memory_order::seq_cst) noexcept -> void {
            atomic_flag_storage::clear(order);
        }

        /**
         * \lang english
         * @brief Clears the flag (volatile-qualified).
         *
         * @param order The memory order of the operation (defaults to memory_order::seq_cst).
         *
         * \lang simp-chinese
         * @brief 清除标志位（volatile 修饰）
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn clear(memory_order order = memory_order::seq_cst) volatile noexcept -> void {
            atomic_flag_storage::clear(order);
        }

        /**
         * \lang english
         * @brief Waits for the flag to change.
         *
         * @param old The expected old value.
         * @param order The memory order of the wait operation (defaults to memory_order::seq_cst).
         *
         * \lang simp-chinese
         * @brief 等待标志位的变化
         *
         * @param old 期望的旧值。
         * @param order 等待操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(bool old, memory_order order = memory_order::seq_cst) const noexcept -> void {
            atomic_flag_storage::wait(old, order);
        }

        /**
         * \lang english
         * @brief Waits for the flag to change (volatile-qualified).
         *
         * @param old The expected old value.
         * @param order The memory order of the wait operation (defaults to memory_order::seq_cst).
         *
         * \lang simp-chinese
         * @brief 等待标志位的变化（volatile 修饰）
         *
         * @param old 期望的旧值。
         * @param order 等待操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(bool old, memory_order order = memory_order::seq_cst) const volatile noexcept -> void {
            atomic_flag_storage::wait(old, order);
        }

        /**
         * \lang english
         * @brief Notifies one waiting thread.
         *
         * \lang simp-chinese
         * @brief 通知一个等待的线程
         */
        rain_fn notify_one() noexcept -> void {
            atomic_flag_storage::notify_one();
        }

        /**
         * \lang english
         * @brief Notifies one waiting thread (volatile-qualified).
         *
         * \lang simp-chinese
         * @brief 通知一个等待的线程（volatile 修饰）
         */
        rain_fn notify_one() volatile noexcept -> void {
            atomic_flag_storage::notify_one();
        }

        /**
         * \lang english
         * @brief Notifies all waiting threads.
         *
         * \lang simp-chinese
         * @brief 通知所有等待的线程
         */
        rain_fn notify_all() noexcept -> void {
            atomic_flag_storage::notify_all();
        }

        /**
         * \lang english
         * @brief Notifies all waiting threads (volatile-qualified).
         *
         * \lang simp-chinese
         * @brief 通知所有等待的线程（volatile 修饰）
         */
        rain_fn notify_all() volatile noexcept -> void {
            atomic_flag_storage::notify_all();
        }
    };

    /**
     * \lang english
     * @brief Tests the flag.
     *
     * @param flag The `atomic_flag` object to test.
     * @param order The memory order of the operation (defaults to memory_order::seq_cst).
     * @return Returns `true` if the flag is set, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 测试标志位
     *
     * @param flag 要测试的 `atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     * @return 如果标志已设置返回 `true`，否则返回 `false`。
     */
    RAINY_INLINE bool atomic_flag_test(const atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test(order);
    }

    /**
     * \lang english
     * @brief Tests the flag (volatile-qualified).
     *
     * @param flag The `volatile atomic_flag` object to test.
     * @param order The memory order of the operation (defaults to memory_order::seq_cst).
     * @return Returns `true` if the flag is set, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 测试标志位（volatile 修饰）
     *
     * @param flag 要测试的 `volatile atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     * @return 如果标志已设置返回 `true`，否则返回 `false`。
     */
    RAINY_INLINE bool atomic_flag_test(const volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test(order);
    }

    /**
     * \lang english
     * @brief Tests and sets the flag.
     *
     * @param flag The `atomic_flag` object to operate on.
     * @param order The memory order of the operation (defaults to memory_order::seq_cst).
     * @return Returns `true` if the flag was set, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 测试并设置标志位
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     * @return 如果标志已设置返回 `true`，否则返回 `false`。
     */
    RAINY_INLINE bool atomic_flag_test_and_set(atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test_and_set(order);
    }

    /**
     * \lang english
     * @brief Tests and sets the flag (volatile-qualified).
     *
     * @param flag The `volatile atomic_flag` object to operate on.
     * @param order The memory order of the operation (defaults to memory_order::seq_cst).
     * @return Returns `true` if the flag was set, otherwise `false`.
     *
     * \lang simp-chinese
     * @brief 测试并设置标志位（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     * @return 如果标志已设置返回 `true`，否则返回 `false`。
     */
    RAINY_INLINE bool atomic_flag_test_and_set(volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test_and_set(order);
    }

    /**
     * \lang english
     * @brief Clears the flag.
     *
     * @param flag The `atomic_flag` object to operate on.
     * @param order The memory order of the operation (defaults to memory_order::seq_cst).
     *
     * \lang simp-chinese
     * @brief 清除标志位
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     */
    RAINY_INLINE void atomic_flag_clear(atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        flag->clear(order);
    }

    /**
     * \lang english
     * @brief Clears the flag (volatile-qualified).
     *
     * @param flag The `volatile atomic_flag` object to operate on.
     * @param order The memory order of the operation (defaults to memory_order::seq_cst).
     *
     * \lang simp-chinese
     * @brief 清除标志位（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     */
    RAINY_INLINE void atomic_flag_clear(volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        flag->clear(order);
    }

    /**
     * \lang english
     * @brief Waits for the flag to change.
     *
     * @param flag The `atomic_flag` object to operate on.
     * @param old The expected old value.
     * @param order The memory order of the wait operation (defaults to memory_order::seq_cst).
     *
     * \lang simp-chinese
     * @brief 等待标志位的变化
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     * @param old 期望的旧值。
     * @param order 等待操作的内存顺序（默认为 memory_order::seq_cst）。
     */
    RAINY_INLINE void atomic_flag_wait(const atomic_flag *flag, bool old, memory_order order = memory_order::seq_cst) noexcept {
        flag->wait(old, order);
    }

    /**
     * \lang english
     * @brief Waits for the flag to change (volatile-qualified).
     *
     * @param flag The `volatile atomic_flag` object to operate on.
     * @param old The expected old value.
     * @param order The memory order of the wait operation (defaults to memory_order::seq_cst).
     *
     * \lang simp-chinese
     * @brief 等待标志位的变化（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     * @param old 期望的旧值。
     * @param order 等待操作的内存顺序（默认为 memory_order::seq_cst）。
     */
    RAINY_INLINE void atomic_flag_wait(const volatile atomic_flag *flag, bool old,
                                       memory_order order = memory_order::seq_cst) noexcept {
        flag->wait(old, order);
    }

    /**
     * \lang english
     * @brief Notifies one waiting thread.
     *
     * @param flag The `atomic_flag` object to operate on.
     *
     * \lang simp-chinese
     * @brief 通知一个等待的线程
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_one(atomic_flag *flag) noexcept {
        flag->notify_one();
    }

    /**
     * \lang english
     * @brief Notifies one waiting thread (volatile-qualified).
     *
     * @param flag The `volatile atomic_flag` object to operate on.
     *
     * \lang simp-chinese
     * @brief 通知一个等待的线程（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_one(volatile atomic_flag *flag) noexcept {
        flag->notify_one();
    }

    /**
     * \lang english
     * @brief Notifies all waiting threads.
     *
     * @param flag The `atomic_flag` object to operate on.
     *
     * \lang simp-chinese
     * @brief 通知所有等待的线程
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_all(atomic_flag *flag) noexcept {
        flag->notify_all();
    }

    /**
     * \lang english
     * @brief Notifies all waiting threads (volatile-qualified).
     *
     * @param flag The `volatile atomic_flag` object to operate on.
     *
     * \lang simp-chinese
     * @brief 通知所有等待的线程（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_all(volatile atomic_flag *flag) noexcept {
        flag->notify_all();
    }
}

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief A scoped fence block used to insert an acquire/release fence pair
     *        around a region of code.
     *
     *  Constructing a full_t fenced_block issues an acquire fence; destroying any
     *  fenced_block issues a release fence. A half_t fenced_block only performs the
     *  release fence on destruction.
     *
     * \lang simp-chinese
     * @brief 作用域栅栏块，用于在一段代码周围插入一对 acquire/release 栅栏。
     *
     *  构造 full_t 的 fenced_block 会发出 acquire 栅栏；销毁任意 fenced_block
     *  都会发出 release 栅栏。half_t 的 fenced_block 仅在析构时执行 release 栅栏。
     */
    class fenced_block : type_traits::helper::non_copyable {
    public:
        // NOLINTBEGIN
        /**
         * \lang english
         * @brief Tag type selecting the half (release-only) fence behavior.
         *
         * \lang simp-chinese
         * @brief 选择半（仅 release）栅栏行为的标签类型。
         */
        enum half_t {
            /**
             * \lang english
             * @brief Tag value for the half fence behavior.
             *
             * \lang simp-chinese
             * @brief 半栅栏行为的标签值。
             */
            half
        };

        /**
         * \lang english
         * @brief Tag type selecting the full (acquire + release) fence behavior.
         *
         * \lang simp-chinese
         * @brief 选择完整（acquire + release）栅栏行为的标签类型。
         */
        enum full_t {
            /**
             * \lang english
             * @brief Tag value for the full fence behavior.
             *
             * \lang simp-chinese
             * @brief 完整栅栏行为的标签值。
             */
            full
        };
        // NOLINTEND

        /**
         * \lang english
         * @brief Constructs a half fenced_block. No fence is issued on construction;
         *        the release fence is issued on destruction.
         *
         * @param <unnamed> Tag value of type half_t.
         *
         * \lang simp-chinese
         * @brief 构造半栅栏 fenced_block。构造时不发出栅栏；
         *        在析构时发出 release 栅栏。
         *
         * @param <unnamed> 类型为 half_t 的标签值。
         */
        explicit fenced_block(half_t) {
        }

        /**
         * \lang english
         * @brief Constructs a full fenced_block. Issues an acquire fence on
         *        construction; the release fence is issued on destruction.
         *
         * @param <unnamed> Tag value of type full_t.
         *
         * \lang simp-chinese
         * @brief 构造完整栅栏 fenced_block。构造时发出 acquire 栅栏；
         *        在析构时发出 release 栅栏。
         *
         * @param <unnamed> 类型为 full_t 的标签值。
         */
        explicit fenced_block(full_t) {
            concurrency::atomic_thread_fence(memory_order_acquire);
        }

        /**
         * \lang english
         * @brief Destroys the fenced_block and issues a release fence.
         *
         * \lang simp-chinese
         * @brief 销毁 fenced_block 并发出 release 栅栏。
         */
        ~fenced_block() {
            concurrency::atomic_thread_fence(memory_order_release);
        }
    };
}

#endif
