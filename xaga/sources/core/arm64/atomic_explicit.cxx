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
#include <rainy/core/layer.hpp>

#if RAINY_IS_ARM64

#if RAINY_USING_MSVC

#define RAINY_ATOMIC_DISPATCH(FUNC, result, order, ...)                                                                               \
    do {                                                                                                                              \
        switch (order) {                                                                                                              \
            case memory_order_relaxed:                                                                                                \
                result = FUNC##_nf(__VA_ARGS__);                                                                                      \
                break;                                                                                                                \
            case memory_order_acquire:                                                                                                \
                result = FUNC##_acq(__VA_ARGS__);                                                                                     \
                break;                                                                                                                \
            case memory_order_release:                                                                                                \
                result = FUNC##_rel(__VA_ARGS__);                                                                                     \
                break;                                                                                                                \
            case memory_order_acq_rel:                                                                                                \
                result = FUNC(__VA_ARGS__);                                                                                           \
                break;                                                                                                                \
            case memory_order_seq_cst:                                                                                                \
            default:                                                                                                                  \
                result = FUNC(__VA_ARGS__);                                                                                           \
                break;                                                                                                                \
        }                                                                                                                             \
    } while (0)
#endif

#endif

#if RAINY_USING_MSVC
namespace rainy::core::pal {
    long interlocked_increment_explicit(volatile long *value, memory_order order) {
        return interlocked_exchange_add_explicit(reinterpret_cast<volatile std::intptr_t *>(value), +1, order);
    }

    std::int8_t interlocked_increment8_explicit(volatile std::int8_t *value, memory_order order) {
        return interlocked_exchange_add8_explicit(value, +1, order);
    }

    std::int16_t interlocked_increment16_explicit(volatile std::int16_t *value, memory_order order) {
        return interlocked_exchange_add16_explicit(value, +1, order);
    }

    std::int32_t interlocked_increment32_explicit(volatile std::int32_t *value, memory_order order) {
        return interlocked_exchange_add32_explicit(value, 1, order);
    }

    std::int64_t interlocked_increment64_explicit(volatile std::int64_t *value, memory_order order) {
        return interlocked_exchange_add64_explicit(value, 1, order);
    }
}

namespace rainy::core::pal {
    long interlocked_decrement_explicit(volatile long *value, memory_order order) {
        return interlocked_exchange_add_explicit(reinterpret_cast<volatile std::intptr_t *>(value), -1, order);
    }

    std::int8_t interlocked_decrement8_explicit(volatile std::int8_t *value, memory_order order) {
        return interlocked_exchange_add8_explicit(value, -1, order);
    }

    std::int16_t interlocked_decrement16_explicit(volatile std::int16_t *value, memory_order order) {
        return interlocked_exchange_add16_explicit(value, -1, order);
    }

    std::int32_t interlocked_decrement32_explicit(volatile std::int32_t *value, memory_order order) {
        return interlocked_exchange_add32_explicit(value, -1, order);
    }

    std::int64_t interlocked_decrement64_explicit(volatile std::int64_t *value, memory_order order) {
        return interlocked_exchange_add64_explicit(value, -1, order);
    }
}

namespace rainy::core::pal {
    std::intptr_t interlocked_exchange_add_explicit(volatile std::intptr_t *value, const std::intptr_t amount, memory_order order) {
        std::intptr_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd, new_val, order, reinterpret_cast<volatile long *>(value), amount);
        return new_val;
    }

    std::int8_t interlocked_exchange_add8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order) {
        std::int8_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd8, new_val, order, reinterpret_cast<volatile char *>(value), amount);
        return new_val;
    }

    std::int16_t interlocked_exchange_add16_explicit(volatile std::int16_t *value, std::int16_t amount, memory_order order) {
        std::int16_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd16, new_val, order, reinterpret_cast<volatile std::int16_t *>(value), amount);
        return new_val;
    }

    std::int32_t interlocked_exchange_add32_explicit(volatile std::int32_t *value, std::int32_t amount, memory_order order) {
        std::int32_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd, new_val, order, reinterpret_cast<volatile long *>(value), amount);
        return new_val;
    }

#if RAINY_USING_64_BIT_PLATFORM
    std::int64_t interlocked_exchange_add64_explicit(volatile std::int64_t *value, std::int64_t amount, memory_order order) {
        std::int64_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchangeAdd64, new_val, order, reinterpret_cast<volatile std::int64_t *>(value), amount);
        return new_val;
    }
