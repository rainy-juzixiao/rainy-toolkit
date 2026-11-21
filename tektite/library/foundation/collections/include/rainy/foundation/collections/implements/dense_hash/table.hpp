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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_TABLE_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_TABLE_HPP

#include <cstddef>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/collections/implements/dense_hash/fwd.hpp>
#include <rainy/foundation/collections/implements/dense_hash/iterator.hpp>
#include <rainy/foundation/collections/implements/dense_hash/local_iterator.hpp>
#include <rainy/foundation/collections/implements/dense_hash/node.hpp>

namespace rainy::foundation::collections::implements {
    struct dense_no_mapped {};

    template <bool IsMap, typename Key, typename Mapped>
    struct dense_stored {
        using type = Key;
    };

    template <typename Key, typename Mapped>
    struct dense_stored<true, Key, Mapped> {
        using type = core::container::pair<const Key, Mapped>;
    };

    template <bool IsMap, typename Key, typename Mapped>
    struct dense_public {
        using type = Key;
    };

    template <typename Key, typename Mapped>
    struct dense_public<true, Key, Mapped> {
        using type = core::container::pair<const Key, Mapped>;
    };

    /**
     * \lang english
     * @brief Shared flat-hash table.
     *
     * @tparam Policy The erase strategy.
     * @tparam IsMap Whether the table behaves as a map (true) or a set (false).
     * @tparam Key The key type.
     * @tparam Mapped The mapped type (unused when IsMap is false).
     * @tparam Hash The hash function object type.
     * @tparam KeyEqual The key equality function object type.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 共享扁平哈希表。
     *
     * @tparam Policy 删除策略。
     * @tparam IsMap 表作为 map（true）还是 set（false）。
     * @tparam Key 键类型。
     * @tparam Mapped 映射类型（IsMap 为 false 时不使用）。
     * @tparam Hash 哈希函数对象类型。
     * @tparam KeyEqual 键相等函数对象类型。
     * @tparam Allocator 分配器类型。
     */
    template <erase_policy Policy, bool IsMap, typename Key, typename Mapped, typename Hash, typename KeyEqual, typename Allocator>
    class dense_table {
        using traits = core::memory::allocator_traits<Allocator>;

    public:
        using key_type = Key;
        using mapped_type = type_traits::other_trans::conditional_t<IsMap, Mapped, dense_no_mapped>;
        using stored_type = typename dense_stored<IsMap, Key, Mapped>::type;
        using value_type = typename dense_public<IsMap, Key, Mapped>::type;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher = Hash;
        using key_equal = KeyEqual;
        using allocator_type = Allocator;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        using node_type = dense_node<stored_type>;
        using node_allocator = typename traits::template rebind_alloc<node_type>;
        using node_container = core::collections::vector<node_type, node_allocator>;
        using bucket_allocator = typename traits::template rebind_alloc<size_type>;
        using bucket_container = core::collections::vector<size_type, bucket_allocator>;

        using iterator = dense_table_iterator<node_type *, value_type, IsMap>;
        using const_iterator = dense_table_iterator<const node_type *, value_type, IsMap>;
        using local_iterator = dense_local_iterator<node_type *, value_type, IsMap>;
        using const_local_iterator = dense_local_iterator<const node_type *, value_type, IsMap>;

        static constexpr size_type npos = node_type::npos;
        static constexpr size_type minimum_bucket_count = 8;
        static constexpr float default_max_load_factor = 0.875f;

        dense_table() : dense_table(minimum_bucket_count) {
        }

        explicit dense_table(const allocator_type &alloc) : dense_table(minimum_bucket_count, hasher(), key_equal(), alloc) {
        }

        dense_table(const hasher &hash, const key_equal &equal, const allocator_type &alloc) :
            dense_table(minimum_bucket_count, hash, equal, alloc) {
        }

        explicit dense_table(const size_type bucket_count, const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                             const allocator_type &alloc = allocator_type()) :
            sparse_(bucket_count < minimum_bucket_count ? minimum_bucket_count : bucket_count, npos, bucket_allocator(alloc)),
            nodes_(node_allocator(alloc)), size_(0), dead_(0), max_load_factor_(default_max_load_factor), hash_(hash), equal_(equal) {
        }

        dense_table(const dense_table &other) : dense_table(other, other.get_allocator()) {
        }

