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
#ifndef RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_FWD_HPP
#define RAINY_FOUNDATION_COLLECTIONS_IMPLEMENTS_DENSE_HASH_FWD_HPP

#include <cstddef>
#include <cstdint>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::collections {
    /**
     * \lang english
     * @brief Selects how a flat-hash container removes an element.
     *
     * \lang simp-chinese
     * @brief 选择扁平哈希容器移除元素的方式。
     */
    enum class erase_policy : std::uint8_t {
        /**
         * \lang english
         * @brief Relocates the trailing node into the freed slot; iteration
         *        order stays stable, erase costs O(bucket chain).
         *
         * \lang simp-chinese
         * @brief 将末尾节点搬入空位；迭代顺序稳定，删除代价为 O(桶链)。
         */
        backshift,
        /**
         * \lang english
         * @brief Marks the slot dead in O(1); iteration order is unstable and
         *        dead slots are reclaimed lazily.
         *
         * \lang simp-chinese
         * @brief 以 O(1) 将槽标记为死亡；迭代顺序不稳定，死槽延迟回收。
         */
        tombstone
    };
}

namespace rainy::foundation::collections::implements {
    template <erase_policy Policy, bool IsMap, typename Key, typename Mapped, typename Hash, typename KeyEqual, typename Allocator>
    class dense_table;

    template <typename Stored>
    struct dense_node;

    template <typename NodePtr, typename PublicValue, bool IsMap>
    class dense_table_iterator;

    template <typename NodePtr, typename PublicValue, bool IsMap>
    class dense_local_iterator;
}

#endif
