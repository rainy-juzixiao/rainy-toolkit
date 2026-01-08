#ifndef RAINY_UTILITY_ITERATOR_HPP
#define RAINY_UTILITY_ITERATOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/core/implements/reverse_iterator.hpp>

namespace rainy::utility {
    template <typename Ty>
    struct input_iterator_pointer final {
        using value_type = Ty;
        using pointer = Ty *;
        using reference = Ty &;

        constexpr input_iterator_pointer(value_type &&val) noexcept(std::is_nothrow_move_constructible_v<value_type>) :
            value{std::move(val)} {
        }

        RAINY_NODISCARD constexpr pointer operator->() noexcept {
            return std::addressof(value);
        }

        RAINY_NODISCARD constexpr reference operator*() noexcept {
            return value;
        }

    private:
        Ty value;
    };

    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class const_iterator {
    public:
        template <typename, typename>
        friend class iterator;

        friend struct foundation::memory::pointer_traits<const_iterator<Iter, Traits>>;

        using iterator_type = Iter;
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        constexpr const_iterator() noexcept : current() {};
        constexpr const_iterator &operator=(const_iterator &&) noexcept = default;
        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;

        /**
         * @brief 构造一个常量迭代器
         * @param current 当前迭代器位置
         */ 
        explicit constexpr const_iterator(iterator_type current) noexcept : current(current) {
        }

        constexpr const_iterator(const const_iterator &right) : current(right.current) {
        }

        constexpr const_iterator(const_iterator &&right) noexcept : current(rainy::utility::exchange(right.current, nullptr)) {
        }

        RAINY_CONSTEXPR20 ~const_iterator() = default;

        constexpr const_pointer get_pointer() const noexcept {
            if constexpr (type_traits::primary_types::is_pointer_v<iterator_type>) {
                return current;
            } else {
                return current.operator->();
            }
        }

        constexpr const_pointer operator->() const noexcept {
            return get_pointer();
        }

        constexpr const_reference dereference() const noexcept {
            return *current;
        }

        constexpr const_reference operator*() const noexcept {
            return dereference();
        }

        constexpr const_iterator &self_inc_prefix() noexcept {
            ++current;
            return *this;
        }

        constexpr const_iterator self_inc_postfix() noexcept {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        constexpr const_iterator &operator++() noexcept {
            return self_inc_prefix();
        }

        constexpr const_iterator operator++(int) noexcept {
            return self_inc_postfix();
        }

        constexpr const_iterator &self_dec_prefix() noexcept {
            --current;
            return *this;
        }

        constexpr const_iterator self_dec_postfix() noexcept {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr const_iterator &operator--() noexcept {
            return self_dec_prefix();
        }

        constexpr const_iterator operator--(int) noexcept {
            return self_dec_postfix();
        }

        constexpr const_iterator &plus_equal(difference_type n) noexcept {
            current += n;
            return *this;
        }

        constexpr const_iterator &minus_equal(difference_type n) noexcept {
            current -= n;
            return *this;
        }

        constexpr const_iterator &operator+=(difference_type n) noexcept {
            return plus_equal(n);
        }

        constexpr const_iterator &operator-=(difference_type n) noexcept {
            return minus_equal(n);
        }

        constexpr bool is_equal(const const_iterator &right) const noexcept {
            return current == right.current;
        }

        constexpr bool not_equal(const const_iterator &right) const noexcept {
            return current != right.current;
        }

        constexpr friend bool operator==(const const_iterator &left, const const_iterator &right) {
            return left.is_equal(right);
        }

        constexpr friend bool operator!=(const const_iterator &left, const const_iterator &right) {
            return left.not_equal(right);
        }

        constexpr const_reference at_subscript(difference_type idx) const noexcept {
            return current[idx];
        }

        constexpr const_reference operator[](difference_type idx) const noexcept {
            return at_subscript(idx);
        }

        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept {
            return this->current == nullptr;
        }

        constexpr explicit operator bool() const noexcept {
            return !empty();
        }

        constexpr const_iterator add_offset(difference_type n) const noexcept {
            return const_iterator{current + n};
        }

        constexpr friend const_iterator operator+(const const_iterator left, difference_type n) {
            return left.add_offset(n);
        }

        constexpr difference_type subtract(difference_type n) const noexcept {
            return current - n;
        }

        constexpr difference_type subtract(const const_iterator &right) const noexcept {
            return current - right.current;
        }

        constexpr friend difference_type operator-(const const_iterator &left, const const_iterator &right) {
            return left.subtract(right);
        }

        constexpr friend difference_type operator-(const const_iterator &left, difference_type n) {
            return left.subtract(n);
        }

        constexpr bool lt(const const_iterator &right) const noexcept {
            return current < right.current;
        }

        constexpr friend bool operator<(const const_iterator &left, const const_iterator &right) {
            return left.lt(right);
        }

        constexpr bool lt_or_equal(const const_iterator &right) const noexcept {
            return current <= right.current;
        }

        constexpr friend bool operator<=(const const_iterator &left, const const_iterator &right) {
            return left.lt_or_equal(right);
        }

        constexpr bool greater_than(const const_iterator &right) const noexcept {
            return current > right.current;
        }

        constexpr friend bool operator>(const const_iterator &left, const const_iterator &right) {
            return left.greater_than(right);
        }

        constexpr bool greater_than_or_equal(const const_iterator &right) const noexcept {
            return current >= right.current;
        }

        constexpr friend bool operator>=(const const_iterator &left, const const_iterator &right) {
            return left.greater_than_or_equal(right);
        }

    private:
        iterator_type current;
    };
}

template <typename Iter, typename Traits>
struct rainy::foundation::memory::pointer_traits<rainy::utility::const_iterator<Iter, Traits>> {
    using pointer = typename rainy::utility::const_iterator<Iter, Traits>;
    using elemen_type = typename pointer::value_type;
    using difference_type = typename pointer::difference_type;

    RAINY_NODISCARD static RAINY_CONSTEXPR20 elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
    template <typename Iter, typename Traits = utility::iterator_traits<Iter>>
    class iterator : public const_iterator<Iter, Traits> {
    public:
        using iterator_type = Iter;
        using iterator_traits = Traits;
        using reference = typename iterator_traits::reference;
        using pointer = typename iterator_traits::pointer;

        using base = const_iterator<Iter, Traits>;

        friend struct foundation::memory::pointer_traits<const_iterator<Iter, Traits>>;

        constexpr iterator() noexcept : base() {};

        constexpr iterator(iterator_type current) noexcept : base(current) {
        }

        constexpr iterator(const iterator &right) : base(right) {
        }

        constexpr iterator(iterator &&right) noexcept : base(right) {
        }

        constexpr iterator &operator=(iterator &&) noexcept = default;
        constexpr iterator &operator=(const iterator &) noexcept = default;

        RAINY_CONSTEXPR20 ~iterator() = default;

        constexpr base &as_const_iterator() const noexcept {
            return *this;
        }

        constexpr pointer get_pointer() noexcept {
            return const_cast<pointer>(static_cast<const base *>(this)->get_pointer());
        }

        constexpr pointer operator->() noexcept {
            return get_pointer();
        }

        constexpr reference dereference() noexcept {
            return const_cast<reference>(static_cast<const base *>(this)->dereference());
        }

        constexpr reference operator*() noexcept {
            return dereference();
        }

        constexpr iterator add_offset(typename base::difference_type n) const noexcept {
            return iterator{this->current + n};
        }

        constexpr friend iterator operator+(const iterator left, typename base::difference_type n) {
            return left.add_offset(n);
        }

        constexpr typename base::difference_type subtract(typename base::difference_type n) const noexcept {
            return this->current - n;
        }

        constexpr typename base::difference_type subtract(const iterator &right) const noexcept {
            return this->current - right.current;
        }

        constexpr iterator &operator++() noexcept {
            ++this->current;
            return *this;
        }

        constexpr iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        constexpr iterator &operator--() noexcept {
            --this->current;
            return *this;
        }

        constexpr iterator operator--(int) noexcept {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr friend typename base::difference_type operator-(const iterator &left, const iterator &right) {
            return left.subtract(right);
        }

        constexpr friend typename base::difference_type operator-(const iterator &left, typename base::difference_type n) {
            return left.subtract(n);
        }
    };
}

template <typename Iter, typename Traits>
struct rainy::foundation::memory::pointer_traits<rainy::utility::iterator<Iter, Traits>> {
    using pointer = typename rainy::utility::iterator<Iter, Traits>;
    using elemen_type = typename pointer::value_type;
    using difference_type = typename pointer::difference_type;

    RAINY_NODISCARD static constexpr elemen_type *to_address(const pointer val) noexcept {
        // 因为iterator自动将其特化模板作为友元。因此，此处允许使用
        return utility::to_address(val.current);
    }
};

namespace rainy::utility {
    template <typename Iter>
    struct iterator_category {
        using type = typename Iter::iterator_category;
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename = void>
    static RAINY_CONSTEXPR_BOOL is_proxy_equal_with_noexcept = false;

    template <typename Ty>
    static RAINY_CONSTEXPR_BOOL is_proxy_equal_with_noexcept<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().proxy_equal_with_helper(
                utility::declval<const Ty &>()))>> =
        noexcept(utility::declval<const Ty &>().proxy_equal_with_helper(utility::declval<const Ty &>()));
}

namespace rainy::utility {
    template <typename Implement, typename Traits>
    class bidirectional_iterator {
    public:
        using implement_type = Implement;
        /* 由于编译器的特性，可能无法找到Implement定义的类型，因此，我们此处需要用户提供驱动此迭代器工作用的iterator_traits */
        using iterator_traits = Traits;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using pointer = value_type &;
        using reference = value_type &;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;
#if RAINY_HAS_CXX20
        using iterator_concept = typename iterator_traits::iterator_category;
#endif

        template <typename UImplement, typename UTraits>
        friend class bidirectional_iterator;

        RAINY_CONSTEXPR20 decltype(auto) operator*() noexcept(noexcept(static_cast<implement_type *>(this)->get_element_impl())) {
            return static_cast<implement_type *>(this)->get_element_impl();
        }

        RAINY_CONSTEXPR20 decltype(auto) operator->() noexcept(noexcept(static_cast<implement_type *>(this)->get_pointer_impl())) {
            return static_cast<implement_type *>(this)->get_pointer_impl();
        }

        RAINY_CONSTEXPR20 decltype(auto) operator*() const
            noexcept(noexcept(static_cast<const implement_type *>(this)->get_element_impl())) {
            return static_cast<const implement_type *>(this)->get_element_impl();
        }

        RAINY_CONSTEXPR20 decltype(auto) operator->() const
            noexcept(noexcept(static_cast<const implement_type *>(this)->get_pointer_impl())) {
            return static_cast<const implement_type *>(this)->get_pointer_impl();
        }

        RAINY_CONSTEXPR20 implement_type &operator++() noexcept(noexcept(static_cast<implement_type *>(this)->next_impl())) {
            static_cast<implement_type *>(this)->next_impl();
            return *static_cast<implement_type *>(this);
        }

        RAINY_CONSTEXPR20 implement_type operator++(int) noexcept(noexcept(static_cast<implement_type *>(this)->next_impl())) {
            implement_type tmp = *static_cast<implement_type *>(this);
            ++(*this);
            return tmp;
        }

        RAINY_CONSTEXPR20 implement_type &operator++() const
            noexcept(noexcept(static_cast<implement_type *>(const_cast<bidirectional_iterator *>(this))->next_impl())) {
            auto self = static_cast<implement_type *>(const_cast<bidirectional_iterator *>(this));
            self->next_impl();
            return *self;
        }

        RAINY_CONSTEXPR20 implement_type operator++(int) const {
            implement_type tmp = *static_cast<const implement_type *>(this);
            ++(*this);
            return tmp;
        }

        RAINY_CONSTEXPR20 implement_type &operator--() noexcept(noexcept(static_cast<implement_type *>(this)->back_impl())) {
            static_cast<implement_type *>(this)->back_impl();
            return *static_cast<implement_type *>(this);
        }

        RAINY_CONSTEXPR20 implement_type operator--(int) noexcept(noexcept(static_cast<implement_type *>(this)->back_impl())) {
            implement_type tmp = *static_cast<implement_type *>(this);
            --(*this);
            return tmp;
        }

        friend RAINY_CONSTEXPR20 bool operator==(const bidirectional_iterator &left, const bidirectional_iterator &right) noexcept(
            implements::is_proxy_equal_with_noexcept<bidirectional_iterator>) {
            return left.proxy_equal_with_helper(right);
        }

        friend RAINY_CONSTEXPR20 bool operator!=(const bidirectional_iterator &left,
                                                 const bidirectional_iterator &right) noexcept(noexcept(left == right)) {
            return !(left == right);
        }

        RAINY_CONSTEXPR20 bool proxy_equal_with_helper(const bidirectional_iterator &right) const noexcept(
            noexcept(static_cast<const implement_type *>(this)->equal_with_impl(static_cast<const implement_type &>(right)))) {
            return static_cast<const implement_type *>(this)->equal_with_impl(static_cast<const implement_type &>(right));
        }
    };

    template <typename MapContainer, typename Iterator>
    class map_mapped_iterator_impl : public utility::bidirectional_iterator<map_mapped_iterator_impl<MapContainer, Iterator>,
                                     utility::make_iterator_traits<typename std::iterator_traits<Iterator>::difference_type,
                                     std::bidirectional_iterator_tag,
                                     typename MapContainer::mapped_type *,
                                     typename MapContainer::mapped_type &,
                                     typename MapContainer::mapped_type >> {
    public:
        using base = utility::bidirectional_iterator<map_mapped_iterator_impl<MapContainer, Iterator>,
              utility::make_iterator_traits<typename std::iterator_traits<Iterator>::difference_type, std::bidirectional_iterator_tag,
              typename MapContainer::mapped_type *, typename MapContainer::mapped_type &, typename MapContainer::mapped_type >>
            ;

        // 关键：显式构造函数接受底层迭代器
        explicit map_mapped_iterator_impl(Iterator it) : current_(it) {
        }

        // 默认构造函数
        map_mapped_iterator_impl() = default;

        // 非 const 访问（仅当 Iterator 是非 const 时可用）
        template <
            typename Iter = Iterator,
            typename = std::enable_if_t<!std::is_const_v<std::remove_reference_t<typename std::iterator_traits<Iter>::reference>> >>
                       typename base::reference get_element_impl() noexcept {
            return current_->second;
        }

        template <
            typename Iter = Iterator,
            typename = std::enable_if_t<!std::is_const_v<std::remove_reference_t<typename std::iterator_traits<Iter>::reference>> >>
                       typename base::pointer get_pointer_impl() noexcept {
            return utility::addressof(current_->second);
        }

        // const 访问（总是可用）
        typename base::const_reference get_element_impl() const noexcept {
            return current_->second;
        }

        typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(current_->second);
        }

        void next_impl() noexcept {
            ++current_;
        }

        void back_impl() noexcept {
            --current_;
        }

        // 支持与不同 cv 限定的迭代器比较
        template <typename OtherIterator>
        bool equal_with_impl(const map_mapped_iterator_impl<MapContainer, OtherIterator> &other) const noexcept {
            return current_ == other.current_;
        }

    private:
        Iterator current_;

        template <typename M, typename I>
        friend class map_mapped_iterator_impl;
    };

    template <typename MapContainer>
    class map_mapped_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::iterator>;

        using base::base;
    };

    template <typename MapContainer>
    class map_mapped_const_iterator : public map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator> {
    public:
        using base = map_mapped_iterator_impl<MapContainer, typename MapContainer::const_iterator>;

        using base::base;
    };

    template <typename Map>
    rain_fn mapped_range(Map &map) -> collections::views::iterator_range<map_mapped_iterator<Map>> {
        using iterator = map_mapped_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator(map.begin()), iterator(map.end())};
    }

