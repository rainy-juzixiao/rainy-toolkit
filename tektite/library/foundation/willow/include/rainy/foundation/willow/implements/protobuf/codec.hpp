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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_CODEC_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_CODEC_HPP
#include <rainy/core/typeinfo.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf::implements {
    using namespace rainy::foundation::exceptions::willow::protobuf;

    using protobuf::byte_buffer;

    inline constexpr std::size_t max_varint_bytes = 10;
    inline constexpr std::uint64_t max_varint_value = 0xFFFFFFFFFFFFFFFFull;
    inline constexpr std::size_t max_message_bytes = static_cast<std::size_t>(2147483648ull);

    inline std::size_t varint_size(std::uint64_t value) noexcept {
        std::size_t size = 1;
        while (value >= 0x80u) {
            value >>= 7;
            ++size;
        }
        return size;
    }

    inline void encode_varint(std::uint64_t value, byte_buffer &out) {
        if (value < 0x80u) {
            out.push_back(static_cast<std::uint8_t>(value));
            return;
        }
        do {
            std::uint8_t byte = static_cast<std::uint8_t>(value & 0x7Full);
            value >>= 7;
            if (value != 0) {
                byte |= 0x80u;
            }
            out.push_back(byte);
        } while (value != 0);
    }

    inline bool decode_varint(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint64_t &value) {
        if (ptr < end && (*ptr & 0x80u) == 0) {
            value = *ptr++;
            return true;
        }
        if (ptr + 1 < end && (*ptr & 0x80u) != 0 && (ptr[1] & 0x80u) == 0) {
            value = (static_cast<std::uint64_t>(*ptr & 0x7Fu)) | (static_cast<std::uint64_t>(ptr[1]) << 7);
            ptr += 2;
            return true;
        }
        value = 0;
        unsigned shift = 0;
        for (std::size_t i = 0; i < max_varint_bytes; ++i) {
            if (ptr >= end) {
                return false;
            }
            std::uint8_t byte = *ptr++;
            if (shift < 64) {
                if (byte & 0x7Fu) {
                    if ((static_cast<std::uint64_t>(byte & 0x7F) << shift >> shift) != static_cast<std::uint64_t>(byte & 0x7F)) {
                        throw_protobuf_parse_error("varint overflows 64 bits");
                    }
                }
                value |= static_cast<std::uint64_t>(byte & 0x7F) << shift;
            } else if ((byte & 0x7F) != 0) {
                throw_protobuf_parse_error("varint overflows 64 bits");
            }
            if ((byte & 0x80) == 0) {
                return true;
            }
            shift += 7;
        }
        throw_protobuf_parse_error("malformed varint exceeds 10 bytes");
        return false;
    }

    inline std::uint32_t encode_zigzag32(std::int32_t value) noexcept {
        return (static_cast<std::uint32_t>(value) << 1) ^ static_cast<std::uint32_t>(value >> 31);
    }

    inline std::uint64_t encode_zigzag64(std::int64_t value) noexcept {
        return (static_cast<std::uint64_t>(value) << 1) ^ static_cast<std::uint64_t>(value >> 63);
    }

    inline std::int32_t decode_zigzag32(std::uint32_t value) noexcept {
        return static_cast<std::int32_t>((value >> 1) ^ (0u - (value & 1u)));
    }

    inline std::int64_t decode_zigzag64(std::uint64_t value) noexcept {
        return static_cast<std::int64_t>((value >> 1) ^ (0ull - (value & 1ull)));
    }

    inline std::uint32_t encode_tag_value(std::uint32_t field_number, wire_type wire) noexcept {
        return (field_number << 3) | static_cast<std::uint32_t>(wire);
    }

    inline std::size_t encode_tag_size(std::uint32_t field_number, wire_type wire) noexcept {
        return varint_size(static_cast<std::uint64_t>(encode_tag_value(field_number, wire)));
    }

    inline void encode_tag(std::uint32_t field_number, wire_type wire, byte_buffer &out) {
        if (field_number == 0) {
            throw_protobuf_serialize_error("field number 0 is invalid");
        }
        std::uint64_t tag = (static_cast<std::uint64_t>(field_number) << 3) | static_cast<std::uint32_t>(wire);
        encode_varint(tag, out);
    }

    inline bool decode_tag(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint32_t &field_number, wire_type &wire) {
        std::uint64_t tag = 0;
        if (!decode_varint(ptr, end, tag)) {
            return false;
        }
        if (tag > 0xFFFFFFFFull) {
            throw_protobuf_parse_error("tag exceeds 32 bits");
        }
        wire = static_cast<wire_type>(tag & 0x7u);
        field_number = static_cast<std::uint32_t>(tag >> 3);
        if (field_number == 0) {
            throw_protobuf_parse_error("field number 0 is invalid");
        }
        return true;
    }

    inline void encode_fixed32(std::uint32_t value, byte_buffer &out) {
        out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
        out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFFu));
        out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFFu));
    }

    inline void encode_fixed64(std::uint64_t value, byte_buffer &out) {
        for (int i = 0; i < 8; ++i) {
            out.push_back(static_cast<std::uint8_t>((value >> (8 * i)) & 0xFFu));
        }
    }

    inline bool decode_fixed32(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint32_t &value) {
        if (static_cast<std::size_t>(end - ptr) < 4) {
            return false;
        }
        value = static_cast<std::uint32_t>(ptr[0]) | (static_cast<std::uint32_t>(ptr[1]) << 8) |
                (static_cast<std::uint32_t>(ptr[2]) << 16) | (static_cast<std::uint32_t>(ptr[3]) << 24);
        ptr += 4;
        return true;
    }

    inline bool decode_fixed64(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint64_t &value) {
        if (static_cast<std::size_t>(end - ptr) < 8) {
            return false;
        }
        value = 0;
        for (int i = 0; i < 8; ++i) {
            value |= static_cast<std::uint64_t>(ptr[i]) << (8 * i);
        }
        ptr += 8;
        return true;
    }

    inline void encode_length(std::size_t size, byte_buffer &out) {
        if (size >= max_message_bytes) {
            throw_protobuf_serialize_error("length prefix exceeds 2GiB");
        }
        encode_varint(static_cast<std::uint64_t>(size), out);
    }

    inline bool decode_length(const std::uint8_t *&ptr, const std::uint8_t *end, std::size_t &size) {
        std::uint64_t value = 0;
        if (!decode_varint(ptr, end, value)) {
            return false;
        }
        if (value >= max_message_bytes) {
            throw_protobuf_parse_error("length prefix exceeds 2GiB");
        }
        size = static_cast<std::size_t>(value);
        return true;
    }

    inline bool skip_field(wire_type wire, const std::uint8_t *&ptr, const std::uint8_t *end) {
        switch (wire) {
            case wire_type::varint: {
                std::uint64_t ignored = 0;
                return decode_varint(ptr, end, ignored);
            }
            case wire_type::i64: {
                if (static_cast<std::size_t>(end - ptr) < 8) {
                    return false;
                }
                ptr += 8;
                return true;
            }
            case wire_type::len: {
                std::size_t size = 0;
                if (!decode_length(ptr, end, size)) {
                    return false;
                }
                if (size > static_cast<std::size_t>(end - ptr)) {
                    return false;
                }
                ptr += size;
                return true;
            }
            case wire_type::sgroup:
            case wire_type::egroup:
                throw_protobuf_parse_error("group wire type is not supported");
            case wire_type::i32: {
                if (static_cast<std::size_t>(end - ptr) < 4) {
                    return false;
                }
                ptr += 4;
                return true;
            }
            default:
                throw_protobuf_parse_error("unknown wire type");
        }
        return false;
    }
}

