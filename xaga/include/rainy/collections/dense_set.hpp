#ifndef RAINY_COLLECTIONS_DENSE_SET_HPP
#define RAINY_COLLECTIONS_DENSE_SET_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/memory/allocator.hpp>
#include <rainy/utility.hpp>
#include <rainy/utility/iterator.hpp>

namespace rainy::collections::implements {
    template <typename Key>
    struct dense_set_node final {
        using value_type = Key;

        template <typename... Args>
        dense_set_node(const std::size_t pos, Args &&...args) : next{pos}, elem{utility::forward<Args>(args)...} {
        }

        template <typename Allocator, typename... Args>
        dense_set_node(std::allocator_arg_t, const Allocator &allocator, const std::size_t pos, Args &&...args) :
            next{pos}, elem{utility::make_obj_using_allocator<value_type>(allocator, utility::forward<Args>(args)...)} {
        }

        template <typename Allocator>
        dense_set_node(std::allocator_arg_t, const Allocator &allocator, const dense_set_node &other) :
            next{other.next}, elem{utility::make_obj_using_allocator<value_type>(allocator, other.elem)} {
        }

        template <typename Allocator>
        dense_set_node(std::allocator_arg_t, const Allocator &allocator, dense_set_node &&other) :
            next{other.next}, elem{utility::make_obj_using_allocator<value_type>(allocator, utility::move(other.elem))} {
        }

        std::size_t next;
        value_type elem;
    };

    template <typename Iter>
    class dense_set_iterator final {
        template <typename>
        friend class dense_set_iterator;

        using elem_type = decltype(std::as_const(std::declval<Iter>()->elem));

    public:
        using value_type = elem_type;
        using pointer = utility::input_iterator_pointer<value_type>;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;

        constexpr dense_set_iterator() noexcept : it{} {
        }

        constexpr dense_set_iterator(const Iter iter) noexcept : it{iter} {
        }

        template <typename Uty,
                  typename = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Iter, Uty> &&
                                                                   type_traits::type_properties::is_constructible_v<Iter, Uty>>>
        constexpr dense_set_iterator(const dense_set_iterator<Uty> &other) noexcept : it{other.it} {
        }

        constexpr dense_set_iterator &operator++() noexcept {
            return ++it, *this;
        }

        constexpr dense_set_iterator operator++(int) noexcept {
            const dense_set_iterator orig = *this;
            return ++(*this), orig;
        }

        constexpr dense_set_iterator &operator--() noexcept {
            return --it, *this;
        }

        constexpr dense_set_iterator operator--(int) noexcept {
            const dense_set_iterator orig = *this;
            return operator--(), orig;
        }

        constexpr dense_set_iterator &operator+=(const difference_type value) noexcept {
            it += value;
            return *this;
        }

        constexpr dense_set_iterator operator+(const difference_type value) const noexcept {
            dense_set_iterator tmp = *this;
            return (tmp += value);
        }

        constexpr dense_set_iterator &operator-=(const difference_type value) noexcept {
            return (*this += -value);
        }

        constexpr dense_set_iterator operator-(const difference_type value) const noexcept {
            return (*this + -value);
        }

        RAINY_NODISCARD constexpr reference operator[](const difference_type value) const noexcept {
            return it[value].elem;
        }

        RAINY_NODISCARD constexpr pointer operator->() const noexcept {
            return operator*();
        }

        RAINY_NODISCARD constexpr reference operator*() const noexcept {
            return operator[](0);
        }

        template <typename Left, typename Right>
        friend constexpr std::ptrdiff_t operator-(const dense_set_iterator<Left> &, const dense_set_iterator<Right> &) noexcept;

        template <typename Left, typename Right>
        friend constexpr bool operator==(const dense_set_iterator<Left> &, const dense_set_iterator<Right> &) noexcept;

        template <typename Left, typename Right>
        friend constexpr bool operator<(const dense_set_iterator<Left> &, const dense_set_iterator<Right> &) noexcept;

