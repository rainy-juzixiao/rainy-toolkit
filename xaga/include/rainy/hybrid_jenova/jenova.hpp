#ifndef RAINY_HYBRID_JENOVA_JENOVA_HPP
#define RAINY_HYBRID_JENOVA_JENOVA_HPP
#include <rainy/core/core.hpp>

namespace rainy::hybrid_jenova {
    template <typename Iter, typename NodeType>
    struct insert_node_return_type {
        using iterator = Iter;
        using node_type = NodeType;

        iterator position;
        bool inserted;
        node_type node;
    };

    static constexpr inline std::size_t npos = static_cast<std::size_t>(-1);
}

#endif
