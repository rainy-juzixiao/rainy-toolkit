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
#include <rainy/core/concurrency/atomicinfra/atomic_base.hpp>

namespace rainy::core::concurrency::implements {
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

        rain_fn is_lock_free() const noexcept -> bool {
            return true;
        }

        explicit atomic_ref_base(Ty &obj) noexcept : ptr_(utility::addressof(obj)) {
        }

        atomic_ref_base(const atomic_ref_base &other) noexcept : ptr_(other.ptr_) {
        }

        atomic_ref_base &operator=(const atomic_ref_base &) = delete;

        rain_fn store(Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> void {
            Ops::store(ptr(), desired, order);
        }

        rain_fn operator=(Ty desired) const noexcept -> Ty {
            store(desired);
            return desired;
        }

        rain_fn load(memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return Ops::load(ptr(), order);
        }

        operator Ty() const noexcept {
            return load();
        }

        rain_fn exchange(Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return Ops::exch(ptr(), desired, order);
        }

        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        rain_fn compare_exchange_weak(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> bool {
            return compare_exchange_weak(expected, desired, order, order);
        }

        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order success, memory_order failure) const noexcept -> bool {
            return Ops::cas(ptr(), expected, desired, success, failure);
        }

        rain_fn compare_exchange_strong(Ty &expected, Ty desired, memory_order order = memory_order::seq_cst) const noexcept -> bool {
            return compare_exchange_strong(expected, desired, order, order);
        }

        rain_fn wait(Ty old, memory_order order = memory_order::seq_cst) const noexcept -> void {
            const volatile Ty *addr = const_cast<const volatile Ty *>(ptr_);
            atomic_wait_impl(addr, old, order);
        }

        rain_fn notify_one() const noexcept -> void {
            atomic_notify_one_impl(ptr());
        }

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

namespace rainy::core::concurrency::implements {
    template <typename Ty>
    class atomic_ref_integral : public implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>>;
        using ops = implements::atomic_ops<Ty>;

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        rain_fn fetch_add(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::add(this->ptr(), arg, order);
        }

        rain_fn fetch_sub(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::sub(this->ptr(), arg, order);
        }

        rain_fn fetch_and(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::band(this->ptr(), arg, order);
        }

        rain_fn fetch_or(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::bor(this->ptr(), arg, order);
        }

        rain_fn fetch_xor(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return ops::bxor(this->ptr(), arg, order);
        }

        rain_fn fetch_max(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return implements::atomic_fetch_max<Ty, ops>(this->ptr(), arg, order);
        }

        rain_fn fetch_min(Ty arg, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            return implements::atomic_fetch_min<Ty, ops>(this->ptr(), arg, order);
        }

        rain_fn operator++(int) const noexcept -> Ty {
            return fetch_add(Ty{1});
        }

        rain_fn operator--(int) const noexcept -> Ty {
            return fetch_sub(Ty{1});
        }

        rain_fn operator++() const noexcept -> Ty {
            return ops::inc(this->ptr(), memory_order::seq_cst);
        }

        rain_fn operator--() const noexcept -> Ty {
            return ops::dec(this->ptr(), memory_order::seq_cst);
        }

        rain_fn operator+=(Ty arg) const noexcept -> Ty {
            return fetch_add(arg) + arg;
        }

        rain_fn operator-=(Ty arg) const noexcept -> Ty {
            return fetch_sub(arg) - arg;
        }

        rain_fn operator&=(Ty arg) const noexcept -> Ty {
            return fetch_and(arg) & arg;
        }

        rain_fn operator|=(Ty arg) const noexcept -> Ty {
            return fetch_or(arg) | arg;
        }

        rain_fn operator^=(Ty arg) const noexcept -> Ty {
            return fetch_xor(arg) ^ arg;
        }
    };
}

namespace rainy::core::concurrency::implements {
    template <typename Ty>
    class atomic_ref_floating : public implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>> {
    public:
        using base = implements::atomic_ref_base<Ty, implements::atomic_ops<Ty>>;