    template <typename Map>
    rain_fn mapped_range(const Map &map) -> collections::views::iterator_range<map_mapped_const_iterator<Map>> {
        using iterator = map_mapped_const_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator(map.cbegin()), iterator(map.cend())};
    }

    template <typename MapContainer>
    class map_key_const_iterator
        : public utility::bidirectional_iterator<
              map_key_const_iterator<MapContainer>,
              utility::make_iterator_traits<typename MapContainer::iterator::difference_type, std::bidirectional_iterator_tag,
                                            typename MapContainer::key_type *, typename MapContainer::key_type &,
                                            typename MapContainer::key_type>> {
    public:
        template <typename MapContainer_>
        friend class map_mapped_iterator;

        using base = utility::bidirectional_iterator<
            map_key_const_iterator<MapContainer>,
            utility::make_iterator_traits<typename MapContainer::iterator::difference_type, std::bidirectional_iterator_tag,
                                          typename MapContainer::key_type *, typename MapContainer::key_type &,
                                          typename MapContainer::key_type>>;

        template <typename Ty>
        explicit map_key_const_iterator(Ty it) : current_(it) {
        }

        typename base::const_reference get_element_impl() const noexcept {
            return current_->first;
        }

        typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(current_->first);
        }

        void next_impl() noexcept {
            ++current_;
        }

        void back_impl() noexcept {
            --current_;
        }

        bool equal_with_impl(const map_key_const_iterator &other) const noexcept {
            return current_ == other.current_;
        }

    private:
        typename MapContainer::const_iterator current_;
    };

    template <typename MapContainer>
    class map_key_iterator : public map_key_const_iterator<MapContainer> {
    public:
        using base = typename map_key_const_iterator<MapContainer>::base;

        using const_iterator = map_key_const_iterator<MapContainer>;
        using const_iterator::const_iterator;

        typename base::pointer get_pointer_impl() noexcept {
            return utility::addressof(this->current_->first);
        }

        typename base::reference &get_element_impl() noexcept {
            return const_cast<typename base::reference &>(this->current_->first);
        }
    };

    template <typename Map>
    rain_fn keyed_range(Map &map) -> collections::views::iterator_range<map_key_iterator<Map>> {
        using iterator = map_key_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator{map.begin()}, iterator{map.end()}};
    }

    template <typename Map>
    rain_fn keyed_range(const Map &map) -> collections::views::iterator_range<map_key_iterator<Map>> {
        using iterator = map_key_iterator<Map>;
        using range_view = collections::views::iterator_range<iterator>;
        return range_view{iterator{map.begin()}, iterator{map.end()}};
    }
}

#endif
