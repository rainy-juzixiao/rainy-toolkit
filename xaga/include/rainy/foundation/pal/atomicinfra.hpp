#ifndef RAINY_FOUNDATION_PAL_ATOMIC_HPP
#define RAINY_FOUNDATION_PAL_ATOMIC_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/pal/implements/tgc_atomicinfra.hpp>

namespace rainy::foundation::pal::atomicinfra {
    void atomic_thread_fence(memory_order order) noexcept;
    void atomic_signal_fence(const memory_order order) noexcept;

    template <typename Ty>
    Ty kill_dependency(Ty arg) noexcept {
        return arg;
    }

    template <typename Ty>
    class atomic : public implements::select_atomic_base_t<Ty> {
    public:
        static_assert(type_traits::type_properties::is_trivially_copyable_v<Ty>, "atomic<T> requires T to be trivially copyable.");
        static_assert(type_traits::type_properties::is_copy_constructible_v<Ty>, "atomic<T> requires T to be copy constructible.");
        static_assert(type_traits::type_properties::is_move_constructible_v<Ty>, "atomic<T> requires T to be move constructible.");
        static_assert(type_traits::type_properties::is_copy_assignable_v<Ty>, "atomic<T> requires T to be copy assignable.");
        static_assert(type_traits::type_properties::is_move_assignable_v<Ty>, "atomic<T> requires T to be move assignable.");
        static_assert(!type_traits::type_properties::is_const_v<Ty>, "atomic<T> requires T to be non-const.");
        static_assert(!type_traits::type_properties::is_volatile_v<Ty>, "atomic<T> requires T to be non-volatile.");

        using base = implements::select_atomic_base_t<Ty>;
        using base::base;

        operator Ty() const volatile noexcept {
            return const_cast<const atomic *>(this)->load();
        }

        operator Ty() const noexcept {
            return this->load();
        }
    };

    using atomic_int = atomic<int>;
    using atomic_bool = atomic<bool>;
}

#endif