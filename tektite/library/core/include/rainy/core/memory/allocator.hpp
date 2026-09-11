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
#ifndef RAINY_CORE_MEMORY_ALLCATOR_HPP
#define RAINY_CORE_MEMORY_ALLCATOR_HPP
#include <atomic>
#include <new>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief Provides uniform interface to query and use allocator types.
     *
     * @tparam Alloc The allocator type to inspect
     *
     * \lang simp-chinese
     * @brief 提供统一接口来查询和使用分配器类型。
     *
     * @tparam Alloc 要检查的分配器类型
     */
    template <typename Alloc>
    struct allocator_traits;

#if RAINY_HAS_CXX23
    /**
     * \lang english
     * @brief The result type of an at-least allocation request.
     *
     * @tparam Ptr The pointer type returned by the allocation
     *
     * \lang simp-chinese
     * @brief “至少分配”请求的结果类型。
     *
     * @tparam Ptr 分配返回的指针类型
     */
    template <typename Ptr, typename = std::size_t>
    struct allocation_result : std::allocation_result<Ptr> {};
#else
    /**
     * \lang english
     * @brief The result type of an at-least allocation request.
     *
     * @tparam Ptr The pointer type returned by the allocation
     * @tparam SizeType The size type used to report the allocated count
     *
     * \lang simp-chinese
     * @brief “至少分配”请求的结果类型。
     *
     * @tparam Ptr 分配返回的指针类型
     * @tparam SizeType 用于报告已分配数量的尺寸类型
     */
    template <typename Ptr, typename SizeType = std::size_t>
    struct allocation_result {
        Ptr ptr;
        SizeType count;
    };
