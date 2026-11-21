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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_LOCAL_ITERATOR_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_LOCAL_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/type_traits/type_relations.hpp>
#include <rainy/foundation/collections/implements/dense_hash/node.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Forward iterator over a single bucket chain.
     *
     * @tparam NodePtr Pointer to a (possibly const) dense node.
     * @tparam PublicValue The value type exposed to users.
     * @tparam IsMap Whether the owning container is a map (affects reference shape).
     *
     * \lang simp-chinese
     * @brief 单个桶链上的前向迭代器。
     *
     * @tparam NodePtr 指向（可能为 const 的）稠密节点的指针。
     * @tparam PublicValue 暴露给用户的值类型。
     * @tparam IsMap 宿主容器是否为 map（影响引用形态）。
     */
    template <typename NodePtr, typename PublicValue, bool IsMap>
    class dense_local_iterator {
        template <typename, typename, bool>
        friend class dense_local_iterator;

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
        using size_type = std::size_t;

        RAINY_CONSTEXPR20 dense_local_iterator() noexcept : base_(nullptr), index_(npos) {
        }

        RAINY_CONSTEXPR20 dense_local_iterator(const node_ptr base, const size_type index) noexcept : base_(base), index_(index) {
        }

        template <typename OtherNodePtr, typename OtherValue, bool OtherIsMap,
                  type_traits::other_trans::enable_if_t<
                      IsMap == OtherIsMap && type_traits::properties::is_constructible_v<node_ptr, OtherNodePtr>, int> = 0>
        RAINY_CONSTEXPR20 dense_local_iterator(const dense_local_iterator<OtherNodePtr, OtherValue, OtherIsMap> &other) noexcept :
            base_(other.base_), index_(other.index_) {
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 size_type index() const noexcept {
            return index_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 reference operator*() const noexcept {
            return (base_ + index_)->elem;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer operator->() const noexcept {
            return utility::addressof(operator*());
        }

        RAINY_CONSTEXPR20 dense_local_iterator &operator++() noexcept {
            index_ = (base_ + index_)->next;
            return *this;
        }

        RAINY_CONSTEXPR20 dense_local_iterator operator++(int) noexcept {
            dense_local_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator==(const dense_local_iterator &right) const noexcept {
            return base_ == right.base_ && index_ == right.index_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool operator!=(const dense_local_iterator &right) const noexcept {
            return !(*this == right);
        }

    private:
        static constexpr size_type npos = static_cast<size_type>(-1);

        node_ptr base_;
        size_type index_;
    };
}

#endif
