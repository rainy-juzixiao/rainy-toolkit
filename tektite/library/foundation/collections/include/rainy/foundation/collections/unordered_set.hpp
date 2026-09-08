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
#ifndef RAINY_FOUNDATION_COLLECTIONS_UNORDERED_SET_HPP
#define RAINY_FOUNDATION_COLLECTIONS_UNORDERED_SET_HPP
#include <cmath>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/utility/hash.hpp>
#include <rainy/core/utility/iterator.hpp>
#include <rainy/foundation/collections/list.hpp>

namespace rainy::foundation::collections::implements {
    template <typename Iter>
    class unordered_set_iterator final {
    public:
        template <typename>
        friend class unordered_set_iterator;

        using value_type = decltype(*std::declval<Iter>());
        using pointer = utility::input_iterator_pointer<value_type>;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;

        constexpr unordered_set_iterator() noexcept : it{} {
        }

        constexpr unordered_set_iterator(const Iter iter) noexcept : it{iter} {
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Iter, Uty> &&
                                                                          type_traits::properties::is_constructible_v<Iter, Uty>,
                                                                      int> = 0>
        constexpr unordered_set_iterator(const unordered_set_iterator<Uty> &other) noexcept : it{other.it} {
        }

        constexpr unordered_set_iterator &operator++() noexcept {
            return ++it, *this;
        }

        constexpr unordered_set_iterator operator++(int) noexcept {
            const unordered_set_iterator orig = *this;
            return ++(*this), orig;
        }

        constexpr unordered_set_iterator &operator--() noexcept {
            return --it, *this;
        }

        constexpr unordered_set_iterator operator--(int) noexcept {
            const unordered_set_iterator orig = *this;
            return operator--(), orig;
        }

        constexpr unordered_set_iterator &operator+=(const difference_type value) noexcept {
            it += value;
            return *this;
        }

        constexpr unordered_set_iterator operator+(const difference_type value) const noexcept {
            unordered_set_iterator tmp = *this;
            return (tmp += value);
        }

        constexpr unordered_set_iterator &operator-=(const difference_type value) noexcept {
            return (*this += -value);
        }

        constexpr unordered_set_iterator operator-(const difference_type value) const noexcept {
            return (*this + -value);
        }

        RAINY_NODISCARD constexpr pointer operator->() const noexcept {
            return operator*();
        }

        RAINY_NODISCARD constexpr reference operator*() const noexcept {
            return *it;
        }

        template <typename L, typename R>
        friend bool operator==(const unordered_set_iterator<L> &, const unordered_set_iterator<R> &) noexcept;

        template <typename L, typename R>
        friend bool operator!=(const unordered_set_iterator<L> &, const unordered_set_iterator<R> &) noexcept;

        Iter base() {
            return it;
        }

    private:
        Iter it;
    };

    template <typename L, typename R>
    inline bool operator==(const unordered_set_iterator<L> &left, const unordered_set_iterator<R> &right) noexcept {
        return left.it == right.it;
    }

    template <typename L, typename R>
    inline bool operator!=(const unordered_set_iterator<L> &left, const unordered_set_iterator<R> &right) noexcept {
        return left.it != right.it;
    }

    template <bool IsConst, typename ListIterator>
    class unordered_set_local_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename ListIterator::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::conditional<IsConst, typename value_type::const_pointer, typename value_type::pointer>::type;
        using reference = typename std::conditional<IsConst, const value_type &, value_type &>::type;

    private:
        ListIterator current_;
        ListIterator end_;

    public:
        unordered_set_local_iterator() = default;
        unordered_set_local_iterator(ListIterator current, ListIterator end) : current_(current), end_(end) {
        }

        reference operator*() const {
            return *current_;
        }
        pointer operator->() const {
            return &(*current_);
        }

        unordered_set_local_iterator &operator++() {
            if (current_ != end_) {
                ++current_;
            }
            return *this;
        }

        unordered_set_local_iterator operator++(int) {
            unordered_set_local_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const unordered_set_local_iterator &other) const {
            return current_ == other.current_;
        }