#endif
}

namespace rainy::core::pal {
    std::intptr_t interlocked_exchange_subtract_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                         memory_order order) {
        return interlocked_exchange_add_explicit(value, -amount, order);
    }

    std::int8_t interlocked_exchange_subtract8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order) {
        return interlocked_exchange_add8_explicit(value, -amount, order);
    }

    std::int16_t interlocked_exchange_subtract16_explicit(volatile std::int16_t *value, std::int16_t amount, memory_order order) {
        return interlocked_exchange_add16_explicit(value, -amount, order);
    }

    std::int32_t interlocked_exchange_subtract32_explicit(volatile std::int32_t *value, std::int32_t amount, memory_order order) {
        return interlocked_exchange_add32_explicit(value, -amount, order);
    }

    std::int64_t interlocked_exchange_subtract64_explicit(volatile std::int64_t *value, std::int64_t amount, memory_order order) {
        return interlocked_exchange_add64_explicit(value, -amount, order);
    }
}

namespace rainy::core::pal {
    std::intptr_t interlocked_exchange_explicit(volatile std::intptr_t *target, std::intptr_t value, memory_order order) {
        long ret;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchange, ret, order, reinterpret_cast<volatile long *>(target), value);
        return ret;
    }

    std::int8_t interlocked_exchange8_explicit(volatile std::int8_t *target, std::int8_t value, memory_order order) {
        std::int8_t ret;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchange8, ret, order, reinterpret_cast<volatile char *>(target), value);
        return ret;
    }

    std::int16_t interlocked_exchange16_explicit(volatile std::int16_t *target, std::int16_t value, memory_order order) {
        std::int16_t ret;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchange16, ret, order, reinterpret_cast<volatile std::int16_t *>(target), value);
        return ret;
    }

    std::int32_t interlocked_exchange32_explicit(volatile std::int32_t *target, std::int32_t value, memory_order order) {
        std::int32_t ret;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchange, ret, order, reinterpret_cast<volatile long *>(target), value);
        return ret;
    }

#if RAINY_USING_64_BIT_PLATFORM
    std::int64_t interlocked_exchange64_explicit(volatile std::int64_t *target, std::int64_t value, memory_order order) {
        std::int64_t ret;
        RAINY_ATOMIC_DISPATCH(_InterlockedExchange64, ret, order, target, value);
        return ret;
    }
#endif

    void *interlocked_exchange_pointer_explicit(volatile void **target, void *value, memory_order order) {
#if RAINY_USING_64BIT_PLATFORM
        return reinterpret_cast<void *>(interlocked_exchange64_explicit(reinterpret_cast<volatile std::int64_t *>(target),
                                                                        reinterpret_cast<std::int64_t>(value), order));
#else
        return reinterpret_cast<void *>(interlocked_exchange32_explicit(reinterpret_cast<volatile std::int32_t *>(target),
                                                                        reinterpret_cast<std::int32_t>(value), order));
#endif
    }

    void *interlocked_compare_exchange_pointer_explicit(volatile void **destination, void *exchange, void *comparand,
                                                        memory_order success, memory_order failure) {
        rainy_assume(destination);
        void *result = interlocked_compare_exchange_pointer_explicit(destination, exchange, comparand, memory_order_seq_cst);
        if (result) {
            atomic_thread_fence(success);
        } else {
            atomic_thread_fence(failure);
        }
        return result;
    }
}

namespace rainy::core::pal {
    std::intptr_t interlocked_and_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
        long new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedAnd, new_val, order, reinterpret_cast<volatile long *>(value), mask);
        return new_val;
    }

    std::int8_t interlocked_and8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedAnd8, new_val, order, reinterpret_cast<volatile char *>(value), mask);
        return new_val;
    }

    std::int16_t interlocked_and16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedAnd16, new_val, order, value, mask);
        return new_val;
    }

    std::int32_t interlocked_and32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedAnd, new_val, order, reinterpret_cast<volatile long *>(value), mask);
        return new_val;
    }

#if RAINY_USING_64_BIT_PLATFORM
    std::int64_t interlocked_and64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedAnd64, new_val, order, value, mask);
        return new_val;
    }
#endif
}

