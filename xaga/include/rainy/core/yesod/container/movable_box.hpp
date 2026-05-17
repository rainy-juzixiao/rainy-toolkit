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
 */                                                                                                                                   \
#ifndef RAINY_FOUNDATION_CONTAINER_MOVABLE_BOX_HPP
#define RAINY_FOUNDATION_CONTAINER_MOVABLE_BOX_HPP

#include <rainy/core/core.hpp>

namespace rainy::foundation::container::implements {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_valid_movable_object =
        type_traits::type_properties::is_copy_constructible_v<Ty> && type_traits::type_properties::is_destructible_v<Ty>;
}

namespace rainy::foundation::container {
    template <typename Ty>
    class movable_box {
    public:
        template <typename UTy = Ty, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<UTy>, int> = 0>
        constexpr movable_box() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<UTy>) :
            value_(), is_valid_{true} {
        }

        template <typename... Args>
        constexpr movable_box(std::in_place_t,
                              Args &&...args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) :
            value_(utility::forward<Args>(args)...), is_valid_{true} {
        }

        RAINY_CONSTEXPR20 ~movable_box() {
            if (is_valid_) {
                value_.~Ty();
            }
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_constructible_v<Ty>, int> = 0>
        constexpr movable_box(const movable_box &right) : is_valid_{right.is_valid_} {
            if (right.is_valid_) {
                utility::construct_in_place(value_, static_cast<const Ty &>(right.value_));
            }
        }

        constexpr movable_box(movable_box &&right) : is_valid_{right.is_valid_} {
            if (right.is_valid_) {
                utility::construct_in_place(value_, static_cast<Ty &&>(right.value_));
            }
        }

        template <typename UTy = Ty,type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copyable_v<UTy>, int> = 0>
        constexpr movable_box &operator=(const movable_box &right) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<UTy> &&
            type_traits::type_properties::is_nothrow_copy_assignable_v<UTy>) {
            if (is_valid_) {
                if (right.is_valid_) {
                    static_cast<Ty &>(value_) = static_cast<const Ty &>(right.value_);
                } else {
                    value_.~Ty();
                    is_valid_ = false;
                }
            } else {
                if (right.is_valid_) {
                    utility::construct_in_place(value_, static_cast<const Ty &>(right.value_));
                    is_valid_ = true;
                } else {
                    // nothing to do
                }
            }
            return *this;
        }

        constexpr movable_box &operator=(movable_box &&right) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<Ty>) {
            if (utility::addressof(right) == this) {
                return *this;
            }
            if (is_valid_) {
                value_.~Ty();
                is_valid_ = false;
            }
            if (right.is_valid_) {
                utility::construct_in_place(value_, static_cast<Ty &&>(right.value_));
                is_valid_ = true;
            }
            return *this;
        }

        constexpr explicit operator bool() const noexcept {
            return is_valid_;
        }

        RAINY_NODISCARD constexpr Ty &operator*() noexcept {
            assert(is_valid_);
            return value_;
        }
        RAINY_NODISCARD constexpr const Ty &operator*() const noexcept {
            assert(is_valid_);
            return value_;
        }

    private:
        union {
            type_traits::cv_modify::remove_cv_t<Ty> value_;
        };
        bool is_valid_;
    };
}

#endif