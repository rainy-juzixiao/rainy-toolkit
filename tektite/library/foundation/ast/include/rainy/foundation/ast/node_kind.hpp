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
#ifndef RAINY_FOUNDATION_AST_NODE_KIND_HPP
#define RAINY_FOUNDATION_AST_NODE_KIND_HPP
#include <cstdint>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief The generic node kinds shared by every syntax front-end.
     *
     * @brief The range below @c generic is reserved for the generic vocabulary. Front-ends add their
     *        own kinds at or above @c generic so the generic kinds stay comparable across languages.
     *
     * \lang simp-chinese
     * @brief 所有语法前端共用的通用节点种类。
     *
     * @brief 小于 @c generic 的区间保留给通用词汇。前端在 @c generic 及以上添加自己的种类，
     *        以便通用种类可跨语言比较。
     */
    enum class node_kind : std::uint16_t {
        invalid = 0,
        literal,
        identifier,
        unary,
        binary,
        conditional,
        member,
        indexed,
        slice,
        call,
        function,
        object,
        array,
        comprehension,
        local,
        assertion,
        error,
        import_expression,
        import_string,
        import_binary,
        self_reference,
        super_reference,
        dollar,
        sequence,
        generic = 0x4000,
        user = 0x8000,
        max = 0xFFFF,
    };

    /**
     * \lang english
     * @brief Returns a stable, human-readable name for a node kind.
     *
     * @return A null-terminated name; @c "unknown" for kinds outside the generic vocabulary.
     *
     * \lang simp-chinese
     * @brief 返回节点种类的稳定可读名称。
     *
     * @return 以 null 结尾的名称；对通用词汇之外的种类返回 @c "unknown"。
     */
    inline const char *kind_name(const node_kind kind) noexcept {
        switch (kind) {
            case node_kind::invalid:
                return "invalid";
            case node_kind::literal:
                return "literal";
            case node_kind::identifier:
                return "identifier";
            case node_kind::unary:
                return "unary";
            case node_kind::binary:
                return "binary";
            case node_kind::conditional:
                return "conditional";
            case node_kind::member:
                return "member";
            case node_kind::indexed:
                return "indexed";
            case node_kind::slice:
                return "slice";
            case node_kind::call:
                return "call";
            case node_kind::function:
                return "function";
            case node_kind::object:
                return "object";
            case node_kind::array:
                return "array";
            case node_kind::comprehension:
                return "comprehension";
            case node_kind::local:
                return "local";
            case node_kind::assertion:
                return "assertion";
            case node_kind::error:
                return "error";
            case node_kind::import_expression:
                return "import_expression";
            case node_kind::import_string:
                return "import_string";
            case node_kind::import_binary:
                return "import_binary";
            case node_kind::self_reference:
                return "self_reference";
            case node_kind::super_reference:
                return "super_reference";
            case node_kind::dollar:
                return "dollar";
            case node_kind::sequence:
                return "sequence";
            case node_kind::generic:
                return "generic";
            case node_kind::user:
                return "user";
            case node_kind::max:
                return "max";
            default:
                return "unknown";
        }
    }

    /**
     * \lang english
     * @brief Tests whether a kind belongs to the generic vocabulary.
     *
     * \lang simp-chinese
     * @brief 判断某个种类是否属于通用词汇。
     */
    constexpr bool is_generic_kind(const node_kind kind) noexcept {
        return kind != node_kind::invalid && static_cast<std::uint16_t>(kind) < static_cast<std::uint16_t>(node_kind::generic);
    }

    /**
     * \lang english
     * @brief Tests whether a kind was introduced by a front-end specialization.
     *
     * \lang simp-chinese
     * @brief 判断某个种类是否由前端特化引入。
     */
    constexpr bool is_user_kind(const node_kind kind) noexcept {
        return static_cast<std::uint16_t>(kind) >= static_cast<std::uint16_t>(node_kind::user);
    }
}

#endif
