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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_TREE_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_TREE_HPP

#include <cstddef>
#include <initializer_list>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/iter_traits.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/collections/implements/rb_tree/iterator.hpp>
#include <rainy/foundation/collections/implements/rb_tree/node.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Owning handle over a single extracted red-black tree node.
     *
     * @tparam IsMap Whether the owning tree is a map.
     * @tparam Key The key type.
     * @tparam Mapped The mapped type.
     * @tparam Allocator The tree's allocator type.
     *
     * \lang simp-chinese
     * @brief 对单个被摘除的红黑树节点的持有句柄。
     *
     * @tparam IsMap 宿主树是否为 map。
     * @tparam Key 键类型。
     * @tparam Mapped 映射类型。
     * @tparam Allocator 树的分配器类型。
     */
    template <bool IsMap, typename Key, typename Mapped, typename Allocator>
    class rb_node_handle {
        template <bool, bool, typename, typename, typename, typename>
        friend class rb_tree;

        using stored_type = type_traits::other_trans::conditional_t<IsMap, core::container::pair<Key, Mapped>, Key>;
        using node_type = rb_node<stored_type>;
        using node_allocator = typename core::memory::allocator_traits<Allocator>::template rebind_alloc<node_type>;
        using node_traits = core::memory::allocator_traits<node_allocator>;

    public:
        using key_type = Key;
        using mapped_type = type_traits::other_trans::conditional_t<IsMap, Mapped, rb_no_mapped>;
        using allocator_type = Allocator;

        rb_node_handle() noexcept : node_(nullptr), alloc_() {
        }

        rb_node_handle(rb_node_handle &&other) noexcept : node_(other.node_), alloc_(other.alloc_) {
            other.node_ = nullptr;
        }

        rb_node_handle &operator=(rb_node_handle &&other) noexcept {
            if (this != utility::addressof(other)) {
                release();
                node_ = other.node_;
                alloc_ = other.alloc_;
                other.node_ = nullptr;
            }
            return *this;
        }

        rb_node_handle(const rb_node_handle &) = delete;
        rb_node_handle &operator=(const rb_node_handle &) = delete;

        ~rb_node_handle() {
            release();
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return node_ == nullptr;
        }

        explicit operator bool() const noexcept {
            return node_ != nullptr;
        }

        RAINY_NODISCARD key_type &key() const {
            if constexpr (IsMap) {
                return const_cast<key_type &>(node_->value.first);
            } else {
                return const_cast<key_type &>(node_->value);
            }
        }

        RAINY_NODISCARD mapped_type &mapped() const {
            static_assert(IsMap, "rb_node_handle::mapped() is only available for map-like trees");
            return const_cast<mapped_type &>(node_->value.second);
        }

        void swap(rb_node_handle &other) noexcept {
            using std::swap;
            swap(node_, other.node_);
            swap(alloc_, other.alloc_);
        }

    private:
        explicit rb_node_handle(node_type *node, const node_allocator &alloc) noexcept : node_(node), alloc_(alloc) {
        }

        void release() {
            if (node_ != nullptr) {
                node_traits::destroy(alloc_, node_);
                node_traits::deallocate(alloc_, node_, 1);
                node_ = nullptr;
            }
        }

        node_type *node_;
        node_allocator alloc_;
    };

    /**
     * \lang english
     * @brief Result of inserting a node handle into a unique red-black tree.
     *
     * @tparam Iterator The tree iterator type.
     * @tparam NodeType The node handle type.
     *
     * \lang simp-chinese
     * @brief 将节点句柄插入唯一红黑树的结果。
     *
     * @tparam Iterator 树迭代器类型。
     * @tparam NodeType 节点句柄类型。
     */
    template <typename Iterator, typename NodeType>
    struct rb_insert_return_type {
        Iterator position;
        bool inserted;
        NodeType node;
    };

    /**
     * \lang english
     * @brief Red-black tree backing the ordered associative containers.
     *
     * @tparam Multi Whether duplicate keys are allowed.
     * @tparam IsMap Whether elements are key/value pairs.
     * @tparam Key The key type.
     * @tparam Mapped The mapped type.
     * @tparam Compare The strict weak ordering used to order keys.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 支撑有序关联容器的红黑树。
     *
     * @tparam Multi 是否允许重复键。
     * @tparam IsMap 元素是否为键值对。
     * @tparam Key 键类型。
     * @tparam Mapped 映射类型。
     * @tparam Compare 用于排序键的严格弱序。
     * @tparam Allocator 分配器类型。
     */
    template <bool Multi, bool IsMap, typename Key, typename Mapped, typename Compare, typename Allocator>
    class rb_tree {
    public:
        using key_type = Key;
        using mapped_type = type_traits::other_trans::conditional_t<IsMap, Mapped, rb_no_mapped>;
        using value_type = type_traits::other_trans::conditional_t<IsMap, core::container::pair<const Key, Mapped>, Key>;
        using stored_type = type_traits::other_trans::conditional_t<IsMap, core::container::pair<Key, Mapped>, Key>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using key_compare = Compare;
        using allocator_type = Allocator;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        using node_type = rb_node<stored_type>;
        using node_allocator = typename core::memory::allocator_traits<Allocator>::template rebind_alloc<node_type>;
        using node_traits = core::memory::allocator_traits<node_allocator>;

        using iterator = rb_tree_iterator<stored_type, value_type, false>;
        using const_iterator = rb_tree_iterator<stored_type, value_type, true>;
        using node_handle = rb_node_handle<IsMap, Key, Mapped, Allocator>;
        using insert_return_type = rb_insert_return_type<iterator, node_handle>;

        /**
         * \lang english
         * @brief Comparator adapter that orders whole elements by their keys.
         *
         * \lang simp-chinese
         * @brief 按键对整个元素排序的比较器适配器。
         */
        class value_compare {
            friend class rb_tree;

        public:
            RAINY_NODISCARD bool operator()(const value_type &left, const value_type &right) const {
                if constexpr (IsMap) {
                    return comp(left.first, right.first);
                } else {
                    return comp(left, right);
                }
            }

        protected:
            Compare comp;
            explicit value_compare(Compare c) : comp(c) {
            }
        };

        rb_tree() : rb_tree(Compare(), Allocator()) {
        }

        explicit rb_tree(const Compare &comp, const Allocator &alloc = Allocator()) :
            header_(), size_(0), compare_(comp), alloc_(alloc) {
            init_header();
        }

        explicit rb_tree(const Allocator &alloc) : rb_tree(Compare(), alloc) {
        }

        rb_tree(const rb_tree &other) : rb_tree(other.compare_, node_traits::select_on_container_copy_construction(other.alloc_)) {
            copy_from(other);
        }

        rb_tree(const rb_tree &other, const Allocator &alloc) : header_(), size_(0), compare_(other.compare_), alloc_(alloc) {
            init_header();
            copy_from(other);
        }

        rb_tree(rb_tree &&other) noexcept :
            header_(other.header_), size_(other.size_), compare_(utility::move(other.compare_)), alloc_(utility::move(other.alloc_)) {
            adopt_header(other);
        }

        rb_tree(rb_tree &&other, const Allocator &alloc) : header_(), size_(0), compare_(utility::move(other.compare_)), alloc_(alloc) {
            init_header();
            if (alloc_ == other.alloc_) {
                header_ = other.header_;
                size_ = other.size_;
                adopt_header(other);
            } else {
                move_from(other);
                other.clear();
            }
        }

        rb_tree(std::initializer_list<value_type> init, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            rb_tree(comp, alloc) {
            insert_range(init.begin(), init.end());
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        rb_tree(InputIt first, InputIt last, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            rb_tree(comp, alloc) {
            insert_range(first, last);
        }

        ~rb_tree() {
            clear();
        }

        rb_tree &operator=(const rb_tree &other) {
            if (this != utility::addressof(other)) {
                rb_tree tmp(other);
                swap(tmp);
            }
            return *this;
        }

        rb_tree &operator=(rb_tree &&other) noexcept(node_traits::is_always_equal::value) {
            if (this == utility::addressof(other)) {
                return *this;
            }
            clear();
            if constexpr (node_traits::propagate_on_container_move_assignment::value) {
                alloc_ = utility::move(other.alloc_);
            }
            if (alloc_ == other.alloc_) {
                compare_ = utility::move(other.compare_);
                header_ = other.header_;
                size_ = other.size_;
                adopt_header(other);
            } else {
                compare_ = utility::move(other.compare_);
                move_from(other);
                other.clear();
            }
            return *this;
        }

        rb_tree &operator=(std::initializer_list<value_type> init) {
            clear();
            insert_range(init.begin(), init.end());
            return *this;
        }

        RAINY_NODISCARD allocator_type get_allocator() const noexcept {
            return allocator_type(alloc_);
        }

        RAINY_NODISCARD key_compare key_comp() const {
            return compare_;
        }

        RAINY_NODISCARD value_compare value_comp() const {
            return value_compare(compare_);
        }

        RAINY_NODISCARD iterator begin() noexcept {
            return iterator(header_.left);
        }

        RAINY_NODISCARD const_iterator begin() const noexcept {
            return const_iterator(header_.left);
        }

        RAINY_NODISCARD const_iterator cbegin() const noexcept {
            return const_iterator(header_.left);
        }

        RAINY_NODISCARD iterator end() noexcept {
            return iterator(&header_);
        }

        RAINY_NODISCARD const_iterator end() const noexcept {
            return const_iterator(&header_);
        }

        RAINY_NODISCARD const_iterator cend() const noexcept {
            return const_iterator(&header_);
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return size_ == 0;
        }

        RAINY_NODISCARD size_type size() const noexcept {
            return size_;
        }

        RAINY_NODISCARD size_type max_size() const noexcept {
            return node_traits::max_size(alloc_);
        }

        void clear() noexcept {
            destroy_subtree(header_.parent);
            header_.parent = nullptr;
            header_.left = &header_;
            header_.right = &header_;
            size_ = 0;
        }

        void swap(rb_tree &other) noexcept {
            using std::swap;
            swap(header_.parent, other.header_.parent);
            swap(header_.left, other.header_.left);
            swap(header_.right, other.header_.right);
            swap(size_, other.size_);
            swap(compare_, other.compare_);
            if constexpr (node_traits::propagate_on_container_swap::value) {
                swap(alloc_, other.alloc_);
            }
            if (header_.parent != nullptr) {
                header_.parent->parent = &header_;
            }
            if (other.header_.parent != nullptr) {
                other.header_.parent->parent = &other.header_;
            }
        }

        template <typename... Args>
        core::container::pair<iterator, bool> emplace_unique(Args &&...args) {
            node_type *node = create_node(utility::forward<Args>(args)...);
            core::container::pair<rb_node_base *, rb_node_base *> where = get_insert_unique_pos(key_of(node->value));
            if (where.first != nullptr) {
                destroy_node(node);
                return core::container::make_pair(iterator(where.first), false);
            }
            link_node(where.second, node);
            ++size_;
            return core::container::make_pair(iterator(node), true);
        }

        template <typename... Args>
        iterator emplace_multi(Args &&...args) {
            node_type *node = create_node(utility::forward<Args>(args)...);
            rb_node_base *parent = get_insert_multi_pos(key_of(node->value));
            link_node(parent, node);
            ++size_;
            return iterator(node);
        }

        core::container::pair<iterator, bool> insert_unique(const value_type &value) {
            return emplace_unique(value);
        }

        core::container::pair<iterator, bool> insert_unique(value_type &&value) {
            return emplace_unique(utility::move(value));
        }

        iterator insert_multi(const value_type &value) {
            return emplace_multi(value);
        }

        iterator insert_multi(value_type &&value) {
            return emplace_multi(utility::move(value));
        }

        template <typename P, type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        core::container::pair<iterator, bool> insert_unique(P &&value) {
            return emplace_unique(utility::forward<P>(value));
        }

        template <typename P, type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert_multi(P &&value) {
            return emplace_multi(utility::forward<P>(value));
        }

        template <typename InputIt>
        void insert_unique_range(InputIt first, InputIt last) {
            for (; first != last; ++first) {
                emplace_unique(*first);
            }
        }

        template <typename InputIt>
        void insert_multi_range(InputIt first, InputIt last) {
            for (; first != last; ++first) {
                emplace_multi(*first);
            }
        }

        template <typename InputIt>
        void insert_range(InputIt first, InputIt last) {
            if constexpr (Multi) {
                insert_multi_range(first, last);
            } else {
                insert_unique_range(first, last);
            }
        }

        template <typename... Args>
        iterator emplace_hint_unique(const_iterator hint, Args &&...args) {
            (void) hint;
            return emplace_unique(utility::forward<Args>(args)...).first;
        }

        template <typename... Args>
        iterator emplace_hint_multi(const_iterator hint, Args &&...args) {
            (void) hint;
            return emplace_multi(utility::forward<Args>(args)...);
        }

        template <typename K>
        RAINY_NODISCARD iterator find(const K &key) {
            return iterator(find_node(key));
        }

        template <typename K>
        RAINY_NODISCARD const_iterator find(const K &key) const {
            return const_iterator(find_node(key));
        }

        template <typename K>
        RAINY_NODISCARD bool contains(const K &key) const {
            return find_node(key) != const_end();
        }

        template <typename K>
        RAINY_NODISCARD size_type count(const K &key) const {
            if constexpr (Multi) {
                const_iterator first = lower_bound(key);
                const_iterator last = upper_bound(key);
                size_type total = 0;
                for (; first != last; ++first) {
                    ++total;
                }
                return total;
            } else {
                return find_node(key) != const_end() ? 1 : 0;
            }
        }

        template <typename K>
        RAINY_NODISCARD iterator lower_bound(const K &key) {
            return iterator(lower_bound_node(key));
        }

        template <typename K>
        RAINY_NODISCARD const_iterator lower_bound(const K &key) const {
            return const_iterator(lower_bound_node(key));
        }

        template <typename K>
        RAINY_NODISCARD iterator upper_bound(const K &key) {
            return iterator(upper_bound_node(key));
        }

        template <typename K>
        RAINY_NODISCARD const_iterator upper_bound(const K &key) const {
            return const_iterator(upper_bound_node(key));
        }

        template <typename K>
        RAINY_NODISCARD core::container::pair<iterator, iterator> equal_range(const K &key) {
            return core::container::make_pair(lower_bound(key), upper_bound(key));
        }

        template <typename K>
        RAINY_NODISCARD core::container::pair<const_iterator, const_iterator> equal_range(const K &key) const {
            return core::container::make_pair(lower_bound(key), upper_bound(key));
        }

        iterator erase(const_iterator pos) {
            rb_node_base *next = rb_increment(const_cast<rb_node_base *>(pos.node()));
            rb_node_base *removed = rb_rebalance_for_erase(&header_, const_cast<rb_node_base *>(pos.node()));
            destroy_node(static_cast<node_type *>(removed));
            --size_;
            return iterator(next);
        }

        iterator erase(const_iterator first, const_iterator last) {
            while (first != last) {
                first = erase(first);
            }
            return iterator(const_cast<rb_node_base *>(first.node()));
        }

        size_type erase(const key_type &key) {
            core::container::pair<iterator, iterator> range = equal_range(key);
            size_type removed = 0;
            iterator it = range.first;
            while (it != range.second) {
                it = erase(it);
                ++removed;
            }
            return removed;
        }

        node_handle extract(const_iterator pos) {
            if (pos == cend()) {
                return node_handle();
            }
            rb_node_base *removed = rb_rebalance_for_erase(&header_, const_cast<rb_node_base *>(pos.node()));
            --size_;
            return node_handle(static_cast<node_type *>(removed), alloc_);
        }

        node_handle extract(const key_type &key) {
            iterator it = find(key);
            if (it == end()) {
                return node_handle();
            }
            return extract(const_iterator(it));
        }

        core::container::pair<iterator, bool> insert_unique_node(node_handle &&handle) {
            if (handle.empty()) {
                return core::container::make_pair(end(), false);
            }
            node_type *node = handle.node_;
            handle.node_ = nullptr;
            core::container::pair<rb_node_base *, rb_node_base *> where = get_insert_unique_pos(key_of(node->value));
            if (where.first != nullptr) {
                handle.node_ = node;
                return core::container::make_pair(iterator(where.first), false);
            }
            link_node(where.second, node);
            ++size_;
            return core::container::make_pair(iterator(node), true);
        }

        iterator insert_multi_node(node_handle &&handle) {
            if (handle.empty()) {
                return end();
            }
            node_type *node = handle.node_;
            handle.node_ = nullptr;
            link_node(get_insert_multi_pos(key_of(node->value)), node);
            ++size_;
            return iterator(node);
        }

        void merge(rb_tree &source) {
            if (this == utility::addressof(source)) {
                return;
            }
            iterator it = source.begin();
            while (it != source.end()) {
                iterator current = it++;
                if constexpr (Multi) {
                    emplace_multi(*current);
                    source.erase(current);
                } else {
                    if (!contains(key_of_public(*current))) {
                        emplace_unique(*current);
                        source.erase(current);
                    }
                }
            }
        }

    private:
        static const key_type &key_of(const stored_type &value) noexcept {
            if constexpr (IsMap) {
                return value.first;
            } else {
                return value;
            }
        }

        static const key_type &key_of_public(const value_type &value) noexcept {
            if constexpr (IsMap) {
                return value.first;
            } else {
                return value;
            }
        }

        static const key_type &key_of_value(rb_node_base *node) noexcept {
            return key_of(static_cast<node_type *>(node)->value);
        }

        RAINY_NODISCARD rb_node_base *const_end() const noexcept {
            return const_cast<rb_node_base *>(&header_);
        }

        void init_header() noexcept {
            header_.parent = nullptr;
            header_.left = &header_;
            header_.right = &header_;
            header_.color = rb_color::red;
        }

        void adopt_header(rb_tree &other) noexcept {
            if (header_.parent != nullptr) {
                header_.parent->parent = &header_;
                header_.left = header_.parent->minimum();
                header_.right = header_.parent->maximum();
            } else {
                header_.left = &header_;
                header_.right = &header_;
            }
            other.init_header();
            other.size_ = 0;
        }

        template <typename... Args>
        node_type *create_node(Args &&...args) {
            node_type *node = node_traits::allocate(alloc_, 1);
            try {
                node_traits::construct(alloc_, node, utility::forward<Args>(args)...);
            } catch (...) {
                node_traits::deallocate(alloc_, node, 1);
                throw;
            }
            return node;
        }

        void destroy_node(node_type *node) noexcept {
            node_traits::destroy(alloc_, node);
            node_traits::deallocate(alloc_, node, 1);
        }

        void destroy_subtree(rb_node_base *node) noexcept {
            while (node != nullptr) {
                destroy_subtree(node->right);
                rb_node_base *left = node->left;
                destroy_node(static_cast<node_type *>(node));
                node = left;
            }
        }

        void link_node(rb_node_base *parent, node_type *node) noexcept {
            node->parent = parent;
            node->left = nullptr;
            node->right = nullptr;
            node->color = rb_color::red;
            if (parent == &header_) {
                header_.parent = node;
                header_.left = node;
                header_.right = node;
            } else if (compare_(key_of(node->value), key_of_value(parent))) {
                parent->left = node;
                if (parent == header_.left) {
                    header_.left = node;
                }
            } else {
                parent->right = node;
                if (parent == header_.right) {
                    header_.right = node;
                }
            }
            rb_rebalance_for_insert(header_.parent, node);
            header_.left = header_.parent->minimum();
            header_.right = header_.parent->maximum();
        }

        template <typename K>
        RAINY_NODISCARD core::container::pair<rb_node_base *, rb_node_base *> get_insert_unique_pos(const K &key) {
            rb_node_base *x = header_.parent;
            rb_node_base *y = &header_;
            bool comp = true;
            while (x != nullptr) {
                y = x;
                comp = compare_(key, key_of_value(x));
                x = comp ? x->left : x->right;
            }
            rb_node_base *j = y;
            if (comp) {
                if (j == header_.left) {
                    return core::container::make_pair(x, y);
                }
                j = rb_decrement(j);
            }
            if (compare_(key_of_value(j), key)) {
                return core::container::make_pair(x, y);
            }
            return core::container::make_pair(j, y);
        }

        template <typename K>
        RAINY_NODISCARD rb_node_base *get_insert_multi_pos(const K &key) {
            rb_node_base *x = header_.parent;
            rb_node_base *y = &header_;
            while (x != nullptr) {
                y = x;
                x = compare_(key, key_of_value(x)) ? x->left : x->right;
            }
            return y;
        }

        template <typename K>
        RAINY_NODISCARD rb_node_base *find_node(const K &key) const {
            rb_node_base *j = lower_bound_node(key);
            if (j == const_end() || compare_(key, key_of_value(j))) {
                return const_end();
            }
            return j;
        }

        template <typename K>
        RAINY_NODISCARD rb_node_base *lower_bound_node(const K &key) const {
            rb_node_base *x = header_.parent;
            rb_node_base *y = const_end();
            while (x != nullptr) {
                if (!compare_(key_of_value(x), key)) {
                    y = x;
                    x = x->left;
                } else {
                    x = x->right;
                }
            }
            return y;
        }

        template <typename K>
        RAINY_NODISCARD rb_node_base *upper_bound_node(const K &key) const {
            rb_node_base *x = header_.parent;
            rb_node_base *y = const_end();
            while (x != nullptr) {
                if (compare_(key, key_of_value(x))) {
                    y = x;
                    x = x->left;
                } else {
                    x = x->right;
                }
            }
            return y;
        }

        void copy_from(const rb_tree &other) {
            for (const_iterator it = other.begin(); it != other.end(); ++it) {
                if constexpr (Multi) {
                    emplace_multi(*it);
                } else {
                    emplace_unique(*it);
                }
            }
        }

        void move_from(rb_tree &other) {
            for (iterator it = other.begin(); it != other.end(); ++it) {
                if constexpr (Multi) {
                    emplace_multi(utility::move(*it));
                } else {
                    emplace_unique(utility::move(*it));
                }
            }
        }

        rb_node_base header_;
        size_type size_;
        Compare compare_;
        node_allocator alloc_;
    };
}

#endif
