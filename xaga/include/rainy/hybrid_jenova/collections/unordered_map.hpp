#ifndef RAINY_HYBRID_JENOVA_UNORDRED_MAP_HPP
#define RAINY_HYBRID_JENOVA_UNORDRED_MAP_HPP
#include <optional>
#include <rainy/core/core.hpp>
#include <rainy/hybrid_jenova/constexpr_hash.hpp>
#include <rainy/utility/pair.hpp>

namespace rainy::hybrid_jenova::collections {
#if RAINY_HAS_CXX23
    template <typename Key, typename Val, std::size_t N, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<void>>
    class hybrid_unordered_map;
#else
    template <typename Key, typename Val, std::size_t N, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
    class hybrid_unordered_map;
#endif
}

namespace rainy::hybrid_jenova::collections::implements {
    template <typename Pair>
    using umap_iterator_traits = utility::make_iterator_traits<std::ptrdiff_t, std::bidirectional_iterator_tag, Pair *, Pair &, Pair>;

    template <typename Key, typename Val, std::size_t N, typename Hasher, typename KeyEqual>
    class hybrid_umap_iterator
        : public utility::bidirectional_iterator<hybrid_umap_iterator<Key, Val, N, Hasher, KeyEqual>,
                                                 implements::umap_iterator_traits<utility::pair<const Key, Val>>> {
    public:
        using container_type = hybrid_unordered_map<Key, Val, N, Hasher, KeyEqual>;
        using value_type = utility::pair<const Key, Val>;
        using pointer = value_type *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using base = utility::bidirectional_iterator<hybrid_umap_iterator, implements::umap_iterator_traits<value_type>>;

        RAINY_CONSTEXPR20 hybrid_umap_iterator(const container_type *container, std::size_t idx) :
            container(container), current_index(idx) {
            find_next_occupied_forward();
        }

        RAINY_CONSTEXPR20 hybrid_umap_iterator(const hybrid_umap_iterator &) noexcept = default;
        RAINY_CONSTEXPR20 hybrid_umap_iterator(hybrid_umap_iterator &&) noexcept = default;

    private:
        static constexpr inline std::size_t npos = static_cast<std::size_t>(-1);

        friend class utility::bidirectional_iterator<hybrid_umap_iterator, implements::umap_iterator_traits<value_type>>;
        friend class collections::hybrid_unordered_map<Key, Val, N, Hasher, KeyEqual>;

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return container == nullptr;
        }

        RAINY_CONSTEXPR20 bool is_valid() const noexcept {
            return this->current_index >= 0 && this->current_index < N;
        }

        RAINY_CONSTEXPR20 bool is_same_container(const container_type *container) const noexcept {
            return this->container == container;
        }

        RAINY_CONSTEXPR20 typename base::const_reference get_element_impl() const noexcept {
            return container->data[current_index];
        }

