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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_FWD_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_FWD_HPP

#include <rainy/core/platform.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Placeholder mapped type used by set-like red-black trees.
     *
     * \lang simp-chinese
     * @brief 类集合红黑树使用的占位映射类型。
     */
    struct rb_no_mapped {};

    /**
     * \lang english
     * @brief Color tag of a red-black tree node.
     *
     * \lang simp-chinese
     * @brief 红黑树节点的颜色标记。
     */
    enum class rb_color : unsigned char {
        /**
         * \lang english
         * @brief The node is black.
         *
         * \lang simp-chinese
         * @brief 节点为黑色。
         */
        black,
        /**
         * \lang english
         * @brief The node is red.
         *
         * \lang simp-chinese
         * @brief 节点为红色。
         */
        red
    };

    /**
     * \lang english
     * @brief The link and color header shared by every red-black tree node and
     *        by the tree header itself.
     *
     * \lang simp-chinese
     * @brief 每个红黑树节点以及树头部共享的链接与颜色头部。
     */
    struct rb_node_base {
        /**
         * \lang english
         * @brief The parent node; the root's parent is the tree header.
         *
         * \lang simp-chinese
         * @brief 父节点；根的父节点为树头部。
         */
        rb_node_base *parent;

        /**
         * \lang english
         * @brief The left child.
         *
         * \lang simp-chinese
         * @brief 左子节点。
         */
        rb_node_base *left;

        /**
         * \lang english
         * @brief The right child.
         *
         * \lang simp-chinese
         * @brief 右子节点。
         */
        rb_node_base *right;

        /**
         * \lang english
         * @brief The node color.
         *
         * \lang simp-chinese
         * @brief 节点颜色。
         */
        rb_color color;

        /**
         * \lang english
         * @brief Returns the leftmost node of the subtree rooted at this node.
         *
         * \lang simp-chinese
         * @brief 返回以此节点为根的子树中的最左节点。
         */
        RAINY_NODISCARD rb_node_base *minimum() noexcept {
            rb_node_base *node = this;
            while (node->left != nullptr) {
                node = node->left;
            }
            return node;
        }

        /**
         * \lang english
         * @brief Returns the rightmost node of the subtree rooted at this node.
         *
         * \lang simp-chinese
         * @brief 返回以此节点为根的子树中的最右节点。
         */
        RAINY_NODISCARD rb_node_base *maximum() noexcept {
            rb_node_base *node = this;
            while (node->right != nullptr) {
                node = node->right;
            }
            return node;
        }
    };

    /**
     * \lang english
     * @brief A red-black tree node that owns a stored value.
     *
     * @tparam Stored The element type stored in the node.
     *
     * \lang simp-chinese
     * @brief 持有存储值的红黑树节点。
     *
     * @tparam Stored 节点所存储的元素类型。
     */
    template <typename Stored>
    struct rb_node : rb_node_base {
        using stored_type = Stored;

        /**
         * \lang english
         * @brief Constructs the node in place from forwarded arguments.
         *
         * @tparam Args The constructor argument types.
         * @param args Arguments forwarded to the stored element.
         *
         * \lang simp-chinese
         * @brief 以转发参数就地构造节点。
         *
         * @tparam Args 构造函数参数类型。
         * @param args 转发给所存元素的构造参数。
         */
        template <typename... Args>
        explicit rb_node(Args &&...args) :
            rb_node_base{nullptr, nullptr, nullptr, rb_color::red}, value(utility::forward<Args>(args)...) {
        }

        rb_node(const rb_node &) = delete;
        rb_node &operator=(const rb_node &) = delete;
        rb_node(rb_node &&) = delete;
        rb_node &operator=(rb_node &&) = delete;

        ~rb_node() = default;

        /**
         * \lang english
         * @brief The stored element.
         *
         * \lang simp-chinese
         * @brief 所存储的元素。
         */
        Stored value;
    };

    template <bool Multi, bool IsMap, typename Key, typename Mapped, typename Compare, typename Allocator>
    class rb_tree;

    template <typename Stored, typename PublicValue, bool IsConst>
    class rb_tree_iterator;
}

#endif
