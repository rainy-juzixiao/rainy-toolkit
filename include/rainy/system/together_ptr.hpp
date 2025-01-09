#ifndef RAINY_TOGETHER_PTR
#define RAINY_TOGETHER_PTR
#include <rainy/base.hpp>

namespace rainy::foundation::system::memory {
    template <typename Ty>
    class together_ptr;

    template <typename Ty>
    class weak_ptr;
}


namespace rainy::foundation::system::memory::internals {

    struct atomic_ref_count_base {
        atomic_ref_count_base(const atomic_ref_count_base &) = delete;
        atomic_ref_count_base &operator=(const atomic_ref_count_base &) = delete;

        virtual ~atomic_ref_count_base() noexcept {
        }

        bool incref_if_not_zero() noexcept {
            rainy_ref volatile_uses = reinterpret_cast<volatile long &>(uses);
            long count = information::system_call::iso_volatile_load32(reinterpret_cast<volatile int *>(&volatile_uses));
            while (count != 0) {
            }
        }


        unsigned long uses = 1;
        unsigned long weaks = 1;
    };
}

#endif //RAINY_TOGETHER_PTR_HPP
