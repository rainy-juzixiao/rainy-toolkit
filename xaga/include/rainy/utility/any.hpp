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
#ifndef RAINY_UTILITY_ANY_HPP
#define RAINY_UTILITY_ANY_HPP

// NOLINTBEGIN

#include <rainy/core/core.hpp>
#include <rainy/utility/implements/any/fwd.hpp>
#include <rainy/utility/implements/any/cast.hpp>
#include <rainy/utility/implements/any/execution_policy.hpp>
#include <rainy/utility/implements/any/iteator.hpp>
#include <rainy/utility/implements/any/raii_manager.hpp>
#include <rainy/utility/implements/any/matcher.hpp>

// NOLINTEND

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4715 4702 6011 26439 26495)
#endif

#define RAINY_ANY_CAST_TO_POINTER_NODISCARD                                                                                           \
    RAINY_NODISCARD_MSG("Ignoring the return value of cast_to_pointer<Ty>() might be an invalid call.")
#define RAINY_ANY_AS_NODISCARD RAINY_NODISCARD_MSG("Ignoring the return value of as<Ty>() might be an invalid call.")

namespace rainy::utility {
    /**
     * @brief 一个混合所有权的动态变量容器
     * @brief
     * 通常用于对不确定的变量或引用进行运算符或访问操作。同时，因其混合所有权的特性，在持有某个对象的时候，basic_any将会采用RAII策略，反之则仅保留使用权，因此，basic_any有大量功能不同于std::any
     * @param Length 指定any小对象的缓冲区长度（一般留空）
     * @param Align 指定any小对象的对齐长度（一般留空）
     */
    template <std::size_t Length, std::size_t Align>
    class basic_any {
    public:
        static_assert(Length != 0, "What? You serious? Why are you input a 0 for Length, It's illegal!");

        
        friend class implements::any_reference<basic_any>;

        template <typename... Handlers>
        using matcher = implements::any_matcher<basic_any,Handlers...>;

        using iterator = implements::any_iterator_impl<basic_any>;
        using const_iterator = iterator;
        using reference = implements::any_reference<basic_any>;
        using const_reference = implements::any_reference<basic_any>;

        constexpr basic_any() noexcept {
        }

        RAINY_INLINE basic_any(const basic_any &right) {
            using namespace implements;
            switch (right.get_representation()) {
                case any_representation::_small:
                    right.storage.small_any_raii_manager->copy_(&storage.buffer, &right.storage.buffer);
                    storage.small_any_raii_manager = right.storage.small_any_raii_manager;
                    break;
                case any_representation::big:
                    storage.ptr = right.storage.big_any_raii_manager->copy_(right.storage.ptr);
                    storage.big_any_raii_manager = right.storage.big_any_raii_manager;
                    break;
                case any_representation::reference:
                    storage.ptr = right.storage.ptr;
                    break;
                default:
                    break;
            }
            storage.executer = right.storage.executer;
            storage.type_data = right.storage.type_data;
        }

        RAINY_INLINE basic_any(basic_any &&right) noexcept {
            move_from(right);
        }

