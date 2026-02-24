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

        bool is_lock_free() const noexcept {
            return true;
        }

        bool is_lock_free() const volatile noexcept {
            return true;
        }

        constexpr atomic_base() noexcept(std::is_nothrow_default_constructible_v<Ty>) : storage_(Ty{}) {
        }

        constexpr atomic_base(Ty desired) noexcept : storage_(desired) {
        }

        atomic_base(const atomic_base &) = delete;
        atomic_base &operator=(const atomic_base &) = delete;
        atomic_base &operator=(const atomic_base &) volatile = delete;

        void store(Ty desired, memory_order order = memory_order::seq_cst) noexcept {
            Ops::store(ptr(), desired, order);
        }

        void store(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept {
            Ops::store(ptr(), desired, order);
        }

        Ty operator=(Ty desired) noexcept {
            store(desired);
            return desired;
        }

        Ty operator=(Ty desired) volatile noexcept {
            store(desired);
            return desired;
        }

        Ty load(memory_order order = memory_order::seq_cst) const noexcept {
            return Ops::load(ptr(), order);
        }

        Ty load(memory_order order = memory_order::seq_cst) const volatile noexcept {
            return Ops::load(ptr(), order);
        }

        operator Ty() const noexcept {
            return load();
        }

        operator Ty() const volatile noexcept {
            return load();
        }

        Ty exchange(Ty desired, memory_order order = memory_order::seq_cst) noexcept {
            return Ops::exch(ptr(), desired, order);
        }

        Ty exchange(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept {
            return Ops::exch(ptr(), desired, order);
        }

        bool compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        bool compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        bool compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept {
            return compare_exchange_weak(expected, desired, order, order);
        }

        bool compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept {
            return compare_exchange_weak(expected, desired, order, order);
        }

        bool compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        bool compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        bool compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept {
            return compare_exchange_strong(expected, desired, order, order);
        }

        bool compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept {
            return compare_exchange_strong(expected, desired, order, order);
        }

        void wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept {
            atomic_wait_impl(ptr(), old, order);
        }

        void wait(Ty old, memory_order order = memory_order::seq_cst) const volatile noexcept {
            atomic_wait_impl(ptr(), old, order);
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

        Ty fetch_add(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return ops::add(this->ptr(), arg, order);
        }

        Ty fetch_add(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return ops::add(this->ptr(), arg, order);
        }

        Ty fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return ops::sub(this->ptr(), arg, order);
        }

        Ty fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return ops::sub(this->ptr(), arg, order);
        }

        Ty fetch_and(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return ops::band(this->ptr(), arg, order);
        }

        Ty fetch_and(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return ops::band(this->ptr(), arg, order);
        }

        Ty fetch_or(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return ops::bor(this->ptr(), arg, order);
        }

        Ty fetch_or(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return ops::bor(this->ptr(), arg, order);
        }

        Ty fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return ops::bxor(this->ptr(), arg, order);
        }

        Ty fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return ops::bxor(this->ptr(), arg, order);
        }

        Ty fetch_max(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        Ty fetch_max(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        Ty fetch_min(Ty arg, memory_order order = memory_order::seq_cst) noexcept {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        Ty fetch_min(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        Ty operator++(int) noexcept {
            return fetch_add(Ty{1});
        }

        Ty operator++(int) volatile noexcept {
            return fetch_add(Ty{1});
        }

        Ty operator--(int) noexcept {
            return fetch_sub(Ty{1});
        }

        Ty operator--(int) volatile noexcept {
            return fetch_sub(Ty{1});
        }

        Ty operator++() noexcept {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        Ty operator++() volatile noexcept {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        Ty operator--() noexcept {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        Ty operator--() volatile noexcept {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        Ty operator+=(Ty arg) noexcept {
            return fetch_add(arg) + arg;
        }

        Ty operator+=(Ty arg) volatile noexcept {
            return fetch_add(arg) + arg;
        }

        Ty operator-=(Ty arg) noexcept {
            return fetch_sub(arg) - arg;
        }

        Ty operator-=(Ty arg) volatile noexcept {
            return fetch_sub(arg) - arg;
        }

        Ty operator&=(Ty arg) noexcept {
            return fetch_and(arg) & arg;
        }

        Ty operator&=(Ty arg) volatile noexcept {
            return fetch_and(arg) & arg;
        }

        Ty operator|=(Ty arg) noexcept {
            return fetch_or(arg) | arg;
        }

        Ty operator|=(Ty arg) volatile noexcept {
            return fetch_or(arg) | arg;
        }

        Ty operator^=(Ty arg) noexcept {
            return fetch_xor(arg) ^ arg;
        }

        Ty operator^=(Ty arg) volatile noexcept {
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

        Ty fetch_add(Ty operand, memory_order order = memory_order::seq_cst) noexcept {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        Ty fetch_add(Ty operand, memory_order order = memory_order::seq_cst) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_add(operand, order);
        }

        Ty fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) noexcept {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        Ty fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand, order);
        }

        Ty operator+=(Ty operand) noexcept {
            return fetch_add(operand) + operand;
        }

        Ty operator+=(Ty operand) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_add(operand) + operand;
        }

        Ty operator-=(Ty operand) noexcept {
            return fetch_sub(operand) - operand;
        }

        Ty operator-=(Ty operand) volatile noexcept {
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

        Ty *fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) noexcept {
            return ops::ptr_add(this->ptr(), n, order);
        }
        Ty *fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) volatile noexcept {
            return const_cast<atomic_pointer *>(this)->fetch_add(n, order);
        }

        Ty *fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) noexcept {
            return ops::ptr_sub(this->ptr(), n, order);
        }
        Ty *fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) volatile noexcept {
            return const_cast<atomic_pointer *>(this)->fetch_sub(n, order);
        }

        Ty *fetch_max(Ty *val, memory_order order = memory_order::seq_cst) noexcept {
            return implements::atomic_ptr_fetch_max(this->ptr(), val, order);
        }
        Ty *fetch_max(Ty *val, memory_order order = memory_order::seq_cst) volatile noexcept {
            return const_cast<atomic_pointer *>(this)->fetch_max(val, order);
        }

        Ty *fetch_min(Ty *val, memory_order order = memory_order::seq_cst) noexcept {
            return implements::atomic_ptr_fetch_min(this->ptr(), val, order);
        }
        Ty *fetch_min(Ty *val, memory_order order = memory_order::seq_cst) volatile noexcept {
            return const_cast<atomic_pointer *>(this)->fetch_min(val, order);
        }

        Ty *operator++(int) noexcept {
            return fetch_add(1);
        }
        Ty *operator++(int) volatile noexcept {
            return fetch_add(1);
        }
        Ty *operator--(int) noexcept {
            return fetch_sub(1);
        }
        Ty *operator--(int) volatile noexcept {
            return fetch_sub(1);
        }

        Ty *operator++() noexcept {
            return fetch_add(1) + 1;
        }
        Ty *operator++() volatile noexcept {
            return fetch_add(1) + 1;
        }
        Ty *operator--() noexcept {
            return fetch_sub(1) - 1;
        }
        Ty *operator--() volatile noexcept {
            return fetch_sub(1) - 1;
        }

        Ty *operator+=(std::ptrdiff_t n) noexcept {
            return fetch_add(n) + n;
        }
        Ty *operator+=(std::ptrdiff_t n) volatile noexcept {
            return fetch_add(n) + n;
        }
        Ty *operator-=(std::ptrdiff_t n) noexcept {
            return fetch_sub(n) - n;
        }
        Ty *operator-=(std::ptrdiff_t n) volatile noexcept {
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
