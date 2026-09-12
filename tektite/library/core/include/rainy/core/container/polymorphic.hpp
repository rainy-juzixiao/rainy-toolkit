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
#ifndef RAINY_CORE_CONTAINER_POLYMORPHIC_HPP
#define RAINY_CORE_CONTAINER_POLYMORPHIC_HPP
#include <rainy/core/type_traits.hpp>
#include <rainy/core/container/compressed_pair.hpp>
#include <rainy/core/memory/allocator.hpp>

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A polymorphic value wrapper that owns a dynamically allocated object of a type derived from Ty.
     *         Provides value semantics with copy/move support through an internal control block.
     *
     * @tparam Ty The base type of the managed object, must be a polymorphic object type
     * @tparam Alloc The allocator type used for the underlying storage
     *
     * \lang simp-chinese
     * @brief 多态值包装器，拥有一个动态分配的、派生自 Ty 类型的对象。
     *         通过内部控制块提供带拷贝/移动支持的值语义。
     *
     * @tparam Ty 被管理对象的基类型，必须是多态对象类型
     * @tparam Alloc 用于底层存储的分配器类型
     */
    template <typename Ty, typename Alloc = memory::allocator<Ty>>
    class polymorphic {
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
         * @brief The allocator type.
         *
         * \lang simp-chinese
         * @brief 分配器类型。
         */
        using allocator_type = Alloc;

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
         * @brief Reference type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理值的引用类型。
         */
        using reference = Ty&;

        /**
         * \lang english
         * @brief Const reference type of the managed value.
         *
         * \lang simp-chinese
         * @brief 被管理值的常量引用类型。
         */
        using const_reference = const Ty&;

        static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be an object type");
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");
        static_assert(!(type_traits::properties::is_const_v<Ty> || type_traits::properties::is_volatile_v<Ty>),
                      "Ty cannot be const/volatile qualified");
        static_assert(type_traits::properties::is_polymorphic_v<Ty>, "Ty must be a polymorphic type");

        /**
         * \lang english
         * @brief Default constructor. Constructs a Ty object in-place.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。就地构造一个 Ty 对象。
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic() noexcept(type_traits::properties::is_nothrow_default_constructible_v<Ty>) :
            pair(allocator_type{}, nullptr) {
            construct_default();
        }

        /**
         * \lang english
         * @brief Allocator-extended default constructor.
         *
         * @param a The allocator to use
         *
         * \lang simp-chinese
         * @brief 分配器扩展的默认构造函数。
         *
         * @param a 要使用的分配器
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a) noexcept(
            type_traits::properties::is_nothrow_default_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_copy_constructible_v<Alloc>) : pair(a, nullptr) {
            construct_default();
        }

        /**
         * \lang english
         * @brief Copy constructor. Copies the managed object if the source is not empty.
         *
         * @param other The source polymorphic to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。若源对象非空则拷贝被管理的对象。
         *
         * @param other 要拷贝的源 polymorphic
         */
        RAINY_CONSTEXPR20 polymorphic(const polymorphic &other) :
            pair(memory::allocator_traits<allocator_type>::select_on_container_copy_construction(other.pair.get_first()), nullptr) {
            if (other.pair.get_second() != nullptr) {
                copy_from(other);
            }
        }

        /**
         * \lang english
         * @brief Allocator-extended copy constructor.
         *
         * @param a The allocator to use
         * @param other The source polymorphic to copy from
         *
         * \lang simp-chinese
         * @brief 分配器扩展的拷贝构造函数。
         *
         * @param a 要使用的分配器
         * @param other 要拷贝的源 polymorphic
         */
        RAINY_CONSTEXPR20 polymorphic(std::allocator_arg_t, const Alloc &a, const polymorphic &other) : pair(a, nullptr) {
            if (other.pair.get_second() != nullptr) {
                copy_from(other);
            }
        }

        /**
         * \lang english
         * @brief Move constructor. Transfers ownership of the managed object.
         *
         * @param other The source polymorphic to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。转移被管理对象的所有权。
         *
         * @param other 要移动的源 polymorphic
         */
        RAINY_CONSTEXPR20 polymorphic(polymorphic &&other) noexcept :
            pair(utility::move(other.pair.get_first()), other.pair.get_second()) {
            other.pair.get_second() = nullptr;
        }

        /**
         * \lang english
         * @brief Allocator-extended move constructor.
         *
         * @param a The allocator to use
         * @param other The source polymorphic to move from
         *
         * \lang simp-chinese
         * @brief 分配器扩展的移动构造函数。
         *
         * @param a 要使用的分配器
         * @param other 要移动的源 polymorphic
         */
        RAINY_CONSTEXPR20 polymorphic(std::allocator_arg_t, const Alloc &a,
                                      polymorphic &&other) noexcept(memory::pointer_traits<Alloc>::is_always_equal::value) :
            pair(a, nullptr) {
            if (pair.get_first() == other.pair.get_first()) {
                pair.get_second() = other.pair.get_second();
                other.pair.get_second() = nullptr;
            } else if (other.pair.get_second() != nullptr) {
                copy_from(other);
                other.reset();
            }
        }

        /**
         * \lang english
         * @brief Constructs the polymorphic from a value of a type derived from Ty.
         *
         * @tparam UTy The actual type of the object, deduced from the argument
         * @param object The value used to initialize the managed object
         *
         * \lang simp-chinese
         * @brief 从派生自 Ty 类型的值构造 polymorphic。
         *
         * @tparam UTy 对象的实际类型，由实参推导
         * @param object 用于初始化被管理对象的值
         */
        template <typename UTy = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, type_traits::other_trans::decay_t<UTy>> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UTy>, polymorphic>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(UTy &&object) : pair(allocator_type{}, nullptr) { // NOLINT
            using decay_type = type_traits::other_trans::decay_t<UTy>;
            construct_from_value<decay_type>(utility::forward<UTy>(object));
        }

        /**
         * \lang english
         * @brief Allocator-extended constructor from a value of a type derived from Ty.
         *
         * @tparam UTy The actual type of the object, deduced from the argument
         * @param a The allocator to use
         * @param object The value used to initialize the managed object
         *
         * \lang simp-chinese
         * @brief 从派生自 Ty 类型的值构造的分配器扩展构造函数。
         *
         * @tparam UTy 对象的实际类型，由实参推导
         * @param a 要使用的分配器
         * @param object 用于初始化被管理对象的值
         */
        template <typename UTy = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, type_traits::other_trans::decay_t<UTy>> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UTy>, polymorphic>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a, UTy &&object) : pair(a, nullptr) {
            using decay_type = type_traits::other_trans::decay_t<UTy>;
            construct_from_value<decay_type>(utility::forward<UTy>(object));
        }

        /**
         * \lang english
         * @brief In-place constructor. Constructs the managed object of type UTy from the given arguments.
         *
         * @tparam UTy The type of the managed object to construct
         * @tparam Ts The types of the arguments forwarded to the constructor of UTy
         * @param ts The arguments forwarded to the constructor of UTy
         *
         * \lang simp-chinese
         * @brief 就地构造函数。使用给定实参构造类型为 UTy 的被管理对象。
         *
         * @tparam UTy 要构造的被管理对象的类型
         * @tparam Ts 转发给 UTy 构造函数的实参类型
         * @param ts 转发给 UTy 构造函数的实参
         */
        template <typename UTy, typename... Ts,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                                                            type_traits::properties::is_constructible_v<UTy, Ts...>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::in_place_type_t<UTy>, Ts &&...ts) : pair(allocator_type{}, nullptr) {
            construct_inplace<UTy>(utility::forward<Ts>(ts)...);
        }

        /**
         * \lang english
         * @brief Allocator-extended in-place constructor.
         *
         * @tparam UTy The type of the managed object to construct
         * @tparam Ts The types of the arguments forwarded to the constructor of UTy
         * @param a The allocator to use
         * @param ts The arguments forwarded to the constructor of UTy
         *
         * \lang simp-chinese
         * @brief 分配器扩展的就地构造函数。
         *
         * @tparam UTy 要构造的被管理对象的类型
         * @tparam Ts 转发给 UTy 构造函数的实参类型
         * @param a 要使用的分配器
         * @param ts 转发给 UTy 构造函数的实参
         */
        template <typename UTy, typename... Ts,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                                                            type_traits::properties::is_constructible_v<UTy, Ts...>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a, std::in_place_type_t<UTy>, Ts &&...ts) :
            pair(a, nullptr) {
            construct_inplace<UTy>(utility::forward<Ts>(ts)...);
        }

        /**
         * \lang english
         * @brief In-place constructor with an initializer list.
         *
         * @tparam UTy The type of the managed object to construct
         * @tparam I The type of the elements of the initializer list
         * @tparam Us The types of the remaining arguments forwarded to the constructor of UTy
         * @param ilist The initializer list passed to the constructor of UTy
         * @param us The remaining arguments forwarded to the constructor of UTy
         *
         * \lang simp-chinese
         * @brief 带初始化器列表的就地构造函数。
         *
         * @tparam UTy 要构造的被管理对象的类型
         * @tparam I 初始化器列表元素的类型
         * @tparam Us 转发给 UTy 构造函数的其余实参类型
         * @param ilist 传递给 UTy 构造函数的初始化器列表
         * @param us 转发给 UTy 构造函数的其余实参
         */
        template <typename UTy, typename I, typename... Us,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                          type_traits::properties::is_constructible_v<UTy, std::initializer_list<I> &, Us...>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::in_place_type_t<UTy>, std::initializer_list<I> ilist, Us &&...us) :
            pair(allocator_type{}, nullptr) {
            construct_inplace<UTy>(ilist, utility::forward<Us>(us)...);
        }

        /**
         * \lang english
         * @brief Allocator-extended in-place constructor with an initializer list.
         *
         * @tparam UTy The type of the managed object to construct
         * @tparam I The type of the elements of the initializer list
         * @tparam Us The types of the remaining arguments forwarded to the constructor of UTy
         * @param a The allocator to use
         * @param ilist The initializer list passed to the constructor of UTy
         * @param us The remaining arguments forwarded to the constructor of UTy
         *
         * \lang simp-chinese
         * @brief 带初始化器列表的分配器扩展就地构造函数。
         *
         * @tparam UTy 要构造的被管理对象的类型
         * @tparam I 初始化器列表元素的类型
         * @tparam Us 转发给 UTy 构造函数的其余实参类型
         * @param a 要使用的分配器
         * @param ilist 传递给 UTy 构造函数的初始化器列表
         * @param us 转发给 UTy 构造函数的其余实参
         */
        template <typename UTy, typename I, typename... Us,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                          type_traits::properties::is_constructible_v<UTy, std::initializer_list<I> &, Us...>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a, std::in_place_type_t<UTy>,
                                               std::initializer_list<I> ilist, Us &&...us) : pair(a, nullptr) {
            construct_inplace<UTy>(ilist, utility::forward<Us>(us)...);
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the managed object if present.
         *
         * \lang simp-chinese
         * @brief 析构函数。若存在被管理对象则销毁它。
         */
        RAINY_CONSTEXPR20 ~polymorphic() noexcept {
            reset();
        }

        /**
         * \lang english
         * @brief Copy assignment operator. Copies the managed object if the source is not empty.
         *
         * @param other The source polymorphic to copy from
         * @return Reference to this polymorphic
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。若源对象非空则拷贝被管理的对象。
         *
         * @param other 要拷贝的源 polymorphic
         * @return 此 polymorphic 的引用
         */
        RAINY_CONSTEXPR20 polymorphic &operator=(const polymorphic &other) {
            if (this != &other) {
                reset();
                if (other.pair.get_second() != nullptr) {
                    copy_from(other);
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator. Transfers ownership of the managed object.
         *
         * @param other The source polymorphic to move from
         * @return Reference to this polymorphic
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。转移被管理对象的所有权。
         *
         * @param other 要移动的源 polymorphic
         * @return 此 polymorphic 的引用
         */
        RAINY_CONSTEXPR20 polymorphic &operator=(polymorphic &&other) noexcept(memory::allocator_traits<Alloc>::is_always_equal::value) {
            if (this != &other) {
                reset();
                if constexpr (memory::allocator_traits<Alloc>::is_always_equal::value) {
                    pair.get_first() = utility::move(other.pair.get_first());
                    pair.get_second() = other.pair.get_second();
                    other.pair.get_second() = nullptr;
                } else {
                    if (pair.get_first() == other.pair.get_first()) {
                        pair.get_second() = other.pair.get_second();
                        other.pair.get_second() = nullptr;
                    } else if (other.pair.get_second() != nullptr) {
                        copy_from(other);
                        other.reset();
                    }
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Dereferences the managed object.
         *
         * @return A reference to the managed object
         *
         * \lang simp-chinese
         * @brief 解引用被管理对象。
         *
         * @return 被管理对象的引用
         */
        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            auto* block = static_cast<control_block_base*>(pair.get_second());
            return *block->get_value_ptr();
        }

        /**
         * \lang english
         * @brief Dereferences the managed object.
         *
         * @return A reference to the managed object
         *
         * \lang simp-chinese
         * @brief 解引用被管理对象。
         *
         * @return 被管理对象的引用
         */
        RAINY_CONSTEXPR20 reference operator*() noexcept {
            auto* block = static_cast<control_block_base*>(pair.get_second());
            return *block->get_value_ptr();
        }

        /**
         * \lang english
         * @brief Accesses the managed object through the base pointer.
         *
         * @return A pointer to the managed object
         *
         * \lang simp-chinese
         * @brief 通过基类指针访问被管理对象。
         *
         * @return 指向被管理对象的指针
         */
        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            auto* block = static_cast<const control_block_base*>(pair.get_second());
            return static_cast<const_pointer>(block->get_value_ptr());
        }

        /**
         * \lang english
         * @brief Accesses the managed object through the base pointer.
         *
         * @return A pointer to the managed object
         *
         * \lang simp-chinese
         * @brief 通过基类指针访问被管理对象。
         *
         * @return 指向被管理对象的指针
         */
        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            auto* block = static_cast<control_block_base*>(pair.get_second());
            return static_cast<pointer>(block->get_value_ptr());
        }

        /**
         * \lang english
         * @brief Checks whether the polymorphic has no managed object, e.g. after being moved from.
         *
         * @return true if there is no managed object, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查 polymorphic 是否没有被管理对象，例如被移动之后。
         *
         * @return 若没有被管理对象则返回 true，否则返回 false
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 bool valueless_after_move() const noexcept {
            return pair.get_second() == nullptr;
        }

        /**
         * \lang english
         * @brief Returns the allocator associated with this polymorphic.
         *
         * @return The allocator
         *
         * \lang simp-chinese
         * @brief 返回与此 polymorphic 关联的分配器。
         *
         * @return 分配器
         */
        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return pair.get_first();
        }

        /**
         * \lang english
         * @brief Swaps the managed objects and allocators of two polymorphic objects.
         *
         * @param other The polymorphic to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个 polymorphic 对象的被管理对象与分配器。
         *
         * @param other 要与之交换的 polymorphic
         */
        RAINY_CONSTEXPR20 void swap(polymorphic &other) noexcept(memory::allocator_traits<Alloc>::is_always_equal::value) {
            if (this != &other) {
                void *temp_ptr = pair.get_second();
                pair.get_second() = other.pair.get_second();
                other.pair.get_second() = temp_ptr;

                if constexpr (!memory::allocator_traits<Alloc>::is_always_equal::value) {
                    allocator_type temp_alloc = utility::move(pair.get_first());
                    pair.get_first() = utility::move(other.pair.get_first());
                    other.pair.get_first() = utility::move(temp_alloc);
                }
            }
        }

        /**
         * \lang english
         * @brief Swaps the managed objects of two polymorphic objects.
         *
         * @param lhs The first polymorphic to swap
         * @param rhs The second polymorphic to swap
         *
         * \lang simp-chinese
         * @brief 交换两个 polymorphic 对象的被管理对象。
         *
         * @param lhs 要交换的第一个 polymorphic
         * @param rhs 要交换的第二个 polymorphic
         */
        friend RAINY_CONSTEXPR20 void swap(polymorphic &lhs, polymorphic &rhs) noexcept(noexcept(lhs.swap(rhs))) {
            lhs.swap(rhs);
        }

    private:
        struct control_block_base {
            virtual ~control_block_base() = default;
            virtual void destroy(allocator_type &alloc) noexcept = 0;
            virtual control_block_base *clone(allocator_type &alloc) const = 0;
            virtual Ty *get_value_ptr() noexcept = 0;
            virtual const Ty *get_value_ptr() const noexcept = 0;
        };

        template <typename U>
        struct control_block : control_block_base {
            template <typename... Args>
            control_block(Args &&...args) : value(utility::forward<Args>(args)...) { // NOLINT
            }

            void destroy(allocator_type &alloc) noexcept override {
                using block_allocator = typename memory::allocator_traits<allocator_type>::template rebind_alloc<control_block>;
                block_allocator block_alloc(alloc);
                auto *derived_this = static_cast<control_block *>(this);
                memory::allocator_traits<block_allocator>::destroy(block_alloc, derived_this);
                memory::allocator_traits<block_allocator>::deallocate(block_alloc, derived_this, 1);
            }

            control_block_base *clone(allocator_type &alloc) const override {
                using block_allocator = typename memory::allocator_traits<allocator_type>::template rebind_alloc<control_block>;
                block_allocator block_alloc(alloc);
                auto *ptr = memory::allocator_traits<block_allocator>::allocate(block_alloc, 1);
                try {
                    memory::allocator_traits<block_allocator>::construct(block_alloc, ptr, value);
                    return ptr;
                } catch (...) {
                    memory::allocator_traits<block_allocator>::deallocate(block_alloc, ptr, 1);
                    throw;
                }
            }

            Ty *get_value_ptr() noexcept override {
                return &value;
            }

            const Ty *get_value_ptr() const noexcept override {
                return &value;
            }

            U value;
        };

        void construct_default() {
            using block_allocator = typename memory::allocator_traits<allocator_type>::template rebind_alloc<control_block<Ty>>; // NOLINT
            block_allocator block_alloc(pair.get_first());
            auto *ptr = memory::allocator_traits<block_allocator>::allocate(block_alloc, 1);
            try {
                memory::allocator_traits<block_allocator>::construct(block_alloc, ptr);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<block_allocator>::deallocate(block_alloc, ptr, 1);
                throw;
            }
        }

        template <typename UTy, typename... Args>
        void construct_inplace(Args &&...args) {
            using block_allocator = typename memory::allocator_traits<allocator_type>::template rebind_alloc<control_block<UTy>>; // NOLINT
            block_allocator block_alloc(pair.get_first());
            auto *ptr = memory::allocator_traits<block_allocator>::allocate(block_alloc, 1);
            try {
                memory::allocator_traits<block_allocator>::construct(block_alloc, ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<block_allocator>::deallocate(block_alloc, ptr, 1);
                throw;
            }
        }

        template <typename UTy, typename V>
        void construct_from_value(V &&v) {
            using block_allocator = typename memory::allocator_traits<allocator_type>::template rebind_alloc<control_block<UTy>>; // NOLINT
            block_allocator block_alloc(pair.get_first());
            auto *ptr = memory::allocator_traits<block_allocator>::allocate(block_alloc, 1);
            try {
                memory::allocator_traits<block_allocator>::construct(block_alloc, ptr, utility::forward<V>(v));
                pair.get_second() = ptr;
            } catch (...) {
                memory::allocator_traits<block_allocator>::deallocate(block_alloc, ptr, 1);
                throw;
            }
        }

        void copy_from(const polymorphic &other) {
            auto *block = static_cast<control_block_base *>(other.pair.get_second());
            pair.get_second() = block->clone(pair.get_first());
        }

        void reset() noexcept {
            if (pair.get_second() != nullptr) {
                auto *block = static_cast<control_block_base *>(pair.get_second());
                block->destroy(pair.get_first());
                pair.get_second() = nullptr;
            }
        }

        compressed_pair<Alloc, void *> pair;
    };
}

namespace rainy::container {
    using rainy::core::container::polymorphic;
}


#endif
