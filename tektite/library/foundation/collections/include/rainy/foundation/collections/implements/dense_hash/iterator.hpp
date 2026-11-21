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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_ITERATOR_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/collections/implements/dense_hash/node.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Forward iterator over the dense node array.
     *
     * @tparam NodePtr Pointer to a (possibly const) dense node.
     * @tparam PublicValue The value type exposed to users.
     * @tparam IsMap Whether the owning container is a map (affects reference shape).
     *
     * \lang simp-chinese
     * @brief 稠密节点数组上的前向迭代器。
     *
     * @tparam NodePtr 指向（可能为 const 的）稠密节点的指针。
     * @tparam PublicValue 暴露给用户的值类型。
     * @tparam IsMap 宿主容器是否为 map（影响引用形态）。
     */
    template <typename NodePtr, typename PublicValue, bool IsMap>
    class dense_table_iterator {
        template <typename, typename, bool>
        friend class dense_table_iterator;

        using bare_node = typename std::remove_pointer<NodePtr>::type;
        static constexpr bool is_const_node = type_traits::properties::is_const_v<bare_node>;

    public:
        using node_type = bare_node;
        using node_ptr = NodePtr;
        using value_type = PublicValue;
        using difference_type = std::ptrdiff_t;
        using pointer = type_traits::other_trans::conditional_t<is_const_node, const value_type *, value_type *>;
        using reference = type_traits::other_trans::conditional_t<is_const_node, const value_type &, value_type &>;
        using iterator_category = std::forward_iterator_tag;

        RAINY_CONSTEXPR20 dense_table_iterator() noexcept : node_(nullptr), end_(nullptr) {
        }

        RAINY_CONSTEXPR20 dense_table_iterator(const node_ptr node, const node_ptr end) noexcept : node_(node), end_(end) {
            skip_dead();
        }

        template <typename OtherNodePtr, typename OtherValue, bool OtherIsMap,
                  type_traits::other_trans::enable_if_t<
                      IsMap == OtherIsMap && type_traits::properties::is_constructible_v<node_ptr, OtherNodePtr>, int> = 0>
        RAINY_CONSTEXPR20 dense_table_iterator(const dense_table_iterator<OtherNodePtr, OtherValue, OtherIsMap> &other) noexcept :
            node_(other.node_), end_(other.end_) {
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 node_ptr node() const noexcept {
            return node_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 reference operator*() const noexcept {
            return node_->elem;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer operator->() const noexcept {
            return utility::addressof(operator*());
        }

        RAINY_CONSTEXPR20 dense_table_iterator &operator++() noexcept {
            ++node_;
            skip_dead();
            return *this;
        }

        RAINY_CONSTEXPR20 dense_table_iterator operator++(int) noexcept {
            dense_table_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(const dense_table_iterator &right) const noexcept {
            return node_ == right.node_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(const dense_table_iterator &right) const noexcept {
            return node_ != right.node_;
        }

    private:
        RAINY_CONSTEXPR20 void skip_dead() noexcept {
            while (node_ != end_ && !node_->alive) {
                ++node_;
            }
        }

        node_ptr node_;
        node_ptr end_;
    };
}

#endif
