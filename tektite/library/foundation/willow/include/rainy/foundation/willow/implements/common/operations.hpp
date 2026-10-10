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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_OPERATIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_OPERATIONS_HPP
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/charconv.hpp>
#include <rainy/foundation/willow/document.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>
#include <rainy/foundation/willow/implements/common/nearly_equal.hpp>
#include <rainy/foundation/willow/reader.hpp>
#include <rainy/foundation/willow/writer.hpp>

namespace rainy::foundation::willow::implements::operations {
    /**
     * \lang english
     * @brief Returns the jsonnet-style type name of a document value.
     *
     * @return One of @c "null", @c "boolean", @c "number", @c "string", @c "array" or @c "object".
     *
     * \lang simp-chinese
     * @brief 返回文档值的 jsonnet 风格类型名。
     *
     * @return @c "null"、@c "boolean"、@c "number"、@c "string"、@c "array" 或 @c "object" 之一。
     */
    template <typename BasicDocument>
    core::text::basic_string_view<typename BasicDocument::char_type> type_name(const BasicDocument &doc) {
        using char_type = typename BasicDocument::char_type;
        switch (doc.type()) {
            case document_type::null:
                return core::text::basic_string_view<char_type>("null");
            case document_type::boolean:
                return core::text::basic_string_view<char_type>("boolean");
            case document_type::number_integer:
            case document_type::number_float:
                return core::text::basic_string_view<char_type>("number");
            case document_type::string:
                return core::text::basic_string_view<char_type>("string");
            case document_type::array:
                return core::text::basic_string_view<char_type>("array");
            case document_type::object:
                return core::text::basic_string_view<char_type>("object");
        }
        return core::text::basic_string_view<char_type>("null");
    }

