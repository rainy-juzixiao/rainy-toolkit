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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_SERIALIZER_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/msgpack/codec.hpp>
#include <rainy/foundation/willow/implements/msgpack/config.hpp>
#include <rainy/foundation/willow/implements/msgpack/exceptions.hpp>
#include <rainy/foundation/willow/implements/msgpack/version.hpp>

#if RAINY_WILLOW_MSGPACK_AVAILABLE

namespace rainy::foundation::willow::msgpack::implements {
    using namespace rainy::foundation::exceptions::willow::msgpack;

    template <typename Document>
    struct msgpack_serializer {
        using char_type = typename Document::char_type;
        using string_type = typename Document::string_type;

        static void encode_into(const Document &doc, byte_buffer &out, const msgpack_options &options) {
            encode_value(doc, out, options);
        }

    private:
        static void encode_str(const string_type &str, byte_buffer &out) {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                const std::size_t size = str.size();
                if (size <= 31) {
                    out.push_back(static_cast<std::uint8_t>(0xa0u | size));
                } else if (size <= 0xffu) {
                    out.push_back(0xd9);
                    out.push_back(static_cast<std::uint8_t>(size));
                } else if (size <= 0xffffu) {
                    out.push_back(0xda);
                    push_be16(out, static_cast<std::uint16_t>(size));
                } else if (size <= 0xffffffffull) {
                    out.push_back(0xdb);
                    push_be32(out, static_cast<std::uint32_t>(size));
                } else {
                    throw_msgpack_serialize_error("msgpack string exceeds the 32-bit length limit");
                }
                const auto *data = reinterpret_cast<const std::uint8_t *>(str.data());
                out.insert(out.end(), data, data + size);
            } else {
                throw_msgpack_serialize_error("msgpack strings require a 1-byte character document");
            }
        }

        static void encode_uint(byte_buffer &out, const std::uint64_t value) {
            if (value <= 0x7fu) {
                out.push_back(static_cast<std::uint8_t>(value));
            } else if (value <= 0xffu) {
                out.push_back(0xcc);
                out.push_back(static_cast<std::uint8_t>(value));
            } else if (value <= 0xffffu) {
                out.push_back(0xcd);
                push_be16(out, static_cast<std::uint16_t>(value));
            } else if (value <= 0xffffffffull) {
                out.push_back(0xce);
                push_be32(out, static_cast<std::uint32_t>(value));
            } else {
                out.push_back(0xcf);
                push_be64(out, value);
            }
        }

        static void encode_int(byte_buffer &out, const std::int64_t value) {
            if (value >= 0) {
                encode_uint(out, static_cast<std::uint64_t>(value));
            } else if (value >= -32) {
                out.push_back(static_cast<std::uint8_t>(value));
            } else if (value >= -128) {
                out.push_back(0xd0);
                out.push_back(static_cast<std::uint8_t>(static_cast<std::int8_t>(value)));
            } else if (value >= -32768) {
                out.push_back(0xd1);
                push_be16(out, static_cast<std::uint16_t>(static_cast<std::int16_t>(value)));
            } else if (value >= -2147483648LL) {
                out.push_back(0xd2);
                push_be32(out, static_cast<std::uint32_t>(static_cast<std::int32_t>(value)));
            } else {
                out.push_back(0xd3);
                push_be64(out, static_cast<std::uint64_t>(value));
            }
        }

        static void encode_float(byte_buffer &out, const double value) {
            const float narrowed = static_cast<float>(value);
            if (static_cast<double>(narrowed) == value) {
                std::uint32_t bits = 0;
                core::builtin::copy_memory(&bits, &narrowed, sizeof(float));
                out.push_back(0xca);
                push_be32(out, bits);
            } else {
                std::uint64_t bits = 0;
                core::builtin::copy_memory(&bits, &value, sizeof(double));
                out.push_back(0xcb);
                push_be64(out, bits);
            }
        }

