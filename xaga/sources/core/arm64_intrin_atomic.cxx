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
#include <rainy/core/implements/arm64_intrin.hpp>

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
inline void rainy_dmb() {
    __asm__ __volatile__("dmb ish" ::: "memory");
}
inline void rainy_dmb_ld() {
    __asm__ __volatile__("dmb ishld" ::: "memory");
}
inline void rainy_dmb_st() {
    __asm__ __volatile__("dmb ishst" ::: "memory");
}
#endif

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC

#define RAINY_ARM64_ATOMIC_RMW(ptr, op, order, result_var)                                                                            \
    do {                                                                                                                              \
        unsigned long _old_val, _new_val;                                                                                             \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            if ((order) == memory_order_acquire || (order) == memory_order_acq_rel || (order) == memory_order_seq_cst) {              \
                __asm__ __volatile__("ldaxr %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                      \
            } else {                                                                                                                  \
                __asm__ __volatile__("ldxr %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                       \
            }                                                                                                                         \
            _new_val = _old_val op;                                                                                                   \
                                                                                                                                      \
            if ((order) == memory_order_release || (order) == memory_order_acq_rel || (order) == memory_order_seq_cst) {              \
                __asm__ __volatile__("stlxr %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");              \
            } else {                                                                                                                  \
                __asm__ __volatile__("stxr %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");               \
            }                                                                                                                         \
        } while (_stxr_failed);                                                                                                       \
        if ((order) == memory_order_seq_cst) {                                                                                        \
            rainy_dmb();                                                                                                              \
        }                                                                                                                             \
        result_var = static_cast<long>(_new_val);                                                                                     \
    } while (0)


#endif

#if RAINY_IS_ARM64

#if RAINY_USING_MSVC
#define RAINY_ATOMIC_DISPATCH(FUNC, obj, val, order)                                                                                  \
    do {                                                                                                                              \
        switch (order) {                                                                                                              \
            case memory_order_relaxed:                                                                                                \
                return FUNC##_nf(obj, val);                                                                                           \
            case memory_order_acquire:                                                                                                \
                return FUNC##_acq(obj, val);                                                                                          \
            case memory_order_release:                                                                                                \
                return FUNC##_rel(obj, val);                                                                                          \
            case memory_order_acq_rel:                                                                                                \
                return FUNC(obj, val);                                                                                                \
            case memory_order_seq_cst:                                                                                                \
            default:                                                                                                                  \
                return FUNC(obj, val);                                                                                                \
        }                                                                                                                             \
    } while (0)

#endif

namespace rainy::core::pal::implements {
#if RAINY_USING_MSVC
    long interlocked_increment_arm64_explicit(volatile long *value, memory_order order) {
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd, value, 1, order);
    }
#else
    long interlocked_increment_arm64_explicit(volatile long *value, memory_order order) {
        long new_val;
        RAINY_ARM64_ATOMIC_RMW(value, +1, order, new_val);
        return new_val;
    }
#endif
}

namespace rainy::core::pal::implements {
#if RAINY_USING_MSVC
    long interlocked_decrement_arm64_explicit(volatile long *value, memory_order order) {
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd, value, -1, order);
    }
#else
    long interlocked_decrement_arm64_explicit(volatile long *value, memory_order order) {
        long new_val;
        RAINY_ARM64_ATOMIC_RMW(value, -1, order, new_val);
        return new_val;
    }
#endif
}

#endif
