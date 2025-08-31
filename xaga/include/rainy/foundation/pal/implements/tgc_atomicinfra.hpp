#ifndef RAINY_FOUNDATION_PAL_IMPLEMENTS_TGCatomicINFRA_HPP
#define RAINY_FOUNDATION_PAL_IMPLEMENTS_TGCatomicINFRA_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::pal::atomicinfra::implements {
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
    struct form_mask_t {};
    static inline constexpr form_mask_t form_mask{};
#endif

    using atomic_counter_t = std::uint32_t;

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
    constexpr bool might_have_non_value_bits =
        !type_traits::type_properties::has_unique_object_representations_v<Ty> && !type_traits::primary_types::is_floating_point_v<Ty>;

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
    RAINY_TOOLKIT_API void invalid_memory_order();
    RAINY_TOOLKIT_API void check_store_memory_order(const memory_order order) noexcept;
    RAINY_TOOLKIT_API void check_load_memory_order(const memory_order order) noexcept;
    RAINY_TOOLKIT_API void check_memory_order(memory_order order) noexcept;
}

namespace rainy::foundation::pal::atomicinfra::implements {
    constexpr unsigned long long atomic_wait_no_timeout = 0xFFFF'FFFF;
    using atomic_wait_indirect_equal_callback_t = bool (*)(const void *storage, void *comprand, std::size_t size,
                                                           void *param) noexcept;

    RAINY_TOOLKIT_API int atomic_wait_direct(const void *const storage, void *const comparand, const std::size_t size,
                                             const std::uint32_t remaining_timeout) noexcept;
    RAINY_TOOLKIT_API void atomic_notify_one_direct(const void *const storage) noexcept;
    RAINY_TOOLKIT_API void atomic_notify_all_direct(const void *const storage) noexcept;

    RAINY_TOOLKIT_API bool atomic_wait_indirect(const void *storage, void *comparand, std::size_t size, void *param,
                                                atomic_wait_indirect_equal_callback_t are_equal,
                                                std::uint32_t remaining_timeout) noexcept;
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
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
                using remove_cvref_t = type_traits::cv_modify::remove_cvref_t<Ty>;
                if constexpr (might_have_non_value_bits<remove_cvref_t>) {
                    storage_for<remove_cvref_t> mask{form_mask};
                    const remove_cvref_t mask_val = atomic_reinterpret_as<remove_cvref_t>(mask.reference());
                    if (((expected_bytes ^ observed_bytes) & mask_val) == 0) {
                        expected_bytes = observed_bytes;
                        continue;
                    }
                }
#endif
                return;
            }
            atomic_wait_direct(storage_ptr, &expected_bytes, sizeof(ValueType), atomic_wait_no_timeout);
        }
    }

    RAINY_TOOLKIT_API void atomic_lock_acquire(std::int32_t &spinlock) noexcept;
    RAINY_TOOLKIT_API void atomic_lock_release(std::int32_t &spinlock) noexcept;
    RAINY_TOOLKIT_API void atomic_lock_acquire(void **spinlock) noexcept;
    RAINY_TOOLKIT_API void atomic_lock_release(void **spinlock) noexcept;
#if RAINY_USING_64_BIT_PLATFORM
    RAINY_TOOLKIT_API bool atomic_wait_compare_16_bytes(const void *storage, void *comparand, std::size_t, void *) noexcept;
#endif
}

namespace rainy::foundation::pal::atomicinfra::implements {
    template <class Ty>
    struct storage_for {
        storage_for() = default;
        storage_for(const storage_for &) = delete;
        storage_for &operator=(const storage_for &) = delete;

#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
        explicit storage_for(form_mask_t) noexcept {
            std::memset(storage, 0xff, sizeof(Ty));
            core::pal::zero_non_value_bits(ptr());
        }
#endif

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
    bool atomic_wait_compare_nonlock_free(const void *storage, void *_Comparand, std::size_t _Size, void *spinlock_raw) noexcept {
        spinlock_t &spinlock = *static_cast<spinlock_t *>(spinlock_raw);
        atomic_lock_acquire(spinlock);
        const auto _Cmpresult = std::memcmp(storage, _Comparand, _Size);
        atomic_lock_release(spinlock);
        return _Cmpresult == 0;
    }

    template <typename Ty>
    struct atomic_padded {
        alignas(sizeof(Ty)) mutable Ty value;
    };

    template <class Ty>
    struct atomic_storage_types {
        using storage = atomic_padded<Ty>;
        using spinlock = std::int32_t;
    };

    template <typename Ty>
    struct atomic_storage_types<Ty &> {
        using storage = Ty &;
        using spinlock = void *;
    };
}

namespace rainy::foundation::pal::atomicinfra::implements {
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
            value_type &expected, const value_type desired,
            const memory_order order = memory_order_seq_cst) noexcept { // CAS with sequential consistency, plain
            check_memory_order(order);
            const auto storage_ptr = utility::addressof(storage_);
            const auto expected_ptr = utility::addressof(expected);
            bool result{};
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            core::builtin::zero_non_value_bits(expected_ptr);
#endif
            atomic_lock_gurad lock{spinlock};
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<value_type>) {
                storage_for<value_type> local;
                const auto local_ptr = local.ptr();
                std::memcpy(local_ptr, storage_ptr, sizeof(value_type));
                core::builtin::zero_non_value_bits_ptrs(local_ptr);
                result = std::memcmp(local_ptr, expected_ptr, sizeof(value_type)) == 0;
            } else {
                result = std::memcmp(storage_ptr, expected_ptr, sizeof(value_type)) == 0;
            }
#else
            result = std::memcmp(storage_ptr, expected_ptr, sizeof(value_type)) == 0;
#endif
            if (result) {
                std::memcpy(storage_ptr, utility::addressof(desired), sizeof(value_type));
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
                            core::builtin::zero_non_value_bits_ptr(local_ptr);
                            core::builtin::zero_non_value_bits_ptr(expected_ptr);
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
                                     &atomic_wait_compare_nonlock_free<decltype(spinlock)>, atomic_wait_no_timeout);
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

