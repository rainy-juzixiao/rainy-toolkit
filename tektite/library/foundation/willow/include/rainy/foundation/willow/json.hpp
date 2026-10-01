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
#ifndef RAINY_FOUNDATION_WILLOW_JSON_HPP
#define RAINY_FOUNDATION_WILLOW_JSON_HPP
#include <cassert>
#include <iomanip>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/json/lexer.hpp>
#include <rainy/foundation/willow/implements/json/parser.hpp>
#include <rainy/foundation/willow/implements/json/serializer.hpp>
#include <rainy/foundation/willow/writer.hpp>
#include <vector>

namespace rainy::foundation::willow::json {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::serializer_args;
}

namespace rainy::foundation::willow::json::implements {
    template <typename BasicDocument, typename Adapter>
    facade<BasicDocument> parse_document(Adapter adapter, const typename BasicDocument::char_type *span_data = nullptr,
                                         std::size_t span_size = 0) {
        BasicDocument result;
        json_parser<BasicDocument, Adapter>(adapter, span_data, span_size).parse(result);
        return facade<BasicDocument>(utility::move(result));
    }
}

namespace rainy::foundation::willow::json {
    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const core::text::basic_string<typename BasicDocument::char_type> &str) {
        string_input_adapter<core::text::basic_string<typename BasicDocument::char_type>> adapter(str);
        return implements::parse_document<BasicDocument>(adapter, str.data(), str.size());
    }

    template <typename BasicDocument = document, typename StringTy = typename BasicDocument::string_type,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_same_v<StringTy, typename BasicDocument::string_type>, int> = 0>
    facade<BasicDocument> parse(const StringTy &str) {
        string_input_adapter<StringTy> adapter(str);
        return implements::parse_document<BasicDocument>(adapter, str.data(), str.size());
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(core::text::basic_string_view<typename BasicDocument::char_type> str) {
        string_view_input_adapter<core::text::basic_string_view<typename BasicDocument::char_type>> adapter(str.data());
        return implements::parse_document<BasicDocument>(adapter, str.data(), str.size());
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const typename BasicDocument::char_type *str) {
        buffer_input_adapter<typename BasicDocument::char_type> adapter(str);
        return implements::parse_document<BasicDocument>(
            adapter, str, core::text::char_traits<typename BasicDocument::char_type>::length(str));
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(std::FILE *file) {
        file_input_adapter<typename BasicDocument::char_type> adapter(file);
        return implements::parse_document<BasicDocument>(adapter);
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const BasicDocument &doc,
                                             const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        typename BasicDocument::string_type result{};
        string_output_adapter<typename BasicDocument::string_type> adapter(result);
        implements::json_serializer<BasicDocument>(&adapter, args).dump(doc);
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const BasicDocument &doc, unsigned int indent,
                                             typename BasicDocument::char_type indent_char = ' ', bool escape_unicode = false) {
        serializer_args<BasicDocument> args;
        args.indent = indent;
        args.indent_char = indent_char;
        args.escape_unicode = escape_unicode;
        return dump(doc, args);
    }

    template <typename BasicDocument>
    void dump(const BasicDocument &doc, output_adapter<typename BasicDocument::char_type> adapter,
              const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        implements::json_serializer<BasicDocument>(utility::move(adapter), args).dump(doc);
    }

    RAINY_INLINE facade<document> operator""_json(const char *str, std::size_t) {
        return json::parse(str);
    }

    RAINY_INLINE facade<document64> operator""_json64(const char *str, std::size_t) {
        return json::parse<document64>(str);
    }

    RAINY_INLINE facade<wdocument> operator""_wjson(const wchar_t *str, std::size_t) {
        return json::parse<wdocument>(str);
    }

    RAINY_INLINE facade<wdocument64> operator""_wjson64(const wchar_t *str, std::size_t) {
        return json::parse<wdocument64>(str);
    }
}

#endif
