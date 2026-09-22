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
#ifndef RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_ANY_ITERATOR_HPP
#define RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_ANY_ITERATOR_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/foundation/container/implements/any/fwd.hpp>
#include <rainy/foundation/container/implements/any/reference.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

namespace rainy::foundation::container::implements {
    template <typename BasicAny>
    class any_iterator_impl {
    public:
        using value_type = BasicAny;
        using reference = typename value_type::reference;
        using const_reference = typename value_type::const_reference;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        static constexpr std::size_t length = sizeof(void *) * 5;

        class arrow_proxy {
        public:
            explicit arrow_proxy(reference ref) : held_(utility::move(ref)) {
            }

            RAINY_NODISCARD reference *operator->() {
                return utility::addressof(held_);
            }

        private:
            reference held_;
        };

        using pointer = arrow_proxy;
        using const_pointer = arrow_proxy;

        struct iterator_proxy_vtable {
            virtual ~iterator_proxy_vtable() = default;

            virtual void next() = 0;

            virtual void previous() = 0;

            virtual reference dereference() = 0;

            virtual const_reference const_dereference() const = 0;

            RAINY_NODISCARD virtual core::typeinfo typeinfo() const = 0;

            virtual void destruct(bool is_local) = 0;

            virtual iterator_proxy_vtable *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;

            virtual bool compare_equal(const iterator_proxy_vtable *right) const = 0;

            virtual bool compare_less(const iterator_proxy_vtable *right) const = 0;

            virtual difference_type subtract(const iterator_proxy_vtable *right) const = 0;

            virtual any_iterator_impl subtract(difference_type diff) const = 0;

            virtual any_iterator_impl add(difference_type diff) const = 0;

            virtual reference key() = 0;

            virtual const_reference key() const = 0;

            virtual reference value() = 0;

            virtual const_reference value() const = 0;

            RAINY_NODISCARD virtual const core::typeinfo &target_type() const noexcept = 0;

            RAINY_NODISCARD virtual const void *target() const noexcept = 0;

            RAINY_NODISCARD virtual any_iterator_category iterator_category() const noexcept = 0;
        };

        any_iterator_impl() = default;

        template <typename IterImpl, typename RealIterator>
        any_iterator_impl(std::in_place_type_t<IterImpl>, RealIterator &&iter) {
            if constexpr (sizeof(IterImpl) > length) {
                proxy = ::new IterImpl(utility::forward<RealIterator>(iter));
                is_local_ = false;
            } else {
                proxy = ::new (static_cast<void *>(buffer)) IterImpl(utility::forward<RealIterator>(iter));
                is_local_ = true;
            }
        }

        ~any_iterator_impl() {
            destroy_proxy();
        }

        any_iterator_impl(const any_iterator_impl &right) {
            copy_from_other(right);
        }

        any_iterator_impl(any_iterator_impl &&right) noexcept {
            move_from_other(utility::move(right));
        }

        any_iterator_impl &operator=(const any_iterator_impl &right) { // NOLINT
            copy_from_other(right);
            return *this;
        }

        any_iterator_impl &operator=(any_iterator_impl &&right) noexcept {
            move_from_other(utility::move(right));
            return *this;
        }

        any_iterator_impl &operator++() {
            validate();
            proxy->next();
            return *this;
        }

        any_iterator_impl operator++(int) {
            validate();
            any_iterator_impl tmp = *this;
            proxy->next();
            return tmp;
        }

        any_iterator_impl &operator--() {
            validate();
            proxy->previous();
            return *this;
        }

        any_iterator_impl operator--(int) {
            validate();
            any_iterator_impl tmp = *this;
            proxy->previous();
            return tmp;
        }

        reference operator*() {
            validate();
            return proxy->dereference();
        }

        const_reference operator*() const {
            validate();
            return proxy->const_dereference();
        }

        RAINY_NODISCARD arrow_proxy operator->() {
            validate();
            return arrow_proxy{proxy->dereference()};
        }

        RAINY_NODISCARD arrow_proxy operator->() const {
            validate();
            return arrow_proxy{proxy->const_dereference()};
        }

        reference operator[](difference_type diff) {
            return *(*this + diff);
        }

        const_reference operator[](difference_type diff) const {
            return *(*this + diff);
        }

        any_iterator_impl &operator+=(difference_type diff) {
            *this = *this + diff;
            return *this;
        }

        any_iterator_impl &operator-=(difference_type diff) {
            *this = *this - diff;
            return *this;
        }

        friend difference_type operator-(const any_iterator_impl &left, const any_iterator_impl &right) {
            if (left.proxy == nullptr || right.proxy == nullptr) {
                if (left.proxy != right.proxy) {
                    throw core::exceptions::runtime::nullpointer_exception(
                        "Cannot subtract iterators that are not bound to the same container");
                }
                return 0;
            }
            return left.proxy->subtract(right.proxy);
        }