    template <class Ty>
    struct atomic_storage<Ty, 1> { // lock-free using 1-byte intrinsics
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty>;

        atomic_storage() = default;

        constexpr atomic_storage(const Ty &value) noexcept : storage_{value} {
        }

        void store(const value_type value) noexcept {
            const auto mem = atomic_address_as<std::int8_t>(storage_);
            const char as_bytes = atomic_reinterpret_as<std::int8_t>(value);
            core::pal::iso_volatile_store8(mem, as_bytes);
        }

        void store(const value_type value, const memory_order order) noexcept {
            const auto mem = atomic_address_as<std::int8_t>(storage_);
            const char as_bytes = atomic_reinterpret_as<std::int8_t>(value);
            check_store_memory_order(order);
            if (order == memory_order_relaxed) {
                core::pal::iso_volatile_store8(mem, as_bytes);
            } else if (order == memory_order_release) {
                core::pal::iso_volatile_store8(mem, as_bytes);
            } else {
                store(value);
            }
        }

        RAINY_NODISCARD value_type load(const memory_order order = memory_order_seq_cst) const noexcept {
            const auto mem = atomic_address_as<char>(storage_);
            char as_bytes;
            as_bytes = core::pal::iso_volatile_load8(reinterpret_cast<volatile const std::int8_t*>(mem));
            check_load_memory_order(order);
            if (order != memory_order_relaxed) {
                core::pal::read_write_barrier();
            }
            return reinterpret_cast<value_type &>(as_bytes);
        }

        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            // exchange with given memory order
            check_memory_order(order);
            char as_bytes = atomic_reinterpret_as<std::int8_t>(value);
            core::pal::interlocked_exchange8(atomic_address_as<std::int8_t>(storage_), as_bytes);
            return reinterpret_cast<value_type &>(as_bytes);
        }

        bool compare_exchange_strong(value_type &expected, const value_type desired,
                                     const memory_order order = memory_order_seq_cst) noexcept { // CAS with given memory order
            check_memory_order(order);
            char expected_bytes = atomic_reinterpret_as<char>(expected); // read before atomic operation
            char prev_bytes;
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<value_type>) {
                storage_for<value_type> mask{form_mask};
                const char mask_val = atomic_reinterpret_as<char>(mask.reference());
                for (;;) {
                    prev_bytes = core::pal::interlocked_compare_exchange8(atomic_address_as<char>(storage_),
                                                                          atomic_reinterpret_as<char>(desired), expected_bytes);
                    if (prev_bytes == expected_bytes) {
                        return true;
                    }

                    if ((prev_bytes ^ expected_bytes) & mask_val) {
                        reinterpret_cast<char &>(expected) = prev_bytes;
                        return false;
                    }
                    expected_bytes = (expected_bytes & mask_val) | (prev_bytes & ~mask_val);
                }
            }
#endif
            prev_bytes = core::pal::interlocked_compare_exchange8(atomic_address_as<char>(storage_),
                                                                  atomic_reinterpret_as<char>(desired), expected_bytes);
            if (prev_bytes == expected_bytes) {
                return true;
            }

            reinterpret_cast<char &>(expected) = prev_bytes;
            return false;
        }

        void wait(const value_type expected, const memory_order order = memory_order_seq_cst) const noexcept {
            atomic_wait_direct(this, atomic_reinterpret_as<char>(expected), order);
        }

        void notify_one() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(addressof(storage_)));
            atomic_notify_one_direct(storage_ptr);
        }

        void notify_all() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(addressof(storage_)));
            atomic_notify_all_direct(storage_ptr);
        }

        typename atomic_storage_types<Ty>::storage storage_;
    };

    template <class Ty>
    struct atomic_storage<Ty, 2> { // lock-free using 2-byte intrinsics
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty>;

        atomic_storage() = default;

        constexpr atomic_storage(const Ty &value) noexcept : storage_{value} {
        }

        void store(const value_type value) noexcept {
            const auto mem = atomic_address_as<short>(storage_);
            const short as_bytes = atomic_reinterpret_as<short>(value);
            core::pal::iso_volatile_store16(mem, as_bytes);
        }

        void store(const value_type value, const memory_order order) noexcept {
            check_memory_order(order);
            const auto mem = atomic_address_as<short>(storage_);
            const short as_bytes = atomic_reinterpret_as<short>(value);
            if (order == memory_order_relaxed) {
                core::pal::iso_volatile_store16(mem, as_bytes);
            } else if (order == memory_order_release) {
                core::pal::read_write_barrier();
                core::pal::iso_volatile_store16(16, mem, as_bytes);
            } else {
                store(value);
            }
        }

        RAINY_NODISCARD value_type load(const memory_order order = memory_order_seq_cst) const noexcept {
            const auto mem = atomic_address_as<short>(storage_);
            short as_bytes = core::pal::iso_volatile_load16(mem);
            check_load_memory_order(order);
            if (order != memory_order_relaxed) {
                core::pal::read_write_barrier();
            }
            return reinterpret_cast<value_type &>(as_bytes);
        }

        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            short as_bytes =
                core::pal::interlocked_exchange16(atomic_address_as<short>(storage_), atomic_reinterpret_as<short>(value));
            check_memory_order(order);
            return reinterpret_cast<value_type &>(as_bytes);
        }

        bool compare_exchange_strong(value_type &expected, const value_type desired,
                                     const memory_order order = memory_order_seq_cst) noexcept { // CAS with given memory order
            short expected_bytes = atomic_reinterpret_as<short>(expected); // read before atomic operation
            short prev_bytes;
            check_memory_order(order);
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<Ty>) {
                storage_for<value_type> mask{form_mask};
                const short mask_val = atomic_reinterpret_as<short>(mask.reference());
                for (;;) {
                    check_memory_order(order);
                    prev_bytes = core::pal::interlocked_compare_exchange16(atomic_address_as<short>(storage_),
                                                                           atomic_reinterpret_as<short>(desired), expected_bytes);
                    if (prev_bytes == expected_bytes) {
                        return true;
                    }
                    if ((prev_bytes ^ expected_bytes) & mask_val) {
                        std::memcpy(addressof(expected), &prev_bytes, sizeof(value_type));
                        return false;
                    }
                    expected_bytes = (expected_bytes & mask_val) | (prev_bytes & ~mask_val);
                }
            }
