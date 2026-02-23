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
#ifndef RAINY_FOUNDATION_CONCURRENCY_ATOMIC_HPP
#define RAINY_FOUNDATION_CONCURRENCY_ATOMIC_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/atomicinfra/atomic_base.hpp>
#include <rainy/foundation/concurrency/atomicinfra/atomic_ref.hpp>

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class atomic : public implements::select_atomic_base_t<Ty> {
    public:
        using implements::select_atomic_base_t<Ty>::select_atomic_base_t; // 继承构造
        using implements::select_atomic_base_t<Ty>::operator=; // 继承赋值
    };
}

namespace rainy::foundation::concurrency {
    using atomic_bool = atomic<bool>;
    using atomic_char = atomic<char>;
    using atomic_schar = atomic<signed char>;
    using atomic_uchar = atomic<unsigned char>;
    using atomic_short = atomic<short>;
    using atomic_ushort = atomic<unsigned short>;
    using atomic_int = atomic<int>;
    using atomic_uint = atomic<unsigned int>;
    using atomic_long = atomic<long>;
    using atomic_ulong = atomic<unsigned long>;
    using atomic_llong = atomic<long long>;
    using atomic_ullong = atomic<unsigned long long>;
    using atomic_char8_t = atomic<char8_t>;
    using atomic_char16_t = atomic<char16_t>;
    using atomic_char32_t = atomic<char32_t>;
    using atomic_wchar_t = atomic<wchar_t>;

    using atomic_int8_t = atomic<int8_t>;
    using atomic_uint8_t = atomic<uint8_t>;
    using atomic_int16_t = atomic<int16_t>;
    using atomic_uint16_t = atomic<uint16_t>;
    using atomic_int32_t = atomic<int32_t>;
    using atomic_uint32_t = atomic<uint32_t>;
    using atomic_int64_t = atomic<int64_t>;
    using atomic_uint64_t = atomic<uint64_t>;

    using atomic_int_least8_t = atomic<int_least8_t>;
    using atomic_uint_least8_t = atomic<uint_least8_t>;
    using atomic_int_least16_t = atomic<int_least16_t>;
    using atomic_uint_least16_t = atomic<uint_least16_t>;
    using atomic_int_least32_t = atomic<int_least32_t>;
    using atomic_uint_least32_t = atomic<uint_least32_t>;
    using atomic_int_least64_t = atomic<int_least64_t>;
    using atomic_uint_least64_t = atomic<uint_least64_t>;

    using atomic_int_fast8_t = atomic<int_fast8_t>;
    using atomic_uint_fast8_t = atomic<uint_fast8_t>;
    using atomic_int_fast16_t = atomic<int_fast16_t>;
    using atomic_uint_fast16_t = atomic<uint_fast16_t>;
    using atomic_int_fast32_t = atomic<int_fast32_t>;
    using atomic_uint_fast32_t = atomic<uint_fast32_t>;
    using atomic_int_fast64_t = atomic<int_fast64_t>;
    using atomic_uint_fast64_t = atomic<uint_fast64_t>;

    using atomic_intptr_t = atomic<intptr_t>;
    using atomic_uintptr_t = atomic<uintptr_t>;
    using atomic_size_t = atomic<size_t>;
    using atomic_ptrdiff_t = atomic<ptrdiff_t>;
    using atomic_intmax_t = atomic<intmax_t>;
    using atomic_uintmax_t = atomic<uintmax_t>;
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    class atomic_ref : public implements::select_atomic_ref_base_t<Ty> {
    public:
        using implements::select_atomic_ref_base_t<Ty>::select_atomic_ref_base_t; // 继承构造
        using implements::select_atomic_ref_base_t<Ty>::operator=; // 继承赋值
    };
}

namespace rainy::foundation::concurrency {
    template <typename Ty>
    bool atomic_is_lock_free(const volatile atomic<Ty> *obj) noexcept {
        return obj->is_lock_free();
    }
    template <typename Ty>
    bool atomic_is_lock_free(const atomic<Ty> *obj) noexcept {
        return obj->is_lock_free();
    }

    template <typename Ty>
    void atomic_store_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept {
        obj->store(desired, order);
    }

    template <typename Ty>
    void atomic_store_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept {
        obj->store(desired, order);
    }

