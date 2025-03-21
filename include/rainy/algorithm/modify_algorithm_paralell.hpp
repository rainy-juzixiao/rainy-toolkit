#ifndef RAINY_MODIFY_ALGORITHM_OPERATOR
#define RAINY_MODIFY_ALGORITHM_OPERATOR
#include <rainy/algorithm/execution.h>

namespace rainy::algorithm::container_operater {
    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy(execution::policy policy, InputIter begin, InputIter end, OutIter dest) {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;
        if (policy == execution::seq) {
            return copy(begin, end, dest);
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return copy(begin, end, dest);
        }
#endif

        auto input_begin = utility::addressof(*begin);
        const std::size_t count = utility::distance(begin, end);
        std::size_t threads = internals::get_paralells(count);
        if (policy == execution::max_par) {
            threads = core::max_threads;
        } else {
            if (threads == 1) {
                return copy(begin, end, dest);
            }
        }
        std::size_t chunk_size = count / threads;
        std::size_t remainder = count % threads;
        std::vector<std::future<void>> tasks(threads);
        InputIter chunk_start = begin;
        OutIter chunk_dest = dest;
        for (std::size_t i = 0; i < threads; ++i) {
            std::size_t current_chunk_size = chunk_size + (i < remainder ? 1 : 0);
            tasks.emplace_back(std::async(std::launch::async, [chunk_start, chunk_dest, current_chunk_size]() {
                copy_n(chunk_start, current_chunk_size, chunk_dest);
            }));
            std::advance(chunk_start, current_chunk_size);
            std::advance(chunk_dest, current_chunk_size);
        }
        for (auto &task: tasks) {
            task.get();
        }
        return chunk_dest;
    }

    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy_n(execution::policy policy, InputIter begin, const std::size_t count, OutIter dest) {
        using value_type = typename InputIter::value_type;
        if (policy == execution::seq) {
            return copy_n(begin, count, dest);
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return copy_n(begin, count, dest);
        }
#endif
        auto input_begin = utility::addressof(*begin);
        std::size_t threads = internals::get_paralells(count); // 获取可能的规模并计算出可能适合的线程数量（考虑最大线程数）
        if (policy == execution::max_par) {
            threads = core::max_threads;
        } else {
            if (threads == 1) {
                // 线程数若为1，则只调用单线程版本
                return copy_n(begin, count, dest);
            }
        }
        std::size_t chunk_size = count / threads;
        std::size_t remainder = count % threads;
        std::vector<std::future<void>> tasks(threads); // 创建task表
        InputIter chunk_start = begin;
        OutIter chunk_dest = dest;
        for (std::size_t i = 0; i < threads; ++i) {
            std::size_t current_chunk_size = chunk_size + (i < remainder ? 1 : 0);
            tasks.emplace_back(std::async(std::launch::async, [chunk_start, chunk_dest, current_chunk_size]() {
                copy_n(chunk_start, current_chunk_size, chunk_dest);
            }));
            // 让std::async为我们分配工作线程（不考虑返回值）
            std::advance(chunk_start, current_chunk_size);
            std::advance(chunk_dest, current_chunk_size);
        }
        for (auto &task: tasks) {
            task.get();
        }
        return chunk_dest;
    }

}

namespace rainy::component::ranges::container_operater {
    template <typename Rng, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy(algorithm::execution::policy policy, Rng &&rng, OutIter dest) {
        return algorithm::container_operater::copy(policy, utility::begin(rng), std::end(rng), dest);
    }

    template <typename Rng, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy_n(algorithm::execution::policy policy, Rng &&rng, std::size_t count, OutIter dest) {
        return algorithm::container_operater::copy_n(policy, utility::begin(rng), count, dest);
    }
}

#endif