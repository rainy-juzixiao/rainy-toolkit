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
#include <rainy/core/core.hpp>
#include <rainy/core/implements/arm64_intrin.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while,readability-duplicate-branches,clang-analyzer-core.UndefinedBinaryOperatorResult)

namespace rainy::core::pal {
    long interlocked_increment(volatile long *value) {
        return interlocked_increment_explicit(value, memory_order_seq_cst);
    }

    std::int8_t interlocked_increment8(volatile std::int8_t *value) {
        return interlocked_increment8_explicit(value, memory_order_seq_cst);
    }

    std::int16_t interlocked_increment16(volatile std::int16_t *value) {
        return interlocked_increment16_explicit(value, memory_order_seq_cst);
    }

    std::int32_t interlocked_increment32(volatile std::int32_t *value) {
        return interlocked_increment32_explicit(value, memory_order_seq_cst);
    }

    std::int64_t interlocked_increment64(volatile std::int64_t *value) {
        return interlocked_increment64_explicit(value, memory_order_seq_cst);
    }
}

namespace rainy::core::pal {
    long interlocked_decrement(volatile long *value) {
        return interlocked_decrement_explicit(value, memory_order_seq_cst);
    }

    std::int8_t interlocked_decrement8(volatile std::int8_t *value) {
        return interlocked_decrement8_explicit(value, memory_order_seq_cst);
    }

    std::int16_t interlocked_decrement16(volatile std::int16_t *value) {
        return interlocked_decrement16_explicit(value, memory_order_seq_cst);
    }

    std::int32_t interlocked_decrement32(volatile std::int32_t *value) {
        return interlocked_decrement32_explicit(value, memory_order_seq_cst);
    }

    std::int64_t interlocked_decrement64(volatile std::int64_t *value) {
        return interlocked_decrement64_explicit(value, memory_order_seq_cst);
    }
}

namespace rainy::core::pal {
    std::int8_t interlocked_exchange_add8(volatile std::int8_t *value, std::int8_t amount) {
        return interlocked_exchange_add8_explicit(value, amount, memory_order_seq_cst);
    }

    std::int16_t interlocked_exchange_add16(volatile std::int16_t *value, std::int16_t amount) {
        return interlocked_exchange_add16_explicit(value, amount, memory_order_seq_cst);
    }

    std::int32_t interlocked_exchange_add32(volatile std::int32_t *value, std::int32_t amount) {
        return interlocked_exchange_add32_explicit(value, amount, memory_order_seq_cst);
    }

    std::int64_t interlocked_exchange_add64(volatile std::int64_t *value, std::int64_t amount) {
        return interlocked_exchange_add64_explicit(value, amount, memory_order_seq_cst);
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

    std::int8_t interlocked_exchange_subtract8(volatile std::int8_t *value, const std::int8_t amount) {
        return interlocked_exchange_add8(value, -amount); // NOLINT
    }

    std::int16_t interlocked_exchange_subtract16(volatile std::int16_t *value, const std::int16_t amount) {
        return interlocked_exchange_add16(value, -amount); // NOLINT
    }

    std::int32_t interlocked_exchange_subtract32(volatile std::int32_t *value, const std::int32_t amount) {
        return interlocked_exchange_add32(value, -amount);
    }

    std::int64_t interlocked_exchange_subtract64(volatile std::int64_t *value, const std::int64_t amount) {
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
        return iso_volatile_load8_explicit(address, memory_order_seq_cst);
    }

    std::int16_t iso_volatile_load16(const volatile std::int16_t *address) {
        return iso_volatile_load16_explicit(address, memory_order_seq_cst);
    }

    std::int32_t iso_volatile_load32(const volatile std::int32_t *address) {
        return iso_volatile_load32_explicit(address, memory_order_seq_cst);
    }

    std::int64_t iso_volatile_load64(const volatile std::int64_t *address) {
        return iso_volatile_load64_explicit(address, memory_order_seq_cst);
    }
}

namespace rainy::core::pal {
    std::intptr_t interlocked_exchange(volatile std::intptr_t *target, std::intptr_t value) {
        return interlocked_exchange_explicit(target, value, memory_order_seq_cst);
    }

    std::int8_t interlocked_exchange8(volatile std::int8_t *target, std::int8_t value) {
        return interlocked_exchange8_explicit(target, value, memory_order_seq_cst);
    }

    std::int16_t interlocked_exchange16(volatile std::int16_t *target, std::int16_t value) {
        return interlocked_exchange16_explicit(target, value, memory_order_seq_cst);
    }

    std::int32_t interlocked_exchange32(volatile std::int32_t *target, std::int32_t value) {
        return interlocked_exchange32_explicit(target, value, memory_order_seq_cst);
    }

    std::int64_t interlocked_exchange64(volatile std::int64_t *target, std::int64_t value) {
        return interlocked_exchange64_explicit(target, value, memory_order_seq_cst);
    }
}

namespace rainy::core::pal {
    void* interlocked_exchange_pointer(volatile void **target, void *value) {
        return interlocked_exchange_pointer_explicit(target, value, memory_order_seq_cst);
    }