#endif

    /**
     * \lang english
     * @brief A stateless allocator that allocates raw memory for objects.
     *
     * This allocator is always equal and propagates on container move assignment.
     *
     * @tparam Ty The type of objects the allocator provides storage for
     *
     * \lang simp-chinese
     * @brief 为对象分配原始内存的无状态分配器。
     *
     * 该分配器始终相等，并且在容器移动赋值时传播。
     *
     * @tparam Ty 分配器为其提供存储的对象类型
     */
    template <typename Ty>
    class allocator {
    public:
        using value_type = Ty;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using propagate_on_container_move_assignment = type_traits::helper::true_type;
        using is_always_equal = type_traits::helper::true_type;

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        constexpr allocator() noexcept = default;

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param rhs The allocator to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param rhs 要拷贝的分配器
         */
        constexpr allocator(const allocator &) noexcept = default;

        /**
         * \lang english
         * @brief Converting constructor from another allocator type.
         *
         * @tparam U The element type of the source allocator
         * @param rhs The allocator to convert from
         *
         * \lang simp-chinese
         * @brief 从另一个分配器类型转换的构造函数。
         *
         * @tparam U 源分配器的元素类型
         * @param rhs 要转换的分配器
         */
        template <typename U>
        constexpr allocator(const allocator<U> &) noexcept {
        }

        /**
         * \lang english
         * @brief Destructor.
         *
         * \lang simp-chinese
         * @brief 析构函数。
         */
        constexpr ~allocator() = default;

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @return Reference to this allocator
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @return 此分配器的引用
         */
        constexpr allocator &operator=(const allocator &) = default;

        /**
         * \lang english
         * @brief Allocates storage for count objects of type Ty.
         *
         * @param count The number of objects to allocate storage for
         * @return Pointer to the allocated storage
         * @throws std::bad_alloc if the allocation fails
         *
         * \lang simp-chinese
         * @brief 为 count 个 Ty 类型对象分配存储。
         *
         * @param count 要分配存储的对象数量
         * @return 指向已分配存储的指针
         * @throws std::bad_alloc 如果分配失败
         */
        RAINY_NODISCARD_RAW_PTR_ALLOC constexpr Ty *allocate(const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<Ty>{}.allocate(count);
            }
#endif
            if (auto *ptr = static_cast<Ty *>(::operator new(count * sizeof(Ty), std::align_val_t{alignof(Ty)}))) {
                return ptr;
            }
            throw std::bad_alloc();
        }

        /**
         * \lang english
         * @brief Allocates at least count objects of type Ty.
         *
         * @param count The minimum number of objects to allocate storage for
         * @return The allocated pointer and the actual number of objects allocated
         *
         * \lang simp-chinese
         * @brief 分配至少 count 个 Ty 类型对象。
         *
         * @param count 要分配存储的对象最小数量
         * @return 已分配的指针和实际分配的对象数量
         */
        RAINY_NODISCARD_RAW_PTR_ALLOC constexpr allocation_result<Ty *, size_type> allocate_at_least(const size_type count) const {
            return {allocate(count), count};
        }

        /**
         * \lang english
         * @brief Constructs an object of type Ty in the given storage.
         *
         * @tparam Args The argument types used to construct the object
         * @param ptr Pointer to the storage where the object is constructed
         * @param args Arguments forwarded to the constructor of Ty
         *
         * \lang simp-chinese
         * @brief 在给定存储中构造一个 Ty 类型的对象。
         *
         * @tparam Args 用于构造对象的参数类型
         * @param ptr 指向构造对象所在存储的指针
         * @param args 转发给 Ty 构造函数实参
         */
        template <typename... Args>
        RAINY_CONSTEXPR20 void construct(value_type *const ptr, Args &&...args) const
            noexcept(type_traits::properties::is_nothrow_constructible_v<value_type, Args...>) {
            utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Destroys the object at the given storage.
         *
         * @param ptr Pointer to the object to destroy
         *
         * \lang simp-chinese
         * @brief 销毁给定存储处的对象。
         *
         * @param ptr 指向要销毁对象的指针
         */
        RAINY_CONSTEXPR20 void destroy(value_type *const ptr) const noexcept(type_traits::properties::is_nothrow_destructible_v<value_type>) {
            ptr->~value_type();
        }

        /**
         * \lang english
         * @brief Deallocates storage previously allocated by allocate.
         *
         * @param p Pointer to the storage to deallocate
         * @param count The number of objects the storage was allocated for
         *
         * \lang simp-chinese
         * @brief 归还先前由 allocate 分配的存储。
         *
         * @param p 指向要归还存储的指针
         * @param count 该存储被分配时的对象数量
         */
        constexpr void deallocate(Ty *const p, const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::allocator<Ty>{}.deallocate(p, count);
                return;
            }
#endif
            ::operator delete(p, count * sizeof(Ty), std::align_val_t{alignof(Ty)});
        }
    };

    /**
     * \lang english
     * @brief Compares two allocators for equality.
     *
     * All allocator instances are always equal.
     *
     * @tparam Ty The element type of the left-hand side allocator
     * @tparam Other The element type of the right-hand side allocator
     * @return true
     *
     * \lang simp-chinese
     * @brief 比较两个分配器是否相等。
     *
     * 所有分配器实例始终相等。
     *
     * @tparam Ty 左侧分配器的元素类型
     * @tparam Other 右侧分配器的元素类型
     * @return true
     */
    template <typename Ty, typename Other>
    RAINY_NODISCARD constexpr bool operator==(const allocator<Ty> &, const allocator<Other> &) noexcept {
        return true;
    }

    /**
     * \lang english
     * @brief Compares two allocators for inequality.
     *
     * @tparam Ty The element type of the left-hand side allocator
     * @tparam Other The element type of the right-hand side allocator
     * @return false
     *
     * \lang simp-chinese
     * @brief 比较两个分配器是否不相等。
     *
     * @tparam Ty 左侧分配器的元素类型
     * @tparam Other 右侧分配器的元素类型
     * @return false
     */
    template <typename Ty, typename Other>
    RAINY_NODISCARD bool operator!=(const allocator<Ty> &, const allocator<Other> &) noexcept {
        return false;
    }
}

namespace rainy::core::memory::implements {
    template <typename Alloc>
    RAINY_CONSTEXPR_BOOL is_std_allocator = false;

    template <typename Elem>
    RAINY_CONSTEXPR_BOOL is_std_allocator<std::allocator<Elem>> = true;

    template <typename Alloc, typename = void>
    struct has_select_on_container_copy_construction : type_traits::helper::false_type {};

    template <typename Alloc>
    struct has_select_on_container_copy_construction<
        Alloc, type_traits::other_trans::void_t<decltype(utility::declval<const Alloc &>().select_on_container_copy_construction())>>
        : type_traits::helper::true_type {};

    template <typename Alloc>
    struct std_allocator_traits;

