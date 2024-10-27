#ifndef RAINY_ATOMIC_HPP
#define RAINY_ATOMIC_HPP
#include <rainy/core.hpp>

namespace rainy::foundation::containers {
    namespace internals {
        using atomic_counter_t = unsigned long;

        template <typename intergral, typename Ty>
        RAINY_NODISCARD volatile intergral *atomic_address_as(Ty &source) noexcept {
            // gets a pointer to the argument as an integral type (to pass to intrinsics)
            static_assert(type_traits::internals::_is_intergral_v<intergral>, "Tried to reinterpret memory as non-integral");
            return &reinterpret_cast<volatile intergral &>(source);
        }

        template <typename intergral, typename Ty>
        RAINY_NODISCARD const volatile intergral *atomic_address_as(const Ty &source) noexcept {
            // gets a pointer to the argument as an integral type (to pass to intrinsics)
            static_assert(type_traits::internals::_is_intergral_v<intergral>, "Tried to reinterpret memory as non-integral");
            return &reinterpret_cast<const volatile intergral &>(source);
        }

        template <typename spinlock_t>
        class atomic_lock_guard {
        public:
            explicit atomic_lock_guard(spinlock_t &spinlock) noexcept : spinlock(spinlock) {
            }

            ~atomic_lock_guard() {
            }

            atomic_lock_guard(const atomic_lock_guard &) = delete;
            atomic_lock_guard &operator=(const atomic_lock_guard &) = delete;

        private:
            spinlock_t &spinlock;
        };

        inline void check_store_memory_order(const memory_order order) noexcept {
            switch (order) {
                case memory_order_relaxed:
                case memory_order_release:
                case memory_order_seq_cst:
                    // nothing to do
                    break;
                case memory_order_consume:
                case memory_order_acquire:
                case memory_order_acq_rel:
                default:
                    information::system_call::debug_break(
                        /* INVALID_MEMORY_ORDER */
                    );
                    break;
            }
        }

        inline void check_load_memory_order(const memory_order order) noexcept {
            switch (order) {
                case memory_order_relaxed:
                case memory_order_consume:
                case memory_order_acquire:
                case memory_order_seq_cst:
                    // nothing to do
                    break;
                case memory_order_release:
                case memory_order_acq_rel:
                default:
                    information::system_call::debug_break(
                        /* INVALID_MEMORY_ORDER */
                    );
                    break;
            }
        }
    }


}



#endif //RAINY_ATOMIC_HPP
