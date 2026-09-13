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
    /**
     * \lang english
     * @brief An allocator that recycles memory through the thread memory pool.
     *
     * Allocations are served from the per-purpose thread recycle pool,
     * which reduces the overhead of repeated new/delete calls.
     *
     * @tparam Ty The element type
     * @tparam Purpose The pool tag selecting which recycle pool to use
     *
     * \lang simp-chinese
     * @brief 通过线程内存池回收内存的分配器。
     *
     * 分配由按用途分类的线程回收池提供，从而减少重复 new/delete 调用的开销。
     *
     * @tparam Ty 元素类型
     * @tparam Purpose 选择使用哪个回收池的池标记
     */
    template <typename Ty, typename Purpose = concurrency::implements::thread_info_base::default_tag>
    class recycling_allocator {
    public:
        typedef Ty value_type;
        using propagate_on_container_move_assignment = type_traits::helper::true_type;
        using is_always_equal = type_traits::helper::true_type;

        /**
         * \lang english
         * @brief Rebinds the allocator to another element type.
         *
         * @tparam U The new element type
         *
         * \lang simp-chinese
         * @brief 将分配器重绑定到另一个元素类型。
         *
         * @tparam U 新的元素类型
         */
        template <typename U>
        struct rebind {
            typedef recycling_allocator<U, Purpose> other;
        };

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        recycling_allocator() noexcept = default;

        /**
         * \lang english
         * @brief Converting constructor from another recycling_allocator type.
         *
         * @tparam U The element type of the source allocator
         * @param rhs The allocator to convert from
         *
         * \lang simp-chinese
         * @brief 从另一个 recycling_allocator 类型转换的构造函数。
         *
         * @tparam U 源分配器的元素类型
         * @param rhs 要转换的分配器
         */
        template <typename U>
        explicit recycling_allocator(const recycling_allocator<U, Purpose> &) noexcept {}

        /**
         * \lang english
         * @brief Allocates storage for count objects of type Ty from the recycle pool.
         *
         * @param count The number of objects to allocate storage for
         * @return Pointer to the allocated storage
         *
         * \lang simp-chinese
         * @brief 从回收池为 count 个 Ty 类型对象分配存储。
         *
         * @param count 要分配存储的对象数量
         * @return 指向已分配存储的指针
         */
        Ty *allocate(const std::size_t count) {
            using call_stack = concurrency::thread_context::thread_call_stack;
            void *p = concurrency::implements::thread_info_base::allocate(Purpose(), call_stack::top(), sizeof(Ty) * count, alignof(Ty));
            return static_cast<Ty *>(p);
        }

        /**
         * \lang english
         * @brief Deallocates storage back to the recycle pool.
         *
         * @param p Pointer to the storage to deallocate
         * @param count The number of objects the storage was allocated for
         *
         * \lang simp-chinese
         * @brief 将存储归还给回收池。
         *
         * @param p 指向要归还存储的指针
         * @param count 该存储被分配时的对象数量
         */
        void deallocate(Ty *p, const std::size_t count) {
            using call_stack = concurrency::thread_context::thread_call_stack;
            concurrency::implements::thread_info_base::deallocate(Purpose(), call_stack::top(), p, sizeof(Ty) * count, alignof(Ty));
        }
    };

    /**
     * \lang english
     * @brief Specialization of recycling_allocator for void.
     *
     * Only supports rebinding; no memory can be allocated for void.
     *
     * @tparam Purpose The pool tag selecting which recycle pool to use
     *
     * \lang simp-chinese
     * @brief recycling_allocator 针对 void 的特化。
     *
     * 仅支持重绑定；无法为 void 分配内存。
     *
     * @tparam Purpose 选择使用哪个回收池的池标记
     */
    template <typename Purpose>
    class recycling_allocator<void, Purpose> {
    public:
        using  value_type = void;

        /**
         * \lang english
         * @brief Rebinds the allocator to another element type.
         *
         * @tparam U The new element type
         *
         * \lang simp-chinese
         * @brief 将分配器重绑定到另一个元素类型。
         *
         * @tparam U 新的元素类型
         */
        template <typename U>
        struct rebind {
            using other = recycling_allocator<U, Purpose> ;
        };

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        recycling_allocator() noexcept = default;

        /**
         * \lang english
         * @brief Converting constructor from another recycling_allocator type.
         *
         * @tparam U The element type of the source allocator
         * @param rhs The allocator to convert from
         *
         * \lang simp-chinese
         * @brief 从另一个 recycling_allocator 类型转换的构造函数。
         *
         * @tparam U 源分配器的元素类型
         * @param rhs 要转换的分配器
         */
        template <typename U>
        explicit recycling_allocator(const recycling_allocator<U, Purpose> &) noexcept {}
    };

    /**
     * \lang english
     * @brief Maps an allocator to the recycling allocator used for the given purpose.
     *
     * @tparam Allocator The allocator type to map
     * @tparam Purpose The pool tag selecting which recycle pool to use
     *
     * \lang simp-chinese
     * @brief 将分配器映射到用于给定用途的回收分配器。
     *
     * @tparam Allocator 要映射的分配器类型
     * @tparam Purpose 选择使用哪个回收池的池标记
     */
    template <typename Allocator, typename Purpose>
    struct get_recycling_allocator {
        using type = Allocator;

        /**
         * \lang english
         * @brief Returns the allocator to use for the given purpose.
         *
         * @param a The source allocator
         * @return The allocator to use
         *
         * \lang simp-chinese
         * @brief 返回用于给定用途的分配器。
         *
         * @param a 源分配器
         * @return 要使用的分配器
         */
        static type get(const Allocator &a) {
            return a;
        }
    };

    /**
     * \lang english
     * @brief Specialization that maps allocator<Ty> to recycling_allocator<Ty, Purpose>.
     *
     * @tparam Ty The element type
     * @tparam Purpose The pool tag selecting which recycle pool to use
     *
     * \lang simp-chinese
     * @brief 将 allocator<Ty> 映射到 recycling_allocator<Ty, Purpose> 的特化。
     *
     * @tparam Ty 元素类型
     * @tparam Purpose 选择使用哪个回收池的池标记
     */
    template <typename Ty, typename Purpose>
    struct get_recycling_allocator<memory::allocator<Ty>, Purpose> {
        using type = recycling_allocator<Ty, Purpose>;

        /**
         * \lang english
         * @brief Returns the recycling allocator for the given purpose.
         *
         * @param allocator The source allocator (unused)
         * @return A recycling_allocator for the given purpose
         *
         * \lang simp-chinese
         * @brief 返回用于给定用途的回收分配器。
         *
         * @param allocator 源分配器（未使用）
         * @return 用于给定用途的 recycling_allocator
         */
        static type get(const memory::allocator<Ty> &) {
            return type();
        }
    };

    /**
     * \lang english
     * @brief Compares two recycling_allocators for equality.
     *
     * All recycling_allocator instances are always equal.
     *
     * @tparam Ty The element type of the left-hand side allocator
     * @tparam Other The element type of the right-hand side allocator
     * @tparam Purpose The pool tag of both allocators
     * @return true
     *
     * \lang simp-chinese
     * @brief 比较两个 recycling_allocator 是否相等。
     *
     * 所有 recycling_allocator 实例始终相等。
     *
     * @tparam Ty 左侧分配器的元素类型
     * @tparam Other 右侧分配器的元素类型
     * @tparam Purpose 两个分配器的池标记
     * @return true
     */
    template <typename Ty, typename Other, typename Purpose>
    constexpr bool operator==(const recycling_allocator<Ty, Purpose> &, const recycling_allocator<Other, Purpose> &) noexcept {
        return true;
    }

    /**
     * \lang english
     * @brief Compares two recycling_allocators for inequality.
     *
     * @tparam Ty The element type of the left-hand side allocator
     * @tparam Other The element type of the right-hand side allocator
     * @tparam Purpose The pool tag of both allocators
     * @return false
     *
     * \lang simp-chinese
     * @brief 比较两个 recycling_allocator 是否不相等。
     *
     * @tparam Ty 左侧分配器的元素类型
     * @tparam Other 右侧分配器的元素类型
     * @tparam Purpose 两个分配器的池标记
     * @return false
     */
    template <typename Ty, typename Other, typename Purpose>
    constexpr bool operator!=(const recycling_allocator<Ty, Purpose> &, const recycling_allocator<Other, Purpose> &) noexcept {
        return false;
    }
}

#endif
