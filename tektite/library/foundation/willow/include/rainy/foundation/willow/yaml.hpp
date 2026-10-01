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
#ifndef RAINY_FOUNDATION_WILLOW_YAML_HPP
#define RAINY_FOUNDATION_WILLOW_YAML_HPP
#include <cassert>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/yaml/lexer.hpp>
#include <rainy/foundation/willow/implements/yaml/parser.hpp>
#include <rainy/foundation/willow/implements/yaml/serializer.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::yaml {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::serializer_args;
}

namespace rainy::foundation::willow::yaml::implements {
    template <typename BasicDocument, typename Adapter>
    facade<BasicDocument> parse_document(Adapter adapter) {
        facade<BasicDocument> result;
        yaml_parser<BasicDocument, Adapter>(adapter).parse(result);
        return result;
    }
}

namespace rainy::foundation::willow::yaml {
    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const core::text::basic_string<typename BasicDocument::char_type> &str) {
        string_input_adapter<core::text::basic_string<typename BasicDocument::char_type>> adapter(str);
        return implements::parse_document<BasicDocument>(adapter);
    }

    template <typename BasicDocument = document, typename StringTy = typename BasicDocument::string_type,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_same_v<StringTy, typename BasicDocument::string_type>, int> = 0>
    facade<BasicDocument> parse(const StringTy &str) {
        string_input_adapter<StringTy> adapter(str);
        return implements::parse_document<BasicDocument>(adapter);
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(core::text::basic_string_view<typename BasicDocument::char_type> str) {
        string_view_input_adapter<core::text::basic_string_view<typename BasicDocument::char_type>> adapter(str);
        return implements::parse_document<BasicDocument>(adapter);
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(const typename BasicDocument::char_type *str) {
        buffer_input_adapter<typename BasicDocument::char_type> adapter(str);
        return implements::parse_document<BasicDocument>(adapter);
    }

    template <typename BasicDocument = document>
    facade<BasicDocument> parse(std::FILE *file) {
        file_input_adapter<typename BasicDocument::char_type> adapter(file);
        return implements::parse_document<BasicDocument>(adapter);
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const facade<BasicDocument> &doc,
                                             const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        typename BasicDocument::string_type result{};
        string_output_adapter<typename BasicDocument::string_type> adapter(result);
        implements::yaml_serializer<BasicDocument>(&adapter, args).dump(doc);
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const facade<BasicDocument> &doc, unsigned int indent,
                                             typename BasicDocument::char_type indent_char = ' ', bool escape_unicode = false) {
        serializer_args<BasicDocument> args;
        args.indent = indent;
        args.indent_char = indent_char;
        args.escape_unicode = escape_unicode;
        return dump(doc, args);
    }

    template <typename BasicDocument>
    void dump(const facade<BasicDocument> &doc, output_adapter<typename BasicDocument::char_type> adapter,
              const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        implements::yaml_serializer<BasicDocument>(utility::move(adapter), args).dump(doc);
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type dump(const facade<BasicDocument> &doc,
                                             const serializer_args<facade<BasicDocument>> &args) {
        serializer_args<BasicDocument> converted;
        converted.precision = args.precision;
        converted.indent = args.indent;
        converted.indent_char = args.indent_char;
        converted.escape_unicode = args.escape_unicode;
        return dump(doc, converted);
    }

    RAINY_INLINE facade<document> operator""_yaml(const char *str, std::size_t) {
        return yaml::parse(str);
    }

    RAINY_INLINE facade<document64> operator""_yaml64(const char *str, std::size_t) {
        return yaml::parse<document64>(str);
    }

    RAINY_INLINE facade<wdocument> operator""_wyaml(const wchar_t *str, std::size_t) {
        return yaml::parse<wdocument>(str);
    }

    RAINY_INLINE facade<wdocument64> operator""_wyaml64(const wchar_t *str, std::size_t) {
        return yaml::parse<wdocument64>(str);
    }
}

#endif