        friend any_iterator_impl operator-(const any_iterator_impl &left, difference_type diff) {
            left.validate();
            return left.proxy->subtract(diff);
        }

        friend any_iterator_impl operator+(const any_iterator_impl &left, difference_type diff) {
            left.validate();
            return left.proxy->add(diff);
        }

        friend any_iterator_impl operator+(difference_type diff, const any_iterator_impl &right) {
            return right + diff;
        }

        friend bool operator==(const any_iterator_impl &left, const any_iterator_impl &right) {
            if (left.proxy == nullptr || right.proxy == nullptr) {
                return left.proxy == right.proxy;
            }
            return left.proxy->compare_equal(right.proxy);
        }

        friend bool operator!=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !(left == right);
        }

        friend bool operator<(const any_iterator_impl &left, const any_iterator_impl &right) {
            if (left.proxy == nullptr || right.proxy == nullptr) {
                if (left.proxy != right.proxy) {
                    throw core::exceptions::runtime::nullpointer_exception(
                        "Cannot order iterators that are not bound to the same container");
                }
                return false;
            }
            return left.proxy->compare_less(right.proxy);
        }

        friend bool operator>(const any_iterator_impl &left, const any_iterator_impl &right) {
            return right < left;
        }

        friend bool operator<=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !(right < left);
        }

        friend bool operator>=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !(left < right);
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return proxy == nullptr;
        }

        RAINY_NODISCARD bool has_value() const noexcept {
            return proxy != nullptr;
        }

        void copy_from_other(const any_iterator_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            destroy_proxy();
            if (right.proxy == nullptr) {
                return;
            }
            is_local_ = right.is_local_;
            proxy = right.proxy->construct_from_this(buffer);
        }

        void move_from_other(any_iterator_impl &&right) {
            if (this == utility::addressof(right) || right.proxy == nullptr) {
                return;
            }
            destroy_proxy();
            if (right.is_local_) {
                proxy = right.proxy->construct_from_this(buffer);
                is_local_ = true;
                right.proxy = nullptr;
                right.is_local_ = false;
            } else {
                proxy = utility::exchange(right.proxy, nullptr);
                is_local_ = false;
            }
        }

        RAINY_NODISCARD any_iterator_category category() const {
            validate();
            return proxy->iterator_category();
        }

        void swap(any_iterator_impl &right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            any_iterator_impl temp;
            temp.move_from_other(utility::move(*this));
            this->move_from_other(utility::move(right));
            right.move_from_other(utility::move(temp));
        }

        reference key() {
            validate();
            return proxy->key();
        }

        const_reference key() const {
            validate();
            return proxy->key();
        }

        reference value() {
            validate();
            return proxy->value();
        }

        const_reference value() const {
            validate();
            return proxy->value();
        }

        friend void swap(any_iterator_impl &left, any_iterator_impl &right) noexcept {
            left.swap(right);
        }

        template <typename Type>
        RAINY_NODISCARD const Type &target_iterator() const {
            validate();
            if (proxy->target_type() != rainy_typeid(Type)) {
                throw core::exceptions::runtime::bad_cast("The requested iterator type does not match the stored one");
            }
            return *static_cast<const Type *>(proxy->target());
        }

    private:
        RAINY_INLINE void validate() const {
            if (proxy == nullptr) {
                throw core::exceptions::runtime::nullpointer_exception("This any iterator does not reference any container");
            }
        }

        RAINY_INLINE void destroy_proxy() noexcept {
            if (proxy == nullptr) {
                return;
            }
            proxy->destruct(is_local_);
            proxy = nullptr;
            is_local_ = false;
        }

        iterator_proxy_vtable *proxy{nullptr};
        bool is_local_{false};
        alignas(std::max_align_t) core::byte_t buffer[length]{};
    };
}

namespace rainy::foundation::container::implements {
    template <typename Elem>
    struct array_iter_fallback {
        using iterator = Elem *;
    };

    template <typename Elem>
    struct const_array_iter_fallback {
        using const_iterator = const Elem *;
    };

    template <typename BasicAny, typename Type>
    struct any_proxy_iterator : BasicAny::iterator::iterator_proxy_vtable {
        using iterator_t = typename type_traits::other_trans::conditional_t<type_traits::extras::iterators::has_iterator_v<Type>, Type,
                                                                           array_iter_fallback<type_traits::modifers::remove_extent_t<Type>>>::iterator;
        using const_iterator_t =
            typename type_traits::other_trans::conditional_t<type_traits::extras::iterators::has_const_iterator_v<Type>, Type,
                                                             const_array_iter_fallback<type_traits::modifers::remove_extent_t<Type>>>::const_iterator;
        using proxy_t = typename BasicAny::iterator::iterator_proxy_vtable;
        using basic_any = BasicAny;
        using reference = typename basic_any::reference;
        using const_reference = typename basic_any::const_reference;

