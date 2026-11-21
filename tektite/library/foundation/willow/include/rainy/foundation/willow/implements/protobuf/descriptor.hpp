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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_DESCRIPTOR_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_DESCRIPTOR_HPP
#include <rainy/core/memory/nebula_ptr.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/direct.hpp>
#include <rainy/foundation/willow/implements/protobuf/dynamic.hpp>
#include <rainy/foundation/willow/implements/protobuf/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf {
    class field_descriptor;

    class descriptor_base {
    public:
        virtual ~descriptor_base() = default;
        RAINY_NODISCARD virtual core::text::string_view message_name() const = 0;
        RAINY_NODISCARD virtual std::size_t field_count() const = 0;
        RAINY_NODISCARD virtual const field_descriptor &field_at(std::size_t index) const = 0;
    };

    class field_descriptor {
    public:
        field_descriptor() noexcept = default;

        RAINY_NODISCARD core::text::string_view name() const noexcept {
            return name_;
        }

        RAINY_NODISCARD std::uint32_t number() const noexcept {
            return number_;
        }

        RAINY_NODISCARD wire_type wire() const noexcept {
            return wire_;
        }

        RAINY_NODISCARD field_kind kind() const noexcept {
            return kind_;
        }

        RAINY_NODISCARD bool is_repeated() const noexcept {
            return is_repeated_;
        }

        RAINY_NODISCARD bool is_message() const noexcept {
            return is_message_;
        }

        RAINY_NODISCARD field_kind element_kind() const noexcept {
            return element_kind_;
        }

        RAINY_NODISCARD std::size_t index() const noexcept {
            return index_;
        }

        RAINY_NODISCARD const descriptor_base &message_descriptor() const {
            if (!is_message_ || message_descriptor_fn_ == nullptr) {
                exceptions::willow::protobuf::throw_protobuf_type_error("field is not a singular message field");
            }
            return message_descriptor_fn_();
        }

    private:
        friend class descriptor_pool;
        template <typename Concept>
        friend class descriptor;
        template <typename Concept>
        friend class add_descriptor;

        using message_descriptor_fn_t = const descriptor_base &(*) ();

        core::text::string_view name_;
        std::uint32_t number_{0};
        wire_type wire_{wire_type::varint};
        field_kind kind_{field_kind::none};
        bool is_repeated_{false};
        bool is_message_{false};
        field_kind element_kind_{field_kind::none};
        std::size_t index_{0};
        message_descriptor_fn_t message_descriptor_fn_{nullptr};
    };

    struct runtime_field_view {
        core::text::string_view name;
        std::uint32_t number{0};
        wire_type wire{wire_type::varint};
        field_kind kind{field_kind::none};
        bool is_repeated{false};
        bool is_message{false};
        field_kind element_kind{field_kind::none};
    };

    class RAINY_TOOLKIT_API descriptor_pool {
    public:
        static descriptor_pool &instance();

        RAINY_NODISCARD const descriptor_base *find_message(core::text::string_view name) const;

        template <typename Concept>
        RAINY_NODISCARD bool has_message() const {
            return find_message(implements::protobuf_message_name<Concept>::value()) != nullptr;
        }

        RAINY_NODISCARD std::size_t size() const;
        void register_descriptor(const descriptor_base &desc);

    private:
        descriptor_pool();

        struct impl;

        core::memory::nebula_ptr<impl> impl_;
    };

    template <typename Concept>
    class descriptor : public descriptor_base {
    public:
        static descriptor &instance() {
            static descriptor inst;
            descriptor_pool::instance().register_descriptor(inst);
            return inst;
        }

        RAINY_NODISCARD core::text::string_view message_name() const noexcept override {
            return name_;
        }

        RAINY_NODISCARD std::size_t field_count() const noexcept override {
            return fields_.size();
        }

        RAINY_NODISCARD const field_descriptor &field_at(std::size_t index) const noexcept override {
            return fields_[index];
        }

        RAINY_NODISCARD const field_descriptor &field(std::size_t index) const noexcept {
            return fields_[index];
        }

        RAINY_NODISCARD const field_descriptor *find_field_by_name(core::text::string_view name) const noexcept {
            for (const auto &fd: fields_) {
                if (fd.name() == name) {
                    return &fd;
                }
            }
            return nullptr;
        }

        RAINY_NODISCARD const field_descriptor *find_field_by_number(std::uint32_t number) const noexcept {
            for (const auto &fd: fields_) {
                if (fd.number() == number) {
                    return &fd;
                }
            }
            return nullptr;
        }

        RAINY_NODISCARD runtime_field_view runtime_field(std::size_t index) const noexcept {
            const auto &fd = fields_[index];
            return runtime_field_view{fd.name_, fd.number_, fd.wire_, fd.kind_, fd.is_repeated_, fd.is_message_, fd.element_kind_};
        }

    private:
        descriptor() : name_{implements::protobuf_message_name<Concept>::value()}, fields_{make_fields()} {
        }

        static core::collections::array<field_descriptor, field_count_v<Concept>> make_fields() {
            return make_fields_impl(type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
        }

        template <std::size_t... Is>
        static core::collections::array<field_descriptor, sizeof...(Is)> make_fields_impl(type_traits::helper::index_sequence<Is...>) {
            return {make_field<Is>()...};
        }

        template <std::size_t Index>
        static field_descriptor make_field() {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind_type = typename field_type::kind;
            field_descriptor fd{};
            fd.name_ = field_name_of<field_type>();
            fd.number_ = field_type::number;
            fd.wire_ = wire_of_v<kind_type>;
            fd.kind_ = kind_of_v<kind_type>;
            fd.is_repeated_ = is_repeated_kind_v<kind_type>;
            fd.is_message_ = is_message_kind_v<kind_type>;
            if constexpr (is_repeated_kind_v<kind_type>) {
                using element = repeated_element_t<kind_type>;
                fd.element_kind_ = kind_of_v<element>;
            } else {
                fd.element_kind_ = field_kind::none;
            }
            fd.index_ = Index;
            if constexpr (is_message_kind_v<kind_type>) {
                fd.message_descriptor_fn_ = &descriptor::template message_descriptor_thunk<Index>;
            } else {
                fd.message_descriptor_fn_ = nullptr;
            }
            return fd;
        }

        template <std::size_t Index>
        static const descriptor_base &message_descriptor_thunk() {
            using field_type = direct_field_at_t<Concept, Index>;
            using kind_type = typename field_type::kind;
            static_assert(is_message_kind_v<kind_type>, "message_descriptor only valid for message fields");
            using nested_concept = message_concept_t<kind_type>;
            return descriptor<nested_concept>::instance();
        }

        core::text::string_view name_;
        core::collections::array<field_descriptor, field_count_v<Concept>> fields_;
    };

    template <typename Concept>
    class add_descriptor {
    public:
        const protobuf::descriptor<Concept> &descriptor() const noexcept {
            return protobuf::descriptor<Concept>::instance();
        }
    };
}

#endif

#endif