        bool operator!=(const unordered_set_local_iterator &other) const {
            return current_ != other.current_;
        }

        template <bool B = IsConst, typename = type_traits::other_trans::enable_if_t<B>>
        unordered_set_local_iterator(const unordered_set_local_iterator<false, ListIterator> &other) :
            current_(other.current_), end_(other.end_) { // NOLINT
        }
    };
}

namespace rainy::foundation::collections::implements {
    template <typename Key, typename Alloc>
    class unordered_set_node_handle {
    public:
        using key_type = Key;
        using value_type = Key;
        using list_type = list<value_type, typename core::memory::allocator_traits<Alloc>::template rebind_alloc<value_type>>;
        using list_node_type = list_node<value_type, typename list_type::node_allocator_type>;

        template <bool, typename, typename, typename, typename>
        friend class unordered_set;

        constexpr unordered_set_node_handle() noexcept : node_() {
        }

        unordered_set_node_handle(unordered_set_node_handle &&other) noexcept : node_(utility::move(other.node_)) {
        }

        unordered_set_node_handle &operator=(unordered_set_node_handle &&other) noexcept {
            if (this != &other) {
                node_ = utility::move(other.node_);
            }
            return *this;
        }

        unordered_set_node_handle(const unordered_set_node_handle &) = delete;
        unordered_set_node_handle &operator=(const unordered_set_node_handle &) = delete;

        ~unordered_set_node_handle() = default;

        bool empty() const noexcept {
            return node_.empty();
        }

        explicit operator bool() const noexcept {
            return !empty();
        }

        key_type &key() const {
            assert(!empty());
            return const_cast<key_type &>(node_.value());
        }

        void swap(unordered_set_node_handle &other) noexcept {
            using std::swap;
            swap(node_, other.node_);
        }

    private:
        explicit unordered_set_node_handle(list_node_type &&node) : node_(utility::move(node)) {
        }

        list_node_type node_;
    };

    template <bool Multi, typename Key, typename Hash, typename KeyEqual, typename Allocator>
    class unordered_set {
    public:
        using key_type = Key;
        using value_type = Key;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher = Hash;
        using key_equal = KeyEqual;
        using allocator_type = Allocator;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

        using list_type = list<value_type, typename std::allocator_traits<Allocator>::template rebind_alloc<value_type>>;
        using list_iterator = typename list_type::iterator;
        using const_list_iterator = typename list_type::const_iterator;

        using bucket_type = std::vector<list_iterator>;
        using bucket_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<bucket_type>;

        using iterator = unordered_set_iterator<list_iterator>;
        using const_iterator = unordered_set_iterator<const_list_iterator>;
        using local_iterator = unordered_set_local_iterator<false, list_iterator>;
        using const_local_iterator = unordered_set_local_iterator<true, const_list_iterator>;

        using node_type = unordered_set_node_handle<Key, Allocator>;
        using insert_return_type = struct {
            iterator position;
            bool inserted;
            node_type node;
        };

        unordered_set() noexcept : elements_(), buckets_(), size_(0), max_load_factor_(default_max_load_factor_), hash_(), equal_() {
            init_buckets(default_bucket_count_);
        }

        explicit unordered_set(const size_type bucket_count, const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                               const allocator_type &alloc = allocator_type()) :
            elements_(alloc), buckets_(), size_(0), max_load_factor_(default_max_load_factor_), hash_(hash), equal_(equal) {
            init_buckets(bucket_count);
        }

        unordered_set(const unordered_set &other) :
            elements_(other.elements_), buckets_(), size_(0), max_load_factor_(other.max_load_factor_), hash_(other.hash_),
            equal_(other.equal_) {
            init_buckets(other.buckets_.size());
            for (auto it = elements_.begin(); it != elements_.end(); ++it) {
                size_type bucket_idx = get_bucket_index(*it);
                buckets_[bucket_idx].push_back(it);
                ++size_;
            }
        }

        unordered_set(unordered_set &&other) noexcept :
            elements_(utility::move(other.elements_)), buckets_(utility::move(other.buckets_)), size_(other.size_),
            max_load_factor_(other.max_load_factor_), hash_(utility::move(other.hash_)), equal_(utility::move(other.equal_)) {
            other.size_ = 0;
        }