#endif
            prev_bytes = core::pal::interlocked_compare_exchange16(atomic_address_as<short>(storage_),
                                                                   atomic_reinterpret_as<short>(desired), expected_bytes);
            if (prev_bytes == expected_bytes) {
                return true;
            }
            std::memcpy(utility::addressof(expected), &prev_bytes, sizeof(Ty));
            return false;
        }

        void wait(const value_type expected, const memory_order order = memory_order_seq_cst) const noexcept {
            wait_direct(this, atomic_reinterpret_as<short>(expected), order);
        }

        void notify_one() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            atomic_notify_one_direct(storage_ptr);
        }

        void notify_all() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            atomic_notify_all_direct(storage_ptr);
        }

        typename atomic_storage_types<Ty>::storage storage_;
    };

    template <class Ty>
    struct atomic_storage<Ty, 4> {
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty>;

        atomic_storage() = default;

        constexpr atomic_storage(const Ty &value) noexcept : storage_{value} {
        }

        void store(const value_type value) noexcept {
            const auto mem = atomic_address_as<int>(storage_);
            const int as_bytes = atomic_reinterpret_as<int>(value);
            core::pal::iso_volatile_store32(mem, as_bytes);
        }

        void store(const value_type value, const memory_order order) noexcept {
            check_memory_order(order);
            const auto mem = atomic_address_as<int>(storage_);
            const int as_bytes = atomic_reinterpret_as<int>(value);
            if (order == memory_order_relaxed) {
                core::pal::iso_volatile_store32(mem, as_bytes);
            } else if (order == memory_order_release) {
                core::pal::read_write_barrier();
                core::pal::iso_volatile_store32(32, mem, as_bytes);
            } else {
                store(value);
            }
        }

        RAINY_NODISCARD value_type
        load(const memory_order order = memory_order_seq_cst) const noexcept { // load with given memory order
            const auto mem = atomic_address_as<int>(storage_);
            int as_bytes;
            as_bytes = core::pal::iso_volatile_load32(mem);
            check_load_memory_order(order);
            if (order != memory_order_relaxed) {
                core::pal::read_write_barrier();
            }
            return reinterpret_cast<value_type &>(as_bytes);
        }

        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            std::int32_t as_bytes = core::pal::interlocked_exchange32(atomic_address_as<std::int32_t>(storage_), atomic_reinterpret_as<std::int32_t>(value));
            return reinterpret_cast<value_type &>(as_bytes);
        }

        bool compare_exchange_strong(value_type &expected, const value_type desired,
                                     const memory_order order = memory_order_seq_cst) noexcept { // CAS with given memory order
            std::int32_t expected_bytes = atomic_reinterpret_as<std::int32_t>(expected); // read before atomic operation
            std::int32_t prev_bytes;
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<value_type>) {
                storage_for<value_type> mask{form_mask};
                const std::int32_t mask_val = atomic_reinterpret_as<std::int32_t>(mask);
                for (;;) {
                    check_memory_order(order);
                    prev_bytes = core::pal::interlocked_compare_exchange32(atomic_address_as<std::int32_t>(storage_),
                                                                           atomic_reinterpret_as<std::int32_t>(desired), expected_bytes);
                    if (prev_bytes == expected_bytes) {
                        return true;
                    }
                    if ((prev_bytes ^ expected_bytes) & mask_val) {
                        std::memcpy(utility::addressof(expected), &prev_bytes, sizeof(value_type));
                        return false;
                    }
                    expected_bytes = (expected_bytes & mask_val) | (prev_bytes & ~mask_val);
                }
            }
#endif
            check_memory_order(order);
            prev_bytes = core::pal::interlocked_compare_exchange32(atomic_address_as<std::int32_t>(storage_),
                                                                   atomic_reinterpret_as<std::int32_t>(desired), expected_bytes);
            if (prev_bytes == expected_bytes) {
                return true;
            }
            std::memcpy(utility::addressof(expected), &prev_bytes, sizeof(value_type));
            return false;
        }

        void wait(const value_type expected, const memory_order order = memory_order_seq_cst) const noexcept {
            wait_direct(this, atomic_reinterpret_as<std::int32_t>(expected), order);
        }

        void notify_one() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            atomic_notify_one_direct(storage_ptr);
        }

        void notify_all() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            atomic_notify_all_direct(storage_ptr);
        }

        typename atomic_storage_types<Ty>::storage storage_;
    };

    template <class Ty>
    struct atomic_storage<Ty, 8> {
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty>;

        atomic_storage() = default;

        constexpr atomic_storage(const Ty &value) noexcept : storage_{value} {
        }

        void store(const value_type value) noexcept { // store with sequential consistency
            const auto mem = atomic_address_as<long long>(storage_);
            const long long as_bytes = atomic_reinterpret_as<long long>(value);
            core::pal::iso_volatile_store64(mem, as_bytes);
        }

        void store(const value_type value, const memory_order order) noexcept { // store with given memory order
            const auto mem = atomic_address_as<long long>(storage_);
            const long long as_bytes = atomic_reinterpret_as<long long>(value);
            check_store_memory_order(order);
            if (order == memory_order_relaxed) {
                core::pal::iso_volatile_store64(mem, as_bytes);
            } else if (order == memory_order_release) {
                core::pal::read_write_barrier();
                core::pal::iso_volatile_store64(mem, as_bytes);
            } else {
                store(value);
            }
        }

        RAINY_NODISCARD value_type
        load(const memory_order order = memory_order_seq_cst) const noexcept { // load with given memory order
            const auto mem = atomic_address_as<long long>(storage_);
            check_memory_order(order);
            long long as_bytes = core::pal::iso_volatile_store64(mem);
            return reinterpret_cast<value_type &>(as_bytes);
        }

#if RAINY_USING_32_BIT_PLATFORM
        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            value_type temp{load()};
            while (!compare_exchange_strong(temp, value, order)) {
            }
            return temp;
        }
