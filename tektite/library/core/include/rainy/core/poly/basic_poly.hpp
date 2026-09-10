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
#ifndef RAINY_CORE_BASIC_POLY_HPP
#define RAINY_CORE_BASIC_POLY_HPP
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::core::implements {
    struct poly_inspector {
        template <typename Type>
        operator Type &&() const; // NOLINT

        template <std::size_t Member, typename... Args>
        RAINY_NODISCARD poly_inspector invoke(Args &&...args) const; // NOLINT

        template <std::size_t Member, typename... Args>
        RAINY_NODISCARD poly_inspector invoke(Args &&...args); // NOLINT
    };

    template <typename Concept>
    struct make_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;

        template <auto... Candidate>
        static auto make(type_traits::other_trans::value_list<Candidate...>) noexcept
            -> decltype(container::make_tuple(vtable_entry(Candidate)...));

        template <typename... Func>
        RAINY_NODISCARD static constexpr auto make(type_traits::other_trans::type_list<Func...>) noexcept {
            if constexpr (sizeof...(Func) == 0u) {
                return decltype(make_with_vl(typename Concept::template impl<inspector>{}))();
            } else if constexpr ((type_traits::primary_types::is_function_v<Func> && ...)) {
                return decltype(container::make_tuple(vtable_entry(utility::declval<Func>())...))();
            }
        }

        template <auto... V>
        RAINY_NODISCARD static constexpr auto make_with_vl(type_traits::other_trans::value_list<V...>) noexcept {
            return decltype(container::make_tuple(vtable_entry(V)...))();
        }

        template <typename Func>
        static auto vtable_entry(Func) noexcept {
            using namespace type_traits;
            using namespace type_traits::primary_types;
            using Traits = function_traits<Func>;
            using ret = typename Traits::return_type;
            if constexpr (Traits::is_const_member_function) {
                return static_cast<modifers::add_pointer_t<
                    typename make_normalfx_type_with_pl<ret, Func, other_trans::type_list<const void *>>::type>>(nullptr);
            } else if constexpr (Traits::is_member_function_pointer) {
                return static_cast<
                    modifers::add_pointer_t<typename make_normalfx_type_with_pl<ret, Func, other_trans::type_list<void *>>::type>>(
                    nullptr);
            } else {
                return static_cast<Func>(nullptr);
            }
        }

        template <typename Type, auto Candidate, typename Ret, typename PtrType, typename... Args>
        static void fill_vtable_entry(Ret (*&entry)(PtrType, Args...)) noexcept {
            if constexpr (type_traits::properties::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
                entry = +[](PtrType, Args... args) -> Ret { return utility::invoke(Candidate, utility::forward<Args>(args)...); };
            } else {
                entry = +[](PtrType instance_ptr, Args... args) -> Ret {
                    return static_cast<Ret>(utility::invoke(
                        Candidate,
                        *static_cast<type_traits::modifers::constness_as_t<Type, type_traits::modifers::remove_pointer_t<PtrType>> *>(instance_ptr),
                        utility::forward<Args>(args)...));
                };
            }
        }

        template <typename VtableType, typename Type, auto... Index>
        RAINY_NODISCARD static auto fill_vtable(type_traits::helper::index_sequence<Index...>) noexcept {
            VtableType impl{};
            (fill_vtable_entry<Type, type_traits::other_trans::value_at<Index, typename Concept::template impl<Type>>::value>(
                 container::get<Index>(impl)),
             ...);
            return impl;
        }
    };
}

namespace rainy::core {
    /**
     * @brief Virtual table provider for polymorphic concepts.
     *        Generates and stores the vtable for a given concept.
     *
     *        多态概念的虚表提供者。
     *        为给定概念生成并存储虚表。
     *
     * @tparam Concept The concept that defines the polymorphic interface
     *                 定义多态接口的概念
     */
    template <typename Concept>
    class poly_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;
        using vtable_type = decltype(implements::make_vtable<Concept>::make_with_vl(typename Concept::template impl<inspector>{}));
        static constexpr bool is_mono = container::tuple_size_v<vtable_type> == 1u;