        static void encode_ext(const Document &doc, byte_buffer &out, const msgpack_options &options) {
            const std::int8_t type = doc.ext_type();
            const byte_buffer &data = doc.ext_data();
            if (type == -1) {
                if ((options.features & feature::timestamp) == feature::none) {
                    throw_msgpack_serialize_error("timestamp node encountered but the timestamp feature is disabled");
                }
                encode_timestamp(data, out);
                return;
            }
            if ((options.features & feature::ext) == feature::none) {
                throw_msgpack_serialize_error("extension node encountered but the ext feature is disabled");
            }
            const std::size_t size = data.size();
            switch (size) {
                case 1:
                    out.push_back(0xd4);
                    break;
                case 2:
                    out.push_back(0xd5);
                    break;
                case 4:
                    out.push_back(0xd6);
                    break;
                case 8:
                    out.push_back(0xd7);
                    break;
                case 16:
                    out.push_back(0xd8);
                    break;
                default:
                    if (size <= 0xffu) {
                        out.push_back(0xc7);
                        out.push_back(static_cast<std::uint8_t>(size));
                    } else if (size <= 0xffffu) {
                        out.push_back(0xc8);
                        push_be16(out, static_cast<std::uint16_t>(size));
                    } else if (size <= 0xffffffffull) {
                        out.push_back(0xc9);
                        push_be32(out, static_cast<std::uint32_t>(size));
                    } else {
                        throw_msgpack_serialize_error("msgpack extension exceeds the 32-bit length limit");
                    }
                    break;
            }
            out.push_back(static_cast<std::uint8_t>(type));
            out.insert(out.end(), data.begin(), data.end());
        }

        static void encode_timestamp(const byte_buffer &data, byte_buffer &out) {
            if (data.size() == 4) {
                out.push_back(0xd6);
                out.push_back(0xff);
                out.insert(out.end(), data.begin(), data.end());
                return;
            }
            if (data.size() == 8) {
                out.push_back(0xd7);
                out.push_back(0xff);
                out.insert(out.end(), data.begin(), data.end());
                return;
            }
            if (data.size() == 12) {
                const std::uint32_t nanoseconds =
                    (static_cast<std::uint32_t>(data[0]) << 24) | (static_cast<std::uint32_t>(data[1]) << 16) |
                    (static_cast<std::uint32_t>(data[2]) << 8) | static_cast<std::uint32_t>(data[3]);
                if (nanoseconds > 999999999u) {
                    throw_msgpack_serialize_error("timestamp nanoseconds exceed 999999999");
                }
                out.push_back(0xc7);
                out.push_back(12);
                out.push_back(0xff);
                out.insert(out.end(), data.begin(), data.end());
                return;
            }
            throw_msgpack_serialize_error("invalid timestamp payload length");
        }

        static void encode_value(const Document &doc, byte_buffer &out, const msgpack_options &options) {
            if constexpr (has_msgpack_ext_v<Document>) {
                if (doc.has_ext()) {
                    encode_ext(doc, out, options);
                    return;
                }
            }
            switch (doc.type()) {
                case document_type::null:
                    out.push_back(0xc0);
                    return;
                case document_type::boolean:
                    out.push_back(doc.as_bool() ? 0xc3 : 0xc2);
                    return;
                case document_type::number_integer:
                    encode_int(out, static_cast<std::int64_t>(doc.as_integer()));
                    return;
                case document_type::number_float:
                    encode_float(out, static_cast<double>(doc.as_float()));
                    return;
                case document_type::string:
                    encode_str(doc.as_string(), out);
                    return;
                case document_type::array: {
                    const auto &array = doc.as_array();
                    const std::size_t size = array.size();
                    if (size <= 15) {
                        out.push_back(static_cast<std::uint8_t>(0x90u | size));
                    } else if (size <= 0xffffu) {
                        out.push_back(0xdc);
                        push_be16(out, static_cast<std::uint16_t>(size));
                    } else if (size <= 0xffffffffull) {
                        out.push_back(0xdd);
                        push_be32(out, static_cast<std::uint32_t>(size));
                    } else {
                        throw_msgpack_serialize_error("msgpack array exceeds the 32-bit length limit");
                    }
                    for (const auto &element: array) {
                        encode_value(element, out, options);
                    }
                    return;
                }
                case document_type::object: {
                    const auto &object = doc.as_object();
                    const std::size_t size = object.size();
                    if (size <= 15) {
                        out.push_back(static_cast<std::uint8_t>(0x80u | size));
                    } else if (size <= 0xffffu) {
                        out.push_back(0xde);
                        push_be16(out, static_cast<std::uint16_t>(size));
                    } else if (size <= 0xffffffffull) {
                        out.push_back(0xdf);
                        push_be32(out, static_cast<std::uint32_t>(size));
                    } else {
                        throw_msgpack_serialize_error("msgpack map exceeds the 32-bit length limit");
                    }
                    for (const auto &entry: object) {
                        encode_str(entry.first, out);
                        encode_value(entry.second, out, options);
                    }
                }
                default:
                    break;
            }
        }
    };
}

#endif

#endif
