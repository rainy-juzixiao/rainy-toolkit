#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_ITERATOR_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_ITERATOR_HPP
#include <rainy/utility/implements/any/fwd.hpp>
#include <rainy/utility/implements/any/reference.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

namespace rainy::utility::implements {
    template <typename BasicAny>
    class any_iterator_impl {
    public:
        using value_type = BasicAny;
        using reference = typename value_type::reference;
        using const_reference = typename value_type::const_reference;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type;
        using const_pointer = value_type;
        using iterator_category = std::random_access_iterator_tag;

        static constexpr std::size_t length = sizeof(void *) * 5;

        struct iterator_proxy_vtable {
            virtual ~iterator_proxy_vtable() = default;
            virtual void next() = 0;
            virtual void previous() = 0;
            virtual reference dereference() = 0;
            virtual const_reference const_dereference() const = 0;
            RAINY_NODISCARD virtual foundation::ctti::typeinfo typeinfo() const = 0;
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
            RAINY_NODISCARD virtual const foundation::ctti::typeinfo& target_type() const noexcept = 0;
            RAINY_NODISCARD virtual const void* target() const noexcept = 0;
            RAINY_NODISCARD virtual any_iterator_category iterator_category() const noexcept = 0;
        };

        any_iterator_impl() = default;

        template <typename IterImpl, typename RealIterator>
        any_iterator_impl(std::in_place_type_t<IterImpl>, RealIterator &&iter) {
            if constexpr (sizeof(IterImpl) >= length) {
                proxy = new IterImpl(utility::forward<RealIterator>(iter));
            } else {
                proxy =
                    utility::construct_at(reinterpret_cast<IterImpl *>(buffer), utility::forward<RealIterator>(iter));
            }
        }

        ~any_iterator_impl() {
            if (proxy) {
                proxy->destruct(is_local());
            }
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
            proxy->next();
            return *this;
        }

        any_iterator_impl &operator--() {
            proxy->previous();
            return *this;
        }

        reference operator*() {
            return proxy->dereference();
        }

        const_reference operator*() const {
            return proxy->const_dereference();
        }

        friend difference_type operator-(const any_iterator_impl &left, const any_iterator_impl &right) {
            return left.proxy->subtract(right.proxy);
        }

        friend any_iterator_impl operator-(const any_iterator_impl &left, difference_type diff) {
            return left.proxy->subtract(diff);
        }

        friend any_iterator_impl operator+(const any_iterator_impl &left, difference_type diff) {
            return left.proxy->add(diff);
        }

        friend bool operator==(const any_iterator_impl &left, const any_iterator_impl &right) {
            return left.proxy->compare_equal(right.proxy);
        }

        friend bool operator!=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !left.proxy->compare_equal(right.proxy);
        }

        friend bool operator<(const any_iterator_impl &left, const any_iterator_impl &right) {
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
            return static_cast<bool>(proxy);
        }

        void copy_from_other(const any_iterator_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            if (proxy) {
                proxy->destruct(is_local());
            }
            proxy = right.proxy->construct_from_this(this->buffer);
        }

        void move_from_other(any_iterator_impl &&right) {
            if (this == utility::addressof(right) || !right.proxy) {
                return;
            }
            if (proxy) { // 防止内存泄漏
                proxy->destruct(is_local());
            }
            if (right.is_local()) {
                proxy = right.proxy->construct_from_this(this->buffer);
                right.proxy = nullptr;
            } else {
                proxy = utility::exchange(right.proxy, nullptr);
            }
        }

        RAINY_NODISCARD any_iterator_category category() const noexcept {
            return proxy->iterator_category();
        }

        void swap(any_iterator_impl &right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            if (!is_local() && !right.is_local()) {
                std::swap(proxy, right.proxy);
            } else {
                any_iterator_impl temp;
                temp.move_from_other(utility::move(*this));
                this->move_from_other(utility::move(right));
                right.move_from_other(utility::move(temp));
            }
        }

        reference key() {
            return proxy->key();
        }

        const_reference key() const {
            return proxy->key();
        }

        reference value() {
            return proxy->value();
        }

        const_reference value() const {
            return proxy->value();
        }

        friend void swap(any_iterator_impl &left, any_iterator_impl &right) noexcept {
            left.swap(right);
        }

