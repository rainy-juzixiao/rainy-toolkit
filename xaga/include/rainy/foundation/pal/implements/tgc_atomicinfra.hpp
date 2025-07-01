#ifndef RAINY_FOUNDATION_PAL_IMPLEMENTS_TGC_ATOMICINFRA_HPP
#define RAINY_FOUNDATION_PAL_IMPLEMENTS_TGC_ATOMICINFRA_HPP
#include <rainy/core/core.hpp>

#if RAINY_HAS_CXX20 && !RAINY_USING_MSVC && !RAINY_USING_EDG
#define RAINY_CMPXCHG_MASK_OUT_PADDING_BITS 1
#else
#define RAINY_CMPXCHG_MASK_OUT_PADDING_BITS 0
#endif

namespace rainy::foundation::pal::atomicinfra::implements {
    using atomic_counter_t = unsigned long;

    template <typename Integral, typename Ty>
    RAINY_NODISCARD volatile Integral *atomic_address_as(Ty &source) noexcept {
        static_assert(type_traits::primary_types::is_integral_v<Integral>, "Tried to reinterpret memory as non-integral");
        return &reinterpret_cast<volatile Integral &>(source);
    }

    template <typename Integral, typename Ty>
    RAINY_NODISCARD const volatile Integral *atomic_address_as(const Ty &source) noexcept {
        static_assert(type_traits::primary_types::is_integral_v<Integral>, "Tried to reinterpret memory as non-integral");
        return &reinterpret_cast<const volatile Integral &>(source);
    }

    template <typename Integral, typename Ty>
    RAINY_NODISCARD Integral atomic_reinterpret_as(const Ty &source) noexcept {
        static_assert(type_traits::primary_types::is_integral_v<Integral>, "Tried to reinterpret memory as non-integral");
        if constexpr (type_traits::primary_types::is_integral_v<Ty> && sizeof(Integral) == sizeof(Ty)) {
            return static_cast<Integral>(source);
        } else if constexpr (type_traits::primary_types::is_pointer_v<Ty> && sizeof(Integral) == sizeof(Ty)) {
            return reinterpret_cast<Integral>(source);
        } else {
            Integral result{};
            std::memcpy(&result, utility::addressof(source), sizeof(source));
            return result;
        }
    }

    template <typename Ty>
    constexpr bool might_have_non_value_bits = !type_traits::type_properties::has_unique_object_representations_v<Ty> && !type_traits::primary_types::is_floating_point_v<Ty>;
}

namespace rainy::foundation::pal::atomicinfra {
    enum class memory_order {
        relaxed,
        consume,
        acquire,
        release,
        acq_rel,
        seq_cst
    };

    inline constexpr memory_order memory_order_relaxed = memory_order::relaxed;
    inline constexpr memory_order memory_order_consume = memory_order::consume;
    inline constexpr memory_order memory_order_acquire = memory_order::acquire;
    inline constexpr memory_order memory_order_release = memory_order::release;
    inline constexpr memory_order memory_order_acq_rel = memory_order::acq_rel;
    inline constexpr memory_order memory_order_seq_cst = memory_order::seq_cst;
}

namespace rainy::foundation::pal::atomicinfra::implements {
    RAINY_TOOLKIT_API void invalidmemory_order();
    RAINY_TOOLKIT_API void check_store_memory_order(const memory_order _order) noexcept;
    RAINY_TOOLKIT_API void check_load_memory_order(const memory_order _order) noexcept;
    RAINY_TOOLKIT_API void check_memory_order(memory_order _order) noexcept;
}

namespace rainy::foundation::pal::atomicinfra::implements {
    constexpr unsigned long long atomic_wait_no_timeout = 0xFFFF'FFFF;

    using atomic_wait_indirect_equal_callback_t = bool (*)(const void *storage, void *comprand, std::size_t size,
                                                           void *param) noexcept;

    RAINY_TOOLKIT_API int atomic_wait_direct(const void *const storage, void *const comparand, const std::size_t size,
                                             const unsigned long remaining_timeout) noexcept;
    RAINY_TOOLKIT_API void atomic_notify_one_direct(const void *const storage) noexcept;
    RAINY_TOOLKIT_API void atomic_notify_all_direct(const void *const storage) noexcept;

    RAINY_TOOLKIT_API bool atomic_wait_indirect(const void *storage, void *comparand, size_t size, void *param,
                                                atomic_wait_indirect_equal_callback_t are_equal,
                                                unsigned long remaining_timeout) noexcept;
    RAINY_TOOLKIT_API void atomic_notify_one_indirect(const void *const storage) noexcept;
    RAINY_TOOLKIT_API void atomic_notify_all_indirect(const void *const storage) noexcept;
    RAINY_TOOLKIT_API void **atomic_get_mutex(const void *const key) noexcept;

