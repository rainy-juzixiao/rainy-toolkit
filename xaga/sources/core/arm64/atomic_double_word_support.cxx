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
#include <intrin.h>

namespace rainy::core::pal {
    bool interlocked_compare_exchange_double_word(volatile native_double_word_t *destination, native_double_word_t exchange,
                                                  native_double_word_t *comparand) noexcept {
        return _InterlockedCompareExchange128(reinterpret_cast<volatile long long *>(destination), static_cast<long long>(exchange.hi),
                                              static_cast<long long>(exchange.lo), reinterpret_cast<long long *>(comparand)) != 0;
    }

    native_double_word_t atomic_load_double_word(const volatile native_double_word_t *address, memory_order /*order*/) noexcept {
        native_double_word_t expected{0, 0};
        _InterlockedCompareExchange128(reinterpret_cast<volatile long long *>(const_cast<volatile native_double_word_t *>(address)),
                                       0LL, 0LL, reinterpret_cast<long long *>(&expected));
        return expected;
    }

    void atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value, memory_order order) noexcept {
        native_double_word_t expected = atomic_load_double_word(address, memory_order::relaxed);
        while (!interlocked_compare_exchange_double_word(address, value, &expected)) {
        }
    }
}

#else

namespace rainy::core::pal {
    bool interlocked_compare_exchange_double_word(volatile native_double_word_t *destination, native_double_word_t exchange,
                                                  native_double_word_t *comparand) noexcept {
        std::uintptr_t cur_lo, cur_hi;
        int failed;
        __asm__ volatile("1:\n"
                         "    ldaxp  %[clo], %[chi], [%[dst]]\n"
                         "    cmp    %[clo], %[elo]\n"
                         "    ccmp   %[chi], %[ehi], #0, eq\n"
                         "    bne    2f\n"
                         "    stlxp  %w[fail], %[nlo], %[nhi], [%[dst]]\n"
                         "    cbnz   %w[fail], 1b\n"
                         "    b      3f\n"
                         "2:\n"
                         "    clrex\n"
                         "3:\n"
                         : [clo] "=&r"(cur_lo), [chi] "=&r"(cur_hi), [fail] "=&r"(failed)
                         : [dst] "r"(destination), [elo] "r"(comparand->lo), [ehi] "r"(comparand->hi), [nlo] "r"(exchange.lo),
                           [nhi] "r"(exchange.hi)
                         : "memory", "cc");

        if (cur_lo == comparand->lo && cur_hi == comparand->hi) {
            return true;
        }
        comparand->lo = cur_lo;
        comparand->hi = cur_hi;
        return false;
    }

    native_double_word_t atomic_load_double_word(const volatile native_double_word_t *address, memory_order /*order*/) noexcept {
        std::uintptr_t lo, hi;
        __asm__ volatile("ldaxp  %[lo], %[hi], [%[src]]\n"
                         "clrex\n"
                         : [lo] "=&r"(lo), [hi] "=&r"(hi)
                         : [src] "r"(address)
                         : "memory");

        return native_double_word_t{lo, hi};
    }

    void atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value,
                                  memory_order /*order*/) noexcept {
        std::uintptr_t cur_lo, cur_hi;
        int failed;

        __asm__ volatile("1:\n"
                         "    ldaxp  %[clo], %[chi], [%[dst]]\n"
                         "    stlxp  %w[fail], %[nlo], %[nhi], [%[dst]]\n"
                         "    cbnz   %w[fail], 1b\n"
                         : [clo] "=&r"(cur_lo), [chi] "=&r"(cur_hi), [fail] "=&r"(failed)
                         : [dst] "r"(address), [nlo] "r"(value.lo), [nhi] "r"(value.hi)
                         : "memory");
    }
}

#endif