namespace rainy::core::pal {
    std::intptr_t interlocked_or_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
        long new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedOr, new_val, order, reinterpret_cast<volatile long *>(value), mask);
        return new_val;
    }

    std::int8_t interlocked_or8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedOr8, new_val, order, reinterpret_cast<volatile char *>(value), mask);
        return new_val;
    }

    std::int16_t interlocked_or16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedOr16, new_val, order, value, mask);
        return new_val;
    }

    std::int32_t interlocked_or32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedOr, new_val, order, reinterpret_cast<volatile long *>(value), mask);
        return new_val;
    }

#if RAINY_USING_64_BIT_PLATFORM
    std::int64_t interlocked_or64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedOr64, new_val, order, reinterpret_cast<volatile std::int64_t *>(value), mask);
        return new_val;
    }
#endif
}

namespace rainy::core::pal {
    std::intptr_t interlocked_xor_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
        std::int16_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedXor, new_val, order, reinterpret_cast<volatile long *>(value), mask);
        return new_val;
    }

    std::int8_t interlocked_xor8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int16_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedXor8, new_val, order, reinterpret_cast<volatile char *>(value), mask);
        return new_val;
    }

    std::int16_t interlocked_xor16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedXor16, new_val, order, value, mask);
        return new_val;
    }

    std::int32_t interlocked_xor32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedXor, new_val, order, reinterpret_cast<volatile long *>(value), mask);
        return new_val;
    }

#if RAINY_USING_64_BIT_PLATFORM
    std::int64_t interlocked_xor64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t new_val;
        RAINY_ATOMIC_DISPATCH(_InterlockedXor64, new_val, order, value, mask);
        return new_val;
    }
#endif
}

#define RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_LOAD(FUNC, result, order, ...)                                                             \
    rainy_assume(address);                                                                                                            \
    result = FUNC(__VA_ARGS__);                                                                                                       \
    if (order == memory_order_acquire || order == memory_order_seq_cst) {                                                             \
        read_barrier();                                                                                                               \
    }                                                                                                                                 \
    if (order == memory_order_seq_cst) {                                                                                              \
        read_write_barrier();                                                                                                         \
    }

#define RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_STORE(FUNC, order, ...)                                                                    \
    rainy_assume(address);                                                                                                            \
    FUNC(__VA_ARGS__);                                                                                                                \
    if (order == memory_order_release || order == memory_order_seq_cst) {                                                             \
        write_barrier();                                                                                                              \
    }                                                                                                                                 \
    if (order == memory_order_seq_cst) {                                                                                              \
        read_write_barrier();                                                                                                         \
    }


namespace rainy::core::pal {
    std::intptr_t iso_volatile_load_explicit(const volatile std::intptr_t *address, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return iso_volatile_load64_explicit(address, order);
#else
        return iso_volatile_load32_explicit(address, order);
#endif
    }

    std::int8_t iso_volatile_load8_explicit(const volatile std::int8_t *address, memory_order order) {
        std::int8_t value;
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_LOAD(__iso_volatile_load8, value, order, reinterpret_cast<const volatile char *>(address));
        return value;
    }

    std::int16_t iso_volatile_load16_explicit(const volatile std::int16_t *address, memory_order order) {
        std::int16_t value;
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_LOAD(__iso_volatile_load16, value, order, address);
        return value;
    }

    std::int32_t iso_volatile_load32_explicit(const volatile std::int32_t *address, memory_order order) {
        std::int32_t value;
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_LOAD(__iso_volatile_load32, value, order, address);
        return value;
    }

    std::int64_t iso_volatile_load64_explicit(const volatile long long *address, memory_order order) {
        std::int64_t value;
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_LOAD(__iso_volatile_load64, value, order, address);
        return value;
    }

    void iso_volatile_store_explicit(volatile void *address, void *value, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        iso_volatile_store64_explicit(reinterpret_cast<volatile long long *>(address), *static_cast<std::uint64_t *>(value), order);
#else
        iso_volatile_store32_explicit(reinterpret_cast<volatile int *>(address), *static_cast<std::uint32_t *>(value), order);
#endif
    }

    void iso_volatile_store8_explicit(volatile std::int8_t *address, std::int8_t value, memory_order order) {
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_STORE(__iso_volatile_store8, order, reinterpret_cast<volatile char *>(address), value);
    }

