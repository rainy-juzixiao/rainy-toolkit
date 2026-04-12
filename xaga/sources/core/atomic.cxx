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

#if RAINY_USING_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif RAINY_USING_LINUX
#include <cerrno>
#include <ctime>
#include <linux/futex.h>
#include <pthread.h>
#include <unistd.h>
#endif

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26110)
#endif

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

#if RAINY_USING_MACOS
        return interlocked_exchange_subtract64(reinterpret_cast<volatile std::int64_t *>(value),
                                               static_cast<const std::int64_t>(amount));
#else
        return interlocked_exchange_subtract64(static_cast<volatile std::int64_t *>(value),
                                               static_cast<const std::int64_t>(amount));
#endif

#else

#if RAINY_USING_MACOS
        return interlocked_exchange_subtract32(reinterpret_cast<volatile std::int32_t *>(value),
                                               static_cast<const std::int32_t>(amount));
#else
        return interlocked_exchange_subtract32(static_cast<volatile std::int32_t *>(value),
                                               static_cast<const std::int32_t>(amount));
#endif

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

#if RAINY_USING_MACOS
        return iso_volatile_load64(reinterpret_cast<const volatile std::int64_t *>(address));
#else
        return iso_volatile_load64(static_cast<const volatile std::int64_t *>(address));
#endif

#else

#if RAINY_USING_MACOS
        return iso_volatile_load32(reinterpret_cast<const volatile std::int32_t *>(address));
#else
        return iso_volatile_load32(static_cast<const volatile std::int32_t *>(address));
#endif

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
    void *interlocked_exchange_pointer(volatile void **target, void *value) {
        return interlocked_exchange_pointer_explicit(target, value, memory_order_seq_cst);
    }

    void *interlocked_compare_exchange_pointer(volatile void **destination, void *exchange, void *comparand) {
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

#if RAINY_USING_MACOS
        return interlocked_and64(reinterpret_cast<volatile std::int64_t *>(value),
                                 static_cast<const std::int64_t>(mask));
#else
        return interlocked_and64(static_cast<volatile std::int64_t *>(value),
                                 static_cast<const std::int64_t>(mask));
#endif

#else

#if RAINY_USING_MACOS
        return interlocked_and32(reinterpret_cast<volatile std::int32_t *>(value),
                                 static_cast<const std::int32_t>(mask));
#else
        return interlocked_and32(static_cast<volatile std::int32_t *>(value),
                                 static_cast<const std::int32_t>(mask));
#endif

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

#if RAINY_USING_MACOS
        return interlocked_or64(reinterpret_cast<volatile std::int64_t *>(value),
                                static_cast<const std::int64_t>(mask));
#else
        return interlocked_or64(static_cast<volatile std::int64_t *>(value),
                                static_cast<const std::int64_t>(mask));
#endif

#else

#if RAINY_USING_MACOS
        return interlocked_or32(reinterpret_cast<volatile std::int32_t *>(value),
                                static_cast<const std::int32_t>(mask));
#else
        return interlocked_or32(static_cast<volatile std::int32_t *>(value),
                                static_cast<const std::int32_t>(mask));
#endif

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

#if RAINY_USING_MACOS
        return interlocked_xor64(reinterpret_cast<volatile std::int64_t *>(value),
                                 static_cast<const std::int64_t>(mask));
#else
        return interlocked_xor64(static_cast<volatile std::int64_t *>(value),
                                 static_cast<const std::int64_t>(mask));
#endif

#else

#if RAINY_USING_MACOS
        return interlocked_xor32(reinterpret_cast<volatile std::int32_t *>(value),
                                 static_cast<const std::int32_t>(mask));
#else
        return interlocked_xor32(static_cast<volatile std::int32_t *>(value),
                                 static_cast<const std::int32_t>(mask));
#endif

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

#if RAINY_USING_MACOS
        iso_volatile_store64(reinterpret_cast<volatile std::int64_t *>(address),
                             *static_cast<std::int64_t *>(value));
#else
        iso_volatile_store64(static_cast<volatile std::int64_t *>(address),
                             *static_cast<std::int64_t *>(value));
#endif

#else

#if RAINY_USING_MACOS
        iso_volatile_store32(reinterpret_cast<volatile std::int32_t *>(address),
                             *static_cast<std::int32_t *>(value));
#else
        iso_volatile_store32(static_cast<volatile std::int32_t *>(address),
                             *static_cast<std::int32_t *>(value));
#endif

#endif
    }

    void iso_volatile_store8(volatile std::int8_t *address, std::int8_t value) {
        return iso_volatile_store8_explicit(address, value, memory_order_seq_cst);
    }

    void iso_volatile_store16(volatile std::int16_t *address, std::int16_t value) {
        return iso_volatile_store16_explicit(address, value, memory_order_seq_cst);
    }

    void iso_volatile_store32(volatile std::int32_t *address, std::int32_t value) {
        return iso_volatile_store32_explicit(address, value, memory_order_seq_cst);
    }

    void iso_volatile_store64(volatile std::int64_t *address, std::int64_t value) {
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
}

