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
#ifndef RAINY_ANNOTATIONS_DONT_HPP
#define RAINY_ANNOTATIONS_DONT_HPP
#include <rainy/core/core.hpp>

namespace rainy::annotations::dont::implements {
    template <typename Ty, bool = true>
    struct non_copyable_impl : type_traits::cv_modify::remove_cvref_t<Ty> {
        using non_copyable_base_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        using non_copyable_base_t::non_copyable_base_t;

        non_copyable_impl() = default;

        non_copyable_impl(const non_copyable_impl &) = delete;
        non_copyable_impl &operator=(const non_copyable_impl &) = delete;
        non_copyable_impl(non_copyable_impl &&) noexcept = default;
        non_copyable_impl &operator=(non_copyable_impl &&) noexcept = default;

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_constructible_v<Ty>, int> = 0>
        non_copyable_impl(non_copyable_base_t &&right) noexcept : non_copyable_base_t{utility::move(right)} {
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty>, int> = 0>
        non_copyable_impl &operator=(non_copyable_base_t &&right) noexcept {
            non_copyable_base_t::operator=(utility::move(right));
            return *this;
        }
    };

    template <typename Ty>
    struct non_copyable_impl<Ty, false> : type_traits::cv_modify::remove_cvref_t<Ty> {
        using non_copyable_base_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        using non_copyable_base_t::non_copyable_base_t;

        non_copyable_impl(const non_copyable_base_t &right) : non_copyable_base_t{right} {
        }

        non_copyable_impl &operator=(const non_copyable_base_t &right) {
            non_copyable_base_t::operator=(right);
            return *this;
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_constructible_v<Ty>, int> = 0>
        non_copyable_impl(non_copyable_base_t &&right) noexcept : non_copyable_base_t{utility::move(right)} {
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty>, int> = 0>
        non_copyable_impl& operator=(non_copyable_base_t &&right) noexcept {
            non_copyable_base_t::operator=(utility::move(right));
            return *this;
        }

        non_copyable_impl(const non_copyable_impl &) = default;
        non_copyable_impl &operator=(const non_copyable_impl &) = default;
        non_copyable_impl(non_copyable_impl &&) noexcept = default;
        non_copyable_impl &operator=(non_copyable_impl &&) noexcept = default;
    };
}

namespace rainy::annotations::dont {
    template <typename Ty>
    using non_copyable = implements::non_copyable_impl<Ty, true>;

    template <typename Ty, typename... Constraints>
    using non_copyable_if = implements::non_copyable_impl<Ty, type_traits::logical_traits::conjunction_v<Constraints...> &&
                                                                  type_traits::type_properties::is_copy_constructible_v<Ty> &&
                                                                  type_traits::type_properties::is_copy_assignable_v<Ty>>;
}

namespace rainy::annotations::dont::implements {
    template <typename Ty, bool = true>
    struct no_copy_construct_impl : type_traits::cv_modify::remove_cvref_t<Ty> {
        using no_copy_construct_base_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        using no_copy_construct_base_t::no_copy_construct_base_t;

        no_copy_construct_impl() = default;

        no_copy_construct_impl(const no_copy_construct_impl &) = delete;
        no_copy_construct_impl &operator=(const no_copy_construct_impl &) = default;
        no_copy_construct_impl(no_copy_construct_impl &&) noexcept = default;
        no_copy_construct_impl &operator=(no_copy_construct_impl &&) noexcept = default;

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_constructible_v<Ty>, int> = 0>
        no_copy_construct_impl(no_copy_construct_base_t &&right) noexcept : no_copy_construct_base_t{utility::move(right)} {
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_constructible_v<Ty>, int> = 0>
        no_copy_construct_impl &operator=(no_copy_construct_base_t &&right) noexcept {
            no_copy_construct_base_t::operator=(utility::move(right));
            return *this;
        }
    };

    template <typename Ty>
    struct no_copy_construct_impl<Ty, false> : type_traits::cv_modify::remove_cvref_t<Ty> {
        using no_copy_construct_base_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        using no_copy_construct_base_t::no_copy_construct_base_t;

        no_copy_construct_impl() = default;

        no_copy_construct_impl(const no_copy_construct_base_t &right) noexcept : no_copy_construct_base_t{right} {
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_constructible_v<Ty>, int> = 0>
        no_copy_construct_impl(no_copy_construct_base_t &&right) noexcept : no_copy_construct_base_t{utility::move(right)} {
        }

        no_copy_construct_impl &operator=(const no_copy_construct_base_t &right) {
            no_copy_construct_base_t::operator=(right);
            return *this;
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_assignable_v<Ty>, int> = 0>
        no_copy_construct_impl &operator=(no_copy_construct_base_t &&right) noexcept {
            no_copy_construct_base_t::operator=(utility::move(right));
            return *this;
        }

        no_copy_construct_impl(const no_copy_construct_impl &) = default;
        no_copy_construct_impl &operator=(const no_copy_construct_impl &) = default;
        no_copy_construct_impl(no_copy_construct_impl &&) noexcept = default;
        no_copy_construct_impl &operator=(no_copy_construct_impl &&) noexcept = default;
    };

    //// 禁止拷贝赋值的实现
    //template <typename Ty, bool = true>
    //struct no_copy_assign_impl : type_traits::cv_modify::remove_cvref_t<Ty> {
    //    using no_copy_assign_base_t = type_traits::cv_modify::remove_cvref_t<Ty>;
    //    using no_copy_assign_base_t::no_copy_assign_base_t;

    //    no_copy_assign_impl() = default;

    //    no_copy_assign_impl(const no_copy_assign_impl &) = default;
    //    no_copy_assign_impl(no_copy_assign_impl &&) noexcept = default;
    //    no_copy_assign_impl &operator=(const no_copy_assign_impl &) = delete;
    //    no_copy_assign_impl &operator=(no_copy_assign_impl &&) noexcept = default;

    //    template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_move_constructible_v<Ty>, int> = 0>
    //    no_copy_assign_impl(no_copy_assign_base_t &&right) noexcept : no_copy_assign_base_t{utility::move(right)} {
    //    }
    //};

    //template <typename Ty>
    //struct no_copy_assign_impl<Ty, false> : type_traits::cv_modify::remove_cvref_t<Ty> {
    //    using no_copy_assign_base_t = type_traits::cv_modify::remove_cvref_t<Ty>;
    //    using no_copy_assign_base_t::no_copy_assign_base_t;

    //    no_copy_assign_impl() = default;

    //    no_copy_assign_impl(const no_copy_assign_impl &) = default;
    //    no_copy_assign_impl(no_copy_assign_impl &&) noexcept = default;
    //    no_copy_assign_impl &operator=(const no_copy_assign_impl &) = default;
    //    no_copy_assign_impl &operator=(no_copy_assign_impl &&) noexcept = default;
    //};
}

namespace rainy::annotations::dont {
    template <typename Ty>
    using no_copy_construct = implements::no_copy_construct_impl<Ty, true>;

    template <typename Ty, typename... Constraints>
    using no_copy_construct_if = implements::no_copy_construct_impl<Ty, type_traits::logical_traits::conjunction_v<Constraints...> &&
                                                                          type_traits::type_properties::is_copy_constructible_v<Ty>>;
}

#endif