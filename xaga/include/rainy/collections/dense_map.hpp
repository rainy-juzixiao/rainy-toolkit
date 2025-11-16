#ifndef RAINY_COLLECTIONS_DENSE_MAP_HPP
#define RAINY_COLLECTIONS_DENSE_MAP_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/pair.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/utility.hpp>
#include <rainy/foundation/functional/functor.hpp>
#include <rainy/foundation/system/memory/allocator.hpp>

namespace rainy::collections::implements {
    template <typename Key, typename Mapped>
    struct dense_map_node final {
        using value_type = utility::pair<Key, Mapped>;

        template <typename... Args>
        dense_map_node(const std::size_t pos, Args &&...args) : next{pos}, elem{utility::forward<Args>(args)...} {
        }

        template <typename Allocator, typename... Args>
        dense_map_node(std::allocator_arg_t, const Allocator &allocator, const std::size_t pos, Args &&...args) :
            next{pos}, elem{utility::make_obj_using_allocator<value_type>(allocator, utility::forward<Args>(args)...)} {
        }

        template <typename Allocator>
        dense_map_node(std::allocator_arg_t, const Allocator &allocator, const dense_map_node &other) :
            next{other.next}, elem{utility::make_obj_using_allocator<value_type>(allocator, other.elem)} {
        }

        template <typename Allocator>
        dense_map_node(std::allocator_arg_t, const Allocator &allocator, dense_map_node &&other) :
            next{other.next}, elem{utility::make_obj_using_allocator<value_type>(allocator, utility::move(other.elem))} {
        }

        std::size_t next;
        value_type elem;
    };

    template <typename Iter>
    class dense_map_iterator final {
        template <typename>
        friend class dense_map_iterator;

        using first_type = decltype(std::as_const(std::declval<Iter>()->elem.first));
        using second_type = decltype((utility::declval<Iter>()->elem.second));

    public:
        using value_type = utility::pair<first_type, second_type>;
        using pointer = utility::input_iterator_pointer<value_type>;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;

        constexpr dense_map_iterator() noexcept : it{} {
        }

        constexpr dense_map_iterator(const Iter iter) noexcept : it{iter} {
        }

        template <typename Uty, typename = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Iter, Uty> && type_traits::type_properties::is_constructible_v<Iter, Uty>>>
        constexpr dense_map_iterator(const dense_map_iterator<Uty> &other) noexcept : it{other.it} {
        }

        constexpr dense_map_iterator &operator++() noexcept {
            return ++it, *this;
        }

        constexpr dense_map_iterator operator++(int) noexcept {
            const dense_map_iterator orig = *this;
            return ++(*this), orig;
        }

        constexpr dense_map_iterator &operator--() noexcept {
            return --it, *this;
        }

        constexpr dense_map_iterator operator--(int) noexcept {
            const dense_map_iterator orig = *this;
            return operator--(), orig;
        }

        constexpr dense_map_iterator &operator+=(const difference_type value) noexcept {
            it += value;
            return *this;
        }

        constexpr dense_map_iterator operator+(const difference_type value) const noexcept {
            dense_map_iterator tmp = *this;
            return (tmp += value);
        }

        constexpr dense_map_iterator &operator-=(const difference_type value) noexcept {
            return (*this += -value);
        }

        constexpr dense_map_iterator operator-(const difference_type value) const noexcept {
            return (*this + -value);
        }

        RAINY_NODISCARD constexpr reference operator[](const difference_type value) const noexcept {
            return {it[value].elem.first, it[value].elem.second};
        }

        RAINY_NODISCARD constexpr pointer operator->() const noexcept {
            return operator*();
        }

        RAINY_NODISCARD constexpr reference operator*() const noexcept {
            return operator[](0);
        }

        template <typename Left, typename Right>
        friend constexpr std::ptrdiff_t operator-(const dense_map_iterator<Left> &, const dense_map_iterator<Right> &) noexcept;

        template <typename Left, typename Right>
        friend constexpr bool operator==(const dense_map_iterator<Left> &, const dense_map_iterator<Right> &) noexcept;

        template <typename Left, typename Right>
        friend constexpr bool operator<(const dense_map_iterator<Left> &, const dense_map_iterator<Right> &) noexcept;