    template <typename Type>
    struct std_allocator_traits<std::allocator<Type>> {
        using allocator_type = std::allocator<Type>;
        using value_type = typename allocator_type::value_type;

        using pointer = value_type *;
        using const_pointer = const value_type *;
        using void_pointer = void *;
        using const_void_pointer = const void *;

        using size_type = std::size_t;
        using difference_type = ptrdiff_t;

        using propagate_on_container_copy_assignment = type_traits::helper::false_type;
        using propagate_on_container_move_assignment = type_traits::helper::true_type;
        using propagate_on_container_swap = type_traits::helper::false_type;
        using is_always_equal = type_traits::helper::true_type;

        template <typename Other>
        using rebind_alloc = std::allocator<Other>;

        template <typename Other>
        using rebind_traits = allocator_traits<std::allocator<Other>>;

        RAINY_CONSTEXPR20 static pointer allocate(allocator_type al, size_type count) {
            return al.allocate(count);
        }

        RAINY_CONSTEXPR20 static pointer allocate(allocator_type al, size_type count, const_void_pointer *) {
            return al.allocate(count);
        }

        RAINY_CONSTEXPR20 static void deallocate(allocator_type al, pointer ptr, size_type count) {
            return al.deallocate(ptr, count);
        }

        template <typename Uty, typename... Args>
        RAINY_CONSTEXPR20 static void construct(allocator_type, Uty *ptr, Args &&...args) noexcept(
            noexcept(utility::construct_at(ptr, utility::forward<Args>(args)...))) {
            utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        template <typename Uty>
        RAINY_CONSTEXPR20 static void destroy(allocator_type, Uty *ptr) {
            ptr->~value_type();
        }

        RAINY_CONSTEXPR20 static size_type max_size(allocator_type al) noexcept {
            return (utility::numeric_limits<size_type>::max)() / sizeof(value_type);
        }

        RAINY_NODISCARD static RAINY_CONSTEXPR20 allocator_type
        select_on_container_copy_construction(const allocator_type &allocator) {
            if constexpr (has_select_on_container_copy_construction<allocator_type>::value) {
                return allocator.select_on_container_copy_construction();
            } else {
                return allocator;
            }
        }
    };

    template <typename Ty, typename = void>
    struct get_pointer_type {
        using type = typename Ty::value_type *;
    };

    template <typename Ty>
    struct get_pointer_type<Ty, type_traits::other_trans::void_t<typename Ty::pointer>> {
        using type = typename Ty::pointer;
    };

    template <typename Ty, typename = void>
    struct get_const_pointer_type {
        using ptr_t = typename get_pointer_type<Ty>::type;
        using val_t = typename Ty::value_type;

        using type = typename memory::pointer_traits<ptr_t>::template rebind<const val_t>;
    };

    template <typename Ty>
    struct get_const_pointer_type<Ty, type_traits::other_trans::void_t<typename Ty::const_pointer>> {
        using type = typename Ty::const_pointer;
    };

    template <typename Ty, typename = void>
    struct get_void_pointer_type {
        using ptr_t = typename get_pointer_type<Ty>::type;

        using type = typename memory::pointer_traits<ptr_t>::template rebind<void>;
    };

    template <typename Ty>
    struct get_void_pointer_type<Ty, type_traits::other_trans::void_t<typename Ty::void_pointer>> {
        using type = typename Ty::void_pointer;
    };

    template <typename Ty, typename = void>
    struct get_const_void_pointer_type {
        using ptr_t = typename get_pointer_type<Ty>::type;
        using type = typename memory::pointer_traits<ptr_t>::template rebind<const void>;
    };

    template <typename Ty>
    struct get_const_void_pointer_type<Ty, type_traits::other_trans::void_t<typename Ty::const_void_pointer>> {
        using type = typename Ty::const_void_pointer;
    };

    template <typename Ty, typename = void>
    struct get_difference_type {
        using ptr_t = typename get_pointer_type<Ty>::type;
        using type = typename memory::pointer_traits<ptr_t>::difference_type;
    };

    template <typename Ty>
    struct get_difference_type<Ty, type_traits::other_trans::void_t<typename Ty::difference_type>> {
        using type = typename Ty::difference_type;
    };

    template <typename Ty, typename = void>
    struct get_size_type {
        using type = type_traits::helper::make_unsigned_t<typename get_difference_type<Ty>::type>;
    };

