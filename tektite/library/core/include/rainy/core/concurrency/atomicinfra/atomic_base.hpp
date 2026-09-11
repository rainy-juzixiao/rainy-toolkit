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
#include <rainy/core/concurrency/atomicinfra/atomic_ops.hpp>
#include <rainy/core/type_traits/primary_types.hpp>
#include <rainy/core/type_traits/properties.hpp>

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::core::concurrency::implements {
    template <typename Ty, typename Ops>
    class atomic_base {
    public:
        using value_type = Ty;

        static constexpr bool is_always_lock_free = true;

        atomic_base() noexcept(type_traits::properties::is_nothrow_default_constructible_v<Ty>) : storage_(Ty{}) {
        }

        atomic_base(Ty desired) noexcept : storage_(desired) {
        }

        rain_fn is_lock_free() const noexcept -> bool {
            return true;
        }

        rain_fn is_lock_free() const volatile noexcept -> bool {
            return true;
        }

        atomic_base(const atomic_base &) = delete;
        atomic_base &operator=(const atomic_base &) = delete;
        atomic_base &operator=(const atomic_base &) volatile = delete;

        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) noexcept -> void {
            Ops::store(ptr(), desired, order);
        }

        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> void {
            Ops::store(ptr(), desired, order);
        }

        rain_fn operator=(Ty desired) noexcept -> Ty {
            store(desired);
            return desired;
        }

        rain_fn operator=(Ty desired) volatile noexcept -> Ty {
            store(desired);
            return desired;
        }

        rain_fn load(memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return Ops::load(ptr(), order);
        }

        rain_fn load(memory_order order = memory_order::seq_cst) const volatile noexcept -> Ty {
            return Ops::load(ptr(), order);
        }

        operator Ty() const noexcept {
            return load();
        }

        operator Ty() const volatile noexcept {
            return load();
        }

        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return Ops::exch(ptr(), desired, order);
        }

        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return Ops::exch(ptr(), desired, order);
        }

        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept -> bool {
            return compare_exchange_weak(expected, desired, order, order);
        }

        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept -> bool {
            return compare_exchange_weak(expected, desired, order, order);
        }

        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) volatile noexcept
            -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) noexcept -> bool {
            return compare_exchange_strong(expected, desired, order, order);
        }

        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) volatile noexcept
            -> bool {
            return compare_exchange_strong(expected, desired, order, order);
        }

        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept -> void {
            atomic_wait_impl(ptr(), old, order);
        }

        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const volatile noexcept -> void {
            atomic_wait_impl(ptr(), old, order);
        }

        rain_fn notify_one() noexcept -> void {
            atomic_notify_one_impl(ptr());
        }

        rain_fn notify_one() volatile noexcept -> void {
            atomic_notify_one_impl(ptr());
        }

        rain_fn notify_all() noexcept -> void {
            atomic_notify_all_impl(ptr());
        }

        rain_fn notify_all() volatile noexcept -> void {
            atomic_notify_all_impl(ptr());
        }

    protected:
        volatile Ty *ptr() noexcept {
            return &storage_;
        }

        volatile Ty *ptr() volatile noexcept {
            return &storage_;
        }

        const volatile Ty *ptr() const noexcept {
            return &storage_;
        }

        const volatile Ty *ptr() const volatile noexcept {
            return &storage_;
        }

    private:
        alignas(sizeof(Ty)) volatile Ty storage_;
    };
}

namespace rainy::core::concurrency::implements {
    template <typename Ty>
    class atomic_integral : public implements::atomic_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_base<Ty, implements::atomic_ops<Ty>>;
        using ops = implements::atomic_ops<Ty>;

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        using base::operator=;

        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::add(this->ptr(), arg, order);
        }

        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::add(this->ptr(), arg, order);
        }

        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::sub(this->ptr(), arg, order);
        }

        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::sub(this->ptr(), arg, order);
        }

        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::band(this->ptr(), arg, order);
        }

        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::band(this->ptr(), arg, order);
        }

        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::bor(this->ptr(), arg, order);
        }

        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::bor(this->ptr(), arg, order);
        }

        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return ops::bxor(this->ptr(), arg, order);
        }

        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return ops::bxor(this->ptr(), arg, order);
        }

        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        rain_fn operator++(int) noexcept -> Ty {
            return fetch_add(Ty{1});
        }

        rain_fn operator++(int) volatile noexcept -> Ty {
            return fetch_add(Ty{1});
        }

        rain_fn operator--(int) noexcept -> Ty {
            return fetch_sub(Ty{1});
        }

        rain_fn operator--(int) volatile noexcept -> Ty {
            return fetch_sub(Ty{1});
        }

        rain_fn operator++() noexcept -> Ty {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        rain_fn operator++() volatile noexcept -> Ty {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        rain_fn operator--() noexcept -> Ty {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        rain_fn operator--() volatile noexcept -> Ty {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        rain_fn operator+=(Ty arg) noexcept -> Ty {
            return fetch_add(arg) + arg;
        }

        rain_fn operator+=(Ty arg) volatile noexcept -> Ty {
            return fetch_add(arg) + arg;
        }

        rain_fn operator-=(Ty arg) noexcept -> Ty {
            return fetch_sub(arg) - arg;
        }

        rain_fn operator-=(Ty arg) volatile noexcept -> Ty {
            return fetch_sub(arg) - arg;
        }

        rain_fn operator&=(Ty arg) noexcept -> Ty {
            return fetch_and(arg) & arg;
        }

        rain_fn operator&=(Ty arg) volatile noexcept -> Ty {
            return fetch_and(arg) & arg;
        }

        rain_fn operator|=(Ty arg) noexcept -> Ty {
            return fetch_or(arg) | arg;
        }

        rain_fn operator|=(Ty arg) volatile noexcept -> Ty {
            return fetch_or(arg) | arg;
        }

        rain_fn operator^=(Ty arg) noexcept -> Ty {
            return fetch_xor(arg) ^ arg;
        }

        rain_fn operator^=(Ty arg) volatile noexcept -> Ty {
            return fetch_xor(arg) ^ arg;
        }
    };
}

namespace rainy::core::concurrency::implements {
    template <typename Ty>
    class atomic_floating : public implements::atomic_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_base<Ty, implements::atomic_ops<Ty>>;
        using ops = implements::atomic_ops<Ty>;

        static_assert(!type_traits::properties::is_const_v<Ty>, "atomic_floating<Ty>: Ty must not be const");
        static_assert(!type_traits::properties::is_volatile_v<Ty>, "atomic_floating<Ty>: Ty must not be volatile");

        using value_type = Ty;
        using difference_type = Ty;

        using base::operator=;

        using base::base;

        rain_fn fetch_add(Ty operand, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        rain_fn fetch_add(Ty operand, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_add(operand, order);
        }

        rain_fn fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        rain_fn fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand, order);
        }

        rain_fn operator+=(Ty operand) noexcept -> Ty {
            return fetch_add(operand) + operand;
        }

        rain_fn operator+=(Ty operand) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_add(operand) + operand;
        }

        rain_fn operator-=(Ty operand) noexcept -> Ty {
            return fetch_sub(operand) - operand;
        }

        rain_fn operator-=(Ty operand) volatile noexcept -> Ty {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand) - operand;
        }
    };
}

