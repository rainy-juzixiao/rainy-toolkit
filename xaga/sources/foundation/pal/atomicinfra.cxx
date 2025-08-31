#include <rainy/core/core.hpp>
#include <rainy/foundation/pal/atomicinfra.hpp>
#include <rainy/foundation/pal/implements/tgc_layer_threading.hpp>
#if RAINY_USING_WINDOWS
#include <windows.h>
#endif

#if RAINY_USING_LINUX
#include <linux/futex.h>
#endif

namespace rainy::foundation::pal::atomicinfra {
    void atomic_thread_fence(memory_order order) noexcept {
        using namespace core::pal;
        if (order == memory_order_relaxed) {
            return;
        }
        read_write_barrier();
        if (order == memory_order_seq_cst) {
            volatile long _guard;
            (void) interlocked_increment(&_guard);
            read_write_barrier();
        }
    }

    void atomic_signal_fence(const memory_order order) noexcept {
        if (order != memory_order_relaxed) {
            core::pal::read_write_barrier();
        }
    }
}