    template <typename Ty>
    struct get_size_type<Ty, type_traits::other_trans::void_t<typename Ty::size_type>> {
        using type = typename Ty::size_type;
    };

    template <typename Ty, typename = void>
    struct get_propagate_on_container_copy {
        using type = type_traits::helper::false_type;
    };

    template <typename Ty>
    struct get_propagate_on_container_copy<Ty, type_traits::other_trans::void_t<typename Ty::propagate_on_container_copy_assignment>> {
        using type = typename Ty::propagate_on_container_copy_assignment;
    };

    template <typename Ty, typename = void>
    struct get_propagate_on_container_move {
        using type = type_traits::helper::false_type;
    };

    template <typename Ty>
    struct get_propagate_on_container_move<Ty, type_traits::other_trans::void_t<typename Ty::propagate_on_container_move_assignment>> {
        using type = typename Ty::propagate_on_container_move_assignment;
    };

    template <typename Ty, typename = void>
    struct get_propagate_on_container_swap {
        using type = type_traits::helper::false_type;
    };

    template <typename Ty>
    struct get_propagate_on_container_swap<Ty, type_traits::other_trans::void_t<typename Ty::propagate_on_container_swap>> {
        using type = typename Ty::propagate_on_container_swap;
    };

    template <typename Ty, typename = void>
    struct get_is_always_equal {
        using type = type_traits::helper::bool_constant<type_traits::properties::is_empty_v<Ty>>;
    };

    template <typename Ty>
    struct get_is_always_equal<Ty, type_traits::other_trans::void_t<typename Ty::is_always_equal>> {
        using type = typename Ty::is_always_equal;
    };

    template <typename Ty, typename Other, typename = void>
    struct get_rebind_type {
        using type = typename type_traits::extras::templates::replace_first_parameter<Other, Ty>::type;
    };

    template <typename Ty, typename Other>
    struct get_rebind_type<Ty, Other, type_traits::other_trans::void_t<typename Ty::template rebind<Other>::other>> {
        using type = typename Ty::template rebind<Other>::other;
    };

    template <typename Alloc, typename SizeType, typename ConstVoidPointer, typename = void>
    struct has_allocate_hint : type_traits::helper::false_type {};

    template <typename Alloc, typename SizeType, typename ConstVoidPointer>
    struct has_allocate_hint<Alloc, SizeType, ConstVoidPointer,
                             type_traits::other_trans::void_t<decltype(utility::declval<Alloc &>().allocate(
                                 utility::declval<const SizeType &>(), utility::declval<const ConstVoidPointer &>()))>>
        : type_traits::helper::false_type {};

    template <typename Alloc>
    struct normal_allocator_traits {
        using allocator_type = Alloc;
        using value_type = typename Alloc::value_type;

        using pointer = typename get_pointer_type<Alloc>::type;
        using const_pointer = typename get_const_pointer_type<Alloc>::type;
        using void_pointer = typename get_void_pointer_type<Alloc>::type;
        using const_void_pointer = typename get_const_void_pointer_type<Alloc>::type;

        using size_type = typename get_size_type<Alloc>::type;
        using difference_type = typename get_difference_type<Alloc>::type;

        using propagate_on_container_copy_assignment = typename get_propagate_on_container_copy<Alloc>::type;
        using propagate_on_container_move_assignment = typename get_propagate_on_container_move<Alloc>::type;
        using propagate_on_container_swap = typename get_propagate_on_container_swap<Alloc>::type;
        using is_always_equal = typename get_is_always_equal<Alloc>::type;

        template <typename Other>
        using rebind_alloc = typename get_rebind_type<Alloc, Other>::type;

        template <typename Other>
        using rebind_traits = allocator_traits<rebind_alloc<Other>>;

        RAINY_NODISCARD_RAW_PTR_ALLOC static RAINY_CONSTEXPR20 pointer allocate(allocator_type &allocator, const size_type count) {
            return allocator.allocate(count);
        }

        RAINY_NODISCARD_RAW_PTR_ALLOC static RAINY_CONSTEXPR20 pointer allocate(allocator_type &allocator, const size_type count,
                                                                                const const_void_pointer hint) {
            if constexpr (has_allocate_hint<allocator_type, size_type, const_void_pointer>::value) {
                return allocator.allocate(count, hint);
            } else {
                return allocator.allocate(count);
            }
        }

