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
#ifndef RAINY_FOUNDATION_WILLOW_INI_HPP
#define RAINY_FOUNDATION_WILLOW_INI_HPP
#include <cassert>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/ini/lexer.hpp>
#include <rainy/foundation/willow/implements/ini/parser.hpp>
#include <rainy/foundation/willow/implements/ini/serializer.hpp>

namespace rainy::foundation::willow::ini {
    using willow::basic_document;
    using willow::document;
    using willow::document_type;
    using willow::serializer_args;
}

namespace rainy::foundation::willow::ini::implements {
    template <typename BasicDocument, typename Adapter>
    facade<BasicDocument> parse_document(Adapter adapter) {
        BasicDocument result;
        ini_parser<BasicDocument, Adapter>(adapter).parse(result);
        return facade<BasicDocument>(utility::move(result));
    }
}

namespace rainy::foundation::willow::ini {
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
    typename BasicDocument::string_type dump(const BasicDocument &doc,
                                             const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        typename BasicDocument::string_type result{};
        string_output_adapter<typename BasicDocument::string_type> adapter(result);
        implements::ini_serializer<BasicDocument>(&adapter, args).dump(doc);
        return result;
    }

    template <typename BasicDocument>
    void dump(const BasicDocument &doc, output_adapter<typename BasicDocument::char_type> adapter,
              const serializer_args<BasicDocument> &args = serializer_args<BasicDocument>{}) {
        implements::ini_serializer<BasicDocument>(utility::move(adapter), args).dump(doc);
    }

    RAINY_INLINE facade<document> operator""_ini(const char *str, std::size_t) {
        return ini::parse(str);
    }

    RAINY_INLINE facade<document64> operator""_ini64(const char *str, std::size_t) {
        return ini::parse<document64>(str);
    }

    RAINY_INLINE facade<wdocument> operator""_wini(const wchar_t *str, std::size_t) {
        return ini::parse<wdocument>(str);
    }

    RAINY_INLINE facade<wdocument64> operator""_wini64(const wchar_t *str, std::size_t) {
        return ini::parse<wdocument64>(str);
    }
}

#endif
