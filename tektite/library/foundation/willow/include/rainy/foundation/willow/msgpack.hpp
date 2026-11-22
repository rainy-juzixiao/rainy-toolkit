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
#ifndef RAINY_FOUNDATION_WILLOW_MSGPACK_HPP
#define RAINY_FOUNDATION_WILLOW_MSGPACK_HPP
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/msgpack/config.hpp>
#include <rainy/foundation/willow/implements/msgpack/parser.hpp>
#include <rainy/foundation/willow/implements/msgpack/serializer.hpp>

#if RAINY_WILLOW_MSGPACK_AVAILABLE

namespace rainy::foundation::willow::msgpack {
    template <typename BasicDocument>
    void pack(const BasicDocument &doc, byte_buffer &out, const msgpack_options &options = {}) {
        implements::msgpack_serializer<BasicDocument>::encode_into(doc, out, options);
    }

    template <typename BasicDocument>
    void pack(const msgpack_document<BasicDocument> &doc, byte_buffer &out, const msgpack_options &options = {}) {
        implements::msgpack_serializer<msgpack_document<BasicDocument>>::encode_into(doc, out, options);
    }

    template <typename BasicDocument>
    byte_buffer pack(const BasicDocument &doc, const msgpack_options &options = {}) {
        byte_buffer out{};
        pack(doc, out, options);
        return out;
    }

    template <typename BasicDocument>
    byte_buffer pack(const msgpack_document<BasicDocument> &doc, const msgpack_options &options = {}) {
        byte_buffer out{};
        pack(doc, out, options);
        return out;
    }

    template <typename BasicDocument = document>
    void unpack_into(const std::uint8_t *data, const std::size_t size, msgpack_document<BasicDocument> &out,
                     const msgpack_options &options = {}) {
        implements::msgpack_parser<BasicDocument>::decode_into(data, size, out, options);
    }

    template <typename BasicDocument = document>
    void unpack_into(const byte_buffer &bytes, msgpack_document<BasicDocument> &out, const msgpack_options &options = {}) {
        unpack_into(bytes.data(), bytes.size(), out, options);
    }

    template <typename BasicDocument = document>
    msgpack_document<BasicDocument> unpack(const std::uint8_t *data, const std::size_t size,
                                           const msgpack_options &options = {}) {
        msgpack_document<BasicDocument> doc;
        unpack_into<BasicDocument>(data, size, doc, options);
        return doc;
    }

    template <typename BasicDocument = document>
    msgpack_document<BasicDocument> unpack(const byte_buffer &bytes, const msgpack_options &options = {}) {
        return unpack<BasicDocument>(bytes.data(), bytes.size(), options);
    }
}

#endif

#endif
