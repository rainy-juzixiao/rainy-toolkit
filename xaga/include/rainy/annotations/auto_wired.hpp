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
#ifndef RAINY_ANNOTATIONS_AUTO_WIRED_HPP
#define RAINY_ANNOTATIONS_AUTO_WIRED_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/ioc.hpp>

namespace rainy::annotations {
    template <typename Type>
    class auto_wired {
    public:
        using type = type_traits::cv_modify::remove_cvref_t<Type>;

        auto_wired() {
            if (utility::ioc::container::is_this_type_available<Type>()) {
                object = utility::ioc::container::get_instance<Type>();
            } else {
                utility::ioc::container::register_type<Type>();
                object = utility::ioc::container::get_instance<Type>();
            }
        }

        template <typename... Args>
        auto_wired(Args &&...args) { // NOLINT
            if (utility::ioc::container::is_this_type_with_construct_args_type_available<Type(Args...)>()) {
                object = utility::ioc::container::get_instance<Type>();
            } else {
                utility::ioc::container::register_type<Type, Type, Args...>();
                object = utility::ioc::container::get_instance<Type>(utility::forward<Args>(args)...);
            }
        }

        auto_wired(const auto_wired &) = default;
        auto_wired &operator=(const auto_wired &) noexcept = default;
        auto_wired(auto_wired &&) noexcept = default;
        auto_wired& operator=(auto_wired &&) noexcept = default;

        rain_fn has_inject_failed() -> bool {
            return object == nullptr;
        }

        rain_fn operator->() noexcept -> type * {
            return object;
        }

        rain_fn operator->() const noexcept -> const type * {
            return object;
        }

        rain_fn operator*() noexcept -> type & {
            return *object;
        }

        rain_fn operator*() const noexcept -> type & {
            return *object;
        }

    private:
        type *object{nullptr};
    };
}

#endif
