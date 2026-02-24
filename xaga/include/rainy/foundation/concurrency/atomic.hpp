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
#include <rainy/foundation/concurrency/atomicinfra/atomic_flag.hpp>
#include <rainy/foundation/concurrency/atomicinfra/atomic_ref.hpp>

namespace rainy::foundation::concurrency {
    /**
     * @brief 原子类型模板类
     *
     * 描述对 Ty 类型的存储值执行 atomic 操作的对象。
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

    using atomic_int8_t = atomic<std::int8_t>;
    using atomic_uint8_t = atomic<std::uint8_t>;
    using atomic_int16_t = atomic<std::int16_t>;
    using atomic_uint16_t = atomic<std::uint16_t>;
    using atomic_int32_t = atomic<std::int32_t>;
    using atomic_uint32_t = atomic<std::uint32_t>;
    using atomic_int64_t = atomic<std::int64_t>;
    using atomic_uint64_t = atomic<std::uint64_t>;

    using atomic_int_least8_t = atomic<std::int_least8_t>;
    using atomic_uint_least8_t = atomic<std::uint_least8_t>;
    using atomic_int_least16_t = atomic<std::int_least16_t>;
    using atomic_uint_least16_t = atomic<std::uint_least16_t>;
    using atomic_int_least32_t = atomic<std::int_least32_t>;
    using atomic_uint_least32_t = atomic<std::uint_least32_t>;
    using atomic_int_least64_t = atomic<std::int_least64_t>;
    using atomic_uint_least64_t = atomic<std::uint_least64_t>;

    using atomic_int_fast8_t = atomic<std::int_fast8_t>;
    using atomic_uint_fast8_t = atomic<std::uint_fast8_t>;
    using atomic_int_fast16_t = atomic<std::int_fast16_t>;
    using atomic_uint_fast16_t = atomic<std::uint_fast16_t>;
    using atomic_int_fast32_t = atomic<std::int_fast32_t>;
    using atomic_uint_fast32_t = atomic<std::uint_fast32_t>;
    using atomic_int_fast64_t = atomic<std::int_fast64_t>;
    using atomic_uint_fast64_t = atomic<std::uint_fast64_t>;

    using atomic_intptr_t = atomic<std::intptr_t>;
    using atomic_uintptr_t = atomic<std::uintptr_t>;
    using atomic_size_t = atomic<std::size_t>;
    using atomic_ptrdiff_t = atomic<std::ptrdiff_t>;
    using atomic_intmax_t = atomic<std::intmax_t>;
    using atomic_uintmax_t = atomic<std::uintmax_t>;
}

namespace rainy::foundation::concurrency {
    /**
     * @brief 原子类型引用模板类
     *
     * 描述对 Ty 类型的存储值执行 atomic 操作的引用。
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

namespace rainy::foundation::concurrency {
    /**
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
     * @brief 存储原子值（使用默认顺序）
     *
     * 使用默认的内存顺序 `memory_order::seq_cst` 存储值。
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
     * @brief 存储原子值（使用默认顺序）（const 类型）
     *
     * 使用默认的内存顺序 `memory_order::seq_cst` 存储值。
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
     * @brief 显式交换原子值
     *
     * 该函数执行一个原子的交换操作，使用显式指定的内存顺序。
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
     * @brief 显式交换原子值（const 类型）
     *
     * 该函数执行一个原子的交换操作，使用显式指定的内存顺序。
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
     * @brief 交换原子值（使用默认顺序）
     *
     * 使用默认的内存顺序 `memory_order::seq_cst` 执行交换操作。
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
     * @brief 交换原子值（使用默认顺序）（const 类型）
     *
     * 使用默认的内存顺序 `memory_order::seq_cst` 执行交换操作。
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
     * @brief 显式弱比较交换原子值
     *
     * 该函数尝试进行原子比较和交换操作（弱版本），并使用显式指定的内存顺序。
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
     * @brief 显式弱比较交换原子值（const 类型）
     *
     * 该函数尝试进行原子比较和交换操作（弱版本），并使用显式指定的内存顺序。
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
     * @brief 弱比较交换原子值（使用默认顺序）
     *
     * 该函数尝试进行原子比较和交换操作（弱版本），并使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 弱比较交换原子值（使用默认顺序）（const 类型）
     *
     * 该函数尝试进行原子比较和交换操作（弱版本），并使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式强比较交换原子值
     *
     * 该函数尝试进行原子比较和交换操作（强版本），并使用显式指定的内存顺序。
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
     * @brief 显式强比较交换原子值（const 类型）
     *
     * 该函数尝试进行原子比较和交换操作（强版本），并使用显式指定的内存顺序。
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
     * @brief 强比较交换原子值（使用默认顺序）
     *
     * 该函数尝试进行原子比较和交换操作（强版本），并使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 强比较交换原子值（使用默认顺序）（const 类型）
     *
     * 该函数尝试进行原子比较和交换操作（强版本），并使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子加法操作
     *
     * 该函数执行一个原子的加法操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子加法操作（const 类型）
     *
     * 该函数执行一个原子的加法操作，使用显式指定的内存顺序。
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
     * @brief 执行原子加法操作（使用默认顺序）
     *
     * 该函数执行一个原子的加法操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子加法操作（使用默认顺序）（const 类型）
     *
     * 该函数执行一个原子的加法操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子减法操作
     *
     * 该函数执行一个原子的减法操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子减法操作（const 类型）
     *
     * 该函数执行一个原子的减法操作，使用显式指定的内存顺序。
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
     * @brief 执行原子减法操作（使用默认顺序）
     *
     * 该函数执行一个原子的减法操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子减法操作（使用默认顺序）（const 类型）
     *
     * 该函数执行一个原子的减法操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子与操作
     *
     * 该函数执行原子的与操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子与操作（const 类型）
     *
     * 该函数执行原子的与操作，使用显式指定的内存顺序。
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
     * @brief 执行原子与操作（使用默认顺序）
     *
     * 该函数执行原子的与操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子与操作（使用默认顺序）（const 类型）
     *
     * 该函数执行原子的与操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子或操作
     *
     * 该函数执行原子的或操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子或操作（const 类型）
     *
     * 该函数执行原子的或操作，使用显式指定的内存顺序。
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
     * @brief 执行原子或操作（使用默认顺序）
     *
     * 该函数执行原子的或操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子或操作（使用默认顺序）（const 类型）
     *
     * 该函数执行原子的或操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子异或操作
     *
     * 该函数执行原子的异或操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子异或操作（const 类型）
     *
     * 该函数执行原子的异或操作，使用显式指定的内存顺序。
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
     * @brief 执行原子异或操作（使用默认顺序）
     *
     * 该函数执行一个原子的异或操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子异或操作（使用默认顺序）（const 类型）
     *
     * 该函数执行一个原子的异或操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子最大值操作
     *
     * 该函数执行原子的最大值操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子最大值操作（const 类型）
     *
     * 该函数执行原子的最大值操作，使用显式指定的内存顺序。
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
     * @brief 执行原子最大值操作（使用默认顺序）
     *
     * 该函数执行一个原子的最大值操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子最大值操作（使用默认顺序）（const 类型）
     *
     * 该函数执行一个原子的最大值操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式执行原子最小值操作
     *
     * 该函数执行原子的最小值操作，使用显式指定的内存顺序。
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
     * @brief 显式执行原子最小值操作（const 类型）
     *
     * 该函数执行原子的最小值操作，使用显式指定的内存顺序。
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
     * @brief 执行原子最小值操作（使用默认顺序）
     *
     * 该函数执行一个原子的最小值操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子最小值操作（使用默认顺序）（const 类型）
     *
     * 该函数执行一个原子的最小值操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 显式等待原子操作
     *
     * 该函数执行原子的等待操作，使用显式指定的内存顺序。
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
     * @brief 显式等待原子操作（const 类型）
     *
     * 该函数执行原子的等待操作，使用显式指定的内存顺序。
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
     * @brief 执行原子等待操作（使用默认顺序）
     *
     * 该函数执行一个原子的等待操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子等待操作（使用默认顺序）（const 类型）
     *
     * 该函数执行一个原子的等待操作，使用默认的内存顺序 `memory_order::seq_cst`。
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
     * @brief 执行原子通知操作（单个）
     *
     * 该函数执行原子的通知操作，仅通知一个等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_one(volatile atomic<Ty> *obj) {
        obj->notify_one();
    }

    /**
     * @brief 执行原子通知操作（单个）（const 类型）
     *
     * 该函数执行原子的通知操作，仅通知一个等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_one(atomic<Ty> *obj) {
        obj->notify_one();
    }

    /**
     * @brief 执行原子通知操作（全部）
     *
     * 该函数执行原子的通知操作，通知所有等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_all(volatile atomic<Ty> *obj) {
        obj->notify_all();
    }

    /**
     * @brief 执行原子通知操作（全部）（const 类型）
     *
     * 该函数执行原子的通知操作，通知所有等待线程。
     *
     * @tparam Ty 要执行通知操作的原子类型。
     * @param obj 要执行通知操作的原子对象。
     */
    template <typename Ty>
    void atomic_notify_all(atomic<Ty> *obj) {
        obj->notify_all();
    }
}

