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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_UNKNOWN_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_UNKNOWN_HPP
#include <cstddef>
#include <cstdint>
#include <rainy/core/collections/vector.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf {
    class unknown_field {
    public:
        unknown_field() noexcept : number_{0}, wire_{wire_type::varint}, payload_{} {
        }
        unknown_field(std::uint32_t number, wire_type wire, byte_buffer payload) noexcept :
            number_{number}, wire_{wire}, payload_{static_cast<byte_buffer &&>(payload)} {
        }

        RAINY_NODISCARD std::uint32_t number() const noexcept {
            return number_;
        }

        RAINY_NODISCARD wire_type wire() const noexcept {
            return wire_;
        }

        RAINY_NODISCARD const byte_buffer &as_bytes() const noexcept {
            return payload_;
        }

        RAINY_NODISCARD std::uint64_t as_varint() const {
            if (wire_ != wire_type::varint) {
                exceptions::willow::protobuf::throw_protobuf_type_error("unknown field is not a varint");
            }
            std::uint64_t value = 0;
            unsigned shift = 0;
            for (std::size_t i = 0; i < payload_.size(); ++i) {
                std::uint8_t byte = payload_[i];
                if (shift < 64) {
                    value |= static_cast<std::uint64_t>(byte & 0x7Fu) << shift;
                }
                if ((byte & 0x80u) == 0) {
                    return value;
                }
                shift += 7;
            }
            exceptions::willow::protobuf::throw_protobuf_parse_error("malformed varint in unknown field");
            return 0;
        }

    private:
        std::uint32_t number_;
        wire_type wire_;
        byte_buffer payload_;
    };

    class unknown_field_set {
    public:
        using value_type = unknown_field;
        using size_type = std::size_t;
        using iterator = core::collections::vector<unknown_field>::iterator;
        using const_iterator = core::collections::vector<unknown_field>::const_iterator;

        unknown_field_set() = default;

        bool empty() const noexcept {
            return storage_.empty();
        }
        size_type size() const noexcept {
            return storage_.size();
        }

        const_iterator begin() const noexcept {
            return storage_.begin();
        }
        const_iterator end() const noexcept {
            return storage_.end();
        }

        const unknown_field &operator[](size_type index) const {
            return storage_[index];
        }

        const unknown_field *find(std::uint32_t number) const noexcept {
            for (const auto &f: storage_) {
                if (f.number() == number) {
                    return &f;
                }
            }
            return nullptr;
        }

        void append(std::uint32_t number, wire_type wire, const std::uint8_t *data, std::size_t size) {
            byte_buffer payload{};
            payload.reserve(size);
            payload.insert(payload.end(), data, data + size);
            storage_.emplace_back(number, wire, static_cast<byte_buffer &&>(payload));
        }

        void remove(std::uint32_t number) {
            for (auto it = storage_.begin(); it != storage_.end(); ++it) {
                if (it->number() == number) {
                    storage_.erase(it);
                    return;
                }
            }
        }

        void clear() noexcept {
            storage_.clear();
        }

    private:
        core::collections::vector<unknown_field> storage_;
    };

    template <typename Concept>
    class add_unknown_fields {
    public:
        unknown_field_set &unknown() noexcept {
            return unknown_storage_;
        }

        RAINY_NODISCARD const unknown_field_set &unknown() const noexcept {
            return unknown_storage_;
        }

    protected:
        unknown_field_set unknown_storage_;
    };
}

#endif

#endif
