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

#if RAINY_USING_MSVC
#if RAINY_IS_X86_PLATFORM
#include <emmintrin.h>
#endif
#include <intrin.h>
#endif

#define RAINY_ATOMIC_BEGIN(order)                                                                                                     \
    do {                                                                                                                              \
        if ((order) == memory_order_release) {                                                                                        \
            write_barrier();                                                                                                          \
        } else if ((order) == memory_order_seq_cst) {                                                                                 \
            read_write_barrier();                                                                                                     \
        }                                                                                                                             \
    } while (0)

#define RAINY_ATOMIC_END(order)                                                                                                       \
    do {                                                                                                                              \
        if ((order) == memory_order_acquire) {                                                                                        \
            read_barrier();                                                                                                           \
        } else if ((order) == memory_order_seq_cst) {                                                                                 \
            read_write_barrier();                                                                                                     \
        }                                                                                                                             \
    } while (0)


namespace rainy::core::pal {
    long interlocked_increment_explicit(volatile long *value, memory_order order) {
        long result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedIncrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clang_tidy = value;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clang_tidy) : : "cc");
        result = *avoid_clang_tidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    long interlocked_decrement_explicit(volatile long *value, memory_order order) {
        long result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedDecrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clang_tidy = value;
        __asm__ __volatile__("lock; decl %0" : "+m"(*avoid_clang_tidy) : : "cc");
        result = *avoid_clang_tidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int8_t interlocked_increment8_explicit(volatile std::int8_t *value, memory_order order) {
        std::int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = interlocked_exchange_add8(value, +1) + 1;
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incb %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int16_t interlocked_increment16_explicit(volatile std::int16_t *value, memory_order order) {
        std::int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedIncrement16(value);
#else
        volatile std::int16_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incw %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int32_t interlocked_increment32_explicit(volatile std::int32_t *value, memory_order order) {
        std::int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedIncrement(reinterpret_cast<volatile long *>(value));
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incw %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int64_t interlocked_increment64_explicit(volatile std::int64_t *value, memory_order order) {
        std::int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedIncrement64(value);
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incq %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int8_t interlocked_decrement8_explicit(volatile std::int8_t *value, memory_order order) {
        std::int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = interlocked_exchange_add8(value, -1) - 1;
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decb %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int16_t interlocked_decrement16_explicit(volatile std::int16_t *value, memory_order order) {
        std::int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedDecrement16(value);
#else
        volatile std::int16_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decw %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int32_t interlocked_decrement32_explicit(volatile std::int32_t *value, memory_order order) {
        std::int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedDecrement(reinterpret_cast<volatile long *>(value));
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decw %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int64_t interlocked_decrement64_explicit(volatile std::int64_t *value, memory_order order) {
        std::int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedDecrement64(value);
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decq %0" : "+m"(*avoid_clangtidy) : : "cc");
        result = *avoid_clangtidy;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }
}

namespace rainy::core::pal {

    std::intptr_t interlocked_exchange_add_explicit(volatile std::intptr_t *value, const std::intptr_t amount, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange_add64_explicit(reinterpret_cast<volatile std::int64_t *>(value), static_cast<std::int64_t>(amount),
                                                   order);
#else
        return interlocked_exchange_add32_explicit(reinterpret_cast<volatile std::int32_t *>(value), static_cast<std::int32_t>(amount),
                                                   order);
#endif
    }

    std::int8_t interlocked_exchange_add8_explicit(volatile std::int8_t *value, std::int8_t amount, memory_order order) {
        std::int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchangeAdd8(reinterpret_cast<volatile char *>(value), amount);
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddb %0, %1" : "+q"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        result = amount;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int16_t interlocked_exchange_add16_explicit(volatile std::int16_t *value, std::int16_t amount, memory_order order) {
        std::int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchangeAdd16(reinterpret_cast<volatile short *>(value), amount);
#else
        volatile std::int16_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddw %0, %1" : "+r"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        result = amount;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int32_t interlocked_exchange_add32_explicit(volatile std::int32_t *value, std::int32_t amount, memory_order order) {
        std::int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(value), amount);
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddl %0, %1" : "+r"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        result = amount;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

#if RAINY_USING_64_BIT_PLATFORM
    std::int64_t interlocked_exchange_add64_explicit(volatile std::int64_t *value, std::int64_t amount, memory_order order) {
        std::int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchangeAdd64(reinterpret_cast<volatile __int64 *>(value), amount);
#else
        volatile std::int64_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddq %0, %1" : "+r"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        result = amount;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }
#endif
}

namespace rainy::core::pal {
    std::intptr_t interlocked_exchange_subtract_explicit(volatile std::intptr_t *value, const std::intptr_t amount,
                                                         memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange_subtract64_explicit(static_cast<volatile std::int64_t *>(value),
                                                        static_cast<const std::int64_t>(amount), order);
#else
        return interlocked_exchange_subtract32_explicit(static_cast<volatile std::int32_t *>(value),
                                                        static_cast<const std::int32_t>(amount), order);
#endif
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
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange64_explicit(reinterpret_cast<volatile std::int64_t *>(target), static_cast<std::int64_t>(value),
                                               order);
#else
        return interlocked_exchange32_explicit(reinterpret_cast<volatile std::int32_t *>(target), static_cast<std::int32_t>(value),
                                               order);
#endif
    }

    std::int8_t interlocked_exchange8_explicit(volatile std::int8_t *target, std::int8_t value, memory_order order) {
        std::int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchange8(reinterpret_cast<volatile char *>(target), value);
#else
        __asm__ __volatile__("lock xchgb %0, %1" : "=q"(value), "+m"(*target) : "0"(value) : "memory");
        result = value;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int16_t interlocked_exchange16_explicit(volatile std::int16_t *target, std::int16_t value, memory_order order) {
        std::int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchange16(target, value);
#else
        __asm__ __volatile__("lock xchgw %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        result = value;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int32_t interlocked_exchange32_explicit(volatile std::int32_t *target, std::int32_t value, memory_order order) {
        std::int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchange(reinterpret_cast<volatile long *>(target), static_cast<long>(value));
#else
        __asm__ __volatile__("lock xchgl %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        result = value;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int64_t interlocked_exchange64_explicit(volatile std::int64_t *target, std::int64_t value, memory_order order) {
        std::int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedExchange64(reinterpret_cast<volatile __int64 *>(target), value);
#else
        __asm__ __volatile__("lock xchgq %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        result = value;
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }
}


namespace rainy::core::pal {
    void* interlocked_exchange_pointer_explicit(volatile void **target, void *value, memory_order order) {
        void *old;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        old = _InterlockedExchangePointer(const_cast<void *volatile *>(target), value);
#else
        __asm__ __volatile__("lock xchg %1, %0" : "=r"(old), "+m"(*target) : "0"(value) : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return old;
    }

    bool interlocked_compare_exchange_explicit(volatile long *destination, long exchange, long comparand, memory_order success,
                                               memory_order failure) {
        bool result;
        RAINY_ATOMIC_BEGIN(success);
#if RAINY_USING_MSVC
        result = _InterlockedCompareExchange(destination, exchange, comparand) == comparand;
#else
        char r;
        __asm__ __volatile__("lock cmpxchg %3, %1\n\t"
                             "sete %0"
                             : "=q"(r), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        result = r;
#endif
        RAINY_ATOMIC_END(result ? success : failure);
        return result;
    }

    bool interlocked_compare_exchange8_explicit(volatile std::int8_t *destination, std::int8_t exchange, std::int8_t comparand,
                                                memory_order success, memory_order failure) {
        bool result;
        RAINY_ATOMIC_BEGIN(success);
#if RAINY_USING_MSVC
        result = _InterlockedCompareExchange8(reinterpret_cast<volatile char *>(destination), exchange, comparand);
#else
        char r;
        __asm__ __volatile__("lock cmpxchgb %2, %1\n\t"
                             "sete %0"
                             : "=q"(r), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        result = r;
#endif
        RAINY_ATOMIC_END(result ? success : failure);
        return result;
    }

    bool interlocked_compare_exchange16_explicit(volatile std::int16_t *destination, std::int16_t exchange, std::int16_t comparand,
                                                 memory_order success, memory_order failure) {
        bool result;
        RAINY_ATOMIC_BEGIN(success);
#if RAINY_USING_MSVC
        result = _InterlockedCompareExchange16(destination, exchange, comparand);
#else
        char r;
        __asm__ __volatile__("lock cmpxchgw %2, %1\n\t"
                             "sete %0"
                             : "=q"(r), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        result = r;
#endif
        RAINY_ATOMIC_END(result ? success : failure);
        return result;
    }

    bool interlocked_compare_exchange32_explicit(volatile std::int32_t *destination, std::int32_t exchange, std::int32_t comparand,
                                                 memory_order success, memory_order failure) {
        bool result;
        RAINY_ATOMIC_BEGIN(success);
#if RAINY_USING_MSVC
        result = _InterlockedCompareExchange(reinterpret_cast<volatile long *>(destination), exchange, comparand) == comparand;
#else
        char r;
        __asm__ __volatile__("lock cmpxchgl %2, %1\n\t"
                             "sete %0"
                             : "=q"(r), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        result = r;
#endif
        RAINY_ATOMIC_END(result ? success : failure);
        return result;
    }

    bool interlocked_compare_exchange64_explicit(volatile std::int64_t *destination, std::int64_t exchange, std::int64_t comparand,
                                                 memory_order success, memory_order failure) {
        bool result;
        RAINY_ATOMIC_BEGIN(success);
#if RAINY_USING_MSVC
        result = _InterlockedCompareExchange64(destination, exchange, comparand) == comparand;
#else
        char r;
#if RAINY_USING_64_BIT_PLATFORM
        __asm__ __volatile__("lock cmpxchg %3, %1\n\t"
                             "sete %0"
                             : "=q"(r), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
#else
        std::int32_t comparand_high = static_cast<std::int32_t>(comparand >> 32);
        std::int32_t comparand_low = static_cast<std::int32_t>(comparand);
        std::int32_t exchange_high = static_cast<std::int32_t>(exchange >> 32);
        std::int32_t exchange_low = static_cast<std::int32_t>(exchange);
        __asm__ __volatile__("lock cmpxchg8b %1\n\t"
                             "sete %0"
                             : "=q"(r), "+m"(*destination), "+d"(comparand_high), "+a"(comparand_low)
                             : "c"(exchange_high), "b"(exchange_low)
                             : "memory");
#endif
        result = r;
#endif
        RAINY_ATOMIC_END(result ? success : failure);
        return result;
    }

    void *interlocked_compare_exchange_pointer_explicit(volatile void **destination, void *exchange, void *comparand,
                                                        memory_order success, memory_order failure) {
        void *result;
        bool exchanged;
        RAINY_ATOMIC_BEGIN(success);
#if RAINY_USING_MSVC
        result = _InterlockedCompareExchangePointer(const_cast<void *volatile *>(destination), exchange, comparand);
        exchanged = (result == comparand);
#else
        __asm__ __volatile__("lock cmpxchg %3, %1\n\t"
                             "sete %0"
                             : "=q"(exchanged), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        result = comparand;
#endif
        RAINY_ATOMIC_END(exchanged ? success : failure);
        return result;
    }

    std::intptr_t interlocked_and_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_and64_explicit(reinterpret_cast<volatile std::int64_t *>(value), static_cast<std::int64_t>(mask), order);
#else
        return interlocked_and32_explicit(reinterpret_cast<volatile std::int32_t *>(value), static_cast<std::int32_t>(mask), order);
#endif
    }

    int8_t interlocked_and8_explicit(volatile int8_t *value, int8_t mask, memory_order order) {
        int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedAnd8(reinterpret_cast<volatile char *>(value), mask);
#else
        __asm__ __volatile__("movb %1, %0\n\t"
                             "lock andb %2, %1"
                             : "=&q"(result), "+m"(*value)
                             : "iq"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    int16_t interlocked_and16_explicit(volatile int16_t *value, int16_t mask, memory_order order) {
        int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedAnd16(reinterpret_cast<volatile short *>(value), mask);
#else
        __asm__ __volatile__("movw %1, %0\n\t"
                             "lock andw %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    int32_t interlocked_and32_explicit(volatile int32_t *value, int32_t mask, memory_order order) {
        int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedAnd(reinterpret_cast<volatile long *>(value), mask);
#else
        __asm__ __volatile__("movl %1, %0\n\t"
                             "lock andl %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    int64_t interlocked_and64_explicit(volatile int64_t *value, int64_t mask, memory_order order) {
        int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedAnd64(reinterpret_cast<volatile __int64 *>(value), mask);
#else
        __asm__ __volatile__("movq %1, %0\n\t"
                             "lock andq %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::intptr_t interlocked_or_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_or64_explicit(reinterpret_cast<volatile std::int64_t *>(value), static_cast<std::int64_t>(mask), order);
#else
        return interlocked_or32_explicit(reinterpret_cast<volatile std::int32_t *>(value), static_cast<std::int32_t>(mask), order);
#endif
    }

    std::int8_t interlocked_or8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedOr8(reinterpret_cast<volatile char *>(value), mask);
#else
        __asm__ __volatile__("movb %1, %0\n\t"
                             "lock orb %2, %1"
                             : "=&q"(result), "+m"(*value)
                             : "iq"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int16_t interlocked_or16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedOr16(reinterpret_cast<volatile short *>(value), mask);
#else
        __asm__ __volatile__("movw %1, %0\n\t"
                             "lock orw %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int32_t interlocked_or32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedOr(reinterpret_cast<volatile long *>(value), mask);
#else
        __asm__ __volatile__("movl %1, %0\n\t"
                             "lock orl %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int64_t interlocked_or64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedOr64(reinterpret_cast<volatile __int64 *>(value), mask);
#else
        __asm__ __volatile__("movq %1, %0\n\t"
                             "lock orq %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::intptr_t interlocked_xor_explicit(volatile std::intptr_t *value, std::intptr_t mask, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_xor64_explicit(reinterpret_cast<volatile std::int64_t *>(value), static_cast<std::int64_t>(mask), order);
#else
        return interlocked_xor32_explicit(reinterpret_cast<volatile std::int32_t *>(value), static_cast<std::int32_t>(mask), order);
#endif
    }

    std::int8_t interlocked_xor8_explicit(volatile std::int8_t *value, std::int8_t mask, memory_order order) {
        std::int8_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedXor8(reinterpret_cast<volatile char *>(value), mask);
#else
        __asm__ __volatile__("movb %1, %0\n\t"
                             "lock xorb %2, %1"
                             : "=&q"(result), "+m"(*value)
                             : "iq"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int16_t interlocked_xor16_explicit(volatile std::int16_t *value, std::int16_t mask, memory_order order) {
        std::int16_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedXor16(reinterpret_cast<volatile short *>(value), mask);
#else
        __asm__ __volatile__("movw %1, %0\n\t"
                             "lock xorw %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int32_t interlocked_xor32_explicit(volatile std::int32_t *value, std::int32_t mask, memory_order order) {
        std::int32_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedXor(reinterpret_cast<volatile long *>(value), mask);
#else
        __asm__ __volatile__("movl %1, %0\n\t"
                             "lock xorl %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }

    std::int64_t interlocked_xor64_explicit(volatile std::int64_t *value, std::int64_t mask, memory_order order) {
        std::int64_t result;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        result = _InterlockedXor64(reinterpret_cast<volatile __int64 *>(value), mask);
#else
        __asm__ __volatile__("movq %1, %0\n\t"
                             "lock xorq %2, %1"
                             : "=&r"(result), "+m"(*value)
                             : "r"(mask)
                             : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return result;
    }


    std::intptr_t iso_volatile_load_explicit(const volatile std::intptr_t *address, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        return iso_volatile_load64_explicit(reinterpret_cast<const volatile long long *>(address), order);
#else
        return iso_volatile_load32_explicit(reinterpret_cast<const volatile std::int32_t *>(address), order);
#endif
    }

    std::int8_t iso_volatile_load8_explicit(const volatile std::int8_t *address, memory_order order) {
        std::int8_t value;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        value = __iso_volatile_load8(reinterpret_cast<const volatile char *>(address));
#else
        __asm__ __volatile__("lfence\n\tmovb (%1), %0" : "=r"(value) : "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return value;
    }

    std::int16_t iso_volatile_load16_explicit(const volatile std::int16_t *address, memory_order order) {
        std::int16_t value;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        value = __iso_volatile_load16(address);
#else
        __asm__ __volatile__("lfence\n\tmovw (%1), %0" : "=r"(value) : "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return value;
    }

    std::int32_t iso_volatile_load32_explicit(const volatile std::int32_t *address, memory_order order) {
        std::int32_t value;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        value = __iso_volatile_load32(address);
#else
        __asm__ __volatile__("lfence\n\tmovl (%1), %0" : "=r"(value) : "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return value;
    }

    std::int64_t iso_volatile_load64_explicit(const volatile long long *address, memory_order order) {
        std::int64_t value;
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        value = __iso_volatile_load64(address);
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        __asm__ __volatile__("lfence\n\tmovq (%1), %0" : "=r"(value) : "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
        return value;
    }

    void iso_volatile_store_explicit(volatile void *address, void *value, memory_order order) {
#if RAINY_USING_64_BIT_PLATFORM
        iso_volatile_store64_explicit(static_cast<volatile long long *>(address), *static_cast<std::uint64_t *>(value), order);
#else
        iso_volatile_store32_explicit(static_cast<volatile int *>(address), *static_cast<std::uint32_t *>(value), order);
#endif
    }

    void iso_volatile_store8_explicit(volatile std::int8_t *address, std::int8_t value, memory_order order) {
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        *address = value;
#else
        __asm__ __volatile__("movb %0, (%1)" : : "r"(value), "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
    }

    void iso_volatile_store16_explicit(volatile std::int16_t *address, std::int16_t value, memory_order order) {
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        *address = value;
#else
        __asm__ __volatile__("movw %0, (%1)" : : "r"(value), "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
    }

    void iso_volatile_store32_explicit(volatile int *address, std::uint32_t value, memory_order order) {
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
        _InterlockedExchange(reinterpret_cast<volatile long *>(address), static_cast<long>(value));
#else
        __asm__ __volatile__("movl %0, (%1)" : : "r"(value), "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
    }

    void iso_volatile_store64_explicit(volatile long long *address, std::uint64_t value, memory_order order) {
        RAINY_ATOMIC_BEGIN(order);
#if RAINY_USING_MSVC
#if RAINY_USING_64_BIT_PLATFORM
        _InterlockedExchange64(address, value);
#else
        ::_interlockedexchange64(address, value);
#endif
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        __asm__ __volatile__("movq %0, (%1)" : : "r"(value), "r"(address) : "memory");
#endif
        RAINY_ATOMIC_END(order);
    }
}
