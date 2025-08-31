#include <rainy/foundation/pal/implements/tgc_atomicinfra.hpp>
#include <rainy/foundation/pal/threading.hpp>

#if RAINY_USING_WINDOWS
#include <windows.h>
#endif

#if RAINY_USING_LINUX
#include <linux/futex.h>
#endif

namespace rainy::foundation::pal::atomicinfra::implements {
    void atomic_lock_acquire(long &spinlock) noexcept {
        int current_backoff = 1;
        constexpr int max_backoff = 64;
        while (core::pal::interlocked_exchange(&spinlock, 1) != 0) {
            while (core::pal::iso_volatile_load32(&reinterpret_cast<int &>(spinlock)) != 0) {
                for (int _Count_down = current_backoff; _Count_down != 0; --_Count_down) {
                    _mm_pause();
                }
                current_backoff = current_backoff < max_backoff ? current_backoff << 1 : max_backoff;
            }
        }
    }

    void atomic_lock_release(long &spinlock) noexcept {
        core::pal::read_write_barrier();
        core::pal::iso_volatile_load32(reinterpret_cast<int *>(&spinlock));
    }

    void atomic_lock_acquire(void **spinlock) noexcept {
        threading::implements::mtx_lock(spinlock);
    }

    void atomic_lock_release(void **spinlock) noexcept {
        threading::implements::mtx_unlock(spinlock);
    }

#if RAINY_USING_64_BIT_PLATFORM
    bool atomic_wait_compare_16_bytes(const void *storage, void *comparand, std::size_t, void *) noexcept {
        const auto dest = static_cast<long long *>(const_cast<void *>(storage));
        const auto cmp = static_cast<const long long *>(comparand);
        alignas(16) long long tmp[2] = {cmp[0], cmp[1]};
        return core::pal::interlocked_compare_exchange128(dest, tmp[1], tmp[0], tmp) != 0;
    }
#endif
}

namespace rainy::foundation::pal::atomicinfra::implements {
    //constexpr unsigned long long atomic_wait_no_deadline = 0xFFFF'FFFF'FFFF'FFFF;

    constexpr size_t wait_table_size_power = 8;
    constexpr size_t wait_table_size = 1 << wait_table_size_power;
    constexpr size_t wait_table_index_mask = wait_table_size - 1;

    struct wait_context {
        const void *storage; // Pointer to wait on
        wait_context *next;
        wait_context *prev;
        std::uintptr_t condition_variable;
    };

    struct guard_wait_context : wait_context {
        guard_wait_context(const void *storage, wait_context *const head) noexcept : wait_context{storage, head, head->prev, 0} {
            prev->next = this;
            next->prev = this;
        }

        ~guard_wait_context() {
            const auto next_local = next;
            const auto prev_local = prev;
            next->prev = prev_local;
            prev->next = next_local;
        }

        guard_wait_context(const guard_wait_context &) = delete;
        guard_wait_context &operator=(const guard_wait_context &) = delete;
    };

    class lock_auard : type_traits::helper::non_copyable {
    public:
        using mtx_t = threading::implements::mtx_t;

        explicit lock_auard(mtx_t locked) noexcept : locked(locked) {
            threading::implements::mtx_lock(&locked);
        }

        ~lock_auard() {
            threading::implements::mtx_unlock(&locked);
        }

    private:
        mtx_t locked;
    };

    struct wait_table_entry {
        alignas(core::hardware_destructive_interference_size) wait_context wait_list_head = {nullptr, nullptr, nullptr, 0};

        constexpr wait_table_entry() noexcept = default;

        threading::implements::mtx_t lock{};
    };

    wait_table_entry &atomic_wait_table_entry(const void *const storage) noexcept {
        static wait_table_entry wait_table[wait_table_size];
        auto index = reinterpret_cast<std::uintptr_t>(storage);
        index ^= index >> (wait_table_size_power * 2);
        index ^= index >> wait_table_size_power;
        return wait_table[index & wait_table_index_mask];
    }