        template <typename Type>
        const Type& target_iterator() const {
            const void* target = proxy->target();
            if (proxy->target_type() == rainy_typeid(Type)) {
                return *static_cast<const Type*>(target);
            }
            std::terminate();
        }

    private:
        RAINY_NODISCARD bool is_local() const noexcept {
            return proxy == reinterpret_cast<const void *>(buffer);
        }

        iterator_proxy_vtable *proxy{nullptr};
        core::byte_t buffer[length]{};
    };
}

namespace rainy::utility::implements {
    template <typename BasicAny, typename Type>
    struct any_proxy_iterator : BasicAny::iterator::iterator_proxy_vtable {
        using iterator_t = typename Type::iterator;
        using const_iterator_t = typename Type::const_iterator;
        using proxy_t = typename BasicAny::iterator::iterator_proxy_vtable;
        using basic_any = BasicAny;
        using reference = typename basic_any::reference;
        using const_reference = typename basic_any::const_reference;

        using any_iterator_t = typename basic_any::iterator;

        using difference_type = typename any_iterator_t::difference_type;

        any_proxy_iterator(iterator_t iterator) : iter{iterator} {
        }

        void destruct(const bool is_local) override {
            if (is_local) {
                std::destroy_at(&iter);
            } else {
                std::destroy_at(&iter);
                delete this;
            }
        }

        void next() override {
            ++iter;
        }

        void previous() override {
            if constexpr (get_iterator_category<iterator_t>() <= any_iterator_category::forward_iterator) {
                foundation::exceptions::logic::throw_any_not_implemented("Current iterator category is not support this operation");
            } else {
                --iter;
            }
        }

        reference dereference() override {
            return *iter;
        }

        const_reference const_dereference() const override {
            return *iter;
        }

        RAINY_NODISCARD foundation::ctti::typeinfo typeinfo() const override {
            return rainy_typeid(any_proxy_iterator);
        }