    void* interlocked_compare_exchange_pointer(volatile void **destination, void *exchange, void *comparand) {
        return interlocked_compare_exchange_pointer_explicit(destination, exchange, comparand, memory_order_seq_cst,
                                                             memory_order_seq_cst);
    }
}

namespace rainy::core::pal {
    bool interlocked_compare_exchange(volatile long *destination, long exchange, long comparand) {
        return interlocked_compare_exchange_explicit(destination, exchange, comparand, memory_order_seq_cst, memory_order_seq_cst);
    }

    bool interlocked_compare_exchange8(volatile std::int8_t *destination, std::int8_t exchange, std::int8_t comparand) {
        return interlocked_compare_exchange8_explicit(destination, exchange, comparand, memory_order_seq_cst, memory_order_seq_cst);
    }

    bool interlocked_compare_exchange16(volatile std::int16_t *destination, std::int16_t exchange, std::int16_t comparand) {
        return interlocked_compare_exchange16_explicit(destination, exchange, comparand, memory_order_seq_cst, memory_order_seq_cst);
    }

    bool interlocked_compare_exchange32(volatile std::int32_t *destination, std::int32_t exchange, std::int32_t comparand) {
        return interlocked_compare_exchange32_explicit(destination, exchange, comparand, memory_order_seq_cst, memory_order_seq_cst);
    }

    bool interlocked_compare_exchange64(volatile std::int64_t *destination, std::int64_t exchange, std::int64_t comparand) {
        return interlocked_compare_exchange64_explicit(destination, exchange, comparand, memory_order_seq_cst, memory_order_seq_cst);
    }
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
        return interlocked_and8_explicit(value, mask, memory_order_seq_cst);
    }

    std::int16_t interlocked_and16(volatile std::int16_t *value, std::int16_t mask) {
        return interlocked_and16_explicit(value, mask, memory_order_seq_cst);
    }

    std::int32_t interlocked_and32(volatile std::int32_t *value, std::int32_t mask) {
        return interlocked_and32_explicit(value, mask, memory_order_seq_cst);
    }

    std::int64_t interlocked_and64(volatile std::int64_t *value, std::int64_t mask) {
        return interlocked_and64_explicit(value, mask, memory_order_seq_cst);
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
        return interlocked_or8_explicit(value, mask, memory_order_seq_cst);
    }

    std::int16_t interlocked_or16(volatile std::int16_t *value, std::int16_t mask) {
        return interlocked_or16_explicit(value, mask, memory_order_seq_cst);
    }

    std::int32_t interlocked_or32(volatile std::int32_t *value, std::int32_t mask) {
        return interlocked_or32_explicit(value, mask, memory_order_seq_cst);
    }

    std::int64_t interlocked_or64(volatile std::int64_t *value, std::int64_t mask) {
        return interlocked_or64_explicit(value, mask, memory_order_seq_cst);
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
        return interlocked_xor8_explicit(value, mask, memory_order_seq_cst);
    }

    std::int16_t interlocked_xor16(volatile std::int16_t *value, std::int16_t mask) {
        return interlocked_xor16_explicit(value, mask, memory_order_seq_cst);
    }

    std::int32_t interlocked_xor32(volatile std::int32_t *value, std::int32_t mask) {
        return interlocked_xor32_explicit(value, mask, memory_order_seq_cst);
    }

    std::int64_t interlocked_xor64(volatile std::int64_t *value, std::int64_t mask) {
        return interlocked_xor64_explicit(value, mask, memory_order_seq_cst);
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
        return iso_volatile_store8_explicit(address, value, memory_order_seq_cst);
    }

    void iso_volatile_store16(volatile std::int16_t *address, std::int16_t value) {
        return iso_volatile_store16_explicit(address, value, memory_order_seq_cst);
    }

    void iso_volatile_store32(volatile int *address, std::uint32_t value) {
        return iso_volatile_store32_explicit(address, value, memory_order_seq_cst);
    }

    void iso_volatile_store64(volatile long long *address, std::uint64_t value) {
        return iso_volatile_store64_explicit(address, value, memory_order_seq_cst);
    }
}

namespace rainy::core::pal {
    void read_write_barrier() noexcept {
#if RAINY_USING_MSVC && !RAINY_IS_ARM64
        _ReadWriteBarrier();
#elif RAINY_USING_MSVC && RAINY_IS_ARM64
        __dmb(_ARM64_BARRIER_ISH);
#elif RAINY_IS_ARM64
        __asm__ __volatile__("dmb ish" ::: "memory");
#else
        __asm__ __volatile__("mfence" ::: "memory");
#endif
    }

    void read_barrier() noexcept {
#if RAINY_USING_MSVC && !RAINY_IS_ARM64
        _mm_lfence();
#elif RAINY_USING_MSVC && RAINY_IS_ARM64
        __dmb(_ARM64_BARRIER_ISHLD);
#elif RAINY_IS_ARM64
        __asm__ __volatile__("dmb ishld" ::: "memory");
#else
        __asm__ __volatile__("lfence" ::: "memory");
#endif
    }

    void write_barrier() noexcept {
#if RAINY_USING_MSVC && !RAINY_IS_ARM64
        _mm_sfence();
#elif RAINY_USING_MSVC && RAINY_IS_ARM64
        __dmb(_ARM64_BARRIER_ISHST);
#elif RAINY_IS_ARM64
        __asm__ __volatile__("dmb ishst" ::: "memory");
#else
        __asm__ __volatile__("sfence" ::: "memory");
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
            read_barrier();
        } else {
            read_write_barrier();
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