#else
        value_type exchange(const value_type value, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            long long as_bytes =
                core::pal::interlocked_exchange64(atomic_address_as<long long>(storage_), atomic_reinterpret_as<long long>(value));
            return reinterpret_cast<value_type &>(as_bytes);
        }
#endif

        bool compare_exchange_strong(value_type &expected, const value_type desired,
                                     const memory_order order = memory_order_seq_cst) noexcept { // CAS with given memory order
            long long expected_bytes = atomic_reinterpret_as<long long>(expected); // read before atomic operation
            long long prev_bytes;
            check_memory_order(order);
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<value_type>) {
                storage_for<value_type> mask{form_mask};
                const long long mask_val = atomic_reinterpret_as<long long>(mask);
                for (;;) {
                    check_memory_order(order);
                    prev_bytes = core::pal::interlocked_compare_exchange64(atomic_address_as<long long>(storage_),
                                                                           atomic_reinterpret_as<long long>(desired), expected_bytes);
                    if (prev_bytes == expected_bytes) {
                        return true;
                    }
                    if ((prev_bytes ^ expected_bytes) & mask_val) {
                        memcpy(addressof(expected), &prev_bytes, sizeof(value_type));
                        return false;
                    }
                    expected_bytes = (expected_bytes & mask_val) | (prev_bytes & ~mask_val);
                }
            }
#endif
            prev_bytes = core::pal::interlocked_compare_exchange64(atomic_address_as<long long>(storage_),
                                                                   atomic_reinterpret_as<long long>(desired), expected_bytes);
            if (prev_bytes == expected_bytes) {
                return true;
            }
            std::memcpy(addressof(expected), &prev_bytes, sizeof(value_type));
            return false;
        }

        void wait(const value_type expected, const memory_order order = memory_order_seq_cst) const noexcept {
            wait_direct(this, atomic_reinterpret_as<long long>(expected), order);
        }

        void notify_one() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(addressof(storage_)));
            atomic_notify_one_direct(storage_ptr);
        }

        void notify_all() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(addressof(storage_)));
            atomic_notify_all_direct(storage_ptr);
        }

        typename atomic_storage_types<Ty>::storage storage_;
    };

#if RAINY_USING_64_BIT_PLATFORM
    template <typename Ty>
    struct atomic_storage<Ty &, 16> {
        using value_type = type_traits::cv_modify::remove_cvref_t<Ty &>;

        atomic_storage() = default;

        constexpr atomic_storage(Ty &value) noexcept : storage_{value} {
        }

        void store(const value_type value) noexcept { // store with sequential consistency
            (void) exchange(value);
        }

        void store(const value_type value, const memory_order order) noexcept { // store with given memory order
            check_store_memory_order(order);
            (void) exchange(value, order);
        }

        RAINY_NODISCARD value_type load() const noexcept { // load with sequential consistency
            long long *const storage_ptr = const_cast<long long *>(atomic_address_as<const long long>(storage_));
            int128_t result{}; // atomic CAS 0 with 0
            core::pal::interlocked_compare_exchange128(storage_ptr, 0, 0, &result.low);
            return reinterpret_cast<value_type &>(result);
        }

        RAINY_NODISCARD value_type load(const memory_order order) const noexcept { // load with given memory order
            check_load_memory_order(order);
            return load();
        }

        value_type exchange(const value_type value) noexcept { // exchange with sequential consistency
            value_type result{value};
            while (!compare_exchange_strong(result, value)) { // keep trying
            }
            return result;
        }

        value_type exchange(const value_type value, const memory_order order) noexcept { // exchange with given memory order
            value_type result{value};
            while (!compare_exchange_strong(result, value, order)) { // keep trying
            }
            return result;
        }

        bool compare_exchange_strong(value_type &_Expected, const value_type _Desired,
                                     const memory_order order = memory_order_seq_cst) noexcept { // CAS with given memory order
            int128_t desired_bytes{};
            std::memcpy(&desired_bytes, utility::addressof(_Desired), sizeof(value_type));
            int128_t expected_temp{};
            std::memcpy(&expected_temp, utility::addressof(_Expected), sizeof(value_type));
            unsigned char result;
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
            if constexpr (might_have_non_value_bits<value_type>) {
                int128_t expected_originally{};
                std::memcpy(&expected_originally, utility::addressof(_Expected), sizeof(value_type));
                storage_for<value_type> mask{_Form_mask};
                int128_t mask_val{};
                std::memcpy(&mask_val, mask._Ptr(), sizeof(value_type));
                for (;;) {
                    (void) order;
                    result = core::pal::interlocked_compare_exchange128(&reinterpret_cast<long long &>(storage_), desired_bytes.high,
                                                                        desired_bytes.low, &expected_temp.low);
                    if (result) {
                        return true;
                    }
                    if (((expected_temp.low ^ expected_originally.low) & mask_val.low) != 0 ||
                        ((expected_temp.high ^ expected_originally.high) & mask_val.high) != 0) {
                        std::memcpy(utility::addressof(_Expected), &expected_temp, sizeof(value_type));
                        return false;
                    }
                    expected_temp.low = (expected_originally.low & mask_val.low) | (expected_temp.low & ~mask_val.low);
                    expected_temp.high = (expected_originally.high & mask_val.high) | (expected_temp.high & ~mask_val.high);
                }
            }
#endif
            (void) order;
            result = core::pal::interlocked_compare_exchange128(&reinterpret_cast<volatile long long &>(storage_), desired_bytes.high,
                                                                desired_bytes.low, &expected_temp.low);
            if (result == 0) {
                std::memcpy(utility::addressof(_Expected), &expected_temp, sizeof(value_type));
            }
            return result != 0;
        }

        void wait(value_type _Expected, memory_order order = memory_order_seq_cst) const noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            const auto expected_ptr = utility::addressof(_Expected);
            int128_t expected_bytes = reinterpret_cast<const int128_t &>(_Expected);

            for (;;) {
                const value_type observed = load(order);
                int128_t observed_bytes = reinterpret_cast<const int128_t &>(observed);
                if (observed_bytes.low != expected_bytes.low || observed_bytes.high != expected_bytes.high) {
#if RAINY_CMPXCHG_MASK_OUT_PADDING_BITS
                    if constexpr (might_have_non_value_bits<value_type>) {
                        storage_for<value_type> mask{form_mask};
                        const int128_t mask_val = reinterpret_cast<const int128_t &>(mask.reference());
                        if (((expected_bytes.low ^ observed_bytes.low) & mask_val.low) == 0 &&
                            ((expected_bytes.high ^ observed_bytes.high) & mask_val.high) == 0) {
                            expected_bytes = observed_bytes;
                            continue;
                        }
                    }
#endif

                    return;
                }
                atomic_wait_indirect(storage_ptr, expected_ptr, sizeof(value_type), nullptr, &atomic_wait_compare_16_bytes,
                                     atomic_wait_no_timeout);
            }
        }

        void notify_one() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            atomic_notify_one_indirect(storage_ptr);
        }

        void notify_all() noexcept {
            const auto storage_ptr = const_cast<const void *>(static_cast<const volatile void *>(utility::addressof(storage_)));
            atomic_notify_all_indirect(storage_ptr);
        }

        struct int128_t {
            alignas(16) long long low;
            long long high;
        };

        typename atomic_storage_types<Ty &>::storage storage_;
    };