        proxy_t *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(type_traits::other_trans::decay_t<decltype(*this)>) >= BasicAny::iterator::length) {
                return ::new any_proxy_iterator(this->iter);
            } else {
                return utility::construct_at(reinterpret_cast<any_proxy_iterator *>(soo_buffer), this->iter);
            }
        }

        bool compare_less(const proxy_t *right) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_le_v<iterator_t>) {
                using cit = const_any_proxy_iterator<BasicAny, Type>;
                if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                    return iter < static_cast<const any_proxy_iterator *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(cit)) {
                    return iter < static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
                }
            }
            return false;
        }

        bool compare_equal(const proxy_t *right) const override {
            using cit = const_any_proxy_iterator<BasicAny, Type>;
            if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                return iter == static_cast<const any_proxy_iterator *>(right)->iter;
            }
            if (right->typeinfo() == rainy_typeid(cit)) {
                return iter == static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
            }
            return false;
        }

        RAINY_NODISCARD any_iterator_category iterator_category() const noexcept override {
            constexpr any_iterator_category category = get_iterator_category<iterator_t>();
            return category;
        }

        any_iterator_t add(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_addition_v<iterator_t>) {
                return {std::in_place_type<any_proxy_iterator>, iter + diff};
            }
            return {};
        }

        any_iterator_t subtract(ptrdiff_t diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                return any_iterator_t{std::in_place_type<any_proxy_iterator>, iter - diff};
            }
            return {};
        }

        difference_type subtract(const proxy_t *right) const override {
            using cit = any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                    return iter - static_cast<const any_proxy_iterator *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(cit)) {
                    return iter - static_cast<const cit *>(right)->iter;
                }
            }
            return {};
        }

        reference key() override {
            if constexpr (type_traits::type_properties::is_associative_container_v<Type>) {
                auto &[key, _] = *iter;
                (void) _;
                return key;
            }
            foundation::exceptions::logic::throw_any_not_implemented("Cannot invoke value() because it's not a value");
            std::terminate();
        }

        const_reference key() const override {
            if constexpr (type_traits::type_properties::is_associative_container_v<Type>) {
                const auto &[key, _] = *iter;
                (void) _;
                return key;
            }
            foundation::exceptions::logic::throw_any_not_implemented("Cannot invoke value() because it's not a value");
            std::terminate();
        }

        reference value() override {
            if constexpr (type_traits::type_properties::is_associative_container_v<Type>) {
                auto &[key, _] = *iter;
                (void) _;
                return key;
            }
            foundation::exceptions::logic::throw_any_not_implemented("Cannot invoke value() because it's not a value");
            std::terminate();
        }

        const_reference value() const override {
            if constexpr (type_traits::type_properties::is_associative_container_v<Type>) {
                const auto &[key, _] = *iter;
                (void) _;
                return key;
            }
            foundation::exceptions::logic::throw_any_not_implemented("Cannot invoke value() because it's not a value");
            std::terminate();
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo& target_type() const noexcept {
            return rainy_typeid(iterator_t);
        }

        RAINY_NODISCARD const void* target() const noexcept {
            return &iter;
        }

        iterator_t iter;
    };

    template <typename BasicAny, typename Type>
    struct const_any_proxy_iterator : BasicAny::iterator::iterator_proxy_vtable {
        using iterator_t = typename Type::const_iterator;
        using proxy_t = typename BasicAny::iterator::iterator_proxy_vtable;
        using basic_any = BasicAny;
        using reference = typename basic_any::reference;
        using const_reference = typename basic_any::const_reference;

        using any_iterator_t = typename basic_any::iterator;

        using difference_type = typename any_iterator_t::difference_type;

        const_any_proxy_iterator(const iterator_t &iterator) : iter{iterator} {
        }

        void destruct(const bool is_local) override {
            if (is_local) {
                std::destroy_at(&iter);
            } else {
                std::destroy_at(&iter);
                delete this;
            }
        }

        void next() override {
            ++iter;
        }

        void previous() override {
            if constexpr (get_iterator_category<iterator_t>() <= any_iterator_category::forward_iterator) {
                foundation::exceptions::logic::throw_any_not_implemented("Current iterator category is not support this operation");
            } else {
                --iter;
            }
        }

        reference dereference() override {
            return *iter;
        }

        typename BasicAny::const_reference const_dereference() const override {
            return *iter;
        }

        RAINY_NODISCARD foundation::ctti::typeinfo typeinfo() const override {
            return rainy_typeid(const_any_proxy_iterator);
        }

        proxy_t *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(type_traits::other_trans::decay_t<decltype(*this)>) >= BasicAny::iterator::length) {
                return ::new const_any_proxy_iterator(this->iter);
            } else {
                return utility::construct_at(reinterpret_cast<const_any_proxy_iterator *>(soo_buffer), this->iter);
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
                return iter == static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
            }
            return false;
        }

        bool compare_less(const proxy_t *right) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_le_v<iterator_t>) {
                using it = any_proxy_iterator<BasicAny, Type>;
                if (right->typeinfo() == rainy_typeid(it)) {
                    return iter < static_cast<const it *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                    return iter < static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
                }
            }
            return false;
        }

        any_iterator_t add(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_addition_v<iterator_t>) {
                return {std::in_place_type<const_any_proxy_iterator>, iter + diff};
            }
            return {};
        }

        any_iterator_t subtract(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                return {std::in_place_type<const_any_proxy_iterator>, iter - diff};
            }
            return {};
        }

        difference_type subtract(const proxy_t *right) const override {
            using it = any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(it)) {
                    return iter - static_cast<const it *>(right)->iter;
                }
                if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                    return iter - static_cast<const it *>(right)->iter;
                }
            }
            return {};
        }

        reference key() override {
            if constexpr (type_traits::type_properties::is_associative_container_v<Type>) {
                const auto &[key, _] = *iter;
                (void) _;
                return key;
            }
            foundation::exceptions::logic::throw_any_not_implemented("Cannot invoke value() because it's not a value");
            std::terminate();
        }

        const_reference key() const override {
            return const_cast<const_any_proxy_iterator *>(this)->key();
        }

        reference value() override {
            if constexpr (type_traits::type_properties::is_associative_container_v<Type>) {
                const auto &[_, value] = *iter;
                (void) _;
                return value;
            }
            foundation::exceptions::logic::throw_any_not_implemented("Cannot invoke value() because it's not a value");
            std::terminate();
        }

        const_reference value() const override {
            return const_cast<const_any_proxy_iterator *>(this)->value();
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo& target_type() const noexcept {
            return rainy_typeid(iterator_t);
        }

        RAINY_NODISCARD const void* target() const noexcept {
            return &iter;
        }

        iterator_t iter;
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif