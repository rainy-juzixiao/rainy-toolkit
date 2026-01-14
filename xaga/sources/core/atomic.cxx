/*
 * Copyright 2025 rainy-juzixiao
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
#include <emmintrin.h>
#include <intrin.h>
#endif

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

// arm64的测试代码，用于评估是否可以将rainy-toolkit pal移植到arm64架构上
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC

#define RAINY_ARM64_ATOMIC_BEGIN(ptr, old)                                                                                            \
    int _stxr_failed;                                                                                                                 \
    do {                                                                                                                              \
        __asm__ __volatile__("ldaxr %0, [%1]" : "=&r"(old) : "r"(ptr) : "memory");

#define RAINY_ARM64_ATOMIC_END(ptr, newval)                                                                                           \
    __asm__ __volatile__("stlxr %w0, %2, [%1]" : "=&r"(_stxr_failed) : "r"(ptr), "r"(newval) : "memory");                             \
    }                                                                                                                                 \
    while (_stxr_failed)                                                                                                              \
        ;                                                                                                                             \
    rainy_dmb();

#endif

namespace rainy::core::pal {
    long interlocked_increment(volatile long *value) {
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        rainy_assume(value);
        long old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#else
        #if RAINY_USING_MSVC
        return _InterlockedIncrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clang_tidy = value;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clang_tidy) : : "cc");
        return *avoid_clang_tidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
#endif
    }

    long interlocked_decrement(volatile long *value) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        long old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old - 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#else
#if RAINY_USING_MSVC
        return _InterlockedDecrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decl %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
#endif
    }
}

namespace rainy::core::pal {
    std::int8_t interlocked_increment8(volatile std::int8_t *value) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return interlocked_exchange_add8(reinterpret_cast<volatile std::int8_t *>(value), 1);
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incb %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int16_t interlocked_increment16(volatile std::int16_t *value) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int16_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return _InterlockedIncrement16(reinterpret_cast<volatile short *>(value));
#else
        volatile std::int16_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incw %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int32_t interlocked_increment32(volatile std::int32_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int32_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return _InterlockedIncrement(reinterpret_cast<volatile long *>(value));
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int64_t interlocked_increment64(volatile std::int64_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return _InterlockedIncrement64(reinterpret_cast<volatile __int64 *>(value));
#else
        volatile std::int64_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incq %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }
}

namespace rainy::core::pal {
    std::int8_t interlocked_decrement8(volatile std::int8_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old - 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return interlocked_exchange_add8(reinterpret_cast<volatile std::int8_t *>(value), -1);
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decb %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int16_t interlocked_decrement16(volatile std::int16_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int16_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old - 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return _InterlockedDecrement16(reinterpret_cast<volatile short *>(value));
#else
        volatile std::int16_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decw %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int32_t interlocked_decrement32(volatile std::int32_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int32_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old - 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return _InterlockedDecrement(reinterpret_cast<volatile long *>(value));
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decl %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int64_t interlocked_decrement64(volatile std::int64_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old - 1;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return newv;
#elif RAINY_USING_MSVC
        return _InterlockedDecrement64(reinterpret_cast<volatile __int64 *>(value));
#else
        volatile std::int64_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decq %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }
}

namespace rainy::core::pal {
    std::int8_t interlocked_exchange_add8(volatile std::int8_t *value, std::int8_t amount) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + amount;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchangeAdd8(reinterpret_cast<volatile char *>(value), amount);
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddb %0, %1" : "+q"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        return amount; // xadd 返回旧值在 amount 中
#endif
    }

    std::int16_t interlocked_exchange_add16(volatile std::int16_t *value, std::int16_t amount) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int16_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + amount;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchangeAdd16(reinterpret_cast<volatile short *>(value), amount);
#else
        volatile std::int16_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddw %0, %1" : "+r"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        return amount;
#endif
    }

    std::int32_t interlocked_exchange_add32(volatile std::int32_t *value, std::int32_t amount) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int32_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + amount;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(value), amount);
#else
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddl %0, %1" : "+r"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        return amount;
#endif
    }

    std::int64_t interlocked_exchange_add64(volatile std::int64_t *value, std::int64_t amount) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old + amount;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchangeAdd64(reinterpret_cast<volatile __int64 *>(value), amount);
#else
        volatile std::int64_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddq %0, %1" : "+r"(amount), "+m"(*avoid_clangtidy) : : "memory", "cc");
        return amount;
#endif
    }
}

namespace rainy::core::pal {
    std::intptr_t interlocked_exchange_subtract(volatile std::intptr_t *value, const std::intptr_t amount) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange_subtract64(static_cast<volatile std::int64_t *>(value), static_cast<const std::int64_t>(amount));
#else
        return interlocked_exchange_subtract32(static_cast<volatile std::int32_t *>(value), static_cast<const std::int32_t>(amount));
#endif
    }

    std::int32_t interlocked_exchange_subtract32(volatile std::int32_t *value, std::int32_t amount) {
        return interlocked_exchange_add32(value, -amount);
    }

    std::int64_t interlocked_exchange_subtract64(volatile std::int64_t *value, std::int64_t amount) {
        return interlocked_exchange_add64(value, -amount);
    }

    std::intptr_t iso_volatile_load(const volatile std::intptr_t *address) {
#if RAINY_USING_64_BIT_PLATFORM
        return iso_volatile_load64(reinterpret_cast<const volatile long long *>(address));
#else
        return iso_volatile_load32(static_cast<const volatile int *>(address));
#endif
    }
}

namespace rainy::core::pal {
    std::int8_t iso_volatile_load8(const volatile std::int8_t *address) {
        rainy_assume(address);
#if RAINY_USING_MSVC
        return __iso_volatile_load8(reinterpret_cast<const volatile char *>(address));
#elif RAINY_IS_ARM64
        std::int8_t value;
        __asm__ __volatile__("ldarb %w0, [%1]" : "=r"(value) : "r"(address) : "memory");
        return value;
#else
        volatile std::int8_t value = 0;
        __asm__ __volatile__("lfence\n movb (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }

    std::int16_t iso_volatile_load16(const volatile std::int16_t *address) {
        rainy_assume(address);

#if RAINY_USING_MSVC
        return __iso_volatile_load16(address);
#elif RAINY_IS_ARM64
        std::int16_t value;
        __asm__ __volatile__("ldarh %w0, [%1]" : "=r"(value) : "r"(address) : "memory");
        return value;
#else
        volatile std::int16_t value = 0;
        __asm__ __volatile__("lfence\n movw (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }

    std::int32_t iso_volatile_load32(const volatile std::int32_t *address) {
        rainy_assume(address);

#if RAINY_USING_MSVC
        return __iso_volatile_load32(address);
#elif RAINY_IS_ARM64
        std::int32_t value;
        __asm__ __volatile__("ldar %w0, [%1]" : "=r"(value) : "r"(address) : "memory");
        return value;
#else
        volatile std::int32_t value = 0;
        __asm__ __volatile__("lfence\n movl (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }

    std::int64_t iso_volatile_load64(const volatile long long *address) {
        rainy_assume(address);

#if RAINY_USING_MSVC
        return __iso_volatile_load64(address);
#elif RAINY_IS_ARM64
        std::int64_t value;
        __asm__ __volatile__("ldar %0, [%1]" : "=r"(value) : "r"(address) : "memory");
        return value;
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        volatile std::int64_t value = 0;
        __asm__ __volatile__("lfence\n movq (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }
}

namespace rainy::core::pal {
    long interlocked_exchange(volatile long *target, long value) {
        rainy_assume(static_cast<bool>(target));
#if RAINY_USING_MSVC
        return _InterlockedExchange(target, value);
#else
        __asm__ __volatile__("lock xchg %1, %0" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int8_t interlocked_exchange8(volatile std::int8_t *target, std::int8_t value) {
        rainy_assume(static_cast<bool>(target));
#if RAINY_USING_MSVC
        return _InterlockedExchange8(reinterpret_cast<volatile char *>(target), value);
#else
        __asm__ __volatile__("lock xchgb %0, %1" : "=q"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int16_t interlocked_exchange16(volatile std::int16_t *target, std::int16_t value) {
        rainy_assume(static_cast<bool>(target));
#if RAINY_USING_MSVC
        return _InterlockedExchange16(target, value);
#else
        __asm__ __volatile__("lock xchgw %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int32_t interlocked_exchange32(volatile std::int32_t *target, std::int32_t value) {
        rainy_assume(static_cast<bool>(target));
#if RAINY_USING_MSVC
        return _InterlockedExchange(reinterpret_cast<volatile long *>(target), static_cast<long>(value));
#else
        __asm__ __volatile__("lock xchgl %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int64_t interlocked_exchange64(volatile std::int64_t *target, std::int64_t value) {
        rainy_assume(static_cast<bool>(target));
#if RAINY_USING_MSVC
        return _InterlockedExchange64(reinterpret_cast<volatile __int64 *>(target), value);
#else
        __asm__ __volatile__("lock xchgq %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }
}

namespace rainy::core::pal {
    void *interlocked_exchange_pointer(volatile void **target, void *value) {
        rainy_assume(target);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        void *old;
        RAINY_ARM64_ATOMIC_BEGIN(target, old)
        void *newv = value;
        RAINY_ARM64_ATOMIC_END(target, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchangePointer(const_cast<void *volatile *>(target), value);
#else
        void *old;
        __asm__ __volatile__("lock xchg %1, %0" : "=r"(old), "+m"(*target) : "0"(value) : "memory");
        return old;
#endif
    }

    bool interlocked_compare_exchange_pointer(volatile void **destination, void *exchange, void *comparand) {
        rainy_assume(static_cast<bool>(destination));
#if RAINY_USING_MSVC
        return _InterlockedCompareExchangePointer(const_cast<void *volatile *>(destination), exchange, comparand) == comparand;
#else
        char result{};
        __asm__ __volatile__("lock cmpxchg %3, %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        return result;
#endif
    }
}

namespace rainy::core::pal {
    bool interlocked_compare_exchange(volatile long *destination, long exchange, long comparand) {
        rainy_assume(destination);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        long old;
        RAINY_ARM64_ATOMIC_BEGIN(destination, old)
        if (old == comparand) {
            long newv = exchange;
            RAINY_ARM64_ATOMIC_END(destination, newv);
            return true;
        }
        RAINY_ARM64_ATOMIC_END(destination, old);
        return false;
#elif RAINY_USING_MSVC
        return _InterlockedCompareExchange(destination, exchange, comparand) == comparand;
#else
        char result{};
        __asm__ __volatile__("lock cmpxchg %3, %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        return result;
#endif
    }

    bool interlocked_compare_exchange8(volatile std::int8_t *destination, std::int8_t exchange, std::int8_t comparand) {
        rainy_assume(destination);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old;
        RAINY_ARM64_ATOMIC_BEGIN(destination, old)
        if (old == comparand) {
            std::int8_t newv = exchange;
            RAINY_ARM64_ATOMIC_END(destination, newv);
            return true;
        }
        RAINY_ARM64_ATOMIC_END(destination, old);
        return false;
#elif RAINY_USING_MSVC
        return _InterlockedCompareExchange8(reinterpret_cast<volatile char *>(destination), exchange, comparand);
#else
        char result;
        __asm__ __volatile__("lock cmpxchgb %2, %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        return result;
#endif
    }

    bool interlocked_compare_exchange16(volatile std::int16_t *destination, std::int16_t exchange, std::int16_t comparand) {
        rainy_assume(destination);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int16_t old;
        RAINY_ARM64_ATOMIC_BEGIN(destination, old)
        if (old == comparand) {
            std::int16_t newv = exchange;
            RAINY_ARM64_ATOMIC_END(destination, newv);
            return true;
        }
        RAINY_ARM64_ATOMIC_END(destination, old);
        return false;
#elif RAINY_USING_MSVC
        return _InterlockedCompareExchange16(destination, exchange, comparand);
#else
        char result;
        __asm__ __volatile__("lock cmpxchgw %2, %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        return result;
#endif
    }

    bool interlocked_compare_exchange32(volatile std::int32_t *destination, std::int32_t exchange, std::int32_t comparand) {
        rainy_assume(destination);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int32_t old;
        RAINY_ARM64_ATOMIC_BEGIN(destination, old)
        if (old == comparand) {
            std::int32_t newv = exchange;
            RAINY_ARM64_ATOMIC_END(destination, newv);
            return true;
        }
        RAINY_ARM64_ATOMIC_END(destination, old);
        return false;
#elif RAINY_USING_MSVC
        return _InterlockedCompareExchange(reinterpret_cast<volatile long *>(destination), exchange, comparand) == comparand;
#else
        char result;
        __asm__ __volatile__("lock cmpxchgl %2, %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
        return result;
#endif
    }

    bool interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange, std::int64_t comparand) {
        rainy_assume(destination);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old;
        RAINY_ARM64_ATOMIC_BEGIN(destination, old)
        if (old == comparand) {
            std::int64_t newv = exchange;
            RAINY_ARM64_ATOMIC_END(destination, newv);
            return true;
        }
        RAINY_ARM64_ATOMIC_END(destination, old);
        return false;
#elif RAINY_USING_MSVC
        return _InterlockedCompareExchange64(destination, exchange, comparand) == comparand;
#else
        char result{};
#if RAINY_USING_64_BIT_PLATFORM
        __asm__ __volatile__("lock cmpxchg %3, %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+a"(comparand)
                             : "r"(exchange)
                             : "memory");
#else
        std::int32_t comparand_high = static_cast<std::int32_t>(comparand >> 32);
        std::int32_t comparand_low = static_cast<std::int32_t>(comparand);
        std::int32_t exchange_high = static_cast<std::int32_t>(exchange >> 32);
        std::int32_t exchange_low = static_cast<std::int32_t>(exchange);
        __asm__ __volatile__("lock cmpxchg8b %1\n\t"
                             "sete %0"
                             : "=q"(result), "+m"(*destination), "+d"(comparand_high), "+a"(comparand_low)
                             : "c"(exchange_high), "b"(exchange_low)
                             : "memory");
#endif
        return result;
#endif
    }

#if RAINY_USING_64_BIT_PLATFORM
    bool interlocked_compare_exchange128(std::int64_t volatile *destination, std::int64_t exchange_high, std::int64_t exchange_low,
                                         std::int64_t *comparand_result) {
        rainy_assume(destination);
        rainy_assume(comparand_result);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        // ARM64 原子 128 位 CAS 需要 LL/SC 或平台支持，如果不支持可以返回 false
        return false;
#elif RAINY_USING_MSVC
        return _InterlockedCompareExchange128(reinterpret_cast<volatile std::int64_t *>(destination), exchange_high, exchange_low,
                                              comparand_result);
#else
        bool result{};
        std::int64_t comparand_low = comparand_result[0];
        std::int64_t comparand_high = comparand_result[1];
        __asm__ __volatile__("lock cmpxchg16b %1\n\t"
                             "setz %0"
                             : "=q"(result), "+m"(*reinterpret_cast<__int128 volatile *>(destination)), "+d"(comparand_high),
                               "+a"(comparand_low)
                             : "c"(exchange_high), "b"(exchange_low)
                             : "memory");
        if (!result) {
            comparand_result[0] = comparand_low;
            comparand_result[1] = comparand_high;
        }
        return result;
#endif
    }
#endif
}

namespace rainy::core::pal {
    std::int8_t interlocked_and8(volatile std::int8_t *value, std::int8_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old & mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedAnd8(reinterpret_cast<volatile char *>(value), mask);
#else
        std::int8_t old;
        __asm__ __volatile__("movb %1, %0\n\t"
                             "lock andb %2, %1"
                             : "=&q"(old), "+m"(*value)
                             : "iq"(mask)
                             : "memory");
        return old;
#endif
    }

    int16_t interlocked_and16(volatile int16_t *value, int16_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        int16_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old & mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedAnd16(reinterpret_cast<volatile short *>(value), mask);
#else
        int16_t old;
        __asm__ __volatile__("movw %1, %0\n\t"
                             "lock andw %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    int32_t interlocked_and32(volatile int32_t *value, int32_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        int32_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old & mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedAnd(reinterpret_cast<volatile long *>(value), mask);
#else
        int32_t old;
        __asm__ __volatile__("movl %1, %0\n\t"
                             "lock andl %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    std::int64_t interlocked_and64(volatile std::int64_t *value, std::int64_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old & mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedAnd64(reinterpret_cast<volatile __int64 *>(value), mask);
#else
        std::int64_t old;
        __asm__ __volatile__("movq %1, %0\n\t"
                             "lock andq %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    // ------------------------ OR ------------------------

    std::int8_t interlocked_or8(volatile std::int8_t *value, std::int8_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old | mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedOr8(reinterpret_cast<volatile char *>(value), mask);
#else
        std::int8_t old;
        __asm__ __volatile__("movb %1, %0\n\t"
                             "lock orb %2, %1"
                             : "=&q"(old), "+m"(*value)
                             : "iq"(mask)
                             : "memory");
        return old;
#endif
    }

    int16_t interlocked_or16(volatile int16_t *value, int16_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        int16_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old | mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedOr16(reinterpret_cast<volatile short *>(value), mask);
#else
        int16_t old;
        __asm__ __volatile__("movw %1, %0\n\t"
                             "lock orw %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    int32_t interlocked_or32(volatile int32_t *value, int32_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        int32_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old | mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedOr(reinterpret_cast<volatile long *>(value), mask);
#else
        int32_t old;
        __asm__ __volatile__("movl %1, %0\n\t"
                             "lock orl %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    std::int64_t interlocked_or64(volatile std::int64_t *value, std::int64_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old | mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedOr64(reinterpret_cast<volatile __int64 *>(value), mask);
#else
        std::int64_t old;
        __asm__ __volatile__("movq %1, %0\n\t"
                             "lock orq %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    // ------------------------ XOR ------------------------

    std::int8_t interlocked_xor8(volatile std::int8_t *value, std::int8_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old ^ mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedXor8(reinterpret_cast<volatile char *>(value), mask);
#else
        std::int8_t old;
        __asm__ __volatile__("movb %1, %0\n\t"
                             "lock xorb %2, %1"
                             : "=&q"(old), "+m"(*value)
                             : "iq"(mask)
                             : "memory");
        return old;
#endif
    }

    int16_t interlocked_xor16(volatile int16_t *value, int16_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        int16_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old ^ mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedXor16(reinterpret_cast<volatile short *>(value), mask);
#else
        int16_t old;
        __asm__ __volatile__("movw %1, %0\n\t"
                             "lock xorw %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    int32_t interlocked_xor32(volatile int32_t *value, int32_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        int32_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old ^ mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedXor(reinterpret_cast<volatile long *>(value), mask);
#else
        int32_t old;
        __asm__ __volatile__("movl %1, %0\n\t"
                             "lock xorl %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    std::int64_t interlocked_xor64(volatile std::int64_t *value, std::int64_t mask) {
        rainy_assume(value);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old, newv;
        RAINY_ARM64_ATOMIC_BEGIN(value, old)
        newv = old ^ mask;
        RAINY_ARM64_ATOMIC_END(value, newv);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedXor64(reinterpret_cast<volatile __int64 *>(value), mask);
#else
        std::int64_t old;
        __asm__ __volatile__("movq %1, %0\n\t"
                             "lock xorq %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }
}

namespace rainy::core::pal {
    void iso_volatile_store(volatile void *address, void *value) {
#if RAINY_USING_64_BIT_PLATFORM
        iso_volatile_store64(static_cast<volatile long long *>(address), *static_cast<std::uint64_t *>(value));
#else
        iso_volatile_store32(static_cast<volatile int *>(address), *static_cast<std::uint32_t *>(value));
#endif
    }

    void iso_volatile_store8(volatile std::int8_t *address, std::int8_t value) {
        rainy_assume(address);

#if RAINY_USING_MSVC
        *address = value; // MSVC 没有 8 位原子交换
#elif RAINY_IS_ARM64
        __asm__ __volatile__("stlrb %w0, [%1]" ::"r"(value), "r"(address) : "memory");
#else
        __asm__ __volatile__("movb %0, (%1)" ::"r"(value), "r"(address) : "memory");
#endif
    }

    void iso_volatile_store16(volatile std::int16_t *address, std::int16_t value) {
        rainy_assume(address);

#if RAINY_USING_MSVC
        *address = value;
#elif RAINY_IS_ARM64
        __asm__ __volatile__("stlrh %w0, [%1]" ::"r"(value), "r"(address) : "memory");
#else
        __asm__ __volatile__("movw %0, (%1)" ::"r"(value), "r"(address) : "memory");
#endif
    }

    void iso_volatile_store32(volatile int *address, std::uint32_t value) {
        rainy_assume(address);

#if RAINY_USING_MSVC
        _InterlockedExchange(reinterpret_cast<volatile long *>(address), static_cast<long>(value));
#elif RAINY_IS_ARM64
        __asm__ __volatile__("stlr %w0, [%1]" ::"r"(value), "r"(address) : "memory");
#else
        __asm__ __volatile__("movl %0, (%1)" ::"r"(value), "r"(address) : "memory");
#endif
    }

    void iso_volatile_store64(volatile long long *address, std::uint64_t value) {
        rainy_assume(address);

#if RAINY_USING_MSVC
#if RAINY_USING_64_BIT_PLATFORM
        _InterlockedExchange64(reinterpret_cast<volatile long long *>(address), value);
#else
        ::_interlockedexchange64(reinterpret_cast<volatile long long *>(address), value);
#endif
#elif RAINY_IS_ARM64
        __asm__ __volatile__("stlr %0, [%1]" ::"r"(value), "r"(address) : "memory");
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        __asm__ __volatile__("movq %0, (%1)" ::"r"(value), "r"(address) : "memory");
#endif
    }
}

namespace rainy::core::pal {
    void read_write_barrier() noexcept {
#if RAINY_USING_MSVC
        _ReadWriteBarrier();
#elif RAINY_IS_ARM64
        __asm__ __volatile__("dmb ish" ::: "memory"); // 全屏障，保证所有访问顺序
#else
        __asm__ __volatile__("mfence" ::: "memory"); // x86/x64 全屏障
#endif
    }

    void read_barrier() noexcept {
#if RAINY_USING_MSVC
        _mm_lfence();
#elif RAINY_IS_ARM64
        __asm__ __volatile__("dmb ishld" ::: "memory"); // 仅读屏障
#else
        __asm__ __volatile__("lfence" ::: "memory"); // x86/x64 读屏障
#endif
    }

    void write_barrier() noexcept {
#if RAINY_USING_MSVC
        _mm_sfence();
#elif RAINY_IS_ARM64
        __asm__ __volatile__("dmb ishst" ::: "memory"); // 仅写屏障
#else
        __asm__ __volatile__("sfence" ::: "memory"); // x86/x64 写屏障
#endif
    }
}

namespace rainy::core::pal {
    static inline void fence_before(memory_order order) {
        if (order == memory_order::release || order == memory_order::acq_rel) {
            write_barrier(); // sfence
        }
    }

    static inline void fence_after(memory_order order) {
        if (order == memory_order::acquire || order == memory_order::consume || order == memory_order::acq_rel) {
            read_barrier(); // lfence
        }
    }
}

namespace rainy::core::pal {
    long interlocked_increment_explicit(volatile long *value, memory_order order) {
        rainy_assume(static_cast<bool>(value));
        fence_before(order);
#if RAINY_USING_MSVC
        long result = _InterlockedIncrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clang_tidy = value;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clang_tidy) : : "cc", "memory");
        long result = *avoid_clang_tidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
        fence_after(order);
        return result;
    }
}
