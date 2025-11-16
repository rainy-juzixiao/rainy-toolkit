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
#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_RAII_MANAGER_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_RAII_MANAGER_HPP
#include <rainy/core/core.hpp>

namespace rainy::utility::implements {
    template <typename Ty, std::size_t Length>
    inline constexpr bool any_is_small = type_traits::type_properties::is_nothrow_move_constructible_v<Ty> && sizeof(Ty) <= Length;

    enum class any_representation : uintptr_t {
        big,
        _small,
        reference
    };

    struct big_any_raii_manager {
        using destory_fn = void(void *target);
        using copy_fn = void *(const void *source);
        using size_fn = std::size_t() noexcept;

        template <typename Ty>
        static void destory(void *const target) {
            ::delete static_cast<Ty *>(target);
        }

        template <typename Ty>
        RAINY_NODISCARD static void *copy(const void *const source) {
            return ::new Ty(*static_cast<const Ty *>(source));
        }

        destory_fn *destory_;
        copy_fn *copy_;
    };

    struct small_any_raii_manager {
        using destroy_fn = void(void *target);
        using copy_fn = void(void *target, const void *source);
        using move_fn = void(void *target, void *source) noexcept;

        template <typename Ty>
        static void destroy(void *const target) {
            static_cast<Ty *const>(target)->~Ty();
        }

        template <typename Ty>
        static void copy(void *const target, const void *const source) {
            ::new (static_cast<Ty *>(target)) Ty(*static_cast<const Ty *>(source));
        }

        template <typename Ty>
        static void move(void *const target, void *const source) noexcept {
            ::new (static_cast<Ty *>(target)) Ty(utility::move(*static_cast<Ty *>(source)));
        }

        destroy_fn *destroy_;
        copy_fn *copy_;
        move_fn *move_;
    };

    template <typename Ty>
    inline constexpr big_any_raii_manager any_big_raii_manager_object = {&big_any_raii_manager::destory<Ty>,
                                                                         &big_any_raii_manager::copy<Ty>};

    template <typename Ty>
    inline constexpr small_any_raii_manager any_small_raii_manager_object = {
        &small_any_raii_manager::destroy<Ty>, &small_any_raii_manager::copy<Ty>, &small_any_raii_manager::move<Ty>};
}

#endif