        dense_table(const dense_table &other, const allocator_type &alloc) :
            sparse_(other.sparse_.size(), npos, bucket_allocator(alloc)), nodes_(node_allocator(alloc)), size_(0), dead_(0),
            max_load_factor_(other.max_load_factor_), hash_(other.hash_), equal_(other.equal_) {
            copy_from(other);
        }

        dense_table(dense_table &&other) noexcept :
            sparse_(utility::move(other.sparse_)), nodes_(utility::move(other.nodes_)), size_(other.size_), dead_(other.dead_),
            max_load_factor_(other.max_load_factor_), hash_(utility::move(other.hash_)), equal_(utility::move(other.equal_)) {
            other.size_ = 0;
            other.dead_ = 0;
        }

        dense_table(dense_table &&other, const allocator_type &alloc) :
            sparse_(bucket_allocator(alloc)), nodes_(node_allocator(alloc)), size_(0), dead_(0),
            max_load_factor_(other.max_load_factor_), hash_(utility::move(other.hash_)), equal_(utility::move(other.equal_)) {
            if constexpr (traits::is_always_equal::value) {
                sparse_ = utility::move(other.sparse_);
                nodes_ = utility::move(other.nodes_);
                size_ = other.size_;
                dead_ = other.dead_;
                other.size_ = 0;
                other.dead_ = 0;
            } else {
                sparse_.resize(other.sparse_.size(), npos);
                move_from(other);
            }
        }

        dense_table &operator=(const dense_table &other) {
            if (this != utility::addressof(other)) {
                dense_table tmp(other, other.get_allocator());
                swap(tmp);
            }
            return *this;
        }

        dense_table &operator=(dense_table &&other) noexcept {
            if (this != utility::addressof(other)) {
                sparse_ = utility::move(other.sparse_);
                nodes_ = utility::move(other.nodes_);
                size_ = other.size_;
                dead_ = other.dead_;
                max_load_factor_ = other.max_load_factor_;
                hash_ = utility::move(other.hash_);
                equal_ = utility::move(other.equal_);
                other.size_ = 0;
                other.dead_ = 0;
            }
            return *this;
        }

        RAINY_NODISCARD allocator_type get_allocator() const noexcept {
            return allocator_type(nodes_.get_allocator());
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return size_ == 0;
        }

        RAINY_NODISCARD size_type size() const noexcept {
            return size_;
        }

        RAINY_NODISCARD size_type max_size() const noexcept {
            return static_cast<size_type>(-1) / (sizeof(node_type) + sizeof(size_type));
        }

        RAINY_NODISCARD size_type bucket_count() const noexcept {
            return sparse_.size();
        }

        RAINY_NODISCARD size_type max_bucket_count() const noexcept {
            return static_cast<size_type>(-1) / sizeof(size_type);
        }

        RAINY_NODISCARD float load_factor() const noexcept {
            return sparse_.empty() ? 0.0f : static_cast<float>(size_) / static_cast<float>(sparse_.size());
        }

        RAINY_NODISCARD float max_load_factor() const noexcept {
            return max_load_factor_;
        }

        void max_load_factor(const float value) noexcept {
            max_load_factor_ = value;
        }

        RAINY_NODISCARD hasher hash_function() const {
            return hash_;
        }

        RAINY_NODISCARD key_equal key_eq() const {
            return equal_;
        }

        void clear() noexcept {
            sparse_.clear();
            sparse_.resize(minimum_bucket_count, npos);
            nodes_.clear();
            size_ = 0;
            dead_ = 0;
        }

        void swap(dense_table &other) noexcept {
            sparse_.swap(other.sparse_);
            nodes_.swap(other.nodes_);
            using std::swap;
            swap(size_, other.size_);
            swap(dead_, other.dead_);
            swap(max_load_factor_, other.max_load_factor_);
            swap(hash_, other.hash_);
            swap(equal_, other.equal_);
        }

        void rehash(const size_type count) {
            size_type target = minimum_bucket_count;
            while (target < count) {
                target *= 2;
            }
            if (target != sparse_.size()) {
                rebuild_sparse(target);
            }
        }

        void reserve(const size_type count) {
            const auto wanted = static_cast<size_type>(static_cast<float>(count) / max_load_factor_) + 1;
            if (wanted > sparse_.size()) {
                rehash(wanted);
            }
        }

        core::container::pair<iterator, bool> insert_value(const value_type &value) {
            if constexpr (IsMap) {
                return insert_stored(stored_type(value.first, value.second));
            } else {
                return insert_stored(stored_type(value));
            }
        }

