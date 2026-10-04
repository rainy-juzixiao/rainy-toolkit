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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_REFLECTION_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_REFLECTION_HPP
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/descriptor.hpp>
#include <rainy/foundation/willow/implements/protobuf/direct.hpp>
#include <rainy/foundation/willow/implements/protobuf/dynamic.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf {
    template <typename Concept>
    class reflection {
    public:
#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_GET
        dynamic_value get(const Concept &obj, core::text::string_view name) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_name(name);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection get: unknown field name");
            }
            return get_at(obj, fd->index());
        }

        dynamic_value get(const Concept &obj, std::uint32_t number) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_number(number);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection get: unknown field number");
            }
            return get_at(obj, fd->index());
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_SET
        template <typename T>
        void set(Concept &obj, core::text::string_view name, T &&value) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_name(name);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: unknown field name");
            }
            set_at(obj, fd->index(), static_cast<T &&>(value));
        }

        template <typename T>
        void set(Concept &obj, std::uint32_t number, T &&value) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_number(number);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: unknown field number");
            }
            set_at(obj, fd->index(), static_cast<T &&>(value));
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_HAS
        bool has(const Concept &obj, core::text::string_view name) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_name(name);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection has: unknown field name");
            }
            return !is_default_at(obj, fd->index());
        }

        bool has(const Concept &obj, std::uint32_t number) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_number(number);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection has: unknown field number");
            }
            return !is_default_at(obj, fd->index());
        }

        bool is_default(const Concept &obj, core::text::string_view name) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_name(name);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection is_default: unknown field name");
            }
            return is_default_at(obj, fd->index());
        }

        bool is_default(const Concept &obj, std::uint32_t number) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_number(number);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection is_default: unknown field number");
            }
            return is_default_at(obj, fd->index());
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_CLEAR
        void clear(Concept &obj, core::text::string_view name) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_name(name);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection clear: unknown field name");
            }
            clear_at(obj, fd->index());
        }

        void clear(Concept &obj, std::uint32_t number) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_number(number);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection clear: unknown field number");
            }
            clear_at(obj, fd->index());
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_MUTATE
        class mutate_ref {
        public:
            template <typename T>
            operator T &() const {
                return *static_cast<T *>(ptr_);
            }

        private:
            friend class reflection<Concept>;
            explicit mutate_ref(void *p) noexcept : ptr_(p) {}
            void *ptr_;
        };

        mutate_ref mutate(Concept &obj, core::text::string_view name) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_name(name);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection mutate: unknown field name");
            }
            return mutate_ref{mutate_at(obj, fd->index())};
        }

        mutate_ref mutate(Concept &obj, std::uint32_t number) const {
            const auto *fd = descriptor<Concept>::instance().find_field_by_number(number);
            if (fd == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection mutate: unknown field number");
            }
            return mutate_ref{mutate_at(obj, fd->index())};
        }
