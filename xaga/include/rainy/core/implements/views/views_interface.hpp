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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_COLLECTIONS_VIEWS_VIEWS_INTERFACE_HPP
#define RAINY_COLLECTIONS_VIEWS_VIEWS_INTERFACE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/implements/views/pipeline.hpp>
#include <rainy/core/implements/views/range_closure.hpp>

namespace rainy::collections::views {
    /**
     * @brief CRTP base class for implementing view interfaces.
     *       用于实现视图接口的CRTP基类。
     *
     * This class provides a common interface for range views, including
     * empty(), size(), front(), back(), operator[], and iterator access.
     * Derived classes must provide begin() and end() members.
     *
     * 此类为范围视图提供通用接口，包括empty()、size()、front()、back()、
     * operator[]和迭代器访问。派生类必须提供begin()和end()成员。
     *
     * @tparam Derived The derived view class (CRTP pattern)
     *                 派生的视图类（CRTP模式）
     */
    template <typename Derived>
    class view_interface {
    public:
        static_assert(type_traits::primary_types::is_class_v<Derived> &&
                      type_traits::type_relations::is_same_v<Derived, type_traits::cv_modify::remove_cv_t<Derived>>);

        /**
         * @brief Checks if the view is empty.
         *        检查视图是否为空。
         *
         * @return true if the view contains no elements, false otherwise
         *         如果视图不包含任何元素则为true，否则为false
         */
        RAINY_NODISCARD constexpr rain_fn empty() -> bool {
            auto &self = cast_to_derived();
            if constexpr (type_traits::extras::meta_method::has_size_v<Derived>) {
                return utility::size(self) == 0;
            } else {
                return utility::begin(self) == utility::end(self);
            }
        }

        /**
         * @brief Checks if the view is empty (const version).
         *        检查视图是否为空（常量版本）。
         *
         * @return true if the view contains no elements, false otherwise
         *         如果视图不包含任何元素则为true，否则为false
         */
        RAINY_NODISCARD constexpr rain_fn empty() const -> bool {
            auto &self = cast_to_derived();
            if constexpr (type_traits::extras::meta_method::has_size_v<const Derived>) {
                return utility::size(self) == 0;
            } else {
                return utility::begin(self) == utility::end(self);
            }
        }

        /**
         * @brief Returns a const iterator to the beginning.
         *        返回指向起始的常量迭代器。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        RAINY_NODISCARD constexpr rain_fn cbegin() -> auto {
            return utility::cbegin(cast_to_derived());
        }

        /**
         * @brief Returns a const iterator to the beginning (const version).
         *        返回指向起始的常量迭代器（常量版本）。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        RAINY_NODISCARD constexpr rain_fn cbegin() const -> auto {
            return utility::cbegin(cast_to_derived());
        }

        /**
         * @brief Returns a const iterator to the end.
         *        返回指向末尾的常量迭代器。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        RAINY_NODISCARD constexpr rain_fn cend() -> auto {
            return utility::cend(cast_to_derived());
        }

        /**
         * @brief Returns a const iterator to the end (const version).
         *        返回指向末尾的常量迭代器（常量版本）。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        RAINY_NODISCARD constexpr rain_fn cend() const -> auto {
            return utility::cend(cast_to_derived());
        }

        /**
         * @brief Checks if the view is non-empty (conversion to bool).
         *        检查视图是否非空（转换为bool）。
         *
         * @return true if the view is not empty, false otherwise
         *         如果视图非空则为true，否则为false
         */
        constexpr explicit operator bool() const {
            return !empty(cast_to_derived());
        }

        /**
         * @brief Returns a pointer to the underlying data (requires contiguous iterator).
         *        返回指向底层数据的指针（需要连续迭代器）。
         *
         * @tparam D Derived type (deduced)
         *           派生类型（推导）
         * @return Pointer to the first element
         *         指向第一个元素的指针
         */
        template <typename D = Derived,
                  typename = decltype(utility::to_address(utility::begin(utility::declval<view_interface<D>>().cast_to_derived())))>
        RAINY_NODISCARD constexpr rain_fn data() -> auto {
            static_assert(type_traits::extras::iterators::is_contiguous_iterator_v<type_traits::extras::iterators::iterator_t<D>>);
            return utility::to_address(utility::begin(cast_to_derived()));
        }

        /**
         * @brief Returns a const pointer to the underlying data (requires contiguous iterator).
         *        返回指向底层数据的常量指针（需要连续迭代器）。
         *
         * @tparam D Derived type (deduced)
         *           派生类型（推导）
         * @return Const pointer to the first element
         *         指向第一个元素的常量指针
         */
        template <typename D = Derived,
                  typename = decltype(utility::to_address(utility::begin(utility::declval<view_interface<D>>().cast_to_derived())))>
        RAINY_NODISCARD constexpr rain_fn data() const -> auto {
            static_assert(type_traits::extras::iterators::is_contiguous_iterator_v<type_traits::extras::iterators::iterator_t<D>>);
            return utility::to_address(utility::begin(cast_to_derived()));
        }