    template <typename BasicDocument>
    double as_number(const BasicDocument &doc) {
        if (!doc.is_number()) {
            exceptions::willow::throw_willow_type_error("expected a number");
        }
        return doc.as_float();
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type as_string(const BasicDocument &doc) {
        if (!doc.is_string()) {
            exceptions::willow::throw_willow_type_error("expected a string");
        }
        return doc;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type as_array(const BasicDocument &doc) {
        if (!doc.is_array()) {
            exceptions::willow::throw_willow_type_error("expected an array");
        }
        return doc;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type as_object(const BasicDocument &doc) {
        if (!doc.is_object()) {
            exceptions::willow::throw_willow_type_error("expected an object");
        }
        return doc;
    }

    /**
     * \lang english
     * @brief Counts the code points of a string value.
     *
     * \lang simp-chinese
     * @brief 统计字符串值的码点数。
     */
    template <typename String>
    std::size_t codepoint_length(const String &str) {
        using char_type = typename String::value_type;
        std::size_t count = 0;
        if constexpr (sizeof(char_type) == 1) {
            for (std::size_t i = 0; i < str.size(); ++i) {
                const auto byte = static_cast<std::uint8_t>(str[i]);
                if ((byte & 0xC0) != 0x80) {
                    ++count;
                }
            }
        } else if constexpr (sizeof(char_type) == 2) {
            for (std::size_t i = 0; i < str.size(); ++i) {
                const auto unit = static_cast<std::uint32_t>(static_cast<std::uint16_t>(str[i]));
                if (unicode_surrogate_lead_begin <= unit && unit <= unicode_surrogate_lead_end && i + 1 < str.size()) {
                    ++i;
                }
                ++count;
            }
        } else {
            count = str.size();
        }
        return count;
    }

    /**
     * \lang english
     * @brief Returns the number of elements of an array, the number of fields of an object, or the
     *        code-point count of a string.
     *
     * \lang simp-chinese
     * @brief 返回数组的元素数、对象的字段数，或字符串的码点数。
     */
    template <typename BasicDocument>
    std::int64_t length(const BasicDocument &doc) {
        switch (doc.type()) {
            case document_type::string:
                return static_cast<std::int64_t>(codepoint_length(doc.as_string()));
            case document_type::array:
                return static_cast<std::int64_t>(doc.as_array().size());
            case document_type::object:
                return static_cast<std::int64_t>(doc.as_object().size());
            default:
                exceptions::willow::throw_willow_type_error("length operates on strings, arrays and objects");
        }
        return 0;
    }

    /**
     * \lang english
     * @brief Compares two values for identity without deep object traversal.
     *
     * \lang simp-chinese
     * @brief 在不深入遍历对象的前提下比较两个值是否同一。
     */
    template <typename BasicDocument>
    bool primitive_equals(const BasicDocument &left, const BasicDocument &right) {
        if (left.type() != right.type()) {
            if (left.is_number() && right.is_number()) {
                return left.as_float() == right.as_float();
            }
            return false;
        }
        switch (left.type()) {
            case document_type::null:
                return true;
            case document_type::boolean:
                return left.as_bool() == right.as_bool();
            case document_type::number_integer:
                return left.as_integer() == right.as_integer();
            case document_type::number_float:
                return nearly_equal(left.as_float(), right.as_float());
            case document_type::string:
                return left.as_string() == right.as_string();
            default:
                return false;
        }
    }

    /**
     * \lang english
     * @brief Compares two values for deep equality, treating objects as unordered field maps.
     *
     * \lang simp-chinese
     * @brief 深入比较两个值是否相等，对象按无序字段映射处理。
     */
    template <typename BasicDocument>
    bool equals(const BasicDocument &left, const BasicDocument &right) {
        if (left.type() != right.type()) {
            return left.is_number() && right.is_number() && nearly_equal(left.as_float(), right.as_float());
        }
        switch (left.type()) {
            case document_type::array: {
                const auto &lhs = left.as_array();
                const auto &rhs = right.as_array();
                if (lhs.size() != rhs.size()) {
                    return false;
                }
                for (std::size_t i = 0; i < lhs.size(); ++i) {
                    if (!equals(lhs[i], rhs[i])) {
                        return false;
                    }
                }
                return true;
            }
            case document_type::object: {
                const auto &lhs = left.as_object();
                const auto &rhs = right.as_object();
                if (lhs.size() != rhs.size()) {
                    return false;
                }
                for (const auto &entry: lhs) {
                    const auto iter = rhs.find(entry.first);
                    if (iter == rhs.end() || !equals(entry.second, iter->second)) {
                        return false;
                    }
                }
                return true;
            }
            default:
                return primitive_equals(left, right);
        }
    }

    /**
     * \lang english
     * @brief Returns @c -1, @c 0 or @c 1 for a total order over values.
     *
     * \lang simp-chinese
     * @brief 返回值的全序比较结果 @c -1、@c 0 或 @c 1。
     */
    template <typename BasicDocument>
    int compare(const BasicDocument &left, const BasicDocument &right) {
        if (left.is_number() && right.is_number()) {
            const double lhs = left.as_float();
            const double rhs = right.as_float();
            return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0);
        }
        if (left.type() == right.type()) {
            switch (left.type()) {
                case document_type::string: {
                    const auto &lhs = left.as_string();
                    const auto &rhs = right.as_string();
                    return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0);
                }
                case document_type::boolean:
                    return left.as_bool() == right.as_bool() ? 0 : (left.as_bool() ? 1 : -1);
                case document_type::null:
                    return 0;
                default:
                    break;
            }
        }
        const auto lhs_type = static_cast<int>(left.type());
        const auto rhs_type = static_cast<int>(right.type());
        return lhs_type < rhs_type ? -1 : (lhs_type > rhs_type ? 1 : 0);
    }

    /**
     * \lang english
     * @brief Computes the floored modulo of two numbers.
     *
     * @brief The caller is responsible for rejecting a zero divisor before calling.
     *
     * \lang simp-chinese
     * @brief 计算两个数的下取整取模。
     *
     * @brief 调用方需在调用前拒绝零除数。
     */
    inline double modulo(const double left, const double right) {
        double result = std::fmod(left, right);
        if (result != 0.0 && ((result < 0.0) != (right < 0.0))) {
            result += right;
        }
        return result;
    }

    /**
     * \lang english
     * @brief Returns the first code point of a string as an integer.
     *
     * \lang simp-chinese
     * @brief 以整数返回字符串的第一个码点。
     */
    template <typename BasicDocument>
    std::int64_t codepoint(const typename BasicDocument::string_type &str) {
        std::size_t index = 0;
        std::uint32_t code = 0;
        unicode_reader<typename BasicDocument::string_type> reader{str, true};
        if (!reader.get_code(index, code)) {
            exceptions::willow::throw_willow_type_error("codepoint requires a non-empty string");
        }
        return static_cast<std::int64_t>(code);
    }

    /**
     * \lang english
     * @brief Builds a one-code-point string from an integer.
     *
     * \lang simp-chinese
     * @brief 从整数构造单码点字符串。
     */
    template <typename BasicDocument>
    typename BasicDocument::node_type char_from_code(const std::int64_t code) {
        typename BasicDocument::string_type buffer;
        unicode_writer<typename BasicDocument::string_type> writer{buffer};
        writer.add_code(static_cast<std::uint32_t>(code));
        return typename BasicDocument::node_type(utility::move(buffer));
    }

    /**
     * \lang english
     * @brief Splits a string into an array of single-code-point strings.
     *
     * \lang simp-chinese
     * @brief 将字符串拆分为单码点字符串组成的数组。
     */
    template <typename BasicDocument>
    typename BasicDocument::node_type string_chars(const BasicDocument &doc) {
        const auto &str = doc.as_string();
        typename BasicDocument::node_type result(document_type::array);
        unicode_reader<typename BasicDocument::string_type> reader{str, true};
        std::size_t index = 0;
        std::uint32_t code = 0;
        while (reader.get_code(index, code)) {
            typename BasicDocument::string_type buffer;
            unicode_writer<typename BasicDocument::string_type> writer{buffer};
            writer.add_code(code);
            result.push_back(typename BasicDocument::node_type(utility::move(buffer)));
        }
        return result;
    }

    template <typename BasicDocument>
    bool object_has(const BasicDocument &doc, const core::text::basic_string_view<typename BasicDocument::char_type> field) {
        return doc.is_object() && doc.find(field) != doc.cend();
    }

    /**
     * \lang english
     * @brief Returns the field names of an object as an array, in iteration order.
     *
     * \lang simp-chinese
     * @brief 以数组返回对象的字段名，按迭代顺序。
     */
    template <typename BasicDocument>
    typename BasicDocument::node_type object_fields(const BasicDocument &doc) {
        if (!doc.is_object()) {
            exceptions::willow::throw_willow_type_error("objectFields requires an object");
        }
        typename BasicDocument::node_type result(document_type::array);
        for (const auto &entry: doc.as_object()) {
            result.push_back(typename BasicDocument::node_type(entry.first));
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type object_values(const BasicDocument &doc) {
        if (!doc.is_object()) {
            exceptions::willow::throw_willow_type_error("objectValues requires an object");
        }
        typename BasicDocument::node_type result(document_type::array);
        for (const auto &entry: doc.as_object()) {
            result.push_back(entry.second);
        }
        return result;
    }

    /**
     * \lang english
     * @brief Joins an array of strings with a separator, or an array of arrays by concatenation.
     *
     * \lang simp-chinese
     * @brief 以分隔符连接字符串数组，或按拼接连接数组的数组。
     */
    template <typename BasicDocument>
    typename BasicDocument::node_type join(const BasicDocument &separator, const BasicDocument &items) {
        if (!items.is_array()) {
            exceptions::willow::throw_willow_type_error("join requires an array");
        }
        const auto &array = items.as_array();
        if (array.empty()) {
            return separator.is_string() ? typename BasicDocument::node_type(typename BasicDocument::string_type{})
                                         : typename BasicDocument::node_type(document_type::array);
        }
        if (separator.is_string()) {
            typename BasicDocument::string_type result;
            for (std::size_t i = 0; i < array.size(); ++i) {
                if (i != 0) {
                    result.append(separator.as_string());
                }
                result.append(array[i].as_string());
            }
            return typename BasicDocument::node_type(utility::move(result));
        }
        typename BasicDocument::node_type result(document_type::array);
        for (std::size_t i = 0; i < array.size(); ++i) {
            if (i != 0) {
                for (const auto &element: separator.as_array()) {
                    result.push_back(element);
                }
            }
            for (const auto &element: array[i].as_array()) {
                result.push_back(element);
            }
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type reverse(const BasicDocument &doc) {
        if (doc.is_array()) {
            typename BasicDocument::node_type result(document_type::array);
            const auto &array = doc.as_array();
            for (std::size_t i = array.size(); i > 0; --i) {
                result.push_back(array[i - 1]);
            }
            return result;
        }
        if (doc.is_string()) {
            const auto chars = string_chars(doc);
            const auto reversed = reverse(chars);
            typename BasicDocument::string_type buffer;
            for (const auto &element: reversed.as_array()) {
                buffer.append(element.as_string());
            }
            return typename BasicDocument::node_type(utility::move(buffer));
        }
        exceptions::willow::throw_willow_type_error("reverse operates on strings and arrays");
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type range(const std::int64_t from, const std::int64_t to) {
        typename BasicDocument::node_type result(document_type::array);
        for (std::int64_t value = from; value < to; ++value) {
            result.push_back(typename BasicDocument::node_type(static_cast<typename BasicDocument::integer_type>(value)));
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type flatten_arrays(const BasicDocument &doc) {
        if (!doc.is_array()) {
            exceptions::willow::throw_willow_type_error("flattenArrays requires an array");
        }
        typename BasicDocument::node_type result(document_type::array);
        for (const auto &element: doc.as_array()) {
            for (const auto &inner: element.as_array()) {
                result.push_back(inner);
            }
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type repeat(const BasicDocument &value, const std::int64_t count) {
        if (value.is_string()) {
            typename BasicDocument::string_type result;
            for (std::int64_t i = 0; i < count; ++i) {
                result.append(value.as_string());
            }
            return typename BasicDocument::node_type(utility::move(result));
        }
        typename BasicDocument::node_type result(document_type::array);
        for (std::int64_t i = 0; i < count; ++i) {
            result.push_back(value);
        }
        return result;
    }

    template <typename BasicDocument>
    std::int64_t count(const BasicDocument &arr, const BasicDocument &value) {
        std::int64_t total = 0;
        for (const auto &element: arr.as_array()) {
            if (equals(element, value)) {
                ++total;
            }
        }
        return total;
    }

    template <typename BasicDocument>
    bool member(const BasicDocument &arr, const BasicDocument &value) {
        for (const auto &element: arr.as_array()) {
            if (equals(element, value)) {
                return true;
            }
        }
        return false;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type set_union(const BasicDocument &left, const BasicDocument &right) {
        typename BasicDocument::node_type result(document_type::array);
        for (const auto &element: left.as_array()) {
            result.push_back(element);
        }
        for (const auto &element: right.as_array()) {
            if (!member(result, element)) {
                result.push_back(element);
            }
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type set_inter(const BasicDocument &left, const BasicDocument &right) {
        typename BasicDocument::node_type result(document_type::array);
        for (const auto &element: left.as_array()) {
            if (member(right, element) && !member(result, element)) {
                result.push_back(element);
            }
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type set_diff(const BasicDocument &left, const BasicDocument &right) {
        typename BasicDocument::node_type result(document_type::array);
        for (const auto &element: left.as_array()) {
            if (!member(right, element) && !member(result, element)) {
                result.push_back(element);
            }
        }
        return result;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type ascii_upper(const BasicDocument &doc) {
        typename BasicDocument::string_type result = doc.as_string();
        for (auto &ch: result) {
            if (ch >= 'a' && ch <= 'z') {
                ch = static_cast<typename BasicDocument::char_type>(ch - 'a' + 'A');
            }
        }
        return typename BasicDocument::node_type(utility::move(result));
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type ascii_lower(const BasicDocument &doc) {
        typename BasicDocument::string_type result = doc.as_string();
        for (auto &ch: result) {
            if (ch >= 'A' && ch <= 'Z') {
                ch = static_cast<typename BasicDocument::char_type>(ch - 'A' + 'a');
            }
        }
        return typename BasicDocument::node_type(utility::move(result));
    }

    template <typename BasicDocument>
    bool starts_with(const BasicDocument &doc, const core::text::basic_string_view<typename BasicDocument::char_type> prefix) {
        const auto &str = doc.as_string();
        return str.size() >= prefix.size() &&
               core::text::char_traits<typename BasicDocument::char_type>::compare(str.data(), prefix.data(), prefix.size()) == 0;
    }

    template <typename BasicDocument>
    bool ends_with(const BasicDocument &doc, const core::text::basic_string_view<typename BasicDocument::char_type> suffix) {
        const auto &str = doc.as_string();
        return str.size() >= suffix.size() &&
               core::text::char_traits<typename BasicDocument::char_type>::compare(str.data() + (str.size() - suffix.size()),
                                                                                   suffix.data(), suffix.size()) == 0;
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type substr(const BasicDocument &doc, const std::int64_t from, const std::int64_t length_value) {
        const auto &str = doc.as_string();
        const auto size = static_cast<std::int64_t>(str.size());
        std::int64_t start = from < 0 ? size + from : from;
        start = start < 0 ? 0 : (start > size ? size : start);
        std::int64_t stop = start + length_value;
        stop = stop < start ? start : (stop > size ? size : stop);
        return typename BasicDocument::node_type(
            typename BasicDocument::string_type(str.data() + static_cast<std::size_t>(start), static_cast<std::size_t>(stop - start)));
    }

    /**
     * \lang english
     * @brief Slices a string or array with jsonnet index/end/step semantics.
     *
     * @brief Negative indices count from the end; the caller rejects a zero step.
     *
     * \lang simp-chinese
     * @brief 按 jsonnet 的 index/end/step 语义切分字符串或数组。
     *
     * @brief 负索引从末尾计数；零步长由调用方拒绝。
     */
    template <typename BasicDocument>
    typename BasicDocument::node_type slice(const BasicDocument &doc, const std::int64_t index, const std::int64_t end,
                                            const std::int64_t step) {
        if (doc.is_array()) {
            const auto &array = doc.as_array();
            const auto size = static_cast<std::int64_t>(array.size());
            std::int64_t begin = index < 0 ? size + index : index;
            std::int64_t stop = end < 0 ? size + end : end;
            begin = begin < 0 ? 0 : (begin > size ? size : begin);
            stop = stop < 0 ? 0 : (stop > size ? size : stop);
            typename BasicDocument::node_type result(document_type::array);
            if (step > 0) {
                for (std::int64_t i = begin; i < stop; i += step) {
                    result.push_back(array[static_cast<std::size_t>(i)]);
                }
            } else {
                for (std::int64_t i = begin; i > stop; i += step) {
                    result.push_back(array[static_cast<std::size_t>(i)]);
                }
            }
            return result;
        }
        if (doc.is_string()) {
            const auto chars = string_chars(doc);
            const auto sliced = slice(chars, index, end, step);
            typename BasicDocument::string_type buffer;
            for (const auto &element: sliced.as_array()) {
                buffer.append(element.as_string());
            }
            return typename BasicDocument::node_type(utility::move(buffer));
        }
        exceptions::willow::throw_willow_type_error("slice operates on strings and arrays");
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type strip_chars(const BasicDocument &doc,
                                                  const core::text::basic_string_view<typename BasicDocument::char_type> chars,
                                                  const bool left, const bool right) {
        const auto &str = doc.as_string();
        std::size_t begin = 0;
        std::size_t end = str.size();
        auto matches = [&chars](const typename BasicDocument::char_type ch) {
            for (const auto candidate: chars) {
                if (candidate == ch) {
                    return true;
                }
            }
            return false;
        };
        if (left) {
            while (begin < end && matches(str[begin])) {
                ++begin;
            }
        }
        if (right) {
            while (end > begin && matches(str[end - 1])) {
                --end;
            }
        }
        return typename BasicDocument::node_type(typename BasicDocument::string_type(str.data() + begin, end - begin));
    }

    template <typename BasicDocument>
    typename BasicDocument::node_type split_limit(const BasicDocument &doc,
                                                  const core::text::basic_string_view<typename BasicDocument::char_type> separator,
                                                  const std::int64_t limit) {
        const auto &str = doc.as_string();
        typename BasicDocument::node_type result(document_type::array);
        if (separator.empty()) {
            return string_chars(doc);
        }
        std::size_t start = 0;
        std::int64_t splits = 0;
        while (limit < 0 || splits < limit) {
            std::size_t found = str.size();
            for (std::size_t i = start; i + separator.size() <= str.size(); ++i) {
                if (core::text::char_traits<typename BasicDocument::char_type>::compare(str.data() + i, separator.data(),
                                                                                       separator.size()) == 0) {
                    found = i;
                    break;
                }
            }
            if (found == str.size()) {
                break;
            }
            result.push_back(typename BasicDocument::node_type(
                typename BasicDocument::string_type(str.data() + start, found - start)));
            start = found + separator.size();
            ++splits;
        }
        result.push_back(
            typename BasicDocument::node_type(typename BasicDocument::string_type(str.data() + start, str.size() - start)));
        return result;
    }

    template <typename BasicDocument>
    std::int64_t find_substr(const BasicDocument &doc,
                             const core::text::basic_string_view<typename BasicDocument::char_type> needle) {
        const auto &str = doc.as_string();
        if (needle.empty()) {
            return 0;
        }
        for (std::size_t i = 0; i + needle.size() <= str.size(); ++i) {
            if (core::text::char_traits<typename BasicDocument::char_type>::compare(str.data() + i, needle.data(), needle.size()) ==
                0) {
                return static_cast<std::int64_t>(i);
            }
        }
        return -1;
    }

    template <typename BasicDocument>
    std::int64_t parse_int(const BasicDocument &doc) {
        const auto &str = doc.as_string();
        std::int64_t value{};
        const auto result = core::text::from_chars(str.data(), str.data() + str.size(), value);
        if (result.ec != std::errc{} || result.ptr != str.data() + str.size()) {
            exceptions::willow::throw_willow_parse_error("parseInt requires a decimal integer string");
        }
        return value;
    }

    inline double clamp(const double value, const double low, const double high) {
        return value < low ? low : (value > high ? high : value);
    }

    inline double sign(const double value) {
        return value < 0.0 ? -1.0 : (value > 0.0 ? 1.0 : 0.0);
    }

    inline bool is_integer_value(const double value) {
        return std::isfinite(value) && std::floor(value) == value;
    }
}

#endif