namespace rainy::foundation::willow::protobuf::implements {
    constexpr std::size_t format_protobuf_message_name(core::text::string_view raw, char *out) noexcept { // NOLINT
        constexpr core::text::string_view elaborated_keywords[] = {"class ", "struct ", "union ", "enum "};

        const auto is_identifier = [](const core::text::string_view segment) {
            if (segment.empty()) {
                return false;
            }
            const auto is_alnum = [](const char ch) {
                return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
            };
            if (const char head = segment[0]; (head < 'a' || head > 'z') && (head < 'A' || head > 'Z') && head != '_') { // NOLINT
                return false;
            }
            for (const char ch: segment) { // NOLINT
                if (!is_alnum(ch)) {
                    return false;
                }
            }
            return true;
        };

        std::size_t index = 0;
        bool advanced = true;
        while (advanced) {
            advanced = false;
            if (raw.substr(index).starts_with("::")) {
                index += 2;
                advanced = true;
                continue;
            }
            for (const auto keyword: elaborated_keywords) {
                if (raw.substr(index).starts_with(keyword)) {
                    index += keyword.size();
                    advanced = true;
                    break;
                }
            }
        }
        std::size_t length = 0;
        bool first = true;
        while (index < raw.size()) {
            const std::size_t next = raw.find("::", index);
            const std::size_t end = next == core::text::string_view::npos ? raw.size() : next;
            const core::text::string_view segment = raw.substr(index, end - index);
            index = end == raw.size() ? raw.size() : end + 2;
            if (!is_identifier(segment)) {
                continue;
            }
            if (!first) {
                if (out != nullptr) {
                    out[length] = '.';
                }
                ++length;
            }
            for (const char ch: segment) {
                if (out != nullptr) {
                    out[length] = ch;
                }
                ++length;
            }
            first = false;
        }
        return length;
    }

    template <typename Concept>
    struct protobuf_message_name {
    private:
        static constexpr core::text::string_view raw_ = core::type_name<Concept>();
        static constexpr std::size_t length_ = format_protobuf_message_name(raw_, nullptr);
        static constexpr core::collections::array<char, length_ == 0 ? 1 : length_> data_ = [] {
            core::collections::array<char, length_ == 0 ? 1 : length_> buffer{};
            format_protobuf_message_name(raw_, buffer.data());
            return buffer;
        }();

    public:
        static constexpr core::text::string_view value() noexcept {
            return {data_.data(), length_};
        }
    };
}

namespace rainy::foundation::willow::protobuf {
    template <typename Concept>
    inline constexpr core::text::string_view protobuf_message_name_v = implements::protobuf_message_name<Concept>::value();
}

#endif

#endif