    template <typename Ty>
    void atomic_store(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept {
        atomic_store_explicit(obj, desired, memory_order::seq_cst);
    }

    template <typename Ty>
    void atomic_store(atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept {
        atomic_store_explicit(obj, desired, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_load_explicit(const volatile atomic<Ty> *obj, memory_order order) noexcept {
        return obj->load(order);
    }
    template <typename Ty>
    Ty atomic_load_explicit(const atomic<Ty> *obj, memory_order order) noexcept {
        return obj->load(order);
    }

    template <typename Ty>
    Ty atomic_load(const volatile atomic<Ty> *obj) noexcept {
        return atomic_load_explicit(obj, memory_order::seq_cst);
    }
    
    template <typename Ty>
    Ty atomic_load(const atomic<Ty> *obj) noexcept {
        return atomic_load_explicit(obj, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_exchange_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept {
        return obj->exchange(desired, order);
    }
    template <typename Ty>
    Ty atomic_exchange_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type desired, memory_order order) noexcept {
        return obj->exchange(desired, order);
    }

    template <typename Ty>
    Ty atomic_exchange(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept {
        return atomic_exchange_explicit(obj, desired, memory_order::seq_cst);
    }
    template <typename Ty>
    Ty atomic_exchange(atomic<Ty> *obj, typename atomic<Ty>::value_type desired) noexcept {
        return atomic_exchange_explicit(obj, desired, memory_order::seq_cst);
    }

    template <typename Ty>
    bool atomic_compare_exchange_weak_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                               typename atomic<Ty>::value_type desired, memory_order success,
                                               memory_order failure) noexcept {
        return obj->compare_exchange_weak(*expected, desired, success, failure);
    }
    template <typename Ty>
    bool atomic_compare_exchange_weak_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                               typename atomic<Ty>::value_type desired, memory_order success,
                                               memory_order failure) noexcept {
        return obj->compare_exchange_weak(*expected, desired, success, failure);
    }

    template <typename Ty>
    bool atomic_compare_exchange_weak(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                      typename atomic<Ty>::value_type desired) noexcept {
        return atomic_compare_exchange_weak_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }
    template <typename Ty>
    bool atomic_compare_exchange_weak(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                      typename atomic<Ty>::value_type desired) noexcept {
        return atomic_compare_exchange_weak_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }

    template <typename Ty>
    bool atomic_compare_exchange_strong_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                                 typename atomic<Ty>::value_type desired, memory_order success,
                                                 memory_order failure) noexcept {
        return obj->compare_exchange_strong(*expected, desired, success, failure);
    }
    template <typename Ty>
    bool atomic_compare_exchange_strong_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                                 typename atomic<Ty>::value_type desired, memory_order success,
                                                 memory_order failure) noexcept {
        return obj->compare_exchange_strong(*expected, desired, success, failure);
    }

    template <typename Ty>
    bool atomic_compare_exchange_strong(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                        typename atomic<Ty>::value_type desired) noexcept {
        return atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }
    template <typename Ty>
    bool atomic_compare_exchange_strong(atomic<Ty> *obj, typename atomic<Ty>::value_type *expected,
                                        typename atomic<Ty>::value_type desired) noexcept {
        return atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order::seq_cst, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_add_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept {
        return obj->fetch_add(arg, order);
    }
    template <typename Ty>
    Ty atomic_fetch_add_explicit(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept {
        return obj->fetch_add(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_add(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept {
        return atomic_fetch_add_explicit(obj, arg, memory_order::seq_cst);
    }
    template <typename Ty>
    Ty atomic_fetch_add(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept {
        return atomic_fetch_add_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_sub_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept {
        return obj->fetch_sub(arg, order);
    }
    template <typename Ty>
    Ty atomic_fetch_sub_explicit(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg, memory_order order) noexcept {
        return obj->fetch_sub(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_sub(volatile atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept {
        return atomic_fetch_sub_explicit(obj, arg, memory_order::seq_cst);
    }
    template <typename Ty>
    Ty atomic_fetch_sub(atomic<Ty> *obj, typename atomic<Ty>::difference_type arg) noexcept {
        return atomic_fetch_sub_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_and_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_and(arg, order);
    }
    template <typename Ty>
    Ty atomic_fetch_and_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_and(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_and(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_and_explicit(obj, arg, memory_order::seq_cst);
    }
    template <typename Ty>
    Ty atomic_fetch_and(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_and_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_or_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_or(arg, order);
    }
    template <typename Ty>
    Ty atomic_fetch_or_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_or(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_or(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_or_explicit(obj, arg, memory_order::seq_cst);
    }
    template <typename Ty>
    Ty atomic_fetch_or(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_or_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_xor_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_xor(arg, order);
    }
    template <typename Ty>
    Ty atomic_fetch_xor_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_xor(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_xor(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_xor_explicit(obj, arg, memory_order::seq_cst);
    }
    template <typename Ty>
    Ty atomic_fetch_xor(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_xor_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_max_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_max(arg, order);
    }
   
    template <typename Ty>
    Ty atomic_fetch_max_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_max(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_max(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_max_explicit(obj, arg, memory_order::seq_cst);
    }
   
    template <typename Ty>
    Ty atomic_fetch_max(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_max_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_min_explicit(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_min(arg, order);
    }
    template <typename Ty>
    Ty atomic_fetch_min_explicit(atomic<Ty> *obj, typename atomic<Ty>::value_type arg, memory_order order) noexcept {
        return obj->fetch_min(arg, order);
    }

    template <typename Ty>
    Ty atomic_fetch_min(volatile atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_min_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    Ty atomic_fetch_min(atomic<Ty> *obj, typename atomic<Ty>::value_type arg) noexcept {
        return atomic_fetch_min_explicit(obj, arg, memory_order::seq_cst);
    }

    template <typename Ty>
    void atomic_wait_explicit(const volatile atomic<Ty> *obj, typename atomic<Ty>::value_type old, memory_order order) {
        obj->wait(old, order);
    }
    template <typename Ty>
    void atomic_wait_explicit(const atomic<Ty> *obj, typename atomic<Ty>::value_type old, memory_order order) {
        obj->wait(old, order);
    }

    template <typename Ty>
    void atomic_wait(const volatile atomic<Ty> *obj, typename atomic<Ty>::value_type old) {
        atomic_wait_explicit(obj, old, memory_order::seq_cst);
    }

    template <typename Ty>
    void atomic_wait(const atomic<Ty> *obj, typename atomic<Ty>::value_type old) {
        atomic_wait_explicit(obj, old, memory_order::seq_cst);
    }

    template <typename Ty>
    void atomic_notify_one(volatile atomic<Ty> *obj) {
        obj->notify_one();
    }
    template <typename Ty>
    void atomic_notify_one(atomic<Ty> *obj) {
        obj->notify_one();
    }

    template <typename Ty>
    void atomic_notify_all(volatile atomic<Ty> *obj) {
        obj->notify_all();
    }

    template <typename Ty>
    void atomic_notify_all(atomic<Ty> *obj) {
        obj->notify_all();
    }
}

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

        void wait(bool /*old*/, memory_order) const noexcept { /* TODO */
        }

        void wait(bool /*old*/, memory_order) const volatile noexcept { /* TODO */
        }

        void notify_one() noexcept { /* TODO */
        }

        void notify_one() volatile noexcept { /* TODO */
        }

        void notify_all() noexcept { /* TODO */
        }

        void notify_all() volatile noexcept { /* TODO */
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

namespace rainy::foundation::concurrency {
    class atomic_flag : public implements::atomic_flag_storage {
    public:
        // ---- 构造 ----
        constexpr atomic_flag() noexcept : implements::atomic_flag_storage() {
        }

        atomic_flag(const atomic_flag &) = delete;
        atomic_flag &operator=(const atomic_flag &) = delete;
        atomic_flag &operator=(const atomic_flag &) volatile = delete;

        // ---- test ----
        bool test(memory_order order = memory_order::seq_cst) const noexcept {
            return atomic_flag_storage::test(order);
        }
        bool test(memory_order order = memory_order::seq_cst) const volatile noexcept {
            return atomic_flag_storage::test(order);
        }

        // ---- test_and_set ----
        bool test_and_set(memory_order order = memory_order::seq_cst) noexcept {
            return atomic_flag_storage::test_and_set(order);
        }
        bool test_and_set(memory_order order = memory_order::seq_cst) volatile noexcept {
            return atomic_flag_storage::test_and_set(order);
        }

        // ---- clear ----
        void clear(memory_order order = memory_order::seq_cst) noexcept {
            atomic_flag_storage::clear(order);
        }
        void clear(memory_order order = memory_order::seq_cst) volatile noexcept {
            atomic_flag_storage::clear(order);
        }

        // ---- wait ----
        void wait(bool old, memory_order order = memory_order::seq_cst) const noexcept {
            atomic_flag_storage::wait(old, order);
        }
        void wait(bool old, memory_order order = memory_order::seq_cst) const volatile noexcept {
            atomic_flag_storage::wait(old, order);
        }

        // ---- notify ----
        void notify_one() noexcept {
            atomic_flag_storage::notify_one();
        }
        void notify_one() volatile noexcept {
            atomic_flag_storage::notify_one();
        }
        void notify_all() noexcept {
            atomic_flag_storage::notify_all();
        }
        void notify_all() volatile noexcept {
            atomic_flag_storage::notify_all();
        }
    };

    inline bool atomic_flag_test(const atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test(order);
    }
    inline bool atomic_flag_test(const volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test(order);
    }

    inline bool atomic_flag_test_and_set(atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test_and_set(order);
    }
    inline bool atomic_flag_test_and_set(volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        return flag->test_and_set(order);
    }

    inline void atomic_flag_clear(atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        flag->clear(order);
    }
    inline void atomic_flag_clear(volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        flag->clear(order);
    }

    inline void atomic_flag_wait(const atomic_flag *flag, bool old, memory_order order = memory_order::seq_cst) noexcept {
        flag->wait(old, order);
    }
    inline void atomic_flag_wait(const volatile atomic_flag *flag, bool old, memory_order order = memory_order::seq_cst) noexcept {
        flag->wait(old, order);
    }

    inline void atomic_flag_notify_one(atomic_flag *flag) noexcept {
        flag->notify_one();
    }
    inline void atomic_flag_notify_one(volatile atomic_flag *flag) noexcept {
        flag->notify_one();
    }
    inline void atomic_flag_notify_all(atomic_flag *flag) noexcept {
        flag->notify_all();
    }
    inline void atomic_flag_notify_all(volatile atomic_flag *flag) noexcept {
        flag->notify_all();
    }
}

#endif
