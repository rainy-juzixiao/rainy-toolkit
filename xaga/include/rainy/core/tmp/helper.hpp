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
#ifndef RAINY_CORE_TMP_HELPER_HPP
#define RAINY_CORE_TMP_HELPER_HPP
#include <rainy/core/tmp/implements.hpp>
#include <rainy/core/tmp/modifers.hpp>

namespace rainy::type_traits::helper {
    template <typename Ty>
    struct wrapper {
        inline static cv_modify::remove_cvref_t<Ty> value;
    };

    template <typename Ty>
    constexpr cv_modify::remove_cvref_t<Ty> &get_fake_object() noexcept {
        return wrapper<cv_modify::remove_cvref_t<Ty>>::value;
    }

    template <typename Ty>
    struct identity {
        using type = Ty;
    };

    template <typename Ty>
    using identity_t = typename identity<Ty>::type;

    template <typename Ty, Ty... Vals>
    struct integer_sequence {
        static_assert(implements::is_integral_v<Ty>, "integer_sequence<T, I...> requires T to be an integral type.");

        using value_type = Ty;

        RAINY_NODISCARD static constexpr std::size_t size() noexcept {
            return sizeof...(Vals);
        }
    };

    template <typename T, T N>
    using make_integer_sequence = typename core::builtin::make_integer_seq<integer_sequence, T, N>::type;

    template <std::size_t... Vals>
    using index_sequence = integer_sequence<std::size_t, Vals...>;

    template <std::size_t Size>
    using make_index_sequence = make_integer_sequence<std::size_t, Size>;

    template <typename... Types>
    using index_sequence_for = make_index_sequence<sizeof...(Types)>;

    class RAINY_TOOLKIT_API non_copyable {
    protected:
        non_copyable() = default;
        ~non_copyable() = default;
        non_copyable(const non_copyable &) = delete;
        non_copyable &operator=(const non_copyable &) = delete;
        non_copyable(non_copyable &&) = default;
        non_copyable &operator=(non_copyable &&) = default;
    };

    class RAINY_TOOLKIT_API non_moveable {
    protected:
        non_moveable() = default;
        ~non_moveable() = default;
        non_moveable(non_moveable &&) = delete;
        non_moveable &operator=(non_moveable &&) = delete;
    };

    template <typename Ty>
    struct make_unsigned {
        static_assert(implements::is_integral_v<Ty> || implements::_is_enum_v<Ty>,
                      "make_unsigned<T> requires Ty to be an integral or enum type");

        using type = implements::_conditional_t<
            implements::_is_const_v<Ty>,
            typename implements::_add_const<implements::make_unsigned_raw<implements::remove_cv_t<Ty>>>::type,
            implements::make_unsigned_raw<implements::remove_cv_t<Ty>>>;
    };

    template <typename Ty>
    using make_unsigned_t = typename make_unsigned<Ty>::type;

    template <typename Ty>
    struct make_signed {
        static_assert(implements::is_integral_v<Ty> || implements::_is_enum_v<Ty>,
                      "make_signed<T> requires Ty to be an integral or enum type");

        using type =
            implements::_conditional_t<implements::_is_const_v<Ty>,
                                       typename implements::_add_const<implements::make_signed_raw<implements::remove_cv_t<Ty>>>::type,
                                       implements::make_signed_raw<implements::remove_cv_t<Ty>>>;
    };

    template <typename Ty>
    using make_signed_t = typename make_signed<Ty>::type;
}

#endif