        RAINY_CONSTEXPR20 typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(container->data[current_index]);
        }

        RAINY_CONSTEXPR20 typename base::reference get_element_impl() noexcept {
            return const_cast<reference>(container->data[current_index]);
        }

        RAINY_CONSTEXPR20 typename base::pointer get_pointer_impl() noexcept {
            return const_cast<pointer>(utility::addressof(container->data[current_index]));
        }

        RAINY_CONSTEXPR20 void next_impl() noexcept {
            ++current_index;
            find_next_occupied_forward();
        }

        RAINY_CONSTEXPR20 void back_impl() noexcept {
            --current_index;
            find_next_occupied_backward();
        }

        RAINY_CONSTEXPR20 bool equal_with_impl(const hybrid_umap_iterator &right) const noexcept {
            return this->container == right.container && this->current_index == right.current_index;
        }

        RAINY_CONSTEXPR20 void find_next_occupied_forward() {
            while (current_index < N && !container->occupied[current_index]) {
                ++current_index;
            }
        }

        RAINY_CONSTEXPR20 void find_next_occupied_backward() {
            while (current_index < N && !container->occupied[current_index]) {
                --current_index;
            }
        }

        const container_type *container;
        std::size_t current_index;
    };

    template <typename Key, typename Val>
    class umap_node_handle {
    public:
        using key_type = Key;
        using mapped_type = Val;
        using size_type = std::size_t;
        using value_type = utility::pair<key_type, mapped_type>;

        template <typename... Args, type_traits::other_trans::enable_if_t<
                                        type_traits::type_properties::is_constructible_v<value_type, Args...>, int> = 0>
        umap_node_handle(Args &&...args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<value_type, Args...>) :
            pair(std::in_place, utility::forward<Args>(args)...) {
        }

        value_type &get() noexcept {
            return pair;
        }

        const value_type &get() const noexcept {
            return pair;
        }

        key_type &key() const noexcept {
            return const_cast<key_type &>(pair->first);
        }

        mapped_type &mapped() const noexcept {
            return const_cast<mapped_type &>(pair->second);
        }

        bool empty() const noexcept {
            return !pair.has_value();
        }

        operator bool() const noexcept {
            return !pair.has_value();
        }

        value_type &&move_this() noexcept {
            return utility::move(*pair);
        }

    private:
        std::optional<value_type> pair;
    };

    template <typename KeyEqual, typename Key, typename Test, typename = void>
    RAINY_CONSTEXPR_BOOL umap_equal_to_test = false;

    template <typename KeyEqual, typename Key, typename Test>
    RAINY_CONSTEXPR_BOOL umap_equal_to_test<
        KeyEqual, Key, Test,
        type_traits::other_trans::void_t<decltype(utility::declval<KeyEqual>()(utility::declval<Key>(), utility::declval<Test>()))>> =
        true;

    template <typename KeyEqual, typename Key, typename Test>
    RAINY_CONSTEXPR_BOOL umap_support_transparent_and_equal =
        umap_equal_to_test<KeyEqual, Key, Test> && type_traits::type_properties::is_transparent_v<KeyEqual>;
}

namespace rainy::hybrid_jenova::collections {
    template <typename Key, typename Val, std::size_t N, typename Hasher, typename KeyEqual>
    class hybrid_unordered_map {
    public:
        friend class implements::hybrid_umap_iterator<Key, Val, N, Hasher, KeyEqual>;

        using key_type = Key;
        using mapped_type = Val;
        using value_type = utility::pair<const key_type, mapped_type>;
        using hasher = Hasher;
        using key_equal = KeyEqual;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using node_type = implements::umap_node_handle<Key, Val>;

        using iterator = implements::hybrid_umap_iterator<Key, Val, N, Hasher, KeyEqual>;
        using const_iterator = const iterator;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = const utility::reverse_iterator<iterator>;

        RAINY_CONSTEXPR20 hybrid_unordered_map(std::initializer_list<value_type> init_list) : occupied{}, occupied_count(0), data{} {
            for (const auto &item: init_list) {
                std::size_t index = find_index(item.first);
#if RAINY_ENABLE_DEBUG
                utility::ct_expects(index != npos, "Current container is full!");
#endif
                if (!occupied[index]) {
                    utility::construct_at(&data[index], utility::move(item));
                    occupied[index] = true;
                    ++occupied_count;
                }
            }
        }

        RAINY_CONSTEXPR20 hybrid_unordered_map() : occupied{}, occupied_count(0), data{} {
        }