        using type = type_traits::other_trans::conditional_t<is_mono, container::tuple_element_t<0u, vtable_type>, const vtable_type *>;

        /**
         * @brief Gets the vtable instance for a specific type.
         *        获取特定类型的虚表实例。
         *
         * @tparam Type The concrete type to get the vtable for
         *              要获取虚表的具体类型
         * @return The vtable (direct value if mono, pointer otherwise)
         *         虚表（如果是单函数则为直接值，否则为指针）
         */
        template <typename Type>
        RAINY_NODISCARD static rain_fn instance() noexcept -> type {
            static_assert(type_traits::type_relations::is_same_v<Type, type_traits::other_trans::decay_t<Type>>,
                          "Type differs from its decayed form");
            static const vtable_type vtable = implements::make_vtable<Concept>::template fill_vtable<vtable_type, Type>(
                type_traits::helper::make_index_sequence<
                    type_traits::other_trans::value_list_size_v<typename Concept::template impl<Type>>>{});
            if constexpr (is_mono) {
                return container::get<0>(vtable);
            } else {
                return &vtable;
            }
        }
    };

    /**
     * @brief Base class for polymorphic objects providing virtual call dispatching.
     *        为多态对象提供虚调用分派的基类。
     *
     * @tparam Poly The derived polymorphic type (CRTP)
     *              派生的多态类型（CRTP）
     */
    template <typename Poly>
    struct poly_base {
        /**
         * @brief Invokes a member function on a const polymorphic object.
         *        在常量多态对象上调用成员函数。
         *
         * @tparam Member The index of the member function to invoke
         *                要调用的成员函数索引
         * @tparam Args The argument types
         *              参数类型
         * @param self The const polymorphic object
         *             常量多态对象
         * @param args The arguments to forward
         *             要转发的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <std::size_t Member, typename... Args>
        rain_fn invoke(const poly_base &self, Args &&...args) const -> decltype(auto) {
            const auto &poly = static_cast<const Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return container::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }

        /**
         * @brief Invokes a member function on a mutable polymorphic object.
         *        在可变多态对象上调用成员函数。
         *
         * @tparam Member The index of the member function to invoke
         *                要调用的成员函数索引
         * @tparam Args The argument types
         *              参数类型
         * @param self The mutable polymorphic object
         *             可变多态对象
         * @param args The arguments to forward
         *             要转发的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <std::size_t Member, typename... Args>
        rain_fn invoke(poly_base &self, Args &&...args) -> decltype(auto) {
            auto &poly = static_cast<Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                static_assert(Member == 0, "Unknown member");
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return container::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }
    };

    /**
     * @brief Helper function to make polymorphic calls.
     *        进行多态调用的辅助函数。
     *
     * @tparam Member The index of the member function to invoke
     *                要调用的成员函数索引
     * @tparam Poly The polymorphic type
     *              多态类型
     * @tparam Args The argument types
     *              参数类型
     * @param self The polymorphic object
     *             多态对象
     * @param args The arguments to forward
     *             要转发的参数
     * @return The result of the function call
     *         函数调用的结果
     */
    template <std::size_t Member, typename Poly, typename... Args>
    rain_fn poly_call(Poly &&self, Args &&...args) -> decltype(auto) {
        return utility::forward<Poly>(self).template invoke<Member>(self, utility::forward<Args>(args)...);
    }

    /**
     * @brief Basic polymorphic wrapper that stores a void pointer and vtable.
     *        存储void指针和虚表的基本多态包装器。
     *
     * @tparam AbstractBody The abstract concept body that defines the interface
     *                      定义接口的抽象概念主体
     */
    template <typename AbstractBody>
    class basic_poly : AbstractBody::template type<poly_base<basic_poly<AbstractBody>>> {
    public:
        friend struct poly_base<basic_poly>;

        using abstract_type = typename AbstractBody::template type<poly_base<basic_poly>>;
        using vtable_info = poly_vtable<AbstractBody>;
        using vtable_type = typename vtable_info::type;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        basic_poly() noexcept = default;

        /**
         * @brief Constructs from a pointer to a concrete type.
         *        从指向具体类型的指针构造。
         *
         * @tparam Type The concrete type
         *              具体类型
         * @param ptr Pointer to the object to wrap
         *            要包装的对象的指针
         */
        template <typename Type>
        basic_poly(Type *ptr) noexcept : // NOLINT
            _ptr(static_cast<void *>(ptr)),
            vtable{
                vtable_info::template instance<type_traits::modifers::remove_cv_t<type_traits::modifers::remove_pointer_t<Type>>>()} {
        }

        /**
         * @brief Constructs a null polymorphic object.
         *        构造空多态对象。
         *
         * @param  nullptr_t
         */
        basic_poly(std::nullptr_t) { // NOLINT
        }

        /**
         * @brief Move constructor
         *        移动构造函数
         *
         * @param other The other polymorphic object to move from
         *              要移动的另一个多态对象
         */
        basic_poly(basic_poly &&other) noexcept : _ptr(other._ptr), vtable(other.vtable) {
            other._ptr = nullptr;
            other.vtable = {};
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other polymorphic object to move from
         *              要移动的另一个多态对象
         * @return Reference to this object
         *         此对象的引用
         */
        basic_poly &operator=(basic_poly &&other) noexcept {
            if (this != &other) {
                _ptr = other._ptr;
                vtable = other.vtable;
                other._ptr = nullptr;
                other.vtable = {};
            }
            return *this;
        }

        /**
         * @brief Assigns a null pointer.
         *        赋值为空指针。
         *
         * @param  nullptr_t
         * @return Reference to this object
         *         此对象的引用
         */
        basic_poly &operator=(std::nullptr_t) noexcept {
            _ptr = nullptr;
            return *this;
        }

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        basic_poly(const basic_poly &) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         */
        basic_poly &operator=(const basic_poly &) = default;

        /**
         * @brief Resets to null state.
         *        重置为空状态。
         */
        rain_fn reset() noexcept -> void {
            _ptr = nullptr;
            vtable = {};
        }

        /**
         * @brief Resets to point to a new object.
         *        重置为指向新对象。
         *
         * @tparam Type The concrete type
         *              具体类型
         * @param ptr Pointer to the new object
         *            新对象的指针
         */
        template <typename Type>
        rain_fn reset(Type *ptr) noexcept -> void {
            _ptr = static_cast<void *>(ptr);
            vtable =
                vtable_info::template instance<type_traits::modifers::remove_cv_t<type_traits::modifers::remove_pointer_t<Type>>>();
        }

        /**
         * @brief Checks if the object is empty.
         *        检查对象是否为空。
         *
         * @return true if empty, false otherwise
         *         如果为空则为true，否则为false
         */
        rain_fn empty() const noexcept -> bool {
            return vtable == nullptr;
        }

        /**
         * @brief Checks if the object holds a non-null pointer.
         *        检查对象是否持有非空指针。
         *
         * @return true if pointer is non-null, false otherwise
         *         如果指针非空则为true，否则为false
         */
        RAINY_NODISCARD explicit operator bool() const noexcept {
            return _ptr != nullptr;
        }

        /**
         * @brief Arrow operator to access the abstract interface.
         *        箭头运算符，用于访问抽象接口。
         *
         * @return Pointer to the abstract interface
         *         指向抽象接口的指针
         */
        RAINY_NODISCARD rain_fn operator->() noexcept -> abstract_type * {
            return this;
        }

        /**
         * @brief Const arrow operator to access the abstract interface.
         *        常量箭头运算符，用于访问抽象接口。
         *
         * @return Pointer to the abstract interface
         *         指向抽象接口的常量指针
         */
        RAINY_NODISCARD rain_fn operator->() const noexcept -> const abstract_type * {
            return this;
        }

        /**
         * @brief Gets the stored pointer as void*.
         *        获取存储的指针作为void*。
         *
         * @return The stored void pointer
         *         存储的void指针
         */
        rain_fn target_as_void_ptr() const noexcept -> void * {
            return _ptr;
        }

    protected:
        rain_fn reset_ptr(void *ptr) noexcept -> void {
            _ptr = ptr;
        }

    private:
        void *_ptr{};
        vtable_type vtable{};
    };
}

#endif