    template <typename Ty, std::size_t = sizeof(type_traits::reference_modify::remove_reference_t<Ty>)>
    struct atomic_storage;

    template <typename Ty, typename ValueType>
    void wait_direct(const atomic_storage<Ty> *const storage, ValueType expected_bytes, const memory_order order) noexcept {
        const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage->storage_)));
        for (;;) {
            const ValueType observed_bytes = atomic_reinterpret_as<ValueType>(storage->load(order));
            if (expected_bytes != observed_bytes) {
                return;
            }
            atomic_wait_direct(storage_ptr, &expected_bytes, sizeof(ValueType), atomic_wait_no_timeout);
        }
    }

    RAINY_TOOLKIT_API void atomic_lock_acquire(long &spinlock) noexcept;
    RAINY_TOOLKIT_API void atomic_lock_release(long &spinlock) noexcept;
    RAINY_TOOLKIT_API void atomic_lock_acquire(void **spinlock) noexcept;
    RAINY_TOOLKIT_API void atomic_lock_release(void **spinlock) noexcept;
}

namespace rainy::foundation::pal::atomicinfra::implements {
    template <class Ty>
    struct storage_for {
        storage_for() = default;
        storage_for(const storage_for &) = delete;
        storage_for &operator=(const storage_for &) = delete;

        RAINY_NODISCARD Ty &reference() noexcept {
            return reinterpret_cast<Ty &>(storage);
        }

        RAINY_NODISCARD Ty *ptr() noexcept {
            return reinterpret_cast<Ty *>(&storage);
        }

        alignas(Ty) unsigned char storage[sizeof(Ty)];
    };

    template <typename Lock>
    class atomic_lock_gurad : type_traits::helper::non_copyable {
    public:
        explicit atomic_lock_gurad(Lock &lock) noexcept : lock(lock) {
            atomic_lock_acquire(lock);
        }

        ~atomic_lock_gurad() {
            atomic_lock_release(lock);
        }

    private:
        Lock &lock;
    };

    template <typename spinlock_t>
    bool atomic_wait_compare_nonlock_free(const void *_Storage, void *_Comparand, size_t _Size,
                                                      void *spinlock_raw) noexcept {
        spinlock_t &spinlock = *static_cast<spinlock_t *>(spinlock_raw);
        atomic_lock_acquire(spinlock);
        const auto _Cmpresult = _CSTD memcmp(_Storage, _Comparand, _Size);
        atomic_lock_release(spinlock);
        return _Cmpresult == 0;
    }

    template <typename Ty>
    struct atomic_padded {
        alignas(sizeof(Ty)) mutable Ty value;
    };

    template <class _Ty>
    struct atomic_storage_types {
        using storage = atomic_padded<_Ty>;
        using spinlock = long;
    };

    template <typename Ty>
    struct atomic_storage_types<Ty &> {
        using storage = Ty &;
        using spinlock = void*;
    };

    template <typename Ty, std::size_t>
    struct atomic_storage {
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty>;
        using guard = atomic_lock_gurad<typename atomic_storage_types<Ty>::spinlock>;

        atomic_storage() = default;

        constexpr atomic_storage(const value_type &value) noexcept : storage_(value) {
        }

        void store(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            check_store_memory_order(order);
            guard lock{spinlock};
            storage_ = value;
        }

        RAINY_NODISCARD value_type load(const memory_order order = memory_order_seq_cst) const noexcept {
            check_load_memory_order(order);
            guard lock{spinlock};
            value_type _Local(storage_);
            return _Local;
        }

        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            guard lock{spinlock};
            value_type result(storage_);
            storage_ = value;
            return result;
        }

