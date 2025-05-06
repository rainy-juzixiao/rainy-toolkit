#include <bitset>

namespace rainy::hybrid_jenova::collections::implements {
    template <typename Key, typename... Args>
    struct in_place_key_extract_map_impl {
        // 默认情况下，无法在emplace家族中提取键，并且必须构造一个可能不会使用的节点
        static constexpr bool extractable = false;
    };

    template <typename Key, typename Value>
    struct in_place_key_extract_map_impl<Key, Key, Value> {
        // 如果调用pair(key, value)构造函数家族，可以使用第一个参数作为键
        static constexpr bool extractable = true;

        static const Key &extract(const Key &val, const Value &) noexcept {
            return val;
        }
    };

    template <typename Key, typename First, typename Second>
    struct in_place_key_extract_map_impl<Key, std::pair<First, Second>> {
        // 如果调用pair(pair<other, other>)构造函数家族，可以使用pair.first成员作为键
        static constexpr bool extractable = std::is_same_v<Key, std::remove_const_t<std::remove_reference_t<First>>>;

        static const Key &extract(const std::pair<First, Second> &val) noexcept {
            return val.first;
        }
    };

    template <typename Key, typename First, typename Second>
    struct in_place_key_extract_map_impl<Key, utility::pair<First, Second>> {
        // 如果调用pair(pair<other, other>)构造函数家族，可以使用pair.first成员作为键
        static constexpr bool extractable = std::is_same_v<Key, std::remove_const_t<std::remove_reference_t<First>>>;

        static const Key &extract(const utility::pair<First, Second> &val) noexcept {
            return val.first;
        }
    };

#if RAINY_HAS_CXX23
    template <typename Key, typename Elem>
    struct in_place_key_extract_map_impl<Key, std::array<Elem, 2>> {
        static constexpr bool extractable = std::is_same_v<Key, std::remove_const_t<Elem>>;

        static const Key &extract(const std::array<Elem, 2> &val) noexcept {
            return val[0];
        }
    };

    template <typename Key, typename First, typename Second>
    struct in_place_key_extract_map_impl<Key, std::tuple<First, Second>> {
        static constexpr bool extractable = std::is_same_v<Key, std::remove_const_t<std::remove_reference_t<First>>>;

        static const Key &extract(const std::tuple<First, Second> &val) noexcept {
            return std::get<0>(val);
        }
    };
#endif

    template <typename Key, typename First, typename... RestTypes>
    struct in_place_key_extract_map_impl<Key, std::piecewise_construct_t, std::tuple<First>, std::tuple<RestTypes...>> {
        // 如果调用piecewise_construct_t构造函数，并且第一个参数是一个1元元组，
        // 我们可以使用get<0>(first_tuple)作为键
        static constexpr bool extractable = std::is_same_v<Key, std::remove_const_t<std::remove_reference_t<First>>>;

        static const Key &extract(const std::piecewise_construct_t &, const std::tuple<First> &tup_val,
                                  const std::tuple<RestTypes...> &) noexcept {
            return std::get<0>(tup_val);
        }
    };

    template <typename... Valtys>
    using in_place_key_extract_map = in_place_key_extract_map_impl<std::remove_const_t<std::remove_reference_t<Valtys>>...>;
}

namespace rainy::hybrid_jenova::collections::implements {
    template <typename Hasher, typename Kty>
    constexpr bool nothrow_hash = noexcept(static_cast<size_t>(utility::declval<const Hasher &>()(utility::declval<const Kty &>())));

    template <typename Traits, std::size_t N>
    class static_hash_table;

    template <typename KeyType, typename Hasher, typename KeyEqual>
    class hybrid_umap_compare_util {
    public:
        using hasher = Hasher;
        using key_equal = KeyEqual;

        hybrid_umap_compare_util() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Hasher> &&
                                            type_traits::type_properties::is_nothrow_default_constructible_v<KeyEqual>) :
            pair({}, {}) {
        }