        using any_iterator_t = typename basic_any::iterator;

        using difference_type = typename any_iterator_t::difference_type;

        any_proxy_iterator(iterator_t iterator) : iter{iterator} {
        }

        void destruct(const bool is_local) override {
            using self_type = type_traits::modifers::remove_cvref_t<decltype(*this)>;
            static_cast<self_type *>(this)->~self_type();
            if (!is_local) {
                ::operator delete(static_cast<void *>(this));
            }
        }

        void next() override {
            ++iter;
        }

        void previous() override {
            if constexpr (get_iterator_category<iterator_t>() <= any_iterator_category::forward_iterator) {
                throw core::exceptions::logic::logic_error("The underlying iterator does not support backward movement");
            } else {
                --iter;
            }
        }

        reference dereference() override {
            if constexpr (get_iterator_category<iterator_t>() == any_iterator_category::output_iterator) {
                throw core::exceptions::logic::logic_error("The underlying iterator cannot be dereferenced");
            } else {
                return *iter;
            }
        }

        const_reference const_dereference() const override {
            if constexpr (get_iterator_category<iterator_t>() == any_iterator_category::output_iterator) {
                throw core::exceptions::logic::logic_error("The underlying iterator cannot be dereferenced");
            } else {
                return *iter;
            }
        }

        RAINY_NODISCARD core::typeinfo typeinfo() const override {
            return rainy_typeid(any_proxy_iterator);
        }

