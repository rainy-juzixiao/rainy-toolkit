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
#ifndef RAINY_FOUNDATION_WILLOW_DOCUMENT_HPP
#define RAINY_FOUNDATION_WILLOW_DOCUMENT_HPP
#include <rainy/core/text/wstring_convert.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/iterator.hpp>

namespace rainy::foundation::willow {
    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType, typename StringType, typename IntegerType,
              typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc, typename NodeTag>
    class basic_document {
    public:
        friend class document_iterator<basic_document>;
        friend class document_iterator<const basic_document>;
        friend struct implements::value_getter<basic_document>;

        template <typename Ty>
        using allocator_type = Alloc<Ty>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using char_type = typename StringType::value_type;
        static_assert(is_supported_char_v<char_type>,
                      "willow documents only allow char, char8_t, wchar_t, char16_t and char32_t strings");
        using string_type =
            typename type_traits::extras::templates::replace_last_parameter<StringType, allocator_type<char_type>>::type;
        using integer_type = IntegerType;
        using float_type = FloatingType;
        using boolean_type = BooleanType;
        using node_type = node_representation_t<basic_document, NodeTag>;
        using array_type = ArrayType<node_type, allocator_type<node_type>>;
        using object_type = typename type_traits::extras::templates::replace_last_parameter<
            ObjectType<string_type, node_type>, allocator_type<std::pair<const string_type, node_type>>>::type;
        using initializer_list = std::initializer_list<node_type>;
        using iterator = document_iterator<basic_document>;
        using const_iterator = document_iterator<const basic_document>;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        basic_document() noexcept = default;

        node_type &as_node() noexcept {
            return static_cast<node_type &>(*this);
        }

        const node_type &as_node() const noexcept {
            return static_cast<const node_type &>(*this);
        }

        basic_document &as_document() noexcept {
            return *this;
        }

        const basic_document &as_document() const noexcept {
            return *this;
        }

        basic_document(std::nullptr_t) {
        }

        basic_document(const document_type type) : value_(type) {
        }

        basic_document(basic_document const &other) : value_(other.value_) {
        }

        basic_document(basic_document &&other) noexcept : value_(utility::move(other.value_)) {
        }