        explicit hybrid_umap_compare_util(const Hasher &hasher) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<Hasher> &&
            type_traits::type_properties::is_nothrow_default_constructible_v<KeyEqual>) : pair(hasher, {}) {
        }

        explicit hybrid_umap_compare_util(const Hasher &hasher, const KeyEqual &key_equal) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<Hasher> &&
            type_traits::type_properties::is_nothrow_copy_constructible_v<KeyEqual>) : pair(hasher, key_equal) {
        }

        template <typename Key>
        RAINY_NODISCARD size_t operator()(const Key &keyval) const noexcept(nothrow_hash<Hasher, Key>) {
            return static_cast<size_t>(pair.get_first()(keyval));
        }

        template <typename Key1, typename Key2>
        RAINY_NODISCARD bool operator()(const Key1 &keyval1, const Key2 &keyval2) const noexcept
        /*(implements::umap_equal_to_test<KeyEqual, Key1, Key2>)*/ {
            return !static_cast<bool>(pair.get_second()(keyval1, keyval2));
        }

        void swap(hybrid_umap_compare_util &right) noexcept(type_traits::type_properties::is_nothrow_swappable_v<Hasher> &&
                                                            type_traits::type_properties::is_nothrow_swappable_v<KeyEqual>) {
            pair.swap(right.pair);
        }

        utility::compressed_pair<Hasher, KeyEqual> pair;
    };

    template <typename Key, typename Mapped, typename UtilityType, bool Multiple>
    class static_umap_traits : public UtilityType {
    public:
        using key_type = Key;
        using mapped_type = Mapped;
        using value_type = utility::pair<const Key, Mapped>;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using mutable_value_type = utility::pair<Key, Mapped>;
        using key_compare = UtilityType;
        using node_type = implements::umap_node_handle<key_type, mapped_type>;
        using hasher = typename UtilityType::hasher;
        using key_equal = typename UtilityType::key_equal;
        using size_type = std::size_t;

        template <typename... Args>
        using in_place_key_extract_t = in_place_key_extract_map<key_type,Args...>;

        static constexpr bool multi = Multiple;

        static_umap_traits() = default;

        explicit static_umap_traits(const UtilityType &utility) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<UtilityType>) : UtilityType(utility) {
        }

        template <typename Ty1, typename Ty2>
        static const Ty1 &get_key(const utility::pair<Ty1, Ty2> &pair) noexcept {
            return pair.first;
        }
    };
}

namespace rainy::hybrid_jenova::collections::implements {
    template <typename Container, typename Traits, std::size_t N>
    class hash_table_iterator : public utility::bidirectional_iterator<hash_table_iterator<Container, Traits, N>, Traits> {
    public:
        using container_type = Container;
        using value_type = typename container_type::value_type;
        using pointer = typename container_type::pointer;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;
        using const_pointer = typename container_type::const_pointer;
        using difference_type = typename container_type::difference_type;
        using iterator_category = std::bidirectional_iterator_tag;
        using base = utility::bidirectional_iterator<hash_table_iterator, Traits>;

        RAINY_CONSTEXPR20 hash_table_iterator(const container_type *container, std::size_t idx) :
            container(container), current_index(idx) {
            find_next_occupied_forward();
        }

        RAINY_CONSTEXPR20 hash_table_iterator(const hash_table_iterator &) noexcept = default;
        RAINY_CONSTEXPR20 hash_table_iterator(hash_table_iterator &&) noexcept = default;

    private:
        static constexpr inline std::size_t npos = static_cast<std::size_t>(-1);

        friend class utility::bidirectional_iterator<hash_table_iterator, implements::umap_iterator_traits<value_type>>;

        template <typename, std::size_t>
        friend class static_hash_table;

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
            return container->table[current_index];
        }

        RAINY_CONSTEXPR20 typename base::const_pointer get_pointer_impl() const noexcept {
            return utility::addressof(container->table[current_index]);
        }

        RAINY_CONSTEXPR20 typename base::reference get_element_impl() noexcept {
            return const_cast<reference>(container->table[current_index]);
        }

        RAINY_CONSTEXPR20 typename base::pointer get_pointer_impl() noexcept {
            return const_cast<pointer>(utility::addressof(container->table[current_index]));
        }

        RAINY_CONSTEXPR20 void next_impl() noexcept {
            ++current_index;
            find_next_occupied_forward();
        }

        RAINY_CONSTEXPR20 void back_impl() noexcept {
            --current_index;
            find_next_occupied_backward();
        }

        RAINY_CONSTEXPR20 bool equal_with_impl(const hash_table_iterator &right) const noexcept {
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
}

namespace rainy::hybrid_jenova::collections::implements {
    template <typename Traits, std::size_t N>
    class static_hash_table {
    public:
        using size_type = typename Traits::size_type;
        using key_type = typename Traits::key_type;
        using value_type = typename Traits::value_type;
        using difference_type = std::ptrdiff_t;
        using reference = value_type &;
        using const_reference = const value_type &;
        using hasher = typename Traits::hasher;
        using key_equal = typename Traits::key_equal;
        using pointer = typename Traits::pointer;
        using const_pointer = typename Traits::const_pointer;
        using node_type = typename Traits::node_type;

        using iterator = type_traits::other_trans::conditional_t<
            type_traits::type_relations::is_same_v<key_type, value_type>,
            const implements::hash_table_iterator<static_hash_table, implements::umap_iterator_traits<value_type>, N>,
            implements::hash_table_iterator<static_hash_table, implements::umap_iterator_traits<value_type>, N>>;
        using const_iterator =
            const implements::hash_table_iterator<static_hash_table, implements::umap_iterator_traits<value_type>, N>;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = const utility::reverse_iterator<iterator>;

        using insertion_return_type = type_traits::other_trans::conditional_t<Traits::multi, iterator, utility::pair<iterator, bool>>;

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

