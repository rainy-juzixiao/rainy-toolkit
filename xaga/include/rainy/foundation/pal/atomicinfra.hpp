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
    class atomic : public implements::atomic_storage<Ty> {
    public:
        using base = implements::atomic_storage<Ty>;
        using base::base;
    };
}

#endif