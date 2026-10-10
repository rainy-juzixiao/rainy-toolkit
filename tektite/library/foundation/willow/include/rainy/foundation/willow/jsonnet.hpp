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
#ifndef RAINY_FOUNDATION_WILLOW_JSONNET_HPP
#define RAINY_FOUNDATION_WILLOW_JSONNET_HPP
#include <cstdio>
#include <rainy/foundation/ast/ast_dump.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/jsonnet/ast.hpp>
#include <rainy/foundation/willow/implements/jsonnet/config.hpp>
#include <rainy/foundation/willow/implements/jsonnet/evaluator.hpp>
#include <rainy/foundation/willow/implements/jsonnet/lexer.hpp>
#include <rainy/foundation/willow/implements/jsonnet/parser.hpp>
#include <rainy/foundation/willow/implements/jsonnet/resolver.hpp>
#include <rainy/foundation/willow/implements/json/serializer.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::willow::jsonnet::implements {
    inline core::text::string read_all(std::FILE *file) {
        core::text::string buffer;
        char chunk[4096];
        std::size_t count = 0;
        while ((count = std::fread(chunk, 1, sizeof(chunk), file)) > 0) {
            buffer.append(chunk, count);
        }
        return buffer;
    }
}

namespace rainy::foundation::willow::jsonnet {
    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const char *data, const std::size_t size) {
        ast_handle handle = make_ast_handle();
        implements::jsonnet_parser parser(data, size, *handle);
        handle->root = parser.parse();
        facade<BasicDocument> result;
        result.set_ast(utility::move(handle));
        return result;
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const core::text::basic_string<char> &str) {
        return parse<BasicDocument>(str.data(), str.size());
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const core::text::basic_string_view<char> str) {
        return parse<BasicDocument>(str.data(), str.size());
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const char *str) {
        return parse<BasicDocument>(str, core::text::char_traits<char>::length(str));
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(std::FILE *file) {
        const core::text::string source = implements::read_all(file);
        return parse<BasicDocument>(source.data(), source.size());
    }

    /**
     * \lang english
     * @brief Evaluates a parsed jsonnet document into a plain document.
     *
     * @param doc A document produced by @c parse.
     * @param resolver Optional source for @c import, @c importstr and @c importbin.
     *
     * \lang simp-chinese
     * @brief 将解析后的 jsonnet 文档求值为普通文档。
     *
     * @param doc 由 @c parse 产生的文档。
     * @param resolver 可选的 @c import、@c importstr 与 @c importbin 源。
     */
    template <typename BasicDocument>
    BasicDocument evaluate(const jsonnet_document<BasicDocument> &doc, const import_resolver &resolver = {}) {
        implements::evaluator engine(&resolver);
        return engine.manifest<BasicDocument>(engine.evaluate(const_cast<jsonnet_node *>(doc.ast())));
    }

    template <typename BasicDocument>
    BasicDocument evaluate(const import_resolver &resolver, const jsonnet_document<BasicDocument> &doc) {
        return evaluate(doc, resolver);
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const BasicDocument &doc,
                                             const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        typename BasicDocument::string_type result{};
        string_output_adapter<typename BasicDocument::string_type> adapter(result);
        json::implements::json_serializer<BasicDocument>(&adapter, args).dump(doc);
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const jsonnet_document<BasicDocument> &doc,
                                             const import_resolver &resolver = {}) {
        return dump<BasicDocument>(evaluate(doc, resolver));
    }

    template <typename BasicDocument>
    core::text::string dump_ast(const jsonnet_document<BasicDocument> &doc, const ast::dump_options &options = {}) {
        ast::dump_options effective = options;
        effective.name_of = &node_name;
        return ast::dump_to_string<char>(doc.ast(), effective);
    }

    RAINY_INLINE facade<document> operator""_jsonnet(const char *str, std::size_t size) {
        return jsonnet::parse<document>(str, size);
    }
}

#endif

#endif