namespace rainy::foundation::concurrency {
    class atomic_flag : public implements::atomic_flag_storage {
    public:
        /**
         * @brief 默认构造函数
         *
         * 初始化一个 `atomic_flag` 对象。
         */
        constexpr atomic_flag() noexcept : implements::atomic_flag_storage() {
        }

        /**
         * @brief 禁止拷贝构造
         */
        atomic_flag(const atomic_flag &) = delete;

        /**
         * @brief 禁止拷贝赋值
         */
        atomic_flag &operator=(const atomic_flag &) = delete;

        /**
         * @brief 禁止拷贝赋值（volatile 修饰）
         */
        atomic_flag &operator=(const atomic_flag &) volatile = delete;

        /**
         * @brief 测试标志位的状态
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test(memory_order order = memory_order::seq_cst) const noexcept -> bool {
            return atomic_flag_storage::test(order);
        }

        /**
         * @brief 测试标志位的状态（volatile 修饰）
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test(memory_order order = memory_order::seq_cst) const volatile noexcept -> bool {
            return atomic_flag_storage::test(order);
        }

        /**
         * @brief 测试并设置标志位
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test_and_set(memory_order order = memory_order::seq_cst) noexcept -> bool {
            return atomic_flag_storage::test_and_set(order);
        }

        /**
         * @brief 测试并设置标志位（volatile 修饰）
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         * @return 如果标志已设置返回 `true`，否则返回 `false`。
         */
        rain_fn test_and_set(memory_order order = memory_order::seq_cst) volatile noexcept -> bool {
            return atomic_flag_storage::test_and_set(order);
        }

