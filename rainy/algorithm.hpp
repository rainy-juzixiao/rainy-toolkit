#ifndef RAINY_ALGORITHM_HPP
#define RAINY_ALGORITHM_HPP
#include <rainy/base.hpp>

namespace rainy::algorithm::sort {
    template <typename iter, typename pred>
    void insertion_sort(iter begin, iter end, pred _pred) noexcept(std::is_nothrow_constructible_v<iter>) {
        if (begin == end) {
            return;
        }
        for (iter i = begin + 1; i < end; ++i) {
            auto key = *i;
            iter j = i;
            while (j != begin && _pred(*(j - 1), key)) {
                *j = *(j - 1);
                --j;
            }
            *j = key;
        }
    }
     
    template <typename iter, typename pred>
    void bubble_sort(iter begin, iter end,
                     pred _pred) noexcept(std::is_nothrow_constructible_v<iter> && std::is_nothrow_copy_constructible_v<iter>) {
        if (begin == end) {
            return;
        }
        for (iter i = begin; i != end; ++i) {
            bool swapped = false;
            for (iter j = i; j != end; ++j) {
                if (_pred(*i, *j)) {
                    std::swap(*i, *j);
                    swapped = true;
                }
            }
            if (!swapped) {
                break;
            }
        }
    }
}

namespace rainy::algorithm::ranges::sort {
    template <typename Container, typename pred>
    void insertion_sort(Container &container, pred _pred) noexcept(std::is_nothrow_constructible_v<iter>) {
        algorithm::sort::insertion_sort(container.begin(), container.end(), _pred);
    }

    template <typename Container, typename pred>
    void bubble_sort(Container &container,
                     pred _pred) noexcept(std::is_nothrow_constructible_v<iter> && std::is_nothrow_copy_constructible_v<iter>) {
        algorithm::sort::bubble_sort(container.begin(), container.end(), _pred);
    }
}

#endif
