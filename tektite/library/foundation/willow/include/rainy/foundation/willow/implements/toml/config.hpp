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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_CONFIG_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_CONFIG_HPP
#include <cstdint>
#include <utility>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/toml/exceptions.hpp>

namespace rainy::foundation::willow::toml::implements {
    enum class token_type {
        uninitialized,
        value_string,
        value_integer,
        value_float,
        value_boolean,
        value_datetime,
        key,
        left_bracket,
        right_bracket,
        left_brace,
        right_brace,
        key_separator,
        value_separator,
        key_dot,
        newline,
        end_of_input
    };
}

namespace rainy::foundation::willow::toml {
    /**
     * \lang english
     * @brief The four date time kinds defined by toml.
     *
     * \lang simp-chinese
     * @brief toml 定义的四种日期时间种类。
     */
    enum class datetime_kind {
        offset_date_time,
        local_date_time,
        local_date,
        local_time
    };

    /**
     * \lang english
     * @brief The components of a toml date time.
     *
     * A local time leaves the date fields zero, a local date leaves the time fields zero.
     *
     * \lang simp-chinese
     * @brief toml 日期时间的各字段。
     *
     * 本地时间将日期字段留零，本地日期将时间字段留零。
     */
    struct datetime_fields {
        datetime_kind kind{datetime_kind::local_date_time};
        std::int32_t year{0};
        std::int32_t month{0};
        std::int32_t day{0};
        std::int32_t hour{0};
        std::int32_t minute{0};
        std::int32_t second{0};
        std::int32_t nanosecond{0};
        std::int32_t offset_minutes{0};
        bool has_offset{false};
        bool use_space_separator{false};
        bool use_zulu{false};
    };
}

namespace rainy::foundation::willow::toml {
    struct toml_node_tag {};

    template <typename BasicDocument>
    struct with_toml_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct with_toml_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>> {
        using type =
            basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, toml_node_tag>;
    };

    template <typename BasicDocument>
    struct without_toml_node_tag {
        using type = BasicDocument;
    };

    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    struct without_toml_node_tag<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, toml_node_tag>> {
        using type = basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, void>;
    };

    template <typename BasicDocument>
    struct toml_document;

    template <typename BasicDocument>
    using toml_document_base = typename with_toml_node_tag<BasicDocument>::type;
}

namespace rainy::foundation::willow {
    template <typename BasicDocument>
    struct node_representation<BasicDocument, toml::toml_node_tag> {
        using type = toml::toml_document<typename toml::without_toml_node_tag<BasicDocument>::type>;
    };
}

namespace rainy::foundation::willow::toml {
    using willow::basic_document;
    using willow::document;
    using willow::document64;
    using willow::document_type;
    using willow::from_other_document;
    using willow::from_other_document_t;
    using willow::serializer_args;

    template <typename BasicDocument>
    struct toml_document : public toml_document_base<BasicDocument> {
        using base_type = toml_document_base<BasicDocument>;

        static_assert(type_traits::type_relations::is_same_v<BasicDocument, typename without_toml_node_tag<BasicDocument>::type>,
                      "toml_document requires an untagged basic_document");

        template <typename Ty>
        using allocator_type = typename base_type::template allocator_type<Ty>;

        using size_type = typename base_type::size_type;
        using difference_type = typename base_type::difference_type;
        using char_type = typename base_type::char_type;
        using string_type = typename base_type::string_type;
        using integer_type = typename base_type::integer_type;
        using float_type = typename base_type::float_type;
        using boolean_type = typename base_type::boolean_type;
        using array_type = typename base_type::array_type;
        using object_type = typename base_type::object_type;
        using initializer_list = typename base_type::initializer_list;
        using iterator = typename base_type::iterator;
        using const_iterator = typename base_type::const_iterator;
        using reverse_iterator = typename base_type::reverse_iterator;
        using const_reverse_iterator = typename base_type::const_reverse_iterator;
        using node_type = typename base_type::node_type;

        using base_type::base_type;

        toml_document() = default;

        toml_document(const base_type &right) : base_type(right) {
        }

        toml_document(base_type &&right) : base_type(utility::move(right)) {
        }

        template <typename OtherDocument>
        toml_document(from_other_document_t, const OtherDocument &right) :
            base_type(willow::implements::convert_document<base_type>(right)) {
        }

        toml_document(const toml_document &right) :
            base_type(static_cast<const base_type &>(right)), state_(clone_state(right.state_)) {
        }

        toml_document(toml_document &&right) noexcept :
            base_type(utility::move(static_cast<base_type &>(right))), state_(right.state_) {
            right.state_ = nullptr;
        }

        ~toml_document() {
            release_state();
        }

        toml_document &operator=(const toml_document &right) {
            if (this != &right) {
                base_type::operator=(static_cast<const base_type &>(right));
                release_state();
                state_ = clone_state(right.state_);
            }
            return *this;
        }

        toml_document &operator=(toml_document &&right) noexcept {
            if (this != &right) {
                base_type::operator=(utility::move(static_cast<base_type &>(right)));
                release_state();
                state_ = right.state_;
                right.state_ = nullptr;
            }
            return *this;
        }

        base_type &base() noexcept {
            return *this;
        }

        const base_type &base() const noexcept {
            return *this;
        }

        base_type &as_document() noexcept {
            return *this;
        }

        const base_type &as_document() const noexcept {
            return *this;
        }

