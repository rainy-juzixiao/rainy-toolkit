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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_CODEC_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_CODEC_HPP
#include <rainy/foundation/willow/implements/msgpack/config.hpp>
#include <rainy/foundation/willow/implements/msgpack/version.hpp>

#if RAINY_WILLOW_MSGPACK_AVAILABLE

namespace rainy::foundation::willow::msgpack::implements {
    inline void push_be16(byte_buffer &out, const std::uint16_t value) {
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xffu));
        out.push_back(static_cast<std::uint8_t>(value & 0xffu));
    }

    inline void push_be32(byte_buffer &out, const std::uint32_t value) {
        out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xffu));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xffu));
        out.push_back(static_cast<std::uint8_t>(value & 0xffu));
    }

    inline void push_be64(byte_buffer &out, const std::uint64_t value) {
        push_be32(out, static_cast<std::uint32_t>((value >> 32) & 0xffffffffull));
        push_be32(out, static_cast<std::uint32_t>(value & 0xffffffffull));
    }

    inline bool take_u8(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint8_t &value) noexcept {
        if (ptr >= end) {
            return false;
        }
        value = *ptr++;
        return true;
    }

    inline bool take_be16(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint16_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 2) {
            return false;
        }
        value = static_cast<std::uint16_t>((static_cast<std::uint16_t>(ptr[0]) << 8) | static_cast<std::uint16_t>(ptr[1]));
        ptr += 2;
        return true;
    }

    inline bool take_be32(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint32_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 4) {
            return false;
        }
        value = (static_cast<std::uint32_t>(ptr[0]) << 24) | (static_cast<std::uint32_t>(ptr[1]) << 16) |
                (static_cast<std::uint32_t>(ptr[2]) << 8) | static_cast<std::uint32_t>(ptr[3]);
        ptr += 4;
        return true;
    }

    inline bool take_be64(const std::uint8_t *&ptr, const std::uint8_t *end, std::uint64_t &value) noexcept {
        if (static_cast<std::size_t>(end - ptr) < 8) {
            return false;
        }
        value = (static_cast<std::uint64_t>(ptr[0]) << 56) | (static_cast<std::uint64_t>(ptr[1]) << 48) |
                (static_cast<std::uint64_t>(ptr[2]) << 40) | (static_cast<std::uint64_t>(ptr[3]) << 32) |
                (static_cast<std::uint64_t>(ptr[4]) << 24) | (static_cast<std::uint64_t>(ptr[5]) << 16) |
                (static_cast<std::uint64_t>(ptr[6]) << 8) | static_cast<std::uint64_t>(ptr[7]);
        ptr += 8;
        return true;
    }

    constexpr std::size_t fixext_length(const std::uint8_t head) noexcept {
        switch (head) {
            case 0xd4:
                return 1;
            case 0xd5:
                return 2;
            case 0xd6:
                return 4;
            case 0xd7:
                return 8;
            case 0xd8:
                return 16;
            default:
                return 0;
        }
    }
}

#endif

#endif
