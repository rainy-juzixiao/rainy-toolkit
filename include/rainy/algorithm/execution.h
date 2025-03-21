#ifndef RAINY_ALGORITHM_PRE_H
#define RAINY_ALGORITHM_PRE_H

#include <rainy/core/core.hpp>
#include <rainy/containers/array.hpp>
#include <vector>
#include <future>

namespace rainy::algorithm::execution {
    enum policy {
        par,
        seq,
        max_par
    };
}

namespace rainy::algorithm::internals {
    RAINY_INLINE std::size_t get_paralells(const std::size_t count) {
        std::size_t paralells = 1;
        containers::array<std::pair<std::size_t, std::size_t>, 4> thresholds = {{{75, 100}, {100, 250}, {300, 648}, {800, 1500}}};
        containers::array<std::size_t, 4> increments = {1, 2, 3, 4};
        for (std::size_t i = 0; i < thresholds.size(); ++i) {
            if (count > thresholds[static_cast<ptrdiff_t>(i)].first && count <= thresholds[static_cast<ptrdiff_t>(i)].second) {
                paralells += increments[static_cast<ptrdiff_t>(i)];
                break;
            }
        }
        if (count > 2500) {
            paralells += 2;
        }
        if (core::max_threads < 16 && count > 2000) {
            paralells += 1;
        } else if (core::max_threads > 16 && count > 1250) {
            if (count > 3500) {
                paralells += 2;
            } else if (count > 2000) {
                paralells += 1;
            }
        }
        paralells = (std::min)(core::max_threads, paralells);
        return paralells;
    }
}

#endif