        RAINY_CONSTEXPR20 hybrid_unordered_map(const hybrid_unordered_map &) = default;
        RAINY_CONSTEXPR20 hybrid_unordered_map &operator=(const hybrid_unordered_map &) = default;
        RAINY_CONSTEXPR20 hybrid_unordered_map(hybrid_unordered_map &&) noexcept = default;
        RAINY_CONSTEXPR20 hybrid_unordered_map &operator=(hybrid_unordered_map &&) noexcept = default;

        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return iterator{this, 0};
        }

        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return const_iterator{this, 0};
        }

        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return const_iterator{this, 0};
        }

        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
            return reverse_iterator{end()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator{end()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator{end()};
        }

        RAINY_CONSTEXPR20 iterator end() noexcept {
            return iterator{this, N};
        }

        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return const_iterator{this, N};
        }

        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return const_iterator{this, N};
        }

        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
            return reverse_iterator{begin()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator{begin()};
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator{begin()};
        }

        RAINY_CONSTEXPR20 mapped_type &operator[](const Key &key) {
            std::size_t index = find_index(key);
            if (!occupied[index]) {
                insert(key, mapped_type{});
                index = find_index(key);
            }
            return data[index].second;
        }

        RAINY_CONSTEXPR20 const mapped_type &operator[](const Key &key) const {
            return at(key);
        }

        RAINY_CONSTEXPR20 iterator find(const key_type &keyval) {
            std::size_t index = bucket(keyval);
            if (occupied.size() <= index || !occupied.at(index)) {
                return end();
            }
            return iterator{this, index};
        }

        RAINY_CONSTEXPR20 const_iterator find(const key_type &keyval) const {
            std::size_t index = bucket(keyval);
            if (occupied.size() <= index || !occupied.at(index)) {
                return end();
            }
            return const_iterator{this, index};
        }

        RAINY_CONSTEXPR20 mapped_type &at(const key_type &keyval) {
            using foundation::system::exceptions::logic::throw_out_of_range;
            size_type index = bucket(keyval);
            if (index == npos) {
                throw_out_of_range("Key not found");
            } else if (occupied[index]) {
                throw_out_of_range("Key not found");
            }
            return data[index].second;
        }

        RAINY_CONSTEXPR20 const mapped_type &at(const key_type &keyval) const {
            using foundation::system::exceptions::logic::throw_out_of_range;
            size_type index = bucket(keyval);
            if (index == npos) {
                throw_out_of_range("Key not found");
            } else if (occupied[index]) {
                throw_out_of_range("Key not found");
            }
            return data[index].second;
        }

        template <typename UKey,type_traits::other_trans::enable_if_t<implements::umap_support_transparent_and_equal<key_equal,key_type,UKey>,int> = 0>
        RAINY_CONSTEXPR20 mapped_type &at(const UKey &keyval) {
            using foundation::system::exceptions::logic::throw_out_of_range;
            size_type index = bucket(keyval);
            if (index == npos) {
                throw_out_of_range("Key not found");
            } else if (occupied[index]) {
                throw_out_of_range("Key not found");
            }
            return data[index].second;
        }

        template <typename UKey, type_traits::other_trans::enable_if_t<implements::umap_support_transparent_and_equal<key_equal,key_type,UKey>,int> = 0>
        RAINY_CONSTEXPR20 const mapped_type &at(const UKey &keyval) const {
            using foundation::system::exceptions::logic::throw_out_of_range;
            size_type index = bucket(keyval);
            if (index == npos) {
                throw_out_of_range("Key not found");
            } else if (occupied[index]) {
                throw_out_of_range("Key not found");
            }
            return data[index].second;
        }

        RAINY_CONSTEXPR20 bool contains(const key_type &key) const {
            const auto &hasher = utils.get_first();
            const auto &key_equal = utils.get_second();

            std::size_t hash_value = hasher(key);
            std::size_t index = hash_value % N;
            std::size_t original_index = index;
            std::size_t probe = 0;
            do {
                if (!occupied[index]) {
                    return false;
                }
                if (key_equal(data[index].first, key)) {
                    return true;
                }
                probe++;
                index = (hash_value + probe) % N;
            } while (index != original_index);
            return false;
        }

        template <typename... Args>
        utility::pair<iterator, bool> emplace(Args &&...args) {
            auto pair = emplace_(utility::forward<Args>(args)...);
            if (pair.first == npos) {
                return {end(), false};
            }
            return {iterator{this, pair.second}, true};
        }

        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
