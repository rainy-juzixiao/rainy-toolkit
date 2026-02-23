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
#ifndef RAINY_FOUNDATION_CONCURRENCY_ATOMICINFRA_ATOMIC_OPS_HPP
#define RAINY_FOUNDATION_CONCURRENCY_ATOMICINFRA_ATOMIC_OPS_HPP
#include <rainy/foundation/concurrency/atomicinfra/fwd.hpp>

namespace rainy::foundation::concurrency::implements {
    template <typename T>
    struct atomic_ops;

    template <>
    struct atomic_ops<std::int8_t> {
        using type = std::int8_t;

        static type load(const volatile type *p, memory_order o) noexcept {
            return core::pal::iso_volatile_load8_explicit(p, o);
        }

        static void store(volatile type *p, type v, memory_order o) noexcept {
            core::pal::iso_volatile_store8_explicit(p, v, o);
        }

        static type exch(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange8_explicit(p, v, o);
        }

        static bool cas(volatile type *p, type &exp, type des, memory_order s, memory_order f) noexcept {
            type old = exp;
            bool ok = core::pal::interlocked_compare_exchange8_explicit(p, des, old, s, f);
            if (!ok) {
                exp = core::pal::iso_volatile_load8_explicit(p, f);
            }
            return ok;
        }

        static type add(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_add8_explicit(p, v, o);
        }

        static type sub(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_subtract8_explicit(p, v, o);
        }

        static type band(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_and8_explicit(p, v, o);
        }

        static type bor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_or8_explicit(p, v, o);
        }

