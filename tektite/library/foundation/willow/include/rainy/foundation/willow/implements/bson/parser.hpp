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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_PARSER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_PARSER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/foundation/willow/implements/bson/codec.hpp>
#include <rainy/foundation/willow/implements/bson/config.hpp>
#include <rainy/foundation/willow/implements/bson/exceptions.hpp>
#include <rainy/foundation/willow/implements/bson/version.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::willow::bson::implements {
    using namespace rainy::foundation::exceptions::willow::bson;

    template <typename BasicDocument>
    void assign_bson_string(bson_document<BasicDocument> &slot, const std::uint8_t *data, const std::size_t size) {
        using char_type = typename BasicDocument::char_type;
        using string_type = typename BasicDocument::string_type;
        if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
            string_type str{reinterpret_cast<const char *>(data), size};
            slot = bson_document<BasicDocument>(str);
        }
#if RAINY_HAS_CXX20
        else if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
            string_type str{};
            str.reserve(size);
            for (std::size_t i = 0; i < size; ++i) {
                str.push_back(static_cast<char8_t>(data[i]));
            }
            slot = bson_document<BasicDocument>(str);
        }
#endif
        else {
            throw_bson_parse_error("bson strings require a 1-byte character document");
        }
    }

    template <typename BasicDocument>
    typename BasicDocument::string_type narrow_key_to_string(const char *data, const std::size_t size) {
        using string_type = typename BasicDocument::string_type;
        return willow::implements::convert_document_string<string_type>(core::text::string(data, size));
    }

    /**
     * \lang english
     * @brief The bson reader, turning wire bytes into a document.
     *
     * The seven element types the floor can hold map onto it directly; the remaining fourteen keep their element
     * type byte and their raw value body on the node, so nothing is lost on the way in.
     *
     * @tparam BasicDocument The untagged document type
     *
     * \lang simp-chinese
     * @brief bson 读取器，把线上字节转换为文档。
     *
     * 地板能承载的七种元素类型直接映射；其余十四种在节点上保留其元素类型字节与原始值体，因此读入时无所丢失。
     *
     * @tparam BasicDocument 未打标签的文档类型
     */
    template <typename BasicDocument>
    struct bson_reader {
        using doc_type = bson_document<BasicDocument>;
        using integer_type = typename BasicDocument::integer_type;

        static void decode_into(const std::uint8_t *data, const std::size_t size, doc_type &out) {
            if (data == nullptr && size != 0) {
                throw_bson_parse_error("null input with nonzero size");
            }
            if (size < 5) {
                throw_bson_parse_error("a bson document needs at least five bytes");
            }
            const std::uint8_t *ptr = data;
            const std::uint8_t *const end = data + size;
            decode_document(ptr, end, out);
            if (ptr != end) {
                throw_bson_parse_error("trailing bytes after the bson document");
            }
        }

    private:
        static void assign_integer(doc_type &out, const std::int64_t value) {
            if (value < static_cast<std::int64_t>((utility::numeric_limits<integer_type>::min)()) ||
                value > static_cast<std::int64_t>((utility::numeric_limits<integer_type>::max)())) {
                throw_bson_parse_error("integer value out of document range");
            }
            out = doc_type(static_cast<integer_type>(value));
        }

        static void read_document_length(const std::uint8_t *&ptr, const std::uint8_t *end, std::int32_t &length) {
            std::uint32_t raw = 0;
            if (!take_le32(ptr, end, raw)) {
                throw_bson_parse_error("truncated bson document length");
            }
            length = static_cast<std::int32_t>(raw);
            if (length < 5) {
                throw_bson_parse_error("a bson document needs at least five bytes");
            }
            if (static_cast<std::size_t>(end - ptr) < static_cast<std::size_t>(length - 4)) {
                throw_bson_parse_error("a bson document runs past the end of the input");
            }
        }

        static void decode_document(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out) {
            std::int32_t length = 0;
            read_document_length(ptr, end, length);
            const std::uint8_t *const document_end = ptr + (length - 4);
            out = doc_type(document_type::object);
            auto &object = out.as_object();
            while (ptr < document_end) {
                if (*ptr == 0) {
                    break;
                }
                const std::uint8_t type_byte = *ptr++;
                const std::uint8_t *key_data = nullptr;
                std::size_t key_size = 0;
                if (!take_cstring(ptr, document_end, key_data, key_size)) {
                    throw_bson_parse_error("truncated bson element key");
                }
                typename BasicDocument::string_type key = narrow_key_to_string<BasicDocument>(
                    reinterpret_cast<const char *>(key_data), key_size);
                doc_type value;
                decode_value(static_cast<element_type>(type_byte), ptr, document_end, value);
                object.emplace(utility::move(key), utility::move(value));
            }
            if (ptr >= document_end || *ptr != 0) {
                throw_bson_parse_error("a bson document is missing its terminator");
            }
            ++ptr;
            if (ptr != document_end) {
                throw_bson_parse_error("a bson document length does not match its contents");
            }
        }

        static void decode_array(const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out) {
            std::int32_t length = 0;
            read_document_length(ptr, end, length);
            const std::uint8_t *const document_end = ptr + (length - 4);
            out = doc_type(document_type::array);
            auto &array = out.as_array();
            while (ptr < document_end) {
                if (*ptr == 0) {
                    break;
                }
                const std::uint8_t type_byte = *ptr++;
                const std::uint8_t *key_data = nullptr;
                std::size_t key_size = 0;
                if (!take_cstring(ptr, document_end, key_data, key_size)) {
                    throw_bson_parse_error("truncated bson array index");
                }
                doc_type value;
                decode_value(static_cast<element_type>(type_byte), ptr, document_end, value);
                array.push_back(utility::move(value));
            }
            if (ptr >= document_end || *ptr != 0) {
                throw_bson_parse_error("a bson array is missing its terminator");
            }
            ++ptr;
            if (ptr != document_end) {
                throw_bson_parse_error("a bson array length does not match its contents");
            }
        }

        static void decode_value(const element_type kind, std::uint8_t const *&ptr, const std::uint8_t *end, doc_type &out) {
            switch (kind) {
                case element_type::double_: {
                    std::uint64_t bits = 0;
                    if (!take_le64(ptr, end, bits)) {
                        throw_bson_parse_error("truncated bson double");
                    }
                    double value = 0;
                    core::builtin::copy_memory(&value, &bits, sizeof(double));
                    out = doc_type(static_cast<typename BasicDocument::float_type>(value));
                    return;
                }
                case element_type::string_: {
                    const std::uint8_t *data = nullptr;
                    std::size_t size = 0;
                    read_length_prefixed(ptr, end, data, size);
                    assign_bson_string<BasicDocument>(out, data, size);
                    return;
                }
                case element_type::document:
                    decode_document(ptr, end, out);
                    return;
                case element_type::array:
                    decode_array(ptr, end, out);
                    return;
                case element_type::boolean: {
                    std::uint8_t value = 0;
                    if (!take_u8(ptr, end, value)) {
                        throw_bson_parse_error("truncated bson boolean");
                    }
                    if (value > 1) {
                        throw_bson_parse_error("a bson boolean must be 0 or 1");
                    }
                    out = doc_type(value != 0);
                    return;
                }
                case element_type::null_:
                    out = doc_type(document_type::null);
                    return;
                case element_type::int32: {
                    std::uint32_t raw = 0;
                    if (!take_le32(ptr, end, raw)) {
                        throw_bson_parse_error("truncated bson int32");
                    }
                    assign_integer(out, static_cast<std::int32_t>(raw));
                    return;
                }
                case element_type::int64: {
                    std::uint64_t raw = 0;
                    if (!take_le64(ptr, end, raw)) {
                        throw_bson_parse_error("truncated bson int64");
                    }
                    assign_integer(out, static_cast<std::int64_t>(raw));
                    return;
                }
                default:
                    decode_native(kind, ptr, end, out);
                    return;
            }
        }

        /**
         * \lang english
         * @brief Slices an element body the floor cannot express and keeps it verbatim.
         *
         * \lang simp-chinese
         * @brief 切出地板无法表达的元素体并原样保留。
         */
        static void decode_native(const element_type kind, const std::uint8_t *&ptr, const std::uint8_t *end, doc_type &out) {
            const std::uint8_t *const body_begin = ptr;
            skip_element_body(kind, ptr, end);
            byte_buffer payload{};
            payload.reserve(static_cast<std::size_t>(ptr - body_begin));
            for (const std::uint8_t *cursor = body_begin; cursor != ptr; ++cursor) {
                payload.push_back(*cursor);
            }
            out = doc_type(document_type::null);
            out.set_native_element(kind, utility::move(payload));
        }
    };
}

#endif

#endif