    private:
        Iter it;
    };

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr std::ptrdiff_t operator-(const dense_map_iterator<Left> &left,
                                                       const dense_map_iterator<Right> &right) noexcept {
        return left.it - right.it;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator==(const dense_map_iterator<Left> &left, const dense_map_iterator<Right> &right) noexcept {
        return left.it == right.it;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator!=(const dense_map_iterator<Left> &left, const dense_map_iterator<Right> &right) noexcept {
        return !(left == right);
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator<(const dense_map_iterator<Left> &left, const dense_map_iterator<Right> &right) noexcept {
        return left.it < right.it;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator>(const dense_map_iterator<Left> &left, const dense_map_iterator<Right> &right) noexcept {
        return right < left;
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator<=(const dense_map_iterator<Left> &left, const dense_map_iterator<Right> &right) noexcept {
        return !(left > right);
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator>=(const dense_map_iterator<Left> &left, const dense_map_iterator<Right> &right) noexcept {
        return !(left < right);
    }

    template <typename Iter>
    class dense_map_local_iterator final {
        template <typename>
        friend class dense_map_local_iterator;

        using first_type = decltype(std::as_const(std::declval<Iter>()->elem.first));
        using second_type = decltype((std::declval<Iter>()->elem.second));

    public:
        using value_type = utility::pair<first_type, second_type>;
        using pointer = utility::input_iterator_pointer<value_type>;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        constexpr dense_map_local_iterator() noexcept : it{}, offset{} {
        }

        constexpr dense_map_local_iterator(Iter iter, const std::size_t pos) noexcept : it{iter}, offset{pos} {
        }

        template <typename Uty, typename = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Iter, Uty> && type_traits::type_properties::is_constructible_v<Iter, Uty>>>
        constexpr dense_map_local_iterator(const dense_map_local_iterator<Uty> &other) noexcept :
            it{other.it}, offset{other.offset} {
        }

        constexpr dense_map_local_iterator &operator++() noexcept {
            return (offset = it[static_cast<typename Iter::difference_type>(offset)].next), *this;
        }

        constexpr dense_map_local_iterator operator++(int) noexcept {
            const dense_map_local_iterator orig = *this;
            return ++(*this), orig;
        }

        RAINY_NODISCARD constexpr pointer operator->() const noexcept {
            return operator*();
        }

        RAINY_NODISCARD constexpr reference operator*() const noexcept {
            const auto idx = static_cast<typename Iter::difference_type>(offset);
            return {it[idx].elem.first, it[idx].elem.second};
        }

        RAINY_NODISCARD constexpr std::size_t index() const noexcept {
            return offset;
        }

    private:
        Iter it;
        std::size_t offset;
    };

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator==(const dense_map_local_iterator<Left> &left,
                                              const dense_map_local_iterator<Right> &right) noexcept {
        return left.index() == right.index();
    }

    template <typename Left, typename Right>
    RAINY_NODISCARD constexpr bool operator!=(const dense_map_local_iterator<Left> &left,
                                              const dense_map_local_iterator<Right> &right) noexcept {
        return !(left == right);
    }
}

namespace rainy::collections {
    template <typename Key, typename Mapped, typename Hash = utility::hash<Key>,
              typename KeyEqual = foundation::functional::equal<Key>,
              typename Allocator = foundation::system::memory::allocator<utility::pair<const Key, Mapped>>>
    class dense_map {
    public:
        static constexpr float default_loadfactor = 0.875f;
        static constexpr std::size_t minimum_capacity = 8u;

        using node_type = implements::dense_map_node<Key, Mapped>;
        using alloc_traits = std::allocator_traits<Allocator>;

        using sparse_container_t = std::vector<std::size_t, typename alloc_traits::template rebind_alloc<std::size_t>>;
        using node_container_t = std::vector<node_type, typename alloc_traits::template rebind_alloc<node_type>>;
        using allocator_type = Allocator;
        using key_type = Key;
        using mapped_type = Mapped;
        using value_type = utility::pair<const Key, Mapped>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher = Hash;
        using key_equal = KeyEqual;
        using iterator = implements::dense_map_iterator<typename node_container_t::iterator>;
        using const_iterator = implements::dense_map_iterator<typename node_container_t::const_iterator>;
        using local_iterator = implements::dense_map_local_iterator<typename node_container_t::iterator>;
        using const_local_iterator = implements::dense_map_local_iterator<typename node_container_t::const_iterator>;

        dense_map() : dense_map{minimum_capacity} {
        }

        explicit dense_map(const allocator_type &allocator) : dense_map{minimum_capacity, hasher{}, key_equal{}, allocator} {
        }

        dense_map(const size_type count, const allocator_type &allocator) : dense_map{count, hasher{}, key_equal{}, allocator} {
        }

        dense_map(const size_type count, const hasher &hash, const allocator_type &allocator) :
            dense_map{count, hash, key_equal{}, allocator} {
        }

        explicit dense_map(const size_type count, const hasher &hash = hasher{}, const key_equal &equal = key_equal{},
                           const allocator_type &allocator = allocator_type{}) :
            storage{node_container_t(typename alloc_traits::template rebind_alloc<node_type>(allocator)),
                    sparse_container_t(typename alloc_traits::template rebind_alloc<std::size_t>(allocator))},
            tools{equal, hash} {
            rehash(count);
        }

        dense_map(const dense_map &) = default;
        dense_map(dense_map &&) noexcept = default;

        dense_map(const dense_map &right, const allocator_type &allocator) :
            storage{node_container_t(right.storage.first, typename alloc_traits::template rebind_alloc<node_type>(allocator)),
                    sparse_container_t(right.storage.second, typename alloc_traits::template rebind_alloc<node_type>(allocator))},
            tools{right.tools}, load_factor_{right.load_factor_} {
        }

        dense_map(dense_map &&right, const allocator_type &allocator) :
            storage{node_container_t(utility::move(right.storage.first),
                                     typename alloc_traits::template rebind_alloc<node_type>(allocator)),
                    sparse_container_t(utility::move(right.storage.second),
                                       typename alloc_traits::template rebind_alloc<node_type>(allocator))},
            tools{utility::move(right.tools)},
            load_factor_{right.load_factor_} {
        }

        dense_map(std::initializer_list<utility::pair<const key_type, mapped_type>> ilist) {
            reserve(static_cast<std::size_t>(ilist.size()));
            for (const auto &item: ilist) {
                insert(item);
            }
        }

        ~dense_map() = default;

        dense_map &operator=(const dense_map &) = default;
        dense_map &operator=(dense_map &&) = default;

        dense_map &operator=(std::initializer_list<utility::pair<const key_type, mapped_type>> ilist) {
            reserve(static_cast<std::size_t>(ilist.size()));
            for (const auto &item: ilist) {
                insert(item);
            }
            return *this;
        }

        void swap(dense_map &other) noexcept {
            using std::swap;
            swap(storage, other.storage);
            swap(load_factor_, other.load_factor_);
        }

        RAINY_NODISCARD allocator_type get_allocator() const noexcept {
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

        utility::pair<iterator, bool> insert(utility::in<value_type> value) {
            return insert_or_do_nothing(value.first, value.second);
        }

        utility::pair<iterator, bool> insert(value_type &&value) {
            return insert_or_do_nothing(utility::move(value.first), utility::move(value.second));
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<value_type, UTy &&>, int> = 0>
        utility::pair<iterator, bool> insert(UTy &&value) {
            return insert_or_do_nothing(utility::forward<UTy>(value).first, utility::forward<UTy>(value).second);
        }

        template <typename Iter>
        void insert(Iter first, Iter last) {
            for (; first != last; ++first) {
                insert(*first);
            }
        }

        template <typename Arg>
        utility::pair<iterator, bool> insert_or_assign(const key_type &keyval, Arg &&value) {
            return insert_or_overwrite(keyval, utility::forward<Arg>(value));
        }

        template <typename Arg>
        utility::pair<iterator, bool> insert_or_assign(key_type &&keyval, Arg &&value) {
            return insert_or_overwrite(utility::move(keyval), utility::forward<Arg>(value));
        }

        template <typename... Args>
        utility::pair<iterator, bool> emplace([[maybe_unused]] Args &&...args) {
            if constexpr (sizeof...(Args) == 0u) {
                return insert_or_do_nothing(key_type{});

            } else if constexpr (sizeof...(Args) == 1u) {
                return insert_or_do_nothing(utility::forward<Args>(args).first..., utility::forward<Args>(args).second...);

            } else if constexpr (sizeof...(Args) == 2u) {
                return insert_or_do_nothing(utility::forward<Args>(args)...);

            } else {
                auto &node = storage.first.emplace_back(storage.first.size(), utility::forward<Args>(args)...);
                const auto index = key_to_bucket(node.elem.first);
                if (auto it = constrained_find(node.elem.first, index); it != end()) {
                    storage.first.pop_back();
                    return utility::make_pair(it, false);
                }
                utility::swap(node.next, storage.second[index]);
                rehash_if_required();
                return utility::make_pair(--end(), true);
            }
        }

        template <typename... Args>
        utility::pair<iterator, bool> emplace_hint([[maybe_unused]] const_iterator hint, Args &&...args) {
            (void) hint;
            return emplace(utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::pair<iterator, bool> try_emplace(const key_type &keyval, Args &&...args) {
            return insert_or_do_nothing(keyval, utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::pair<iterator, bool> try_emplace(key_type &&keyval, Args &&...args) {
            return insert_or_do_nothing(utility::move(keyval), utility::forward<Args>(args)...);
        }

        iterator erase(const_iterator pos) {
            const auto diff = pos - cbegin();
            erase(pos->first);
            return begin() + diff;
        }

        iterator erase(const_iterator first, const_iterator last) {
            const auto dist = first - cbegin();
            for (auto from = last - cbegin(); from != dist; --from) {
                erase(storage.first[static_cast<size_type>(from) - 1u].elem.first);
            }
            return (begin() + dist);
        }

        size_type erase(const key_type &keyval) {
            const auto &equal = tools.get_first();
            for (size_type *curr = &storage.second[key_to_bucket(keyval)]; *curr != (utility::numeric_limits<size_type>::max)();
                 curr = &storage.first[*curr].next) {
                if (equal(storage.first[*curr].elem.first, keyval)) {
                    const auto index = *curr;
                    *curr = storage.first[*curr].next;
                    move_and_pop(index);
                    return 1u;
                }
            }
            return 0u;
        }

        RAINY_NODISCARD mapped_type &at(utility::in<key_type> keyval) {
            auto it = find(keyval);
            assert(it != end() && "Invalid keyval");
            return it->second;
        }

        RAINY_NODISCARD const mapped_type &at(utility::in<key_type> keyval) const {
            auto it = find(keyval);
            assert(it != cend() && "Invalid keyval");
            return it->second;
        }

        RAINY_NODISCARD mapped_type &operator[](utility::in<key_type> keyval) {
            return insert_or_do_nothing(keyval).first->second;
        }

        RAINY_NODISCARD mapped_type &operator[](key_type &&keyval) {
            return insert_or_do_nothing(utility::move(keyval)).first->second;
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

        RAINY_NODISCARD iterator find(utility::in<key_type> keyval) {
            return constrained_find(keyval, key_to_bucket(keyval));
        }

        RAINY_NODISCARD const_iterator find(utility::in<key_type> keyval) const {
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
        RAINY_NODISCARD type_traits::other_trans::enable_if_t<
            type_traits::type_properties::is_transparent_v<hasher> && type_traits::type_properties::is_transparent_v<key_equal>,
            std::conditional_t<false, Uty, utility::pair<const_iterator, const_iterator>>>
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
                    const auto index = key_to_bucket(storage.first[pos].elem.first);
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
                if (equal(it->first, keyval)) {
                    return begin() + static_cast<difference_type>(it.index());
                }
            }
            return end();
        }

        template <typename Uty>
        RAINY_NODISCARD auto constrained_find(const Uty &keyval, std::size_t bucket) const {
            const auto &equal = tools.get_first();
            for (auto it = cbegin(bucket), last = cend(bucket); it != last; ++it) {
                if (equal(it->first, keyval)) {
                    return cbegin() + static_cast<difference_type>(it.index());
                }
            }

            return cend();
        }

        template <typename Uty, typename... Args>
        RAINY_NODISCARD auto insert_or_do_nothing(Uty &&keyval, Args &&...args) {
            const auto index = key_to_bucket(keyval);
            if (auto it = constrained_find(keyval, index); it != end()) {
                return utility::pair{it, false};
            }
            storage.first.emplace_back(storage.second[index], std::piecewise_construct,
                                       std::forward_as_tuple(utility::forward<Uty>(keyval)),
                                       std::forward_as_tuple(utility::forward<Args>(args)...));
            storage.second[index] = storage.first.size() - 1u;
            rehash_if_required();
            return utility::pair{--end(), true};
        }

        template <typename Uty, typename Arg>
        RAINY_NODISCARD auto insert_or_overwrite(Uty &&keyval, Arg &&value) {
            const auto index = key_to_bucket(keyval);
            if (auto it = constrained_find(keyval, index); it != end()) {
                it->second = utility::forward<Arg>(value);
                return std::make_pair(it, false);
            }
            storage.first.emplace_back(storage.second[index], utility::forward<Uty>(keyval), utility::forward<Arg>(value));
            storage.second[index] = storage.first.size() - 1u;
            rehash_if_required();
            return std::make_pair(--end(), true);
        }

        void move_and_pop(const std::size_t pos) {
            if (const auto last = size() - 1u; pos != last) {
                size_type *curr = &storage.second[key_to_bucket(storage.first.back().elem.first)];
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
    template <typename Key, typename Value, typename Allocator>
    struct uses_allocator<rainy::collections::implements::dense_map_node<Key, Value>, Allocator> : std::true_type {};
}

#endif