        basic_document(string_type const &value) : value_(value) {
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<string_type, StringViewLike> &&
                                                            !type_traits::type_relations::is_base_of_v<basic_document, StringViewLike>,
                                                        int> = 0>
        basic_document(const StringViewLike &value) : value_{string_type{value}} {
        }

        basic_document(array_type const &arr) : value_(arr) {
        }

        basic_document(object_type const &object) : value_(object) {
        }

        basic_document(integer_type value) : value_(value) {
        }

        template <typename IntegerUTy,
                  typename type_traits::other_trans::enable_if_t<type_traits::primary_types::is_integral_v<IntegerUTy>, int> = 0>
        basic_document(IntegerUTy value) : value_(static_cast<integer_type>(value)) {
        }

        basic_document(float_type value) : value_(value) {
        }

        template <typename FloatingUTy, typename type_traits::other_trans::enable_if_t<
                                            type_traits::primary_types::is_floating_point_v<FloatingUTy>, int> = 0>
        basic_document(FloatingUTy value) : value_(static_cast<float_type>(value)) {
        }

        basic_document(boolean_type value) : value_(value) {
        }

        basic_document(initializer_list const &init_list, document_type exact_type = document_type::null) {
            bool is_an_object = std::all_of(init_list.begin(), init_list.end(), [](const basic_document &elem) {
                return (elem.is_array() && elem.size() == 2 && elem[0].is_string());
            });
            if (exact_type != document_type::object && exact_type != document_type::array) {
                exact_type = is_an_object ? document_type::object : document_type::array;
            }
            if (exact_type == document_type::object) {
                assert(is_an_object);
                value_ = document_type::object;
                for (const node_type &item: init_list) {
                    object_type &object = (*value_.data.object);
                    array_type &values = (*item.value_.data.vector);
                    object.emplace(values[0].as_string(), values[1]);
                }
            } else {
                value_ = document_type::array;
                auto &vec = (*value_.data.vector);
                vec.reserve(init_list.size());
                vec.assign(init_list.begin(), init_list.end());
            }
        }

        static basic_document object(initializer_list const &init_list) {
            return basic_document(init_list, document_type::object);
        }

        static basic_document array(initializer_list const &init_list) {
            return basic_document(init_list, document_type::array);
        }

        bool is_object() const {
            return value_.type == document_type::object;
        }

        bool is_array() const {
            return value_.type == document_type::array;
        }

        bool is_string() const {
            return value_.type == document_type::string;
        }

        bool is_bool() const {
            return value_.type == document_type::boolean;
        }

        bool is_integer() const {
            return value_.type == document_type::number_integer;
        }

        bool is_float() const {
            return value_.type == document_type::number_float;
        }

        bool is_number() const {
            return is_integer() || is_float();
        }

        bool is_null() const {
            return value_.type == document_type::null;
        }

        bool is_primitive() const {
            switch (value_.type) {
                case document_type::number_integer:
                case document_type::number_float:
                case document_type::string:
                case document_type::boolean:
                case document_type::null:
                    return true;
                default:
                    break;
            }
            return false;
        }

        document_type type() const {
            return value_.type;
        }

        std::string_view type_name() const {
            switch (type()) {
                case document_type::object:
                    return "object";
                case document_type::array:
                    return "array";
                case document_type::string:
                    return "string";
                case document_type::number_integer:
                    return "integer";
                case document_type::number_float:
                    return "float";
                case document_type::boolean:
                    return "boolean";
                case document_type::null:
                    return "null";
            }
            return {};
        }

        void swap(basic_document &rhs) noexcept {
            value_.swap(rhs.value_);
        }

        iterator begin() {
            iterator iter(this);
            iter.set_begin();
            return iter;
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator cbegin() const {
            const_iterator iter(this);
            iter.set_begin();
            return iter;
        }

        iterator end() {
            iterator iter(this);
            iter.set_end();
            return iter;
        }

        const_iterator end() const {
            return cend();
        }

        const_iterator cend() const {
            const_iterator iter(this);
            iter.set_end();
            return iter;
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const {
            return rbegin();
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const {
            return rend();
        }

        size_type size() const {
            switch (type()) {
                case document_type::null:
                    return 0;
                case document_type::array:
                    return value_.data.vector->size();
                case document_type::object:
                    return value_.data.object->size();
                default:
                    return 1;
            }
        }

        bool empty() const {
            if (is_object()) {
                return value_.data.object->empty();
            }
            if (is_array()) {
                return value_.data.vector->empty();
            }
            return is_null();
        }

        bool contains(const string_type &key) const noexcept {
            return find(key) != cend();
        }

        template <typename Key>
        const_iterator find(Key &&key) const {
            if (is_object()) {
                const_iterator iter(this);
                iter.object_it_ = value_.data.object->find(utility::forward<Key>(key));
                return iter;
            }
            return cend();
        }

        const_iterator find(core::text::basic_string_view<char_type> key) const {
            if (is_object()) {
                const_iterator iter(this);
                iter.object_it_ = value_.data.object->find({key.data(), key.size()});
                return iter;
            }
            return cend();
        }

        template <typename Key>
        size_type count(Key &&key) const {
            return is_object() ? value_.data.object->count(utility::forward<Key>(key)) : 0;
        }

        size_type erase(const typename object_type::key_type &key) {
            if (!is_object()) {
                exceptions::willow::throw_willow_invalid_key("cannot use erase() with non-object value");
            }
            return value_.data.object->erase(key);
        }

        void erase(const size_type index) {
            if (!is_array()) {
                exceptions::willow::throw_willow_invalid_key("cannot use erase() with non-array value");
            }
            value_.data.vector->erase(value_.data.vector->begin() + static_cast<difference_type>(index));
        }

        template <typename It,
                  typename type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<It, iterator> ||
                                                                     type_traits::type_relations::is_same_v<It, const_iterator>,
                                                                 int> = 0>
        It erase(It pos) {
            It result = end();

            switch (type()) {
                case document_type::object: {
                    result.it_.object_iter = value_.data.object->erase(pos.it_.object_iter);
                    break;
                }

                case document_type::array: {
                    result.it_.array_iter = value_.data.vector->erase(pos.it_.array_iter);
                    break;
                }

                default:
                    exceptions::willow::throw_willow_invalid_iterator("cannot use erase() with non-object & non-array value");
            }

            return result;
        }

        template <typename It,
                  typename type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<It, iterator> ||
                                                                     type_traits::type_relations::is_same_v<It, const_iterator>,
                                                                 int> = 0>
        It erase(It first, It last) {
            It result = end();
            switch (type()) {
                case document_type::object: {
                    result.it_.object_iter = value_.data.object->erase(first.it_.object_iter, last.it_.object_iter);
                    break;
                }
                case document_type::array: {
                    result.it_.array_iter = value_.data.vector->erase(first.it_.array_iter, last.it_.array_iter);
                    break;
                }
                default:
                    exceptions::willow::throw_willow_invalid_iterator("cannot use erase() with non-object & non-array value");
            }
            return result;
        }

        void push_back(basic_document &&elem) {
            if (!is_null() && !is_array()) {
                exceptions::willow::throw_willow_type_error("cannot use push_back() with non-array value");
            }
            if (is_null()) {
                value_ = document_type::array;
            }
            value_.data.vector->push_back(std::move(elem));
        }

        void push_back(const basic_document &elem) {
            if (!is_null() && !is_array()) {
                exceptions::willow::throw_willow_type_error("cannot use push_back() with non-array value");
            }
            if (is_null()) {
                value_ = document_type::array;
            }
            value_.data.vector->push_back(elem);
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_constructible_v<basic_document, Args...>, int> = 0>
        void emplace_back(Args &&...args) {
            if (!is_null() && !is_array()) {
                exceptions::willow::throw_willow_type_error("cannot use emplace_back() with non-array value");
            }
            if (is_null()) {
                value_ = document_type::array;
            }
            value_.data.vector->emplace_back(utility::forward<Args>(args)...);
        }

        basic_document &operator+=(basic_document &&elem) {
            push_back(utility::move(elem));
            return *this;
        }

        void clear() {
            value_.clear();
        }

        boolean_type as_bool() const {
            if (!is_bool()) {
                exceptions::willow::throw_willow_type_error("document value must be boolean");
            }
            return value_.data.boolean;
        }

        integer_type as_integer() const {
            if (!is_number()) {
                exceptions::willow::throw_willow_type_error("document value must be integer");
            }
            if (is_float()) {
                return static_cast<integer_type>(value_.data.number_float);
            }
            return value_.data.number_integer;
        }

        float_type as_float() const {
            if (!is_number()) {
                exceptions::willow::throw_willow_type_error("document value must be float");
            }
            if (is_integer()) {
                return static_cast<float_type>(value_.data.number_integer);
            }
            return value_.data.number_float;
        }

        array_type &as_array() {
            if (!is_array()) {
                exceptions::willow::throw_willow_type_error("document value must be array");
            }
            return (*value_.data.vector);
        }

        const array_type &as_array() const {
            if (!is_array()) {
                exceptions::willow::throw_willow_type_error("document value must be array");
            }
            return (*value_.data.vector);
        }

        const string_type &as_string() const {
            if (!is_string()) {
                exceptions::willow::throw_willow_type_error("document value must be string");
            }
            return (*value_.data.string);
        }

        object_type &as_object() {
            if (!is_object()) {
                exceptions::willow::throw_willow_type_error("document value must be object");
            }
            return (*value_.data.object);
        }

        core::text::basic_string_view<typename string_type::value_type> as_string_view() const {
            if (!is_string()) {
                exceptions::willow::throw_willow_type_error("document value must be string");
            }
            const string_type &str = *value_.data.string;
            return {str.data(), str.size()};
        }

        const object_type &as_object() const {
            if (!is_object()) {
                exceptions::willow::throw_willow_type_error("document value must be object");
            }
            return (*value_.data.object);
        }

        template <typename Ty>
        Ty get() const {
            Ty value{};
            implements::value_getter<basic_document>::assign(*this, value);
            return value;
        }

        basic_document &operator=(basic_document const &other) {
            value_ = other.value_;
            return *this;
        }

        basic_document &operator=(basic_document &&other) noexcept {
            value_ = utility::move(other.value_);
            return *this;
        }

        node_type &operator[](size_type index) {
            if (is_null()) {
                value_ = document_type::array;
            }
            if (!is_array()) {
                exceptions::willow::throw_willow_invalid_key("operator[] called on a non-array object");
            }
            auto &vec = (*value_.data.vector);
            if (index >= vec.size()) {
                vec.insert(vec.end(), index - vec.size() + 1, basic_document{});
            }
            return vec[index];
        }

        const node_type &operator[](size_type index) const {
            if (!is_array()) {
                exceptions::willow::throw_willow_invalid_key("operator[] called on a non-array type");
            }
            if (index >= (*value_.data.vector).size()) {
                throw std::out_of_range("operator[] index out of range");
            }
            return (*value_.data.vector)[index];
        }

        RAINY_INLINE node_type &operator[](const typename object_type::key_type &key) {
            if (is_null()) {
                value_ = document_type::object;
            }
            if (!is_object()) {
                exceptions::willow::throw_willow_invalid_key("operator[] called on a non-object type");
            }
            return (*value_.data.object)[key];
        }

        RAINY_INLINE node_type &operator[](const typename object_type::key_type &key) const {
            if (!is_object()) {
                exceptions::willow::throw_willow_invalid_key("operator[] called on a non-object object");
            }
            auto iter = value_.data.object->find(key);
            if (iter == value_.data.object->end()) {
                throw std::out_of_range("operator[] key out of range");
            }
            return iter->second;
        }

        template <typename CharType>
        RAINY_INLINE node_type &operator[](CharType *key) {
            if (is_null()) {
                value_ = document_type::object;
            }
            if (!is_object()) {
                exceptions::willow::throw_willow_invalid_key("operator[] called on a non-object object");
            }
            return (*value_.data.object)[key];
        }

        template <typename CharType>
        RAINY_INLINE const node_type &operator[](CharType *key) const {
            if (!is_object()) {
                exceptions::willow::throw_willow_invalid_key("operator[] called on a non-object object");
            }
            auto iter = value_.data.object->find(key);
            if (iter == value_.data.object->end()) {
                throw std::out_of_range("operator[] key out of range");
            }
            return iter->second;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return get<Ty>();
        }

        friend bool operator==(const basic_document &lhs, const basic_document &rhs) {
            return lhs.value_ == rhs.value_;
        }

        friend bool operator!=(const basic_document &lhs, const basic_document &rhs) {
            return !(lhs == rhs);
        }

        friend bool operator<(const basic_document &lhs, const basic_document &rhs) {
            const auto lhs_type = lhs.type();
            const auto rhs_type = rhs.type();
            if (lhs_type == rhs_type) {
                switch (lhs_type) {
                    case document_type::array:
                        return (*lhs.value_.data.vector) < (*rhs.value_.data.vector);
                    case document_type::object:
                        return (*lhs.value_.data.object) < (*rhs.value_.data.object);
                    case document_type::string:
                        return (*lhs.value_.data.string) < (*rhs.value_.data.string);
                    case document_type::boolean:
                        return (lhs.value_.data.boolean < rhs.value_.data.boolean);
                    case document_type::number_integer:
                        return (lhs.value_.data.number_integer < rhs.value_.data.number_integer);
                    case document_type::number_float:
                        return (lhs.value_.data.number_float < rhs.value_.data.number_float);
                    case document_type::null:
                    default:
                        return false;
                }
            }
            if (lhs_type == document_type::number_integer && rhs_type == document_type::number_float) {
                return (static_cast<float_type>(lhs.value_.data.number_integer) < rhs.value_.data.number_float);
            }
            if (lhs_type == document_type::number_float && rhs_type == document_type::number_integer) {
                return (lhs.value_.data.number_float < static_cast<float_type>(rhs.value_.data.number_integer));
            }
            return false;
        }

        friend bool operator<=(const basic_document &lhs, const basic_document &rhs) {
            return !(rhs < lhs);
        }

        friend bool operator>(const basic_document &lhs, const basic_document &rhs) {
            return rhs < lhs;
        }

        friend bool operator>=(const basic_document &lhs, const basic_document &rhs) {
            return !(lhs < rhs);
        }

    private:
        implements::value<basic_document> value_;
    };
}

namespace rainy::foundation::willow::implements {
    template <typename CharType>
    struct document_utf8_converter;

    template <>
    struct document_utf8_converter<char16_t> {
        using type = core::text::wstring_convert<core::text::codecvt_utf8_utf16<char16_t>, core::text::basic_string, char16_t>;
    };

    template <>
    struct document_utf8_converter<char32_t> {
        using type = core::text::wstring_convert<core::text::codecvt_utf8<char32_t>, core::text::basic_string, char32_t>;
    };

    template <>
    struct document_utf8_converter<wchar_t> {
#if RAINY_USING_WINDOWS
        using type = core::text::wstring_convert<core::text::codecvt_utf8_utf16<wchar_t>, core::text::basic_string, wchar_t>;
#else
        using type = core::text::wstring_convert<core::text::codecvt_utf8<char32_t>, core::text::basic_string, char32_t>;
#endif
    };

    template <typename TargetString>
    TargetString decode_document_utf8(const char *first, const char *last) {
        using target_char_type = typename TargetString::value_type;
        if constexpr (type_traits::type_relations::is_same_v<target_char_type, char>) {
            return TargetString(first, last);
        }
#if RAINY_HAS_CXX20
        else if constexpr (type_traits::type_relations::is_same_v<target_char_type, char8_t>) {
            TargetString result;
            result.reserve(static_cast<std::size_t>(last - first));
            for (; first != last; ++first) {
                result.push_back(static_cast<char8_t>(static_cast<unsigned char>(*first)));
            }
            return result;
        } else
#endif
        {
            using converter = typename document_utf8_converter<target_char_type>::type;
            const auto converted = converter{}.from_bytes(first, last);
            return TargetString(converted.begin(), converted.end());
        }
    }

    template <typename TargetString, typename SourceString>
    TargetString convert_document_string(const SourceString &source) {
        using source_char_type = typename SourceString::value_type;
        using target_char_type = typename TargetString::value_type;

        if constexpr (type_traits::type_relations::is_same_v<source_char_type, target_char_type>) {
            return TargetString(source.begin(), source.end());
        } else if constexpr (type_traits::type_relations::is_same_v<source_char_type, char>) {
            return decode_document_utf8<TargetString>(source.data(), source.data() + source.size());
        }
#if RAINY_HAS_CXX20
        else if constexpr (type_traits::type_relations::is_same_v<source_char_type, char8_t>) {
            return decode_document_utf8<TargetString>(reinterpret_cast<const char *>(source.data()),
                                                       reinterpret_cast<const char *>(source.data() + source.size()));
        }
#endif
        else {
            using converter = typename document_utf8_converter<source_char_type>::type;
            typename converter::wide_string wide(source.begin(), source.end());
            const auto utf8 = converter{}.to_bytes(wide);
            return decode_document_utf8<TargetString>(utf8.data(), utf8.data() + utf8.size());
        }
    }

    template <typename TargetDocument, typename SourceDocument>
    TargetDocument convert_document(const SourceDocument &source) {
        const auto &source_document = source.as_document();
        using source_type = type_traits::modifers::remove_cvref_t<decltype(source_document)>;
        if constexpr (type_traits::type_relations::is_same_v<TargetDocument, source_type> &&
                      type_traits::type_relations::is_same_v<typename TargetDocument::node_type, TargetDocument>) {
            return TargetDocument(source_document);
        } else if (source_document.is_object()) {
            TargetDocument result(document_type::object);
            for (const auto &entry: source_document.as_object()) {
                auto key = convert_document_string<typename TargetDocument::string_type>(entry.first);
                result[key] = convert_document<typename TargetDocument::node_type>(entry.second);
            }
            return result;
        } else if (source_document.is_array()) {
            TargetDocument result(document_type::array);
            for (const auto &element: source_document.as_array()) {
                result.push_back(convert_document<typename TargetDocument::node_type>(element));
            }
            return result;
        } else if (source_document.is_string()) {
            return TargetDocument(convert_document_string<typename TargetDocument::string_type>(source_document.as_string()));
        } else if (source_document.is_integer()) {
            return TargetDocument(static_cast<typename TargetDocument::integer_type>(source_document.as_integer()));
        } else if (source_document.is_float()) {
            return TargetDocument(static_cast<typename TargetDocument::float_type>(source_document.as_float()));
        } else if (source_document.is_bool()) {
            return TargetDocument(static_cast<typename TargetDocument::boolean_type>(source_document.as_bool()));
        } else {
            return TargetDocument{};
        }
    }
}

namespace rainy::foundation::willow {
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_basic_document_v = false;

    template <template <class Key, class Ty, class... Args> class ObjectType, template <class Key, class... Args> class ArrayType,
              typename StringType, typename IntegerType, typename FloatingType, typename BooleanType, template <class Ty> class Alloc,
              typename NodeTag>
    RAINY_CONSTEXPR_BOOL is_basic_document_v<
        basic_document<ObjectType, ArrayType, StringType, IntegerType, FloatingType, BooleanType, Alloc, NodeTag>> = true;

    template <typename Ty>
    struct is_basic_document : type_traits::helper::bool_constant<is_basic_document_v<Ty>> {};
}

#endif
