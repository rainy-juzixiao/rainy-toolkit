#include <rainy/core/layer.hpp>

#if RAINY_USING_MSVC
#include <intrin.h>

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

namespace rainy::core::pal {
    bool interlocked_compare_exchange_double_word(volatile native_double_word_t *destination, native_double_word_t exchange,
                                                  native_double_word_t *comparand) noexcept {
        long long result;
        RAINY_ATOMIC_DISPATCH(_InterlockedCompareExchange128, result, memory_order::seq_cst,
                              reinterpret_cast<volatile long long *>(destination), static_cast<long long>(exchange.hi),
                              static_cast<long long>(exchange.lo), reinterpret_cast<long long *>(comparand));
        return result != 0;
    }

    native_double_word_t atomic_load_double_word(const volatile native_double_word_t *address, memory_order order) noexcept {
        native_double_word_t expected{0, 0};
        long long result;
        RAINY_ATOMIC_DISPATCH(_InterlockedCompareExchange128, result, order,
                              reinterpret_cast<volatile long long *>(const_cast<volatile native_double_word_t *>(address)), 0LL, 0LL,
                              reinterpret_cast<long long *>(&expected));
        (void) result;
        return expected;
    }

    void atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value, memory_order order) noexcept {
        native_double_word_t expected = atomic_load_double_word(address, memory_order::relaxed);
        long long result;
        do {
            RAINY_ATOMIC_DISPATCH(_InterlockedCompareExchange128, result, order, reinterpret_cast<volatile long long *>(address),
                                  static_cast<long long>(value.hi), static_cast<long long>(value.lo),
                                  reinterpret_cast<long long *>(&expected));
        } while (result == 0);
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

    native_double_word_t atomic_load_double_word(const volatile native_double_word_t *address, memory_order order) noexcept {
        std::uintptr_t lo, hi;

        switch (order) {
            case memory_order::relaxed:
                __asm__ volatile("ldxp  %[lo], %[hi], [%[src]]\n"
                                 "clrex\n"
                                 : [lo] "=&r"(lo), [hi] "=&r"(hi)
                                 : [src] "r"(address)
                                 : "memory");
                break;
            case memory_order::acquire:
            case memory_order::seq_cst:
            default:
                __asm__ volatile("ldaxp %[lo], %[hi], [%[src]]\n"
                                 "clrex\n"
                                 : [lo] "=&r"(lo), [hi] "=&r"(hi)
                                 : [src] "r"(address)
                                 : "memory");
                break;
        }

        return native_double_word_t{lo, hi};
    }

    void atomic_store_double_word(volatile native_double_word_t *address, native_double_word_t value, memory_order order) noexcept {
        std::uintptr_t cur_lo, cur_hi;
        int failed;

        switch (order) {
            case memory_order::relaxed:
                __asm__ volatile("1:\n"
                                 "    ldxp  %[clo], %[chi], [%[dst]]\n"
                                 "    stxp  %w[fail], %[nlo], %[nhi], [%[dst]]\n"
                                 "    cbnz  %w[fail], 1b\n"
                                 : [clo] "=&r"(cur_lo), [chi] "=&r"(cur_hi), [fail] "=&r"(failed)
                                 : [dst] "r"(address), [nlo] "r"(value.lo), [nhi] "r"(value.hi)
                                 : "memory");
                break;
            case memory_order::release:
                __asm__ volatile("1:\n"
                                 "    ldxp  %[clo], %[chi], [%[dst]]\n"
                                 "    stlxp %w[fail], %[nlo], %[nhi], [%[dst]]\n"
                                 "    cbnz  %w[fail], 1b\n"
                                 : [clo] "=&r"(cur_lo), [chi] "=&r"(cur_hi), [fail] "=&r"(failed)
                                 : [dst] "r"(address), [nlo] "r"(value.lo), [nhi] "r"(value.hi)
                                 : "memory");
                break;
            case memory_order::seq_cst:
            case memory_order::acq_rel:
            default:
                __asm__ volatile("1:\n"
                                 "    ldaxp %[clo], %[chi], [%[dst]]\n"
                                 "    stlxp %w[fail], %[nlo], %[nhi], [%[dst]]\n"
                                 "    cbnz  %w[fail], 1b\n"
                                 : [clo] "=&r"(cur_lo), [chi] "=&r"(cur_hi), [fail] "=&r"(failed)
                                 : [dst] "r"(address), [nlo] "r"(value.lo), [nhi] "r"(value.hi)
                                 : "memory");
                break;
        }
    }
}

#endif
