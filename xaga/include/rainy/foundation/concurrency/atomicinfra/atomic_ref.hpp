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

        bool is_lock_free() const noexcept {
            return true;
        }

        explicit atomic_ref_base(Ty &obj) noexcept : ptr_(utility::addressof(obj)) {
        }

        atomic_ref_base(const atomic_ref_base &other) noexcept : ptr_(other.ptr_) {
        }

        atomic_ref_base &operator=(const atomic_ref_base &) = delete;

        void store(Ty desired, memory_order order = memory_order::seq_cst) const noexcept {
            Ops::store(ptr(), desired, order);
        }

        Ty operator=(Ty desired) const noexcept {
            store(desired);
            return desired;
        }

        Ty load(memory_order order = memory_order::seq_cst) const noexcept {
            return Ops::load(ptr(), order);
        }

        operator Ty() const noexcept {
            return load();
        }

        Ty exchange(Ty desired, memory_order order = memory_order::seq_cst) const noexcept {
            return Ops::exch(ptr(), desired, order);
        }

        bool compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        bool compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept {
            return compare_exchange_weak(expected, desired, order, order);
        }

        bool compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        bool compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept {
            return compare_exchange_strong(expected, desired, order, order);
        }

        void wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept {
            const volatile Ty *addr = const_cast<const volatile Ty *>(ptr_);
            atomic_wait_impl(addr, old, order);
        }

        void notify_one() const noexcept {
            atomic_notify_one_impl(ptr());
        }

        void notify_all() const noexcept {
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

        Ty fetch_add(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::add(this->ptr(), arg, order);
        }

        Ty fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::sub(this->ptr(), arg, order);
        }

        Ty fetch_and(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::band(this->ptr(), arg, order);
        }
        Ty fetch_or(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::bor(this->ptr(), arg, order);
        }
        Ty fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::bxor(this->ptr(), arg, order);
        }

        // ---- fetch_max / fetch_min（CAS 循环）----
        Ty fetch_max(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }
        Ty fetch_min(Ty arg, memory_order order = memory_order::seq_cst) const noexcept {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        // ---- operator++ / operator-- ----
        Ty operator++(int) const noexcept {
            return fetch_add(Ty{1});
        }
        Ty operator--(int) const noexcept {
            return fetch_sub(Ty{1});
        }
        Ty operator++() const noexcept {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }
        Ty operator--() const noexcept {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        // ---- 复合赋值 ----
        Ty operator+=(Ty arg) const noexcept {
            return fetch_add(arg) + arg;
        }
        Ty operator-=(Ty arg) const noexcept {
            return fetch_sub(arg) - arg;
        }
        Ty operator&=(Ty arg) const noexcept {
            return fetch_and(arg) & arg;
        }
        Ty operator|=(Ty arg) const noexcept {
            return fetch_or(arg) | arg;
        }
        Ty operator^=(Ty arg) const noexcept {
            return fetch_xor(arg) ^ arg;
        }
    };

    template <typename Ty>
    class atomic_ref_floating : public implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>>;

        static_assert(!std::is_const_v<Ty>);
        static_assert(!std::is_volatile_v<Ty>);

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        // ---- fetch_add / fetch_sub（CAS 循环）----
        Ty fetch_add(Ty operand, memory_order order = memory_order::seq_cst) const noexcept {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }
        Ty fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) const noexcept {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        // ---- 复合赋值 ----
        Ty operator+=(Ty operand) const noexcept {
            return fetch_add(operand) + operand;
        }
        Ty operator-=(Ty operand) const noexcept {
            return fetch_sub(operand) - operand;
        }
    };

    template <typename Ty>
    class atomic_ref_pointer : public implements::atomic_ref_base<Ty *, implements::atomic_ops<Ty *>> {
    public:
        using base = implements::atomic_ref_base<Ty *, implements::atomic_ops<Ty *>>;
        using ops = implements::atomic_ops<Ty *>;

        static_assert(!std::is_function_v<Ty>);

        using value_type = Ty *;
        using difference_type = std::ptrdiff_t;

        using base::base;

        // ---- fetch_add / fetch_sub ----
        Ty *fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::ptr_add(this->ptr(), n, order);
        }
        Ty *fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) const noexcept {
            return ops::ptr_sub(this->ptr(), n, order);
        }

        // ---- fetch_max / fetch_min ----
        Ty *fetch_max(Ty *val, memory_order order = memory_order::seq_cst) const noexcept {
            return implements::atomic_ptr_fetch_max(this->ptr(), val, order);
        }
        Ty *fetch_min(Ty *val, memory_order order = memory_order::seq_cst) const noexcept {
            return implements::atomic_ptr_fetch_min(this->ptr(), val, order);
        }

        // ---- operator++ / operator-- ----
        Ty *operator++(int) const noexcept {
            return fetch_add(1);
        }
        Ty *operator--(int) const noexcept {
            return fetch_sub(1);
        }
        Ty *operator++() const noexcept {
            return fetch_add(1) + 1;
        }
        Ty *operator--() const noexcept {
            return fetch_sub(1) - 1;
        }

        // ---- 复合赋值 ----
        Ty *operator+=(std::ptrdiff_t n) const noexcept {
            return fetch_add(n) + n;
        }
        Ty *operator-=(std::ptrdiff_t n) const noexcept {
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