    private:
        Iter it;
    };

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr std::ptrdiff_t operator-(const dense_set_iterator<Left> &left,
                                                       const dense_set_iterator<Right> &right) noexcept {
        return left.it - right.it;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator==(const dense_set_iterator<Left> &left, const dense_set_iterator<Right> &right) noexcept {
        return left.it == right.it;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator!=(const dense_set_iterator<Left> &left, const dense_set_iterator<Right> &right) noexcept {
        return !(left == right);
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator<(const dense_set_iterator<Left> &left, const dense_set_iterator<Right> &right) noexcept {
        return left.it < right.it;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator>(const dense_set_iterator<Left> &left, const dense_set_iterator<Right> &right) noexcept {
        return right < left;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator<=(const dense_set_iterator<Left> &left, const dense_set_iterator<Right> &right) noexcept {
        return !(left > right);
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator>=(const dense_set_iterator<Left> &left, const dense_set_iterator<Right> &right) noexcept {
        return !(left < right);
    }

    template <typename Iter>
    class dense_set_local_iterator final {
        template <typename>
        friend class dense_set_local_iterator;

        using elem_type = decltype(std::as_const(std::declval<Iter>()->elem));

    public:
        using value_type = elem_type;
        using pointer = utility::input_iterator_pointer<value_type>;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        constexpr dense_set_local_iterator() noexcept : it{}, offset{} {
        }

        constexpr dense_set_local_iterator(Iter iter, const std::size_t pos) noexcept : it{iter}, offset{pos} {
        }

        template <typename Uty,
                  typename = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Iter, Uty> &&
                                                                   type_traits::type_properties::is_constructible_v<Iter, Uty>>>
        constexpr dense_set_local_iterator(const dense_set_local_iterator<Uty> &other) noexcept : it{other.it}, offset{other.offset} {
        }

        constexpr dense_set_local_iterator &operator++() noexcept {
            return (offset = it[static_cast<typename Iter::difference_type>(offset)].next), *this;
        }

        constexpr dense_set_local_iterator operator++(int) noexcept {
            const dense_set_local_iterator orig = *this;
            return ++(*this), orig;
        }

        RAINY_NODISCARD constexpr pointer operator->() const noexcept {
            return operator*();
        }

        RAINY_NODISCARD constexpr reference operator*() const noexcept {
            const auto idx = static_cast<typename Iter::difference_type>(offset);
            return it[idx].elem;
        }

        RAINY_NODISCARD constexpr std::size_t index() const noexcept {
            return offset;
        }

    private:
        Iter it;
        std::size_t offset;
    };

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator==(const dense_set_local_iterator<Left> &left,
                                              const dense_set_local_iterator<Right> &right) noexcept {
        return left.index() == right.index();
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator!=(const dense_set_local_iterator<Left> &left,
                                              const dense_set_local_iterator<Right> &right) noexcept {
        return !(left == right);
    }
}

namespace rainy::collections {
    template <typename Key, typename Hash = utility::hash<Key>, typename KeyEqual = foundation::functional::equal<Key>,
              typename Allocator = std::allocator<Key>>
    class dense_set {
    public:
        static constexpr float default_loadfactor = 0.875f;
        static constexpr std::size_t minimum_capacity = 8u;

        using node_type = implements::dense_set_node<Key>;
        using alloc_traits = std::allocator_traits<Allocator>;

        static_assert(type_traits::type_relations::is_same_v<typename alloc_traits::value_type, Key>, "Invalid value type");
        using sparse_container_t = std::vector<std::size_t, typename alloc_traits::template rebind_alloc<std::size_t>>;
        using node_container_t = std::vector<node_type, typename alloc_traits::template rebind_alloc<node_type>>;
        using allocator_type = Allocator;
        using key_type = Key;
        using value_type = Key;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher = Hash;
        using key_equal = KeyEqual;
        using iterator = implements::dense_set_iterator<typename node_container_t::iterator>;
        using const_iterator = implements::dense_set_iterator<typename node_container_t::const_iterator>;
        using local_iterator = implements::dense_set_local_iterator<typename node_container_t::iterator>;
        using const_local_iterator = implements::dense_set_local_iterator<typename node_container_t::const_iterator>;

        dense_set() : dense_set{minimum_capacity} {
        }

        explicit dense_set(const allocator_type &allocator) : dense_set{minimum_capacity, hasher{}, key_equal{}, allocator} {
        }

        dense_set(const size_type count, const allocator_type &allocator) : dense_set{count, hasher{}, key_equal{}, allocator} {
        }

        dense_set(const size_type count, const hasher &hash, const allocator_type &allocator) :
            dense_set{count, hash, key_equal{}, allocator} {
        }

        explicit dense_set(const size_type count, const hasher &hash = hasher{}, const key_equal &equal = key_equal{},
                           const allocator_type &allocator = allocator_type{}) :
            storage{node_container_t{allocator}, sparse_container_t{allocator}} {
            rehash(count);
        }

        dense_set(const dense_set &) = default;

        dense_set(const dense_set &other, const allocator_type &allocator) : storage{other.storage}, load_factor_{other.load_factor_} {
        }

        dense_set(dense_set &&) noexcept = default;

        dense_set(dense_set &&other, const allocator_type &allocator) :
            storage{utility::move(other.storage)}, load_factor_{other.load_factor_} {
        }

        dense_set(std::initializer_list<value_type> ilist) {
            reserve(static_cast<std::size_t>(ilist.size()));
            for (const auto &item: ilist) {
                insert(item);
            }
        }

        ~dense_set() = default;

        dense_set &operator=(const dense_set &) = default;
        dense_set &operator=(dense_set &&) noexcept = default;

        dense_set &operator=(std::initializer_list<value_type> ilist) {
            clear();
            reserve(static_cast<std::size_t>(ilist.size()));
            for (const auto &item: ilist) {
                insert(item);
            }
            return *this;
        }

        void swap(dense_set &other) noexcept {
            using std::swap;
            swap(storage, other.storage);
            swap(load_factor_, other.load_factor_);
        }

        RAINY_NODISCARD constexpr allocator_type get_allocator() const noexcept {
            return storage.second.get_allocator();
        }

        RAINY_NODISCARD const_iterator cbegin() const noexcept {
            return storage.first.begin();
        }

        RAINY_NODISCARD const_iterator begin() const noexcept {
            return cbegin();
        }

        RAINY_NODISCARD iterator begin() noexcept {
            return storage.first.begin();
        }

        RAINY_NODISCARD const_iterator cend() const noexcept {
            return storage.first.end();
        }

        RAINY_NODISCARD const_iterator end() const noexcept {
            return cend();
        }

        RAINY_NODISCARD iterator end() noexcept {
            return storage.first.end();
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return storage.first.empty();
        }

        RAINY_NODISCARD size_type size() const noexcept {
            return storage.first.size();
        }

        RAINY_NODISCARD size_type max_size() const noexcept {
            return storage.first.max_size();
        }

        void clear() noexcept {
            storage.first.clear();
            storage.second.clear();
            rehash(0u);
        }

        utility::pair<iterator, bool> insert(const value_type &value) {
            return insert_or_do_nothing(value);
        }

        utility::pair<iterator, bool> insert(value_type &&value) {
            return insert_or_do_nothing(utility::move(value));
        }

        template <typename Iter>
        void insert(Iter first, Iter last) {
            for (; first != last; ++first) {
                insert(*first);
            }
        }

        template <typename... Args>
        utility::pair<iterator, bool> emplace(Args &&...args) {
            return insert_or_do_nothing(utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::pair<iterator, bool> emplace_hint([[maybe_unused]] const_iterator hint, Args &&...args) {
            (void) hint;
            return emplace(utility::forward<Args>(args)...);
        }

        iterator erase(const_iterator pos) {
            const auto diff = pos - cbegin();
            erase(*pos);
            return begin() + diff;
        }

        iterator erase(const_iterator first, const_iterator last) {
            const auto dist = first - cbegin();

            for (auto from = last - cbegin(); from != dist; --from) {
                erase(storage.first[static_cast<size_type>(from) - 1u].elem);
            }

            return (begin() + dist);
        }

        size_type erase(const key_type &keyval) {
            const auto &equal = tools.get_first();
            for (size_type *curr = &storage.second[key_to_bucket(keyval)]; *curr != (utility::numeric_limits<size_type>::max)();
                 curr = &storage.first[*curr].next) {
                if (equal(storage.first[*curr].elem, keyval)) {
                    const auto index = *curr;
                    *curr = storage.first[*curr].next;
                    move_and_pop(index);
                    return 1u;
                }
            }
            return 0u;
        }

        RAINY_NODISCARD size_type count(const key_type &keyval) const {
            return find(keyval) != end();
        }

        template <typename Uty>
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<type_traits::type_properties::is_transparent_v<hasher> &&
                                                                  type_traits::type_properties::is_transparent_v<key_equal>,
                                                              std::conditional_t<false, Uty, size_type>>
        count(const Uty &keyval) const {
            return find(keyval) != end();
        }

        RAINY_NODISCARD iterator find(const key_type &keyval) {
            return constrained_find(keyval, key_to_bucket(keyval));
        }

        RAINY_NODISCARD const_iterator find(const key_type &keyval) const {
            return constrained_find(keyval, key_to_bucket(keyval));
        }

        template <typename Uty>
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<type_traits::type_properties::is_transparent_v<hasher> &&
                                                                  type_traits::type_properties::is_transparent_v<key_equal>,
                                                              std::conditional_t<false, Uty, iterator>>
        find(const Uty &keyval) {
            return constrained_find(keyval, key_to_bucket(keyval));
        }

        template <typename Uty>
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<type_traits::type_properties::is_transparent_v<hasher> &&
                                                                  type_traits::type_properties::is_transparent_v<key_equal>,
                                                              std::conditional_t<false, Uty, const_iterator>>
        find(const Uty &keyval) const {
            return constrained_find(keyval, key_to_bucket(keyval));
        }

        RAINY_NODISCARD utility::pair<iterator, iterator> equal_range(const key_type &keyval) {
            const auto it = find(keyval);
            return {it, it + !(it == end())};
        }

        RAINY_NODISCARD utility::pair<const_iterator, const_iterator> equal_range(const key_type &keyval) const {
            const auto it = find(keyval);
            return {it, it + !(it == cend())};
        }

        template <typename Uty>
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<type_traits::type_properties::is_transparent_v<hasher> &&
                                                                  type_traits::type_properties::is_transparent_v<key_equal>,
                                                              std::conditional_t<false, Uty, utility::pair<iterator, iterator>>>
        equal_range(const Uty &keyval) {
            const auto it = find(keyval);
            return {it, it + !(it == end())};
        }

        template <typename Uty>
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<type_traits::type_properties::is_transparent_v<hasher>
            &&type_traits::type_properties::is_transparent_v<key_equal>, std::conditional_t < false, Uty, utility::pair<const_iterator, const_iterator>>>
        equal_range(const Uty &keyval) const {
            const auto it = find(keyval);
            return {it, it + !(it == cend())};
        }

        RAINY_NODISCARD bool contains(const key_type &keyval) const {
            return (find(keyval) != cend());
        }

        template <typename Uty>
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<type_traits::type_properties::is_transparent_v<hasher> &&
                                                                  type_traits::type_properties::is_transparent_v<key_equal>,
                                                              std::conditional_t<false, Uty, bool>>
        contains(const Uty &keyval) const {
            return (find(keyval) != cend());
        }

        RAINY_NODISCARD const_local_iterator cbegin(const size_type index) const {
            return {storage.first.begin(), storage.second[index]};
        }

        RAINY_NODISCARD const_local_iterator begin(const size_type index) const {
            return cbegin(index);
        }

        RAINY_NODISCARD local_iterator begin(const size_type index) {
            return {storage.first.begin(), storage.second[index]};
        }

        RAINY_NODISCARD const_local_iterator cend([[maybe_unused]] const size_type index) const {
            return {storage.first.begin(), (utility::numeric_limits<size_type>::max)()};
        }

        RAINY_NODISCARD const_local_iterator end(const size_type index) const {
            return cend(index);
        }

        RAINY_NODISCARD local_iterator end([[maybe_unused]] const size_type index) {
            return {storage.first.begin(), (utility::numeric_limits<size_type>::max)()};
        }

        RAINY_NODISCARD size_type bucket_count() const {
            return storage.second.size();
        }

        RAINY_NODISCARD size_type max_bucket_count() const {
            return storage.second.max_size();
        }

        RAINY_NODISCARD size_type bucket_size(const size_type index) const {
            return static_cast<size_type>(std::distance(begin(index), end(index)));
        }

        RAINY_NODISCARD size_type bucket(const key_type &keyval) const {
            return key_to_bucket(keyval);
        }

        RAINY_NODISCARD float load_factor() const {
            return static_cast<float>(size()) / static_cast<float>(bucket_count());
        }

        RAINY_NODISCARD float max_load_factor() const {
            return load_factor_;
        }

        void max_load_factor(const float value) {
            assert(value > 0.f && "Invalid load factor");
            load_factor_ = value;
            rehash(0u);
        }

        void rehash(const size_type count) {
            auto value = count > minimum_capacity ? count : minimum_capacity;
            const auto cap = static_cast<size_type>(static_cast<float>(size()) / max_load_factor());
            value = value > cap ? value : cap;
            if (const auto sz = core::builtin::next_power_of_two(value); sz != bucket_count()) {
                storage.second.resize(sz);
                for (auto &&elem: storage.second) {
                    elem = (utility::numeric_limits<size_type>::max)();
                }
                for (size_type pos{}, last = size(); pos < last; ++pos) {
                    const auto index = key_to_bucket(storage.first[pos].elem);
                    storage.first[pos].next = std::exchange(storage.second[index], pos);
                }
            }
        }

        void reserve(const size_type count) {
            storage.first.reserve(count);
            storage.second.reserve(count);
            rehash(static_cast<size_type>(std::ceil(static_cast<float>(count) / max_load_factor())));
        }

        RAINY_NODISCARD key_equal key_eq() const {
            return tools.get_first();
        }

        RAINY_NODISCARD hasher hash_function() const {
            return tools.get_second();
        }

    private:
        template <typename Uty>
        RAINY_NODISCARD std::size_t key_to_bucket(const Uty &keyval) const noexcept {
            return core::builtin::mod(static_cast<size_type>(tools.get_second()(keyval)), bucket_count());
        }

        template <typename Uty>
        RAINY_NODISCARD auto constrained_find(const Uty &keyval, std::size_t bucket) {
            const auto &equal = tools.get_first();
            for (auto it = begin(bucket), last = end(bucket); it != last; ++it) {
                if (equal(*it, keyval)) {
                    return begin() + static_cast<difference_type>(it.index());
                }
            }
            return end();
        }

        template <typename Uty>
        RAINY_NODISCARD auto constrained_find(const Uty &keyval, std::size_t bucket) const {
            const auto &equal = tools.get_first();
            for (auto it = cbegin(bucket), last = cend(bucket); it != last; ++it) {
                if (equal(*it, keyval)) {
                    return cbegin() + static_cast<difference_type>(it.index());
                }
            }
            return cend();
        }

        template <typename... Args>
        RAINY_NODISCARD auto insert_or_do_nothing(Args &&...args) {
            auto elem = value_type{utility::forward<Args>(args)...};
            const auto index = key_to_bucket(elem);
            if (auto it = constrained_find(elem, index); it != end()) {
                return utility::pair{it, false};
            }
            storage.first.emplace_back(storage.second[index], utility::move(elem));
            storage.second[index] = storage.first.size() - 1u;
            rehash_if_required();
            return utility::pair{--end(), true};
        }

        void move_and_pop(const std::size_t pos) {
            if (const auto last = size() - 1u; pos != last) {
                size_type *curr = &storage.second[key_to_bucket(storage.first.back().elem)];
                storage.first[pos] = utility::move(storage.first.back());
                for (; *curr != last; curr = &storage.first[*curr].next) {
                }
                *curr = pos;
            }
            storage.first.pop_back();
        }

        void rehash_if_required() {
            if (const auto bc = bucket_count(); size() > static_cast<size_type>(static_cast<float>(bc) * max_load_factor())) {
                rehash(bc * 2u);
            }
        }

        utility::pair<node_container_t, sparse_container_t> storage;
        utility::compressed_pair<key_equal, hasher> tools;
        float load_factor_{default_loadfactor};
    };
}

namespace std {
    template <typename Key, typename Allocator>
    struct uses_allocator<rainy::collections::implements::dense_set_node<Key>, Allocator> : std::true_type {};
}

#endif
