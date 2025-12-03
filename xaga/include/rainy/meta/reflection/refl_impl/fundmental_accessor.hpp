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
#ifndef RAINY_META_REFL_IMPL_FUNDMENTAL_ACCESSOR_HPP
#define RAINY_META_REFL_IMPL_FUNDMENTAL_ACCESSOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::meta::reflection {
    enum class fundmental_category {
        bool_t, // bool
        char_t, // char
        signed_char_t, // signed char
        unsigned_char_t, // unsigned char
        char8, // char8_t
        char16, // char16_t
        char32, // char32_t
        wchar, // wchar_t

        short_t, // short
        int_t, // int
        long_t, // long
        long_long_t, // long long

        unsigned_short_t, // unsigned short
        unsigned_int_t, // unsigned int
        unsigned_long_t, // unsigned long
        unsigned_long_long_t, // unsigned long long

        float_t, // float
        double_t, // double
        long_double_t, // long double

        void_t, // void
        byte_t, // std::byte (C++17)
        nullptr_t, // std::nullptr_t

        none
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Ty>
    constexpr fundmental_category deduction_category() noexcept {
        auto type = foundation::ctti::typeinfo::create<Ty>();
        switch (type) { 
            case rainy_typehash(int):
                return fundmental_category::int_t;
            case rainy_typehash(long):
                return fundmental_category::long_t;
            case rainy_typehash(long long):
                return fundmental_category::long_long_t;
        }
        return fundmental_category::none;
    }
}

namespace rainy::meta::reflection::implements {
    struct fundmental_type_accessor {
        virtual const foundation::ctti::typeinfo &typeinfo() const noexcept = 0;
        virtual std::string_view name() const noexcept = 0;
        virtual fundmental_category category() const noexcept = 0;
        virtual bool is_arithmetic() const noexcept = 0;
        virtual bool is_integral() const noexcept = 0;
        virtual bool is_floating_point() const noexcept = 0;
        virtual bool is_signed() const noexcept = 0;
        virtual bool is_nullptr() const noexcept = 0;
        virtual bool is_convertible_from(const foundation::ctti::typeinfo& type) const noexcept = 0;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type>
    struct fundmental_type_accessor_impl final : fundmental_type_accessor {
        fundmental_type_accessor_impl(std::string_view name) : name_{name} {
        }

        const foundation::ctti::typeinfo &typeinfo() const noexcept {
            return rainy_typeid(Type);
        }

        std::string_view name() const noexcept {
            return name_;
        }

        fundmental_category category() const noexcept {
            return deduction_category<Type>();
        }

        bool is_arithmetic() const noexcept {
            return type_traits::composite_types::is_arithmetic_v<Type>;
        }

        bool is_integral() const noexcept {
            return type_traits::primary_types::is_integral_v<Type>;
        }

        bool is_floating_point() const noexcept {
            return type_traits::primary_types::is_floating_point_v<Type>;
        }

        bool is_signed() const noexcept {
            return type_traits::type_properties::is_signed_v<Type>;
        }

        bool is_nullptr() const noexcept {
            return type_traits::primary_types::is_null_pointer_v<Type>;
        }

        bool is_convertible_from(const foundation::ctti::typeinfo &type) const noexcept {
            return utility::any_converter<Type>::is_convertible(type);
        }

        std::string_view name_;
    };
}

#endif