        using base_type::array;
        using base_type::as_array;
        using base_type::as_bool;
        using base_type::as_float;
        using base_type::as_integer;
        using base_type::as_object;
        using base_type::as_string;
        using base_type::as_string_view;
        using base_type::begin;
        using base_type::cbegin;
        using base_type::cend;
        using base_type::clear;
        using base_type::contains;
        using base_type::count;
        using base_type::crbegin;
        using base_type::crend;
        using base_type::emplace_back;
        using base_type::empty;
        using base_type::end;
        using base_type::erase;
        using base_type::find;
        using base_type::get;
        using base_type::is_array;
        using base_type::is_bool;
        using base_type::is_float;
        using base_type::is_integer;
        using base_type::is_null;
        using base_type::is_number;
        using base_type::is_object;
        using base_type::is_primitive;
        using base_type::is_string;
        using base_type::object;
        using base_type::push_back;
        using base_type::rbegin;
        using base_type::rend;
        using base_type::size;
        using base_type::type_name;
        using base_type::operator[];

        void swap(toml_document &other) noexcept {
            base_type::swap(static_cast<base_type &>(other));
            std::swap(state_, other.state_);
        }

        toml_document &operator+=(toml_document &&elem) {
            base_type::operator+=(static_cast<base_type &&>(static_cast<base_type &>(elem)));
            return *this;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return static_cast<const base_type &>(*this).template get<Ty>();
        }

        /**
         * \lang english
         * @brief Indicates whether this node holds a date time.
         *
         * \lang simp-chinese
         * @brief 指示该节点是否持有日期时间。
         */
        bool has_datetime() const noexcept {
            return state_ != nullptr && state_->has_datetime;
        }

        /**
         * \lang english
         * @brief Returns the date time components.
         *
         * \lang simp-chinese
         * @brief 返回日期时间的各字段。
         */
        datetime_fields as_datetime() const {
            return state().datetime;
        }

        void set_datetime(const datetime_fields &fields) {
            node_state &s = state();
            s.has_datetime = true;
            s.datetime = fields;
        }

        bool is_explicit_table() const noexcept {
            return state_ != nullptr && state_->explicit_table;
        }

        void mark_explicit_table() {
            state().explicit_table = true;
        }

        bool is_inline_table() const noexcept {
            return state_ != nullptr && state_->inline_table;
        }

        void mark_inline_table() {
            state().inline_table = true;
        }

        bool is_array_table() const noexcept {
            return state_ != nullptr && state_->array_table;
        }

        void mark_array_table() {
            state().array_table = true;
        }

        friend bool operator==(const toml_document &left, const toml_document &right) {
            if (!(static_cast<const base_type &>(left) == static_cast<const base_type &>(right))) {
                return false;
            }
            if (left.has_datetime() != right.has_datetime()) {
                return false;
            }
            if (!left.has_datetime()) {
                return true;
            }
            return left.state_->datetime.kind == right.state_->datetime.kind &&
                   left.state_->datetime.year == right.state_->datetime.year &&
                   left.state_->datetime.month == right.state_->datetime.month &&
                   left.state_->datetime.day == right.state_->datetime.day &&
                   left.state_->datetime.hour == right.state_->datetime.hour &&
                   left.state_->datetime.minute == right.state_->datetime.minute &&
                   left.state_->datetime.second == right.state_->datetime.second &&
                   left.state_->datetime.nanosecond == right.state_->datetime.nanosecond &&
                   left.state_->datetime.offset_minutes == right.state_->datetime.offset_minutes &&
                   left.state_->datetime.has_offset == right.state_->datetime.has_offset;
        }

        friend bool operator!=(const toml_document &left, const toml_document &right) {
            return !(left == right);
        }

        friend bool operator<(const toml_document &left, const toml_document &right) {
            return static_cast<const base_type &>(left) < static_cast<const base_type &>(right);
        }

        friend bool operator<=(const toml_document &left, const toml_document &right) {
            return static_cast<const base_type &>(left) <= static_cast<const base_type &>(right);
        }

        friend bool operator>(const toml_document &left, const toml_document &right) {
            return static_cast<const base_type &>(left) > static_cast<const base_type &>(right);
        }

        friend bool operator>=(const toml_document &left, const toml_document &right) {
            return static_cast<const base_type &>(left) >= static_cast<const base_type &>(right);
        }

    private:
        struct node_state {
            bool has_datetime{false};
            datetime_fields datetime{};
            bool explicit_table{false};
            bool inline_table{false};
            bool array_table{false};
        };

        static node_state *clone_state(const node_state *right) {
            if (right == nullptr) {
                return nullptr;
            }
            return willow::implements::value<base_type>::template create<node_state>(*right);
        }

        void release_state() noexcept {
            if (state_ != nullptr) {
                willow::implements::value<base_type>::template destroy<node_state>(state_);
                state_ = nullptr;
            }
        }

        node_state &state() {
            if (state_ == nullptr) {
                state_ = willow::implements::value<base_type>::template create<node_state>();
            }
            return *state_;
        }

        const node_state &state() const {
            if (state_ == nullptr) {
                exceptions::willow::toml::throw_toml_type_error("node has no date time payload");
            }
            return *state_;
        }

        node_state *state_{nullptr};
    };

    template <typename BasicDocument = document>
    using facade = toml_document<BasicDocument>;

    template <typename Ty, typename = void>
    struct has_toml_datetime : type_traits::helper::false_type {};

    template <typename Ty>
    struct has_toml_datetime<Ty, type_traits::other_trans::void_t<decltype(std::declval<const Ty &>().has_datetime())>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool has_toml_datetime_v = has_toml_datetime<Ty>::value;
}

namespace rainy::foundation::willow {
    template <>
    struct choose_language<language::toml> {
        template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::unordered_map,
                  template <typename Key, typename... Args> typename ArrayType = core::collections::vector,
                  typename StringType = core::text::string, typename IntegerType = std::int32_t,
                  typename FloatingType = double, typename BooleanType = bool,
                  template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
        using type =
            toml::toml_document<basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc>>;
    };
}

#endif
