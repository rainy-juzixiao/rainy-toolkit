#ifndef RAINY_MODIFY_ALGORITHM_OPERATOR
#define RAINY_MODIFY_ALGORITHM_OPERATOR
#include <rainy/algorithm/pre.h>

namespace rainy::algorithm::container_operater {
    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy(InputIter begin, InputIter end, OutIter dest) noexcept(
        std::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::internals::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename InputIter::value_type>>) {
        using value_type = typename InputIter::value_type;
        if (begin == end || (end - 1) == begin) {
            return dest; // 不进行复制
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            auto input_end = utility::addressof(*(end - 1)) + 1; // 防止MSVC编译器的DEBUG功能导致此处无法运作
            auto out_dest = utility::addressof(*dest);
            for (auto i = input_begin; i != input_end; ++i) {
                *out_dest = *i;
            }
            return out_dest;
        }
#endif
        if constexpr (std::is_standard_layout_v<value_type> && std::is_trivial_v<value_type>) {
            const auto input_begin = utility::addressof(*begin);
            const auto input_end = utility::addressof(*(end - 1)) + 1; // 防止MSVC编译器的DEBUG功能导致此处无法运作
            auto out_dest = utility::addressof(*dest);
            std::memcpy(out_dest, input_begin, sizeof(value_type) * utility::distance(input_begin, input_end));
        } else {
            for (InputIter i = begin; begin != end; ++i, ++dest) {
                *dest = *i;
            }
        }
        return dest;
    }

    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy(execution::policy policy, InputIter begin, InputIter end, OutIter dest) {
        using value_type = typename InputIter::value_type;
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
            threads = information::max_threads;
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
    RAINY_CONSTEXPR20 OutIter copy_n(InputIter begin, const std::size_t count, OutIter dest) noexcept(
        std::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::internals::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename InputIter::value_type>>) {

        using value_type = type_traits::other_trans::conditional_t<type_traits::internals::_is_pointer_v<InputIter>,
                                                                   type_traits::pointer_modify::remove_pointer_t<InputIter>,
                                                                   typename InputIter::value_type>;

        if (count == 0) {
            return dest; // 不进行复制
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            // 在C++20中，如果我们在常量环境求值，我们不需要考虑过多运行时优化
            auto input_begin = utility::addressof(*begin);
            auto out_dest = utility::addressof(*dest);
            for (std::size_t i = 0; i < count; ++i, ++input_begin, ++dest) {
                *out_dest = *input_begin;
            }
        } else
#endif
        {
            if constexpr (std::is_standard_layout_v<value_type> && std::is_trivial_v<value_type>) {
                const auto input_begin = utility::addressof(*begin);
                auto out_dest = utility::addressof(*dest);
                std::memcpy(out_dest, input_begin, sizeof(value_type) * count);
            } else {
                for (std::size_t i = 0; i < count; ++i, ++begin, ++dest) {
                    *dest = *begin;
                }
            }
        }
        return dest;
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
            threads = information::max_threads;
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


    template <typename InputIter, typename OutIter, typename Fx>
    constexpr OutIter transform(InputIter begin, InputIter end, OutIter dest, Fx func) noexcept(
        std::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::internals::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename InputIter::value_type>>) {
        for (InputIter iter = begin; iter != end; ++iter, ++dest) {
            *dest = func(*iter);
        }
        return dest;
    }

    template <typename InputIter, typename OutIter, typename Fx>
    constexpr OutIter transform(InputIter begin1, InputIter end1, InputIter begin2, OutIter dest, Fx func) noexcept(
        std::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::internals::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename InputIter::value_type>>) {
        if (begin1 == end1 || (end1 - 1) == begin1) {
            return dest;
        }
        for (InputIter iter = begin1; iter != end1; ++iter, ++dest, ++begin2) {
            *dest = func(*iter, *begin2);
        }
        return dest;
    }
}

namespace rainy::algorithm::ranges::container_operater {
    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy(InputContainer &container, OutContainer dest) noexcept(
        std::is_nothrow_copy_constructible_v<typename InputContainer::value_type>) {
        return algorithm::container_operater::copy(container.begin(), container.end(), dest.begin());
    }

    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy(execution::policy policy, InputContainer &container, OutContainer dest) {
        return algorithm::container_operater::copy(policy, container.begin(), container.end(), dest.begin());
    }

    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy_n(InputContainer &container, const std::size_t count, OutContainer dest) noexcept(
        std::is_nothrow_copy_constructible_v<typename InputContainer::value_type>) {
        return algorithm::container_operater::copy_n(container.begin(), count, dest.begin());
    }

    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy_n(execution::policy policy, InputContainer &container, const std::size_t count, OutContainer dest) {
        return algorithm::container_operater::copy_n(policy, container.begin(), count, dest.begin());
    }
}


#endif