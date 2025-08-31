#include <rainy/foundation/pal/implements/tgc_atomicinfra.hpp>

namespace rainy::foundation::pal::atomicinfra::implements {

    void invalid_memory_order() {
#if RAINY_ENABLE_DEBUG
        core::pal::debug_break();
#else
        std::terminate();
#endif
    }

    void check_store_memory_order(const memory_order order) noexcept {
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
                invalid_memory_order();
                break;
        }
    }

    void check_load_memory_order(const memory_order order) noexcept {
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
                invalid_memory_order();
                break;
        }
    }

    void check_memory_order(memory_order order) noexcept {
        if (order > memory_order_seq_cst) {
            invalid_memory_order();
        }
    }
}