        static RAINY_CONSTEXPR20 void deallocate(allocator_type &allocator, pointer ptr, size_type count) {
            allocator.deallocate(ptr, count);
        }

        template <typename Ty_, typename... Args>
        static RAINY_CONSTEXPR20 void construct(allocator_type &allocator, Ty_ *ptr, Args &&...args) {
            allocator.construct(ptr, utility::forward<Args>(args)...);
        }

        template <typename Ty_>
        static RAINY_CONSTEXPR20 void destroy(allocator_type &allocator, Ty_ *ptr) {
            allocator.destroy(ptr);
        }

        static RAINY_CONSTEXPR20 size_type max_size(allocator_type al) noexcept {
            if constexpr (type_traits::extras::meta_method::has_max_size_v<allocator_type>) {
                return al.max_size();
            } else {
                return (utility::numeric_limits<size_type>::max)() / sizeof(value_type);
            }
        }

        RAINY_NODISCARD static RAINY_CONSTEXPR20 allocator_type
        select_on_container_copy_construction(const allocator_type &allocator) {
            if constexpr (has_select_on_container_copy_construction<allocator_type>::value) {
                return allocator.select_on_container_copy_construction();
            } else {
                return allocator;
            }
        }
    };
}

namespace rainy::core::memory {
    template <typename Alloc>
    struct allocator_traits
        : type_traits::other_trans::conditional_t<implements::is_std_allocator<Alloc>, implements::std_allocator_traits<Alloc>,
                                                  implements::normal_allocator_traits<Alloc>> {};
}

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief An allocator that serves allocations from a fixed in-place block.
     *
     * The first allocation of at most N elements is served from the internal
     * storage block; larger or subsequent allocations fall back to allocator<Ty>.
     *
     * @tparam Ty The element type
     * @tparam N The number of elements that fit in the internal block
     *
     * \lang simp-chinese
     * @brief 从固定内联块中提供分配的分配器。
     *
     * 首次最多 N 个元素的分配由内部存储块提供；更大或后续的分配回退到 allocator<Ty>。
     *
     * @tparam Ty 元素类型
     * @tparam N 内部块可容纳的元素数量
     */
    template <typename Ty, std::size_t N>
    class block_allocator {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;

        /**
         * \lang english
         * @brief The alignment of the internal storage block.
         *
         * \lang simp-chinese
         * @brief 内部存储块的对齐方式。
         */
        static constexpr std::size_t align = alignof(value_type);

        /**
         * \lang english
         * @brief The size of one element.
         *
         * \lang simp-chinese
         * @brief 单个元素的大小。
         */
        static constexpr std::size_t element_size = sizeof(value_type);

        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。
         */
        RAINY_CONSTEXPR20 block_allocator() noexcept = default;

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param rhs The block_allocator to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param rhs 要拷贝的 block_allocator
         */
        RAINY_CONSTEXPR20 block_allocator(const block_allocator &) noexcept = default;

        /**
         * \lang english
         * @brief Converting constructor from another block_allocator type.
         *
         * @tparam U The element type of the source allocator
         * @tparam N_ The block size of the source allocator
         * @param rhs The allocator to convert from
         *
         * \lang simp-chinese
         * @brief 从另一个 block_allocator 类型转换的构造函数。
         *
         * @tparam U 源分配器的元素类型
         * @tparam N_ 源分配器的块大小
         * @param rhs 要转换的分配器
         */
        template <typename U, std::size_t N_>
        RAINY_CONSTEXPR20 explicit block_allocator(const block_allocator<U, N_> &) noexcept {
        }

        /**
         * \lang english
         * @brief Destructor.
         *
         * \lang simp-chinese
         * @brief 析构函数。
         */
        RAINY_CONSTEXPR20 ~block_allocator() = default;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param rhs The block_allocator to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param rhs 要移动的 block_allocator
         */
        constexpr block_allocator(block_allocator &&) noexcept = default;

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @return Reference to this allocator
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @return 此分配器的引用
         */
        constexpr block_allocator &operator=(const block_allocator &) noexcept = default;

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @return Reference to this allocator
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @return 此分配器的引用
         */
        constexpr block_allocator &operator=(block_allocator &&) noexcept = default;

