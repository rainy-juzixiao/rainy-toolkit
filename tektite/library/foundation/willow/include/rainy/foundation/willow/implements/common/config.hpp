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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_CONFIG_HPP
#include <map>
#include <memory_resource>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/collections/unordered_map.hpp>
#include <type_traits>

namespace rainy::foundation::willow::implements {
    inline constexpr std::uint32_t unicode_surrogate_base = 0x10000;
    inline constexpr std::uint32_t unicode_surrogate_lead_begin = 0xD800;
    inline constexpr std::uint32_t unicode_surrogate_lead_end = 0xDBFF;
    inline constexpr std::uint32_t unicode_surrogate_trail_begin = 0xDC00;
    inline constexpr std::uint32_t unicode_surrogate_trail_end = 0xDFFF;
    inline constexpr std::uint32_t unicode_surrogate_bits = 10;
    inline constexpr std::uint32_t unicode_surrogate_max_sur = 0x3FF;

    RAINY_TOOLKIT_API std::uint32_t merge_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) noexcept;
}

namespace rainy::foundation::willow {
    RAINY_TOOLKIT_API std::pmr::memory_resource *set_memory_resource(std::pmr::memory_resource *memres) noexcept;

    RAINY_TOOLKIT_API std::pmr::memory_resource *get_memory_resource() noexcept;

    struct from_other_document_t {};

    inline constexpr from_other_document_t from_other_document{};

    enum class document_type {
        number_integer,
        number_float,
        string,
        array,
        object,
        boolean,
        null,
    };

    enum class language {
        bson,
        cbor,
        hjson,
        ini,
        json,
        json5,
        jsonnet,
        msgpack,
        protobuf,
        toml,
        xml,
        yaml,
    };

    template <language Lang>
    struct choose_language;

#if RAINY_HAS_CXX20
    template <typename Ty>
    inline constexpr bool is_supported_char_v =
        type_traits::type_relations::is_same_v<Ty, char> || type_traits::type_relations::is_same_v<Ty, char8_t> ||
        type_traits::type_relations::is_same_v<Ty, wchar_t> || type_traits::type_relations::is_same_v<Ty, char16_t> ||
        type_traits::type_relations::is_same_v<Ty, char32_t>;
#else
    template <typename Ty>
    inline constexpr bool is_supported_char_v =
        type_traits::type_relations::is_same_v<Ty, char> || type_traits::type_relations::is_same_v<Ty, wchar_t> ||
        type_traits::type_relations::is_same_v<Ty, char16_t> || type_traits::type_relations::is_same_v<Ty, char32_t>;
#endif


    template <typename BasicDocument, typename NodeTag = void>
    struct node_representation {
        using type = BasicDocument;
    };

    template <typename BasicDocument, typename NodeTag = void>
    using node_representation_t = typename node_representation<BasicDocument, NodeTag>::type;

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
              template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
              typename StringType = core::text::string, typename IntegerType = std::int32_t, typename FloatingType = double,
              typename BooleanType = bool, template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator,
              typename NodeTag = void>
    class basic_document;

    using document = basic_document<>;
    using document64 = basic_document<collections::unordered_map, core::collections::vector, core::text::string, std::int64_t>;
    using wdocument = basic_document<collections::unordered_map, core::collections::vector, core::text::wstring>;
    using wdocument64 = basic_document<collections::unordered_map, core::collections::vector, core::text::wstring, std::int64_t>;
    using u16document = basic_document<collections::unordered_map, core::collections::vector, core::text::u16string>;
    using u16document64 = basic_document<collections::unordered_map, core::collections::vector, core::text::u16string, std::int64_t>;

#if RAINY_HAS_CXX20
    using u8document = basic_document<collections::unordered_map, core::collections::vector, core::text::u8string>;
    using u8document64 = basic_document<collections::unordered_map, core::collections::vector, core::text::u8string, std::int64_t>;
#endif

    template <typename BasicDocument>
    struct serializer_args {
        using char_type = typename BasicDocument::char_type;
        using float_type = typename BasicDocument::float_type;

        int precision = utility::numeric_limits<float_type>::digits10 + 1;
        unsigned int indent = 0;
        char_type indent_char = ' ';
        bool escape_unicode = false;
    };
}

#endif