        /**
         * @brief 清除标志位
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn clear(memory_order order = memory_order::seq_cst) noexcept -> void {
            atomic_flag_storage::clear(order);
        }

        /**
         * @brief 清除标志位（volatile 修饰）
         *
         * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn clear(memory_order order = memory_order::seq_cst) volatile noexcept -> void {
            atomic_flag_storage::clear(order);
        }

        /**
         * @brief 等待标志位的变化
         *
         * @param old 期望的旧值。
         * @param order 等待操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(bool old, memory_order order = memory_order::seq_cst) const noexcept -> void {
            atomic_flag_storage::wait(old, order);
        }

        /**
         * @brief 等待标志位的变化（volatile 修饰）
         *
         * @param old 期望的旧值。
         * @param order 等待操作的内存顺序（默认为 memory_order::seq_cst）。
         */
        rain_fn wait(bool old, memory_order order = memory_order::seq_cst) const volatile noexcept -> void {
            atomic_flag_storage::wait(old, order);
        }

        /**
         * @brief 通知一个等待的线程
         */
        rain_fn notify_one() noexcept -> void {
            atomic_flag_storage::notify_one();
        }

        /**
         * @brief 通知一个等待的线程（volatile 修饰）
         */
        rain_fn notify_one() volatile noexcept -> void {
            atomic_flag_storage::notify_one();
        }

        /**
         * @brief 通知所有等待的线程
         */
        rain_fn notify_all() noexcept -> void {
            atomic_flag_storage::notify_all();
        }

        /**
         * @brief 通知所有等待的线程（volatile 修饰）
         */
        rain_fn notify_all() volatile noexcept -> void {
            atomic_flag_storage::notify_all();
        }
    };

    /**
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
     * @brief 清除标志位
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     */
    RAINY_INLINE void atomic_flag_clear(atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        flag->clear(order);
    }

    /**
     * @brief 清除标志位（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     * @param order 操作的内存顺序（默认为 memory_order::seq_cst）。
     */
    RAINY_INLINE void atomic_flag_clear(volatile atomic_flag *flag, memory_order order = memory_order::seq_cst) noexcept {
        flag->clear(order);
    }

    /**
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
     * @brief 通知一个等待的线程
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_one(atomic_flag *flag) noexcept {
        flag->notify_one();
    }

    /**
     * @brief 通知一个等待的线程（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_one(volatile atomic_flag *flag) noexcept {
        flag->notify_one();
    }

    /**
     * @brief 通知所有等待的线程
     *
     * @param flag 要操作的 `atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_all(atomic_flag *flag) noexcept {
        flag->notify_all();
    }

    /**
     * @brief 通知所有等待的线程（volatile 修饰）
     *
     * @param flag 要操作的 `volatile atomic_flag` 对象。
     */
    RAINY_INLINE void atomic_flag_notify_all(volatile atomic_flag *flag) noexcept {
        flag->notify_all();
    }
}

#endif