    void iso_volatile_store16_explicit(volatile std::int16_t *address, std::int16_t value, memory_order order) {
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_STORE(__iso_volatile_store16, order, address, value);
    }

    void iso_volatile_store32_explicit(volatile int *address, std::uint32_t value, memory_order order) {
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_STORE(__iso_volatile_store32, order, reinterpret_cast<volatile std::int32_t *>(address),
                                                 value);
    }

    void iso_volatile_store64_explicit(volatile long long *address, std::uint64_t value, memory_order order) {
        RAINY_ATOMIC_DISPATCH_ISO_VOLATILE_STORE(__iso_volatile_store64, order, address, value);
    }
}

#else

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

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC

#define RAINY_ARM64_ATOMIC_BEGIN(ptr, old)                                                                                            \
    volatile int _stxr_failed;                                                                                                        \
    do {                                                                                                                              \
        __asm__ __volatile__("ldaxr %0, [%1]" : "=&r"(old) : "r"(ptr) : "memory");

#define RAINY_ARM64_ATOMIC_END(ptr, newval)                                                                                           \
    __asm__ __volatile__("stlxr %w0, %2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(newval) : "memory");                             \
    }                                                                                                                                 \
    while (_stxr_failed)                                                                                                              \
        ;                                                                                                                             \
    rainy_dmb();

#define RAINY_ARM64_ATOMIC_OP_8(ptr, old_out, new_out, op)                                                                            \
    do {                                                                                                                              \
        uint8_t _old_val, _new_val;                                                                                                   \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxrb %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                         \
            _new_val = _old_val op;                                                                                                   \
            __asm__ __volatile__("stlxrb %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                 \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int8_t>(_old_val);                                                                                 \
        new_out = static_cast<std::int8_t>(_new_val);                                                                                 \
    } while (0)

#define RAINY_ARM64_ATOMIC_OP_16(ptr, old_out, new_out, op)                                                                           \
    do {                                                                                                                              \
        uint16_t _old_val, _new_val;                                                                                                  \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxrh %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                         \
            _new_val = _old_val op;                                                                                                   \
            __asm__ __volatile__("stlxrh %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                 \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int16_t>(_old_val);                                                                                \
        new_out = static_cast<std::int16_t>(_new_val);                                                                                \
    } while (0)

#define RAINY_ARM64_ATOMIC_OP_32(ptr, old_out, new_out, op)                                                                           \
    do {                                                                                                                              \
        uint32_t _old_val, _new_val;                                                                                                  \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxr %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                          \
            _new_val = _old_val op;                                                                                                   \
            __asm__ __volatile__("stlxr %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                  \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int32_t>(_old_val);                                                                                \
        new_out = static_cast<std::int32_t>(_new_val);                                                                                \
    } while (0)

#define RAINY_ARM64_ATOMIC_OP_64(ptr, old_out, new_out, op)                                                                           \
    do {                                                                                                                              \
        uint64_t _old_val, _new_val;                                                                                                  \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxr %0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                           \
            _new_val = _old_val op;                                                                                                   \
            __asm__ __volatile__("stlxr %w0, %2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                   \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int64_t>(_old_val);                                                                                \
        new_out = static_cast<std::int64_t>(_new_val);                                                                                \
    } while (0)

#define RAINY_ARM64_ATOMIC_OP_LONG(ptr, old_out, new_out, op)                                                                         \
    do {                                                                                                                              \
        unsigned long _old_val, _new_val;                                                                                             \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxr %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                          \
            _new_val = _old_val op;                                                                                                   \
            __asm__ __volatile__("stlxr %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                  \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<long>(_old_val);                                                                                        \
        new_out = static_cast<long>(_new_val);                                                                                        \
    } while (0)

#define RAINY_ARM64_COMPARE_EXCHANGE(dest, exchange, comparand, result)                                                               \
    do {                                                                                                                              \
        volatile int _stxr_failed;                                                                                                    \
        __typeof__(comparand) _old;                                                                                                   \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxr %0, [%1]" : "=&r"(_old) : "r"(dest) : "memory");                                              \
            if (_old != (comparand)) {                                                                                                \
                __asm__ __volatile__("clrex" ::: "memory");                                                                           \
                rainy_dmb();                                                                                                          \
                result = false;                                                                                                       \
                break;                                                                                                                \
            }                                                                                                                         \
            __asm__ __volatile__("stlxr %w0, %2, [%1]" : "=&r"(_stxr_failed) : "r"(dest), "r"(exchange) : "memory");                  \
        } while (_stxr_failed);                                                                                                       \
        if (_old == (comparand)) {                                                                                                    \
            rainy_dmb();                                                                                                              \
            result = true;                                                                                                            \
        }                                                                                                                             \
    } while (0)

#define RAINY_ARM64_ATOMIC_EXCHANGE_8(ptr, new_val, old_out)                                                                          \
    do {                                                                                                                              \
        uint8_t _old_val;                                                                                                             \
        uint8_t _new_val = static_cast<uint8_t>(new_val);                                                                             \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxrb %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                         \
            __asm__ __volatile__("stlxrb %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                 \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int8_t>(_old_val);                                                                                 \
    } while (0)

#define RAINY_ARM64_ATOMIC_EXCHANGE_16(ptr, new_val, old_out)                                                                         \
    do {                                                                                                                              \
        uint16_t _old_val;                                                                                                            \
        uint16_t _new_val = static_cast<uint16_t>(new_val);                                                                           \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxrh %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                         \
            __asm__ __volatile__("stlxrh %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                 \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int16_t>(_old_val);                                                                                \
    } while (0)

#define RAINY_ARM64_ATOMIC_EXCHANGE_32(ptr, new_val, old_out)                                                                         \
    do {                                                                                                                              \
        uint32_t _old_val;                                                                                                            \
        uint32_t _new_val = static_cast<uint32_t>(new_val);                                                                           \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxr %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                          \
            __asm__ __volatile__("stlxr %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                  \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int32_t>(_old_val);                                                                                \
    } while (0)

#define RAINY_ARM64_ATOMIC_EXCHANGE_64(ptr, new_val, old_out)                                                                         \
    do {                                                                                                                              \
        uint64_t _old_val;                                                                                                            \
        uint64_t _new_val = static_cast<uint64_t>(new_val);                                                                           \
        volatile int _stxr_failed;                                                                                                    \
        do {                                                                                                                          \
            __asm__ __volatile__("ldaxr %0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                           \
            __asm__ __volatile__("stlxr %w0, %2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");                   \
        } while (_stxr_failed);                                                                                                       \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<std::int64_t>(_old_val);                                                                                \
    } while (0)

#define RAINY_ARM64_ATOMIC_EXCHANGE_LONG(ptr, new_val, old_out)                                                                       \
    do {                                                                                                                              \
        unsigned long _old_val;                                                                                                       \
        unsigned long _new_val = static_cast<unsigned long>(new_val);                                                                 \
        volatile int _stxr_failed;                                                                                                    \
        if constexpr (sizeof(long) == 8) {                                                                                            \
            do {                                                                                                                      \
                __asm__ __volatile__("ldaxr %0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                       \
                __asm__ __volatile__("stlxr %w0, %2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");               \
            } while (_stxr_failed);                                                                                                   \
        } else {                                                                                                                      \
            do {                                                                                                                      \
                __asm__ __volatile__("ldaxr %w0, [%1]" : "=&r"(_old_val) : "r"(ptr) : "memory");                                      \
                __asm__ __volatile__("stlxr %w0, %w2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(_new_val) : "memory");              \
            } while (_stxr_failed);                                                                                                   \
        }                                                                                                                             \
        rainy_dmb();                                                                                                                  \
        old_out = static_cast<long>(_old_val);                                                                                        \
    } while (0)


#endif

namespace rainy::core::pal {
    long interlocked_increment_explicit(volatile long *value, memory_order order) {
        long old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_LONG(value, old_val, new_val, +1);
        return new_val;
    }

    std::int8_t interlocked_increment8_explicit(volatile std::int8_t *value, memory_order order) {
        std::int8_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_8(value, old_val, new_val, +1);
        return new_val;
    }

    std::int16_t interlocked_increment16_explicit(volatile std::int16_t *value, memory_order order) {
        std::int16_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_16(value, old_val, new_val, +1);
        return new_val;
    }

    std::int32_t interlocked_increment32_explicit(volatile std::int32_t *value, memory_order order) {
        std::int32_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_32(value, old_val, new_val, +1);
        return new_val;
    }

    std::int64_t interlocked_increment64_explicit(volatile std::int64_t *value, memory_order order) {
        std::int64_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_64(value, old_val, new_val, +1);
        return new_val;
    }

    long interlocked_decrement_explicit(volatile long *value, memory_order order) {
        long old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_LONG(value, old_val, new_val, -1);
        return new_val;
    }

    std::int8_t interlocked_decrement8_explicit(volatile std::int8_t *value, memory_order order) {
        std::int8_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_8(value, old_val, new_val, -1);
        return new_val;
    }

    std::int16_t interlocked_decrement16_explicit(volatile std::int16_t *value, memory_order order) {
        std::int16_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_16(value, old_val, new_val, -1);
        return new_val;
    }

    std::int32_t interlocked_decrement32_explicit(volatile std::int32_t *value, memory_order order) {
        std::int32_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_32(value, old_val, new_val, -1);
        return new_val;
    }

    std::int64_t interlocked_decrement64_explicit(volatile std::int64_t *value, memory_order order) {
        std::int64_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_64(value, old_val, new_val, -1);
        return new_val;
    }

    std::intptr_t interlocked_exchange_add_explicit(volatile std::intptr_t *value, const std::intptr_t amount, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange_add64_explicit(reinterpret_cast<volatile std::int64_t *>(value), amount, order);
#else
        return interlocked_exchange_add32_explicit(reinterpret_cast<volatile std::int32_t *>(value), amount, order);
#endif
    }

    std::int8_t interlocked_exchange_add8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order) {
        std::int8_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_8(value, old_val, new_val, +amount);
        return old_val;
    }

    std::int16_t interlocked_exchange_add16_explicit(volatile std::int16_t *value, std::int16_t amount, memory_order order) {
        std::int16_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_16(value, old_val, new_val, +amount);
        return old_val;
    }

    std::int32_t interlocked_exchange_add32_explicit(volatile std::int32_t *value, std::int32_t amount, memory_order order) {
        std::int32_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_32(value, old_val, new_val, +amount);
        return old_val;
    }

    std::int64_t interlocked_exchange_add64_explicit(volatile std::int64_t *value, std::int64_t amount, memory_order order) {
        std::int64_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_64(value, old_val, new_val, +amount);
        return old_val;
    }

    std::intptr_t interlocked_exchange_subtract_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                         memory_order order) {
        return interlocked_exchange_add_explicit(value, -amount, order);
    }

    std::int8_t interlocked_exchange_subtract8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order) {
        return interlocked_exchange_add8_explicit(value, -amount, order);
    }

    std::int16_t interlocked_exchange_subtract16_explicit(volatile std::int16_t *value, std::int16_t amount, memory_order order) {
        return interlocked_exchange_add16_explicit(value, -amount, order);
    }

    std::int32_t interlocked_exchange_subtract32_explicit(volatile std::int32_t *value, std::int32_t amount, memory_order order) {
        return interlocked_exchange_add32_explicit(value, -amount, order);
    }

    std::int64_t interlocked_exchange_subtract64_explicit(volatile std::int64_t *value, std::int64_t amount, memory_order order) {
        return interlocked_exchange_add64_explicit(value, -amount, order);
    }

    std::intptr_t interlocked_exchange_explicit(volatile std::intptr_t *target, std::intptr_t value, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange64_explicit(reinterpret_cast<volatile std::int64_t *>(target), value, order);
#else
        return interlocked_exchange32_explicit(reinterpret_cast<volatile std::int32_t *>(target), value, order);
#endif
    }

    std::int8_t interlocked_exchange8_explicit(volatile std::int8_t *target, std::int8_t value, memory_order order) {
        std::int8_t old_val;
        RAINY_ARM64_ATOMIC_EXCHANGE_8(target, value, old_val);
        return old_val;
    }

    std::int16_t interlocked_exchange16_explicit(volatile std::int16_t *target, std::int16_t value, memory_order order) {
        std::int16_t old_val;
        RAINY_ARM64_ATOMIC_EXCHANGE_16(target, value, old_val);
        return old_val;
    }

    std::int32_t interlocked_exchange32_explicit(volatile std::int32_t *target, std::int32_t value, memory_order order) {
        std::int32_t old_val;
        RAINY_ARM64_ATOMIC_EXCHANGE_32(target, value, old_val);
        return old_val;
    }

    std::int64_t interlocked_exchange64_explicit(volatile std::int64_t *target, std::int64_t value, memory_order order) {
        std::int64_t old_val;
        RAINY_ARM64_ATOMIC_EXCHANGE_64(target, value, old_val);
        return old_val;
    }

    void *interlocked_exchange_pointer_explicit(volatile void **target, void *value, memory_order order) {
        void *old_val;
#if RAINY_USING_64_BIT_PLATFORM
        RAINY_ARM64_ATOMIC_EXCHANGE_64(reinterpret_cast<volatile std::int64_t *>(target), reinterpret_cast<std::int64_t>(value),
                                       *reinterpret_cast<std::int64_t *>(&old_val));
#else
        RAINY_ARM64_ATOMIC_EXCHANGE_32(reinterpret_cast<volatile std::int32_t *>(target), reinterpret_cast<std::int32_t>(value),
                                       *reinterpret_cast<std::int32_t *>(&old_val));
#endif
        return old_val;
    }

    bool interlocked_compare_exchange_explicit(volatile long *destination, long exchange, long comparand, memory_order success,
                                               memory_order failure) {
        bool result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
    }

    bool interlocked_compare_exchange8_explicit(volatile std::int8_t *destination, std::int8_t exchange, std::int8_t comparand,
                                                memory_order success, memory_order failure) {
        bool result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
    }

    bool interlocked_compare_exchange16_explicit(volatile std::int16_t *destination, std::int16_t exchange, std::int16_t comparand,
                                                 memory_order success, memory_order failure) {
        bool result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
    }

    bool interlocked_compare_exchange32_explicit(volatile std::int32_t *destination, std::int32_t exchange, std::int32_t comparand,
                                                 memory_order success, memory_order failure) {
        bool result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
    }

    bool interlocked_compare_exchange64_explicit(volatile std::int64_t *destination, std::int64_t exchange, std::int64_t comparand,
                                                 memory_order success, memory_order failure) {
        bool result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
    }

    void *interlocked_compare_exchange_pointer_explicit(volatile void **destination, void *exchange, void *comparand,
                                                        memory_order success, memory_order failure) {
        rainy_assume(destination);
        void *old;
        unsigned int tmp;
        __asm__ __volatile__("1:\n"
                             "ldxr   %0, [%3]\n" // old = *destination
                             "cmp    %0, %4\n" // old == comparand ?
                             "b.ne   2f\n" // 不相等直接退出
                             "stxr   %w1, %5, [%3]\n" // 尝试写 exchange
                             "cbnz   %w1, 1b\n" // 写失败则重试
                             "2:\n"
                             : "=&r"(old), "=&r"(tmp), "+m"(*destination)
                             : "r"(destination), "r"(comparand), "r"(exchange)
                             : "cc", "memory");
        if (old == comparand) {
            if (success == memory_order_seq_cst || success == memory_order_release || success == memory_order_acq_rel) {
                __asm__ __volatile__("dmb ish" ::: "memory");
            }
        } else {
            if (failure == memory_order_seq_cst || failure == memory_order_acquire) {
                __asm__ __volatile__("dmb ish" ::: "memory");
            }
        }
        return old;
    }


    std::intptr_t interlocked_and_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_and64_explicit(reinterpret_cast<volatile std::int64_t *>(value), mask, order);
#else
        return interlocked_and32_explicit(reinterpret_cast<volatile std::int32_t *>(value), mask, order);
#endif
    }

    std::int8_t interlocked_and8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_8(value, old_val, new_val, &mask);
        return new_val;
    }

    std::int16_t interlocked_and16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_16(value, old_val, new_val, &mask);
        return new_val;
    }

    std::int32_t interlocked_and32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_32(value, old_val, new_val, &mask);
        return new_val;
    }

    std::int64_t interlocked_and64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_64(value, old_val, new_val, &mask);
        return new_val;
    }

    // ========== bitwise OR operations ==========
    std::intptr_t interlocked_or_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_or64_explicit(reinterpret_cast<volatile std::int64_t *>(value), mask, order);
#else
        return interlocked_or32_explicit(reinterpret_cast<volatile std::int32_t *>(value), mask, order);
#endif
    }

    std::int8_t interlocked_or8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_8(value, old_val, new_val, | mask);
        return new_val;
    }

    std::int16_t interlocked_or16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_16(value, old_val, new_val, | mask);
        return new_val;
    }

    std::int32_t interlocked_or32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_32(value, old_val, new_val, | mask);
        return new_val;
    }

    std::int64_t interlocked_or64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_64(value, old_val, new_val, | mask);
        return new_val;
    }

    // ========== bitwise XOR operations ==========
    std::intptr_t interlocked_xor_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_xor64_explicit(reinterpret_cast<volatile std::int64_t *>(value), mask, order);
#else
        return interlocked_xor32_explicit(reinterpret_cast<volatile std::int32_t *>(value), mask, order);
#endif
    }

    std::int8_t interlocked_xor8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_8(value, old_val, new_val, ^mask);
        return new_val;
    }

    std::int16_t interlocked_xor16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_16(value, old_val, new_val, ^mask);
        return new_val;
    }

    std::int32_t interlocked_xor32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_32(value, old_val, new_val, ^mask);
        return new_val;
    }

    std::int64_t interlocked_xor64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t old_val, new_val;
        RAINY_ARM64_ATOMIC_OP_64(value, old_val, new_val, ^mask);
        return new_val;
    }

    // ========== atomic load operations ==========
    std::intptr_t iso_volatile_load_explicit(const volatile std::intptr_t *address, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return iso_volatile_load64_explicit(reinterpret_cast<const volatile long long *>(address), order);
#else
        return iso_volatile_load32_explicit(reinterpret_cast<const volatile std::int32_t *>(address), order);
#endif
    }

    std::int8_t iso_volatile_load8_explicit(const volatile std::int8_t *address, memory_order order) {
        rainy_assume(address);
        std::int8_t value = *address;
        if (order == memory_order_acquire || order == memory_order_seq_cst) {
            rainy_dmb_ld();
        }
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
        return value;
    }

    std::int16_t iso_volatile_load16_explicit(const volatile std::int16_t *address, memory_order order) {
        rainy_assume(address);
        std::int16_t value = *address;
        if (order == memory_order_acquire || order == memory_order_seq_cst) {
            rainy_dmb_ld();
        }
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
        return value;
    }

    std::int32_t iso_volatile_load32_explicit(const volatile std::int32_t *address, memory_order order) {
        rainy_assume(address);
        std::int32_t value = *address;
        if (order == memory_order_acquire || order == memory_order_seq_cst) {
            rainy_dmb_ld();
        }
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
        return value;
    }

    std::int64_t iso_volatile_load64_explicit(const volatile long long *address, memory_order order) {
        rainy_assume(address);
        std::int64_t value = *address;
        if (order == memory_order_acquire || order == memory_order_seq_cst) {
            rainy_dmb_ld();
        }
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
        return value;
    }

    // ========== atomic store operations ==========
    void iso_volatile_store_explicit(volatile void *address, void *value, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        iso_volatile_store64_explicit(reinterpret_cast<volatile long long *>(address), *static_cast<std::uint64_t *>(value), order);
#else
        iso_volatile_store32_explicit(reinterpret_cast<volatile int *>(address), *static_cast<std::uint32_t *>(value), order);
#endif
    }

    void iso_volatile_store8_explicit(volatile std::int8_t *address, std::int8_t value, memory_order order) {
        rainy_assume(address);
        if (order == memory_order_release || order == memory_order_seq_cst) {
            rainy_dmb_st();
        }
        *address = value;
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
    }

    void iso_volatile_store16_explicit(volatile std::int16_t *address, std::int16_t value, memory_order order) {
        rainy_assume(address);
        if (order == memory_order_release || order == memory_order_seq_cst) {
            rainy_dmb_st();
        }
        *address = value;
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
    }

    void iso_volatile_store32_explicit(volatile int *address, std::uint32_t value, memory_order order) {
        rainy_assume(address);
        if (order == memory_order_release || order == memory_order_seq_cst) {
            rainy_dmb_st();
        }
        *address = value;
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
    }

    void iso_volatile_store64_explicit(volatile long long *address, std::uint64_t value, memory_order order) {
        rainy_assume(address);
        if (order == memory_order_release || order == memory_order_seq_cst) {
            rainy_dmb_st();
        }
        *address = value;
        if (order == memory_order_seq_cst) {
            rainy_dmb();
        }
    }
}

#endif