        /**
         * @brief Returns the number of elements in the view.
         *        返回视图中的元素数量。
         *
         * @return The size of the view
         *         视图的大小
         */
        RAINY_NODISCARD constexpr rain_fn size() -> auto {
            auto &self = cast_to_derived();
            return utility::distance(utility::begin(self), utility::end(self));
        }

        /**
         * @brief Returns the number of elements in the view (const version).
         *        返回视图中的元素数量（常量版本）。
         *
         * @return The size of the view
         *         视图的大小
         */
        RAINY_NODISCARD constexpr rain_fn size() const -> auto {
            auto &self = cast_to_derived();
            return utility::distance(utility::begin(self), utility::end(self));
        }

        /**
         * @brief Returns a reference to the first element.
         *        返回第一个元素的引用。
         *
         * @return Reference to the first element
         *         第一个元素的引用
         */
        RAINY_NODISCARD constexpr rain_fn front() -> decltype(auto) {
            auto &self = cast_to_derived();
            return *utility::begin(self);
        }

        /**
         * @brief Returns a const reference to the first element.
         *        返回第一个元素的常量引用。
         *
         * @return Const reference to the first element
         *         第一个元素的常量引用
         */
        RAINY_NODISCARD constexpr rain_fn front() const -> decltype(auto) {
            auto &self = cast_to_derived();
            return *utility::begin(self);
        }

        /**
         * @brief Returns a reference to the last element.
         *        返回最后一个元素的引用。
         *
         * @return Reference to the last element
         *         最后一个元素的引用
         */
        RAINY_NODISCARD constexpr rain_fn back() -> decltype(auto) {
            auto &self = cast_to_derived();
            auto last = utility::end(self);
            return *--last;
        }

        /**
         * @brief Returns a const reference to the last element.
         *        返回最后一个元素的常量引用。
         *
         * @return Const reference to the last element
         *         最后一个元素的常量引用
         */
        RAINY_NODISCARD constexpr rain_fn back() const -> decltype(auto) {
            auto &self = cast_to_derived();
            auto last = utility::end(self);
            return *--last;
        }

        /**
         * @brief Subscript operator for random-access views.
         *        随机访问视图的下标运算符。
         *
         * @tparam D Derived type (deduced)
         *           派生类型（推导）
         * @param idx Index to access
         *            要访问的索引
         * @return Reference to the element at the specified index
         *         指定索引处元素的引用
         */
        template <typename D = Derived>
        RAINY_NODISCARD constexpr rain_fn operator[](const typename D::difference_type idx)->decltype(auto) {
            static_assert(type_traits::extras::iterators::is_random_access_iterator_v<type_traits::extras::iterators::iterator_t<D>>);
            auto &self = cast_to_derived();
            return utility::begin(self)[idx];
        }

        /**
         * @brief Subscript operator for random-access views (const version).
         *        随机访问视图的下标运算符（常量版本）。
         *
         * @tparam D Derived type (deduced)
         *           派生类型（推导）
         * @param idx Index to access
         *            要访问的索引
         * @return Const reference to the element at the specified index
         *         指定索引处元素的常量引用
         */
        template <typename D = Derived>
        RAINY_NODISCARD constexpr rain_fn operator[](const typename D::difference_type idx) const->decltype(auto) {
            static_assert(
                type_traits::extras::iterators::is_random_access_iterator_v<type_traits::extras::iterators::const_iterator_t<D>>);
            auto &self = cast_to_derived();
            return utility::begin(self)[idx];
        }

    private:
        /**
         * @brief Casts *this to the derived type.
         *        将*this转换为派生类型。
         *
         * @return Reference to the derived object
         *         派生对象的引用
         */
        RAINY_NODISCARD constexpr rain_fn cast_to_derived() noexcept -> Derived & {
            static_assert(type_traits::type_relations::is_base_of_v<view_interface, Derived>,
                          "view_interface's template argument D must derive from view_interface<D> ");
            return static_cast<Derived &>(*this);
        }

        /**
         * @brief Casts *this to the const derived type.
         *        将*this转换为常量派生类型。
         *
         * @return Const reference to the derived object
         *         派生对象的常量引用
         */
        RAINY_NODISCARD constexpr rain_fn cast_to_derived() const noexcept -> const Derived & {
            static_assert(type_traits::type_relations::is_base_of_v<view_interface, Derived>,
                          "view_interface's template argument D must derive from view_interface<D> ");
            return static_cast<const Derived &>(*this);
        }
    };
}
#endif