        unordered_set(std::initializer_list<value_type> init, size_type bucket_count = default_bucket_count_,
                      const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                      const allocator_type &alloc = allocator_type()) :
            elements_(alloc), buckets_(), size_(0), max_load_factor_(default_max_load_factor_), hash_(hash), equal_(equal) {
            size_type actual_count = bucket_count > init.size() ? bucket_count : init.size();
            init_buckets(actual_count);
            for (const auto &item : init) {
                insert(item);
            }
        }

        ~unordered_set() = default;

        unordered_set &operator=(const unordered_set &other) {
            if (this != &other) {
                unordered_set tmp(other);
                swap(tmp);
            }
            return *this;
        }

        unordered_set &operator=(unordered_set &&other) noexcept {
            if (this != &other) {
                elements_ = utility::move(other.elements_);
                buckets_ = utility::move(other.buckets_);
                size_ = other.size_;
                max_load_factor_ = other.max_load_factor_;
                hash_ = utility::move(other.hash_);
                equal_ = utility::move(other.equal_);
                other.size_ = 0;
            }
            return *this;
        }

        unordered_set &operator=(std::initializer_list<value_type> init) {
            clear();
            reserve(init.size());
            for (const auto &item : init) {
                insert(item);
            }
            return *this;
        }

        allocator_type get_allocator() const noexcept {
            return elements_.get_allocator();
        }

        iterator begin() noexcept {
            return iterator(elements_.begin());
        }

        const_iterator begin() const noexcept {
            return const_iterator(elements_.begin());
        }

        const_iterator cbegin() const noexcept {
            return const_iterator(elements_.begin());
        }

        iterator end() noexcept {
            return iterator(elements_.end());
        }

        const_iterator end() const noexcept {
            return const_iterator(elements_.end());
        }

        const_iterator cend() const noexcept {
            return const_iterator(elements_.end());
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return size_ == 0;
        }

        RAINY_NODISCARD size_type size() const noexcept {
            return size_;
        }

        RAINY_NODISCARD size_type max_size() const noexcept {
            return elements_.max_size();
        }

        void clear() noexcept {
            elements_.clear();
            for (auto &bucket : buckets_) {
                bucket.clear();
            }
            size_ = 0;
        }

        container::pair<iterator, bool> insert(const value_type &value) {
            if constexpr (Multi) {
                return container::make_pair(insert_multi(value), true);
            } else {
                return insert_unique(value);
            }
        }

        container::pair<iterator, bool> insert(value_type &&value) {
            if constexpr (Multi) {
                return container::make_pair(insert_multi(utility::move(value)), true);
            } else {
                return insert_unique(utility::move(value));
            }
        }

        template <typename P, typename = type_traits::other_trans::enable_if_t<std::is_constructible_v<value_type, P &&>>>
        container::pair<iterator, bool> insert(P &&value) {
            if constexpr (Multi) {
                return container::make_pair(insert_multi(utility::forward<P>(value)), true);
            } else {
                return insert_unique(utility::forward<P>(value));
            }
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last) {
            for (; first != last; ++first) {
                insert(*first);
            }
        }

        void insert(std::initializer_list<value_type> init) {
            insert(init.begin(), init.end());
        }