        insertion_return_type insert(const value_type &val) {
            return emplace(val);
        }

        insertion_return_type insert(value_type &&val) {
            return emplace(utility::move(val));
        }

        template <typename PairLike, type_traits::other_trans::enable_if_t<
                                     type_traits::type_properties::is_constructible_v<value_type, PairLike &&>, int> = 0>
        insertion_return_type insert(PairLike &&val) {
            return this->emplace(utility::move(val));
        }



        iterator insert(const_iterator hint, const value_type& val) {
            return emplace_hint(hint, val);
        }

        iterator insert(const_iterator hint, value_type &&val) {
            return emplace_hint(hint, utility::move(val));
        }

        template <typename... Args>
        insertion_return_type emplace(Args &&...args) {
            auto pair = emplace_(utility::forward<Args>(args)...);
            if (pair.first == npos) {
                return make_invalid_insertion();
            }
            return make_insertion(pair.first);
        }

        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
#if RAINY_ENABLE_DEBUG
            if (hint.empty() || (!hint.is_valid() && !hint.is_same_container(this))) {
                return make_invalid_iter();
            }
#else
            (void) hint;
#endif
            auto pair = this->emplace_(utility::forward<Args>(args)...);
            if (pair.first == npos) {
                return make_invalid_iter();
            }
            return make_iter(pair.first);
        }

        template <typename UKey, type_traits::other_trans::enable_if_t<
                                     implements::umap_support_transparent_and_equal<key_equal, key_type, UKey>, int> = 0>
        RAINY_CONSTEXPR20 size_type bucket(UKey &&key) const {
            std::size_t hash_value = this->traits_object(key);
            std::size_t index = hash_value % N;
            std::size_t original_index = index;
            std::size_t probe = 0;
            do {
                if (!occupied[index] || this->traits_object(table[index].first, key)) {
                    return index;
                }
                probe++;
                index = (hash_value + probe) % N;
            } while (index != original_index);
            return npos;
        }

        RAINY_CONSTEXPR20 std::size_t bucket(const key_type &key) const {
            std::size_t hash_value = this->traits_object(key);
            std::size_t index = hash_value % N;
            std::size_t original_index = index;
            std::size_t probe = 0;
            do {
                if (!occupied[index] || this->traits_object(table[index].first, key)) {
                    return index;
                }
                probe++;
                index = (hash_value + probe) % N;
            } while (index != original_index);
            return npos;
        }

    private:
        friend class implements::hash_table_iterator<static_hash_table, implements::umap_iterator_traits<value_type>, N>;

        template <typename... Args>
        RAINY_CONSTEXPR20 utility::pair<size_type, bool> emplace_(Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<value_type, Args...> &&
            type_traits::type_properties::is_nothrow_move_constructible_v<value_type>) {
            using in_place_key_extract = Traits::template in_place_key_extract_t<Args...>;
            if constexpr (!Traits::multi && in_place_key_extract::extractable) {
                const auto& key = in_place_key_extract::extract(args...);
                return insert_new_node(this->bucket(key), utility::forward<Args>(args)...);
            } else {
                // 无论如何，对于多键容器还是没法解压的，都需要造一个对象
                value_type construct_object(utility::forward<Args>(args)...);
                const auto &key = Traits::get_key(construct_object);
                return insert_new_node(this->bucket(key), utility::move(construct_object)); // 我们会在结束的时候，移动它
            }
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 utility::pair<size_type, bool> insert_new_node(size_type bucket, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_move_constructible_v<value_type>) {
            if (bucket < N) {
                if (occupied[bucket]) {
                    return {npos, false};
                }
                utility::construct_at(&table[bucket], utility::forward<Args>(args)...);
                occupied[bucket] = true;
                ++occupied_count;
                return {bucket, true};
            }
            return {npos, false};
        }

        RAINY_CONSTEXPR20 iterator make_invalid_iter() const noexcept {
            return end();
        }

        RAINY_CONSTEXPR20 insertion_return_type make_invalid_insertion() const noexcept {
            if constexpr (Traits::multi) {
                return end();
            } else {
                return {end(), false};
            }
        }

        RAINY_CONSTEXPR20 iterator make_iter(size_type bucket) const noexcept {
            return iterator{this, bucket};
        }

        RAINY_CONSTEXPR20 insertion_return_type make_insertion(size_type bucket) const noexcept {
            if constexpr (Traits::multi) {
                return iterator{this, bucket};
            } else {
                return {iterator{this, bucket}, false};
            }
        }

        RAINY_CONSTEXPR20 node_type make_extract(size_type bucket) {
            if (bucket != npos && bucket < N && occupied[bucket]) {
                node_type ret(utility::move(table.at(bucket)));
                occupied[bucket] = false;
                --occupied_count;
                return ret;
            }
            return {};
        }

        Traits traits_object;
        std::bitset<N> occupied;
        std::size_t occupied_count{0};
        rainy::collections::array<value_type, N> table{};
    };
}
