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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_NODE_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_NODE_HPP

#include <cstddef>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::collections::implements {
    /**
     * \lang english
     * @brief A single flat-hash node: chain link plus stored element.
     *
     * @tparam Stored The element type stored in the node.
     *
     * \lang simp-chinese
     * @brief 单个扁平哈希节点：链指针加存储元素。
     *
     * @tparam Stored 节点所存储的元素类型。
     */
    template <typename Stored>
    struct dense_node {
        using stored_type = Stored;
        using size_type = std::size_t;

        /**
         * \lang english
         * @brief Sentinel meaning "no next node".
         *
         * \lang simp-chinese
         * @brief 表示「无下一节点」的哨兵值。
         */
        static constexpr size_type npos = static_cast<size_type>(-1);

        /**
         * \lang english
         * @brief Constructs the node in place from forwarded arguments.
         *
         * @tparam Args The constructor argument types.
         * @param link The next-node index for this node's chain.
         * @param args Arguments forwarded to the stored element.
         *
         * \lang simp-chinese
         * @brief 以转发参数就地构造节点。
         *
         * @tparam Args 构造函数参数类型。
         * @param link 该节点所在链的下一节点下标。
         * @param args 转发给所存元素的构造参数。
         */
        template <typename... Args>
        RAINY_CONSTEXPR20 explicit dense_node(const size_type link, Args &&...args) :
            next(link), alive(true), elem(utility::forward<Args>(args)...) {
        }

        RAINY_CONSTEXPR20 dense_node(const dense_node &) = default;

        RAINY_CONSTEXPR20 dense_node(dense_node &&) = default;

        RAINY_CONSTEXPR20 dense_node &operator=(const dense_node &) = default;

        RAINY_CONSTEXPR20 dense_node &operator=(dense_node &&) = default;

        RAINY_CONSTEXPR20 ~dense_node() = default;

        size_type next;
        bool alive;
        Stored elem;
    };
}

#endif