namespace rainy::core::pal::implements {
    static void verify_timeout() noexcept {
#if RAINY_ENABLE_DEBUG
#if RAINY_USING_WINDOWS
        if (::GetLastError() != ERROR_TIMEOUT) {
            std::abort();
        }
#else
        if (errno != ETIMEDOUT) {
            std::abort();
        }
#endif
#endif
    }

    static bool supports_direct(const std::size_t size) noexcept {
#if RAINY_USING_WINDOWS
        return size == 1 || size == 2 || size == 4 || size == 8;
#elif RAINY_USING_LINUX
        return size == 4;
#else
        return false;
#endif
    }
}

namespace rainy::core::pal::implements {
    struct platform_lock {
#if RAINY_USING_WINDOWS
        SRWLOCK lock = SRWLOCK_INIT;
#else
        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
#endif

        void acquire() noexcept {
#if RAINY_USING_WINDOWS
            ::AcquireSRWLockExclusive(&lock);
#else
            ::pthread_mutex_lock(&lock);
#endif
        }

        void release() noexcept {
#if RAINY_USING_WINDOWS
            ::ReleaseSRWLockExclusive(&lock);
#else
            ::pthread_mutex_unlock(&lock);
#endif
        }

#if RAINY_USING_WINDOWS
        PSRWLOCK native() noexcept {
            return &lock;
        }
#else
        pthread_mutex_t *native() noexcept {
            return &lock;
        }
#endif

        platform_lock() noexcept = default;
        platform_lock(const platform_lock &) = delete;
        platform_lock &operator=(const platform_lock &) = delete;
    };

    struct platform_lock_guard {
        platform_lock &target;
        explicit platform_lock_guard(platform_lock &target) noexcept : target(target) {
            target.acquire();
        }
        ~platform_lock_guard() noexcept {
            target.release();
        }
        platform_lock_guard(const platform_lock_guard &) = delete;
        platform_lock_guard &operator=(const platform_lock_guard &) = delete;
    };

    struct wait_context {
        const void *storage;
        wait_context *next;
        wait_context *prev;
#if RAINY_USING_WINDOWS
        CONDITION_VARIABLE cond;
#else
        pthread_cond_t cond;
#endif
#if RAINY_ENABLE_DEBUG
        uintptr_t magic;
#endif
    };

    struct scoped_wait_context : wait_context {
        scoped_wait_context(const void *storage, wait_context *head) noexcept : wait_context() {
            this->storage = storage;
            this->next = head;
            this->prev = head->prev;
#if RAINY_ENABLE_DEBUG
            this->magic = 0xDEADBEEF;
#endif
            prev->next = this;
            next->prev = this;

#if RAINY_USING_WINDOWS
            ::InitializeConditionVariable(&cond);
#else
            ::pthread_cond_init(&cond, nullptr);
#endif
        }