#endif
}

namespace rainy::foundation::pal::atomicinfra::implements {
    template <typename Ty, std::size_t = sizeof(Ty)>
    struct atomic_integral;

    template <typename Ty>
    struct atomic_integral<Ty, 1> : atomic_storage<Ty> {
        using base = atomic_storage<Ty>;
        using typename base::value_type;

        using base::base;

        value_type fetch_add(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            char result = core::pal::interlocked_exchange8(atomic_address_as<char>(this->storage_), static_cast<char>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_and(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            char result = core::pal::interlocked_and8(atomic_address_as<char>(this->storage_), static_cast<char>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_or(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            char result = core::pal::interlocked_or8(atomic_address_as<char>(this->storage_), static_cast<char>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_xor(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            char result = core::pal::interlocked_xor8(atomic_address_as<char>(this->storage_), static_cast<char>(operand));
            return static_cast<value_type>(result);
        }

        value_type operator++(int) noexcept {
            return static_cast<value_type>(core::pal::interlocked_exchange_add8(atomic_address_as<char>(this->storage_), 1));
        }

        value_type operator++() noexcept {
            unsigned char before =
                static_cast<unsigned char>(core::pal::interlocked_exchange_add8(atomic_address_as<char>(this->storage_), 1));
            ++before;
            return static_cast<value_type>(before);
        }

        value_type operator--(int) noexcept {
            return static_cast<value_type>(core::pal::interlocked_exchange_add8(atomic_address_as<char>(this->storage_), -1));
        }

        value_type operator--() noexcept {
            unsigned char before =
                static_cast<unsigned char>(core::pal::interlocked_exchange_add8(atomic_address_as<char>(this->storage_), -1));
            --before;
            return static_cast<value_type>(before);
        }
    };

    template <typename Ty>
    struct atomic_integral<Ty, 2> : atomic_storage<Ty> {
        using base = atomic_storage<Ty>;
        using typename base::value_type;

        using base::base;

        value_type fetch_add(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            short result =
                core::pal::interlocked_exchange_add16(atomic_address_as<short>(this->storage_), static_cast<short>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_and(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            short result = core::pal::interlocked_and16(atomic_address_as<short>(this->storage_), static_cast<short>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_or(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            short result = core::pal::interlocked_or16(atomic_address_as<short>(this->storage_), static_cast<short>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_xor(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            short result = core::pal::interlocked_xor16(atomic_address_as<short>(this->storage_), static_cast<short>(operand));
            return static_cast<value_type>(result);
        }

        value_type operator++(int) noexcept {
            unsigned short after =
                static_cast<unsigned short>(core::pal::interlocked_increment16(atomic_address_as<short>(this->storage_)));
            --after;
            return static_cast<value_type>(after);
        }

        value_type operator++() noexcept {
            return static_cast<value_type>(core::pal::interlocked_increment16(atomic_address_as<short>(this->storage_)));
        }

        value_type operator--(int) noexcept {
            unsigned short after =
                static_cast<unsigned short>(core::pal::interlocked_decrement16(atomic_address_as<short>(this->storage_)));
            ++after;
            return static_cast<value_type>(after);
        }

        value_type operator--() noexcept {
            return static_cast<value_type>(core::pal::interlocked_decrement16(atomic_address_as<short>(this->storage_)));
        }
    };

    template <typename Ty>
    struct atomic_integral<Ty, 4> : atomic_storage<Ty> { // atomic integral operations using 4-byte intrinsics
        using base = atomic_storage<Ty>;
        using typename base::value_type;

        using base::base;

        value_type fetch_add(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            std::int32_t result =
                core::pal::interlocked_exchange_add32(atomic_address_as<std::int32_t>(this->storage_), static_cast<std::int32_t>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_and(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            std::int32_t result = core::pal::interlocked_and32(atomic_address_as<std::int32_t>(this->storage_), static_cast<std::int32_t>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_or(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            std::int32_t result = core::pal::interlocked_or32(atomic_address_as<std::int32_t>(this->storage_), static_cast<std::int32_t>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_xor(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            std::int32_t result = core::pal::interlocked_xor32(atomic_address_as<std::int32_t>(this->storage_), static_cast<std::int32_t>(operand));
            return static_cast<value_type>(result);
        }

        value_type operator++(int) noexcept {
            std::uint32_t after =
                static_cast<std::uint32_t>(core::pal::interlocked_increment32(atomic_address_as<std::int32_t>(this->storage_)));
            --after;
            return static_cast<value_type>(after);
        }

        value_type operator++() noexcept {
            return static_cast<value_type>(core::pal::interlocked_increment32(atomic_address_as<std::int32_t>(this->storage_)));
        }

        value_type operator--(int) noexcept {
            std::uint32_t after =
                static_cast<std::uint32_t>(core::pal::interlocked_decrement32(atomic_address_as<std::int32_t>(this->storage_)));
            ++after;
            return static_cast<value_type>(after);
        }

        value_type operator--() noexcept {
            return static_cast<value_type>(core::pal::interlocked_decrement32(atomic_address_as<std::int32_t>(this->storage_)));
        }
    };

    template <typename Ty>
    struct atomic_integral<Ty, 8> : atomic_storage<Ty> {
        using base = atomic_storage<Ty>;
        using typename base::value_type;

        using base::base;

#if RAINY_USING_32_BIT_PLATFORM
        value_type fetch_add(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            value_type temp{this->load()};
            while (!this->compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        value_type fetch_and(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            value_type temp{this->load()};
            while (!this->compare_exchange_strong(temp, temp & operand, order)) {
            }
            return temp;
        }

        value_type fetch_or(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            value_type temp{this->load()};
            while (!this->compare_exchange_strong(temp, temp | operand, order)) {
            }
            return temp;
        }

        value_type fetch_xor(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            value_type temp{this->load()};
            while (!this->compare_exchange_strong(temp, temp ^ operand, order)) {
            }

            return temp;
        }

        value_type operator++(int) noexcept {
            return fetch_add(static_cast<value_type>(1));
        }

        value_type operator++() noexcept {
            return fetch_add(static_cast<value_type>(1)) + static_cast<value_type>(1);
        }

        value_type operator--(int) noexcept {
            return fetch_add(static_cast<value_type>(-1));
        }

        value_type operator--() noexcept {
            return fetch_add(static_cast<value_type>(-1)) - static_cast<value_type>(1);
        }
#else
        value_type fetch_add(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            long long result =
                core::pal::interlocked_exchange_add64(atomic_address_as<long long>(this->storage_), static_cast<long long>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_and(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            long long result =
                core::pal::interlocked_and64(atomic_address_as<long long>(this->storage_), static_cast<long long>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_or(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            long long result =
                core::pal::interlocked_or64(atomic_address_as<long long>(this->storage_), static_cast<long long>(operand));
            return static_cast<value_type>(result);
        }

        value_type fetch_xor(const value_type operand, const memory_order order = memory_order_seq_cst) noexcept {
            check_memory_order(order);
            long long result =
                core::pal::interlocked_xor64(atomic_address_as<long long>(this->storage_), static_cast<long long>(operand));
            return static_cast<value_type>(result);
        }

        value_type operator++(int) noexcept {
            unsigned long long after =
                static_cast<unsigned long long>(core::pal::interlocked_increment64(atomic_address_as<long long>(this->storage_)));
            --after;
            return static_cast<value_type>(after);
        }

        value_type operator++() noexcept {
            return static_cast<value_type>(core::pal::interlocked_increment64(atomic_address_as<long long>(this->storage_)));
        }

        value_type operator--(int) noexcept {
            unsigned long long after =
                static_cast<unsigned long long>(core::pal::interlocked_decrement64(atomic_address_as<long long>(this->storage_)));
            ++after;
            return static_cast<value_type>(after);
        }

        value_type operator--() noexcept {
            return static_cast<value_type>(core::pal::interlocked_decrement64(atomic_address_as<long long>(this->storage_)));
        }
#endif
    };
}

namespace rainy::foundation::pal::atomicinfra::implements {
    template <typename Ty>
    struct atomic_integral_facade : atomic_integral<Ty> {
        static_assert(!type_traits::type_properties::is_const_v<Ty>);
        static_assert(!type_traits::type_properties::is_volatile_v<Ty>);

        using base = atomic_integral<Ty>;
        using difference_type = Ty;

        using base::base;

        using base::fetch_add;

        Ty fetch_add(const Ty operand) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_add(operand);
        }

        Ty fetch_add(const Ty operand, const memory_order order) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_add(operand, order);
        }

        RAINY_NODISCARD static Ty negate(const Ty value) noexcept {
            return static_cast<Ty>(0U - static_cast<std::make_unsigned_t<Ty>>(value));
        }

        Ty fetch_sub(const Ty operand) noexcept {
            return fetch_add(negate(operand));
        }

        Ty fetch_sub(const Ty operand) volatile noexcept {
            return fetch_add(negate(operand));
        }

        Ty fetch_sub(const Ty operand, const memory_order order) noexcept {
            return fetch_add(negate(operand), order);
        }

        Ty fetch_sub(const Ty operand, const memory_order order) volatile noexcept {
            return fetch_add(negate(operand), order);
        }

        using base::fetch_and;
        Ty fetch_and(const Ty operand) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_and(operand);
        }

        Ty fetch_and(const Ty operand, const memory_order order) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_and(operand, order);
        }

        using base::fetch_or;
        
        Ty fetch_or(const Ty operand) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_or(operand);
        }

        Ty fetch_or(const Ty operand, const memory_order order) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_or(operand, order);
        }

        using base::fetch_xor;

        Ty fetch_xor(const Ty operand) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_xor(operand);
        }

        Ty fetch_xor(const Ty operand, const memory_order order) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_xor(operand, order);
        }

        using base::operator++;

        Ty operator++(int) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator++(0);
        }

        Ty operator++() volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator++();
        }

        using base::operator--;
        
        Ty operator--(int) volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator--(0);
        }

        Ty operator--() volatile noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator--();
        }

        Ty operator+=(const Ty operand) noexcept {
            return static_cast<Ty>(this->base::fetch_add(operand) + operand);
        }

        Ty operator+=(const Ty operand) volatile noexcept {
            return static_cast<Ty>(const_cast<atomic_integral_facade *>(this)->base::fetch_add(operand) + operand);
        }

        Ty operator-=(const Ty operand) noexcept {
            return static_cast<Ty>(fetch_sub(operand) - operand);
        }

        Ty operator-=(const Ty operand) volatile noexcept {
            return static_cast<Ty>(const_cast<atomic_integral_facade *>(this)->fetch_sub(operand) - operand);
        }

        Ty operator&=(const Ty operand) noexcept {
            return static_cast<Ty>(this->base::fetch_and(operand) & operand);
        }

        Ty operator&=(const Ty operand) volatile noexcept {
            return static_cast<Ty>(const_cast<atomic_integral_facade *>(this)->base::fetch_and(operand) & operand);
        }

        Ty operator|=(const Ty operand) noexcept {
            return static_cast<Ty>(this->base::fetch_or(operand) | operand);
        }

        Ty operator|=(const Ty operand) volatile noexcept {
            return static_cast<Ty>(const_cast<atomic_integral_facade *>(this)->base::fetch_or(operand) | operand);
        }

        Ty operator^=(const Ty operand) noexcept {
            return static_cast<Ty>(this->base::fetch_xor(operand) ^ operand);
        }

        Ty operator^=(const Ty operand) volatile noexcept {
            return static_cast<Ty>(const_cast<atomic_integral_facade *>(this)->base::fetch_xor(operand) ^ operand);
        }
    };

    template <typename Ty>
    struct atomic_integral_facade<Ty &> : atomic_integral<Ty &> {
        using base = atomic_integral<Ty &>;
        using difference_type = type_traits::cv_modify::remove_cv_t<Ty>;
        using typename base::value_type;

        using base::base;

        RAINY_NODISCARD static value_type negate(const value_type value) noexcept {
            return static_cast<value_type>(0U - static_cast<type_traits::helper::make_unsigned_t<value_type>>(value));
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_add(const value_type operand) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_add(operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_add(const value_type operand, const memory_order order) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_add(operand, order);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_sub(const value_type operand) const noexcept {
            return fetch_add(negate(operand));
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_sub(const value_type operand, const memory_order order) const noexcept {
            return fetch_add(negate(operand), order);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator++(int) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator++(0);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator++() const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator++();
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator--(int) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator--(0);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator--() const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::operator--();
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator+=(const value_type operand) const noexcept {
            return static_cast<value_type>(fetch_add(operand) + operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator-=(const value_type operand) const noexcept {
            return static_cast<value_type>(fetch_sub(operand) - operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_and(const value_type operand) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_and(operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_and(const value_type operand, const memory_order order) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_and(operand, order);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_or(const value_type operand) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_or(operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_or(const value_type operand, const memory_order order) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_or(operand, order);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_xor(const value_type operand) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_xor(operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_xor(const value_type operand, const memory_order order) const noexcept {
            return const_cast<atomic_integral_facade *>(this)->base::fetch_xor(operand, order);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator&=(const value_type operand) const noexcept {
            return static_cast<value_type>(fetch_and(operand) & operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator|=(const value_type operand) const noexcept {
            return static_cast<value_type>(fetch_or(operand) | operand);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator^=(const value_type operand) const noexcept {
            return static_cast<value_type>(fetch_xor(operand) ^ operand);
        }
    };

    template <typename Ty>
    struct atomic_floating : atomic_storage<Ty> {
        static_assert(!type_traits::type_properties::is_const_v<Ty>);
        static_assert(!type_traits::type_properties::is_volatile_v<Ty>);

        using base = atomic_storage<Ty>;
        using difference_type = Ty;

        using base::base;

        Ty fetch_add(const Ty operand, const memory_order order = memory_order_seq_cst) noexcept {
            Ty temp{this->load(memory_order_relaxed)};
            while (!this->compare_exchange_strong(temp, temp + operand, order)) { // keep trying
            }

            return temp;
        }

        Ty fetch_add(const Ty operand, const memory_order order = memory_order_seq_cst) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_add(operand, order);
        }

        Ty fetch_sub(const Ty operand, const memory_order order = memory_order_seq_cst) noexcept {
            Ty temp{this->load(memory_order_relaxed)};
            while (!this->compare_exchange_strong(temp, temp - operand, order)) { // keep trying
            }

            return temp;
        }

        Ty fetch_sub(const Ty operand, const memory_order order = memory_order_seq_cst) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand, order);
        }

        Ty operator+=(const Ty operand) noexcept {
            return fetch_add(operand) + operand;
        }

        Ty operator+=(const Ty operand) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_add(operand) + operand;
        }

        Ty operator-=(const Ty operand) noexcept {
            return fetch_sub(operand) - operand;
        }

        Ty operator-=(const Ty operand) volatile noexcept {
            return const_cast<atomic_floating *>(this)->fetch_sub(operand) - operand;
        }
    };

    template <typename Ty>
    struct atomic_floating<Ty &> : atomic_storage<Ty &> {
        using base = atomic_storage<Ty &>;
        using difference_type = type_traits::cv_modify::remove_cv_t<Ty>;
        using typename base::value_type;

        using base::base;

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_add(const value_type operand, const memory_order order = memory_order_seq_cst) const noexcept
        {
            value_type temp{this->load(memory_order_relaxed)};
            while (!const_cast<atomic_floating *>(this)->base::compare_exchange_strong(temp, temp + operand, order)) {
            }
            return temp;
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_sub(const value_type operand, const memory_order order = memory_order_seq_cst) const noexcept
        {
            value_type temp{this->load(memory_order_relaxed)};
            while (!const_cast<atomic_floating *>(this)->base::compare_exchange_strong(temp, temp - operand, order)) {
            }
            return temp;
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator+=(const value_type operand) const noexcept
        {
            return fetch_add(operand) + operand;
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator-=(const value_type operand) const noexcept
        {
            return fetch_sub(operand) - operand;
        }
    };

    template <typename Ty>
    struct atomic_pointer : atomic_storage<Ty> {
        using base = atomic_storage<Ty>;
        using difference_type = ptrdiff_t;

        using base::base;

        Ty fetch_add(const ptrdiff_t diff, const memory_order order = memory_order_seq_cst) noexcept {
            const ptrdiff_t shift_bytes = static_cast<ptrdiff_t>(static_cast<std::size_t>(diff) * sizeof(type_traits::pointer_modify::remove_pointer_t<Ty>));
            ptrdiff_t result;
            check_memory_order(order);
#if RAINY_USING_64_BIT_PLATFORM
            result = core::pal::interlocked_exchange_add64(atomic_address_as<long long>(this->storage_), shift_bytes);
#else
            result = core::pal::interlocked_exchange_add32(atomic_address_as<std::int32_t>(this->storage_), shift_bytes);
#endif
            return reinterpret_cast<Ty>(result);
        }

        Ty fetch_add(const ptrdiff_t diff) volatile noexcept {
            return const_cast<atomic_pointer *>(this)->fetch_add(diff);
        }

        Ty fetch_add(const ptrdiff_t diff, const memory_order order) volatile noexcept {
            return const_cast<atomic_pointer *>(this)->fetch_add(diff, order);
        }

        Ty fetch_sub(const ptrdiff_t diff) volatile noexcept {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff)));
        }

        Ty fetch_sub(const ptrdiff_t diff) noexcept {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff)));
        }

        Ty fetch_sub(const ptrdiff_t diff, const memory_order order) volatile noexcept {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff)), order);
        }

        Ty fetch_sub(const ptrdiff_t diff, const memory_order order) noexcept {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff)), order);
        }

        Ty operator++(int) volatile noexcept {
            return fetch_add(1);
        }

        Ty operator++(int) noexcept {
            return fetch_add(1);
        }

        Ty operator++() volatile noexcept {
            return fetch_add(1) + 1;
        }

        Ty operator++() noexcept {
            return fetch_add(1) + 1;
        }

        Ty operator--(int) volatile noexcept {
            return fetch_add(-1);
        }

        Ty operator--(int) noexcept {
            return fetch_add(-1);
        }

        Ty operator--() volatile noexcept {
            return fetch_add(-1) - 1;
        }

        Ty operator--() noexcept {
            return fetch_add(-1) - 1;
        }

        Ty operator+=(const ptrdiff_t diff) volatile noexcept {
            return fetch_add(diff) + diff;
        }

        Ty operator+=(const ptrdiff_t diff) noexcept {
            return fetch_add(diff) + diff;
        }

        Ty operator-=(const ptrdiff_t diff) volatile noexcept {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff))) - diff;
        }

        Ty operator-=(const ptrdiff_t diff) noexcept {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff))) - diff;
        }
    };

    template <typename Ty>
    struct atomic_pointer<Ty &> : atomic_storage<Ty &> {
        using base = atomic_storage<Ty &>;
        using difference_type = ptrdiff_t;
        using typename base::value_type;

        using base::base;

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_add(const ptrdiff_t diff, const memory_order order = memory_order_seq_cst) const noexcept {
            const ptrdiff_t shift_bytes =
                static_cast<ptrdiff_t>(static_cast<std::size_t>(diff) * sizeof(type_traits::pointer_modify::remove_pointer_t<Ty>));
            ptrdiff_t result{};
            check_memory_order(order);
#if RAINY_USING_64_BIT_PLATFORM
            result = core::pal::interlocked_compare_exchange64(atomic_address_as<long long>(this->storage_), shift_bytes);
#else
            result = core::pal::interlocked_compare_exchange32(atomic_address_as<std::int32_t>(this->storage_), shift_bytes);
#endif
            return reinterpret_cast<value_type>(result);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_sub(const ptrdiff_t diff) const noexcept  {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff)));
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type fetch_sub(const ptrdiff_t diff, const memory_order order) const noexcept  {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff)), order);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator++(int) const noexcept  {
            return fetch_add(1);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator++() const noexcept  {
            return fetch_add(1) + 1;
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator--(int) const noexcept  {
            return fetch_add(-1);
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator--() const noexcept  {
            return fetch_add(-1) - 1;
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator+=(const ptrdiff_t diff) const noexcept  {
            return fetch_add(diff) + diff;
        }

        template <type_traits::other_trans::enable_if_t<!type_traits::type_properties::is_const_v<Ty>, int> = 0>
        value_type operator-=(const ptrdiff_t diff) const noexcept  {
            return fetch_add(static_cast<ptrdiff_t>(0 - static_cast<std::size_t>(diff))) - diff;
        }
    };

    template <typename Ty>
    struct atomic_nonobject_pointer : atomic_storage<Ty> {
        using base = atomic_storage<Ty>;
        using difference_type = ptrdiff_t;

        using base::base;
    };

    template <typename Ty>
    struct atomic_nonobject_pointer<Ty &> : atomic_storage<Ty &> {
        using base = atomic_storage<Ty &>;

        using base::base;
    };
}

namespace rainy::foundation::pal::atomicinfra::implements {
    template <typename TVal, typename Ty = TVal>
    using select_atomic_base_t =
        typename type_traits::other_trans::select<type_traits::primary_types::is_floating_point_v<TVal>>::template apply<
            atomic_floating<Ty>,
            typename type_traits::other_trans::select<
                type_traits::primary_types::is_integral_v<TVal> &&
                !type_traits::type_relations::is_same_v<bool, type_traits::cv_modify::remove_cv_t<TVal>>>::
                template apply<atomic_integral_facade<Ty>,
                               typename type_traits::other_trans::select<type_traits::primary_types::is_pointer_v<TVal>>::
                                   template apply<typename type_traits::other_trans::select<type_traits::composite_types::is_object_v<
                                                      type_traits::pointer_modify::remove_pointer_t<TVal>>>::
                                                      template apply<atomic_pointer<Ty>, atomic_nonobject_pointer<Ty>>,
                                                  atomic_storage<Ty>>>>;
}

#endif
