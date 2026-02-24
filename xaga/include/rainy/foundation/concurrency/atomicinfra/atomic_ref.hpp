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
#ifndef RAINY_FOUNDATION_CONCURRENCY_ATOMICINFRA_ATOMIC_REF_HPP
#define RAINY_FOUNDATION_CONCURRENCY_ATOMICINFRA_ATOMIC_REF_HPP
#include <rainy/foundation/concurrency/atomicinfra/atomic_base.hpp>

namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    struct atomic_ref_alignment {
        static constexpr std::size_t value = alignof(Ty);
    };

    template <typename Ty, typename Ops>
    class atomic_ref_base {
    public:
        using value_type = Ty;

        static constexpr std::size_t required_alignment = atomic_ref_alignment<Ty>::value;
        static constexpr bool is_always_lock_free = true;

        /**
         * @brief 判断当前对象是否是无锁的
         *
         * @return 返回 `true` 表示当前对象是无锁的。
         */
        rain_fn is_lock_free() const noexcept -> bool {
            return true;
        }

        /**
         * @brief 构造一个对给定对象的引用进行原子操作的基础对象
         *
         * @param obj 需要引用的对象。
         */
        explicit atomic_ref_base(Ty &obj) noexcept : ptr_(utility::addressof(obj)) {
        }

        /**
         * @brief 拷贝构造函数
         *
         * @param other 另一个 `atomic_ref_base` 对象。
         */
        atomic_ref_base(const atomic_ref_base &other) noexcept : ptr_(other.ptr_) {
        }

        /**
         * @brief 禁止拷贝赋值
         *
         * 该函数会被删除，禁止赋值操作。
         */
        atomic_ref_base &operator=(const atomic_ref_base &) = delete;

        /**
         * @brief 存储给定的值
         *
         * @param desired 要存储的值。
         * @param order 存储操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> void {
            Ops::store(ptr(), desired, order);
        }

        /**
         * @brief 赋值操作符
         *
         * 通过 `store` 函数存储值，并返回原始值。
         *
         * @param desired 要存储的值。
         * @return 返回存储前的值。
         */
        rain_fn operator=(Ty desired) const noexcept -> Ty {
            store(desired);
            return desired;
        }

        /**
         * @brief 加载当前值
         *
         * @param order 加载操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加载的值。
         */
        rain_fn load(memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return Ops::load(ptr(), order);
        }

        /**
         * @brief 转换为 `Ty` 类型
         *
         * @return 返回当前对象的值。
         */
        operator Ty() const noexcept {
            return load();
        }

        /**
         * @brief 执行交换操作
         *
         * @param desired 目标值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return Ops::exch(ptr(), desired, order);
        }

        /**
         * @brief 弱比较交换操作
         *
         * @param expected 预期的值。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 如果交换成功返回 `true`，否则返回 `false`。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        /**
         * @brief 弱比较交换操作（简化版本）
         *
         * @param expected 预期的值。
         * @param desired 目标值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果交换成功返回 `true`，否则返回 `false`。
         */
        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> bool {
            return compare_exchange_weak(expected, desired, order, order);
        }

        /**
         * @brief 强比较交换操作
         *
         * @param expected 预期的值。
         * @param desired 目标值。
         * @param success 成功时的内存顺序。
         * @param failure 失败时的内存顺序。
         * @return 如果交换成功返回 `true`，否则返回 `false`。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        /**
         * @brief 强比较交换操作（简化版本）
         *
         * @param expected 预期的值。
         * @param desired 目标值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果交换成功返回 `true`，否则返回 `false`。
         */
        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> bool {
            return compare_exchange_strong(expected, desired, order, order);
        }

        /**
         * @brief 等待值发生变化
         *
         * @param old 当前值。
         * @param order 等待的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept -> void {
            const volatile Ty *addr = const_cast<const volatile Ty *>(ptr_);
            atomic_wait_impl(addr, old, order);
        }

        /**
         * @brief 通知一个等待的线程
         */
        rain_fn notify_one() const noexcept -> void {
            atomic_notify_one_impl(ptr());
        }

        /**
         * @brief 通知所有等待的线程
         */
        rain_fn notify_all() const noexcept -> void {
            atomic_notify_all_impl(ptr());
        }

    protected:
        volatile Ty *ptr() const noexcept {
            return ptr_;
        }

    private:
        Ty *ptr_;
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    class atomic_ref_integral : public implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>>;
        using ops = implements::atomic_ops<Ty>;

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        /**
         * @brief 执行加法操作
         *
         * @param arg 要加的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作后的值。
         */
        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::add(this->ptr(), arg, order);
        }

        /**
         * @brief 执行减法操作
         *
         * @param arg 要减的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作后的值。
         */
        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::sub(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位与操作
         *
         * @param arg 与操作数。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位与操作后的值。
         */
        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::band(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位或操作
         *
         * @param arg 或操作数。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位或操作后的值。
         */
        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::bor(this->ptr(), arg, order);
        }

        /**
         * @brief 执行按位异或操作
         *
         * @param arg 异或操作数。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回按位异或操作后的值。
         */
        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::bxor(this->ptr(), arg, order);
        }

        /**
         * @brief 执行最大值比较并交换操作
         *
         * @param arg 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        /**
         * @brief 执行最小值比较并交换操作
         *
         * @param arg 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的值。
         */
        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        /**
         * @brief 后置自增操作
         *
         * @return 返回自增前的值。
         */
        rain_fn operator++(int) const noexcept -> Ty {
            return fetch_add(Ty{1});
        }

        /**
         * @brief 后置自减操作
         *
         * @return 返回自减前的值。
         */
        rain_fn operator--(int) const noexcept -> Ty {
            return fetch_sub(Ty{1});
        }

        /**
         * @brief 前置自增操作
         *
         * @return 返回自增后的值。
         */
        rain_fn operator++() const noexcept -> Ty {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        /**
         * @brief 前置自减操作
         *
         * @return 返回自减后的值。
         */
        rain_fn operator--() const noexcept -> Ty {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        /**
         * @brief 执行加法并赋值操作
         *
         * @param arg 增加的值。
         * @return 返回加法操作后的值。
         */
        rain_fn operator+=(Ty arg) const noexcept -> Ty {
            return fetch_add(arg) + arg;
        }

        /**
         * @brief 执行减法并赋值操作
         *
         * @param arg 减少的值。
         * @return 返回减法操作后的值。
         */
        rain_fn operator-=(Ty arg) const noexcept -> Ty {
            return fetch_sub(arg) - arg;
        }

        /**
         * @brief 执行按位与并赋值操作
         *
         * @param arg 与操作数。
         * @return 返回按位与操作后的值。
         */
        rain_fn operator&=(Ty arg) const noexcept -> Ty {
            return fetch_and(arg) & arg;
        }

        /**
         * @brief 执行按位或并赋值操作
         *
         * @param arg 或操作数。
         * @return 返回按位或操作后的值。
         */
        rain_fn operator|=(Ty arg) const noexcept -> Ty {
            return fetch_or(arg) | arg;
        }

        /**
         * @brief 执行按位异或并赋值操作
         *
         * @param arg 异或操作数。
         * @return 返回按位异或操作后的值。
         */
        rain_fn operator^=(Ty arg) const noexcept -> Ty {
            return fetch_xor(arg) ^ arg;
        }
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    class atomic_ref_floating : public implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>>;

        static_assert(!std::is_const_v<Ty>);
        static_assert(!std::is_volatile_v<Ty>);

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        /**
         * @brief 执行加法操作
         *
         * @param operand 要加的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作前的值。
         */
        rain_fn fetch_add(Ty operand, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        /**
         * @brief 执行减法操作
         *
         * @param operand 要减的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作前的值。
         */
        rain_fn fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        /**
         * @brief 执行加法并赋值操作
         *
         * @param operand 要加的值。
         * @return 返回加法后的结果。
         */
        rain_fn operator+=(Ty operand) const noexcept -> Ty {
            return fetch_add(operand) + operand;
        }

        /**
         * @brief 执行减法并赋值操作
         *
         * @param operand 要减的值。
         * @return 返回减法后的结果。
         */
        rain_fn operator-=(Ty operand) const noexcept -> Ty {
            return fetch_sub(operand) - operand;
        }
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename Ty>
    class atomic_ref_pointer : public implements::atomic_ref_base<Ty *, implements::atomic_ops<Ty *>> {
    public:
        using base = implements::atomic_ref_base<Ty *, implements::atomic_ops<Ty *>>;
        using ops = implements::atomic_ops<Ty *>;

        static_assert(!std::is_function_v<Ty>);

        using value_type = Ty *;
        using difference_type = std::ptrdiff_t;

        using base::base;

        /**
         * @brief 执行指针加法操作
         *
         * @param n 要加的偏移量。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回加法操作后的指针。
         */
        rain_fn fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return ops::ptr_add(this->ptr(), n, order);
        }

        /**
         * @brief 执行指针减法操作
         *
         * @param n 要减的偏移量。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回减法操作后的指针。
         */
        rain_fn fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return ops::ptr_sub(this->ptr(), n, order);
        }

        /**
         * @brief 执行最大值比较并交换操作
         *
         * @param val 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的指针。
         */
        rain_fn fetch_max(Ty *val, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return implements::atomic_ptr_fetch_max(this->ptr(), val, order);
        }

        /**
         * @brief 执行最小值比较并交换操作
         *
         * @param val 比较的值。
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 返回交换前的指针。
         */
        rain_fn fetch_min(Ty *val, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return implements::atomic_ptr_fetch_min(this->ptr(), val, order);
        }

        /**
         * @brief 后置自增操作
         *
         * @return 返回自增前的指针。
         */
        rain_fn operator++(int) const noexcept -> Ty * {
            return fetch_add(1);
        }

        /**
         * @brief 后置自减操作
         *
         * @return 返回自减前的指针。
         */
        rain_fn operator--(int) const noexcept -> Ty * {
            return fetch_sub(1);
        }

        /**
         * @brief 前置自增操作
         *
         * @return 返回自增后的指针。
         */
        rain_fn operator++() const noexcept -> Ty * {
            return fetch_add(1) + 1;
        }

        /**
         * @brief 前置自减操作
         *
         * @return 返回自减后的指针。
         */
        rain_fn operator--() const noexcept -> Ty * {
            return fetch_sub(1) - 1;
        }

        /**
         * @brief 执行加法并赋值操作
         *
         * @param n 增加的偏移量。
         * @return 返回加法后的指针。
         */
        rain_fn operator+=(std::ptrdiff_t n) const noexcept -> Ty * {
            return fetch_add(n) + n;
        }

        /**
         * @brief 执行减法并赋值操作
         *
         * @param n 减少的偏移量。
         * @return 返回减法后的指针。
         */
        rain_fn operator-=(std::ptrdiff_t n) const noexcept -> Ty * {
            return fetch_sub(n) - n;
        }
    };
}