        core::container::pair<iterator, bool> insert_value(value_type &&value) {
            if constexpr (IsMap) {
                return insert_stored(stored_type(utility::move(value.first), utility::move(value.second)));
            } else {
                return insert_stored(stored_type(utility::move(value)));
            }
        }

        template <typename... Args>
        core::container::pair<iterator, bool> emplace(Args &&...args) {
            return insert_stored(stored_type(utility::forward<Args>(args)...));
        }

        template <typename... Args>
        core::container::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args) {
            return try_emplace_impl(key, utility::forward<Args>(args)...);
        }

        template <typename... Args>
        core::container::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args) {
            return try_emplace_impl(utility::move(key), utility::forward<Args>(args)...);
        }

        template <typename M>
        core::container::pair<iterator, bool> insert_or_assign(const key_type &key, M &&obj) {
            return insert_or_assign_impl(key, utility::forward<M>(obj));
        }

        template <typename M>
        core::container::pair<iterator, bool> insert_or_assign(key_type &&key, M &&obj) {
            return insert_or_assign_impl(utility::move(key), utility::forward<M>(obj));
        }

        RAINY_NODISCARD mapped_type &operator[](const key_type &key) {
            return try_emplace_impl(key).first.node()->elem.second;
        }

        RAINY_NODISCARD mapped_type &operator[](key_type &&key) {
            return try_emplace_impl(utility::move(key)).first.node()->elem.second;
        }

        RAINY_NODISCARD mapped_type &at(const key_type &key) {
            auto it = find(key);
            if (it == end()) {
                core::exceptions::logic::throw_out_of_range("dense_map::at: key not found");
            }
            return it.node()->elem.second;
        }

        RAINY_NODISCARD const mapped_type &at(const key_type &key) const {
            auto it = find(key);
            if (it == end()) {
                core::exceptions::logic::throw_out_of_range("dense_map::at: key not found");
            }
            return it.node()->elem.second;
        }

        size_type erase(const key_type &key) {
            return erase_impl(key);
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>())),
                  type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<K, key_type>, int> = 0>
        size_type erase(const K &key) {
            return erase_impl(key);
        }

        iterator erase(const_iterator pos) {
            const size_type index = static_cast<size_type>(pos.node() - nodes_.data());
            if constexpr (Policy == erase_policy::tombstone) {
                const size_type next_index = advance_index(index);
                size_type successor_index = next_index;
                if (dead_ + 1 > size_ - 1) {
                    successor_index = 0;
                    for (size_type i = 0; i < index; ++i) {
                        if (nodes_[i].alive) {
                            ++successor_index;
                        }
                    }
                }
                erase_at(index);
                return make_iterator(successor_index);
            } else {
                const size_type last = nodes_.size() - 1;
                erase_at(index);
                if (index == last) {
                    return end();
                }
                return make_iterator(index + 1 == last ? index : index + 1);
            }
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD iterator find(const K &key) {
            return make_iterator(find_index(key));
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD const_iterator find(const K &key) const {
            return make_const_iterator(find_index(key));
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD size_type count(const K &key) const {
            return find_index(key) == npos ? 0 : 1;
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD bool contains(const K &key) const {
            return find_index(key) != npos;
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD core::container::pair<iterator, iterator> equal_range(const K &key) {
            auto first = find(key);
            auto last = first;
            if (last != end()) {
                ++last;
            }
            return {first, last};
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD core::container::pair<const_iterator, const_iterator> equal_range(const K &key) const {
            auto first = find(key);
            auto last = first;
            if (last != end()) {
                ++last;
            }
            return {first, last};
        }

        template <typename K, typename = decltype(utility::declval<hasher &>()(utility::declval<const K &>()))>
        RAINY_NODISCARD size_type bucket(const K &key) const {
            return bucket_for(key);
        }

        RAINY_NODISCARD size_type bucket_size(const size_type index) const {
            size_type cnt = 0;
            for (size_type i = sparse_[index]; i != npos; i = nodes_[i].next) {
                ++cnt;
            }
            return cnt;
        }

        RAINY_NODISCARD local_iterator begin(const size_type index) {
            return local_iterator(nodes_.data(), sparse_[index]);
        }

        RAINY_NODISCARD const_local_iterator begin(const size_type index) const {
            return const_local_iterator(nodes_.data(), sparse_[index]);
        }

        RAINY_NODISCARD const_local_iterator cbegin(const size_type index) const {
            return const_local_iterator(nodes_.data(), sparse_[index]);
        }

        RAINY_NODISCARD local_iterator end(const size_type) {
            return local_iterator(nodes_.data(), npos);
        }

        RAINY_NODISCARD const_local_iterator end(const size_type) const {
            return const_local_iterator(nodes_.data(), npos);
        }

        RAINY_NODISCARD const_local_iterator cend(const size_type) const {
            return const_local_iterator(nodes_.data(), npos);
        }

        RAINY_NODISCARD iterator begin() noexcept {
            return make_iterator(first_live_index());
        }

        RAINY_NODISCARD const_iterator begin() const noexcept {
            return make_const_iterator(first_live_index());
        }

        RAINY_NODISCARD const_iterator cbegin() const noexcept {
            return make_const_iterator(first_live_index());
        }

        RAINY_NODISCARD iterator end() noexcept {
            return make_iterator(npos);
        }

        RAINY_NODISCARD const_iterator end() const noexcept {
            return make_const_iterator(npos);
        }

        RAINY_NODISCARD const_iterator cend() const noexcept {
            return make_const_iterator(npos);
        }

    private:
        static const key_type &key_of(const stored_type &value) noexcept {
            if constexpr (IsMap) {
                return value.first;
            } else {
                return value;
            }
        }

        RAINY_NODISCARD size_type bucket_for(const key_type &key) const noexcept {
            return static_cast<size_type>(hash_(key)) & (sparse_.size() - 1);
        }

        template <typename K>
        RAINY_NODISCARD size_type bucket_for(const K &key) const noexcept {
            return static_cast<size_type>(hash_(key)) & (sparse_.size() - 1);
        }

        template <typename K>
        RAINY_NODISCARD size_type find_index(const K &key) const {
            if (sparse_.empty()) {
                return npos;
            }
            const size_type b = bucket_for(key);
            for (size_type i = sparse_[b]; i != npos; i = nodes_[i].next) {
                if (equal_(key_of(nodes_[i].elem), key)) {
                    return i;
                }
            }
            return npos;
        }

        RAINY_NODISCARD size_type first_live_index() const noexcept {
            for (size_type i = 0; i < nodes_.size(); ++i) {
                if (nodes_[i].alive) {
                    return i;
                }
            }
            return npos;
        }

        RAINY_NODISCARD size_type advance_index(const size_type from) const noexcept {
            for (size_type i = from + 1; i < nodes_.size(); ++i) {
                if (nodes_[i].alive) {
                    return i;
                }
            }
            return npos;
        }

        iterator make_iterator(const size_type index) noexcept {
            if (index == npos) {
                return iterator(nodes_.data() + nodes_.size(), nodes_.data() + nodes_.size());
            }
            return iterator(nodes_.data() + index, nodes_.data() + nodes_.size());
        }

        const_iterator make_const_iterator(const size_type index) const noexcept {
            if (index == npos) {
                return const_iterator(nodes_.data() + nodes_.size(), nodes_.data() + nodes_.size());
            }
            return const_iterator(nodes_.data() + index, nodes_.data() + nodes_.size());
        }

        void ensure_capacity_for(const size_type additional) {
            const auto needed = static_cast<size_type>(static_cast<float>(size_ + additional) / max_load_factor_) + 1;
            if (needed > sparse_.size()) {
                size_type target = sparse_.size();
                while (target < needed) {
                    target *= 2;
                }
                rebuild_sparse(target);
            }
        }

        void rebuild_sparse(const size_type new_count) {
            sparse_.clear();
            sparse_.resize(new_count, npos);
            for (size_type i = 0; i < nodes_.size(); ++i) {
                if (!nodes_[i].alive) {
                    continue;
                }
                const size_type b = bucket_for(key_of(nodes_[i].elem));
                nodes_[i].next = sparse_[b];
                sparse_[b] = i;
            }
        }

        core::container::pair<iterator, bool> insert_stored(stored_type &&value) {
            const key_type &key = key_of(value);
            if (const size_type found = find_index(key); found != npos) {
                return {make_iterator(found), false};
            }
            ensure_capacity_for(1);
            const size_type b = bucket_for(key);
            nodes_.emplace_back(sparse_[b], utility::move(value));
            sparse_[b] = nodes_.size() - 1;
            ++size_;
            return {make_iterator(sparse_[b]), true};
        }

        template <typename K, typename... Args>
        core::container::pair<iterator, bool> try_emplace_impl(K &&key, Args &&...args) {
            if (const size_type found = find_index(key); found != npos) {
                return {make_iterator(found), false};
            }
            ensure_capacity_for(1);
            const size_type b = bucket_for(key);
            if constexpr (IsMap) {
                nodes_.emplace_back(sparse_[b], utility::forward<K>(key), Mapped(utility::forward<Args>(args)...));
            } else {
                nodes_.emplace_back(sparse_[b], utility::forward<K>(key));
            }
            sparse_[b] = nodes_.size() - 1;
            ++size_;
            return {make_iterator(sparse_[b]), true};
        }

        template <typename K, typename M>
        core::container::pair<iterator, bool> insert_or_assign_impl(K &&key, M &&obj) {
            if (const size_type found = find_index(key); found != npos) {
                nodes_[found].elem.second = utility::forward<M>(obj);
                return {make_iterator(found), false};
            }
            ensure_capacity_for(1);
            const size_type b = bucket_for(key);
            nodes_.emplace_back(sparse_[b], utility::forward<K>(key), utility::forward<M>(obj));
            sparse_[b] = nodes_.size() - 1;
            ++size_;
            return {make_iterator(sparse_[b]), true};
        }

        template <typename K>
        size_type erase_impl(const K &key) {
            if (sparse_.empty()) {
                return 0;
            }
            const size_type b = bucket_for(key);
            size_type *link = utility::addressof(sparse_[b]);
            while (*link != npos) {
                if (equal_(key_of(nodes_[*link].elem), key)) {
                    const size_type index = *link;
                    *link = nodes_[index].next;
                    destroy_slot(index);
                    return 1;
                }
                link = utility::addressof(nodes_[*link].next);
            }
            return 0;
        }

        void erase_at(const size_type index) {
            const size_type b = bucket_for(key_of(nodes_[index].elem));
            size_type *link = utility::addressof(sparse_[b]);
            while (*link != index) {
                link = utility::addressof(nodes_[*link].next);
            }
            *link = nodes_[index].next;
            destroy_slot(index);
        }

        void destroy_slot(const size_type index) {
            if constexpr (Policy == erase_policy::tombstone) {
                nodes_[index].alive = false;
                --size_;
                ++dead_;
                if (dead_ > size_) {
                    compact();
                }
            } else {
                backshift_erase(index);
            }
        }

        void backshift_erase(const size_type index) {
            const size_type last = nodes_.size() - 1;
            if (index != last) {
                const size_type lb = bucket_for(key_of(nodes_[last].elem));
                size_type *link = utility::addressof(sparse_[lb]);
                while (*link != last) {
                    link = utility::addressof(nodes_[*link].next);
                }
                nodes_[index] = utility::move(nodes_[last]);
                *link = index;
            }
            nodes_.pop_back();
            --size_;
        }

        void compact() {
            size_type write = 0;
            for (size_type i = 0; i < nodes_.size(); ++i) {
                if (nodes_[i].alive) {
                    if (write != i) {
                        nodes_[write] = utility::move(nodes_[i]);
                    }
                    ++write;
                }
            }
            while (nodes_.size() > write) {
                nodes_.pop_back();
            }
            dead_ = 0;
            rebuild_sparse(sparse_.size());
        }

        void copy_from(const dense_table &other) {
            for (size_type i = 0; i < other.nodes_.size(); ++i) {
                if (!other.nodes_[i].alive) {
                    continue;
                }
                const size_type b = bucket_for(key_of(other.nodes_[i].elem));
                nodes_.emplace_back(sparse_[b], other.nodes_[i].elem);
                sparse_[b] = nodes_.size() - 1;
                ++size_;
            }
        }

        void move_from(dense_table &other) {
            for (size_type i = 0; i < other.nodes_.size(); ++i) {
                if (!other.nodes_[i].alive) {
                    continue;
                }
                const size_type b = bucket_for(key_of(other.nodes_[i].elem));
                nodes_.emplace_back(sparse_[b], utility::move(other.nodes_[i].elem));
                sparse_[b] = nodes_.size() - 1;
                ++size_;
            }
            other.clear();
        }

        bucket_container sparse_;
        node_container nodes_;
        size_type size_;
        size_type dead_;
        float max_load_factor_;
        hasher hash_;
        key_equal equal_;
    };
}

#endif