        bool compare_exchange_strong(
            value_type &expected, const value_type _Desired,
            const memory_order order = memory_order_seq_cst) noexcept { // CAS with sequential consistency, plain
            check_memory_order(order);
            const auto storage_ptr = utility::addressof(storage_);
            const auto expected_ptr = utility::addressof(expected);
            bool result{};
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            ::__builtin_zero_non_value_bits(expected_ptr);
#endif // _CMPXCHG_MASK_OUT_PADDING_BITS
            atomic_lock_gurad lock{spinlock};
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<value_type>) {
                storage_for<value_type> local;
                const auto local_ptr = local.ptr();
                std::memcpy(local_ptr, storage_ptr, sizeof(value_type));
                ::__builtin_zero_non_value_bits(local_ptr);
                result = std::memcmp(local_ptr, expected_ptr, sizeof(value_type)) == 0;
            } else {
                result = std::memcmp(storage_ptr, expected_ptr, sizeof(value_type)) == 0;
            }
#else
            result = std::memcmp(storage_ptr, expected_ptr, sizeof(value_type)) == 0;
#endif
            if (result) {
                std::memcpy(storage_ptr, _STD addressof(_Desired), sizeof(value_type));
            } else {
                std::memcpy(expected_ptr, storage_ptr, sizeof(value_type));
            }
            return result;
        }

        void wait(value_type expected, memory_order = memory_order_seq_cst) const noexcept {
            const auto storage_ptr = utility::addressof(storage_);
            const auto expected_ptr = utility::addressof(expected);
            for (;;) {
                {
                    if (std::memcmp(storage_ptr, expected_ptr, sizeof(value_type)) != 0) {
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
                        if constexpr (might_have_non_value_bits<value_type>) {
                            storage_for<value_type> local;
                            const auto local_ptr = local.ptr();
                            std::memcpy(local_ptr, storage_ptr, sizeof(value_type));
                            ::__builtin_zero_non_value_bits(local_ptr);
                            ::__builtin_zero_non_value_bits(expected_ptr);
                            if (std::memcmp(local_ptr, expected_ptr, sizeof(value_type)) == 0) {
                                std::memcpy(expected_ptr, storage_ptr, sizeof(value_type));
                            } else {
                                return;
                            }
                        } else
#endif
                        {
                            return;
                        }
                    }
                }
                atomic_wait_indirect(storage_ptr, expected_ptr, sizeof(value_type), &spinlock,
                                     &atomic_wait_compare_nonlock_free<decltype(spinlock)>,atomic_wait_no_timeout);
            }
        }

        void notify_one() noexcept {
            atomic_notify_one_indirect(utility::addressof(storage_));
        }

        void notify_all() noexcept {
            atomic_notify_all_indirect(utility::addressof(storage_));
        }

        Ty storage_{};

    protected:
        void initspinlock_for_ref() noexcept {
            spinlock = atomic_get_mutex(utility::addressof(storage_));
        }

    private:
        mutable typename atomic_storage_types<Ty>::spinlock spinlock{};
    };

    template <typename Ty>
    struct atomic_storage<Ty, 1> {
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty>;

        atomic_storage() = default;

        constexpr atomic_storage(const Ty &value) noexcept : _storage{value} {
        }

        void store(const value_type value) noexcept { // store with sequential consistency
            const auto mem = implements::atomic_address_as<char>(_storage);
            const char as_bytes = implements::atomic_reinterpret_as<char>(value);
            (void) core::pal::interlocked_exchange8(mem, as_bytes);
        }

        void store(const value_type value, const memory_order order) noexcept { // store with given memory order
            const auto mem = implements::atomic_address_as<char>(_storage);
            const char as_bytes = implements::atomic_reinterpret_as<char>(value);
            check_store_memory_order(order);
            if (order == memory_order_relaxed) {
                core::pal::iso_volatile_store8(mem, as_bytes);
            } else if (order == memory_order_release) {
                core::pal::read_write_barrier();
                core::pal::iso_volatile_store8(mem, as_bytes);
            } else {
                store(value);
            }
        }

        _NODISCARD value_type load(const memory_order order = memory_order_seq_cst) const noexcept { // load with given memory order
            const auto mem = implements::atomic_address_as<char>(_storage);
            char as_bytes = core::pal::iso_volatile_load8(mem);
            check_memory_order(order);
            if (order != memory_order_relaxed) {
                core::pal::read_write_barrier();
            }
            return reinterpret_cast<value_type &>(as_bytes);
        }

        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            char as_bytes = core::pal::interlocked_exchange8(implements::atomic_address_as<char>(_storage),
                                                             implements::atomic_reinterpret_as<char>(value));
            return reinterpret_cast<value_type &>(as_bytes);
        }

        bool compare_exchange_strong(value_type &expected, const value_type desired,
                                     const memory_order _order = memory_order_seq_cst) noexcept { // CAS with given memory order
            char expected_bytes = implements::atomic_reinterpret_as<char>(expected); // read before atomic operation
            char prev_bytes;
            check_memory_order(_order);
            prev_bytes = core::pal::interlocked_compare_exchange8(implements::atomic_address_as<char>(_storage),
                                                                  implements::atomic_reinterpret_as<char>(desired), expected_bytes);
            if (prev_bytes == expected_bytes) {
                return true;
            }
            reinterpret_cast<char &>(expected) = prev_bytes;
            return false;
        }

        void wait(const value_type expected, const memory_order order = memory_order_seq_cst) const noexcept {
            wait_direct(this, implements::atomic_reinterpret_as<char>(expected), order);
        }

        void notify_one() noexcept {
            const auto _storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(_storage)));
            atomic_notify_one_direct(_storage_ptr);
        }

        void notify_all() noexcept {
            const auto _storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(_storage)));
            atomic_notify_all_direct(_storage_ptr);
        }

        typename atomic_storage_types<Ty>::storage _storage;
    };
}

#endif