        static type bxor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_xor8_explicit(p, v, o);
        }

        static type inc(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_increment8_explicit(p, o);
        }

        static type dec(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_decrement8_explicit(p, o);
        }
    };

    template <>
    struct atomic_ops<std::int16_t> {
        using type = std::int16_t;

        static type load(const volatile type *p, memory_order o) noexcept {
            return core::pal::iso_volatile_load16_explicit(p, o);
        }

        static void store(volatile type *p, type v, memory_order o) noexcept {
            core::pal::iso_volatile_store16_explicit(p, v, o);
        }

        static type exch(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange16_explicit(p, v, o);
        }

        static bool cas(volatile type *p, type &exp, type des, memory_order s, memory_order f) noexcept {
            type old = exp;
            bool ok = core::pal::interlocked_compare_exchange16_explicit(p, des, old, s, f);
            if (!ok) {
                exp = core::pal::iso_volatile_load16_explicit(p, f);
            }
            return ok;
        }

        static type add(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_add16_explicit(p, v, o);
        }
        static type sub(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_subtract16_explicit(p, v, o);
        }

        static type band(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_and16_explicit(p, v, o);
        }

        static type bor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_or16_explicit(p, v, o);
        }

        static type bxor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_xor16_explicit(p, v, o);
        }

        static type inc(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_increment16_explicit(p, o);
        }

        static type dec(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_decrement16_explicit(p, o);
        }
    };

    template <>
    struct atomic_ops<std::int32_t> {
        using type = std::int32_t;

        static type load(const volatile type *p, memory_order o) noexcept {
            return core::pal::iso_volatile_load32_explicit(p, o);
        }

        static void store(volatile type *p, type v, memory_order o) noexcept {
            core::pal::iso_volatile_store32_explicit(p, v, o);
        }

        static type exch(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange32_explicit(p, v, o);
        }

        static bool cas(volatile type *p, type &exp, type des, memory_order s, memory_order f) noexcept {
            type old = exp;
            bool ok = core::pal::interlocked_compare_exchange32_explicit(p, des, old, s, f);
            if (!ok)
                exp = core::pal::iso_volatile_load32_explicit(p, f);
            return ok;
        }

        static type add(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_add32_explicit(p, v, o);
        }

        static type sub(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_subtract32_explicit(p, v, o);
        }

        static type band(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_and32_explicit(p, v, o);
        }

        static type bor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_or32_explicit(p, v, o);
        }

        static type bxor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_xor32_explicit(p, v, o);
        }

        static type inc(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_increment32_explicit(p, o);
        }

        static type dec(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_decrement32_explicit(p, o);
        }
    };

    template <>
    struct atomic_ops<std::int64_t> {
        using type = std::int64_t;

        static type load(const volatile type *p, memory_order o) noexcept {
            return core::pal::iso_volatile_load64_explicit(p, o);
        }

        static void store(volatile type *p, type v, memory_order o) noexcept {
            core::pal::iso_volatile_store64_explicit(p, v, o);
        }

        static type exch(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange64_explicit(p, v, o);
        }

        static bool cas(volatile type *p, type &exp, type des, memory_order s, memory_order f) noexcept {
            type old = exp;
            bool ok = core::pal::interlocked_compare_exchange64_explicit(p, des, old, s, f);
            if (!ok)
                exp = core::pal::iso_volatile_load64_explicit(p, f);
            return ok;
        }

        static type add(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_add64_explicit(p, v, o);
        }

        static type sub(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_exchange_subtract64_explicit(p, v, o);
        }

        static type band(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_and64_explicit(p, v, o);
        }

        static type bor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_or64_explicit(p, v, o);
        }

        static type bxor(volatile type *p, type v, memory_order o) noexcept {
            return core::pal::interlocked_xor64_explicit(p, v, o);
        }

        static type inc(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_increment64_explicit(p, o);
        }

        static type dec(volatile type *p, memory_order o) noexcept {
            return core::pal::interlocked_decrement64_explicit(p, o);
        }
    };

    template <typename Unsigned, typename Signed>
    struct atomic_ops_unsigned_adapter : atomic_ops<Signed> {
        using type = Unsigned;
    };

    template <>
    struct atomic_ops<std::uint8_t> : atomic_ops_unsigned_adapter<std::uint8_t, std::int8_t> {};
    template <>
    struct atomic_ops<std::uint16_t> : atomic_ops_unsigned_adapter<std::uint16_t, std::int16_t> {};
    template <>
    struct atomic_ops<std::uint32_t> : atomic_ops_unsigned_adapter<std::uint32_t, std::int32_t> {};
    template <>
    struct atomic_ops<std::uint64_t> : atomic_ops_unsigned_adapter<std::uint64_t, std::int64_t> {};

    template <typename T, typename = void>
    struct select_ops_type {
        using type = T;
    }; // fallback（触发static_assert）

    template <typename T>
    struct select_ops_type<T, std::enable_if_t<sizeof(T) == 1>> {
        using type = std::int8_t;
    };
    template <typename T>
    struct select_ops_type<T,
                           std::enable_if_t<sizeof(T) == 2 && !std::is_same_v<T, std::int16_t> && !std::is_same_v<T, std::uint16_t>>> {
        using type = std::int16_t;
    };
    template <typename T>
    struct select_ops_type<T,
                           std::enable_if_t<sizeof(T) == 4 && !std::is_same_v<T, std::int32_t> && !std::is_same_v<T, std::uint32_t>>> {
        using type = std::int32_t;
    };
    template <typename T>
    struct select_ops_type<T,
                           std::enable_if_t<sizeof(T) == 8 && !std::is_same_v<T, std::int64_t> && !std::is_same_v<T, std::uint64_t>>> {
        using type = std::int64_t;
    };

    template <typename T, typename Ops>
    T atomic_fetch_max(volatile T *p, T val, memory_order order) noexcept {
        T old = Ops::load(p, memory_order::relaxed);
        while (old < val) {
            T exp = old;
            if (Ops::cas(p, exp, val, order, memory_order::relaxed))
                break;
            old = exp; // cas 失败时 exp 已被更新为当前值
        }
        return old;
    }

    template <typename T, typename Ops>
    T atomic_fetch_min(volatile T *p, T val, memory_order order) noexcept {
        T old = Ops::load(p, memory_order::relaxed);
        while (old > val) {
            T exp = old;
            if (Ops::cas(p, exp, val, order, memory_order::relaxed))
                break;
            old = exp;
        }
        return old;
    }
}

namespace rainy::foundation::concurrency::implements {
    template <typename Float>
    struct float_int_traits;

    template <>
    struct float_int_traits<float> {
        using float_type = float;
        using int_type = std::int32_t;
        static_assert(sizeof(float_type) == sizeof(int_type));
    };

    template <>
    struct float_int_traits<double> {
        using float_type = double;
        using int_type = std::int64_t;
        static_assert(sizeof(float_type) == sizeof(int_type));
    };

    template <>
    struct float_int_traits<long double> {
        using float_type = long double;
        using int_type = std::int64_t;
        static_assert(sizeof(float_type) == sizeof(int_type));
    };

    template <typename Float>
    struct atomic_ops_float {
        using type = Float;
        using int_type = typename float_int_traits<Float>::int_type;
        using iops = atomic_ops<int_type>; // 已有的整数 ops

        static int_type to_int(Float f) noexcept {
            int_type result{};
            std::memcpy(&result, &f, sizeof(int_type));
            return result;
        }

        static Float to_float(int_type i) noexcept {
            Float result{};
            std::memcpy(&result, &i, sizeof(Float));
            return result;
        }

        static Float load(const volatile type *p, memory_order o) noexcept {
            // 先把浮点指针重解释为整数指针再做原子读
            return to_float(iops::load(reinterpret_cast<const volatile int_type *>(p), o));
        }

