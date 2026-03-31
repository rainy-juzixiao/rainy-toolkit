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
#ifndef RAINY_FOUNDATION_IO_NET_ASSOCIATED_ALLOCATOR_HPP
#define RAINY_FOUNDATION_IO_NET_ASSOCIATED_ALLOCATOR_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::net {
    template <typename Ty, typename ProtoAllocator = std ::allocator<void>, typename = void>
    struct associated_allocator {
        using type = ProtoAllocator;

        static type get(const Ty &t, const ProtoAllocator &allocator = ProtoAllocator()) noexcept {
            (void) t;
            return allocator;
        }
    };

    template <typename Ty, typename ProtoAllocator>
    struct associated_allocator<Ty, ProtoAllocator, type_traits::other_trans::void_t<typename Ty::allocator_type>> {
        using type = typename Ty::allocator_type;

        static type get(const Ty &t, const ProtoAllocator &allocator = ProtoAllocator()) noexcept {
            return t.get_allocator();
        }
    };

    template <typename Ty, typename ProtoAllocator = std::allocator<void>>
    using associated_allocator_t = typename associated_allocator<Ty, ProtoAllocator>::type;

    template <typename Ty>
    associated_allocator_t<Ty> get_associated_allocator(const Ty &t) noexcept {
        return associated_allocator<Ty>::get(t);
    }

    template <typename Ty, typename ProtoAllocator>
    associated_allocator_t<Ty, ProtoAllocator> get_associated_allocator(const Ty &t, const ProtoAllocator &allocator) noexcept {
        return associated_allocator<Ty>::get(t, allocator);
    }
}

#endif