        template <typename... Args>
        container::pair<iterator, bool> emplace(Args &&...args) {
            if constexpr (Multi) {
                return container::make_pair(insert_multi(utility::forward<Args>(args)...), true);
            } else {
                return insert_unique(utility::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
            (void)hint;
            return emplace(utility::forward<Args>(args)...).first;
        }

        iterator erase(const_iterator pos) {
            return erase_impl(pos);
        }

        iterator erase(const_iterator first, const_iterator last) {
            return erase_impl(first, last);
        }

        size_type erase(const key_type &key) {
            return erase_impl(key);
        }

        void swap(unordered_set &other) noexcept {
            using std::swap;
            swap(elements_, other.elements_);
            swap(buckets_, other.buckets_);
            swap(size_, other.size_);
            swap(max_load_factor_, other.max_load_factor_);
            swap(hash_, other.hash_);
            swap(equal_, other.equal_);
        }

        size_type count(const key_type &key) const {
            if constexpr (Multi) {
                if (buckets_.empty()) {
                    return 0;
                }
                size_type bucket_idx = get_bucket_index(key);
                const auto &bucket = buckets_[bucket_idx];
                size_type cnt = 0;
                for (auto it : bucket) {
                    if (equal_(*it, key)) {
                        ++cnt;
                    }
                }
                return cnt;
            } else {
                return find(key) != end() ? 1 : 0;
            }
        }

        template <typename K>
        auto count(const K &key) const -> decltype(equal_(std::declval<const key_type &>(), key), size_type()) {
            if constexpr (Multi) {
                if (buckets_.empty()) {
                    return 0;
                }
                size_type bucket_idx = get_bucket_index(key);
                const auto &bucket = buckets_[bucket_idx];
                size_type cnt = 0;
                for (auto it : bucket) {
                    if (equal_(*it, key)) {
                        ++cnt;
                    }
                }
                return cnt;
            } else {
                return find(key) != end() ? 1 : 0;
            }
        }

        iterator find(const key_type &key) {
            if (buckets_.empty()) {
                return end();
            }
            size_type bucket_idx = get_bucket_index(key);
            auto it = find_in_bucket(bucket_idx, key);
            return iterator(it);
        }

        const_iterator find(const key_type &key) const {
            if (buckets_.empty()) {
                return end();
            }
            size_type bucket_idx = get_bucket_index(key);
            auto it = find_in_bucket(bucket_idx, key);
            return const_iterator(it);
        }

        template <typename K>
        auto find(const K &key) -> decltype(equal_(std::declval<const key_type &>(), key), iterator()) {
            if (buckets_.empty()) {
                return end();
            }
            size_type bucket_idx = get_bucket_index(key);
            auto it = find_in_bucket(bucket_idx, key);
            return iterator(it);
        }

        template <typename K>
        auto find(const K &key) const -> decltype(equal_(utility::declval<const key_type &>(), key), const_iterator()) {
            if (buckets_.empty()) {
                return end();
            }
            size_type bucket_idx = get_bucket_index(key);
            auto it = find_in_bucket(bucket_idx, key);
            return const_iterator(it);
        }

        bool contains(const key_type &key) const {
            return find(key) != end();
        }

        template <typename K>
        auto contains(const K &key) const -> decltype(equal_(std::declval<const key_type &>(), key), bool()) {
            return find(key) != end();
        }

        container::pair<iterator, iterator> equal_range(const key_type &key) {
            if constexpr (Multi) {
                if (buckets_.empty()) {
                    return {end(), end()};
                }
                size_type bucket_idx = get_bucket_index(key);
                const auto &bucket = buckets_[bucket_idx];
                iterator first = end();
                iterator last = end();
                for (auto it : bucket) {
                    if (equal_(*it, key)) {
                        if (first == end()) {
                            first = iterator(it);
                        }
                        last = iterator(it);
                        ++last;
                    }
                }
                return {first, last};
            } else {
                auto it = find(key);
                if (it == end()) {
                    return {it, it};
                }
                auto next = it;
                ++next;
                return {it, next};
            }
        }

        container::pair<const_iterator, const_iterator> equal_range(const key_type &key) const {
            if constexpr (Multi) {
                if (buckets_.empty()) {
                    return {end(), end()};
                }
                size_type bucket_idx = get_bucket_index(key);
                const auto &bucket = buckets_[bucket_idx];
                const_iterator first = end();
                const_iterator last = end();
                for (auto it : bucket) {
                    if (equal_(*it, key)) {
                        if (first == end()) {
                            first = const_iterator(it);
                        }
                        last = const_iterator(it);
                        ++last;
                    }
                }
                return {first, last};
            } else {
                auto it = find(key);
                if (it == end()) {
                    return {it, it};
                }
                auto next = it;
                ++next;
                return {it, next};
            }
        }

        local_iterator begin(size_type n) {
            assert(n < bucket_count());
            if (buckets_[n].empty()) {
                return local_iterator(elements_.end(), elements_.end());
            }
            return local_iterator(buckets_[n].front(), elements_.end());
        }

        const_local_iterator begin(size_type n) const {
            assert(n < bucket_count());
            if (buckets_[n].empty()) {
                return const_local_iterator(elements_.end(), elements_.end());
            }
            return const_local_iterator(buckets_[n].front(), elements_.end());
        }

        const_local_iterator cbegin(size_type n) const {
            return begin(n);
        }

        local_iterator end(size_type n) {
            assert(n < bucket_count());
            return local_iterator(elements_.end(), elements_.end());
        }

        const_local_iterator end(size_type n) const {
            assert(n < bucket_count());
            return const_local_iterator(elements_.end(), elements_.end());
        }

        const_local_iterator cend(size_type n) const {
            return end(n);
        }

        size_type bucket_count() const noexcept {
            return buckets_.size();
        }

        size_type max_bucket_count() const noexcept {
            return buckets_.max_size();
        }

        size_type bucket_size(size_type n) const {
            assert(n < bucket_count());
            return buckets_[n].size();
        }

        size_type bucket(const key_type &key) const {
            return get_bucket_index(key);
        }

        float load_factor() const noexcept {
            return buckets_.empty() ? 0.0f : static_cast<float>(size_) / static_cast<float>(buckets_.size());
        }

        float max_load_factor() const noexcept {
            return max_load_factor_;
        }

        void max_load_factor(float ml) {
            assert(ml > 0.0f);
            max_load_factor_ = ml;
        }

        void rehash(size_type count) {
            size_type new_bucket_count = count > 0 ? next_power_of_two(count) : default_bucket_count_;
            size_type min_buckets = static_cast<size_type>(std::ceil(static_cast<float>(size_) / max_load_factor_));
            if (new_bucket_count < min_buckets) {
                new_bucket_count = next_power_of_two(min_buckets);
            }
            if (new_bucket_count == buckets_.size()) {
                return;
            }
            std::vector<bucket_type> new_buckets(new_bucket_count);
            for (auto it = elements_.begin(); it != elements_.end(); ++it) {
                size_type bucket_idx = hash_(*it) % new_bucket_count;
                new_buckets[bucket_idx].push_back(it);
            }
            buckets_ = utility::move(new_buckets);
        }

        void reserve(size_type count) {
            rehash(static_cast<size_type>(std::ceil(static_cast<float>(count) / max_load_factor_)));
        }

        hasher hash_function() const {
            return hash_;
        }

        key_equal key_eq() const {
            return equal_;
        }

        void merge(unordered_set &right) {
            for (auto item : right) {
                insert(item);
            }
            right.clear();
        }

        void merge(unordered_set &&right) {
            for (value_type &&item : right) {
                insert(utility::move(item));
            }
            right.clear();
        }

        node_type extract(const_iterator position) {
            if (position == end()) {
                return node_type{};
            }
            auto list_it = position.base();
            const key_type &key = *list_it;
            size_type bucket_idx = get_bucket_index(key);
            auto &bucket = buckets_[bucket_idx];
            for (auto it = bucket.begin(); it != bucket.end(); ++it) {
                if (*it == list_it) {
                    bucket.erase(it);
                    break;
                }
            }
            auto extracted_node = elements_.extract_node(list_it);
            --size_;
            return node_type(utility::move(extracted_node));
        }

        node_type extract(const key_type &key) {
            auto it = find(key);
            if (it == end()) {
                return node_type{};
            }
            return extract(const_iterator(it));
        }

        insert_return_type insert(node_type &&nh) {
            if (nh.empty()) {
                return {end(), false, node_type{}};
            }
            if (static_cast<float>(size_ + 1) > static_cast<float>(buckets_.size()) * max_load_factor_) {
                rehash(buckets_.size() * 2);
            }
            const key_type &key = nh.node_.value();
            if constexpr (!Multi) {
                size_type bucket_idx = get_bucket_index(key);
                auto existing = find_in_bucket(bucket_idx, key);
                if (existing != elements_.end()) {
                    return {iterator(existing), false, utility::move(nh)};
                }
            }
            auto new_iter = elements_.insert(elements_.end(), utility::move(nh.node_));
            size_type bucket_idx = get_bucket_index(key);
            buckets_[bucket_idx].push_back(new_iter);
            ++size_;
            return {iterator(new_iter), true, node_type{}};
        }

        iterator insert(const_iterator hint, node_type &&nh) {
            (void)hint;
            return insert(utility::move(nh)).position;
        }

    private:
        static constexpr size_type default_bucket_count_ = 8;
        static constexpr float default_max_load_factor_ = 1.0f;

        size_type get_bucket_index(const key_type &key) const {
            return hash_(key) % buckets_.size();
        }

        template <typename K>
        size_type get_bucket_index(const K &key) const {
            return hash_(key) % buckets_.size();
        }

        size_type next_power_of_two(size_type n) const {
            if (n == 0) {
                return 1;
            }
            --n;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            if constexpr (sizeof(size_type) > 4) {
                n |= n >> 32;
            }
            return n + 1;
        }

        void init_buckets(size_type count) {
            size_type actual_count = count > 0 ? next_power_of_two(count) : default_bucket_count_;
            buckets_.resize(actual_count);
        }

        template <typename K>
        list_iterator find_in_bucket(size_type bucket_idx, const K &key) {
            for (auto it : buckets_[bucket_idx]) {
                if (equal_(*it, key)) {
                    return it;
                }
            }
            return elements_.end();
        }

        template <typename K>
        const_list_iterator find_in_bucket(size_type bucket_idx, const K &key) const {
            for (auto it : buckets_[bucket_idx]) {
                if (equal_(*it, key)) {
                    return it;
                }
            }
            return elements_.end();
        }

        template <typename... Args>
        container::pair<iterator, bool> insert_unique(Args &&...args) {
            if (static_cast<float>(size_ + 1) > static_cast<float>(buckets_.size()) * max_load_factor_) {
                rehash(buckets_.size() * 2);
            }
            elements_.emplace_back(utility::forward<Args>(args)...);
            auto new_iter = std::prev(elements_.end());
            const key_type &key = *new_iter;
            size_type bucket_idx = get_bucket_index(key);
            auto existing = find_in_bucket(bucket_idx, key);
            if (existing != elements_.end()) {
                elements_.erase(new_iter);
                return container::make_pair(iterator(existing), false);
            }
            buckets_[bucket_idx].emplace_back(new_iter);
            ++size_;
            return container::make_pair(iterator{new_iter}, true);
        }

        template <typename... Args>
        iterator insert_multi(Args &&...args) {
            if (static_cast<float>(size_ + 1) > static_cast<float>(buckets_.size()) * max_load_factor_) {
                rehash(buckets_.size() * 2);
            }
            elements_.emplace_back(utility::forward<Args>(args)...);
            auto new_iter = std::prev(elements_.end());
            const key_type &key = *new_iter;
            size_type bucket_idx = get_bucket_index(key);
            buckets_[bucket_idx].push_back(new_iter);
            ++size_;
            return iterator(new_iter);
        }

        size_type erase_impl(const key_type &key) {
            if (buckets_.empty()) {
                return 0;
            }
            size_type bucket_idx = get_bucket_index(key);
            auto &bucket = buckets_[bucket_idx];
            size_type count = 0;
            if constexpr (Multi) {
                auto it = bucket.begin();
                while (it != bucket.end()) {
                    if (equal_(**it, key)) {
                        elements_.erase(*it);
                        it = bucket.erase(it);
                        ++count;
                        --size_;
                    } else {
                        ++it;
                    }
                }
            } else {
                for (auto it = bucket.begin(); it != bucket.end(); ++it) {
                    if (equal_(**it, key)) {
                        elements_.erase(*it);
                        bucket.erase(it);
                        --size_;
                        return 1;
                    }
                }
            }
            return count;
        }

        iterator erase_impl(const_iterator pos) {
            if (pos == end()) {
                return end();
            }
            auto list_it = pos.base();
            const key_type &key = *list_it;
            size_type bucket_idx = get_bucket_index(key);
            auto &bucket = buckets_[bucket_idx];
            for (auto it = bucket.begin(); it != bucket.end(); ++it) {
                if (*it == list_it) {
                    bucket.erase(it);
                    break;
                }
            }
            auto next = elements_.erase(list_it);
            --size_;
            return iterator(next);
        }

        iterator erase_impl(const_iterator first, const_iterator last) {
            while (first != last) {
                first = erase_impl(first);
            }
            return iterator(last.base());
        }

        list_type elements_;
        std::vector<bucket_type> buckets_;
        size_type size_;
        float max_load_factor_;
        hasher hash_;
        key_equal equal_;
    };
}

namespace rainy::foundation::collections {
    template <typename Key, typename Hash = utility::hash<Key>, typename KeyEqual = functional::equal<Key>,
              typename Allocator = core::memory::allocator<Key>>
    class unordered_set : public implements::unordered_set<false, Key, Hash, KeyEqual, Allocator> {
    public:
        using base = implements::unordered_set<false, Key, Hash, KeyEqual, Allocator>;
        using key_type = typename base::key_type;
        using value_type = typename base::value_type;
        using size_type = typename base::size_type;
        using difference_type = typename base::difference_type;
        using hasher = typename base::hasher;
        using key_equal = typename base::key_equal;
        using allocator_type = typename base::allocator_type;
        using reference = typename base::reference;
        using const_reference = typename base::const_reference;
        using pointer = typename base::pointer;
        using const_pointer = typename base::const_pointer;
        using iterator = typename base::iterator;
        using const_iterator = typename base::const_iterator;
        using local_iterator = typename base::local_iterator;
        using const_local_iterator = typename base::const_local_iterator;
        using base::base;
    };
}

namespace rainy::foundation::collections {
    template <typename Key, typename Hash = utility::hash<Key>, typename KeyEqual = functional::equal<Key>,
              typename Allocator = core::memory::allocator<Key>>
    class unordered_multiset : public implements::unordered_set<true, Key, Hash, KeyEqual, Allocator> {
    private:
        using base = implements::unordered_set<true, Key, Hash, KeyEqual, Allocator>;
    public:
        using key_type = typename base::key_type;
        using value_type = typename base::value_type;
        using size_type = typename base::size_type;
        using difference_type = typename base::difference_type;
        using hasher = typename base::hasher;
        using key_equal = typename base::key_equal;
        using allocator_type = typename base::allocator_type;
        using reference = typename base::reference;
        using const_reference = typename base::const_reference;
        using pointer = typename base::pointer;
        using const_pointer = typename base::const_pointer;
        using iterator = typename base::iterator;
        using const_iterator = typename base::const_iterator;
        using local_iterator = typename base::local_iterator;
        using const_local_iterator = typename base::const_local_iterator;
        using base::base;
    };
}

namespace rainy::foundation::collections {
    template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
    bool operator==(const unordered_set<Key, Hash, KeyEqual, Alloc> &left,
                    const unordered_set<Key, Hash, KeyEqual, Alloc> &right) {
        if (left.size() != right.size()) {
            return false;
        }
        for (const auto &elem : left) {
            if (right.count(elem) == 0) {
                return false;
            }
        }
        return true;
    }

    template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
    bool operator!=(const unordered_set<Key, Hash, KeyEqual, Alloc> &left,
                    const unordered_set<Key, Hash, KeyEqual, Alloc> &right) {
        return !(left == right);
    }

    template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
    void swap(unordered_set<Key, Hash, KeyEqual, Alloc> &left,
              unordered_set<Key, Hash, KeyEqual, Alloc> &right) noexcept {
        left.swap(right);
    }

    template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
    void swap(unordered_multiset<Key, Hash, KeyEqual, Alloc> &left,
              unordered_multiset<Key, Hash, KeyEqual, Alloc> &right) noexcept {
        left.swap(right);
    }
}

namespace rainy::collections {
    using foundation::collections::unordered_set;
    using foundation::collections::unordered_multiset;
}

#endif