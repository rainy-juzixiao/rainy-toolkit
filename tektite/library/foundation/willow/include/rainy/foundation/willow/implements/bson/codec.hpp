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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_CODEC_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_CODEC_HPP
#include <rainy/foundation/willow/implements/bson/config.hpp>
#include <rainy/foundation/willow/implements/bson/exceptions.hpp>
#include <rainy/foundation/willow/implements/bson/version.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::willow::bson::implements {
    using namespace rainy::foundation::exceptions::willow::bson;

    inline void push_le32(byte_buffer &out, const std::uint32_t value) {
        out.push_back(static_cast<std::uint8_t>(value & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xffu));
    }

    inline void push_le64(byte_buffer &out, const std::uint64_t value) {
        push_le32(out, static_cast<std::uint32_t>(value & 0xffffffffull));
        push_le32(out, static_cast<std::uint32_t>((value >> 32) & 0xffffffffull));
    }

    inline bool take_u8(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint8_t &value) noexcept {
        if (ptr >= end) {
            return false;
        }
        value = *ptr++;
        return true;
    }

    inline bool take_le32(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint32_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 4) {
            return false;
        }
        value = static_cast<std::uint32_t>(ptr[0]) | (static_cast<std::uint32_t>(ptr[1]) << 8) |
                (static_cast<std::uint32_t>(ptr[2]) << 16) | (static_cast<std::uint32_t>(ptr[3]) << 24);
        ptr += 4;
        return true;
    }

    inline bool take_le64(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint64_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 8) {
            return false;
        }
        value = static_cast<std::uint64_t>(ptr[0]) | (static_cast<std::uint64_t>(ptr[1]) << 8) |
                (static_cast<std::uint64_t>(ptr[2]) << 16) | (static_cast<std::uint64_t>(ptr[3]) << 24) |
                (static_cast<std::uint64_t>(ptr[4]) << 32) | (static_cast<std::uint64_t>(ptr[5]) << 40) |
                (static_cast<std::uint64_t>(ptr[6]) << 48) | (static_cast<std::uint64_t>(ptr[7]) << 56);
        ptr += 8;
        return true;
    }

    /**
     * \lang english
     * @brief Copies the cstring at ptr and advances past its terminator.
     *
     * Bson keys are cstrings: utf-8 bytes followed by a single 0x00, which is also the element terminator, so a
     * key may not contain a 0x00 byte.
     *
     * \lang simp-chinese
     * @brief 复制 ptr 处的 cstring 并越过其终止符。
     *
     * bson 的键是 cstring：utf-8 字节后跟单个 0x00，它同时是元素的终止符，因此键不能含 0x00 字节。
     */
    inline bool take_cstring(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t *&data,
                             std::size_t &size) noexcept {
        const std::uint8_t *const begin = ptr;
        while (ptr < end && *ptr != 0) {
            ++ptr;
        }
        if (ptr >= end) {
            return false;
        }
        data = begin;
        size = static_cast<std::size_t>(ptr - begin);
        ++ptr;
        return true;
    }

    inline void skip_cstring(const std::uint8_t *&ptr, const std::uint8_t *end) {
        const std::uint8_t *data = nullptr;
        std::size_t size = 0;
        if (!take_cstring(ptr, end, data, size)) {
            throw_bson_parse_error("truncated bson cstring");
        }
    }

    inline void require_bytes(const std::uint8_t *&ptr, const std::uint8_t *end, const std::size_t count) {
        if (static_cast<std::size_t>(end - ptr) < count) {
            throw_bson_parse_error("truncated bson value body");
        }
        ptr += count;
    }

    /**
     * \lang english
     * @brief Reads a length prefixed bson string, which is an int32 count including its 0x00 terminator.
     *
     * \lang simp-chinese
     * @brief 读取带长度前缀的 bson 字符串，其 int32 计数包含末尾的 0x00 终止符。
     */
    inline void read_length_prefixed(const std::uint8_t *&ptr, const std::uint8_t *end, const std::uint8_t *&data,
                                     std::size_t &size) {
        std::uint32_t raw = 0;
        if (!take_le32(ptr, end, raw)) {
            throw_bson_parse_error("truncated bson string length");
        }
        const std::int32_t length = static_cast<std::int32_t>(raw);
        if (length < 1) {
            throw_bson_parse_error("a bson string length must be at least one");
        }
        const std::size_t total = static_cast<std::size_t>(length);
        if (static_cast<std::size_t>(end - ptr) < total) {
            throw_bson_parse_error("a bson string runs past the end of the input");
        }
        if (ptr[total - 1] != 0) {
            throw_bson_parse_error("a bson string is not terminated by 0x00");
        }
        data = ptr;
        size = total - 1;
        ptr += total;
    }

    inline void skip_document_body(const std::uint8_t *&ptr, const std::uint8_t *end) {
        std::uint32_t raw = 0;
        if (!take_le32(ptr, end, raw)) {
            throw_bson_parse_error("truncated bson document length");
        }
        const std::int32_t length = static_cast<std::int32_t>(raw);
        if (length < 5) {
            throw_bson_parse_error("a bson document needs at least five bytes");
        }
        if (static_cast<std::size_t>(end - ptr) < static_cast<std::size_t>(length) - 4) {
            throw_bson_parse_error("a bson document runs past the end of the input");
        }
        ptr += static_cast<std::size_t>(length) - 4;
    }

    /**
     * \lang english
     * @brief Advances past the value body of one element without interpreting it.
     *
     * Every one of the twenty one element types is handled, so a reader can step over an element it does not
     * understand, and direct mode can skip a field it does not map.
     *
     * \lang simp-chinese
     * @brief 不解释地越过一个元素的值体。
     *
     * 二十一种元素类型全部处理，因此读取器可以跨过它不理解的元素，direct 模式也可以跳过它未映射的字段。
     */
    inline void skip_element_body(const element_type kind, const std::uint8_t *&ptr, const std::uint8_t *end) {
        switch (kind) {
            case element_type::double_:
            case element_type::datetime:
            case element_type::timestamp:
                require_bytes(ptr, end, 8);
                return;
            case element_type::string_:
            case element_type::code:
            case element_type::symbol: {
                const std::uint8_t *data = nullptr;
                std::size_t size = 0;
                read_length_prefixed(ptr, end, data, size);
                return;
            }
            case element_type::document:
            case element_type::array:
                skip_document_body(ptr, end);
                return;
            case element_type::binary: {
                std::uint32_t raw = 0;
                if (!take_le32(ptr, end, raw)) {
                    throw_bson_parse_error("truncated bson binary length");
                }
                if (static_cast<std::int32_t>(raw) < 0) {
                    throw_bson_parse_error("a bson binary length must not be negative");
                }
                const std::size_t size = static_cast<std::size_t>(raw);
                require_bytes(ptr, end, size + 1);
                return;
            }
            case element_type::undefined:
            case element_type::null_:
            case element_type::max_key:
            case element_type::min_key:
                return;
            case element_type::object_id:
                require_bytes(ptr, end, 12);
                return;
            case element_type::boolean:
                require_bytes(ptr, end, 1);
                return;
            case element_type::decimal128:
                require_bytes(ptr, end, 16);
                return;
            case element_type::regex:
                skip_cstring(ptr, end);
                skip_cstring(ptr, end);
                return;
            case element_type::db_pointer: {
                const std::uint8_t *data = nullptr;
                std::size_t size = 0;
                read_length_prefixed(ptr, end, data, size);
                require_bytes(ptr, end, 12);
                return;
            }
            case element_type::code_with_scope: {
                std::uint32_t raw = 0;
                if (!take_le32(ptr, end, raw)) {
                    throw_bson_parse_error("truncated bson code with scope length");
                }
                const std::int32_t total = static_cast<std::int32_t>(raw);
                if (total < 4) {
                    throw_bson_parse_error("a bson code with scope length is too small");
                }
                require_bytes(ptr, end, static_cast<std::size_t>(total) - 4);
                return;
            }
            case element_type::int32:
                require_bytes(ptr, end, 4);
                return;
            case element_type::int64:
                require_bytes(ptr, end, 8);
                return;
            default:
                throw_bson_parse_error("unknown bson element type");
        }
    }
}

#endif

#endif