        static_assert(!type_traits::properties::is_const_v<Ty>);
        static_assert(!type_traits::properties::is_volatile_v<Ty>);

        using value_type = Ty;
        using difference_type = Ty;

        using base::base;

        rain_fn fetch_add(Ty operand, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        rain_fn fetch_sub(Ty operand, memory_order order = memory_order::seq_cst) const noexcept -> Ty {
            Ty temp = this->load(memory_order::relaxed);
            while (!this->compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        rain_fn operator+=(Ty operand) const noexcept -> Ty {
            return fetch_add(operand) + operand;
        }

        rain_fn operator-=(Ty operand) const noexcept -> Ty {
            return fetch_sub(operand) - operand;
        }
    };
}

namespace rainy::core::concurrency::implements {
    template <typename Ty>
    class atomic_ref_pointer : public implements::atomic_ref_base<Ty *, implements::atomic_ops<Ty *>> {
    public:
        using base = implements::atomic_ref_base<Ty *, implements::atomic_ops<Ty *>>;
        using ops = implements::atomic_ops<Ty *>;

        static_assert(!type_traits::primary_types::is_function_v<Ty>);

        using value_type = Ty *;
        using difference_type = std::ptrdiff_t;

        using base::base;

        rain_fn fetch_add(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return ops::ptr_add(this->ptr(), n, order);
        }

        rain_fn fetch_sub(std::ptrdiff_t n, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return ops::ptr_sub(this->ptr(), n, order);
        }

        rain_fn fetch_max(Ty *val, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return implements::atomic_ptr_fetch_max(this->ptr(), val, order);
        }

        rain_fn fetch_min(Ty *val, memory_order order = memory_order::seq_cst) const noexcept -> Ty * {
            return implements::atomic_ptr_fetch_min(this->ptr(), val, order);
        }

        rain_fn operator++(int) const noexcept -> Ty * {
            return fetch_add(1);
        }

        rain_fn operator--(int) const noexcept -> Ty * {
            return fetch_sub(1);
        }

        rain_fn operator++() const noexcept -> Ty * {
            return fetch_add(1) + 1;
        }

        rain_fn operator--() const noexcept -> Ty * {
            return fetch_sub(1) - 1;
        }

        rain_fn operator+=(std::ptrdiff_t n) const noexcept -> Ty * {
            return fetch_add(n) + n;
        }

        rain_fn operator-=(std::ptrdiff_t n) const noexcept -> Ty * {
            return fetch_sub(n) - n;
        }
    };
}

namespace rainy::core::concurrency::implements {
    template <typename TVal, typename Ty>
    struct atomic_ref_base_selector {
        using type = type_traits::other_trans::conditional_t<
            type_traits::type_relations::is_same_v<Ty, bool>, atomic_ref_base<bool, atomic_ops<bool>>,
            type_traits::other_trans::conditional_t<
                type_traits::primary_types::is_integral_v<Ty>, atomic_ref_integral<Ty>,
                type_traits::other_trans::conditional_t<
                    type_traits::primary_types::is_floating_point_v<Ty>, atomic_ref_floating<Ty>,
                    type_traits::other_trans::conditional_t<type_traits::primary_types::is_pointer_v<Ty>,
                                                            atomic_ref_pointer<type_traits::modifers::remove_pointer_t<Ty>>,
                                                            atomic_ref_base<Ty, atomic_ops<Ty>>>>>>;

        static_assert(type_traits::properties::is_trivially_copyable_v<Ty>,
                      "atomic_ref<Ty>: Ty must be TriviallyCopyable (ISO C++ [atomics.types.generic])");
    };

    template <typename TVal, typename Ty = TVal>
    using select_atomic_ref_base_t = typename implements::atomic_ref_base_selector<TVal, Ty>::type;
}

#endif