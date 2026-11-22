/*
 * Copyright 2026 rainy::foundation::container-juzixiao
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
#ifndef RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_ANY_REFERENCE_HPP
#define RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_ANY_REFERENCE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/container/implements/any/fwd.hpp>
#include <rainy/foundation/container/implements/any/raii_manager.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26439)
#endif

#define RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(use_operator)                                                                 \
    friend bool use_operator(const any_reference &a, const any_reference &b) {                                                        \
        return use_operator(static_cast<const BasicAny &>(a), static_cast<const BasicAny &>(b));                                      \
    }                                                                                                                                 \
                                                                                                                                      \
    friend bool use_operator(const any_reference &a, const BasicAny &b) {                                                             \
        return use_operator(static_cast<const BasicAny &>(a), b);                                                                     \
    }                                                                                                                                 \
    friend bool use_operator(const BasicAny &a, const any_reference &b) {                                                             \
        return use_operator(a, static_cast<const BasicAny &>(b));                                                                     \
    }

#define RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB(use_operator, pred)                                                   \
    friend basic_any use_operator(const any_reference &a, const any_reference &b) {                                                   \
        return pred{}(static_cast<const BasicAny &>(a), static_cast<const BasicAny &>(b));                                            \
    }                                                                                                                                 \
                                                                                                                                      \
    friend basic_any use_operator(const any_reference &a, const BasicAny &b) {                                                        \
        return pred{}(static_cast<const BasicAny &>(a), b);                                                                           \
    }                                                                                                                                 \
    friend basic_any use_operator(const BasicAny &a, const any_reference &b) {                                                        \
        return pred{}(a, static_cast<const BasicAny &>(b));                                                                           \
    }

#define RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPOUND_ARITHMETIC_OPERATOR_STUB(use_operator, op)                                        \
    any_reference &use_operator(const basic_any &right) {                                                                             \
        (static_cast<basic_any &>(*this)) op(right);                                                                                  \
        return *this;                                                                                                                 \
    }

namespace rainy::foundation::container::implements {
    template <typename BasicAny>
    class any_reference : private BasicAny {
    public:
        using basic_any = BasicAny;

        any_reference() : basic_any{} {
        }

        /// Construct from an existing basic_any by storing a reference to it.
        /// This allows property accessors to return any_reference for member objects of type any.
        any_reference(basic_any &value) {
            this->storage.ptr = utility::addressof(value);
            this->storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(basic_any &)) |
                                      static_cast<std::uintptr_t>(any_representation::reference);
            this->storage.executer = &any_execution_policy_object<basic_any &, basic_any>;
        }

        /// Construct from a const basic_any by storing a const reference.
        any_reference(const basic_any &value) {
            this->storage.ptr = utility::addressof(value);
            this->storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(const basic_any &)) |
                                      static_cast<std::uintptr_t>(any_representation::reference);
            this->storage.executer = &any_execution_policy_object<const basic_any &, basic_any>;
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<ValueType>, basic_any, any_reference>, int> = 0>
        any_reference(ValueType &&value) : basic_any{std::in_place_type<decltype(value)>, std::forward<ValueType>(value)} {
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<ValueType>, basic_any, any_reference>, int> = 0>
        any_reference(core::internal_construct_tag_t, ValueType &&value) : basic_any{std::in_place_type<type_traits::other_trans::decay_t<ValueType>>, std::forward<ValueType>(value)} {
        }

        any_reference(const any_reference &) = default;

        any_reference(any_reference &&) = default;

        template <typename ValueType>
        any_reference &operator=(ValueType &&value) {
            auto tuple = core::container::make_tuple(this, BasicAny{std::forward<ValueType>(value)});
            this->storage.executer->invoke(any_operation::assign, static_cast<basic_any *>(this), &tuple);
            return *this;
        }

        any_reference &operator=(const any_reference &right) {
            if (utility::addressof(right) == this) {
                return *this;
            }
            auto tuple = core::container::make_tuple(this, static_cast<const basic_any &>(right));
            this->storage.executer->invoke(any_operation::assign, static_cast<basic_any *>(this), &tuple);
            return *this;
        }

        any_reference &operator=(any_reference &&right) { // NOLINT
            if (utility::addressof(right) == this) {
                return *this;
            }
            auto tuple = core::container::make_tuple(this, static_cast<basic_any &&>(right));
            this->storage.executer->invoke(any_operation::assign, static_cast<basic_any *>(this), &tuple);
            return *this;
        }

        void swap(any_reference &right) noexcept {
            this->swap_value(right);
        }

        friend void swap(any_reference left, any_reference right) noexcept {
            left.swap(right);
        }

        RAINY_NODISCARD bool has_ownership() const noexcept {
            return false;
        }

        basic_any construct_from_this() {
            basic_any any{};
            auto tuple = core::container::make_tuple(false, this, &any);
            basic_any::storage.executer->invoke(implements::any_operation::construct_from, static_cast<basic_any *>(this), &tuple);
            return any;
        }

        basic_any construct_from_this() const {
            basic_any any{};
            auto tuple = core::container::make_tuple(true, this, &any);
            basic_any::storage.executer->invoke(implements::any_operation::construct_from, static_cast<basic_any *>(this), &tuple);
            return any;
        }

        template <typename CharType, typename AnyReference,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<AnyReference>, any_reference>, int> = 0>
        friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &left, const AnyReference &right) {
            return left << static_cast<const basic_any &>(right);
        }

        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(operator<);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(operator>);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(operator==);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(operator!=);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(operator<=);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB(operator>=);

        RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB(operator+, std::plus);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB(operator-, std::minus);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB(operator*, std::multiplies);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB(operator%, std::modulus);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB(operator/, std::divides);

        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPOUND_ARITHMETIC_OPERATOR_STUB(operator+=, +=);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPOUND_ARITHMETIC_OPERATOR_STUB(operator-=, -=);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPOUND_ARITHMETIC_OPERATOR_STUB(operator%=, %=);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPOUND_ARITHMETIC_OPERATOR_STUB(operator*=, *=);
        RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPOUND_ARITHMETIC_OPERATOR_STUB(operator/=, /=);

        using basic_any::as;
        using basic_any::begin;
        using basic_any::convert;
        using basic_any::destructure;
        using basic_any::end;
        using basic_any::has_value;
        using basic_any::hash_code;
        using basic_any::inner_decleartion_type;
        using basic_any::insert;
        using basic_any::is;
        using basic_any::is_convertible;
        using basic_any::is_one_of;
        using basic_any::is_one_of_convertible;
        using basic_any::match;
        using basic_any::match_for;
        using basic_any::resize;
        using basic_any::size;
        using basic_any::swap_value;
        using basic_any::target_as_void_ptr;
        using basic_any::type;
        using basic_any::operator[];
    };
}

#undef RAINY_GENERATE_BASIC_ANY_REFERENCE_ARITHMETIC_OPERATOR_STUB
#undef RAINY_GENERATE_BASIC_ANY_REFERENCE_BINARY_OPERATOR_STUB
#undef RAINY_GENERATE_BASIC_ANY_REFERENCE_COMPARE_STUB

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
