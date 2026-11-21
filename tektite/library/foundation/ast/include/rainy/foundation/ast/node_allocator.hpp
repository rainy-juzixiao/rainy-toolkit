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
#ifndef RAINY_FOUNDATION_AST_NODE_ALLOCATOR_HPP
#define RAINY_FOUNDATION_AST_NODE_ALLOCATOR_HPP
#include <memory>
#include <memory_resource>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief A polymorphic child buffer whose contents live in a caller-provided memory resource.
     *
     * \lang simp-chinese
     * @brief 内容存放在调用方提供的内存资源中的多态子节点缓冲。
     */
    template <typename Ty>
    using node_buffer = core::collections::vector<Ty, std::pmr::polymorphic_allocator<Ty>>;

    /**
     * \lang english
     * @brief Allocates syntax nodes from a shared memory resource so references stay stable.
     *
     * @brief Nodes are never relocated while the owning tree is alive; the resource is expected to
     *        outlive every tree built through this allocator.
     *
     * \lang simp-chinese
     * @brief 从共享内存资源中分配语法节点，使引用保持稳定。
     *
     * @brief 只要拥有该树的资源存活，节点就不会被搬移；内存资源的生命周期应长于经它构建的每棵树。
     */
    template <typename Ty = void>
    class node_allocator {
    public:
        using value_type = Ty;

        explicit node_allocator(std::pmr::memory_resource *resource = std::pmr::get_default_resource()) noexcept :
            resource_(resource != nullptr ? resource : std::pmr::get_default_resource()) {
        }

        template <typename Other>
        node_allocator(const node_allocator<Other> &other) noexcept : resource_(other.resource()) {
        }

        std::pmr::memory_resource *resource() const noexcept {
            return resource_;
        }

        template <typename Other, typename... Args>
        Other *create(Args &&...args) {
            std::pmr::polymorphic_allocator<Other> allocator{resource_};
            Other *ptr = allocator.allocate(1);
            try {
                std::allocator_traits<std::pmr::polymorphic_allocator<Other>>::construct(allocator, ptr,
                                                                                         utility::forward<Args>(args)...);
            } catch (...) {
                allocator.deallocate(ptr, 1);
                throw;
            }
            return ptr;
        }

        template <typename Other>
        void destroy(Other *ptr) noexcept {
            if (ptr == nullptr) {
                return;
            }
            std::pmr::polymorphic_allocator<Other> allocator{resource_};
            std::allocator_traits<std::pmr::polymorphic_allocator<Other>>::destroy(allocator, ptr);
            allocator.deallocate(ptr, 1);
        }

        template <typename Other>
        node_allocator<Other> rebind() const noexcept {
            return node_allocator<Other>{resource_};
        }

    private:
        std::pmr::memory_resource *resource_;
    };
}

#endif