    void assume_timeout() noexcept {
#ifdef RAINY_ENABLE_DEBUG
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

#if RAINY_USING_LINUX
    static int futex_word = 0;

    static void millis_to_timespec(unsigned long ms, struct timespec *ts) {
        ts->tv_sec = ms / 1000;
        ts->tv_nsec = (ms % 1000) * 1000000;
    }
#endif

#if RAINY_USING_WINDOWS
    int atomic_wait_direct(const void *const storage, void *const comparand, const std::size_t size,
                           const unsigned long remaining_timeout) noexcept {
        const auto result =
            WaitOnAddress(const_cast<volatile void *>(storage), const_cast<void *>(comparand), size, remaining_timeout);
        if (!result) {
            assume_timeout();
        }
        return result;
    }

    void atomic_notify_one_direct(const void *const storage) noexcept {
        WakeByAddressSingle(const_cast<void *>(storage));
    }

    void atomic_notify_all_direct(const void *const storage) noexcept {
        WakeByAddressAll(const_cast<void *>(storage));
    }
#else
    int atomic_wait_direct(const void *const storage, void *const comparand, const std::size_t size,
                           const unsigned long remaining_timeout) noexcept {
        if (std::memcmp(storage, comparand, size) != 0) {
            return 0;
        }
        ::timespec timeout_ts;
        millis_to_timespec(remaining_timeout, &timeout_ts);
        int local_val = futex_word;
        int ret = syscall(SYS_futex, &futex_word, FUTEX_WAIT, local_val, &timeout_ts, nullptr, 0);
        if (ret == -1) {
            if (errno == ETIMEDOUT) {
                assume_timeout();
            } else if (errno != EAGAIN) {
                std::abort();
            }
        }
        return ret;
    }

    void atomic_notify_one_direct(const void *const storage) noexcept {
        auto &entry = atomic_wait_table_entry(storage);
        lock_auard guard(entry.lock);
        wait_context *ctx = entry.wait_list_head.next;
        while (ctx && ctx != &entry.wait_list_head) {
            if (ctx->storage == storage) {
                pthread_cond_signal(reinterpret_cast<pthread_cond_t *>(&ctx->condition_variable));
                break;
            }
            ctx = ctx->next;
        }
    }

    void atomic_notify_all_direct(const void *const storage) noexcept {
        auto &entry = atomic_wait_table_entry(storage);
        lock_auard guard(entry.lock);
        wait_context *ctx = entry.wait_list_head.next;
        while (ctx && ctx != &entry.wait_list_head) {
            if (ctx->storage == storage) {
                pthread_cond_broadcast(reinterpret_cast<pthread_cond_t *>(&ctx->condition_variable)); // wake all on this condition
            }
            ctx = ctx->next;
        }
    }
#endif

    void atomic_notify_one_indirect(const void *const storage) noexcept {
        auto &entry = atomic_wait_table_entry(storage);
        lock_auard guard(entry.lock);
        wait_context *context = entry.wait_list_head.next;
        if (!context) {
            return;
        }
        for (; context != &entry.wait_list_head; context = context->next) {
            if (context->storage == storage) {
#if RAINY_USING_WINDOWS
                WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(context->condition_variable));
#else
                pthread_cond_signal(reinterpret_cast<pthread_cond_t *>(&context->condition_variable));
#endif
                break;
            }
        }
    }

    void atomic_notify_all_indirect(const void *const storage) noexcept {
        auto &entry = atomic_wait_table_entry(storage);
        lock_auard _guard(entry.lock);
        wait_context *context = entry.wait_list_head.next;
        if (!context) {
            return;
        }
        for (; context != &entry.wait_list_head; context = context->next) {
            if (context->storage == storage) {
#if RAINY_USING_WINDOWS
                WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&context->condition_variable));
#else
                pthread_cond_broadcast(reinterpret_cast<pthread_cond_t *>(&context->condition_variable));
#endif
            }
        }
    }

#if RAINY_USING_WINDOWS
    bool atomic_wait_indirect(const void *storage, void *comparand, size_t size, void *_Param,
                              atomic_wait_indirect_equal_callback_t are_equal, unsigned long remaining_timeout) noexcept {
        auto &entry = atomic_wait_table_entry(storage);
        lock_auard guard(entry.lock);
        if (!entry.wait_list_head.next) {
            entry.wait_list_head.next = &entry.wait_list_head;
            entry.wait_list_head.prev = &entry.wait_list_head;
        }
        guard_wait_context context{storage, &entry.wait_list_head};
        for (;;) {
            if (!are_equal(storage, comparand, size, _Param)) {
                return 1;
            }
            if (!SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&context.condition_variable),
                                           reinterpret_cast<PSRWLOCK>(threading::implements::native_mtx_handle(&entry.lock)),
                                           remaining_timeout, 0)) {
                implements::assume_timeout();
                return 0;
            }
            if (remaining_timeout != implements::atomic_wait_no_timeout) {
                return 1;
            }
        }
    }
#else
    bool atomic_wait_indirect(const void *storage, void *comparand, size_t size, void *_Param,
                              atomic_wait_indirect_equal_callback_t are_equal, unsigned long remaining_timeout) noexcept {
        auto &entry = atomic_wait_table_entry(storage);
        lock_auard guard(entry.lock);
        if (!entry.wait_list_head.next) {
            entry.wait_list_head.next = &entry.wait_list_head;
            entry.wait_list_head.prev = &entry.wait_list_head;
        }
        guard_wait_context context{storage, &entry.wait_list_head};
        for (;;) {
            if (!are_equal(storage, comparand, size, _Param)) {
                return true;
            }
            int result;
            if (remaining_timeout == implements::atomic_wait_no_timeout) {
                result = pthread_cond_wait(reinterpret_cast<pthread_cond_t *>(&context.condition_variable),
                                           reinterpret_cast<pthread_mutex_t *>(&entry.lock));
            } else {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += remaining_timeout / 1000;
                ts.tv_nsec += (remaining_timeout % 1000) * 1000000;
                if (ts.tv_nsec >= 1000000000L) {
                    ts.tv_sec += 1;
                    ts.tv_nsec -= 1000000000L;
                }
                result = pthread_cond_timedwait(reinterpret_cast<pthread_cond_t *>(&context.condition_variable),
                                                reinterpret_cast<pthread_mutex_t *>(&entry.lock), &ts);
            }
            if (result == ETIMEDOUT) {
                implements::assume_timeout();
                return false;
            }
            if (remaining_timeout != implements::atomic_wait_no_timeout) {
                return true;
            }
        }
    }
#endif

    void **atomic_get_mutex(const void *const key) noexcept {
        constexpr size_t table_size_power = 8;
        constexpr size_t table_size = 1 << table_size_power;
        constexpr size_t table_index_mask = table_size - 1;
        struct table_entry {
            alignas(core::hardware_destructive_interference_size) void **mutex;
        };
        static table_entry table[table_size]{};
        auto index = reinterpret_cast<std::uintptr_t>(key);
        index ^= index >> (table_size_power * 2);
        index ^= index >> table_size_power;
        return table[index & table_index_mask].mutex;
    }
}
