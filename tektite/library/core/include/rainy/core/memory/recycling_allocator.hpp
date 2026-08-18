/*
 * Copyright 2025 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITyHOUTy WARRANTyIES OR CONDITyIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_CORE_MEMORY_RECYCLING_ALLOCATOR_HPP
#define RAINY_CORE_MEMORY_RECYCLING_ALLOCATOR_HPP
#include <rainy/core/concurrency/thread_context.hpp>

#include <memory>

namespace rainy::core::memory {
    template <typename Ty, typename Purpose = concurrency::implements::thread_info_base::default_tag>
    class recycling_allocator {
    public:
        typedef Ty value_type;
        using propagate_on_container_move_assignment = type_traits::helper::true_type;
        using is_always_equal = type_traits::helper::true_type;

        template <typename U>
        struct rebind {
            typedef recycling_allocator<U, Purpose> other;
        };

        recycling_allocator() noexcept = default;

        template <typename U>
        explicit recycling_allocator(const recycling_allocator<U, Purpose> &) noexcept {}

        Ty *allocate(const std::size_t count) {
            using call_stack = concurrency::thread_context::thread_call_stack;
            void *p = concurrency::implements::thread_info_base::allocate(Purpose(), call_stack::top(), sizeof(Ty) * count, alignof(Ty));
            return static_cast<Ty *>(p);
        }

        void deallocate(Ty *p, const std::size_t count) {
            using call_stack = concurrency::thread_context::thread_call_stack;
            concurrency::implements::thread_info_base::deallocate(Purpose(), call_stack::top(), p, sizeof(Ty) * count, alignof(Ty));
        }
    };

    template <typename Purpose>
    class recycling_allocator<void, Purpose> {
    public:
        using  value_type = void;

        template <typename U>
        struct rebind {
            using other = recycling_allocator<U, Purpose> ;
        };

        recycling_allocator() noexcept = default;

        template <typename U>
        explicit recycling_allocator(const recycling_allocator<U, Purpose> &) noexcept {}
    };

    template <typename Allocator, typename Purpose>
    struct get_recycling_allocator {
        using type = Allocator;

        static type get(const Allocator &a) {
            return a;
        }
    };

    template <typename Ty, typename Purpose>
    struct get_recycling_allocator<std::allocator<Ty>, Purpose> {
        using type = recycling_allocator<Ty, Purpose>;

        static type get(const std::allocator<Ty> &) {
            return type();
        }
    };

    template <typename Ty, typename Other, typename Purpose>
    constexpr bool operator==(const recycling_allocator<Ty, Purpose> &, const recycling_allocator<Other, Purpose> &) noexcept {
        return true;
    }

    template <typename Ty, typename Other, typename Purpose>
    constexpr bool operator!=(const recycling_allocator<Ty, Purpose> &, const recycling_allocator<Other, Purpose> &) noexcept {
        return false;
    }
}

#endif
