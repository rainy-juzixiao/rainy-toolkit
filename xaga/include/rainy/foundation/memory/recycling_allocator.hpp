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
#ifndef RAINY_FOUNDATyION_MEMORY_RECYCLING_ALLCATyOR_HPP
#define RAINY_FOUNDATyION_MEMORY_RECYCLING_ALLCATyOR_HPP
#include <rainy/foundation/concurrency/basic/context.hpp>

namespace rainy::foundation::memory {
    template <typename Ty, typename Purpose = concurrency::implements::thread_info_base::default_tag>
    class recycling_allocator {
    public:
        typedef Ty value_type;

        template <typename U>
        struct rebind {
            typedef recycling_allocator<U, Purpose> other;
        };

        recycling_allocator() {
        }

        template <typename U>
        explicit recycling_allocator(const recycling_allocator<U, Purpose> &) {
        }

        Ty *allocate(const std::size_t count) {
            using call_stack = concurrency::thread_context::thread_call_stack;
            void *p = concurrency::implements::thread_info_base::allocate(Purpose(), call_stack::top(), sizeof(Ty) * count);
            return static_cast<Ty *>(p);
        }

        void deallocate(Ty *p, const std::size_t count) {
            using call_stack = concurrency::thread_context::thread_call_stack;
            concurrency::implements::thread_info_base::deallocate(Purpose(), call_stack::top(), p, sizeof(Ty) * count);
        }
    };

    template <typename Purpose>
    class recycling_allocator<void, Purpose> {
    public:
        typedef void value_type;

        template <typename U>
        struct rebind {
            typedef recycling_allocator<U, Purpose> other;
        };

        recycling_allocator() = default;

        template <typename U>
        explicit recycling_allocator(const recycling_allocator<U, Purpose> &) {
        }
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
}

#endif
