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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_NODE_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_RB_TREE_NODE_HPP

#include <rainy/foundation/collections/implements/rb_tree/fwd.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief Returns the in-order successor of @p node.
     *
     * @tparam NodeBase The node-base type, possibly const-qualified.
     * @param node The node to advance from; may be the tree header.
     * @return The successor node, or the header when @p node is the rightmost.
     *
     * \lang simp-chinese
     * @brief 返回 @p node 的中序后继。
     *
     * @tparam NodeBase 节点基类型，可带 const 限定。
     * @param node 起始节点；可以是树头部。
     * @return 后继节点；当 @p node 为最右节点时返回头部。
     */
    template <typename NodeBase>
    RAINY_NODISCARD NodeBase *rb_increment(NodeBase *node) noexcept {
        if (node->right != nullptr) {
            node = node->right;
            while (node->left != nullptr) {
                node = node->left;
            }
        } else {
            NodeBase *parent = node->parent;
            while (node == parent->right) {
                node = parent;
                parent = parent->parent;
            }
            if (node->right != parent) {
                node = parent;
            }
        }
        return node;
    }

    /**
     * \lang english
     * @brief Returns the in-order predecessor of @p node.
     *
     * @tparam NodeBase The node-base type, possibly const-qualified.
     * @param node The node to move back from; may be the tree header.
     * @return The predecessor node, or the header when @p node is the leftmost.
     *
     * \lang simp-chinese
     * @brief 返回 @p node 的中序前驱。
     *
     * @tparam NodeBase 节点基类型，可带 const 限定。
     * @param node 起始节点；可以是树头部。
     * @return 前驱节点；当 @p node 为最左节点时返回头部。
     */
    template <typename NodeBase>
    RAINY_NODISCARD NodeBase *rb_decrement(NodeBase *node) noexcept {
        if (node->color == rb_color::red && node->parent->parent == node) {
            node = node->right;
        } else if (node->left != nullptr) {
            node = node->left;
            while (node->right != nullptr) {
                node = node->right;
            }
        } else {
            NodeBase *parent = node->parent;
            while (node == parent->left) {
                node = parent;
                parent = parent->parent;
            }
            node = parent;
        }
        return node;
    }

    /**
     * \lang english
     * @brief Rotates the subtree at @p node to the left.
     *
     * @param root The tree root, updated in place when the rotation changes it.
     * @param node The subtree root to rotate.
     *
     * \lang simp-chinese
     * @brief 将以 @p node 为根的子树向左旋转。
     *
     * @param root 树根；旋转改变时原地更新。
     * @param node 要旋转的子树根。
     */
    inline void rb_rotate_left(rb_node_base *&root, rb_node_base *node) noexcept {
        rb_node_base *const right = node->right;
        node->right = right->left;
        if (right->left != nullptr) {
            right->left->parent = node;
        }
        right->parent = node->parent;
        if (node == root) {
            root = right;
        } else if (node == node->parent->left) {
            node->parent->left = right;
        } else {
            node->parent->right = right;
        }
        right->left = node;
        node->parent = right;
    }

    /**
     * \lang english
     * @brief Rotates the subtree at @p node to the right.
     *
     * @param root The tree root, updated in place when the rotation changes it.
     * @param node The subtree root to rotate.
     *
     * \lang simp-chinese
     * @brief 将以 @p node 为根的子树向右旋转。
     *
     * @param root 树根；旋转改变时原地更新。
     * @param node 要旋转的子树根。
     */
    inline void rb_rotate_right(rb_node_base *&root, rb_node_base *node) noexcept {
        rb_node_base *const left = node->left;
        node->left = left->right;
        if (left->right != nullptr) {
            left->right->parent = node;
        }
        left->parent = node->parent;
        if (node == root) {
            root = left;
        } else if (node == node->parent->right) {
            node->parent->right = left;
        } else {
            node->parent->left = left;
        }
        left->right = node;
        node->parent = left;
    }

    /**
     * \lang english
     * @brief Restores the red-black invariants after inserting @p node.
     *
     * @param root The tree root, updated in place when rotations change it.
     * @param node The freshly inserted node.
     *
     * \lang simp-chinese
     * @brief 在插入 @p node 之后恢复红黑不变量。
     *
     * @param root 树根；旋转改变时原地更新。
     * @param node 新插入的节点。
     */
    inline void rb_rebalance_for_insert(rb_node_base *&root, rb_node_base *node) noexcept {
        node->color = rb_color::red;
        while (node != root && node->parent->color == rb_color::red) {
            rb_node_base *const grand = node->parent->parent;
            if (node->parent == grand->left) {
                rb_node_base *const uncle = grand->right;
                if (uncle != nullptr && uncle->color == rb_color::red) {
                    node->parent->color = rb_color::black;
                    uncle->color = rb_color::black;
                    grand->color = rb_color::red;
                    node = grand;
                } else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        rb_rotate_left(root, node);
                    }
                    node->parent->color = rb_color::black;
                    node->parent->parent->color = rb_color::red;
                    rb_rotate_right(root, node->parent->parent);
                }
            } else {
                rb_node_base *const uncle = grand->left;
                if (uncle != nullptr && uncle->color == rb_color::red) {
                    node->parent->color = rb_color::black;
                    uncle->color = rb_color::black;
                    grand->color = rb_color::red;
                    node = grand;
                } else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        rb_rotate_right(root, node);
                    }
                    node->parent->color = rb_color::black;
                    node->parent->parent->color = rb_color::red;
                    rb_rotate_left(root, node->parent->parent);
                }
            }
        }
        root->color = rb_color::black;
    }

    /**
     * \lang english
     * @brief Restores the red-black invariants after a black node was unlinked.
     *
     * @param root The tree root, updated in place when rotations change it.
     * @param node The doubly-black subtree root, or nullptr.
     * @param parent The parent to use while @p node is nullptr.
     *
     * \lang simp-chinese
     * @brief 在黑色节点被摘除之后恢复红黑不变量。
     *
     * @param root 树根；旋转改变时原地更新。
     * @param node 双黑子树根，或 nullptr。
     * @param parent 当 @p node 为 nullptr 时使用的父节点。
     */
    inline void rb_erase_fixup(rb_node_base *&root, rb_node_base *node, rb_node_base *parent) noexcept {
        while (node != root && (node == nullptr || node->color == rb_color::black)) {
            if (node == parent->left) {
                rb_node_base *sibling = parent->right;
                if (sibling->color == rb_color::red) {
                    sibling->color = rb_color::black;
                    parent->color = rb_color::red;
                    rb_rotate_left(root, parent);
                    sibling = parent->right;
                }
                if ((sibling->left == nullptr || sibling->left->color == rb_color::black) &&
                    (sibling->right == nullptr || sibling->right->color == rb_color::black)) {
                    sibling->color = rb_color::red;
                    node = parent;
                    parent = parent->parent;
                } else {
                    if (sibling->right == nullptr || sibling->right->color == rb_color::black) {
                        if (sibling->left != nullptr) {
                            sibling->left->color = rb_color::black;
                        }
                        sibling->color = rb_color::red;
                        rb_rotate_right(root, sibling);
                        sibling = parent->right;
                    }
                    sibling->color = parent->color;
                    parent->color = rb_color::black;
                    if (sibling->right != nullptr) {
                        sibling->right->color = rb_color::black;
                    }
                    rb_rotate_left(root, parent);
                    break;
                }
            } else {
                rb_node_base *sibling = parent->left;
                if (sibling->color == rb_color::red) {
                    sibling->color = rb_color::black;
                    parent->color = rb_color::red;
                    rb_rotate_right(root, parent);
                    sibling = parent->left;
                }
                if ((sibling->right == nullptr || sibling->right->color == rb_color::black) &&
                    (sibling->left == nullptr || sibling->left->color == rb_color::black)) {
                    sibling->color = rb_color::red;
                    node = parent;
                    parent = parent->parent;
                } else {
                    if (sibling->left == nullptr || sibling->left->color == rb_color::black) {
                        if (sibling->right != nullptr) {
                            sibling->right->color = rb_color::black;
                        }
                        sibling->color = rb_color::red;
                        rb_rotate_left(root, sibling);
                        sibling = parent->left;
                    }
                    sibling->color = parent->color;
                    parent->color = rb_color::black;
                    if (sibling->left != nullptr) {
                        sibling->left->color = rb_color::black;
                    }
                    rb_rotate_right(root, parent);
                    break;
                }
            }
        }
        if (node != nullptr) {
            node->color = rb_color::black;
        }
    }

    /**
     * \lang english
     * @brief Unlinks @p node from the tree and restores the red-black invariants.
     *
     * @param header The tree header.
     * @param node The node to unlink.
     * @return The node whose physical storage the caller must release.
     *
     * \lang simp-chinese
     * @brief 将 @p node 从树上摘除并恢复红黑不变量。
     *
     * @param header 树头部。
     * @param node 要摘除的节点。
     * @return 调用者需要释放其物理存储的节点。
     */
    inline rb_node_base *rb_rebalance_for_erase(rb_node_base *header, rb_node_base *node) noexcept {
        rb_node_base *root = header->parent;
        rb_node_base *target = node;
        rb_node_base *replacement = nullptr;
        rb_node_base *replacement_parent = nullptr;

        if (target->left == nullptr) {
            replacement = target->right;
        } else if (target->right == nullptr) {
            replacement = target->left;
        } else {
            target = target->right;
            while (target->left != nullptr) {
                target = target->left;
            }
            replacement = target->right;
        }

        if (target != node) {
            node->left->parent = target;
            target->left = node->left;
            if (target != node->right) {
                replacement_parent = target->parent;
                if (replacement != nullptr) {
                    replacement->parent = target->parent;
                }
                target->parent->left = replacement;
                target->right = node->right;
                node->right->parent = target;
            } else {
                replacement_parent = target;
            }
            if (root == node) {
                root = target;
            } else if (node->parent->left == node) {
                node->parent->left = target;
            } else {
                node->parent->right = target;
            }
            target->parent = node->parent;
            const rb_color saved = target->color;
            target->color = node->color;
            node->color = saved;
            target = node;
        } else {
            replacement_parent = target->parent;
            if (replacement != nullptr) {
                replacement->parent = target->parent;
            }
            if (root == node) {
                root = replacement;
            } else if (node->parent->left == node) {
                node->parent->left = replacement;
            } else {
                node->parent->right = replacement;
            }
        }

        if (target->color != rb_color::red) {
            rb_erase_fixup(root, replacement, replacement_parent);
        }

        header->parent = root;
        header->left = root == nullptr ? header : root->minimum();
        header->right = root == nullptr ? header : root->maximum();
        return target;
    }
}

#endif
