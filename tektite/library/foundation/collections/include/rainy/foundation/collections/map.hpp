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
#ifndef RAINY_FOUNDATION_COLLECTIONS_MAP_HPP
#define RAINY_FOUNDATION_COLLECTIONS_MAP_HPP

#include <initializer_list>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/functional/functor.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/collections/implements/rb_tree/tree.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief A sorted associative container with unique keys.
     *
     *  Elements are key/value pairs ordered by their keys. Iteration is in
     *  ascending key order. Lookup, insertion, and removal are logarithmic.
     *
     * @tparam Key The key type.
     * @tparam Mapped The mapped value type.
     * @tparam Compare The strict weak ordering used to order keys.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 具有唯一键的有序关联容器。
     *
     *  元素为按键排序的键值对。迭代按键升序进行。查找、插入与删除均为对数复杂度。
     *
     * @tparam Key 键类型。
     * @tparam Mapped 映射值类型。
     * @tparam Compare 用于排序键的严格弱序。
     * @tparam Allocator 分配器类型。
     */
    template <typename Key, typename Mapped, typename Compare = functional::less<Key>,
              typename Allocator = core::memory::allocator<core::container::pair<const Key, Mapped>>>
    class map : private implements::rb_tree<false, true, Key, Mapped, Compare, Allocator> {
    public:
        using base = implements::rb_tree<false, true, Key, Mapped, Compare, Allocator>;

        using key_type = typename base::key_type;
        using mapped_type = typename base::mapped_type;
        using value_type = typename base::value_type;
        using size_type = typename base::size_type;
        using difference_type = typename base::difference_type;
        using key_compare = typename base::key_compare;
        using allocator_type = typename base::allocator_type;
        using reference = typename base::reference;
        using const_reference = typename base::const_reference;
        using pointer = typename base::pointer;
        using const_pointer = typename base::const_pointer;
        using iterator = typename base::iterator;
        using const_iterator = typename base::const_iterator;
        using node_type = typename base::node_handle;
        using insert_return_type = typename base::insert_return_type;
        using value_compare = typename base::value_compare;

        using base::base;
        using base::begin;
        using base::cbegin;
        using base::cend;
        using base::clear;
        using base::contains;
        using base::count;
        using base::empty;
        using base::end;
        using base::equal_range;
        using base::erase;
        using base::extract;
        using base::find;
        using base::get_allocator;
        using base::key_comp;
        using base::lower_bound;
        using base::max_size;
        using base::size;
        using base::upper_bound;
        using base::value_comp;

        void swap(map &other) noexcept {
            base::swap(other);
        }

        void merge(map &source) {
            base::merge(source);
        }

        map() = default;

        map(const map &) = default;

        map(map &&) noexcept = default;

        map &operator=(const map &) = default;

        map &operator=(map &&) noexcept = default;

        explicit map(const Compare &comp, const Allocator &alloc = Allocator()) : base(comp, alloc) {
        }

        explicit map(const Allocator &alloc) : base(alloc) {
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        map(InputIt first, InputIt last, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) : base(comp, alloc) {
            this->insert_unique_range(first, last);
        }

        map(std::initializer_list<value_type> init, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            base(comp, alloc) {
            this->insert_unique_range(init.begin(), init.end());
        }

        core::container::pair<iterator, bool> insert(const value_type &value) {
            return this->insert_unique(value);
        }

        core::container::pair<iterator, bool> insert(value_type &&value) {
            return this->insert_unique(utility::move(value));
        }

        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        core::container::pair<iterator, bool> insert(P &&value) {
            return this->insert_unique(utility::forward<P>(value));
        }

        iterator insert(const_iterator hint, const value_type &value) {
            return this->emplace_hint_unique(hint, value);
        }

        iterator insert(const_iterator hint, value_type &&value) {
            return this->emplace_hint_unique(hint, utility::move(value));
        }

        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert(const_iterator hint, P &&value) {
            return this->emplace_hint_unique(hint, utility::forward<P>(value));
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last) {
            this->insert_unique_range(first, last);
        }

        void insert(std::initializer_list<value_type> init) {
            this->insert_unique_range(init.begin(), init.end());
        }

        insert_return_type insert(node_type &&handle) {
            core::container::pair<iterator, bool> result = this->insert_unique_node(utility::move(handle));
            if (result.second) {
                return {result.first, true, node_type{}};
            }
            return {result.first, false, utility::move(handle)};
        }

        iterator insert(const_iterator hint, node_type &&handle) {
            (void) hint;
            return insert(utility::move(handle)).position;
        }

        template <typename... Args>
        core::container::pair<iterator, bool> emplace(Args &&...args) {
            return this->emplace_unique(utility::forward<Args>(args)...);
        }

        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
            return this->emplace_hint_unique(hint, utility::forward<Args>(args)...);
        }

        template <typename M>
        core::container::pair<iterator, bool> insert_or_assign(const key_type &key, M &&obj) {
            return insert_or_assign_impl(key, utility::forward<M>(obj));
        }

        template <typename M>
        core::container::pair<iterator, bool> insert_or_assign(key_type &&key, M &&obj) {
            return insert_or_assign_impl(utility::move(key), utility::forward<M>(obj));
        }

        template <typename... Args>
        core::container::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args) {
            return try_emplace_impl(key, utility::forward<Args>(args)...);
        }

        template <typename... Args>
        core::container::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args) {
            return try_emplace_impl(utility::move(key), utility::forward<Args>(args)...);
        }

        template <typename... Args>
        iterator try_emplace(const_iterator hint, const key_type &key, Args &&...args) {
            (void) hint;
            return try_emplace_impl(key, utility::forward<Args>(args)...).first;
        }

        template <typename... Args>
        iterator try_emplace(const_iterator hint, key_type &&key, Args &&...args) {
            (void) hint;
            return try_emplace_impl(utility::move(key), utility::forward<Args>(args)...).first;
        }

        mapped_type &at(const key_type &key) {
            iterator it = this->find(key);
            if (it == this->end()) {
                core::exceptions::logic::throw_out_of_range("map::at: key not found");
            }
            return it->second;
        }

        const mapped_type &at(const key_type &key) const {
            const_iterator it = this->find(key);
            if (it == this->end()) {
                core::exceptions::logic::throw_out_of_range("map::at: key not found");
            }
            return it->second;
        }

        mapped_type &operator[](const key_type &key) {
            return try_emplace_impl(key).first->second;
        }

        mapped_type &operator[](key_type &&key) {
            return try_emplace_impl(utility::move(key)).first->second;
        }

    private:
        template <typename K, typename M>
        core::container::pair<iterator, bool> insert_or_assign_impl(K &&key, M &&obj) {
            iterator it = this->find(key);
            if (it != this->end()) {
                it->second = utility::forward<M>(obj);
                return core::container::make_pair(it, false);
            }
            return this->emplace_unique(utility::forward<K>(key), utility::forward<M>(obj));
        }

        template <typename K, typename... Args>
        core::container::pair<iterator, bool> try_emplace_impl(K &&key, Args &&...args) {
            iterator it = this->find(key);
            if (it != this->end()) {
                return core::container::make_pair(it, false);
            }
            return this->emplace_unique(utility::forward<K>(key), mapped_type(utility::forward<Args>(args)...));
        }
    };

    /**
     * \lang english
     * @brief A sorted associative container that allows duplicate keys.
     *
     * @tparam Key The key type.
     * @tparam Mapped The mapped value type.
     * @tparam Compare The strict weak ordering used to order keys.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 允许重复键的有序关联容器。
     *
     * @tparam Key 键类型。
     * @tparam Mapped 映射值类型。
     * @tparam Compare 用于排序键的严格弱序。
     * @tparam Allocator 分配器类型。
     */
    template <typename Key, typename Mapped, typename Compare = functional::less<Key>,
              typename Allocator = core::memory::allocator<core::container::pair<const Key, Mapped>>>
    class multimap : private implements::rb_tree<true, true, Key, Mapped, Compare, Allocator> {
    public:
        using base = implements::rb_tree<true, true, Key, Mapped, Compare, Allocator>;

        using key_type = typename base::key_type;
        using mapped_type = typename base::mapped_type;
        using value_type = typename base::value_type;
        using size_type = typename base::size_type;
        using difference_type = typename base::difference_type;
        using key_compare = typename base::key_compare;
        using allocator_type = typename base::allocator_type;
        using reference = typename base::reference;
        using const_reference = typename base::const_reference;
        using pointer = typename base::pointer;
        using const_pointer = typename base::const_pointer;
        using iterator = typename base::iterator;
        using const_iterator = typename base::const_iterator;
        using node_type = typename base::node_handle;
        using value_compare = typename base::value_compare;

        using base::base;
        using base::begin;
        using base::cbegin;
        using base::cend;
        using base::clear;
        using base::contains;
        using base::count;
        using base::empty;
        using base::end;
        using base::equal_range;
        using base::erase;
        using base::extract;
        using base::find;
        using base::get_allocator;
        using base::key_comp;
        using base::lower_bound;
        using base::max_size;
        using base::size;
        using base::upper_bound;
        using base::value_comp;

        void swap(multimap &other) noexcept {
            base::swap(other);
        }

        void merge(multimap &source) {
            base::merge(source);
        }

        multimap() = default;

        multimap(const multimap &) = default;

        multimap(multimap &&) noexcept = default;

        multimap &operator=(const multimap &) = default;

        multimap &operator=(multimap &&) noexcept = default;

        explicit multimap(const Compare &comp, const Allocator &alloc = Allocator()) : base(comp, alloc) {
        }

        explicit multimap(const Allocator &alloc) : base(alloc) {
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        multimap(InputIt first, InputIt last, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            base(comp, alloc) {
            this->insert_multi_range(first, last);
        }

        multimap(std::initializer_list<value_type> init, const Compare &comp = Compare(), const Allocator &alloc = Allocator()) :
            base(comp, alloc) {
            this->insert_multi_range(init.begin(), init.end());
        }

        iterator insert(const value_type &value) {
            return this->insert_multi(value);
        }

        iterator insert(value_type &&value) {
            return this->insert_multi(utility::move(value));
        }

        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert(P &&value) {
            return this->insert_multi(utility::forward<P>(value));
        }

        iterator insert(const_iterator hint, const value_type &value) {
            return this->emplace_hint_multi(hint, value);
        }

        iterator insert(const_iterator hint, value_type &&value) {
            return this->emplace_hint_multi(hint, utility::move(value));
        }

        template <typename P,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<value_type, P &&>, int> = 0>
        iterator insert(const_iterator hint, P &&value) {
            return this->emplace_hint_multi(hint, utility::forward<P>(value));
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last) {
            this->insert_multi_range(first, last);
        }

        void insert(std::initializer_list<value_type> init) {
            this->insert_multi_range(init.begin(), init.end());
        }

        iterator insert(node_type &&handle) {
            return this->insert_multi_node(utility::move(handle));
        }

        iterator insert(const_iterator hint, node_type &&handle) {
            (void) hint;
            return this->insert_multi_node(utility::move(handle));
        }

        template <typename... Args>
        iterator emplace(Args &&...args) {
            return this->emplace_multi(utility::forward<Args>(args)...);
        }

        template <typename... Args>
        iterator emplace_hint(const_iterator hint, Args &&...args) {
            return this->emplace_hint_multi(hint, utility::forward<Args>(args)...);
        }
    };

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator==(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        if (left.size() != right.size()) {
            return false;
        }
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end(); ++lit, ++rit) {
            if (lit->first != rit->first || lit->second != rit->second) {
                return false;
            }
        }
        return true;
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator!=(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return !(left == right);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end() && rit != right.end(); ++lit, ++rit) {
            if (lit->first < rit->first) {
                return true;
            }
            if (rit->first < lit->first) {
                return false;
            }
            if (lit->second < rit->second) {
                return true;
            }
            if (rit->second < lit->second) {
                return false;
            }
        }
        return lit == left.end() && rit != right.end();
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return right < left;
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<=(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return !(right < left);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>=(const map<Key, Mapped, Compare, Alloc> &left, const map<Key, Mapped, Compare, Alloc> &right) {
        return !(left < right);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator==(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        if (left.size() != right.size()) {
            return false;
        }
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end(); ++lit, ++rit) {
            if (lit->first != rit->first || lit->second != rit->second) {
                return false;
            }
        }
        return true;
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator!=(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return !(left == right);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        auto lit = left.begin();
        auto rit = right.begin();
        for (; lit != left.end() && rit != right.end(); ++lit, ++rit) {
            if (lit->first < rit->first) {
                return true;
            }
            if (rit->first < lit->first) {
                return false;
            }
            if (lit->second < rit->second) {
                return true;
            }
            if (rit->second < lit->second) {
                return false;
            }
        }
        return lit == left.end() && rit != right.end();
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return right < left;
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator<=(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return !(right < left);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    bool operator>=(const multimap<Key, Mapped, Compare, Alloc> &left, const multimap<Key, Mapped, Compare, Alloc> &right) {
        return !(left < right);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    void swap(map<Key, Mapped, Compare, Alloc> &left, map<Key, Mapped, Compare, Alloc> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }

    template <typename Key, typename Mapped, typename Compare, typename Alloc>
    void swap(multimap<Key, Mapped, Compare, Alloc> &left,
              multimap<Key, Mapped, Compare, Alloc> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

namespace rainy::collections {
    using foundation::collections::map;
    using foundation::collections::multimap;
}

namespace std { // NOLINT
    template <typename Key, typename Mapped, typename Compare, typename Allocator, typename Alloc>
    struct uses_allocator<rainy::foundation::collections::map<Key, Mapped, Compare, Allocator>, Alloc> :
        rainy::type_traits::helper::true_type {}; // NOLINT

    template <typename Key, typename Mapped, typename Compare, typename Allocator, typename Alloc>
    struct uses_allocator<rainy::foundation::collections::multimap<Key, Mapped, Compare, Allocator>, Alloc> :
        rainy::type_traits::helper::true_type {}; // NOLINT
}

#endif