namespace rainy::foundation::concurrency::implements {
    template <typename TVal, typename Ty>
    struct atomic_ref_base_selector {
    private:
        using type = std::conditional_t<
            // 1. bool 独立路由：只有基础操作，无算术/位运算
            //    直接使用 atomic_base，不进入 atomic_integral
            std::is_same_v<Ty, bool>, atomic_ref_base<bool, atomic_ops<bool>>,
            std::conditional_t<
                // 2. 整数（已排除 bool）
                std::is_integral_v<Ty>, atomic_ref_integral<Ty>,
                std::conditional_t<
                    // 3. 浮点
                    std::is_floating_point_v<Ty>, atomic_ref_floating<Ty>,
                    std::conditional_t<
                        // 4. 指针
                        std::is_pointer_v<Ty>, atomic_ref_pointer<std::remove_pointer_t<Ty>>,
                        // 5. 通用回退（用户自定义 trivially copyable 类型）
                        atomic_ref_base<Ty, atomic_ops<Ty>>>>>>;

    public:
        static_assert(std::is_trivially_copyable_v<Ty>, "atomic<Ty>: Ty must be TriviallyCopyable (ISO C++ [atomics.types.generic])");

        using result = type;
    };

    template <typename TVal, typename Ty = TVal>
    using select_atomic_ref_base_t = typename implements::atomic_ref_base_selector<TVal, Ty>::result;
}

#endif