        template <typename ValueType, type_traits::other_trans::enable_if_t<
                                          type_traits::logical_traits::conjunction_v<
                                              type_traits::logical_traits::negation<type_traits::type_relations::is_any_of<
                                                  type_traits::other_trans::decay_t<ValueType>, basic_any, reference>>,
                                              type_traits::logical_traits::negation<type_traits::primary_types::is_specialization<
                                                  type_traits::other_trans::decay_t<ValueType>, std::in_place_type_t>>,
                                              type_traits::type_properties::is_copy_constructible<ValueType>,
                                              type_traits::logical_traits::negation<is_any<ValueType>>>,
                                          int> = 0>
        basic_any(ValueType &&value) { // NOLINT
            emplace_<ValueType>(utility::forward<ValueType>(value));
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<ValueType, Types...> &&
                          !type_traits::type_relations::is_any_of_v<ValueType, basic_any, reference> && !is_any_v<ValueType>,
                      int> = 0>
        basic_any(std::in_place_type_t<ValueType>, Types &&...args) { // NOLINT
            emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<ValueType, std::initializer_list<Elem> &, Types...> &&
                          !type_traits::type_relations::is_any_of_v<ValueType, basic_any, reference> && !is_any_v<ValueType>,
                      int> = 0>
        explicit basic_any(std::in_place_type_t<ValueType>, std::initializer_list<Elem> ilist, Types &&...args) {
            emplace_<ValueType>(ilist, utility::forward<Types>(args)...);
        }

        basic_any(reference right) : basic_any{right.construct_from_this()} { // NOLINT
        }

        RAINY_INLINE ~basic_any() {
            reset();
        }

        basic_any &operator=(const basic_any &right) {
            basic_any tmp = right;
            reset_and_move_from(tmp);
            return *this;
        }

        basic_any &operator=(basic_any &&right) noexcept {
            basic_any tmp = utility::move(right);
            reset_and_move_from(tmp);
            return *this;
        }

        template <typename ValueType, type_traits::other_trans::enable_if_t<
                                          !type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<ValueType>,
                                                                                    basic_any, reference, const_reference>,
                                          int> = 0>
        basic_any &operator=(ValueType &&value) {
            basic_any tmp = utility::forward<ValueType>(value);
            reset_and_move_from(tmp);
            return *this;
        }

        template <
            typename ValueType, typename... Types,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<ValueType, Types...>, int> = 0>
        decltype(auto) emplace(Types &&...args) {
            reset();
            return emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<ValueType, std::initializer_list<Elem> &, Types...>, int> = 0>
        decltype(auto) emplace(std::initializer_list<Elem> ilist, Types &&...args) {
            reset();
            return emplace_<ValueType>(ilist, utility::forward<Types>(args)...);
        }

        void reset() {
            using namespace implements;
            if (!has_value()) {
                return;
            }
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_any_raii_manager->destroy_(&storage.buffer);
                    break;
                case any_representation::big:
                    storage.big_any_raii_manager->destory_(const_cast<void *>(storage.ptr));
                    break;
                case any_representation::reference:
                default:
                    break;
            }
            storage.type_data = 0;
        }

        void swap(basic_any &right) noexcept {
            basic_any tmp = utility::move(*this);
            reset_and_move_from(right);
            right.reset_and_move_from(tmp);
        }

        RAINY_NODISCARD bool has_value() const noexcept {
            return storage.type_data != 0;
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &type() const noexcept {
            const foundation::ctti::typeinfo *const info = type_info();
            return info != nullptr ? *info : rainy_typeid(void);
        }

        template <typename Type>
        RAINY_ANY_AS_NODISCARD auto as() -> decltype(auto) {
            static constexpr auto location = source_location::current();
            utility::throw_exception_if<foundation::exceptions::cast::bad_any_cast>(implements::is_as_runnable<Type>(type()),
                                                                                    location);
            return implements::as_impl<Type>(target_as_void_ptr(), type());
        }

        template <typename Type>
        RAINY_ANY_AS_NODISCARD auto as() const -> decltype(auto) {
            using namespace type_traits::cv_modify;
            using ret_type = decltype(utility::declval<basic_any &>().template as<Type>());
            rainy_let nonconst = const_cast<basic_any *>(this);
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ret_type>) {
                return nonconst->template as<type_traits::reference_modify::add_const_rvalue_ref_t<Type>>();
            } else {
                return nonconst->template as<type_traits::reference_modify::add_const_lvalue_ref_t<Type>>();
            }
        }

        template <typename Decayed>
        RAINY_ANY_CAST_TO_POINTER_NODISCARD rain_fn cast_to_pointer() const noexcept
            -> type_traits::pointer_modify::add_pointer_t<type_traits::cv_modify::add_const_t<
                type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                                        type_traits::reference_modify::remove_reference_t<Decayed> *, Decayed *>>> {
            using namespace type_traits;
            using type = other_trans::conditional_t<composite_types::is_reference_v<Decayed>,
                                                    reference_modify::remove_reference_t<Decayed>, Decayed>;
            if (!implements::is_as_runnable<type>(this->type())) {
                return nullptr;
            }
            return static_cast<const type *>(target_as_void_ptr());
        }

        template <typename Decayed>
        RAINY_ANY_CAST_TO_POINTER_NODISCARD rain_fn cast_to_pointer() noexcept -> type_traits::pointer_modify::add_pointer_t<
            type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                                    type_traits::reference_modify::remove_reference_t<Decayed>, Decayed>> {
            using namespace type_traits;
            using type = other_trans::conditional_t<composite_types::is_reference_v<Decayed>,
                                                    reference_modify::remove_reference_t<Decayed>, Decayed>;
            if (!implements::is_as_runnable<type>(this->type())) {
                return nullptr;
            }
            return static_cast<type *>(const_cast<void *>(target_as_void_ptr()));
        }

        template <typename TargetType>
        basic_any &transform() {
            if (is<TargetType>()) {
                basic_any(std::in_place_type<TargetType>, this->template as<TargetType>()).swap(*this);
                return *this;
            }
            if (is_convertible<TargetType>()) {
                basic_any(std::in_place_type<TargetType>, this->template convert<TargetType>()).swap(*this);
            }
            return *this;
        }

        template <typename Fx>
        basic_any &transform(Fx &&handler) {
            using namespace type_traits;
            using type_list = typename other_trans::tuple_like_to_type_list<primary_types::param_list_in_tuple<Fx>>::type;
            using target_type = typename other_trans::type_at<0, type_list>::type;
            if (is<target_type>()) {
                basic_any(utility::invoke(handler, this->template as<target_type>())).swap(*this);
            } else if (is_convertible<target_type>()) {
                basic_any(utility::invoke(handler, this->template convert<target_type>())).swap(*this);
            }
            return *this;
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() {
            using foundation::ctti::traits;
            if (type().is_compatible(rainy_typeid(TargetType))) {
                return as<TargetType>();
            }
            if constexpr (is_any_convert_invocable<TargetType>) {
                return any_converter<TargetType>::basic_convert(this->target_as_void_ptr(), this->type());
            } else {
                foundation::exceptions::cast::throw_bad_any_cast();
            }
            std::terminate();
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() const {
            return const_cast<basic_any *>(this)->convert<TargetType>();
        }

        template <typename TargetType>
        RAINY_NODISCARD bool is_convertible() const noexcept {
            return is_any_convertible<TargetType>(this->type());
        }

        RAINY_NODISCARD const void *target_as_void_ptr() const {
            const void *ptr = nullptr;
            switch (get_representation()) {
                case implements::any_representation::big:
                case implements::any_representation::reference:
                    ptr = reinterpret_cast<const void *>(storage.ptr);
                    break;
                case implements::any_representation::_small:
                    ptr = reinterpret_cast<const void *const>(&storage.buffer);
                    break;
            }
            return ptr;
        }

        friend bool operator<(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::less};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator<=(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::less_eq};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator==(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::eq};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator>=(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::greater_eq};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator>(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::greater};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator!=(const basic_any &left, const basic_any &right) {
            return !(left == right);
        }

        friend basic_any operator+(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::add, &tuple);
            return recv;
        }

        friend basic_any operator-(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::subtract, &tuple);
            return recv;
        }

        friend basic_any operator%(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::mod, &tuple);
            return recv;
        }

        friend basic_any operator*(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::multiply, &tuple);
            return recv;
        }

