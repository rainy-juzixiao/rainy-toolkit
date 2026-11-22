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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_ITERATOR_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/foundation/collections/implements/rb_tree/fwd.hpp>
#include <rainy/foundation/collections/implements/rb_tree/node.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Bidirectional iterator over the nodes of a red-black tree.
     *
     * @tparam Stored The element type stored in each node.
     * @tparam PublicValue The value type exposed to users.
     * @tparam IsConst Whether the iterator is const.
     *
     * \lang simp-chinese
     * @brief 红黑树节点上的双向迭代器。
     *
     * @tparam Stored 每个节点所存储的元素类型。
     * @tparam PublicValue 暴露给用户的值类型。
     * @tparam IsConst 该迭代器是否为 const。
     */
    template <typename Stored, typename PublicValue, bool IsConst>
    class rb_tree_iterator {
        template <typename, typename, bool>
        friend class rb_tree_iterator;

        using node_type = rb_node<Stored>;
        using base_pointer = type_traits::other_trans::conditional_t<IsConst, const rb_node_base *, rb_node_base *>;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = PublicValue;
        using difference_type = std::ptrdiff_t;
        using pointer = type_traits::other_trans::conditional_t<IsConst, const PublicValue *, PublicValue *>;
        using reference = type_traits::other_trans::conditional_t<IsConst, const PublicValue &, PublicValue &>;

        rb_tree_iterator() noexcept : node_(nullptr) {
        }

        explicit rb_tree_iterator(base_pointer node) noexcept : node_(node) {
        }

        template <bool WasConst, type_traits::other_trans::enable_if_t<IsConst && !WasConst, int> = 0>
        rb_tree_iterator(const rb_tree_iterator<Stored, PublicValue, WasConst> &right) noexcept : node_(right.node_) {
        }

        RAINY_NODISCARD reference operator*() const noexcept {
            return *reinterpret_cast<pointer>(utility::addressof(node_value()));
        }

        RAINY_NODISCARD pointer operator->() const noexcept {
            return utility::addressof(operator*());
        }

        rb_tree_iterator &operator++() noexcept {
            node_ = rb_increment(node_);
            return *this;
        }

        rb_tree_iterator operator++(int) noexcept {
            rb_tree_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        rb_tree_iterator &operator--() noexcept {
            node_ = rb_decrement(node_);
            return *this;
        }

        rb_tree_iterator operator--(int) noexcept {
            rb_tree_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        template <typename OtherStored, typename OtherValue, bool OtherConst>
        RAINY_NODISCARD bool operator==(const rb_tree_iterator<OtherStored, OtherValue, OtherConst> &right) const noexcept {
            return node_ == right.node_;
        }

        template <typename OtherStored, typename OtherValue, bool OtherConst>
        RAINY_NODISCARD bool operator!=(const rb_tree_iterator<OtherStored, OtherValue, OtherConst> &right) const noexcept {
            return node_ != right.node_;
        }

        /**
         * \lang english
         * @brief Returns the underlying node-base pointer.
         *
         * \lang simp-chinese
         * @brief 返回底层节点基类指针。
         */
        RAINY_NODISCARD base_pointer node() const noexcept {
            return node_;
        }

    private:
        RAINY_NODISCARD Stored &node_value() const noexcept {
            return static_cast<node_type *>(const_cast<rb_node_base *>(node_))->value;
        }

        base_pointer node_;
    };
}

#endif
