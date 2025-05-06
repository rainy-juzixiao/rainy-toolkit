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
#include <intrin.h>
#endif

namespace rainy::core::pal {
    long interlocked_increment(volatile long *value) {
        rainy_assume(static_cast<bool>(value)); // 假定传入的value是一个有效的地址
#if RAINY_USING_MSVC
        return _InterlockedIncrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clang_tidy = value;
        __asm__ __volatile__("lock; incl %0" : "+m"(*avoid_clang_tidy) : : "cc");
        return *avoid_clang_tidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
    }

    long interlocked_decrement(volatile long *value) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_USING_MSVC
        return _InterlockedDecrement(value);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile long *avoid_clangtidy = value;
        __asm__ __volatile__("lock; decl %0" : "+m"(*avoid_clangtidy) : : "cc");
        return *avoid_clangtidy;
#else
        static_assert(false, "rainy-toolkit only supports GCC Clang and MSVC platforms");
#endif
    }

    std::intptr_t interlocked_exchange_add(volatile std::intptr_t *value, const std::intptr_t amount) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange_add64(static_cast<volatile std::int64_t *>(value), static_cast<const std::int64_t>(amount));
#else
        return interlocked_exchange_add32(static_cast<volatile std::int32_t *>(value), static_cast<const std::int32_t>(amount));