#if RAINY_ENABLE_DEBUG
            if (hint.empty() || (!hint.is_valid() && !hint.is_same_container(this))) {
                return end();
            }
#else
            (void) hint;
#endif
            auto pair = emplace_(utility::forward<Args>(args)...);
            if (pair.first == npos) {
                return end();
            }
            return iterator{this, pair.second};
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 utility::pair<iterator, bool> try_emplace(const key_type &keyval, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<value_type, const key_type &, Args...>) {
            constexpr bool is_nothrow_construct =
                noexcept(type_traits::type_properties::is_nothrow_constructible_v<value_type, key_type &&, Args...>);
            return try_emplace_(
                [](value_type *placement,const key_type &keyval, Args &&...args) {
                    utility::construct_at(placement, std::piecewise_construct,
                                          std::forward_as_tuple(keyval),
                                          std::forward_as_tuple(utility::forward<Args>(args)...));
                },
                utility::forward<const key_type&>(keyval), utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::pair<iterator, bool> try_emplace(key_type &&keyval, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<value_type, key_type &&, Args...>) {
            constexpr bool is_nothrow_construct =
                noexcept(type_traits::type_properties::is_nothrow_constructible_v<value_type, key_type &&, Args...>);
            return try_emplace_(
                [](value_type *placement, key_type &&keyval, Args &&...args) {
                    utility::construct_at(placement, std::piecewise_construct,
                                          std::forward_as_tuple<key_type &&>(utility::move(keyval)),
                                          std::forward_as_tuple(utility::forward<Args>(args)...));
                },
                utility::forward<key_type>(keyval), utility::forward<Args>(args)...);
        }

        template <typename UKey, typename... Args, type_traits::other_trans::enable_if_t<implements::umap_support_transparent_and_equal<key_equal,key_type,UKey>,int> = 0>
        utility::pair<iterator, bool> try_emplace(UKey &&keyval, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<value_type, UKey &&, Args...>) {
            constexpr bool is_nothrow_construct =
                noexcept(type_traits::type_properties::is_nothrow_constructible_v<value_type, UKey &&, Args...>);
            return try_emplace_(
                [](value_type *placement, UKey &&keyval, Args &&...args) noexcept(is_nothrow_construct) {
                    utility::construct_at(placement, std::piecewise_construct, std::forward_as_tuple<UKey>(keyval),
                                          std::forward_as_tuple(utility::forward<Args>(args)...));
                },
                utility::forward<UKey&&>(keyval), utility::forward<Args>(args)...);
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<value_type,Args...>,int> = 0>
        iterator try_emplace(const_iterator hint, const key_type &keyval, Args &&...args) {
#if RAINY_ENABLE_DEBUG
            if (hint.empty() || (!hint.is_valid() && !hint.is_same_container(this))) {
                return end();
            }
#else
            (void) hint;
#endif
            return try_emplace(utility::forward<const key_type &>(keyval), utility::forward<Args>(args)...).first;
        }

        iterator try_emplace(const_iterator hint, node_type &&node_handle) {
#if RAINY_ENABLE_DEBUG
            if (hint.empty() || (!hint.is_valid() && !hint.is_same_container(this))) {
                return end();
            }
#else
            (void) hint;
#endif
            return try_emplace_([](value_type *placement,
                                   node_type &&node_handle) { utility::construct_at(placement, node_handle.move_this()); },
                                utility::move(node_handle))
                .first;
        }

        utility::pair<iterator, bool> insert(const value_type &pair) {
            size_type index = bucket(pair.first);
            if (index == npos) {
                return {end(), false};
            }
            iterator iter{this, index};
            if (!occupied[index]) {
                utility::construct_at(&data[index], utility::move(pair));
                occupied[index] = true;
                ++occupied_count;
                return {iter, true};
            }
            return {iter, false};
        }

        utility::pair<iterator, bool> insert(value_type &&pair) {
            std::size_t index = find_index(pair);
            if (index == npos) {
                return {end(), false};
            }
            iterator iter{this, index};
            if (!occupied[index]) {
                utility::construct_at(&data[index], utility::move(pair));
                occupied[index] = true;
                ++occupied_count;
                return {iter, true};
            }
            return {iter, false};
        }

        template <typename Pair, type_traits::other_trans::enable_if_t<
                                     type_traits::type_properties::is_constructible_v<value_type, Pair &&>, int> = 0>
        utility::pair<iterator, bool> insert(Pair &&value) {
            return this->emplace(utility::forward<Pair>(value));
        }

        insert_node_return_type<iterator, node_type> insert(node_type &&node_handle) noexcept {
            if (node_handle.empty()) {
                return {end(), false, utility::move(node_handle)};
            }
            if (std::size_t insert_bucket = bucket(node_handle.key()); insert_bucket != npos) {
                if (occupied[insert_bucket]) {
                    return {iterator{this, insert_bucket}, false, utility::move(node_handle)};
                }
                // 由于先前检查过node_handle是否为空，因此，此处不会导致异常
                utility::construct_at(&data[insert_bucket], utility::move(node_handle.move_this()));
                occupied[insert_bucket] = true;
                ++occupied_count;
                return {iterator{this, insert_bucket}, true, node_handle};
            }
            return {end(), false, utility::move(node_handle)};
        }

        iterator insert(const_iterator hint, node_type &&node_handle) noexcept {
#if RAINY_ENABLE_DEBUG
            if (hint.empty() || (!hint.is_valid() && !hint.is_same_container(this))) {
                return {end(), false, utility::move(node_handle)};
            }
#else
            (void) hint;
#endif
            return insert(node_handle).position;
        }

        RAINY_CONSTEXPR20 void clear() {
            occupied.fill(false);
            occupied_count = 0;
        }

        RAINY_CONSTEXPR20 size_type bucket(const Key &key) const {
            return find_index(key);
        }

        template <typename UKey>
            requires implements::umap_support_transparent_and_equal<key_equal, key_type, UKey>
        size_type bucket(UKey &&key) const {
            return find_index(key);
        }

        RAINY_CONSTEXPR20 std::size_t size() const noexcept {
            return occupied_count;
        }

        RAINY_CONSTEXPR20 std::size_t bucket_count() const noexcept {
            return N;
        }

        RAINY_CONSTEXPR20 std::size_t max_bucket_count() const noexcept {
            return N;
        }

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return size() == 0;
        }

        RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return N;
        }

        RAINY_CONSTEXPR20 float load_factor() const noexcept {
            return static_cast<float>(size()) / N;
        }

        template <typename Iter = iterator,
                  type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Iter, const_iterator>, int> = 0>
        RAINY_CONSTEXPR20 iterator erase(iterator pos) {
#if RAINY_ENABLE_DEBUG
            if (pos.empty() || !pos.is_same_container(pos.container) || pos == end()) {
                return end();
            }
#endif
            if (size_type wait_for_erase_bucket = bucket(pos->first); wait_for_erase_bucket != npos) {
                erase_bucket(wait_for_erase_bucket);
                ++pos;
                return pos;
            }
            return end();
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator pos) {
            return this->erase<iterator>(static_cast<iterator>(pos));
        }

        RAINY_CONSTEXPR20 node_type extract(const_iterator pos) {
#if RAINY_ENABLE_DEBUG
            if (pos.empty() || !pos.is_same_container(pos.container) || pos == end()) {
                return {};
            }
#endif
            return make_extract(bucket(pos->first));
        }

        RAINY_CONSTEXPR20 node_type extract(const key_type &keyval) {
            return make_extract(bucket(keyval));
        }

        template <typename UKey, type_traits::other_trans::enable_if_t<
                                     implements::umap_support_transparent_and_equal<key_equal, key_type, UKey> ,int> = 0>
        RAINY_CONSTEXPR20 node_type extract(UKey &&keyval) {
            return make_extract(bucket(keyval));
        }

    private:
        static constexpr inline std::size_t npos = static_cast<std::size_t>(-1);

        rainy::collections::array<utility::pair<const Key, Val>, N> data;
        rainy::collections::array<bool, N> occupied;
        std::size_t occupied_count;
        utility::compressed_pair<hasher, key_equal> utils;

        template <typename Fx, typename KeyType, typename... Args>
        utility::pair<iterator, bool> try_emplace_(Fx &&constrcut_fn, KeyType &&keyval, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<value_type, KeyType&&, Args...>) {
            if (std::size_t emplace_bucket = bucket(keyval); emplace_bucket != npos) {
                if (occupied[emplace_bucket]) {
                    return {make_iterator(emplace_bucket), false};
                }
                // Fx == void(value_type*, KeyType&& arg1, Args&&... args)
                constrcut_fn(&data[emplace_bucket], utility::forward<KeyType>(keyval), utility::forward<Args>(args)...);
                occupied[emplace_bucket] = true;
                ++occupied_count;
                return {make_iterator(emplace_bucket), true};
            }
            return {end(), false};
        }

        RAINY_CONSTEXPR20 void erase_bucket(size_type bucket) {
            if (bucket < N) {
                occupied[bucket] = false;
                std::destroy_at(&data[bucket]);
                --occupied_count;
            }
        }

        iterator make_iterator(size_type bucket) {
            if (bucket == npos) {
                return end();
            }
            return iterator{this, bucket};
        }

        template <typename UKey, type_traits::other_trans::enable_if_t<
                                     implements::umap_support_transparent_and_equal<key_equal, key_type, UKey>, int> = 0>
        RAINY_CONSTEXPR20 size_type find_index(UKey &&key) const {
            const auto &hasher = utils.get_first();
            const auto &key_equal = utils.get_second();
            std::size_t hash_value = hasher(key);
            std::size_t index = hash_value % N;
            std::size_t original_index = index;
            std::size_t probe = 0;
            do {
                if (!occupied[index] || key_equal(data[index].first, key)) {
                    return index;
                }
                probe++;
                index = (hash_value + probe) % N;
            } while (index != original_index);
            return npos;
        }

        RAINY_CONSTEXPR20 std::size_t find_index(const key_type &key) const {
            const auto &hasher = utils.get_first();
            const auto &key_equal = utils.get_second();

            std::size_t hash_value = hasher(key);
            std::size_t index = hash_value % N;
            std::size_t original_index = index;
            std::size_t probe = 0;
            do {
                if (!occupied[index] || key_equal(data[index].first, key)) {
                    return index;
                }
                probe++;
                index = (hash_value + probe) % N;
            } while (index != original_index);
            return npos;
        }

        RAINY_CONSTEXPR20 node_type make_extract(size_type bucket) {
            if (bucket != npos && bucket < N && occupied[bucket]) {
                node_type ret(utility::move(data.at(bucket)));
                occupied[bucket] = false;
                --occupied_count;
                return ret;
            }
            return {};
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 utility::pair<size_type, bool> emplace_(Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<value_type, Args...> &&
            type_traits::type_properties::is_nothrow_move_constructible_v<value_type>) {
            node_type new_insert_handle(utility::forward<Args>(args)...);
            const auto &keyval = new_insert_handle.key();
            size_type bucket = this->bucket(keyval);
            if (bucket < N) {
                if (occupied[bucket]) {
                    return {npos, false};
                }
                utility::construct_at(&data[bucket], new_insert_handle.move_this());
                occupied[bucket] = true;
                ++occupied_count;
                return {bucket, true};
            }
            return {npos, false};
        }
    };
}

#endif