/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_CORE_UTILITY_UNINITIALIZED_HPP
#define RAINY_CORE_UTILITY_UNINITIALIZED_HPP
#include <cstring>
#include <memory>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <type_traits>

namespace rainy::utility::implements {
    template <typename SrcIter, typename DestIter>
    RAINY_CONSTEXPR_BOOL is_mem_copyable_v =
        type_traits::properties::is_trivially_copyable_v<
            type_traits::modifers::remove_cv_t<type_traits::modifers::remove_reference_t<decltype(*utility::declval<SrcIter>())>>> &&
        type_traits::primary_types::is_pointer_v<SrcIter> && type_traits::primary_types::is_pointer_v<DestIter> &&
        type_traits::type_relations::is_same_v<type_traits::modifers::remove_cv_t<type_traits::modifers::remove_reference_t<decltype(*utility::declval<SrcIter>())>>,
                       type_traits::modifers::remove_cv_t<type_traits::modifers::remove_reference_t<decltype(*utility::declval<DestIter>())>>>;

    template <typename SrcIter, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_copy_impl(SrcIter first, SrcIter last, DestIter dest) -> DestIter {
        for (; first != last; ++first, ++dest) {
            core::builtin::construct_at(utility::addressof(*dest), *first);
        }
        return dest;
    }

    template <typename SrcIter, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_move_impl(SrcIter first, SrcIter last, DestIter dest) -> DestIter {
        for (; first != last; ++first, ++dest) {
            core::builtin::construct_at(utility::addressof(*dest), move(*first));
        }
        return dest;
    }

    template <typename SrcIter, typename SizeType, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_copy_n_impl(SrcIter first, SizeType count, DestIter dest) -> DestIter {
        for (SizeType i = 0; i < count; ++i, ++first, ++dest) {
            core::builtin::construct_at(utility::addressof(*dest), *first);
        }
        return dest;
    }

    template <typename SrcIter, typename SizeType, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_move_n_impl(SrcIter first, SizeType count, DestIter dest) -> DestIter {
        for (SizeType i = 0; i < count; ++i, ++first, ++dest) {
            core::builtin::construct_at(utility::addressof(*dest), move(*first));
        }
        return dest;
    }

    template <typename DestIter, typename SizeType>
    RAINY_CONSTEXPR20 rain_fn uninitialized_value_construct_n_impl(DestIter first, SizeType count) -> DestIter {
        for (SizeType i = 0; i < count; ++i, ++first) {
            core::builtin::construct_at(utility::addressof(*first));
        }
        return first;
    }
}

namespace rainy::utility {
    template <typename SrcIter, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_copy(SrcIter first, SrcIter last, DestIter dest) -> DestIter {
        if constexpr (implements::is_mem_copyable_v<SrcIter, DestIter>) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return implements::uninitialized_copy_impl(first, last, dest);
            }
#endif
            const auto count = static_cast<std::size_t>(last - first);
            if (count != 0) {
                core::builtin::copy_memory(static_cast<void *>(dest), static_cast<const void *>(first), sizeof(*first) * count);
            }
            return dest + count;
        } else {
            return implements::uninitialized_copy_impl(first, last, dest);
        }
    }

    template <typename SrcIter, typename SizeType, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_copy_n(SrcIter first, SizeType count, DestIter dest) -> DestIter {
        if constexpr (implements::is_mem_copyable_v<SrcIter, DestIter>) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return implements::uninitialized_copy_n_impl(first, count, dest);
            }
#endif
            if (count != 0) {
                core::builtin::copy_memory(static_cast<void *>(dest), static_cast<const void *>(first),
                            sizeof(*first) * static_cast<std::size_t>(count));
            }
            return dest + count;
        } else {
            return implements::uninitialized_copy_n_impl(first, count, dest);
        }
    }

    template <typename SrcIter, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_move(SrcIter first, SrcIter last, DestIter dest) -> DestIter {
        if constexpr (implements::is_mem_copyable_v<SrcIter, DestIter>) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return implements::uninitialized_move_impl(first, last, dest);
            }
#endif
            const auto count = static_cast<std::size_t>(last - first);
            if (count != 0) {
                core::builtin::copy_memory(static_cast<void *>(dest), static_cast<const void *>(first), sizeof(*first) * count);
            }
            return dest + count;
        } else {
            return implements::uninitialized_move_impl(first, last, dest);
        }
    }

    template <typename SrcIter, typename SizeType, typename DestIter>
    RAINY_CONSTEXPR20 rain_fn uninitialized_move_n(SrcIter first, SizeType count, DestIter dest) -> DestIter {
        if constexpr (implements::is_mem_copyable_v<SrcIter, DestIter>) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return implements::uninitialized_move_n_impl(first, count, dest);
            }
#endif
            if (count != 0) {
                core::builtin::copy_memory(static_cast<void *>(dest), static_cast<const void *>(first),
                            sizeof(*first) * static_cast<std::size_t>(count));
            }
            return dest + count;
        } else {
            return implements::uninitialized_move_n_impl(first, count, dest);
        }
    }

    template <typename DestIter, typename SizeType>
    RAINY_CONSTEXPR20 rain_fn uninitialized_value_construct_n(DestIter first, SizeType count) -> DestIter {
        if constexpr (type_traits::primary_types::is_pointer_v<DestIter> &&
                      type_traits::properties::is_trivially_copyable_v<
                          type_traits::modifers::remove_cv_t<type_traits::modifers::remove_reference_t<decltype(*utility::declval<DestIter>())>>> &&
                      type_traits::properties::is_trivially_default_constructible_v<
                          type_traits::modifers::remove_cv_t<type_traits::modifers::remove_reference_t<decltype(*utility::declval<DestIter>())>>>) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return implements::uninitialized_value_construct_n_impl(first, count);
            }
#endif
            if (count != 0) {
                core::builtin::set_memory(static_cast<void *>(first), 0, sizeof(*first) * static_cast<std::size_t>(count));
            }
            return first + count;
        } else {
            return implements::uninitialized_value_construct_n_impl(first, count);
        }
    }
}

#endif