        proxy_t *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(*this) > BasicAny::iterator::length) {
                return ::new any_proxy_iterator(this->iter);
            } else {
                return ::new (reinterpret_cast<any_proxy_iterator *>(soo_buffer)) any_proxy_iterator(this->iter);
            }
        }

        bool compare_less(const proxy_t *right) const override {
            using cit = const_any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_lt_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                    return iter < static_cast<const any_proxy_iterator *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(cit)) {
                    return iter < static_cast<const cit *>(right)->iter;
                }
            }
            throw core::exceptions::logic::logic_error("Cannot order iterators of different types");
        }

        bool compare_equal(const proxy_t *right) const override {
            using cit = const_any_proxy_iterator<BasicAny, Type>;
            if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                return iter == static_cast<const any_proxy_iterator *>(right)->iter;
            }
            if (right->typeinfo() == rainy_typeid(cit)) {
                return iter == static_cast<const cit *>(right)->iter;
            }
            return false;
        }

        RAINY_NODISCARD any_iterator_category iterator_category() const noexcept override {
            constexpr any_iterator_category category = get_iterator_category<iterator_t>();
            return category;
        }

        any_iterator_t add(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_addition_v<iterator_t>) {
                return any_iterator_t{std::in_place_type<any_proxy_iterator>, iter + diff};
            } else {
                throw core::exceptions::logic::logic_error("The underlying iterator does not support random access");
            }
        }

        any_iterator_t subtract(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                return any_iterator_t{std::in_place_type<any_proxy_iterator>, iter - diff};
            } else {
                throw core::exceptions::logic::logic_error("The underlying iterator does not support random access");
            }
        }

        difference_type subtract(const proxy_t *right) const override {
            using cit = const_any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                    return iter - static_cast<const any_proxy_iterator *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(cit)) {
                    return iter - static_cast<const cit *>(right)->iter;
                }
            }
            throw core::exceptions::logic::logic_error("Cannot subtract iterators of different types");
        }

        reference key() override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                auto &[key, _] = *iter;
                (void) _;
                return key;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("key() is only available for associative containers");
            }
        }

        const_reference key() const override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                const auto &[key, _] = *iter;
                (void) _;
                return key;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("key() is only available for associative containers");
            }
        }

        reference value() override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                auto &[_, value] = *iter;
                (void) _;
                return value;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("value() is only available for associative containers");
            }
        }

        const_reference value() const override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                const auto &[_, value] = *iter;
                (void) _;
                return value;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("value() is only available for associative containers");
            }
        }

        RAINY_NODISCARD const core::typeinfo &target_type() const noexcept override {
            return rainy_typeid(iterator_t);
        }

        RAINY_NODISCARD const void *target() const noexcept override {
            return utility::addressof(iter);
        }

        iterator_t iter;
    };

    template <typename BasicAny, typename Type>
    struct const_any_proxy_iterator : BasicAny::iterator::iterator_proxy_vtable {
        using iterator_t =
            typename type_traits::other_trans::conditional_t<type_traits::extras::iterators::has_const_iterator_v<Type>, Type,
                                                             const_array_iter_fallback<type_traits::modifers::remove_extent_t<Type>>>::const_iterator;
        using proxy_t = typename BasicAny::iterator::iterator_proxy_vtable;
        using basic_any = BasicAny;
        using reference = typename basic_any::reference;
        using const_reference = typename basic_any::const_reference;

        using any_iterator_t = typename basic_any::iterator;

        using difference_type = typename any_iterator_t::difference_type;

        const_any_proxy_iterator(iterator_t iterator) : iter{iterator} {
        }

        void destruct(const bool is_local) override {
            using self_type = type_traits::modifers::remove_cvref_t<decltype(*this)>;
            static_cast<self_type *>(this)->~self_type();
            if (!is_local) {
                ::operator delete(static_cast<void *>(this));
            }
        }

        void next() override {
            ++iter;
        }

        void previous() override {
            if constexpr (get_iterator_category<iterator_t>() <= any_iterator_category::forward_iterator) {
                throw core::exceptions::logic::logic_error("The underlying iterator does not support backward movement");
            } else {
                --iter;
            }
        }

        reference dereference() override {
            if constexpr (get_iterator_category<iterator_t>() == any_iterator_category::output_iterator) {
                throw core::exceptions::logic::logic_error("The underlying iterator cannot be dereferenced");
            } else {
                return *iter;
            }
        }

        typename BasicAny::const_reference const_dereference() const override {
            if constexpr (get_iterator_category<iterator_t>() == any_iterator_category::output_iterator) {
                throw core::exceptions::logic::logic_error("The underlying iterator cannot be dereferenced");
            } else {
                return *iter;
            }
        }

        RAINY_NODISCARD core::typeinfo typeinfo() const override {
            return rainy_typeid(const_any_proxy_iterator);
        }

        proxy_t *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(*this) > BasicAny::iterator::length) {
                return ::new const_any_proxy_iterator(this->iter);
            } else {
                return ::new (reinterpret_cast<const_any_proxy_iterator *>(soo_buffer)) const_any_proxy_iterator(this->iter);
            }
        }

        RAINY_NODISCARD any_iterator_category iterator_category() const noexcept override {
            constexpr any_iterator_category category = get_iterator_category<iterator_t>();
            return category;
        }

        bool compare_equal(const proxy_t *right) const override {
            using it = any_proxy_iterator<BasicAny, Type>;
            if (right->typeinfo() == rainy_typeid(it)) {
                return iter == static_cast<const it *>(right)->iter;
            }
            if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                return iter == static_cast<const const_any_proxy_iterator *>(right)->iter;
            }
            return false;
        }

        bool compare_less(const proxy_t *right) const override {
            using it = any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_lt_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(it)) {
                    return iter < static_cast<const it *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                    return iter < static_cast<const const_any_proxy_iterator *>(right)->iter;
                }
            }
            throw core::exceptions::logic::logic_error("Cannot order iterators of different types");
        }

        any_iterator_t add(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_addition_v<iterator_t>) {
                return any_iterator_t{std::in_place_type<const_any_proxy_iterator>, iter + diff};
            } else {
                throw core::exceptions::logic::logic_error("The underlying iterator does not support random access");
            }
        }

        any_iterator_t subtract(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                return any_iterator_t{std::in_place_type<const_any_proxy_iterator>, iter - diff};
            } else {
                throw core::exceptions::logic::logic_error("The underlying iterator does not support random access");
            }
        }

        difference_type subtract(const proxy_t *right) const override {
            using it = any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(it)) {
                    return iter - static_cast<const it *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                    return iter - static_cast<const const_any_proxy_iterator *>(right)->iter;
                }
            }
            throw core::exceptions::logic::logic_error("Cannot subtract iterators of different types");
        }

        reference key() override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                auto &[key, _] = *iter;
                (void) _;
                return key;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("key() is only available for associative containers");
            }
        }

        const_reference key() const override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                const auto &[key, _] = *iter;
                (void) _;
                return key;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("key() is only available for associative containers");
            }
        }

        reference value() override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                auto &[_, value] = *iter;
                (void) _;
                return value;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("value() is only available for associative containers");
            }
        }

        const_reference value() const override {
            if constexpr (type_traits::properties::is_map_like_v<Type>) {
                const auto &[_, value] = *iter;
                (void) _;
                return value;
            } else if constexpr (type_traits::extras::meta_types::has_key_type_v<Type>) {
                return *iter;
            } else {
                throw core::exceptions::logic::logic_error("value() is only available for associative containers");
            }
        }

        RAINY_NODISCARD const core::typeinfo &target_type() const noexcept override {
            return rainy_typeid(iterator_t);
        }

        RAINY_NODISCARD const void *target() const noexcept override {
            return utility::addressof(iter);
        }

        iterator_t iter;
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