        /**
         * \lang english
         * @brief Allocates storage for count objects of type Ty.
         *
         * @param count The number of objects to allocate storage for
         * @return Pointer to the allocated storage
         *
         * \lang simp-chinese
         * @brief 为 count 个 Ty 类型对象分配存储。
         *
         * @param count 要分配存储的对象数量
         * @return 指向已分配存储的指针
         */
        RAINY_NODISCARD_RAW_PTR_ALLOC RAINY_CONSTEXPR20 pointer allocate(const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<Ty>{}.allocate(count);
            } else
#endif
            {
                if (count > N || allocated_) {
                    return allocator<Ty>{}.allocate(count);
                }
                allocated_ = true;
                return reinterpret_cast<pointer>(const_cast<core::byte_t *>(resources));
            }
        }

        /**
         * \lang english
         * @brief Deallocates storage previously allocated by allocate.
         *
         * @param block Pointer to the storage to deallocate
         * @param count The number of objects the storage was allocated for
         *
         * \lang simp-chinese
         * @brief 归还先前由 allocate 分配的存储。
         *
         * @param block 指向要归还存储的指针
         * @param count 该存储被分配时的对象数量
         */
        RAINY_CONSTEXPR20 void deallocate(value_type *block, const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<Ty>{}.deallocate(block, count);
            } else
#endif
            {
                if (!block || count == 0) {
                    return;
                }
                if (block != static_cast<const void *>(utility::addressof(resources))) {
                    allocator<Ty>{}.deallocate(block, count);
                } else {
                    allocated_ = false;
                }
            }
        }

        /**
         * \lang english
         * @brief Allocates at least count objects of type Ty.
         *
         * @param count The minimum number of objects to allocate storage for
         * @return The allocated pointer and the actual number of objects allocated
         *
         * \lang simp-chinese
         * @brief 分配至少 count 个 Ty 类型对象。
         *
         * @param count 要分配存储的对象最小数量
         * @return 已分配的指针和实际分配的对象数量
         */
        RAINY_NODISCARD_RAW_PTR_ALLOC constexpr allocation_result<pointer> allocate_at_least(const size_type count) const {
            return {allocate(count), count};
        }

        /**
         * \lang english
         * @brief Constructs an object of type Ty in the given storage.
         *
         * @tparam Args The argument types used to construct the object
         * @param ptr Pointer to the storage where the object is constructed
         * @param args Arguments forwarded to the constructor of Ty
         *
         * \lang simp-chinese
         * @brief 在给定存储中构造一个 Ty 类型的对象。
         *
         * @tparam Args 用于构造对象的参数类型
         * @param ptr 指向构造对象所在存储的指针
         * @param args 转发给 Ty 构造函数实参
         */
        template <typename... Args>
        RAINY_CONSTEXPR20 void construct(value_type *const ptr, Args &&...args) const
            noexcept(type_traits::properties::is_nothrow_constructible_v<value_type, Args...>) {
            utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Destroys the object at the given storage.
         *
         * @param ptr Pointer to the object to destroy
         *
         * \lang simp-chinese
         * @brief 销毁给定存储处的对象。
         *
         * @param ptr 指向要销毁对象的指针
         */
        RAINY_CONSTEXPR20 void destroy(value_type *const ptr) const noexcept(type_traits::properties::is_nothrow_destructible_v<value_type>) {
            ptr->~value_type();
        }

    private:
        mutable std::atomic_bool allocated_{false};
        alignas(Ty) core::byte_t resources[N == 0 ? 1 : element_size * N]{};
    };
}

namespace rainy::core::memory::implements {
    template <typename Alloc>
    struct alloc_construct_ptr {
        using pointer = typename allocator_traits<Alloc>::pointer;

        RAINY_CONSTEXPR20 explicit alloc_construct_ptr(Alloc &alloc) : alloc(alloc), ptr(nullptr) {
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer release() noexcept {
            return utility::exchange(ptr, nullptr);
        }

        RAINY_CONSTEXPR20 void allocate() {
            ptr = nullptr;
            ptr = alloc.allocate(1);
        }

        RAINY_CONSTEXPR20 ~alloc_construct_ptr() {
            if (ptr) {
                alloc.deallocate(ptr, 1);
            }
        }

        alloc_construct_ptr(const alloc_construct_ptr &) = delete;
        alloc_construct_ptr &operator=(const alloc_construct_ptr &) = delete;

        Alloc &alloc;
        pointer ptr;
    };
}

#endif
