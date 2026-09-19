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
#ifndef RAINY_CORE_CONTAINER_INDIRECT_HPP
#define RAINY_CORE_CONTAINER_INDIRECT_HPP
#include <rainy/core/type_traits.hpp>
#include <rainy/core/container/compressed_pair.hpp>
#include <rainy/core/memory/allocator.hpp>

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A tag type used to construct an indirect in a deferred (empty) state.
     *
     * \lang simp-chinese
     * @brief 用于将 indirect 构造为延迟（空）状态的标签类型。
     */
    struct defered_init_t {};

    /**
     * \lang english
     * @brief A constant of type defered_init_t used to select the deferred-initialization constructor.
     *
     * \lang simp-chinese
     * @brief 用于选择延迟初始化构造函数的 defered_init_t 类型常量。
     */
    inline constexpr defered_init_t defered_init;

    /**
     * \lang english
     * @brief An indirect value wrapper that owns a single object of type Ty allocated on the heap.
     *         Provides value semantics with copy/move support and allocator awareness.
     *
     * @tparam Ty The type of the managed object
     * @tparam Alloc The allocator type used for the underlying storage
     *
     * \lang simp-chinese
     * @brief 间接值包装器，拥有一个在堆上分配的 Ty 类型对象。
     *         提供带拷贝/移动支持且分配器感知的值语义。
     *
     * @tparam Ty 被管理对象的类型
     * @tparam Alloc 用于底层存储的分配器类型
     */
    template <typename Ty, typename Alloc = memory::allocator<Ty>>
    class indirect {
    public:
        /**
         * \lang english
         * @brief The type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理的值类型。
         */
        using value_type = Ty;

        /**
         * \lang english
         * @brief Pointer type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理值的指针类型。
         */
        using pointer = Ty *;

        /**
         * \lang english
         * @brief Const pointer type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理值的常量指针类型。
         */
        using const_pointer = const Ty *;

        /**
         * \lang english
         * @brief The allocator type.
         *
         * \lang simp-chinese
         * @brief 分配器类型。
         */
        using allocator_type = Alloc;

        /**
         * \lang english
         * @brief Reference type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理值的引用类型。
         */
        using reference = value_type &;

        /**
         * \lang english
         * @brief Const reference type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理值的常量引用类型。
         */
        using const_reference = const value_type &;

        static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be a object");
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");
        static_assert(!(type_traits::properties::is_const_v<type_traits::modifers::remove_reference_t<Ty>> ||
                        type_traits::properties::is_volatile_v<type_traits::modifers::remove_reference_t<Ty>>),
                      "Ty cannot be a const/volatile type");
        static_assert(!type_traits::type_relations::is_same_v<Ty, std::in_place_t>, "Ty cannot be std::in_place_t");
        static_assert(!type_traits::primary_types::is_specialization_v<Ty, std::in_place_type_t>,
                      "Ty cannot be a specialization of std::in_place_type_t");

        /**
         * \lang english
         * @brief Default constructor. Constructs a Ty object in-place.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。就地构造一个 Ty 对象。
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 indirect() noexcept(type_traits::properties::is_nothrow_default_constructible_v<Ty>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Constructs an empty indirect without constructing the managed object.
         *
         * \lang simp-chinese
         * @brief 构造一个空的 indirect，不构造被管理对象。
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 indirect(defered_init_t) noexcept : pair(allocator_type{}, nullptr) { // NOLINT
        }

        /**
         * \lang english
         * @brief Constructs the managed object in-place from the given arguments.
         *
         * @tparam Args Types of the arguments
         * @param tag std::in_place_t tag for disambiguation
         * @param args Arguments used to construct the managed object
         *
         * \lang simp-chinese
         * @brief 从给定参数就地构造被管理对象。
         *
         * @tparam Args 参数的类型
         * @param tag 用于消歧的 std::in_place_t 标签
         * @param args 用于构造被管理对象的参数
         */
        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...>, int> = 0>
        RAINY_CONSTEXPR20 indirect(std::in_place_t, // NOLINT
                                   Args &&...args) noexcept(type_traits::properties::is_nothrow_default_constructible_v<Ty>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Constructs the managed object in-place from an initializer list and arguments.
         *
         * @tparam Elem Type of the initializer list elements
         * @tparam Args Types of the arguments
         * @param tag std::in_place_t tag for disambiguation
         * @param ilist The initializer list used to construct the managed object
         * @param args Arguments used to construct the managed object
         *
         * \lang simp-chinese
         * @brief 从初始化列表和参数就地构造被管理对象。
         *
         * @tparam Elem 初始化列表元素的类型
         * @tparam Args 参数的类型
         * @param tag 用于消歧的 std::in_place_t 标签
         * @param ilist 用于构造被管理对象的初始化列表
         * @param args 用于构造被管理对象的参数
         */
        template <typename Elem, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, std::initializer_list<Elem> &, Args...>, int> = 0>
        RAINY_CONSTEXPR20 indirect(std::in_place_t, std::initializer_list<Elem> ilist, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &, Args...>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, ilist, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Allocator-extended default constructor.
         *
         * @param tag allocator_arg_t tag for disambiguation
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 分配器扩展的默认构造函数。
         *
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param allocator 要使用的分配器
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Ty> &&
                                                            type_traits::properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(std::allocator_arg_t, const allocator_type &allocator) noexcept(
            type_traits::properties::is_nothrow_default_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Allocator-extended constructor that produces an empty indirect.
         *
         * @param tag defered_init_t tag for disambiguation
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 分配器扩展的、产生空 indirect 的构造函数。
         *
         * @param tag 用于消歧的 defered_init_t 标签
         * @param allocator 要使用的分配器
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Ty> &&
                                                            type_traits::properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(defered_init_t, const allocator_type &allocator) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
        }

        /**
         * \lang english
         * @brief Allocator-extended constructor that constructs the managed object in-place.
         *
         * @tparam Args Types of the arguments
         * @param tag allocator_arg_t tag for disambiguation
         * @param allocator The allocator to use
         * @param args Arguments used to construct the managed object
         *
         * \lang simp-chinese
         * @brief 分配器扩展的、就地构造被管理对象的构造函数。
         *
         * @tparam Args 参数的类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param allocator 要使用的分配器
         * @param args 用于构造被管理对象的参数
         */
        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...> &&
                                                            type_traits::properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(std::in_place_t, const allocator_type &allocator, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_default_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param right The indirect to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param right 要拷贝的 indirect
         */
        RAINY_CONSTEXPR20 indirect(const indirect &right) noexcept(type_traits::properties::is_nothrow_copy_constructible_v<Ty>) :
            pair(memory::allocator_traits<allocator_type>::select_on_container_copy_construction(right.pair.get_first()), nullptr) {
            if (right.pair.get_second() != nullptr) {
                pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
                try {
                    memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, *right.pair.get_second());
                    pair.get_second() = ptr;
                } catch (...) {
                    memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                    throw;
                }
            }
        }

        /**
         * \lang english
         * @brief Allocator-extended constructor that constructs the managed object in-place from an initializer list.
         *
         * @tparam Elem Type of the initializer list elements
         * @tparam Args Types of the arguments
         * @param tag allocator_arg_t tag for disambiguation
         * @param allocator The allocator to use
         * @param in_place std::in_place_t tag for disambiguation
         * @param ilist The initializer list used to construct the managed object
         * @param args Arguments used to construct the managed object
         *
         * \lang simp-chinese
         * @brief 分配器扩展的、从初始化列表就地构造被管理对象的构造函数。
         *
         * @tparam Elem 初始化列表元素的类型
         * @tparam Args 参数的类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param allocator 要使用的分配器
         * @param in_place 用于消歧的 std::in_place_t 标签
         * @param ilist 用于构造被管理对象的初始化列表
         * @param args 用于构造被管理对象的参数
         */
        template <typename Elem, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, std::initializer_list<Elem> &, Args...> &&
                          type_traits::properties::is_copy_constructible_v<allocator_type>,
                      int> = 0>
        RAINY_CONSTEXPR20 indirect(
            std::allocator_arg_t, const allocator_type &allocator, std::in_place_t, std::initializer_list<Elem> ilist,
            Args &&...args) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &,
                                                                                              Args...> &&
                                     type_traits::properties::is_nothrow_copy_constructible_v<allocator_type>) :
            pair(allocator, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, ilist, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Converting constructor from a value of a type constructible to Ty.
         *
         * @tparam U Type of the value to convert from
         * @param u The value used to construct the managed object
         *
         * \lang simp-chinese
         * @brief 从可转换为 Ty 类型的值进行转换构造。
         *
         * @tparam U 要转换的值类型
         * @param u 用于构造被管理对象的值
         */
        template <typename U = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, U &&> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, indirect> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, std::in_place_t>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit indirect(U &&u) noexcept( // NOLINT
            type_traits::properties::is_nothrow_constructible_v<Ty, U &&>) : pair(allocator_type{}, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<U>(u));
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                if constexpr (!type_traits::properties::is_nothrow_constructible_v<Ty, U &&>) {
                    throw;
                }
            }
        }

        /**
         * \lang english
         * @brief Allocator-extended converting constructor from a value of a type constructible to Ty.
         *
         * @tparam U Type of the value to convert from
         * @param tag allocator_arg_t tag for disambiguation
         * @param allocator The allocator to use
         * @param u The value used to construct the managed object
         *
         * \lang simp-chinese
         * @brief 分配器扩展的、从可转换为 Ty 类型的值进行转换构造的构造函数。
         *
         * @tparam U 要转换的值类型
         * @param tag 用于消歧的 allocator_arg_t 标签
         * @param allocator 要使用的分配器
         * @param u 用于构造被管理对象的值
         */
        template <typename U = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, U &&> &&
                          type_traits::properties::is_copy_constructible_v<allocator_type> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, indirect> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, std::in_place_t>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit indirect(std::allocator_arg_t, const allocator_type &allocator, U &&u) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, U &&> &&
            type_traits::properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<U>(u));
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param right The indirect to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param right 要移动的 indirect
         */
        RAINY_CONSTEXPR20 indirect(indirect &&right) noexcept : pair(utility::move(right.pair.get_first()), right.pair.get_second()) {
            right.pair.get_second() = nullptr;
        }

        /**
         * \lang english
         * @brief Converting copy constructor from another indirect.
         *
         * @tparam Uy Type of the value in the source indirect
         * @tparam UAlloc Allocator type of the source indirect
         * @param right The indirect to copy from
         *
         * \lang simp-chinese
         * @brief 从另一个 indirect 进行转换拷贝构造。
         *
         * @tparam Uy 源 indirect 中的值类型
         * @tparam UAlloc 源 indirect 的分配器类型
         * @param right 要拷贝的 indirect
         */
        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, const Uy &> &&
                                                            type_traits::type_relations::is_convertible_v<const Uy &, Ty>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(const indirect<Uy, UAlloc> &right) noexcept( // NOLINT
            type_traits::properties::is_nothrow_constructible_v<Ty, const Uy &>) : pair(allocator_type{}, nullptr) {
            if (!right.empty()) {
                pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    memory::allocator_traits<allocator_type>::construct(pair.first(), ptr, *right);
                    pair.get_second() = ptr;
                } catch (...) {
                    memory::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
        }

        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Uy &&> &&
                                                            type_traits::type_relations::is_convertible_v<Uy &&, Ty>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(indirect<Uy, UAlloc> &&right) noexcept( // NOLINT
            type_traits::properties::is_nothrow_constructible_v<Ty, Uy &&>) : pair(allocator_type{}, nullptr) {
            if (!right.empty()) {
                pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    memory::allocator_traits<allocator_type>::construct(pair.first(), ptr, utility::move(*right));
                    pair.get_second() = ptr;
                    right.reset();
                } catch (...) {
                    memory::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the managed object and deallocates its storage.
         *
         * \lang simp-chinese
         * @brief 析构函数。销毁被管理对象并释放其存储。
         */
        RAINY_CONSTEXPR20 ~indirect() noexcept {
            reset();
        }

        RAINY_CONSTEXPR20 indirect &operator=(const indirect &right) noexcept(
            type_traits::properties::is_nothrow_copy_assignable_v<Ty>) {
            if (this != &right) {
                if (right.pair.get_second() == nullptr) {
                    reset();
                } else if (pair.get_second() != nullptr) {
                    *pair.get_second() = *right.pair.get_second();
                } else {
                    pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
                    try {
                        memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, *right.pair.get_second());
                        pair.get_second() = ptr;
                    } catch (...) {
                        memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                        throw;
                    }
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right The indirect to move from
         * @return Reference to this indirect
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 要移动的 indirect
         * @return 此 indirect 的引用
         */
        RAINY_CONSTEXPR20 indirect &operator=(indirect &&right) noexcept {
            if (this != &right) {
                reset();
                pair.get_first() = utility::move(right.pair.get_first());
                pair.get_second() = right.pair.get_second();
                right.pair.get_second() = nullptr;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns a value to the managed object.
         *
         * @tparam U Type of the value to assign
         * @param u The value to assign
         * @return Reference to this indirect
         *
         * \lang simp-chinese
         * @brief 向被管理对象赋值。
         *
         * @tparam U 要赋值的类型
         * @param u 要赋的值
         * @return 此 indirect 的引用
         */
        template <typename U = Ty, type_traits::other_trans::enable_if_t<
                                       type_traits::properties::is_assignable_v<Ty &, U &&> &&
                                           !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, indirect>,
                                       int> = 0>
        RAINY_CONSTEXPR20 indirect &operator=(U &&u) noexcept(type_traits::properties::is_nothrow_assignable_v<Ty &, U &&>) {
            if (pair.get_second() != nullptr) {
                *pair.get_second() = utility::forward<U>(u);
            } else {
                pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
                try {
                    memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<U>(u));
                    pair.get_second() = ptr;
                } catch (...) {
                    memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                    throw;
                }
            }
            return *this;
        }

        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_assignable_v<Ty &, const Uy &>, int> = 0>
        RAINY_CONSTEXPR20 indirect &operator=(const indirect<Uy, UAlloc> &right) noexcept(
            type_traits::properties::is_nothrow_assignable_v<Ty &, const Uy &>) {
            if (right.empty()) {
                reset();
            } else if (pair.get_second() != nullptr) {
                *pair.get_second() = *right;
            } else {
                pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    memory::allocator_traits<allocator_type>::construct(pair.first(), ptr, *right);
                    pair.get_second() = ptr;
                } catch (...) {
                    memory::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Converting move assignment from another indirect.
         *
         * @tparam Uy Type of the value in the source indirect
         * @tparam UAlloc Allocator type of the source indirect
         * @param right The indirect to move from
         * @return Reference to this indirect
         *
         * \lang simp-chinese
         * @brief 从另一个 indirect 进行转换移动赋值。
         *
         * @tparam Uy 源 indirect 中的值类型
         * @tparam UAlloc 源 indirect 的分配器类型
         * @param right 要移动的 indirect
         * @return 此 indirect 的引用
         */
        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_assignable_v<Ty &, Uy &&>, int> = 0>
        RAINY_CONSTEXPR20 indirect &operator=(indirect<Uy, UAlloc> &&right) noexcept(
            type_traits::properties::is_nothrow_assignable_v<Ty &, Uy &&>) {
            if (right.empty()) {
                reset();
            } else if (pair.get_second() != nullptr) {
                *pair.get_second() = utility::move(*right);
                right.reset();
            } else {
                pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    memory::allocator_traits<allocator_type>::construct(pair.first(), ptr, utility::move(*right));
                    pair.get_second() = ptr;
                    right.reset();
                } catch (...) {
                    memory::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
            return *this;
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...>, int> = 0>
        RAINY_CONSTEXPR20 reference
        emplace(Args &&...args) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) {
            reset();
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
            return *pair.get_second();
        }

        template <typename Elem, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<Ty, std::initializer_list<Elem> &, Args...>, int> = 0>
        RAINY_CONSTEXPR20 reference emplace(std::initializer_list<Elem> ilist, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &, Args...>) {
            reset();
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, ilist, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
            return *pair.get_second();
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<Ty, Args...> &&
                                                            type_traits::properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 reference emplace(const allocator_type &allocator, Args &&...args) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, Args...> &&
            type_traits::properties::is_nothrow_copy_constructible_v<allocator_type>) {
            reset();
            pair.get_first() = allocator;
            pointer ptr = memory::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                memory::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
            return *pair.get_second();
        }

        /**
         * \lang english
         * @brief Destroys the managed object and sets the indirect to the empty state.
         *
         * \lang simp-chinese
         * @brief 销毁被管理对象并将 indirect 置为空状态。
         */
        RAINY_CONSTEXPR20 void reset() noexcept {
            if (pair.get_second() != nullptr) {
                memory::allocator_traits<allocator_type>::destroy(pair.get_first(), pair.get_second());
                memory::allocator_traits<allocator_type>::deallocate(pair.get_first(), pair.get_second(), 1);
                pair.get_second() = nullptr;
            }
        }

        /**
         * \lang english
         * @brief Accesses the managed object.
         *
         * @return Pointer to the managed object
         *
         * \lang simp-chinese
         * @brief 访问被管理对象。
         *
         * @return 指向被管理对象的指针
         */
        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            return pair.get_second();
        }

        /**
         * \lang english
         * @brief Accesses the managed object.
         *
         * @return Const pointer to the managed object
         *
         * \lang simp-chinese
         * @brief 访问被管理对象。
         *
         * @return 指向被管理对象的常量指针
         */
        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            return pair.get_second();
        }

        /**
         * \lang english
         * @brief Dereferences the managed object.
         *
         * @return Reference to the managed object
         *
         * \lang simp-chinese
         * @brief 解引用被管理对象。
         *
         * @return 被管理对象的引用
         */
        RAINY_CONSTEXPR20 reference operator*() noexcept {
            return *pair.get_second();
        }

        /**
         * \lang english
         * @brief Dereferences the managed object.
         *
         * @return Const reference to the managed object
         *
         * \lang simp-chinese
         * @brief 解引用被管理对象。
         *
         * @return 被管理对象的常量引用
         */
        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            return *pair.get_second();
        }

        /**
         * \lang english
         * @brief Checks whether the indirect is in the empty state.
         *
         * @return true if no object is managed, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查 indirect 是否处于空状态。
         *
         * @return 如果未管理任何对象则为true，否则为false
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 bool empty() const noexcept {
            return pair.get_second() == nullptr;
        }

        /**
         * \lang english
         * @brief Returns the allocator associated with the indirect.
         *
         * @return The associated allocator
         *
         * \lang simp-chinese
         * @brief 返回与 indirect 关联的分配器。
         *
         * @return 关联的分配器
         */
        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return pair.get_first();
        }

        /**
         * \lang english
         * @brief Returns a pointer to the managed object.
         *
         * @return Pointer to the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向被管理对象的指针。
         *
         * @return 指向被管理对象的指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 pointer get() noexcept {
            return pair.get_second();
        }

        /**
         * \lang english
         * @brief Returns a const pointer to the managed object.
         *
         * @return Const pointer to the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向被管理对象的常量指针。
         *
         * @return 指向被管理对象的常量指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 const_pointer get() const noexcept {
            return pair.get_second();
        }

        /**
         * \lang english
         * @brief Returns an iterator to the beginning of the single-element range.
         *
         * @return Pointer to the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向单元素范围开头的迭代器。
         *
         * @return 指向被管理对象的指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 pointer begin() noexcept {
            return empty() ? nullptr : get();
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning of the single-element range.
         *
         * @return Const pointer to the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向单元素范围开头的常量迭代器。
         *
         * @return 指向被管理对象的常量指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 const_pointer begin() const noexcept {
            return empty() ? nullptr : get();
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning of the single-element range.
         *
         * @return Const pointer to the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向单元素范围开头的常量迭代器。
         *
         * @return 指向被管理对象的常量指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 const_pointer cbegin() const noexcept {
            return empty() ? nullptr : get();
        }

        /**
         * \lang english
         * @brief Returns an iterator to the end of the single-element range.
         *
         * @return Pointer one past the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向单元素范围末尾的迭代器。
         *
         * @return 指向被管理对象之后位置的指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 pointer end() noexcept {
            return empty() ? nullptr : get() + 1;
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the end of the single-element range.
         *
         * @return Const pointer one past the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向单元素范围末尾的常量迭代器。
         *
         * @return 指向被管理对象之后位置的常量指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 const_pointer end() const noexcept {
            return empty() ? nullptr : get() + 1;
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the end of the single-element range.
         *
         * @return Const pointer one past the managed object, or nullptr if empty
         *
         * \lang simp-chinese
         * @brief 返回指向单元素范围末尾的常量迭代器。
         *
         * @return 指向被管理对象之后位置的常量指针，如果为空则为nullptr
         */
        RAINY_CONSTEXPR20 const_pointer cend() const noexcept {
            return empty() ? nullptr : get() + 1;
        }

        /**
         * \lang english
         * @brief Swaps the contents with another indirect.
         *
         * @param right The indirect to swap with
         *
         * \lang simp-chinese
         * @brief 与另一个 indirect 交换内容。
         *
         * @param right 要交换的 indirect
         */
        RAINY_CONSTEXPR20 void swap(indirect &right) noexcept {
            if (this != &right) {
                pointer temp_ptr = pair.get_second();
                pair.get_second() = right.pair.get_second();
                right.pair.get_second() = temp_ptr;
                allocator_type temp_alloc = utility::move(pair.get_first());
                pair.get_first() = utility::move(right.pair.get_first());
                right.pair.get_first() = utility::move(temp_alloc);
            }
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool valueless_after_move() const noexcept {
            return empty();
        }

    private:
        compressed_pair<Alloc, Ty *> pair;
    };

    /**
     * \lang english
     * @brief Deduction guide that deduces indirect from a value.
     *
     * @tparam Value The type of the value
     *
     * \lang simp-chinese
     * @brief 从值推导 indirect 类型的推导指引。
     *
     * @tparam Value 值的类型
     */
    template <typename Value>
    indirect(Value) -> indirect<Value>;

    /**
     * \lang english
     * @brief Deduction guide that deduces indirect from a value and an allocator.
     *
     * @tparam Allocator The allocator type
     * @tparam Value The type of the value
     *
     * \lang simp-chinese
     * @brief 从值和分配器推导 indirect 类型的推导指引。
     *
     * @tparam Allocator 分配器类型
     * @tparam Value 值的类型
     */
    template <typename Allocator, typename Value>
    indirect(std::allocator_arg_t, Allocator, Value)
        -> indirect<Value, typename memory::allocator_traits<Allocator>::template rebind_alloc<Value>>;
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Equality comparison between two indirect objects.
     *
     * @tparam Ty Type of the left managed value
     * @tparam Alloc Allocator type of the left indirect
     * @tparam Uy Type of the right managed value
     * @tparam UAlloc Allocator type of the right indirect
     * @param left The left indirect
     * @param right The right indirect
     * @return true if both are empty or their values are equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 indirect 对象之间的相等比较。
     *
     * @tparam Ty 左被管理值类型
     * @tparam Alloc 左 indirect 的分配器类型
     * @tparam Uy 右被管理值类型
     * @tparam UAlloc 右 indirect 的分配器类型
     * @param left 左 indirect
     * @param right 右 indirect
     * @return 如果两者都为空或它们的值相等则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator==(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(*left == *right)) {
        if (left.empty() != right.empty()) {
            return false;
        }
        if (left.empty()) {
            return true;
        }
        return *left == *right;
    }

    /**
     * \lang english
     * @brief Inequality comparison between two indirect objects.
     *
     * @tparam Ty Type of the left managed value
     * @tparam Alloc Allocator type of the left indirect
     * @tparam Uy Type of the right managed value
     * @tparam UAlloc Allocator type of the right indirect
     * @param left The left indirect
     * @param right The right indirect
     * @return true if the indirect objects are not equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 indirect 对象之间的不相等比较。
     *
     * @tparam Ty 左被管理值类型
     * @tparam Alloc 左 indirect 的分配器类型
     * @tparam Uy 右被管理值类型
     * @tparam UAlloc 右 indirect 的分配器类型
     * @param left 左 indirect
     * @param right 右 indirect
     * @return 如果两个 indirect 对象不相等则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator!=(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Less-than comparison between two indirect objects.
     *
     * @tparam Ty Type of the left managed value
     * @tparam Alloc Allocator type of the left indirect
     * @tparam Uy Type of the right managed value
     * @tparam UAlloc Allocator type of the right indirect
     * @param left The left indirect
     * @param right The right indirect
     * @return true if left is empty or its value is less than the value of right, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 indirect 对象之间的小于比较。
     *
     * @tparam Ty 左被管理值类型
     * @tparam Alloc 左 indirect 的分配器类型
     * @tparam Uy 右被管理值类型
     * @tparam UAlloc 右 indirect 的分配器类型
     * @param left 左 indirect
     * @param right 右 indirect
     * @return 如果 left 为空或其值小于 right 的值则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator<(const indirect<Ty, Alloc> &left,
                                     const indirect<Uy, UAlloc> &right) noexcept(noexcept(*left < *right)) {
        if (right.empty()) {
            return false;
        }
        if (left.empty()) {
            return true;
        }
        return *left < *right;
    }

    /**
     * \lang english
     * @brief Less-than-or-equal comparison between two indirect objects.
     *
     * @tparam Ty Type of the left managed value
     * @tparam Alloc Allocator type of the left indirect
     * @tparam Uy Type of the right managed value
     * @tparam UAlloc Allocator type of the right indirect
     * @param left The left indirect
     * @param right The right indirect
     * @return true if left is not greater than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 indirect 对象之间的小于等于比较。
     *
     * @tparam Ty 左被管理值类型
     * @tparam Alloc 左 indirect 的分配器类型
     * @tparam Uy 右被管理值类型
     * @tparam UAlloc 右 indirect 的分配器类型
     * @param left 左 indirect
     * @param right 右 indirect
     * @return 如果 left 不大于 right 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator<=(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Greater-than comparison between two indirect objects.
     *
     * @tparam Ty Type of the left managed value
     * @tparam Alloc Allocator type of the left indirect
     * @tparam Uy Type of the right managed value
     * @tparam UAlloc Allocator type of the right indirect
     * @param left The left indirect
     * @param right The right indirect
     * @return true if right is less than left, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 indirect 对象之间的大于比较。
     *
     * @tparam Ty 左被管理值类型
     * @tparam Alloc 左 indirect 的分配器类型
     * @tparam Uy 右被管理值类型
     * @tparam UAlloc 右 indirect 的分配器类型
     * @param left 左 indirect
     * @param right 右 indirect
     * @return 如果 right 小于 left 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator>(const indirect<Ty, Alloc> &left,
                                     const indirect<Uy, UAlloc> &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Greater-than-or-equal comparison between two indirect objects.
     *
     * @tparam Ty Type of the left managed value
     * @tparam Alloc Allocator type of the left indirect
     * @tparam Uy Type of the right managed value
     * @tparam UAlloc Allocator type of the right indirect
     * @param left The left indirect
     * @param right The right indirect
     * @return true if left is not less than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 两个 indirect 对象之间的大于等于比较。
     *
     * @tparam Ty 左被管理值类型
     * @tparam Alloc 左 indirect 的分配器类型
     * @tparam Uy 右被管理值类型
     * @tparam UAlloc 右 indirect 的分配器类型
     * @param left 左 indirect
     * @param right 右 indirect
     * @return 如果 left 不小于 right 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator>=(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Compares an indirect with nullptr for equality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The indirect
     * @param right nullptr
     * @return true if the indirect is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 将 indirect 与 nullptr 进行相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left indirect
     * @param right nullptr
     * @return 如果 indirect 为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator==(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return left.empty();
    }

    /**
     * \lang english
     * @brief Compares nullptr with an indirect for equality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left nullptr
     * @param right The indirect
     * @return true if the indirect is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 将 nullptr 与 indirect 进行相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left nullptr
     * @param right indirect
     * @return 如果 indirect 为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator==(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return right.empty();
    }

    /**
     * \lang english
     * @brief Compares an indirect with nullptr for inequality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The indirect
     * @param right nullptr
     * @return true if the indirect is not empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 将 indirect 与 nullptr 进行不相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left indirect
     * @param right nullptr
     * @return 如果 indirect 不为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator!=(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return !left.empty();
    }

    /**
     * \lang english
     * @brief Compares nullptr with an indirect for inequality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left nullptr
     * @param right The indirect
     * @return true if the indirect is not empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 将 nullptr 与 indirect 进行不相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left nullptr
     * @param right indirect
     * @return 如果 indirect 不为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator!=(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return !right.empty();
    }

    /**
     * \lang english
     * @brief Compares an indirect with nullptr using less-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The indirect
     * @param right nullptr
     * @return false (an empty indirect is never less than nullptr)
     *
     * \lang simp-chinese
     * @brief 使用小于比较 indirect 与 nullptr。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left indirect
     * @param right nullptr
     * @return false（空的 indirect 永远不小于 nullptr）
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<(const indirect<Ty, Alloc> &, std::nullptr_t) noexcept {
        return false;
    }

    /**
     * \lang english
     * @brief Compares nullptr with an indirect using less-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left nullptr
     * @param right The indirect
     * @return true if the indirect is not empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用小于比较 nullptr 与 indirect。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left nullptr
     * @param right indirect
     * @return 如果 indirect 不为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return !right.empty();
    }

    /**
     * \lang english
     * @brief Compares an indirect with nullptr using less-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The indirect
     * @param right nullptr
     * @return true if the indirect is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用小于等于比较 indirect 与 nullptr。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left indirect
     * @param right nullptr
     * @return 如果 indirect 为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<=(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return left.empty();
    }

    /**
     * \lang english
     * @brief Compares nullptr with an indirect using less-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left nullptr
     * @param right The indirect
     * @return true
     *
     * \lang simp-chinese
     * @brief 使用小于等于比较 nullptr 与 indirect。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left nullptr
     * @param right indirect
     * @return true
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<=(std::nullptr_t, const indirect<Ty, Alloc> &) noexcept {
        return true;
    }

    /**
     * \lang english
     * @brief Compares an indirect with nullptr using greater-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The indirect
     * @param right nullptr
     * @return true if the indirect is not empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用大于比较 indirect 与 nullptr。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left indirect
     * @param right nullptr
     * @return 如果 indirect 不为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return !left.empty();
    }

    /**
     * \lang english
     * @brief Compares nullptr with an indirect using greater-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left nullptr
     * @param right The indirect
     * @return false (nullptr is never greater than an indirect)
     *
     * \lang simp-chinese
     * @brief 使用大于比较 nullptr 与 indirect。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left nullptr
     * @param right indirect
     * @return false（nullptr 永远不会大于 indirect）
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>(std::nullptr_t, const indirect<Ty, Alloc> &) noexcept {
        return false;
    }

    /**
     * \lang english
     * @brief Compares an indirect with nullptr using greater-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The indirect
     * @param right nullptr
     * @return true
     *
     * \lang simp-chinese
     * @brief 使用大于等于比较 indirect 与 nullptr。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left indirect
     * @param right nullptr
     * @return true
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>=(const indirect<Ty, Alloc> &, std::nullptr_t) noexcept {
        return true;
    }

    /**
     * \lang english
     * @brief Compares nullptr with an indirect using greater-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left nullptr
     * @param right The indirect
     * @return true if the indirect is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用大于等于比较 nullptr 与 indirect。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left nullptr
     * @param right indirect
     * @return 如果 indirect 为空则为true，否则为false
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>=(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return right.empty();
    }

    /**
     * \lang english
     * @brief Compares the value of an indirect with a value for equality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The indirect
     * @param right The value
     * @return true if the indirect is not empty and its value equals right, false otherwise
     *
     * \lang simp-chinese
     * @brief 将 indirect 的值与一个值进行相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left indirect
     * @param right 值
     * @return 如果 indirect 不为空且其值等于 right 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator==(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(*left == right)) {
        return !left.empty() && *left == right;
    }

    /**
     * \lang english
     * @brief Compares a value with the value of an indirect for equality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The value
     * @param right The indirect
     * @return true if the indirect is not empty and left equals its value, false otherwise
     *
     * \lang simp-chinese
     * @brief 将一个值与 indirect 的值进行相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left 值
     * @param right indirect
     * @return 如果 indirect 不为空且 left 等于其值则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator==(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left == *right)) {
        return !right.empty() && left == *right;
    }

    /**
     * \lang english
     * @brief Compares the value of an indirect with a value for inequality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The indirect
     * @param right The value
     * @return true if the comparison for equality is false, false otherwise
     *
     * \lang simp-chinese
     * @brief 将 indirect 的值与一个值进行不相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left indirect
     * @param right 值
     * @return 如果相等比较为false则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator!=(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Compares a value with the value of an indirect for inequality.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The value
     * @param right The indirect
     * @return true if the comparison for equality is false, false otherwise
     *
     * \lang simp-chinese
     * @brief 将一个值与 indirect 的值进行不相等比较。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left 值
     * @param right indirect
     * @return 如果相等比较为false则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator!=(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Compares the value of an indirect with a value using less-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The indirect
     * @param right The value
     * @return true if the indirect is empty or its value is less than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用小于比较 indirect 的值与一个值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left indirect
     * @param right 值
     * @return 如果 indirect 为空或其值小于 right 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(*left < right)) {
        return left.empty() || *left < right;
    }

    /**
     * \lang english
     * @brief Compares a value with the value of an indirect using less-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The value
     * @param right The indirect
     * @return true if the indirect is not empty and left is less than its value, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用小于比较一个值与 indirect 的值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left 值
     * @param right indirect
     * @return 如果 indirect 不为空且 left 小于其值则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left < *right)) {
        return !right.empty() && left < *right;
    }

    /**
     * \lang english
     * @brief Compares the value of an indirect with a value using less-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The indirect
     * @param right The value
     * @return true if right is not less than left, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用小于等于比较 indirect 的值与一个值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left indirect
     * @param right 值
     * @return 如果 right 不小于 left 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<=(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares a value with the value of an indirect using less-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The value
     * @param right The indirect
     * @return true if right is not less than left, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用小于等于比较一个值与 indirect 的值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left 值
     * @param right indirect
     * @return 如果 right 不小于 left 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<=(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares the value of an indirect with a value using greater-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The indirect
     * @param right The value
     * @return true if right is less than left, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用大于比较 indirect 的值与一个值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left indirect
     * @param right 值
     * @return 如果 right 小于 left 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares a value with the value of an indirect using greater-than.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The value
     * @param right The indirect
     * @return true if right is less than left, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用大于比较一个值与 indirect 的值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left 值
     * @param right indirect
     * @return 如果 right 小于 left 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares the value of an indirect with a value using greater-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The indirect
     * @param right The value
     * @return true if left is not less than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用大于等于比较 indirect 的值与一个值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left indirect
     * @param right 值
     * @return 如果 left 不小于 right 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>=(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Compares a value with the value of an indirect using greater-than-or-equal.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @tparam Uy Type of the value
     * @param left The value
     * @param right The indirect
     * @return true if left is not less than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 使用大于等于比较一个值与 indirect 的值。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Uy 值的类型
     * @param left 值
     * @param right indirect
     * @return 如果 left 不小于 right 则为true，否则为false
     */
    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>=(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Swaps two indirect objects.
     *
     * @tparam Ty Type of the managed value
     * @tparam Alloc Allocator type of the indirect
     * @param left The first indirect
     * @param right The second indirect
     *
     * \lang simp-chinese
     * @brief 交换两个 indirect 对象。
     *
     * @tparam Ty 被管理值类型
     * @tparam Alloc indirect 的分配器类型
     * @param left 第一个 indirect
     * @param right 第二个 indirect
     */
    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 void swap(indirect<Ty, Alloc> &left, indirect<Ty, Alloc> &right) noexcept {
        left.swap(right);
    }

    /**
     * \lang english
     * @brief Constructs an indirect owning an object constructed in-place from the given arguments.
     *
     * @tparam Ty Type of the managed object
     * @tparam Args Types of the arguments
     * @param args Arguments used to construct the managed object
     * @return An indirect owning the newly constructed object
     *
     * \lang simp-chinese
     * @brief 构造一个拥有从给定参数就地构造的对象的 indirect。
     *
     * @tparam Ty 被管理对象的类型
     * @tparam Args 参数的类型
     * @param args 用于构造被管理对象的参数
     * @return 拥有新构造对象的 indirect
     */
    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 indirect<Ty> make_indirect(Args &&...args) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) {
        return indirect<Ty>(std::in_place, utility::forward<Args>(args)...);
    }

    /**
     * \lang english
     * @brief Constructs an indirect with the given allocator, owning an object constructed in-place.
     *
     * @tparam Ty Type of the managed object
     * @tparam Alloc Allocator type of the indirect
     * @tparam Args Types of the arguments
     * @param alloc The allocator to use
     * @param args Arguments used to construct the managed object
     * @return An indirect owning the newly constructed object
     *
     * \lang simp-chinese
     * @brief 使用给定分配器构造一个拥有就地构造对象的 indirect。
     *
     * @tparam Ty 被管理对象的类型
     * @tparam Alloc indirect 的分配器类型
     * @tparam Args 参数的类型
     * @param alloc 要使用的分配器
     * @param args 用于构造被管理对象的参数
     * @return 拥有新构造对象的 indirect
     */
    template <typename Ty, typename Alloc, typename... Args>
    RAINY_CONSTEXPR20 indirect<Ty, Alloc> make_indirect(const Alloc &alloc, Args &&...args) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty, Args...> &&
        type_traits::properties::is_nothrow_copy_constructible_v<Alloc>) {
        return indirect<Ty, Alloc>(std::in_place, alloc, utility::forward<Args>(args)...);
    }

    /**
     * \lang english
     * @brief Constructs an indirect owning an object constructed in-place from an initializer list and arguments.
     *
     * @tparam Ty Type of the managed object
     * @tparam Elem Type of the initializer list elements
     * @tparam Args Types of the arguments
     * @param ilist The initializer list used to construct the managed object
     * @param args Arguments used to construct the managed object
     * @return An indirect owning the newly constructed object
     *
     * \lang simp-chinese
     * @brief 构造一个拥有从初始化列表和参数就地构造的对象的 indirect。
     *
     * @tparam Ty 被管理对象的类型
     * @tparam Elem 初始化列表元素的类型
     * @tparam Args 参数的类型
     * @param ilist 用于构造被管理对象的初始化列表
     * @param args 用于构造被管理对象的参数
     * @return 拥有新构造对象的 indirect
     */
    template <typename Ty, typename Elem, typename... Args>
    RAINY_CONSTEXPR20 indirect<Ty> make_indirect(std::initializer_list<Elem> ilist, Args &&...args) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &, Args...>) {
        return indirect<Ty>(std::in_place, ilist, utility::forward<Args>(args)...);
    }
}

namespace rainy::container {
    using rainy::core::container::defered_init_t;
    using rainy::core::container::defered_init;
    using rainy::core::container::indirect;
    using rainy::core::container::swap;
    using rainy::core::container::make_indirect;
    using rainy::core::container::operator==;
    using rainy::core::container::operator!=;
    using rainy::core::container::operator<;
    using rainy::core::container::operator<=;
    using rainy::core::container::operator>;
    using rainy::core::container::operator>=;
}


#endif
