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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_DYNAMIC_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_DYNAMIC_HPP
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf {
    enum class field_kind : std::uint8_t {
        none = 0,
        int32 = 1,
        int64 = 2,
        uint32 = 3,
        uint64 = 4,
        sint32 = 5,
        sint64 = 6,
        boolean = 7,
        fixed32 = 8,
        sfixed32 = 9,
        floating = 10,
        fixed64 = 11,
        sfixed64 = 12,
        doubling = 13,
        string = 14,
        bytes = 15,
        message = 16,
    };

    template <typename Kind>
    struct kind_of;

    template <>
    struct kind_of<proto::int32> : type_traits::helper::integral_constant<field_kind, field_kind::int32> {};
    template <>
    struct kind_of<proto::int64> : type_traits::helper::integral_constant<field_kind, field_kind::int64> {};
    template <>
    struct kind_of<proto::uint32> : type_traits::helper::integral_constant<field_kind, field_kind::uint32> {};
    template <>
    struct kind_of<proto::uint64> : type_traits::helper::integral_constant<field_kind, field_kind::uint64> {};
    template <>
    struct kind_of<proto::sint32> : type_traits::helper::integral_constant<field_kind, field_kind::sint32> {};
    template <>
    struct kind_of<proto::sint64> : type_traits::helper::integral_constant<field_kind, field_kind::sint64> {};
    template <>
    struct kind_of<proto::boolean> : type_traits::helper::integral_constant<field_kind, field_kind::boolean> {};
    template <>
    struct kind_of<proto::fixed32> : type_traits::helper::integral_constant<field_kind, field_kind::fixed32> {};
    template <>
    struct kind_of<proto::sfixed32> : type_traits::helper::integral_constant<field_kind, field_kind::sfixed32> {};
    template <>
    struct kind_of<proto::floating> : type_traits::helper::integral_constant<field_kind, field_kind::floating> {};
    template <>
    struct kind_of<proto::fixed64> : type_traits::helper::integral_constant<field_kind, field_kind::fixed64> {};
    template <>
    struct kind_of<proto::sfixed64> : type_traits::helper::integral_constant<field_kind, field_kind::sfixed64> {};
    template <>
    struct kind_of<proto::doubling> : type_traits::helper::integral_constant<field_kind, field_kind::doubling> {};
    template <>
    struct kind_of<proto::string> : type_traits::helper::integral_constant<field_kind, field_kind::string> {};
    template <>
    struct kind_of<proto::bytes> : type_traits::helper::integral_constant<field_kind, field_kind::bytes> {};

    template <typename Concept>
    struct kind_of<proto::message<Concept>> : type_traits::helper::integral_constant<field_kind, field_kind::message> {};
    template <typename Element>
    struct kind_of<proto::repeated<Element>> : kind_of<Element> {};

    template <typename Kind>
    inline constexpr field_kind kind_of_v = kind_of<Kind>::value;

    enum class dynamic_kind : std::uint8_t {
        null = 0,
        int64 = 1,
        uint64 = 2,
        doubling = 3,
        boolean = 4,
        string = 5,
        bytes = 6,
        message = 7,
        repeated = 8,
    };

    struct dynamic_bytes {
        const std::uint8_t *data{nullptr};
        std::size_t size{0};
    };

    class dynamic_value {
    public:
        dynamic_value() noexcept : kind_(dynamic_kind::null), storage_{} {
        }

        static dynamic_value of_int64(std::int64_t value) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::int64;
            v.storage_.i64 = value;
            return v;
        }
        static dynamic_value of_uint64(std::uint64_t value) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::uint64;
            v.storage_.u64 = value;
            return v;
        }
        static dynamic_value of_double(double value) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::doubling;
            v.storage_.d = value;
            return v;
        }
        static dynamic_value of_boolean(bool value) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::boolean;
            v.storage_.b = value;
            return v;
        }
        static dynamic_value of_string(core::text::string_view value) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::string;
            v.storage_.sv = value;
            return v;
        }
        static dynamic_value of_bytes(const std::uint8_t *data, std::size_t size) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::bytes;
            v.storage_.bytes.data = data;
            v.storage_.bytes.size = size;
            return v;
        }
        static dynamic_value of_message(const void *ptr) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::message;
            v.storage_.ptr = ptr;
            return v;
        }
        static dynamic_value of_repeated(const void *ptr) noexcept {
            dynamic_value v;
            v.kind_ = dynamic_kind::repeated;
            v.storage_.ptr = ptr;
            return v;
        }

        dynamic_kind kind() const noexcept {
            return kind_;
        }
        bool is_null() const noexcept {
            return kind_ == dynamic_kind::null;
        }

        std::int64_t as_int64() const {
            enforce_kind(dynamic_kind::int64);
            return storage_.i64;
        }
        std::uint64_t as_uint64() const {
            enforce_kind(dynamic_kind::uint64);
            return storage_.u64;
        }
        double as_double() const {
            enforce_kind(dynamic_kind::doubling);
            return storage_.d;
        }
        bool as_boolean() const {
            enforce_kind(dynamic_kind::boolean);
            return storage_.b;
        }
        core::text::string_view as_string() const {
            if (kind_ == dynamic_kind::bytes) {
                return {reinterpret_cast<const char *>(storage_.bytes.data), storage_.bytes.size};
            }
            enforce_kind(dynamic_kind::string);
            return storage_.sv;
        }
        dynamic_bytes as_bytes() const {
            enforce_kind(dynamic_kind::bytes);
            return storage_.bytes;
        }
        const void *as_message() const {
            enforce_kind(dynamic_kind::message);
            return storage_.ptr;
        }
        const void *as_repeated() const {
            enforce_kind(dynamic_kind::repeated);
            return storage_.ptr;
        }

    private:
        void enforce_kind(dynamic_kind expected) const {
            if (kind_ != expected) {
                exceptions::willow::protobuf::throw_protobuf_type_error("dynamic_value kind mismatch");
            }
        }

        dynamic_kind kind_;
        union storage {
            std::int64_t i64;
            std::uint64_t u64;
            double d;
            bool b;
            core::text::string_view sv;
            dynamic_bytes bytes;
            const void *ptr;

            storage() noexcept : ptr{nullptr} {
            }
        } storage_;
    };

    template <typename Kind>
    struct is_signed_scalar_kind : type_traits::helper::false_type {};
    template <>
    struct is_signed_scalar_kind<proto::int32> : type_traits::helper::true_type {};
    template <>
    struct is_signed_scalar_kind<proto::int64> : type_traits::helper::true_type {};
    template <>
    struct is_signed_scalar_kind<proto::sint32> : type_traits::helper::true_type {};
    template <>
    struct is_signed_scalar_kind<proto::sint64> : type_traits::helper::true_type {};
    template <>
    struct is_signed_scalar_kind<proto::sfixed32> : type_traits::helper::true_type {};
    template <>
    struct is_signed_scalar_kind<proto::sfixed64> : type_traits::helper::true_type {};

    template <typename Kind>
    inline constexpr bool is_signed_scalar_kind_v = is_signed_scalar_kind<Kind>::value;

    template <typename Kind>
    struct is_unsigned_scalar_kind : type_traits::helper::false_type {};
    template <>
    struct is_unsigned_scalar_kind<proto::uint32> : type_traits::helper::true_type {};
    template <>
    struct is_unsigned_scalar_kind<proto::uint64> : type_traits::helper::true_type {};
    template <>
    struct is_unsigned_scalar_kind<proto::fixed32> : type_traits::helper::true_type {};
    template <>
    struct is_unsigned_scalar_kind<proto::fixed64> : type_traits::helper::true_type {};

    template <typename Kind>
    inline constexpr bool is_unsigned_scalar_kind_v = is_unsigned_scalar_kind<Kind>::value;

    template <typename Kind>
    struct is_floating_scalar_kind : type_traits::helper::false_type {};
    template <>
    struct is_floating_scalar_kind<proto::floating> : type_traits::helper::true_type {};
    template <>
    struct is_floating_scalar_kind<proto::doubling> : type_traits::helper::true_type {};

    template <typename Kind>
    inline constexpr bool is_floating_scalar_kind_v = is_floating_scalar_kind<Kind>::value;
}

#endif

#endif
