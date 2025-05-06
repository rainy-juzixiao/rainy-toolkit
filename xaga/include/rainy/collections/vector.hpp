#ifndef RAINY_VECTOR_HPP
#define RAINY_VECTOR_HPP

#include <rainy/base.hpp>

namespace rainy::collections::implements {
    template <typename Ty>
    struct vector_storage {
        vector_storage() = default;

        Ty *begin;
        Ty *end;
        Ty *last;
    };
}

namespace rainy::collections {
    template <typename Ty,typename Alloc = std::allocator<Ty>>
    class vector {
    public:
    private:
    };

    
}
#endif