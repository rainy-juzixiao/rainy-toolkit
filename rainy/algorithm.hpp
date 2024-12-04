#ifndef RAINY_ALGORITHM_HPP
#define RAINY_ALGORITHM_HPP
#include <rainy/base.hpp>

namespace rainy::algorithm::sort {
    template <typename Iter, typename Pred>
    void insertion_sort(Iter begin, Iter end, Pred _pred) noexcept(std::is_nothrow_constructible_v<Iter>) {
        if (begin == end) {
            return;
        }
        for (Iter i = begin + 1; i < end; ++i) {
            auto key = *i;
            Iter j = i;
            while (j != begin && _pred(*(j - 1), key)) {
                *j = *(j - 1);
                --j;
            }
            *j = key;
        }
    }

    template <typename Iter, typename Pred>
    void bubble_sort(Iter begin, Iter end,
                     Pred _pred) noexcept(std::is_nothrow_constructible_v<Iter> && std::is_nothrow_copy_constructible_v<Iter>) {
        if (begin == end) {
            return;
        }
        for (Iter i = begin; i != end; ++i) {
            bool swapped = false;
            for (Iter j = i; j != end; ++j) {
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

    template <typename Iter, typename Pred>
    void shell_sort(Iter begin, Iter end, Pred _pred) noexcept(std::is_nothrow_constructible_v<Iter>) {
        if (begin == end) {
            return;
        }
        auto n = std::distance(begin, end);
        for (auto gap = n / 2; gap > 0; gap /= 2) {
            // 为什么不直接调用insertion_sort是因为，我测试了1000个数字的随机数组。
            // 结果，实际跑分中，shell_sort使用insertion_sort简化逻辑的版本速度比insertion_sort本身更慢
            for (Iter i = begin + gap; i < end; ++i) {
                auto key = *i;
                Iter j = i;
                while (j >= begin + gap && _pred(*(j - gap), key)) {
                    *j = *(j - gap);
                    j -= gap;
                }
                *j = key;
            }
        }
    }
}

namespace rainy::algorithm::ranges::sort {
    template <typename Container, typename pred>
    void insertion_sort(Container &container, pred _pred) {
        algorithm::sort::insertion_sort(container.begin(), container.end(), _pred);
    }

    template <typename Container, typename pred>
    void bubble_sort(Container &container,
                     pred _pred) {
        algorithm::sort::bubble_sort(container.begin(), container.end(), _pred);
    }
}

#endif
