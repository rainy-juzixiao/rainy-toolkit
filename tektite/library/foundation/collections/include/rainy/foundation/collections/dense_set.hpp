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
#ifndef RAINY_FOUNDATION_COLLECTIONS_DENSE_SET_HPP
#define RAINY_FOUNDATION_COLLECTIONS_DENSE_SET_HPP

#include <initializer_list>
#include <rainy/core/functional/functor.hpp>
#include <rainy/core/type_traits/meta_methods.hpp>
#include <rainy/core/utility/hash.hpp>
#include <rainy/foundation/collections/implements/dense_hash/table.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief Flat-hash set with stable iteration order (backshift erase).
     *
     * @tparam Key The element type.
     * @tparam Hash The hash function object type.
     * @tparam KeyEqual The key equality function object type.
     * @tparam Allocator The allocator type.
     *
     * \lang simp-chinese
     * @brief 迭代顺序稳定的扁平哈希集合（backshift 删除）。
     *
     * @tparam Key 元素类型。
     * @tparam Hash 哈希函数对象类型。
     * @tparam KeyEqual 键相等函数对象类型。
     * @tparam Allocator 分配器类型。
     */
    template <typename Key, typename Hash = utility::hash<Key>, typename KeyEqual = functional::equal<Key>,
              typename Allocator = core::memory::allocator<Key>>
    class dense_set : public implements::dense_table<erase_policy::backshift, false, Key, Key, Hash, KeyEqual, Allocator> {
        using base = implements::dense_table<erase_policy::backshift, false, Key, Key, Hash, KeyEqual, Allocator>;

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

        dense_set() = default;

        dense_set(const dense_set &) = default;

        dense_set(dense_set &&) noexcept = default;

        dense_set &operator=(const dense_set &) = default;

        dense_set &operator=(dense_set &&) noexcept = default;

        dense_set(std::initializer_list<value_type> ilist, const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                  const allocator_type &alloc = allocator_type()) : base(ilist.size(), hash, equal, alloc) {
            insert(ilist);
        }

        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        dense_set(InputIt first, InputIt last, const hasher &hash = hasher(), const key_equal &equal = key_equal(),
                  const allocator_type &alloc = allocator_type()) : base(hash, equal, alloc) {
            insert(first, last);
        }

        core::container::pair<iterator, bool> insert(const value_type &value) {
            return this->insert_value(value);
        }

        core::container::pair<iterator, bool> insert(value_type &&value) {
            return this->insert_value(utility::move(value));
        }

        template <typename InputIt>
        void insert(InputIt first, InputIt last) {
            for (; first != last; ++first) {
                this->insert_value(*first);
            }
        }

        void insert(std::initializer_list<value_type> ilist) {
            insert(ilist.begin(), ilist.end());
        }
    };

    template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
    void swap(dense_set<Key, Hash, KeyEqual, Alloc> &left, dense_set<Key, Hash, KeyEqual, Alloc> &right) noexcept {
        left.swap(right);
    }
}

#endif
