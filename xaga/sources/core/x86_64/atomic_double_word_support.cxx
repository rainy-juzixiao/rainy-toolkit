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
        bool success;
        // clang-format off
        __asm__ volatile("lock cmpxchg16b %[dst]\n"
                         "sete            %[ok]\n"
                         : [dst] "+m"(*destination),
                           "+a"(comparand->lo),
                           "+d"(comparand->hi),
                           [ok] "=q"(success)
                         : "b"(exchange.lo),
                           "c"(exchange.hi)
                         : "memory", "cc");
        // clang-format on
        return success;
    }

    native_double_word_t atomic_load_double_word(const volatile native_double_word_t *address, memory_order) noexcept {
        native_double_word_t result{0, 0};
        __asm__ volatile("lock cmpxchg16b %[src]\n"
                         : [src] "+m"(*const_cast<volatile native_double_word_t *>(address)), "+a"(result.lo), "+d"(result.hi)
                         : "b"(std::uintptr_t{0}), "c"(std::uintptr_t{0})
                         : "memory", "cc");
        return result;
    }

    void atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value, memory_order order) noexcept {
        native_double_word_t expected = atomic_load_double_word(address, memory_order::relaxed);
        while (!interlocked_compare_exchange_double_word(address, value, &expected)) {
        }
    }
}

#endif
