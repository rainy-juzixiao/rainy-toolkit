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

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,readability-duplicate-branches,clang-analyzer-core.UndefinedBinaryOperatorResult)

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

// arm64的宏代码，用于评估是否可以将rainy-toolkit pal移植到arm64架构上
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC

#define RAINY_ARM64_ATOMIC_BEGIN(ptr, old)                                                                                            \
    volatile int _stxr_failed;                                                                                                                 \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile  _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
        volatile int _stxr_failed;                                                                                                             \
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
    long interlocked_increment(volatile long *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        volatile long old{}, newv{}; // NOLINT
        RAINY_ARM64_ATOMIC_OP_LONG(value, old, newv, +1);
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

    std::int8_t interlocked_increment8(volatile std::int8_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        volatile std::int8_t old, newv; // NOLINT
        RAINY_ARM64_ATOMIC_OP_8(value, old, newv, +1);
        return newv;
#elif RAINY_USING_MSVC
        return interlocked_exchange_add8(value, 1) + 1;
#else
        volatile std::int8_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; incb %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#endif
    }

    std::int16_t interlocked_increment16(volatile std::int16_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        volatile std::int16_t old, newv; // NOLINT
        RAINY_ARM64_ATOMIC_OP_16(value, old, newv, +1);
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
        volatile std::int32_t old, newv; // NOLINT
        RAINY_ARM64_ATOMIC_OP_32(value, old, newv, +1);
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
        volatile std::int64_t old, newv; // NOLINT
        RAINY_ARM64_ATOMIC_OP_64(value, old, newv, +1);
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
    long interlocked_decrement(volatile long *value) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        volatile long old, newv; // NOLINT
        RAINY_ARM64_ATOMIC_OP_LONG(value, old, newv, -1);
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

    std::int8_t interlocked_decrement8(volatile std::int8_t *value) {
        rainy_assume(value);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_OP_8(value, old, newv, -1);
        return newv;
#elif RAINY_USING_MSVC
        return interlocked_exchange_add8(value, -1) - 1;
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
        RAINY_ARM64_ATOMIC_OP_16(value, old, newv, -1);
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
        RAINY_ARM64_ATOMIC_OP_32(value, old, newv, -1);
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
        RAINY_ARM64_ATOMIC_OP_64(value, old, newv, -1);
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
        volatile std::int8_t old, newv;
        RAINY_ARM64_ATOMIC_OP_8(value, old, newv, +amount);
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
        volatile std::int16_t old, newv; // NOLINT
        RAINY_ARM64_ATOMIC_OP_16(value, old, newv, +amount);
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
        RAINY_ARM64_ATOMIC_OP_32(value, old, newv, +amount);
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
        RAINY_ARM64_ATOMIC_OP_64(value, old, newv, +amount);
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

    std::int8_t interlocked_exchange_subtract8(volatile std::int8_t *value, std::int8_t amount) {
        return interlocked_exchange_add8(value, -amount);
    }

    std::int16_t interlocked_exchange_subtract16(volatile std::int16_t *value, std::int16_t amount) {
        return interlocked_exchange_add16(value, -amount);
    }

    std::int32_t interlocked_exchange_subtract32(volatile std::int32_t *value, std::int32_t amount) {
        return interlocked_exchange_add32(value, -amount);
    }

    std::int64_t interlocked_exchange_subtract64(volatile std::int64_t *value, std::int64_t amount) {
        return interlocked_exchange_add64(value, -amount);
    }
}

namespace rainy::core::pal {
    std::intptr_t iso_volatile_load(const volatile std::intptr_t *address) {
#if RAINY_USING_64_BIT_PLATFORM
        return iso_volatile_load64(reinterpret_cast<const volatile std::int64_t *>(address)); // NOLINT
#else
        return iso_volatile_load32(static_cast<const volatile int *>(address));
#endif
    }

    std::int8_t iso_volatile_load8(const volatile std::int8_t *address) {
        rainy_assume(address);
#if RAINY_USING_MSVC
        return __iso_volatile_load8(reinterpret_cast<const volatile char *>(address));
#elif RAINY_IS_ARM64
        std::int8_t value; // NOLINT
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
        std::int16_t value; // NOLINT
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
        std::int32_t value; // NOLINT
        __asm__ __volatile__("ldar %w0, [%1]" : "=r"(value) : "r"(address) : "memory");
        return value;
#else
        volatile std::int32_t value = 0;
        __asm__ __volatile__("lfence\n movl (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }

    std::int64_t iso_volatile_load64(const volatile std::int64_t *address) {
        rainy_assume(address);
#if RAINY_USING_MSVC
        return __iso_volatile_load64(address);
#elif RAINY_IS_ARM64
        std::int64_t value; // NOLINT
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
        rainy_assume(target);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        long old;
        RAINY_ARM64_ATOMIC_EXCHANGE_LONG(target, value, old);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchange(target, value);
#else
        __asm__ __volatile__("lock xchg %1, %0" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int8_t interlocked_exchange8(volatile std::int8_t *target, std::int8_t value) {
        rainy_assume(target);
#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int8_t old;
        RAINY_ARM64_ATOMIC_EXCHANGE_8(target, value, old);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchange8(reinterpret_cast<volatile char *>(target), value);
#else
        __asm__ __volatile__("lock xchgb %0, %1" : "=q"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int16_t interlocked_exchange16(volatile std::int16_t *target, std::int16_t value) {
        rainy_assume(target);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int16_t old;
        RAINY_ARM64_ATOMIC_EXCHANGE_16(target, value, old);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchange16(target, value);
#else
        __asm__ __volatile__("lock xchgw %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int32_t interlocked_exchange32(volatile std::int32_t *target, std::int32_t value) {
        rainy_assume(target);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int32_t old;
        RAINY_ARM64_ATOMIC_EXCHANGE_32(target, value, old);
        return old;
#elif RAINY_USING_MSVC
        return _InterlockedExchange(reinterpret_cast<volatile long *>(target), static_cast<long>(value));
#else
        __asm__ __volatile__("lock xchgl %0, %1" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }

    std::int64_t interlocked_exchange64(volatile std::int64_t *target, std::int64_t value) {
        rainy_assume(target);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        std::int64_t old;
        RAINY_ARM64_ATOMIC_EXCHANGE_64(target, value, old);
        return old;
#elif RAINY_USING_MSVC
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
        rainy_assume(destination);

#if RAINY_IS_ARM64 && !RAINY_USING_MSVC
        bool result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
#elif RAINY_USING_MSVC
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
        long result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
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
        std::int8_t result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
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
        std::int16_t result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
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
        std::int32_t result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
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
        std::int64_t result;
        RAINY_ARM64_COMPARE_EXCHANGE(destination, exchange, comparand, result);
        return result;
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
        // ARM64 原子 128 位 CAS 需要 LL/SC 或平台支持
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
    std::intptr_t interlocked_and(volatile std::intptr_t *value, const std::intptr_t mask) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_and64(reinterpret_cast<volatile std::intptr_t *>(value), mask);
#else
        return interlocked_and32(reinterpret_cast<volatile std::intptr_t *>(value), mask);
#endif
    }

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
}

namespace rainy::core::pal {
    std::intptr_t interlocked_or(volatile std::intptr_t *value, std::intptr_t mask) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_or64(reinterpret_cast<volatile std::intptr_t *>(value), mask);
#else
        return interlocked_or32(reinterpret_cast<volatile std::intptr_t *>(value), mask);
#endif
    }

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
}

namespace rainy::core::pal {
    std::intptr_t interlocked_xor(volatile std::intptr_t *value, std::intptr_t mask) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_xor64(reinterpret_cast<volatile std::intptr_t *>(value), mask);
#else
        return interlocked_xor32(reinterpret_cast<volatile std::intptr_t *>(value), mask);
#endif
    }

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
    void atomic_thread_fence(const memory_order order) noexcept {
        if (order == memory_order_relaxed) {
            return;
        }
#if RAINY_IS_X86_PLATFORM
        // x86 / x64 平台
        rainy_compiler_barrier();
        if (order == memory_order_seq_cst) {
            volatile long guard = 0;
            core::pal::interlocked_increment(&guard);
            rainy_compiler_barrier();
        }
#elif RAINY_IS_ARM64
        // ARM 平台
        if (order == memory_order_acquire || order == memory_order_consume) {
            rainy_dmb_ld();
        } else {
            rainy_dmb();
        }
#else
        static_assert(false, "atomic_thread_fence: unsupported architecture");
#endif
    }

    //     long interlocked_increment_explicit(volatile long *value, memory_order order) {
    //         rainy_assume(static_cast<bool>(value));
    //         fence_before(order);
    // #if RAINY_USING_MSVC
    //         long result = _InterlockedIncrement(value);
    // #elif RAINY_USING_GCC || RAINY_USING_CLANG
    //         volatile long *avoid_clang_tidy = value;
    //         __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clang_tidy) : : "cc", "memory");
    //         long result = *avoid_clang_tidy;
    // #else
    //         static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
    // #endif
    //         fence_after(order);
    //         return result;
    //     }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while,readability-duplicate-branches,clang-analyzer-core.UndefinedBinaryOperatorResult)
