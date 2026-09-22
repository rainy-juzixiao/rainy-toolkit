/*
 * Copyright 2026 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_CONTAINER_ANY_HPP
#define RAINY_FOUNDATION_CONTAINER_ANY_HPP

// NOLINTBEGIN

#include <ostream>

#include <rainy/core/platform.hpp>
#include <rainy/foundation/container/implements/any/cast.hpp>
#include <rainy/foundation/container/implements/any/execution_policy.hpp>
#include <rainy/foundation/container/implements/any/fwd.hpp>
#include <rainy/foundation/container/implements/any/iteator.hpp>
#include <rainy/foundation/container/implements/any/matcher.hpp>
#include <rainy/foundation/container/implements/any/raii_manager.hpp>
#include <rainy/core/container/pair.hpp>

// NOLINTEND

#if RAINY_HAS_CXX20
#include <sstream>
#endif

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4715 4702 6011 26439 26495)
#endif

#define RAINY_ANY_CAST_TO_POINTER_NODISCARD                                                                                           \
    RAINY_NODISCARD_MSG("Ignoring the return value of cast_to_pointer<Ty>() might be an invalid call.")
#define RAINY_ANY_AS_NODISCARD RAINY_NODISCARD_MSG("Ignoring the return value of as<Ty>() might be an invalid call.")

namespace rainy::foundation::container {
    struct auto_deduce_t {
        explicit constexpr auto_deduce_t() = default;
    };

    constexpr inline auto_deduce_t auto_deduce{};
}

namespace rainy::foundation::container {
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
        friend class implements::any_reference<basic_any>;
        friend struct implements::any_execution_policy;

        template <typename... Handlers>
        using matcher = implements::any_matcher<basic_any, Handlers...>;

        using iterator = implements::any_iterator_impl<basic_any>;
        using const_iterator = iterator;
        using reference = implements::any_reference<basic_any>;
        using const_reference = implements::any_reference<basic_any>;

        constexpr basic_any() noexcept {
        }

        RAINY_INLINE basic_any(const basic_any &right) {
            using namespace implements;
            if (!right.has_value()) {
                return;
            }
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

        /**
         * @brief 将目标对象进行移动构造
         * @return 要移动的对象
         */
        RAINY_INLINE basic_any(basic_any &&right) noexcept {
            move_from(right);
        }

        template <typename T>
        basic_any(wrap_any_tag, T &&value) { // NOLINT
            if constexpr (type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<T>, basic_any>) {
                if (!value.has_value()) {
                    return;
                }
            }
            emplace_<basic_any>(utility::forward<T>(value));
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::logical_traits::negation<type_traits::type_relations::is_any_of<
                              type_traits::other_trans::decay_t<ValueType>, basic_any, reference>>,
                          type_traits::logical_traits::negation<type_traits::primary_types::is_specialization<
                              type_traits::other_trans::decay_t<ValueType>, std::in_place_type_t>>,
                          type_traits::properties::is_copy_constructible<ValueType>,
                          type_traits::logical_traits::negation<type_traits::type_relations::is_same<ValueType, basic_any>>>,
                      int> = 0>
        basic_any(ValueType &&value) { // NOLINT
            emplace_<ValueType>(utility::forward<ValueType>(value));
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<ValueType, Types...> &&
                                                            !type_traits::type_relations::is_any_of_v<ValueType, basic_any, reference>,
                                                        int> = 0>
        basic_any(std::in_place_type_t<ValueType>, Types &&...args) { // NOLINT
            emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<ValueType, std::initializer_list<Elem> &, Types...> &&
                          !type_traits::type_relations::is_any_of_v<ValueType, basic_any, reference>,
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

        basic_any &operator=(reference ref) noexcept {
            basic_any tmp = ref.construct_from_this();
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

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<ValueType, Types...>, int> = 0>
        decltype(auto) emplace(Types &&...args) {
            reset();
            return emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_constructible_v<ValueType, std::initializer_list<Elem> &, Types...>, int> = 0>
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

        RAINY_NODISCARD const core::typeinfo &type() const noexcept {
            const core::typeinfo *const info = type_info();
            return info != nullptr ? *info : rainy_typeid(void);
        }

        RAINY_NODISCARD const core::typeinfo &unwrapped_type() const noexcept {
            const core::typeinfo *const info = type_info();
            if (info == nullptr) {
                return rainy_typeid(void);
            }
            // For wrapped basic_any (stored type is basic_any), transparently return the
            // inner basic_any's type so is<T>() / as<T>() see the wrapped value's type.
            if (info->remove_cvref() == rainy_typeid(basic_any)) {
                return static_cast<const basic_any *>(target_as_void_ptr())->type();
            }
            return *info;
        }

        RAINY_INLINE_NODISCARD core::typeinfo inner_decleartion_type(any_inner_declaertion query) const noexcept {
            core::typeinfo type;
            core::container::tuple tuple{query, &type};
            storage.executer->invoke(implements::any_operation::query_inner_declaertion_type, const_cast<basic_any *>(this), &tuple);
            return type;
        }

        template <typename Type>
        RAINY_ANY_AS_NODISCARD auto as() -> decltype(auto) {
            const core::typeinfo *const raw = type_info();
            if (rainy_unlikely(!is<Type>())) {
                if (raw->remove_cvref() == rainy_typeid(basic_any)) {
                    if (!type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Type>, basic_any>) {
                        auto *inner = static_cast<basic_any *>(const_cast<void *>(target_as_void_ptr()));
                        while (true) {
                            const core::typeinfo *ir = inner->type_info();
                            if (ir == nullptr || ir->remove_cvref() != rainy_typeid(basic_any)) {
                                break;
                            }
                            inner = static_cast<basic_any *>(const_cast<void *>(inner->target_as_void_ptr()));
                        }
                        return implements::as_impl<Type>(inner->target_as_void_ptr(), inner->unwrapped_type());
                    }
                    return implements::as_impl<Type>(
                        target_as_void_ptr(),
                        rainy_typeid(type_traits::modifers::remove_cvref_t<type_traits::other_trans::decay_t<Type>>));
                }
                if (!implements::is_as_runnable<Type>(type())) {
                    throw std::bad_cast();
                }
            }
            return implements::as_impl<Type>(target_as_void_ptr(), type());
        }

        template <typename Type>
        RAINY_ANY_AS_NODISCARD auto as() const -> decltype(auto) {
            using ret_type = decltype(std::declval<basic_any &>().template as<Type>());
            rainy_let nonconst = const_cast<basic_any *>(this);
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ret_type>) {
                return nonconst->template as<type_traits::modifers::add_const_rvalue_ref_t<Type>>();
            } else {
                return nonconst->template as<type_traits::modifers::add_const_lvalue_ref_t<Type>>();
            }
        }

        template <typename Decayed>
        RAINY_ANY_CAST_TO_POINTER_NODISCARD rain_fn
        cast_to_pointer() const noexcept -> type_traits::modifers::add_pointer_t<type_traits::modifers::add_const_t<type_traits::other_trans::conditional_t<
            type_traits::composite_types::is_reference_v<Decayed>, type_traits::modifers::remove_reference_t<Decayed> *, Decayed *>>> {
            using type = type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                            type_traits::modifers::remove_reference_t<Decayed>, Decayed>;
            if (!rainy::foundation::container::implements::is_as_runnable<type>(this->type())) {
                return nullptr;
            }
            return static_cast<const type *>(target_as_void_ptr());
        }

        template <typename Decayed>
        RAINY_ANY_CAST_TO_POINTER_NODISCARD rain_fn cast_to_pointer() noexcept
            -> type_traits::modifers::add_pointer_t<type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                                                       type_traits::modifers::remove_reference_t<Decayed>, Decayed>> {
            using type = type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                            type_traits::modifers::remove_reference_t<Decayed>, Decayed>;
            if (!rainy::foundation::container::implements::is_as_runnable<type>(this->type())) {
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
            using type_list = type_traits::primary_types::function_argument_list<Fx>;
            using target_type = typename type_traits::other_trans::type_at_t<0, type_list>;
            if (is<target_type>()) {
                basic_any(std::invoke(handler, this->template as<target_type>())).swap(*this);
            } else if (is_convertible<target_type>()) {
                basic_any(std::invoke(handler, this->template convert<target_type>())).swap(*this);
            }
            return *this;
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() {
            if (type().is_compatible(rainy_typeid(TargetType))) {
                return as<TargetType>();
            }
            if constexpr (is_any_convert_invocable<TargetType>) {
                return utility::any_converter<TargetType>::basic_convert(this->target_as_void_ptr(), this->type());
            } else {
                throw std::bad_cast{};
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

        RAINY_NODISCARD bool is_convertible(const core::typeinfo &type) const noexcept {
            return is_convertible_to(this->type(), type);
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
            if (left.has_value() && right.has_value()) {
                core::container::tuple tuple{&left, &right, implements::any_compare_operation::less};
                return left.storage.executer->invoke(implements::any_operation::compare, const_cast<basic_any *>(&left), &tuple);
            }
            return false;
        }

        friend bool operator<=(const basic_any &left, const basic_any &right) {
            if (left.has_value() && right.has_value()) {
                core::container::tuple tuple{&left, &right, implements::any_compare_operation::less_eq};
                return left.storage.executer->invoke(implements::any_operation::compare, const_cast<basic_any *>(&left), &tuple);
            }
            return false;
        }

        friend bool operator==(const basic_any &left, const basic_any &right) {
            if (left.has_value() && right.has_value()) {
                core::container::tuple tuple{&left, &right, implements::any_compare_operation::eq};
                return left.storage.executer->invoke(implements::any_operation::compare, const_cast<basic_any *>(&left), &tuple);
            }
            return false;
        }

        friend bool operator>=(const basic_any &left, const basic_any &right) {
            if (left.has_value() && right.has_value()) {
                core::container::tuple tuple{&left, &right, implements::any_compare_operation::greater_eq};
                return left.storage.executer->invoke(implements::any_operation::compare, const_cast<basic_any *>(&left), &tuple);
            }
            return false;
        }

        friend bool operator>(const basic_any &left, const basic_any &right) {
            if (left.has_value() && right.has_value()) {
                core::container::tuple tuple{&left, &right, implements::any_compare_operation::greater};
                return left.storage.executer->invoke(implements::any_operation::compare, const_cast<basic_any *>(&left), &tuple);
            }
            return false;
        }

        friend bool operator!=(const basic_any &left, const basic_any &right) {
            if (left.has_value() && right.has_value()) {
                return !(left == right);
            }
            return false;
        }

        friend basic_any operator+(const basic_any &left, const basic_any &right) {
            basic_any recv;
            core::container::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::add, const_cast<basic_any *>(&left), &tuple);
            return recv;
        }

        friend basic_any operator-(const basic_any &left, const basic_any &right) {
            basic_any recv;
            core::container::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::subtract, const_cast<basic_any *>(&left), &tuple);
            return recv;
        }

        friend basic_any operator%(const basic_any &left, const basic_any &right) {
            basic_any recv;
            core::container::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::mod, const_cast<basic_any *>(&left), &tuple);
            return recv;
        }

        friend basic_any operator*(const basic_any &left, const basic_any &right) {
            basic_any recv;
            core::container::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::multiply, const_cast<basic_any *>(&left), &tuple);
            return recv;
        }

        friend basic_any operator/(const basic_any &left, const basic_any &right) {
            basic_any recv;
            core::container::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::divide, const_cast<basic_any *>(&left), &tuple);
            return recv;
        }

        basic_any operator--() {
            basic_any recv;
            core::container::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::decr_prefix, const_cast<basic_any *>(this), &tuple);
            return recv;
        }

        basic_any operator++() {
            basic_any recv;
            core::container::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::incr_prefix, const_cast<basic_any *>(this), &tuple);
            return recv;
        }

        basic_any operator++(int) {
            basic_any recv;
            core::container::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::incr_postfix, const_cast<basic_any *>(this), &tuple);
            return recv;
        }

        basic_any operator--(int) {
            basic_any recv;
            core::container::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::decr_postfix, const_cast<basic_any *>(this), &tuple);
            return recv;
        }

        basic_any operator*() const {
            basic_any recv;
            core::container::tuple tuple{true, this, &recv};
            storage.executer->invoke(implements::any_operation::dereference, const_cast<basic_any *>(this), &tuple);
            return recv;
        }

        basic_any &operator+=(const basic_any &right) {
            core::container::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::add, const_cast<basic_any *>(this), &tuple);
            return *this;
        }

        basic_any &operator-=(const basic_any &right) {
            core::container::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::subtract, const_cast<basic_any *>(this), &tuple);
            return *this;
        }

        basic_any &operator/=(const basic_any &right) {
            core::container::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::divide, const_cast<basic_any *>(this), &tuple);
            return *this;
        }

        basic_any &operator%=(const basic_any &right) {
            core::container::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::mod, const_cast<basic_any *>(this), &tuple);
            return *this;
        }

        basic_any &operator*=(const basic_any &right) {
            core::container::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::multiply, const_cast<basic_any *>(this), &tuple);
            return *this;
        }

        reference operator[](std::size_t index) {
            reference ret;
            basic_any the_index{std::in_place_type<std::size_t>, index};
            core::container::tuple tuple{false, this, &ret, &the_index};
            storage.executer->invoke(implements::any_operation::access_element, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        const_reference operator[](std::size_t index) const {
            const_reference ret;
            basic_any the_index{std::in_place_type<std::size_t>, index};
            core::container::tuple tuple{true, this, &ret, &the_index};
            storage.executer->invoke(implements::any_operation::access_element, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        reference operator[](const basic_any &key) {
            reference ret;
            core::container::tuple tuple{false, this, &ret, &key};
            storage.executer->invoke(implements::any_operation::access_element, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        const_reference operator[](const basic_any &key) const {
            const_reference ret;
            core::container::tuple tuple{true, this, &ret, &key};
            storage.executer->invoke(implements::any_operation::access_element, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        RAINY_NODISCARD iterator begin() {
            iterator ret{};
            core::container::tuple tuple{false, this, &ret};
            storage.executer->invoke(implements::any_operation::container_begin, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        RAINY_NODISCARD const_iterator begin() const {
            const_iterator ret{};
            core::container::tuple tuple{true, this, &ret};
            storage.executer->invoke(implements::any_operation::container_begin, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        RAINY_NODISCARD iterator end() {
            iterator ret{};
            core::container::tuple tuple{false, this, &ret};
            storage.executer->invoke(implements::any_operation::container_end, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        RAINY_NODISCARD const_iterator end() const {
            const_iterator ret{};
            core::container::tuple tuple{true, this, &ret};
            storage.executer->invoke(implements::any_operation::container_end, const_cast<basic_any *>(this), &tuple);
            return ret;
        }

        /**
         * @brief 获取any存储的当前类型变量对应的哈希值
         * @attention 需当前类型支持计算哈希
         * @attention 优先通过std::hash求值，如果std::hash不可用，则使用rainy::foundation::container::hash作为哈希支持
         */
        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            std::size_t ret{};
            core::container::tuple tuple{this, &ret};
            if (storage.executer->invoke(implements::any_operation::eval_hash, const_cast<basic_any *>(this), &tuple)) {
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
            return implements::match_variant_helper<0, std::variant<Types...>, type_traits::other_trans::type_list<Types...>>(res);
        }

        template <typename... Fx>
        auto match_for(auto_deduce_t, Fx &&...funcs) const
            -> core::container::variant<type_traits::primary_types::function_return_type<Fx>...> {
            static_assert((!type_traits::primary_types::is_void_v<type_traits::primary_types::function_return_type<Fx>> && ...),
                          "Cannot accept a void-ret type functions.");
            using auto_deduce_type_list = type_traits::other_trans::type_list<type_traits::primary_types::function_return_type<Fx>...>;
            using variant_type =
                typename type_traits::other_trans::type_list_to_tuple_like<auto_deduce_type_list, core::container::variant>::type;
            auto res = match(utility::forward<Fx>(funcs)...);
            return implements::match_variant_helper<0, variant_type, auto_deduce_type_list>(res);
        }

        template <typename Ty>
        bool destructure(Ty &&receiver) {
            return implements::destructure_impl<false>(this, storage.executer, utility::forward<Ty>(receiver));
        }

        template <typename Ty>
        bool destructure(Ty &&receiver) const {
            return implements::destructure_impl<true>(this, storage.executer, utility::forward<Ty>(receiver));
        }

        template <typename... Types>
        core::container::tuple<Types...> destructure() {
            core::container::tuple<Types...> ret = {};
            this->destructure(ret);
            return ret;
        }

        template <typename... Types>
        core::container::tuple<Types...> destructure() const {
            core::container::tuple<Types...> ret = {};
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
            core::container::tuple<std::basic_ostream<CharType> * /* ostream */, const basic_any * /* basic_any */> params{&left,
                                                                                                                           &right};
            core::container::tuple<bool /* is_char/is_wchar_t */, void * /* params */> tuple{is_char, &params};
            if (const bool ok =
                    right.storage.executer->invoke(implements::any_operation::output_any, const_cast<basic_any *>(&right), &tuple);
                !ok) {
                left.setstate(std::ios::ios_base::failbit);
            }
            return left;
        }

        /**
         * \lang english
         * @brief Formats the stored value into out, following the specification held by parse_context.
         *
         * @param parse_context The context carrying the format specification
         * @param out The string the formatted value is appended to
         * @return true when the stored type is formattable, false otherwise
         *
         * \lang simp-chinese
         * @brief 按照 parse_context 携带的格式规范，把所存的值格式化到 out。
         *
         * @param parse_context 携带格式规范的上下文
         * @param out 格式化结果追加到的字符串
         * @return 所存类型可格式化时返回 true，否则返回 false
         */
        template <typename CharType>
        bool format(core::text::basic_format_parse_context<CharType> &parse_context, core::text::basic_string<CharType> &out) const {
            if (!has_value()) {
                return false;
            }
            core::container::tuple tuple{static_cast<void *>(&parse_context), static_cast<void *>(&out),
                                         type_traits::type_relations::is_same_v<CharType, wchar_t>};
            return storage.executer->invoke(implements::any_operation::format, const_cast<basic_any *>(this), &tuple);
        }

        template <typename Type>
        RAINY_NODISCARD bool is() const noexcept { // NOLINT
            return type() == rainy_typeid(Type);
        }

        template <typename Type>
        RAINY_NODISCARD bool unwraped_is() const noexcept { // NOLINT
            if (type() == rainy_typeid(Type)) {
                return true;
            }
            if (type().remove_cvref() == rainy_typeid(basic_any)) {
                return static_cast<const basic_any *>(target_as_void_ptr())->template unwraped_is<Type>();
            }
            return false;
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
            auto tuple = core::container::make_tuple(this, &reference);
            storage.executer->invoke(implements::any_operation::swap_value, const_cast<basic_any *>(this), &tuple);
        }

        const_iterator insert(const const_iterator &pos, const basic_any &value) {
            const_iterator iterator;
            core::container::tuple tuple{this, &iterator, &pos, &value};
            storage.executer->invoke(implements::any_operation::container_insert_seq_like, const_cast<basic_any *>(this), &tuple);
            return iterator;
        }

        core::container::pair<const_iterator, bool> insert_as_maplike(const basic_any &key) {
            core::container::pair<const_iterator, bool> ret_pair;
            core::container::tuple tuple{this, &ret_pair, &key, nullptr};
            storage.executer->invoke(implements::any_operation::container_insert_map_like, const_cast<basic_any *>(this), &tuple);
            return ret_pair;
        }

        core::container::pair<const_iterator, bool> insert_as_maplike(const basic_any &key, const basic_any &value) {
            core::container::pair<const_iterator, bool> ret_pair;
            core::container::tuple tuple{this, &ret_pair, &key, &value};
            storage.executer->invoke(implements::any_operation::container_insert_map_like, const_cast<basic_any *>(this), &tuple);
            return ret_pair;
        }

        void resize(const std::size_t new_size) {
            core::container::tuple tuple{this, new_size};
            storage.executer->invoke(implements::any_operation::container_resize, const_cast<basic_any *>(this), &tuple);
        }

        RAINY_NODISCARD std::size_t size() const {
            std::size_t size{};
            core::container::tuple tuple{this, &size};
            storage.executer->invoke(implements::any_operation::container_size, const_cast<basic_any *>(this), &tuple);
            return size;
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        template <implements::any_operation Operation>
        reference as_ref_impl() noexcept {
            reference reference;
            if (has_value()) {
                auto tuple = core::container::make_tuple(true, this, &reference);
                storage.executer->invoke(Operation, const_cast<basic_any *>(this), &tuple);
            }
            return reference;
        }

        template <implements::any_operation Operation>
        RAINY_NODISCARD const_reference as_ref_impl() const noexcept {
            const_reference reference;
            if (has_value()) {
                auto tuple = core::container::make_tuple(true, this, &reference);
                storage.executer->invoke(Operation, const_cast<basic_any *>(this), &tuple);
            }
            return reference;
        }

        RAINY_NODISCARD implements::any_representation get_representation() const noexcept {
            return static_cast<implements::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const core::typeinfo *type_info() const noexcept {
            return reinterpret_cast<const core::typeinfo *>(storage.type_data & ~rep_mask);
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
            using decayed = type_traits::other_trans::decay_t<Decayed>;
            if constexpr (type_traits::composite_types::is_reference_v<Decayed>) {
                using remove_reference = type_traits::modifers::remove_reference_t<Decayed>;
                if constexpr (type_traits::primary_types::is_function_v<remove_reference> ||
                              (type_traits::primary_types::is_pointer_v<remove_reference> &&
                               type_traits::primary_types::is_function_v<
                                   type_traits::modifers::remove_pointer_t<remove_reference>>) ) {
                    // Function references and function pointer references cannot be stored
                    // as const void*. Decay to the value type and store by value.
                    return emplace_<decayed>(utility::forward<Types>(args)...);
                } else if constexpr (!type_traits::primary_types::is_array_v<remove_reference>) {
                    return emplace_ref<Decayed>(utility::forward<Types>(args)...);
                } else {
                    return emplace_<decayed>(utility::forward<Types>(args)...);
                }
            } else if constexpr (rainy::foundation::container::implements::any_is_small<decayed, sizeof(storage_t::buffer)>) {
                auto &object = reinterpret_cast<decayed &>(storage.buffer);
                storage.small_any_raii_manager = &rainy::foundation::container::implements::any_small_raii_manager_object<decayed>;
                ::new (utility::addressof(object)) decayed(utility::forward<Types>(args)...);
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(rainy::foundation::container::implements::any_representation::_small);
                storage.executer = &rainy::foundation::container::implements::any_execution_policy_object<decayed, basic_any>;
                return object;
            } else {
                rainy_let new_alloc_ptr = ::new decayed(utility::forward<Types>(args)...);
                storage.ptr = new_alloc_ptr;
                storage.big_any_raii_manager = &rainy::foundation::container::implements::any_big_raii_manager_object<decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(rainy::foundation::container::implements::any_representation::big);
                storage.executer = &rainy::foundation::container::implements::any_execution_policy_object<decayed, basic_any>;
                return *new_alloc_ptr;
            }
        }

        template <typename ReferenceType, typename Type>
        RAINY_INLINE decltype(auto) emplace_ref(Type &&reference) {
            storage.ptr = utility::addressof(reference);
            storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(ReferenceType)) |
                                static_cast<std::uintptr_t>(implements::any_representation::reference);
            storage.executer =
                &implements::any_execution_policy_object<type_traits::modifers::remove_reference_t<Type>, basic_any>;
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

namespace rainy::foundation::container {
    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<
                  type_traits::properties::is_constructible_v<any, std::in_place_type_t<Ty>, Args...>, int> = 0>
    RAINY_NODISCARD any make_any(Args &&...args) {
        return basic_any{std::in_place_type<Ty>, utility::forward<Args>(args)...};
    }

    template <
        typename Ty, typename U, typename... Args,
        type_traits::other_trans::enable_if_t<
            type_traits::properties::is_constructible_v<any, std::in_place_type_t<Ty>, std::initializer_list<U> &, Args...>, int> = 0>
    RAINY_NODISCARD any make_any(std::initializer_list<U> initializer_list, Args &&...args) {
        return basic_any{std::in_place_type<Ty>, initializer_list, utility::forward<Args>(args)...};
    }

    template <std::size_t Length, std::size_t Align>
    RAINY_INLINE void swap(basic_any<Length, Align> &left, basic_any<Length, Align> &right) noexcept {
        left.swap(right);
    }

    template <std::size_t Length, std::size_t Align>
    RAINY_INLINE basic_any<Length, Align> wrap_any(basic_any<Length, Align> &value) noexcept {
        return basic_any(wrap_any_tag{}, value);
    }
}

namespace rainy::utility {
    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::composite_types::is_reference_v<TargetType> ||
                                                               type_traits::primary_types::is_pointer_reference_v<TargetType>>> {
        static decltype(auto) basic_convert(const void *target_pointer, const core::typeinfo &type) {
            return foundation::container::implements::as_impl<TargetType>(target_pointer, type);
        }

        static bool is_convertible(const core::typeinfo &type) {
            return foundation::container::implements::is_as_runnable<TargetType>(type);
        }
    };

    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::primary_types::is_pointer_v<TargetType> ||
                                                               type_traits::primary_types::is_pointer_reference_v<TargetType>>> {
        static rain_fn basic_convert(const void *target_pointer, const core::typeinfo &type) -> TargetType {
            if (type.is_nullptr()) {
                return nullptr;
            }
            return foundation::container::implements::as_impl<TargetType>(target_pointer, type);
        }

        static bool is_convertible(const core::typeinfo &type) {
            if (type.is_nullptr()) {
                return true;
            }
            return foundation::container::implements::is_as_runnable<TargetType>(type);
        }
    };

    template <typename TargetType>
    struct any_converter<TargetType, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<
                                         std::nullptr_t, type_traits::modifers::remove_reference_t<TargetType>>>> {
        static rain_fn basic_convert(const void *target_pointer, const core::typeinfo &type) -> TargetType {
            if (type.is_pointer() || type.is_nullptr()) {
                return nullptr;
            }
            throw std::bad_cast{};
        }

        static bool is_convertible(const core::typeinfo &type) {
            return type.is_pointer() || type.is_nullptr();
        }
    };

    template <typename TargetType>
    struct any_converter<TargetType, type_traits::other_trans::enable_if_t<type_traits::composite_types::is_arithmetic_v<TargetType>>>
        : core::enable_for_type_convert<any_converter<TargetType>> {
        static TargetType basic_convert(const void *target_pointer, const core::typeinfo &type) {
            switch (const core::typeinfo target_type = type; target_type.remove_cvref().hash_code()) {
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
#if RAINY_USING_LINUX
                case rainy_typehash(long long unsigned int):
                    return static_cast<TargetType>(*static_cast<const long long unsigned int *>(target_pointer));
#endif
#if RAINY_USING_MACOS
                case rainy_typehash(unsigned long):
                    return static_cast<TargetType>(*static_cast<const unsigned long *>(target_pointer));
#endif
                default:
                    break;
            }
            throw std::bad_cast{};
        }

        static bool is_convertible(const core::typeinfo &type) {
            switch (const core::typeinfo target_type = type; target_type.remove_cvref().hash_code()) {
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
#if RAINY_USING_LINUX
                case rainy_typehash(long long unsigned int):
#endif
                    return true;
                default:
                    return false;
            }
        }
    };

    template <typename CharType, typename Traits>
    struct any_converter<std::basic_string_view<CharType, Traits>>
        : core::enable_for_type_convert<any_converter<std::basic_string_view<CharType, Traits>>> {
        using target_type = std::basic_string_view<CharType, Traits>;

        static target_type basic_convert(const void *target_pointer, const core::typeinfo &type) {
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
            throw std::bad_cast{};
        }

        static bool is_convertible(const core::typeinfo &type) {
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

    template <typename CharType, typename Traits>
    struct any_converter<std::basic_string<CharType, Traits>>
        : core::enable_for_type_convert<any_converter<std::basic_string<CharType, Traits>>> {
        using target_type = std::basic_string<CharType, Traits>;

        static target_type basic_convert(const void *target_pointer, const core::typeinfo &type) {
            using const_pointer = const CharType *;
            using pointer = CharType *;
            using pointer_to_const = const CharType *const;
            using same_type = std::basic_string<CharType, Traits>;
            using same_type_with_const = const std::basic_string<CharType, Traits>;
            using basic_string_view_t = std::basic_string_view<CharType, Traits>;
            using const_basic_string_view_t = const std::basic_string_view<CharType, Traits>;
            switch (type.remove_reference().hash_code()) {
                case rainy_typehash(const_pointer):
                case rainy_typehash(pointer):
                case rainy_typehash(pointer_to_const):
                    // return target_type{static_cast<const_pointer>(target_pointer)}; [[deprecated]]
                    return *static_cast<const const_pointer *>(target_pointer);
                case rainy_typehash(same_type):
                case rainy_typehash(same_type_with_const):
                    return *static_cast<const target_type *>(target_pointer);
                case rainy_typehash(basic_string_view_t):
                    RAINY_FALLTHROUGH;
                case rainy_typehash(const_basic_string_view_t): {
                    const basic_string_view_t &str = *static_cast<const basic_string_view_t *>(target_pointer);
                    return {str.data(), str.size()};
                }
                default:
                    break;
            }
            throw std::bad_cast{};
        }

        static bool is_convertible(const core::typeinfo &type) {
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

namespace rainy::core::text::implements {
    template <std::size_t Length, std::size_t Align, typename CharType>
    inline constexpr bool has_conflicting_formatter_v<foundation::container::basic_any<Length, Align>, CharType> = true;
}

namespace rainy::core::text {
    template <std::size_t Length, std::size_t Align, typename CharType>
    struct formatter<foundation::container::basic_any<Length, Align>, CharType> {
        constexpr auto parse(basic_format_parse_context<CharType> &ctx) -> typename basic_format_parse_context<CharType>::iterator {
            auto iter = ctx.begin();
            const auto end = ctx.end();
            const auto first = iter;
            while (iter != end && *iter != CharType('}')) {
                ++iter; // NOLINT
            }
            specification = basic_string_view<CharType>(first, static_cast<std::size_t>(iter - first));
            return iter;
        }

        template <typename FormatContext>
        auto format(const foundation::container::basic_any<Length, Align> &value, FormatContext &ctx) const
            -> typename FormatContext::iterator {
            basic_string<CharType> buffer;
            basic_format_parse_context<CharType> parse_context{specification};
            if (!value.format(parse_context, buffer)) {
                exceptions::runtime::throw_format_error("formatter is not specialized for the stored type");
            }
            return core::algorithm::copy(buffer.begin(), buffer.end(), ctx.out());
        }

    private:
        basic_string_view<CharType> specification{};
    };
}

namespace std { // NOLINT
    template <std::size_t Length, std::size_t Align>
    struct hash<::rainy::foundation::container::basic_any<Length, Align>> { // NOLINT
        RAINY_NODISCARD std::size_t operator()(const ::rainy::foundation::container::basic_any<Length, Align> &right) const {
            return right.hash_code();
        }
    };
}

#if RAINY_HAS_CXX20
template <std::size_t Length, std::size_t Align>
struct std::formatter<::rainy::foundation::container::basic_any<Length, Align>, char> {
    constexpr auto parse(auto &ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Invalid format specifier for rainy::foundation::container::basic_any");
        }
        return it;
    }

    auto format(const ::rainy::foundation::container::basic_any<Length, Align> &val, auto &ctx) const {
        std::ostringstream ss;
        ss << val;
        for (auto c: ss.str()) {
            *ctx.out()++ = c;
        }
        return ctx.out();
    }
};

template <std::size_t Length, std::size_t Align>
struct std::formatter<::rainy::foundation::container::basic_any<Length, Align>, wchar_t> {
    constexpr auto parse(auto &ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Invalid format specifier for rainy::foundation::container::basic_any");
        }
        return it;
    }

    auto format(const ::rainy::foundation::container::basic_any<Length, Align> &val, auto &ctx) const {
        std::wostringstream ss;
        ss << val;
        for (auto c: ss.str()) {
            *ctx.out()++ = c;
        }
        return ctx.out();
    }
};
#endif

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#undef RAINY_ANY_AS_NODISCARD
#undef RAINY_ANY_CAST_TO_POINTER_NODISCARD

#endif