#endif
    }

    std::int32_t interlocked_exchange_add32(volatile std::int32_t *value, std::int32_t amount) {
        rainy_assume(static_cast<bool>(value));
        volatile std::int32_t old_value = (*value);
#if RAINY_USING_MSVC
        ::_InterlockedExchangeAdd(reinterpret_cast<volatile long *>(value), amount);
#elif RAINY_USING_GCC || RAINY_USING_CLANG
        volatile std::int32_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xadd %0, %1" : "+r"(amount) : "m"(*avoid_clangtidy) : "memory", "cc");
#endif
        return old_value;
    }

    std::int64_t interlocked_exchange_add64(volatile std::int64_t *value, std::int64_t amount) {
        rainy_assume(static_cast<bool>(value));
        volatile std::int64_t old_value = (*value);
#if RAINY_USING_MSVC
#if RAINY_USING_64_BIT_PLATFORM
        ::_InterlockedExchangeAdd64(value, amount);
#else
        ::_interlockedexchangeadd64(value, amount);
#endif
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        volatile std::int64_t *avoid_clangtidy = value;
        __asm__ __volatile__("lock; xaddq %0, %1" : "+r"(amount) : "m"(*avoid_clangtidy) : "memory", "cc");
#endif
        return old_value;
    }

    std::intptr_t interlocked_exchange_subtract(volatile std::intptr_t *value, const std::intptr_t amount) {
#if RAINY_USING_64_BIT_PLATFORM
        return interlocked_exchange_subtract64(static_cast<volatile std::int64_t *>(value),
                                               static_cast<const std::int64_t>(amount));
#else
        return interlocked_exchange_subtract32(static_cast<volatile std::int32_t *>(value),
                                               static_cast<const std::int32_t>(amount));
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
        return iso_volatile_load64(static_cast<const volatile long long *>(address));
#else
        return iso_volatile_load32(static_cast<const volatile int *>(address));
#endif
    }

    std::int32_t iso_volatile_load32(const volatile int *address) { // NOLINT
        rainy_assume(static_cast<bool>(address));
#if RAINY_USING_MSVC
        return __iso_volatile_load32(address);
#else
        volatile std::int32_t value = 0;
        __asm__ __volatile__("lfence\n movl (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }

    std::int64_t iso_volatile_load64(const volatile long long *address) { // NOLINT
        rainy_assume(static_cast<bool>(address));
#if RAINY_USING_MSVC
        return __iso_volatile_load64(address);
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        volatile std::int64_t value = 0;
        __asm__ __volatile__("lfence\n movq (%1), %0" : "=r"(value) : "r"(address) : "memory");
        return value;
#endif
    }

    long interlocked_exchange(volatile long *target, long value) {
        rainy_assume(static_cast<bool>(target));
#if RAINY_USING_MSVC
        return _InterlockedExchange(target, value);
#else
        __asm__ __volatile__("lock xchg %1, %0" : "=r"(value), "+m"(*target) : "0"(value) : "memory");
        return value;
#endif
    }
    
    void *interlocked_exchange_pointer(volatile void **target, void *value) {
#if RAINY_USING_MSVC
        return _InterlockedExchangePointer(const_cast<void *volatile *>(target), value);
#else
        void *old;
        // 使用内联汇编交换值并返回旧值
        __asm__ __volatile__("lock xchg %1, %0" : "=r"(old), "+m"(*target) : "0"(value) : "memory");
        return old; // 返回旧值，即交换前的 *target 值
#endif
    }

    bool interlocked_compare_exchange(volatile long *destination, long exchange, long comparand) {
        rainy_assume(static_cast<bool>(destination));
#if RAINY_USING_MSVC
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

    bool interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange, std::int64_t comparand) {
        rainy_assume(static_cast<bool>(destination));
#if RAINY_USING_MSVC
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

    long interlocked_and(volatile long *value, long mask) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_USING_MSVC
        return _InterlockedAnd(value, mask);
#else
        long old{};
        __asm__ __volatile__("mov %1, %0\n\t"
                             "lock and %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    long interlocked_or(volatile long *value, long mask) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_USING_MSVC
        return _InterlockedOr(value, mask);
#else
        long old{};
        __asm__ __volatile__("mov %1, %0\n\t"
                             "lock or %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    long interlocked_xor(volatile long *value, long mask) {
        rainy_assume(static_cast<bool>(value));
#if RAINY_USING_MSVC
        return _InterlockedXor(value, mask);
#else
        long old{};
        __asm__ __volatile__("mov %1, %0\n\t"
                             "lock xor %2, %1"
                             : "=&r"(old), "+m"(*value)
                             : "r"(mask)
                             : "memory");
        return old;
#endif
    }

    void iso_volatile_store(volatile void *address, void *value) {
#if RAINY_USING_64_BIT_PLATFORM
        iso_volatile_store64(static_cast<volatile long long *>(address), *static_cast<std::uint64_t *>(value));
#else
        iso_volatile_store32(static_cast<volatile int *>(address), *static_cast<std::uint32_t *>(value));
#endif
    }

    void iso_volatile_store32(volatile int *address, std::uint32_t value) {
        rainy_assume(static_cast<bool>(address));
#if RAINY_USING_MSVC
        _InterlockedExchange(reinterpret_cast<volatile long *>(address), static_cast<long>(value));
#else
        __asm__ __volatile__("movl %0, (%1)"
                             : // no output
                             : "r"(value), "r"(address)
                             : "memory");
#endif
    }

    void iso_volatile_store64(volatile long long *address, std::uint64_t value) {
        rainy_assume(static_cast<bool>(address));
#if RAINY_USING_MSVC
#if RAINY_USING_64_BIT_PLATFORM
        _InterlockedExchange64(reinterpret_cast<volatile long long *>(address), value);
#else
        ::_interlockedexchange64(reinterpret_cast<volatile long long *>(address), value);
#endif
#elif (RAINY_USING_GCC || RAINY_USING_CLANG) && RAINY_USING_64_BIT_PLATFORM
        __asm__ __volatile__("movq %0, (%1)"
                             : // no output
                             : "r"(value), "r"(address)
                             : "memory");
#endif
    }

    void iso_memory_fence() {
#if RAINY_USING_MSVC
        _ReadWriteBarrier(); // MSVC 内存屏障
#else
        __asm__ __volatile__("mfence" // x86/x86_64 内存屏障指令
                             :
                             :
                             : "memory");
#endif
    }


    void memory_barrier() {
        iso_memory_fence();
    }
}