        friend basic_any operator/(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::divide, &tuple);
            return recv;
        }

        basic_any operator--() {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::decr_prefix, &tuple);
            return recv;
        }

        basic_any operator++() {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::incr_prefix, &tuple);
            return recv;
        }

        basic_any operator++(int) {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::incr_postfix, &tuple);
            return recv;
        }

        basic_any operator--(int) {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::decr_postfix, &tuple);
            return recv;
        }

        basic_any operator*() const {
            basic_any recv;
            std::tuple tuple{true, this, &recv};
            storage.executer->invoke(implements::any_operation::dereference, &tuple);
            return recv;
        }

        basic_any &operator+=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::add, &tuple);
            return *this;
        }

        basic_any &operator-=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::subtract, &tuple);
            return *this;
        }

        basic_any &operator/=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::divide, &tuple);
            return *this;
        }

        basic_any &operator%=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::mod, &tuple);
            return *this;
        }

        basic_any &operator*=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::multiply, &tuple);
            return *this;
        }

        reference operator[](std::size_t index) {
            reference ret;
            basic_any the_index{std::in_place_type<std::size_t>, index};
            std::tuple tuple{false, this, &ret, &the_index};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        const_reference operator[](std::size_t index) const {
            const_reference ret;
            basic_any the_index{std::in_place_type<std::size_t>, index};
            std::tuple tuple{true, this, &ret, &the_index};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        reference operator[](const basic_any &key) {
            reference ret;
            std::tuple tuple{false, this, &ret, &key};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        const_reference operator[](const basic_any &key) const {
            const_reference ret;
            std::tuple tuple{true, this, &ret, &key};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        RAINY_NODISCARD iterator begin() {
            iterator ret{};
            std::tuple tuple{false, this, &ret};
            storage.executer->invoke(implements::any_operation::call_begin, &tuple);
            return ret;
        }

        RAINY_NODISCARD const_iterator begin() const {
            const_iterator ret{};
            std::tuple tuple{true, this, &ret};
            storage.executer->invoke(implements::any_operation::call_begin, &tuple);
            return ret;
        }

        RAINY_NODISCARD iterator end() {
            iterator ret{};
            std::tuple tuple{false, this, &ret};
            storage.executer->invoke(implements::any_operation::call_end, &tuple);
            return ret;
        }

        RAINY_NODISCARD const_iterator end() const {
            const_iterator ret{};
            std::tuple tuple{true, this, &ret};
            storage.executer->invoke(implements::any_operation::call_end, &tuple);
            return ret;
        }

        /**
         * @brief 获取any存储的当前类型变量对应的哈希值
         * @attention 需当前类型支持计算哈希
         * @attention 优先通过std::hash求值，如果std::hash不可用，则使用rainy::utility::hash作为哈希支持
         */
        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            std::size_t ret{};
            std::tuple tuple{this, &ret};
            if (storage.executer->invoke(implements::any_operation::eval_hash, &tuple)) {
                return ret;
            }
            std::terminate();
        }

        template <typename... Fx>
        basic_any match(Fx &&...funcs) const noexcept((type_traits::primary_types::function_traits<Fx>::is_noexcept && ...)) {
            return matcher<Fx...>{utility::forward<Fx>(funcs)...}.invoke(*this);
        }

        template <typename Rx, typename... Fx>
        Rx match(Fx &&...funcs) const {
            return matcher<Fx...>{utility::forward<Fx>(funcs)...}.invoke(*this).template convert<Rx>();
        }

        template <typename... Types, typename... Fx>
        std::variant<Types...> match_for(Fx &&...funcs) const {
            auto res = match(utility::forward<Fx>(funcs)...);
            return match_variant_helper<0, std::variant<Types...>, type_traits::other_trans::type_list<Types...>>(res);
        }

        template <typename... Fx>
        auto match_for(auto_deduce_t, Fx &&...funcs) const -> std::variant<type_traits::primary_types::function_return_type<Fx>...> {
            using namespace type_traits::other_trans;
            using namespace type_traits::primary_types;
            static_assert((!type_traits::type_relations::is_void_v<function_return_type<Fx>> && ...),
                          "Cannot accept a void-ret type functions.");
            using auto_deduce_type_list = type_list<function_return_type<Fx>...>;
            using variant_type = typename type_list_to_tuple_like<auto_deduce_type_list, std::variant>::type;
            auto res = match(utility::forward<Fx>(funcs)...);
            return match_variant_helper<0, variant_type, auto_deduce_type_list>(res);
        }

        template <typename Ty>
        bool destructure(Ty &&receiver) {
            return destructure_impl<false>(utility::forward<Ty>(receiver));
        }

        template <typename Ty>
        bool destructure(Ty &&receiver) const {
            return destructure_impl<true>(utility::forward<Ty>(receiver));
        }

        template <typename... Types>
        std::tuple<Types...> destructure() {
            std::tuple<Types...> ret = {};
            this->destructure(ret);
            return ret;
        }

        template <typename... Types>
        std::tuple<Types...> destructure() const {
            std::tuple<Types...> ret = {};
            this->destructure(ret);
            return ret;
        }

        template <typename CharType, typename Any,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Any>, basic_any>, int> = 0>
        friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &left, const Any &right) {
            if (!right.has_value()) {
                return left;
            }
            constexpr bool is_char = type_traits::type_relations::is_same_v<CharType, char>;
            std::tuple<std::basic_ostream<CharType> * /* ostream */, const basic_any * /* any */> params{&left, &right};
            std::tuple<bool /* is_char/is_wchar_t */, void * /* params */> tuple{is_char, &params};
            if (const bool ok = right.storage.executer->invoke(implements::any_operation::output_any, &tuple); !ok) {
                left.setstate(std::ios::ios_base::failbit);
            }
            return left;
        }

        template <
            typename CharType, typename AnyReference,
            type_traits::other_trans::enable_if_t<
                type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<AnyReference>, reference, const_reference>,
                int> = 0>
        friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &left, const AnyReference &right) {
            return left << right.as_value();
        }

        template <typename Type>
        RAINY_NODISCARD bool is() const noexcept {
            return type() == rainy_typeid(Type);
        }

        template <typename... Types>
        RAINY_NODISCARD bool is_one_of() const noexcept {
            return (is<Types>() || ...);
        }

        template <typename... Types>
        RAINY_NODISCARD bool is_one_of_convertible() const noexcept {
            return (is_convertible<Types>() || ...);
        }

        RAINY_NODISCARD bool has_ownership() const noexcept {
            return get_representation() != implements::any_representation::reference && has_value();
        }

        RAINY_NODISCARD reference as_reference() noexcept {
            return as_ref_impl<implements::any_operation::get_reference>();
        }

        RAINY_NODISCARD const_reference as_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_reference>();
        }

        RAINY_NODISCARD const_reference as_const_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_reference>();
        }

        RAINY_NODISCARD reference as_lvalue_reference() noexcept {
            return as_ref_impl<implements::any_operation::get_lvalue_reference>();
        }

        RAINY_NODISCARD const_reference as_lvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_lvalue_reference>();
        }

        RAINY_NODISCARD const_reference as_const_lvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_lvalue_reference>();
        }

        reference as_rvalue_reference() noexcept {
            return as_ref_impl<implements::any_operation::get_rvalue_reference>();
        }

        RAINY_NODISCARD const_reference as_rvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_rvalue_reference>();
        }

        RAINY_NODISCARD const_reference as_const_rvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_rvalue_reference>();
        }

        void swap_value(basic_any &reference) {
            auto tuple = std::make_tuple(this, &reference);
            storage.executer->invoke(implements::any_operation::swap_value, &tuple);
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        template <implements::any_operation Operation>
        reference as_ref_impl() noexcept {
            reference reference;
            auto tuple = std::make_tuple(true, this, &reference);
            storage.executer->invoke(Operation, &tuple);
            return reference;
        }

        template <implements::any_operation Operation>
        RAINY_NODISCARD const_reference as_ref_impl() const noexcept {
            const_reference reference;
            auto tuple = std::make_tuple(true, this, &reference);
            storage.executer->invoke(Operation, &tuple);
            return reference;
        }

        template <bool UseConst, typename Ty>
        bool destructure_impl(Ty &&receiver) const {
            using implements::any_binding_package;
            using namespace type_traits;
            using namespace type_traits::primary_types;
            static_assert(!type_traits::type_properties::is_const_v<Ty>);
            constexpr std::size_t size = implements::eval_for_destructure_pack_receiver_size<Ty>();
            static_assert(size != 0, "Cannot process a invalid receiver!");
            collections::array<any_binding_package, size> array;
            std::size_t count{};
            bool ret = storage.executer->invoke(implements::any_operation::query_for_is_tuple_like, &count);
            if (!ret || count != size) {
                return false;
            }
            std::tuple tuple{this, UseConst, collections::views::make_array_view(array)};
            ret = storage.executer->invoke(implements::any_operation::destructre_this_pack, &tuple);
            if (!ret) {
                return false;
            }
            if constexpr (function_traits<Ty>::valid && !is_member_object_pointer_v<Ty>) {
                this->template call_handler_with_array<UseConst>(utility::forward<Ty>(receiver), array,
                                                                 type_traits::helper::make_index_sequence<size>{});
                return true;
            } else if constexpr (is_pair_v<Ty>) {
                this->template fill_pair_with_array<UseConst>(utility::forward<Ty>(receiver), array);
                return true;
            } else if constexpr (is_tuple_v<Ty>) {
                this->template fill_tuple_with_array<UseConst>(utility::forward<Ty>(receiver), array,
                                                               type_traits::helper::make_index_sequence<size>{});
                return true;
            } else if constexpr (member_count_v<cv_modify::remove_cvref_t<Ty>> != 0) {
                auto so_as_tuple = utility::struct_bind_tuple(receiver);
                this->template fill_structure_with_array<UseConst>(so_as_tuple, array,
                                                                   type_traits::helper::make_index_sequence<size>{});
                return true;
            }
            return false;
        }

        RAINY_NODISCARD implements::any_representation get_representation() const noexcept {
            return static_cast<implements::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo *type_info() const noexcept {
            return reinterpret_cast<const foundation::ctti::typeinfo *>(storage.type_data & ~rep_mask);
        }

        template <bool Const, typename Fx, std::size_t N, std::size_t... Is>
        void call_handler_with_array(Fx &&handler, const collections::array<implements::any_binding_package, N> &array,
                                     type_traits::helper::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using fn_traits = type_traits::primary_types::function_traits<Fx>;
            using type_list = typename tuple_like_to_type_list<typename fn_traits::tuple_like_type>::type;
            utility::invoke(
                utility::forward<Fx>(handler),
                utility::forward<conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                               typename type_at<Is, type_list>::type>>(
                    implements::convert_any_binding_package<
                        conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                      typename type_at<Is, type_list>::type>,
                        Is>::impl(array[Is]))...);
        }

        template <bool Const, std::size_t N, typename Tuple, std::size_t... Is>
        void fill_tuple_with_array(Tuple &tuple, const collections::array<implements::any_binding_package, N> &array,
                                   type_traits::helper::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using utility::swap;
            std::destroy_at(&tuple);
            using type_list = typename tuple_like_to_type_list<type_traits::cv_modify::remove_cvref_t<Tuple>>::type;
            utility::construct_in_place(
                tuple,
                utility::forward<conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                               typename type_at<Is, type_list>::type>>(
                    implements::convert_any_binding_package<
                        conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                      typename type_at<Is, type_list>::type>,
                        Is>::impl(array[Is]))...);
        }


        template <bool Const, std::size_t N, typename Tuple, std::size_t... Is>
        void fill_structure_with_array(Tuple &so_as_tuple, const collections::array<implements::any_binding_package, N> &array,
                                       type_traits::helper::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using utility::swap;
            Tuple tmp{so_as_tuple};
            (((*std::get<Is>(tmp)) = implements::convert_any_binding_package<
                  type_traits::other_trans::conditional_t<
                      Const, type_traits::cv_modify::add_const_t<std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>>,
                      std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>>,
                  Is>::impl(array[Is])),
             ...);
            swap(tmp, so_as_tuple);
        }

        template <bool Const, typename Pair, std::size_t... Is>
        void fill_pair_with_array(Pair &pair, const collections::array<implements::any_binding_package, 2> &array) const {
            using implements::convert_any_binding_package;
            Pair tmp{};
            auto &[first, second] = tmp; // 从pair中解包
            using first_type = decltype(first);
            using second_type = decltype(second);
            static_assert(type_traits::type_properties::is_copy_assignable_v<first_type>,
                          "The first element of pair-like type is not assignable");
            static_assert(type_traits::type_properties::is_copy_assignable_v<second_type>,
                          "The second element of pair-like type is not assignable");
            first = implements::convert_any_binding_package<
                type_traits::other_trans::conditional_t<Const, type_traits::cv_modify::add_const_t<first_type>, first_type>,
                0>::impl(array[0]);
            second = implements::convert_any_binding_package<
                type_traits::other_trans::conditional_t<Const, type_traits::cv_modify::add_const_t<second_type>, second_type>,
                1>::impl(array[1]);
            std::swap(tmp, pair);
        }

        template <std::size_t Idx = 0, typename Variant, typename TypeList>
        auto match_variant_helper(const basic_any &res) const {
            if constexpr (Idx < type_traits::other_trans::type_list_size_v<TypeList>) {
                using type = typename type_traits::other_trans::type_at<Idx, TypeList>::type;
                if (res.template is<type>()) {
                    return Variant{res.template as<type>()};
                }
                if (res.template is_convertible<type>()) {
                    return Variant{res.template convert<type>()};
                }
                return match_variant_helper<Idx + 1, Variant, TypeList>(res);
            } else {
                return Variant{};
            }
        }

        void move_from(basic_any &right) noexcept {
            using namespace implements;
            storage.type_data = right.storage.type_data;
            storage.executer = right.storage.executer;
            switch (right.get_representation()) {
                case any_representation::_small:
                    storage.small_any_raii_manager = right.storage.small_any_raii_manager;
                    storage.small_any_raii_manager->move_(&storage.buffer, &right.storage.buffer);
                    break;
                case any_representation::big:
                    storage.big_any_raii_manager = right.storage.big_any_raii_manager;
                    storage.ptr = right.storage.ptr;
                    right.storage.type_data = 0;
                    break;
                case any_representation::reference:
                    storage.ptr = right.storage.ptr;
                    right.storage.type_data = 0;
                    break;
                default:
                    break;
            }
        }

        RAINY_INLINE void reset_and_move_from(basic_any &right) noexcept {
            reset();
            move_from(right);
        }

        template <typename Decayed, typename... Types>
        RAINY_INLINE decltype(auto) emplace_(Types &&...args) {
            using namespace type_traits;
            using decayed = other_trans::decay_t<Decayed>;
            if constexpr (composite_types::is_reference_v<Decayed>) {
                using remove_reference = reference_modify::remove_reference_t<Decayed>;
                if constexpr (!primary_types::is_array_v<remove_reference>) {
                    return emplace_ref<Decayed>(utility::forward<Types>(args)...);
                } else {
                    return emplace_<decayed>(utility::forward<Types>(args)...);
                }
            } else if constexpr (implements::any_is_small<decayed, sizeof(storage_t::buffer)>) {
                auto &object = reinterpret_cast<decayed &>(storage.buffer);
                storage.small_any_raii_manager = &implements::any_small_raii_manager_object<decayed>;
                ::new (utility::addressof(object)) decayed(utility::forward<Types>(args)...);
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::_small);
                storage.executer = &implements::any_execution_policy_object<decayed, basic_any>;
                return object;
            } else {
                rainy_let new_alloc_ptr = ::new decayed(utility::forward<Types>(args)...);
                storage.ptr = new_alloc_ptr;
                storage.big_any_raii_manager = &implements::any_big_raii_manager_object<decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::big);
                storage.executer = &implements::any_execution_policy_object<decayed, basic_any>;
                return *new_alloc_ptr;
            }
        }

        template <typename ReferenceType, typename Type>
        RAINY_INLINE decltype(auto) emplace_ref(Type &&reference) {
            storage.ptr = utility::addressof(reference);
            storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(ReferenceType)) |
                                static_cast<std::uintptr_t>(implements::any_representation::reference);
            storage.executer = &implements::any_execution_policy_object<Type, basic_any>;
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ReferenceType>) {
                return utility::move(reference);
            } else {
                return reference;
            }
        }

        struct storage_t {
            union {
                const void *ptr;
                alignas(Align) core::byte_t buffer[Length];
            };
            union {
                const implements::big_any_raii_manager *big_any_raii_manager;
                const implements::small_any_raii_manager *small_any_raii_manager;
            };
            const implements::any_execution_policy *executer;
            std::uintptr_t type_data;
        };

        union {
            std::max_align_t dummy;
            storage_t storage{};
        };
    };

    /**
     * @brief 一个混合所有权的动态变量容器
     * @brief
     * 通常用于对不确定的变量或引用进行运算符或访问操作。同时，因其混合所有权的特性，在持有某个对象的时候，basic_any将会采用RAII策略，反之则仅保留使用权，因此，basic_any有大量功能不同于std::any
     */
    using any = basic_any<>;
}

