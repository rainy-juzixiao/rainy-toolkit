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
    template <typename Derived>
    class view_interface {
    public:
        static_assert(type_traits::primary_types::is_class_v<Derived> &&
                      type_traits::type_relations::is_same_v<Derived, type_traits::cv_modify::remove_cv_t<Derived>>);

        RAINY_NODISCARD constexpr bool empty() {
            auto &self = cast_to_derived();
            if constexpr (type_traits::extras::meta_method::has_size_v<Derived>) {
                return utility::size(self) == 0;
            } else {
                return utility::begin(self) == utility::end(self);
            }
        }

        RAINY_NODISCARD constexpr bool empty() const {
            auto &self = cast_to_derived();
            if constexpr (type_traits::extras::meta_method::has_size_v<const Derived>) {
                return utility::size(self) == 0;
            } else {
                return utility::begin(self) == utility::end(self);
            }
        }

        RAINY_NODISCARD constexpr auto cbegin() {
            return utility::cbegin(cast_to_derived());
        }

        RAINY_NODISCARD constexpr auto cbegin() const {
            return utility::cbegin(cast_to_derived());
        }

        RAINY_NODISCARD constexpr auto cend() {
            return utility::cend(cast_to_derived());
        }

        RAINY_NODISCARD constexpr auto cend() const {
            return utility::cend(cast_to_derived());
        }

        constexpr explicit operator bool() const {
            return !empty(cast_to_derived());
        }

        template <typename D = Derived, typename = decltype(utility::to_address(utility::begin(utility::declval<view_interface<D>>().cast_to_derived())))>
        RAINY_NODISCARD constexpr auto data() {
            static_assert(
                type_traits::extras::iterators::is_contiguous_iterator_v<type_traits::extras::iterators::iterator_t<D>>);
            return utility::to_address(utility::begin(cast_to_derived()));
        }

        template <typename D = Derived,
                  typename = decltype(utility::to_address(utility::begin(utility::declval<view_interface<D>>().cast_to_derived())))>
        RAINY_NODISCARD constexpr auto data() const {
            static_assert(
                type_traits::extras::iterators::is_contiguous_iterator_v<type_traits::extras::iterators::iterator_t<D>>);
            return utility::to_address(utility::begin(cast_to_derived()));
        }

        RAINY_NODISCARD constexpr auto size() {
            auto &self = cast_to_derived();
            return utility::distance(utility::begin(self), utility::end(self));
        }

        RAINY_NODISCARD constexpr auto size() const {
            auto &self = cast_to_derived();
            return utility::distance(utility::begin(self), utility::end(self));
        }

        RAINY_NODISCARD constexpr decltype(auto) front() {
            auto &self = cast_to_derived();
            return *utility::begin(self);
        }

        RAINY_NODISCARD constexpr decltype(auto) front() const {
            auto &self = cast_to_derived();
            return *utility::begin(self);
        }

        RAINY_NODISCARD constexpr decltype(auto) back() {
            auto &self = cast_to_derived();
            auto last = utility::end(self);
            return *--last;
        }

        RAINY_NODISCARD constexpr decltype(auto) back() const {
            auto &self = cast_to_derived();
            auto last = utility::end(self);
            return *--last;
        }

        template <typename D = Derived>
        RAINY_NODISCARD constexpr decltype(auto) operator[](const typename D::difference_type idx) {
            static_assert(type_traits::extras::iterators::is_random_access_iterator_v<type_traits::extras::iterators::iterator_t<D>>);
            auto &self = cast_to_derived();
            return utility::begin(self)[idx];
        }

        template <typename D = Derived>
        RAINY_NODISCARD constexpr decltype(auto) operator[](const typename D::difference_type idx) const {
            static_assert(
                type_traits::extras::iterators::is_random_access_iterator_v<type_traits::extras::iterators::const_iterator_t<D>>);
            auto &self = cast_to_derived();
            return utility::begin(self)[idx];
        }

    private:
        RAINY_NODISCARD constexpr Derived &cast_to_derived() noexcept {
            static_assert(type_traits::type_relations::is_base_of_v<view_interface, Derived>,
                          "view_interface's template argument D must derive from view_interface<D> ");
            return static_cast<Derived &>(*this);
        }

        RAINY_NODISCARD constexpr const Derived &cast_to_derived() const noexcept {
            static_assert(type_traits::type_relations::is_base_of_v<view_interface, Derived>,
                          "view_interface's template argument D must derive from view_interface<D> ");
            return static_cast<const Derived &>(*this);
        }
    };
}

#endif