        ~scoped_wait_context() noexcept {
            // 先保存前后节点
            auto *next_node = this->next;
            // 从链表中移除
            if (auto *prev_node = this->prev; next_node && prev_node) {
                next_node->prev = prev_node;
                prev_node->next = next_node;
            }
            // 置空指针，防止误用
            this->next = nullptr;
            this->prev = nullptr;
#if RAINY_ENABLE_DEBUG
            this->magic = 0;
#endif

#if !RAINY_USING_WINDOWS
            ::pthread_cond_destroy(&cond);
#endif
        }

        RAINY_NODISCARD bool is_valid() const noexcept {
#if RAINY_ENABLE_DEBUG
            return magic == 0xDEADBEEF && next != nullptr && prev != nullptr;
#else
            return next != nullptr && prev != nullptr;
#endif
        }

        scoped_wait_context(const scoped_wait_context &) = delete;
        scoped_wait_context &operator=(const scoped_wait_context &) = delete;
    };

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4324)
#endif

    struct alignas(core::hardware_destructive_interference_size) wait_table_entry {
        // 确保链表已初始化
        void ensure_initialized() noexcept {
            if (!head.next) {
                head.next = &head;
                head.prev = &head;
            }
        }

        constexpr wait_table_entry() noexcept = default;

        // NOLINTBEGIN
        wait_context head{nullptr, nullptr, nullptr
#if RAINY_USING_WINDOWS
                          ,
                          CONDITION_VARIABLE()
#else
                          ,
                          PTHREAD_COND_INITIALIZER
#endif
        };
        // NOLINTEND
        platform_lock lock{};
    };

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

    constexpr std::size_t table_power = 8;
    constexpr std::size_t table_size = 1u << table_power;
    constexpr std::size_t table_mask = table_size - 1;

    static wait_table_entry &entry_for(const void *storage) noexcept {
        static wait_table_entry table[table_size];
        auto key = reinterpret_cast<std::uintptr_t>(storage);
        key ^= key >> (table_power * 2);
        key ^= key >> table_power;
        return table[key & table_mask]; // NOLINT
    }

    static bool wait_direct(const void *storage, const void *comparand, std::size_t size) noexcept {
#if RAINY_USING_WINDOWS
        const BOOL ok =
            ::WaitOnAddress(const_cast<volatile void *>(storage), const_cast<void *>(comparand), static_cast<SIZE_T>(size), INFINITE);
        if (!ok)
            verify_timeout();
        return static_cast<bool>(ok);
#elif RAINY_USING_LINUX
        int val = 0;
        std::memcpy(&val, comparand, sizeof(int));
        if (const long ret = ::syscall(SYS_futex, const_cast<void *>(storage), FUTEX_WAIT_PRIVATE, val, nullptr, nullptr, 0);
            ret == -1 && errno != EAGAIN && errno != EINTR) {
            verify_timeout();
        }
        return true;
#else
        (void) storage;
        (void) comparand;
        (void) size;
        std::abort();
        return false;
#endif
    }

    static void notify_one_direct(const void *storage) noexcept {
#if RAINY_USING_WINDOWS
        ::WakeByAddressSingle(const_cast<void *>(storage));
#elif RAINY_USING_LINUX
        ::syscall(SYS_futex, const_cast<void *>(storage), FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
#endif
    }

    static void notify_all_direct(const void *storage) noexcept {
#if RAINY_USING_WINDOWS
        ::WakeByAddressAll(const_cast<void *>(storage));
#elif RAINY_USING_LINUX
        ::syscall(SYS_futex, const_cast<void *>(storage), FUTEX_WAKE_PRIVATE, INT_MAX, nullptr, nullptr, 0);
#endif
    }

    static void wait_indirect(const void *storage, const void *comparand, const std::size_t size, const atomic_wait_equal_fn equal_fn,
                              void *ctx) noexcept {
        auto &entry = entry_for(storage);
        platform_lock_guard guard(entry.lock);
        entry.ensure_initialized();
        scoped_wait_context wctx{storage, &entry.head};
        for (;;) {
            if (const bool still_same =
                    equal_fn ? equal_fn(storage, comparand, size, ctx) : (std::memcmp(storage, comparand, size) == 0);
                !still_same) {
                return;
            }
#if RAINY_USING_WINDOWS
            const BOOL ok = ::SleepConditionVariableSRW(&wctx.cond, entry.lock.native(), INFINITE, 0);
            if (!ok) {
                verify_timeout();
                return;
            }
#else
            if (const int ret = ::pthread_cond_wait(&wctx.cond, entry.lock.native()); ret != 0 && ret != EINTR) {
                verify_timeout();
                return;
            }
#endif
        }
    }

    static void notify_one_indirect(const void *storage) noexcept {
        auto &entry = entry_for(storage);
        platform_lock_guard guard(entry.lock);
        // 确保链表已初始化
        if (!entry.head.next) {
            entry.ensure_initialized();
        }
        wait_context *ctx = entry.head.next;
        wait_context *next_ctx = nullptr;
        while (ctx != &entry.head) {
            if (ctx == nullptr) {
                break;
            }
            // 预先保存下一个节点
            next_ctx = ctx->next;
            // 检查节点有效性
            if (const bool node_valid = (ctx->prev != nullptr && ctx->next != nullptr); !node_valid) {
                ctx = next_ctx;
                continue;
            }
            if (ctx->storage == storage) {
#if RAINY_USING_WINDOWS
                ::WakeConditionVariable(&ctx->cond);
#else
                ::pthread_cond_signal(&ctx->cond);
#endif
                break;
            }
            ctx = next_ctx;
        }
    }

    static void notify_all_indirect(const void *storage) noexcept {
        auto &entry = entry_for(storage);
        platform_lock_guard guard(entry.lock);
        // 确保链表已初始化
        if (!entry.head.next) {
            entry.ensure_initialized();
        }
        // 安全遍历：先收集所有需要唤醒的节点
        wait_context *ctx = entry.head.next;
        wait_context *next_ctx = nullptr;
        while (ctx != &entry.head) {
            if (ctx == nullptr) {
                break;
            }
            // 预先保存下一个节点
            next_ctx = ctx->next;
            // 检查节点有效性
            if (const bool node_valid = (ctx->prev != nullptr && ctx->next != nullptr); !node_valid) {
                ctx = next_ctx;
                continue;
            }
            if (ctx->storage == storage) {
#if RAINY_USING_WINDOWS
                ::WakeAllConditionVariable(&ctx->cond);
#else
                ::pthread_cond_broadcast(&ctx->cond);
#endif
            }
            ctx = next_ctx;
        }
    }
}

namespace rainy::core::pal {
    void atomic_wait(const void *storage, const void *comparand, const std::size_t size, const atomic_wait_equal_fn equal_fn,
                     void *ctx) noexcept {
        if (implements::supports_direct(size) && equal_fn == nullptr) {
            if (std::memcmp(storage, comparand, size) == 0) {
                implements::wait_direct(storage, comparand, size);
            }
        } else {
            implements::wait_indirect(storage, comparand, size, equal_fn, ctx);
        }
    }

    void atomic_notify_one(const void *storage, const std::size_t size) noexcept {
        if (implements::supports_direct(size)) {
            implements::notify_one_direct(storage);
        } else {
            implements::notify_one_indirect(storage);
        }
    }

    void atomic_notify_all(const void *storage, const std::size_t size) noexcept {
        if (implements::supports_direct(size)) {
            implements::notify_all_direct(storage);
        } else {
            implements::notify_all_indirect(storage);
        }
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

// NOLINTEND(cppcoreguidelines-avoid-do-while,readability-duplicate-branches,clang-analyzer-core.UndefinedBinaryOperatorResult)