        static void store(volatile type *p, Float v, memory_order o) noexcept {
            iops::store(reinterpret_cast<volatile int_type *>(p), to_int(v), o);
        }

        static Float exch(volatile type *p, Float v, memory_order o) noexcept {
            return to_float(iops::exch(reinterpret_cast<volatile int_type *>(p), to_int(v), o));
        }

        static bool cas(volatile type *p, Float &expected, Float desired, memory_order s, memory_order f) noexcept {
            // 将 expected 转为整数，CAS 完成后若失败将当前整数值写回 expected
            int_type exp_i = to_int(expected);
            bool ok = iops::cas(reinterpret_cast<volatile int_type *>(p), exp_i, to_int(desired), s, f);
            if (!ok) {
                // iops::cas 失败时已把最新整数值写入 exp_i
                expected = to_float(exp_i);
            }
            return ok;
        }
    };

    template <>
    struct atomic_ops<float> : atomic_ops_float<float> {};
    template <>
    struct atomic_ops<double> : atomic_ops_float<double> {};
    template <>
    struct atomic_ops<long double> : atomic_ops_float<long double> {};
}

namespace rainy::foundation::concurrency::implements {
    template <typename T>
    struct atomic_ops_pointer {
        using type = T *;
        using iops = atomic_ops<std::intptr_t>; // intptr_t 整数 ops

        // 辅助转换：指针 <-> intptr_t，无 UB（标准允许指针与整数互转）
        static std::intptr_t to_int(T *p) noexcept {
            return reinterpret_cast<std::intptr_t>(p);
        }
        static T *to_ptr(std::intptr_t i) noexcept {
            return reinterpret_cast<T *>(i);
        }

        static T *load(const volatile type *p, memory_order o) noexcept {
            return to_ptr(iops::load(reinterpret_cast<const volatile std::intptr_t *>(p), o));
        }

        static void store(volatile type *p, T *v, memory_order o) noexcept {
            iops::store(reinterpret_cast<volatile std::intptr_t *>(p), to_int(v), o);
        }

        static T *exch(volatile type *p, T *v, memory_order o) noexcept {
            return reinterpret_cast<T *>(core::pal::interlocked_exchange_pointer_explicit(reinterpret_cast<volatile void **>(p),
                                                                                          reinterpret_cast<void *>(v), o));
        }

        static bool cas(volatile type *p, T *&expected, T *desired, memory_order s, memory_order f) noexcept {
            void *exp_v = reinterpret_cast<void *>(expected);
            void *result = core::pal::interlocked_compare_exchange_pointer_explicit(reinterpret_cast<volatile void **>(p),
                                                                                    reinterpret_cast<void *>(desired), exp_v, s, f);
            // PAL 的指针 CAS 返回操作前的原始值：
            //   若等于 expected 则成功，否则失败并将最新值写回 expected
            if (result == exp_v) {
                return true;
            }
            expected = reinterpret_cast<T *>(result);
            return false;
        }

        static T *ptr_add(volatile type *p, std::ptrdiff_t n, memory_order o) noexcept {
            std::intptr_t byte_offset = static_cast<std::intptr_t>(n) * static_cast<std::intptr_t>(sizeof(T));
            return to_ptr(core::pal::interlocked_exchange_add_explicit(reinterpret_cast<volatile std::intptr_t *>(p), byte_offset, o));
        }

        static T *ptr_sub(volatile type *p, std::ptrdiff_t n, memory_order o) noexcept {
            std::intptr_t byte_offset = static_cast<std::intptr_t>(n) * static_cast<std::intptr_t>(sizeof(T));
            return to_ptr(
                core::pal::interlocked_exchange_subtract_explicit(reinterpret_cast<volatile std::intptr_t *>(p), byte_offset, o));
        }
    };

    template <typename T>
    struct atomic_ops<T *> : atomic_ops_pointer<T> {};

    template <typename T>
    T *atomic_ptr_fetch_max(volatile T **p, T *val, memory_order order) noexcept {
        using ops = atomic_ops<T *>;
        T *old = ops::load(p, memory_order::relaxed);
        while (old < val) {
            T *exp = old;
            if (ops::cas(p, exp, val, order, memory_order::relaxed))
                break;
            old = exp;
        }
        return old;
    }

    template <typename T>
    T *atomic_ptr_fetch_min(volatile T **p, T *val, memory_order order) noexcept {
        using ops = atomic_ops<T *>;
        T *old = ops::load(p, memory_order::relaxed);
        while (old > val) {
            T *exp = old;
            if (ops::cas(p, exp, val, order, memory_order::relaxed))
                break;
            old = exp;
        }
        return old;
    }
}

#endif