namespace rainy::core::concurrency::implements {
    template <typename Ty>
    class atomic_pointer : public implements::atomic_base<Ty *, implements::atomic_ops<Ty *>> {
    public:
        using base = implements::atomic_base<Ty *, implements::atomic_ops<Ty *>>;
        using ops = implements::atomic_ops<Ty *>;

        static_assert(!type_traits::primary_types::is_function_v<Ty>, "atomic_pointer<Ty>: Ty must not be a function type");

        using value_type = Ty *;
        using difference_type = std::ptrdiff_t;

        using base::operator=;

        using base::base;

        rain_fn fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return ops::ptr_add(this->ptr(), n, order);
        }

        rain_fn fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_add(n, order);
        }

        rain_fn fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return ops::ptr_sub(this->ptr(), n, order);
        }

        rain_fn fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_sub(n, order);
        }

        rain_fn fetch_max(Ty *val, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return implements::atomic_ptr_fetch_max(this->ptr(), val, order);
        }

        rain_fn fetch_max(Ty *val, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_max(val, order);
        }

        rain_fn fetch_min(Ty *val, memory_order order = memory_order::seq_cst) noexcept -> Ty * {
            return implements::atomic_ptr_fetch_min(this->ptr(), val, order);
        }

        rain_fn fetch_min(Ty *val, memory_order order = memory_order::seq_cst) volatile noexcept -> Ty * {
            return const_cast<atomic_pointer *>(this)->fetch_min(val, order);
        }

        rain_fn operator++(int) noexcept -> Ty * {
            return fetch_add(1);
        }

        rain_fn operator++(int) volatile noexcept -> Ty * {
            return fetch_add(1);
        }

        rain_fn operator--(int) noexcept -> Ty * {
            return fetch_sub(1);
        }

        rain_fn operator--(int) volatile noexcept -> Ty * {
            return fetch_sub(1);
        }

        rain_fn operator++() noexcept -> Ty * {
            return fetch_add(1) + 1;
        }

        rain_fn operator++() volatile noexcept -> Ty * {
            return fetch_add(1) + 1;
        }

        rain_fn operator--() noexcept -> Ty * {
            return fetch_sub(1) - 1;
        }

        rain_fn operator--() volatile noexcept -> Ty * {
            return fetch_sub(1) - 1;
        }

        rain_fn operator+=(std::ptrdiff_t n) noexcept -> Ty * {
            return fetch_add(n) + n;
        }

        rain_fn operator+=(std::ptrdiff_t n) volatile noexcept -> Ty * {
            return fetch_add(n) + n;
        }

        rain_fn operator-=(std::ptrdiff_t n) noexcept -> Ty * {
            return fetch_sub(n) - n;
        }

        rain_fn operator-=(std::ptrdiff_t n) volatile noexcept -> Ty * {
            return fetch_sub(n) - n;
        }
    };
}

namespace rainy::core::concurrency::implements {
    template <typename TVal, typename Ty>
    struct atomic_base_selector {
        using type = type_traits::other_trans::conditional_t<
            type_traits::type_relations::is_same_v<Ty, bool>, atomic_base<bool, atomic_ops<bool>>,
            type_traits::other_trans::conditional_t<
                type_traits::primary_types::is_integral_v<Ty>, atomic_integral<Ty>,
                type_traits::other_trans::conditional_t<
                    type_traits::primary_types::is_floating_point_v<Ty>, atomic_floating<Ty>,
                    type_traits::other_trans::conditional_t<type_traits::primary_types::is_pointer_v<Ty>,
                                                            atomic_pointer<type_traits::modifers::remove_pointer_t<Ty>>,
                                                            atomic_base<Ty, atomic_ops<Ty>>>>>>;

        static_assert(type_traits::properties::is_trivially_copyable_v<Ty>,
                      "atomic<Ty>: Ty must be TriviallyCopyable (ISO C++ [atomics.types.generic])");
    };

    template <typename TVal, typename Ty = TVal>
    using select_atomic_base_t = typename implements::atomic_base_selector<TVal, Ty>::type;
}

#endif

#endif