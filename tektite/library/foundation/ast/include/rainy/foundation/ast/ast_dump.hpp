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
#ifndef RAINY_FOUNDATION_AST_AST_DUMP_HPP
#define RAINY_FOUNDATION_AST_AST_DUMP_HPP
#include <cstddef>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/ast/node.hpp>

namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief Controls how a syntax tree is rendered as text.
     *
     * \lang simp-chinese
     * @brief 控制语法树如何渲染为文本。
     */
    struct dump_options {
        unsigned int indent = 2;
        bool annotations = true;
        const char *(*name_of)(node_kind) = nullptr;
    };
}

namespace rainy::foundation::ast::implements {
    template <typename CharType, typename Sink>
    struct structural_dumper {
        Sink &sink;
        const dump_options &options;

        const char *name_of(const node_kind kind) const {
            return options.name_of != nullptr ? options.name_of(kind) : kind_name(kind);
        }

        void put(const CharType *data, const std::size_t size) {
            if (size != 0) {
                sink(data, size);
            }
        }

        void put_char(const CharType ch) {
            sink(&ch, 1);
        }

        void put_ascii(const char *text) {
            for (const char *p = text; *p != '\0'; ++p) {
                put_char(static_cast<CharType>(*p));
            }
        }

        void put_unsigned(std::uint32_t value) {
            char digits[10];
            std::size_t length = 0;
            do {
                digits[length++] = static_cast<char>('0' + value % 10);
                value /= 10;
            } while (value != 0);
            while (length > 0) {
                put_char(static_cast<CharType>(digits[--length]));
            }
        }

        void write_indent(const unsigned int level) const {
            const std::size_t width = static_cast<std::size_t>(level) * options.indent;
            for (std::size_t i = 0; i < width; ++i) {
                sink(&indent_char, 1);
            }
        }

        void run(const ast_node *node, const unsigned int level) {
            if (node == nullptr) {
                return;
            }
            write_indent(level);
            put_ascii(name_of(node->kind));
            if (options.annotations) {
                put_ascii(" @");
                put_unsigned(node->location.begin.line);
                put_char(static_cast<CharType>(':'));
                put_unsigned(node->location.begin.column);
            }
            put_char(static_cast<CharType>('\n'));
            for (const ast_node *child: node->children) {
                run(child, level + 1);
            }
        }

        CharType indent_char{static_cast<CharType>(' ')};
    };

}
namespace rainy::foundation::ast {
    /**
     * \lang english
     * @brief Renders the structure of a syntax tree into a caller-provided sink.
     *
     * @brief @c sink is invoked with @c (const CharType*, size). Only the node kinds and layout are
     *        emitted; front-ends that need payloads in the dump provide their own renderer.
     *
     * \lang simp-chinese
     * @brief 将语法树的结构渲染到调用方提供的接收器。
     *
     * @brief @c sink 以 @c (const CharType*, size) 被调用。仅输出节点种类与结构；需要把载荷写入
     *        导出的前端自行提供渲染器。
     */
    template <typename CharType, typename Sink>
    void dump(const ast_node *node, Sink &&sink, const dump_options &options = {}) {
        implements::structural_dumper<CharType, Sink> dumper{sink, options};
        dumper.run(node, 0);
    }

    /**
     * \lang english
     * @brief Renders the structure of a syntax tree into a string.
     *
     * \lang simp-chinese
     * @brief 将语法树的结构渲染为字符串。
     */
    template <typename CharType = char>
    core::text::basic_string<CharType> dump_to_string(const ast_node *node, const dump_options &options = {}) {
        core::text::basic_string<CharType> result;
        dump<CharType>(node, [&result](const CharType *data, const std::size_t size) { result.append(data, size); }, options);
        return result;
    }
}

#endif