namespace rainy::utility {
    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, Args...>, int> = 0>
    RAINY_NODISCARD any make_any(Args &&...args) {
        return any{std::in_place_type<Ty>, utility::forward<Args>(args)...};
    }

    template <typename Ty, typename U, typename... Args,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, std::initializer_list<U> &, Args...>,
                  int> = 0>
    RAINY_NODISCARD any make_any(std::initializer_list<U> initializer_list, Args &&...args) {
        return any{std::in_place_type<Ty>, initializer_list, utility::forward<Args>(args)...};
    }

    template <std::size_t Length, std::size_t Align>
    void swap(basic_any<Length, Align> &left, basic_any<Length, Align> &right) noexcept {
        left.swap(right);
    }
}

namespace std {
    template <std::size_t Length, std::size_t Align>
    void swap(::rainy::utility::basic_any<Length, Align> &left, ::rainy::utility::basic_any<Length, Align> &right) noexcept { // NOLINT
        left.swap(right);
    }
}

namespace rainy::utility {
    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::composite_types::is_reference_v<TargetType> ||
                                                               type_traits::primary_types::is_pointer_reference_v<TargetType>>> {
        static decltype(auto) basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            return implements::as_impl<TargetType>(target_pointer, type);
        }

        static bool is_convertible(const foundation::ctti::typeinfo &type) {
            return implements::is_as_runnable<TargetType>(type);
        }
    };

    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::composite_types::is_arithmetic_v<TargetType>>> {
        static TargetType basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            switch (const foundation::ctti::typeinfo target_type = type; target_type.remove_cvref().hash_code()) {
                case rainy_typehash(int):
                    return static_cast<TargetType>(*static_cast<const int *>(target_pointer));
                case rainy_typehash(char):
                    return static_cast<TargetType>(*static_cast<const char *>(target_pointer));
                case rainy_typehash(float):
                    return static_cast<TargetType>(*static_cast<const float *>(target_pointer));
                case rainy_typehash(double):
                    return static_cast<TargetType>(*static_cast<const double *>(target_pointer));
                case rainy_typehash(long):
                    return static_cast<TargetType>(*static_cast<const long *>(target_pointer));
                case rainy_typehash(bool):
                    return static_cast<TargetType>(*static_cast<const bool *>(target_pointer));
                case rainy_typehash(long long):
                    return static_cast<TargetType>(*static_cast<const long long *>(target_pointer));
                case rainy_typehash(short):
                    return static_cast<TargetType>(*static_cast<const short *>(target_pointer));
                case rainy_typehash(std::int8_t):
                    return static_cast<TargetType>(*static_cast<const std::int8_t *>(target_pointer));
                case rainy_typehash(std::uint8_t):
                    return static_cast<TargetType>(*static_cast<const std::uint8_t *>(target_pointer));
                case rainy_typehash(std::uint16_t):
                    return static_cast<TargetType>(*static_cast<const std::uint16_t *>(target_pointer));
                case rainy_typehash(std::uint32_t):
                    return static_cast<TargetType>(*static_cast<const std::uint32_t *>(target_pointer));
                case rainy_typehash(std::uint64_t):
                    return static_cast<TargetType>(*static_cast<const std::uint64_t *>(target_pointer));
                default:
                    break;
            }
            foundation::exceptions::cast::throw_bad_any_cast();
            std::terminate();
        }

        static bool is_convertible(const foundation::ctti::typeinfo &type) {
            switch (const foundation::ctti::typeinfo target_type = type; target_type.remove_cvref().hash_code()) {
                case rainy_typehash(int):
                case rainy_typehash(char):
                case rainy_typehash(float):
                case rainy_typehash(double):
                case rainy_typehash(long):
                case rainy_typehash(bool):
                case rainy_typehash(long long):
                case rainy_typehash(short):
                case rainy_typehash(std::int8_t):
                case rainy_typehash(std::uint8_t):
                case rainy_typehash(std::uint16_t):
                case rainy_typehash(std::uint32_t):
                case rainy_typehash(std::uint64_t):
                    return true;
                default:
                    return false;
            }
        }
    };

    template <typename CharType, typename Traits>
    struct any_converter<std::basic_string_view<CharType, Traits>> {
        using target_type = std::basic_string_view<CharType, Traits>;

        static target_type basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            using namespace foundation::exceptions::cast;
            using const_pointer = const CharType *;
            using pointer = CharType *;
            using pointer_to_const = const CharType *const;
            using same_type = std::basic_string_view<CharType, Traits>;
            using same_type_with_const = const std::basic_string_view<CharType, Traits>;
            using basic_string_t = std::basic_string<CharType, Traits>;
            using const_basic_string_t = const std::basic_string<CharType, Traits>;
            switch (type.remove_reference().hash_code()) {
                case rainy_typehash(const_pointer):
                case rainy_typehash(pointer):
                case rainy_typehash(pointer_to_const):
                    // return target_type{static_cast<const_pointer>(target_pointer)}; [[deprecated]]
                    return *static_cast<const const_pointer *>(target_pointer);
                case rainy_typehash(same_type):
                case rainy_typehash(same_type_with_const):
                    return *static_cast<const target_type *>(target_pointer);
                case rainy_typehash(basic_string_t):
                    RAINY_FALLTHROUGH;
                case rainy_typehash(const_basic_string_t): {
                    const basic_string_t &str = *static_cast<const basic_string_t *>(target_pointer);
                    return {str.data(), str.size()};
                }
                default:
                    break;
            }
            throw_bad_any_cast();
            std::terminate();
        }

        static bool is_convertible(const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            switch (type.remove_reference().hash_code()) {
                case rainy_typehash(const CharType *):
                case rainy_typehash(CharType *):
                case rainy_typehash(const CharType *const):
                case rainy_typehash(std::basic_string_view<CharType>):
                case rainy_typehash(std::basic_string<CharType>):
                case rainy_typehash(const std::basic_string_view<CharType>):
                case rainy_typehash(const std::basic_string<CharType>):
                    return true;
                default:
                    break;
            }
            return false;
        }
    };
}

namespace std { // NOLINT
    template <std::size_t Length, std::size_t Align>
    struct hash<::rainy::utility::basic_any<Length, Align>> { // NOLINT
        RAINY_NODISCARD std::size_t operator()(const ::rainy::utility::any &right) const {
            return right.hash_code();
        }
    };
}

namespace rainy::utility { // NOLINT
    template <std::size_t Length, std::size_t Align>
    struct hash<basic_any<Length, Align>> {
        using result_type = std::size_t;
        using argument_type = basic_any<Length, Align>;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return val.hash_code();
        }

        RAINY_NODISCARD std::size_t operator()(const argument_type &right) const {
            return right.hash_code();
        }
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#undef RAINY_ANY_AS_NODISCARD
#undef RAINY_ANY_CAST_TO_POINTER_NODISCARD

#endif