#endif

    private:
        template <std::size_t... Is>
        static dynamic_value dispatch_get(const Concept &obj, std::size_t index,
                                          type_traits::helper::index_sequence<Is...>) {
            using fn_t = dynamic_value (*)(const Concept &);
            static constexpr fn_t table[] = {&get_field<Is>...};
            return table[index](obj);
        }

        static dynamic_value get_at(const Concept &obj, std::size_t index) {
            return dispatch_get(obj, index, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

        template <typename T>
        static void set_at(Concept &obj, std::size_t index, T &&value) {
            using DT = type_traits::modifers::remove_cvref_t<T>;
            if constexpr (type_traits::type_relations::is_same_v<DT, bool>) {
                dispatch_set_bool(obj, index, value,
                                  type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            } else if constexpr (type_traits::primary_types::is_integral_v<DT> &&
                                 type_traits::properties::is_signed_v<DT>) {
                dispatch_set_int64(obj, index, static_cast<std::int64_t>(value),
                                   type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            } else if constexpr (type_traits::primary_types::is_integral_v<DT> &&
                                 type_traits::properties::is_unsigned_v<DT>) {
                dispatch_set_uint64(obj, index, static_cast<std::uint64_t>(value),
                                    type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            } else if constexpr (type_traits::primary_types::is_floating_point_v<DT>) {
                dispatch_set_double(obj, index, static_cast<double>(value),
                                    type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            } else if constexpr (type_traits::type_relations::is_convertible_v<DT, core::text::string_view>) {
                dispatch_set_string(obj, index, static_cast<core::text::string_view>(value),
                                    type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            } else {
                static_assert(!type_traits::type_relations::is_same_v<DT, DT>,
                              "reflection set: unsupported value type");
            }
        }

        template <std::size_t... Is>
        static void dispatch_set_int64(Concept &obj, std::size_t index, std::int64_t value,
                                       type_traits::helper::index_sequence<Is...>) {
            using fn_t = void (*)(Concept &, std::int64_t);
            static constexpr fn_t table[] = {&set_int64_field<Is>...};
            table[index](obj, value);
        }

        template <std::size_t... Is>
        static void dispatch_set_uint64(Concept &obj, std::size_t index, std::uint64_t value,
                                        type_traits::helper::index_sequence<Is...>) {
            using fn_t = void (*)(Concept &, std::uint64_t);
            static constexpr fn_t table[] = {&set_uint64_field<Is>...};
            table[index](obj, value);
        }

        template <std::size_t... Is>
        static void dispatch_set_double(Concept &obj, std::size_t index, double value,
                                        type_traits::helper::index_sequence<Is...>) {
            using fn_t = void (*)(Concept &, double);
            static constexpr fn_t table[] = {&set_double_field<Is>...};
            table[index](obj, value);
        }

        template <std::size_t... Is>
        static void dispatch_set_bool(Concept &obj, std::size_t index, bool value,
                                      type_traits::helper::index_sequence<Is...>) {
            using fn_t = void (*)(Concept &, bool);
            static constexpr fn_t table[] = {&set_bool_field<Is>...};
            table[index](obj, value);
        }

        template <std::size_t... Is>
        static void dispatch_set_string(Concept &obj, std::size_t index, core::text::string_view value,
                                       type_traits::helper::index_sequence<Is...>) {
            using fn_t = void (*)(Concept &, core::text::string_view);
            static constexpr fn_t table[] = {&set_string_field<Is>...};
            table[index](obj, value);
        }

        template <std::size_t... Is>
        static bool dispatch_is_default(const Concept &obj, std::size_t index,
                                        type_traits::helper::index_sequence<Is...>) {
            using fn_t = bool (*)(const Concept &);
            static constexpr fn_t table[] = {&is_default_field<Is>...};
            return table[index](obj);
        }

        static bool is_default_at(const Concept &obj, std::size_t index) {
            return dispatch_is_default(obj, index,
                                       type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

        template <std::size_t... Is>
        static void dispatch_clear(Concept &obj, std::size_t index,
                                   type_traits::helper::index_sequence<Is...>) {
            using fn_t = void (*)(Concept &);
            static constexpr fn_t table[] = {&clear_field<Is>...};
            table[index](obj);
        }

        static void clear_at(Concept &obj, std::size_t index) {
            dispatch_clear(obj, index, type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

        template <std::size_t... Is>
        static void *dispatch_mutate(Concept &obj, std::size_t index,
                                     type_traits::helper::index_sequence<Is...>) {
            using fn_t = void *(*)(Concept &);
            static constexpr fn_t table[] = {&mutate_field<Is>...};
            return table[index](obj);
        }

        static void *mutate_at(Concept &obj, std::size_t index) {
            return dispatch_mutate(obj, index,
                                   type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

        template <std::size_t Index>
        static dynamic_value get_field(const Concept &obj) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            const auto &member = obj.*(field_type::member);
            if constexpr (is_repeated_kind_v<kind>) {
                return dynamic_value::of_repeated(static_cast<const void *>(&member));
            } else if constexpr (is_message_kind_v<kind>) {
                return dynamic_value::of_message(static_cast<const void *>(&member));
            } else if constexpr (type_traits::type_relations::is_same_v<kind, proto::string>) {
                return dynamic_value::of_string(core::text::string_view(member.data(), member.size()));
            } else if constexpr (type_traits::type_relations::is_same_v<kind, proto::bytes>) {
                return dynamic_value::of_bytes(reinterpret_cast<const std::uint8_t *>(member.data()), member.size());
            } else if constexpr (type_traits::type_relations::is_same_v<kind, proto::boolean>) {
                return dynamic_value::of_boolean(member);
            } else if constexpr (is_floating_scalar_kind_v<kind>) {
                return dynamic_value::of_double(static_cast<double>(member));
            } else if constexpr (is_signed_scalar_kind_v<kind>) {
                return dynamic_value::of_int64(static_cast<std::int64_t>(member));
            } else if constexpr (is_unsigned_scalar_kind_v<kind>) {
                return dynamic_value::of_uint64(static_cast<std::uint64_t>(member));
            } else {
                return dynamic_value{};
            }
        }

        template <std::size_t Index>
        static void set_int64_field(Concept &obj, std::int64_t value) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            if constexpr (is_signed_scalar_kind_v<kind>) {
                auto &slot = obj.*(field_type::member);
                protobuf::implements::assign_direct_integer(slot, value);
            } else {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: field is not a signed integer");
            }
        }

        template <std::size_t Index>
        static void set_uint64_field(Concept &obj, std::uint64_t value) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            if constexpr (is_unsigned_scalar_kind_v<kind>) {
                auto &slot = obj.*(field_type::member);
                protobuf::implements::assign_direct_uinteger(slot, value);
            } else {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: field is not an unsigned integer");
            }
        }

        template <std::size_t Index>
        static void set_double_field(Concept &obj, double value) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            if constexpr (is_floating_scalar_kind_v<kind>) {
                auto &slot = obj.*(field_type::member);
                using raw = type_traits::modifers::remove_cvref_t<decltype(slot)>;
                slot = static_cast<raw>(value);
            } else {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: field is not a floating-point scalar");
            }
        }

        template <std::size_t Index>
        static void set_bool_field(Concept &obj, bool value) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            if constexpr (type_traits::type_relations::is_same_v<kind, proto::boolean>) {
                auto &slot = obj.*(field_type::member);
                slot = value;
            } else {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: field is not a boolean");
            }
        }

        template <std::size_t Index>
        static void set_string_field(Concept &obj, core::text::string_view value) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            if constexpr (type_traits::type_relations::is_same_v<kind, proto::string> ||
                          type_traits::type_relations::is_same_v<kind, proto::bytes>) {
                auto &slot = obj.*(field_type::member);
                using raw = type_traits::modifers::remove_cvref_t<decltype(slot)>;
                static_assert(direct_assignable_string_v<kind, raw>,
                              "protobuf string/bytes member requires assign(pointer, size)");
                using value_type = typename raw::value_type;
                static_assert(sizeof(value_type) == 1, "protobuf string/bytes member requires 1-byte characters");
                slot.assign(reinterpret_cast<const value_type *>(value.data()), value.size());
            } else {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection set: field is not a string or bytes field");
            }
        }

        template <std::size_t Index>
        static bool is_default_field(const Concept &obj) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            const auto &member = obj.*(field_type::member);
            return protobuf::implements::direct_member_is_default<kind>(member);
        }

        template <std::size_t Index>
        static void clear_field(Concept &obj) {
            using field_type = direct_field_at_t<Concept, Index>;
            auto &slot = obj.*(field_type::member);
            using raw = type_traits::modifers::remove_cvref_t<decltype(slot)>;
            slot = raw{};
        }

        template <std::size_t Index>
        static void *mutate_field(Concept &obj) {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind = typename field_type::kind;
            if constexpr (is_repeated_kind_v<kind> || is_message_kind_v<kind>) {
                auto &slot = obj.*(field_type::member);
                return static_cast<void *>(&slot);
            } else {
                exceptions::willow::protobuf::throw_protobuf_type_error("reflection mutate: field is not a message or repeated field");
            }
        }
    };

    template <typename Concept>
    class add_reflection {
    public:
#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_GET
        RAINY_NODISCARD dynamic_value get(core::text::string_view name) const {
            return reflection<Concept>{}.get(static_cast<const Concept &>(*this), name);
        }

        RAINY_NODISCARD dynamic_value get(std::uint32_t number) const {
            return reflection<Concept>{}.get(static_cast<const Concept &>(*this), number);
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_SET
        template <typename T>
        void set(core::text::string_view name, T &&value) {
            reflection<Concept>{}.set(static_cast<Concept &>(*this), name, static_cast<T &&>(value));
        }
        template <typename T>
        void set(std::uint32_t number, T &&value) {
            reflection<Concept>{}.set(static_cast<Concept &>(*this), number, static_cast<T &&>(value));
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_HAS
        RAINY_NODISCARD bool has(core::text::string_view name) const {
            return reflection<Concept>{}.has(static_cast<const Concept &>(*this), name);
        }

        RAINY_NODISCARD bool has(std::uint32_t number) const {
            return reflection<Concept>{}.has(static_cast<const Concept &>(*this), number);
        }

        RAINY_NODISCARD bool is_default(core::text::string_view name) const {
            return reflection<Concept>{}.is_default(static_cast<const Concept &>(*this), name);
        }

        RAINY_NODISCARD bool is_default(std::uint32_t number) const {
            return reflection<Concept>{}.is_default(static_cast<const Concept &>(*this), number);
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_CLEAR
        void clear(core::text::string_view name) {
            reflection<Concept>{}.clear(static_cast<Concept &>(*this), name);
        }
        void clear(std::uint32_t number) {
            reflection<Concept>{}.clear(static_cast<Concept &>(*this), number);
        }
#endif

#if RAINY_WILLOW_PROTOBUF_HAS_REFLECTION_MUTATE
        typename reflection<Concept>::mutate_ref mutate(core::text::string_view name) {
            return reflection<Concept>{}.mutate(static_cast<Concept &>(*this), name);
        }
        typename reflection<Concept>::mutate_ref mutate(std::uint32_t number) {
            return reflection<Concept>{}.mutate(static_cast<Concept &>(*this), number);
        }
#endif
    };
}

#endif

#endif
