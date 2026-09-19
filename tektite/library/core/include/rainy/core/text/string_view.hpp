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
#ifndef RAINY_YESOD_TEXT_STRING_VIEW_HPP
#define RAINY_YESOD_TEXT_STRING_VIEW_HPP

// NOLINTBEGIN
#include <rainy/core/platform.hpp>
#include <rainy/core/text/fwd.hpp>
#include <rainy/core/text/char_traits.hpp>
#include <rainy/core/type_traits.hpp>
// NOLINTEND

namespace rainy::core::text {
    /**
     * \lang english
     * @brief A non-owning constant view over a contiguous sequence of characters.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     *
     * \lang simp-chinese
     * @brief 对一段连续字符序列的非拥有常量视图。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     */
    template <typename CharType, typename Traits>
    class basic_string_view {
    public:
        /**
         * \lang english
         * @brief The character traits type.
         *
         * \lang simp-chinese
         * @brief 字符 traits 类型。
         */
        using traits_type = Traits;
        /**
         * \lang english
         * @brief The character element type.
         *
         * \lang simp-chinese
         * @brief 字符元素类型。
         */
        using value_type = CharType;
        /**
         * \lang english
         * @brief The pointer type.
         *
         * \lang simp-chinese
         * @brief 指针类型。
         */
        using pointer = value_type *;
        /**
         * \lang english
         * @brief The constant pointer type.
         *
         * \lang simp-chinese
         * @brief 常量指针类型。
         */
        using const_pointer = const value_type *;
        /**
         * \lang english
         * @brief The reference type.
         *
         * \lang simp-chinese
         * @brief 引用类型。
         */
        using reference = value_type &;
        /**
         * \lang english
         * @brief The constant reference type.
         *
         * \lang simp-chinese
         * @brief 常量引用类型。
         */
        using const_reference = const value_type &;
        /**
         * \lang english
         * @brief The constant iterator type.
         *
         * \lang simp-chinese
         * @brief 常量迭代器类型。
         */
        using const_iterator = const value_type *;
        /**
         * \lang english
         * @brief The iterator type; a constant view only provides constant iterators.
         *
         * \lang simp-chinese
         * @brief 迭代器类型；常量视图仅提供常量迭代器。
         */
        using iterator = const_iterator;
        /**
         * \lang english
         * @brief The constant reverse iterator type.
         *
         * \lang simp-chinese
         * @brief 常量反向迭代器类型。
         */
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;
        /**
         * \lang english
         * @brief The reverse iterator type.
         *
         * \lang simp-chinese
         * @brief 反向迭代器类型。
         */
        using reverse_iterator = utility::reverse_iterator<const_iterator>;
        /**
         * \lang english
         * @brief The unsigned size type.
         *
         * \lang simp-chinese
         * @brief 无符号大小类型。
         */
        using size_type = size_t;
        /**
         * \lang english
         * @brief The signed difference type.
         *
         * \lang simp-chinese
         * @brief 有符号差值类型。
         */
        using difference_type = ptrdiff_t;

        /**
         * \lang english
         * @brief The owning string type with the same character type.
         *
         * \lang simp-chinese
         * @brief 具有相同字符类型的拥有型字符串类型。
         */
        using basic_string = text::basic_string<value_type>;

        /**
         * \lang english
         * @brief A special value meaning "not found" or "all remaining characters".
         *
         * \lang simp-chinese
         * @brief 表示"未找到"或"剩余全部字符"的特殊值。
         */
        static constexpr size_type npos = static_cast<size_type>(-1);

        /**
         * \lang english
         * @brief Constructs an empty string view.
         *
         * \lang simp-chinese
         * @brief 构造一个空字符串视图。
         */
        constexpr basic_string_view() noexcept = default;
        /**
         * \lang english
         * @brief Copy constructor.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         */
        constexpr basic_string_view(const basic_string_view &) noexcept = default;
        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         */
        constexpr basic_string_view &operator=(const basic_string_view &) noexcept = default;

        /**
         * \lang english
         * @brief Constructs a string view over a null-terminated character string.
         *
         * @param str The null-terminated character pointer
         *
         * \lang simp-chinese
         * @brief 基于以空字符结尾的字符串构造字符串视图。
         *
         * @param str 以空字符结尾的字符指针
         */
        constexpr basic_string_view(const value_type *str) : data_{str}, size_{traits_type::length(str)} {
        }

        /**
         * \lang english
         * @brief Deleted: a string view cannot be constructed from a null pointer.
         *
         * \lang simp-chinese
         * @brief 已删除：禁止使用空指针构造字符串视图。
         */
        constexpr basic_string_view(std::nullptr_t) = delete;

        /**
         * \lang english
         * @brief Constructs a string view over a character sequence with an explicit length.
         *
         * @param str The character pointer
         * @param length The number of characters to view
         *
         * \lang simp-chinese
         * @brief 基于指定长度的字符序列构造字符串视图。
         *
         * @param str 字符指针
         * @param length 视图包含的字符数量
         */
        constexpr basic_string_view(const value_type *str, size_type length) : data_{str}, size_{length} {
        }

        /**
         * \lang english
         * @brief Constructs a string view over the half-open range [begin, end).
         *
         * @param begin Pointer to the first character
         * @param end Pointer one past the last character
         *
         * \lang simp-chinese
         * @brief 基于左闭右开区间 [begin, end) 构造字符串视图。
         *
         * @param begin 指向首字符的指针
         * @param end 指向末字符之后位置的指针
         */
        constexpr basic_string_view(const_pointer begin, const_pointer end) noexcept : data_(begin), size_(end - begin) {
        }

        template <typename R, type_traits::other_trans::enable_if_t<
                                  type_traits::extras::meta_method::has_data_v<type_traits::modifers::remove_cvref_t<R>> &&
                                      type_traits::extras::meta_method::has_size_v<type_traits::modifers::remove_cvref_t<R>>,
                                  int> = 0>
        /**
         * \lang english
         * @brief Constructs a string view from any contiguous range that provides data() and size().
         *
         * @tparam R The contiguous range type
         * @param right The source range
         *
         * \lang simp-chinese
         * @brief 由任意提供 data() 与 size() 的连续范围构造字符串视图。
         *
         * @tparam R 连续范围类型
         * @param right 源范围
         */
        constexpr basic_string_view(const R &right) : data_(right.data()), size_(right.size()) {
        }

        template <
            typename It, typename End,
            type_traits::other_trans::enable_if_t<
                type_traits::extras::iterators::is_contiguous_iterator_v<It> && type_traits::extras::iterators::is_iterator_v<End> &&
                    type_traits::type_relations::is_same_v<type_traits::extras::iterators::iter_value_t<It>, value_type> &&
                    !type_traits::type_relations::is_convertible_v<It, size_type> &&
                    !type_traits::type_relations::is_convertible_v<End, size_type>,
                int> = 0>
        /**
         * \lang english
         * @brief Constructs a string view from a pair of contiguous iterators.
         *
         * @tparam It The iterator type
         * @tparam End The end sentinel type
         * @param begin Iterator to the first character
         * @param end Iterator one past the last character
         *
         * \lang simp-chinese
         * @brief 由一对连续迭代器构造字符串视图。
         *
         * @tparam It 迭代器类型
         * @tparam End 末尾哨兵类型
         * @param begin 指向首字符的迭代器
         * @param end 指向末字符之后位置的迭代器
         */
        constexpr basic_string_view(It begin, End end) noexcept :
            data_{begin}, size_{static_cast<size_type>(utility::distance(begin, end))} {
        }

        /**
         * \lang english
         * @brief Returns an iterator to the first character.
         *
         * @return An iterator to the first character
         *
         * \lang simp-chinese
         * @brief 返回指向首字符的迭代器。
         *
         * @return 指向首字符的迭代器
         */
        constexpr const_iterator begin() const noexcept {
            return data_;
        }

        /**
         * \lang english
         * @brief Returns an iterator one past the last character.
         *
         * @return An iterator one past the last character
         *
         * \lang simp-chinese
         * @brief 返回指向末字符之后位置的迭代器。
         *
         * @return 指向末字符之后位置的迭代器
         */
        constexpr const_iterator end() const noexcept {
            return data_ + size_;
        }

        /**
         * \lang english
         * @brief Returns a constant iterator to the first character.
         *
         * @return A constant iterator to the first character
         *
         * \lang simp-chinese
         * @brief 返回指向首字符的常量迭代器。
         *
         * @return 指向首字符的常量迭代器
         */
        constexpr const_iterator cbegin() const noexcept {
            return data_;
        }

        /**
         * \lang english
         * @brief Returns a constant iterator one past the last character.
         *
         * @return A constant iterator one past the last character
         *
         * \lang simp-chinese
         * @brief 返回指向末字符之后位置的常量迭代器。
         *
         * @return 指向末字符之后位置的常量迭代器
         */
        constexpr const_iterator cend() const noexcept {
            return data_ + size_;
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator to the last character.
         *
         * @return A reverse iterator to the last character
         *
         * \lang simp-chinese
         * @brief 返回指向末字符的反向迭代器。
         *
         * @return 指向末字符的反向迭代器
         */
        constexpr const_reverse_iterator rbegin() const noexcept {
            return reverse_iterator{end()};
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator one before the first character.
         *
         * @return A reverse iterator one before the first character
         *
         * \lang simp-chinese
         * @brief 返回指向首字符之前位置的反向迭代器。
         *
         * @return 指向首字符之前位置的反向迭代器
         */
        constexpr const_reverse_iterator rend() const noexcept {
            return reverse_iterator{begin()};
        }

        /**
         * \lang english
         * @brief Returns a constant reverse iterator to the last character.
         *
         * @return A constant reverse iterator to the last character
         *
         * \lang simp-chinese
         * @brief 返回指向末字符的常量反向迭代器。
         *
         * @return 指向末字符的常量反向迭代器
         */
        constexpr const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator{cend()};
        }

        /**
         * \lang english
         * @brief Returns a constant reverse iterator one before the first character.
         *
         * @return A constant reverse iterator one before the first character
         *
         * \lang simp-chinese
         * @brief 返回指向首字符之前位置的常量反向迭代器。
         *
         * @return 指向首字符之前位置的常量反向迭代器
         */
        constexpr const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator{cbegin()};
        }

        /**
         * \lang english
         * @brief Returns the number of characters in the view.
         *
         * @return The number of characters
         *
         * \lang simp-chinese
         * @brief 返回视图中的字符数量。
         *
         * @return 字符数量
         */
        constexpr size_type size() const noexcept {
            return size_;
        }

        /**
         * \lang english
         * @brief Returns the number of characters in the view.
         *
         * @return The number of characters
         *
         * \lang simp-chinese
         * @brief 返回视图中的字符数量。
         *
         * @return 字符数量
         */
        constexpr size_type length() const noexcept {
            return size_;
        }

        /**
         * \lang english
         * @brief Returns the maximum number of characters the view can hold.
         *
         * @return The maximum size
         *
         * \lang simp-chinese
         * @brief 返回视图可容纳的最大字符数。
         *
         * @return 最大大小
         */
        constexpr size_type max_size() const noexcept {
            return (utility::numeric_limits<value_type>::max)();
        }

        /**
         * \lang english
         * @brief Checks whether the view is empty.
         *
         * @return true if the view is empty, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否为空。
         *
         * @return 若视图为空返回 true，否则返回 false
         */
        constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        /**
         * \lang english
         * @brief Returns the character at the given position without bounds checking.
         *
         * @param pos The position of the character to return
         * @return A reference to the character at the given position
         *
         * \lang simp-chinese
         * @brief 返回指定位置的字符，不进行越界检查。
         *
         * @param pos 待返回字符的位置
         * @return 指定位置字符的引用
         */
        constexpr const_reference operator[](size_type pos) const {
            return data_[pos];
        }

        /**
         * \lang english
         * @brief Returns the character at the given position with bounds checking.
         *
         * @param pos The position of the character to return
         * @return A reference to the character at the given position
         *
         * \lang simp-chinese
         * @brief 返回指定位置的字符，并进行越界检查。
         *
         * @param pos 待返回字符的位置
         * @return 指定位置字符的引用
         */
        constexpr const_reference at(size_type pos) const {
            range_check_access(pos);
            return data_[pos];
        }

        /**
         * \lang english
         * @brief Returns the first character.
         *
         * @return A reference to the first character
         *
         * \lang simp-chinese
         * @brief 返回首字符。
         *
         * @return 首字符的引用
         */
        constexpr const_reference front() const {
            return data_[0];
        }

        /**
         * \lang english
         * @brief Returns the last character.
         *
         * @return A reference to the last character
         *
         * \lang simp-chinese
         * @brief 返回末字符。
         *
         * @return 末字符的引用
         */
        constexpr const_reference back() const {
            return data_[size() - 1];
        }

        /**
         * \lang english
         * @brief Returns a pointer to the underlying character array.
         *
         * @return A pointer to the underlying character array
         *
         * \lang simp-chinese
         * @brief 返回指向底层字符数组的指针。
         *
         * @return 指向底层字符数组的指针
         */
        constexpr const_pointer data() const noexcept {
            return data_;
        }

        /**
         * \lang english
         * @brief Moves the start of the view forward by count characters.
         *
         * @param count The number of characters to remove from the front
         *
         * \lang simp-chinese
         * @brief 将视图起点向后移动 count 个字符。
         *
         * @param count 从开头移除的字符数量
         */
        constexpr void remove_prefix(size_type count) {
            assert(count < size_);
            data_ += count;
            size_ -= count;
        }

        /**
         * \lang english
         * @brief Shortens the view by removing count characters from the end.
         *
         * @param count The number of characters to remove from the back
         *
         * \lang simp-chinese
         * @brief 从末尾移除 count 个字符以缩短视图。
         *
         * @param count 从末尾移除的字符数量
         */
        constexpr void remove_suffix(size_type count) {
            assert(count < size_);
            size_ -= count;
        }

        /**
         * \lang english
         * @brief Exchanges the contents of the two views.
         *
         * @param right The view to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个视图的内容。
         *
         * @param right 与之交换的视图
         */
        constexpr void swap(basic_string_view &right) noexcept {
            std::swap(this->data_, right.data_);
            std::swap(this->size_, right.size_);
        }

        /**
         * \lang english
         * @brief Copies a substring into a destination character buffer.
         *
         * @param dest The destination buffer
         * @param count The number of characters to copy
         * @param pos The starting position in this view
         * @return The number of characters actually copied
         *
         * \lang simp-chinese
         * @brief 将子串复制到目标字符缓冲区。
         *
         * @param dest 目标缓冲区
         * @param count 待复制的字符数量
         * @param pos 本视图中的起始位置
         * @return 实际复制的字符数量
         */
        constexpr size_type copy(pointer dest, size_type count, size_type pos = 0) const {
            range_check_access(pos);
            size_type rcount = (core::min) (count, size() - pos);
            traits_type::copy(dest, data() + pos, rcount);
            return rcount;
        }

        /**
         * \lang english
         * @brief Returns a view of the substring starting at pos with the given length.
         *
         * @param pos The starting position
         * @param count The length of the substring
         * @return A view of the requested substring
         *
         * \lang simp-chinese
         * @brief 返回从 pos 开始、长度为 count 的子串视图。
         *
         * @param pos 起始位置
         * @param count 子串长度
         * @return 所请求子串的视图
         */
        constexpr basic_string_view substr(size_type pos = 0, size_type count = npos) const {
            range_check_substr(pos);
            size_type rcount = (core::min) (count, size_ - pos);
            return {data_ + pos, rcount};
        }

        /**
         * \lang english
         * @brief Compares this view with another string view lexicographically.
         *
         * @param right The string view to compare with
         *
         * \lang simp-chinese
         * @brief 按字典序将此视图与另一个字符串视图进行比较。
         *
         * @param right 与之比较的字符串视图
         */
        constexpr int compare(basic_string_view right) const noexcept {
            const size_type rlen = (core::min) (size_, right.size_);
            if (const int result = traits_type::compare(data_, right.data_, rlen); result != 0) {
                return result;
            }
            return size_ == right.size_ ? 0 : (size_ < right.size_ ? -1 : 1);
        }

        /**
         * \lang english
         * @brief Compares the substring [pos1, pos1+n1) of this view with another string view.
         *
         * @param pos1 The starting position of the substring
         * @param n1 The length of the substring
         * @param s The string view to compare with
         *
         * \lang simp-chinese
         * @brief 将此视图的子串 [pos1, pos1+n1) 与另一个字符串视图进行比较。
         *
         * @param pos1 子串的起始位置
         * @param n1 子串的长度
         * @param s 与之比较的字符串视图
         */
        constexpr int compare(size_type pos1, size_type n1, basic_string_view s) const {
            return substr(pos1, n1).compare(s);
        }

        /**
         * \lang english
         * @brief Compares the substrings of two string views.
         *
         * @param pos1 The starting position of this substring
         * @param n1 The length of this substring
         * @param s The other string view
         * @param pos2 The starting position of the other substring
         * @param n2 The length of the other substring
         *
         * \lang simp-chinese
         * @brief 比较两个字符串视图的子串。
         *
         * @param pos1 本视图子串的起始位置
         * @param n1 本视图子串的长度
         * @param s 另一个字符串视图
         * @param pos2 另一个子串的起始位置
         * @param n2 另一个子串的长度
         */
        constexpr int compare(size_type pos1, size_type n1, basic_string_view s, size_type pos2, size_type n2) const {
            return substr(pos1, n1).compare(s.substr(pos2, n2));
        }

        /**
         * \lang english
         * @brief Compares this view with a null-terminated character string lexicographically.
         *
         * @param s The null-terminated character string to compare with
         *
         * \lang simp-chinese
         * @brief 按字典序将此视图与以空字符结尾的字符串进行比较。
         *
         * @param s 与之比较的以空字符结尾的字符串
         */
        constexpr int compare(const CharType *s) const {
            return compare(basic_string_view(s));
        }

        /**
         * \lang english
         * @brief Compares the substring [pos1, pos1+n1) of this view with a null-terminated character string.
         *
         * @param pos1 The starting position of the substring
         * @param n1 The length of the substring
         * @param s The null-terminated character string to compare with
         *
         * \lang simp-chinese
         * @brief 将此视图的子串 [pos1, pos1+n1) 与以空字符结尾的字符串进行比较。
         *
         * @param pos1 子串的起始位置
         * @param n1 子串的长度
         * @param s 与之比较的以空字符结尾的字符串
         */
        constexpr int compare(size_type pos1, size_type n1, const CharType *s) const {
            return substr(pos1, n1).compare(basic_string_view(s));
        }

        /**
         * \lang english
         * @brief Compares the substring of this view with a character sequence of explicit length.
         *
         * @param pos1 The starting position of the substring
         * @param n1 The length of the substring
         * @param s The character sequence to compare with
         * @param n2 The length of the character sequence
         *
         * \lang simp-chinese
         * @brief 将此视图的子串与指定长度的字符序列进行比较。
         *
         * @param pos1 子串的起始位置
         * @param n1 子串的长度
         * @param s 与之比较的字符序列
         * @param n2 字符序列的长度
         */
        constexpr int compare(size_type pos1, size_type n1, const CharType *s, size_type n2) const {
            return substr(pos1, n1).compare(basic_string_view(s, n2));
        }

        /**
         * \lang english
         * @brief Checks whether the view starts with the given string view.
         *
         * @param sv The prefix to check for
         * @return true if the view starts with sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的字符串视图开头。
         *
         * @param sv 待检查的前缀
         * @return 若视图以 sv 开头返回 true，否则返回 false
         */
        constexpr bool starts_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = this->begin();
            if (sv_size > this->size()) {
                return false;
            }
            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        /**
         * \lang english
         * @brief Checks whether the view starts with the given string view.
         *
         * @param sv The prefix to check for
         * @return true if the view starts with sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的字符串视图开头。
         *
         * @param sv 待检查的前缀
         * @return 若视图以 sv 开头返回 true，否则返回 false
         */
        constexpr bool starts_with(const basic_string_view sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = this->begin();
            if (sv_size > this->size()) {
                return false;
            }
            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        /**
         * \lang english
         * @brief Checks whether the view starts with the given character.
         *
         * @param ch The character to check for
         * @return true if the view starts with ch, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的字符开头。
         *
         * @param ch 待检查的字符
         * @return 若视图以 ch 开头返回 true，否则返回 false
         */
        constexpr bool starts_with(CharType ch) const noexcept {
            return *this->begin() == ch;
        }

        /**
         * \lang english
         * @brief Checks whether the view starts with the given null-terminated character string.
         *
         * @param cstr The null-terminated string to check for
         * @return true if the view starts with cstr, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的以空字符结尾的字符串开头。
         *
         * @param cstr 待检查的以空字符结尾的字符串
         * @return 若视图以 cstr 开头返回 true，否则返回 false
         */
        constexpr bool starts_with(const CharType *cstr) const {
            auto length = traits_type::length(cstr);
            auto begin = this->begin();
            if (length > this->size()) {
                return false;
            }
            return equal_(cstr, cstr + length, begin, begin + length);
        }

        /**
         * \lang english
         * @brief Checks whether the view ends with the given string view.
         *
         * @param sv The suffix to check for
         * @return true if the view ends with sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的字符串视图结尾。
         *
         * @param sv 待检查的后缀
         * @return 若视图以 sv 结尾返回 true，否则返回 false
         */
        constexpr bool ends_with(std::basic_string_view<CharType, Traits> sv) const noexcept {
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = this->end();
            if (sv_size > this->size()) {
                return false;
            }
            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        /**
         * \lang english
         * @brief Checks whether the view ends with the given string view.
         *
         * @param sv The suffix to check for
         * @return true if the view ends with sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的字符串视图结尾。
         *
         * @param sv 待检查的后缀
         * @return 若视图以 sv 结尾返回 true，否则返回 false
         */
        constexpr bool ends_with(const basic_string_view sv) const noexcept {
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = this->end();
            if (sv_size > this->size()) {
                return false;
            }
            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        /**
         * \lang english
         * @brief Checks whether the view ends with the given character.
         *
         * @param ch The character to check for
         * @return true if the view ends with ch, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的字符结尾。
         *
         * @param ch 待检查的字符
         * @return 若视图以 ch 结尾返回 true，否则返回 false
         */
        constexpr bool ends_with(CharType ch) const noexcept {
            return *(this->end() - 1) == ch;
        }

        /**
         * \lang english
         * @brief Checks whether the view ends with the given null-terminated character string.
         *
         * @param cstr The null-terminated string to check for
         * @return true if the view ends with cstr, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否以给定的以空字符结尾的字符串结尾。
         *
         * @param cstr 待检查的以空字符结尾的字符串
         * @return 若视图以 cstr 结尾返回 true，否则返回 false
         */
        constexpr bool ends_with(CharType const *cstr) const {
            auto length = traits_type::length(cstr);
            if (length > this->size()) {
                return false;
            }
            auto end = this->end();
            return equal_(cstr, cstr + length, end - length, end);
        }

        /**
         * \lang english
         * @brief Checks whether the view contains the given substring.
         *
         * @param x The substring to look for
         * @return true if the view contains x, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否包含给定的子串。
         *
         * @param x 待查找的子串
         * @return 若视图包含 x 返回 true，否则返回 false
         */
        constexpr bool contains(basic_string_view x) const noexcept {
            return find(x) != npos;
        }

        /**
         * \lang english
         * @brief Checks whether the view contains the given character.
         *
         * @param x The character to look for
         * @return true if the view contains x, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否包含给定的字符。
         *
         * @param x 待查找的字符
         * @return 若视图包含 x 返回 true，否则返回 false
         */
        constexpr bool contains(CharType x) const noexcept {
            for (size_type i = 0; i < size(); ++i) {
                if (data_[i] == x) {
                    return true;
                }
            }
            return false;
        }

        /**
         * \lang english
         * @brief Checks whether the view contains the given null-terminated character string.
         *
         * @param x The null-terminated string to look for
         * @return true if the view contains x, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查视图是否包含给定的以空字符结尾的字符串。
         *
         * @param x 待查找的以空字符结尾的字符串
         * @return 若视图包含 x 返回 true，否则返回 false
         */
        constexpr bool contains(const CharType *x) const {
            return find(x) != npos;
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of the given substring starting at pos.
         *
         * @param s The substring to search for
         * @param pos The position at which to start the search
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找给定子串首次出现的位置。
         *
         * @param s 待搜索的子串
         * @param pos 搜索的起始位置
         * @return 首次出现的下标，未找到时返回 npos
         */
        constexpr size_type find(basic_string_view s, size_type pos = 0) const noexcept {
            if (pos > size_) {
                return npos;
            }
            if (s.size_ == 0) {
                return pos;
            }
            if (s.size_ > size_ - pos) {
                return npos;
            }
            // NOLINTBEGIN
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto p = data_ + pos; p <= data_ + size_ - s.size_; ++p) {
                    bool match = true;
                    for (size_type i = 0; i < s.size_; ++i) {
                        if (!traits_type::eq(p[i], s.data_[i])) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return static_cast<size_type>(p - data_);
                    }
                }
                return npos;
            }
#endif
            // NOLINTEND
            const CharType *result = nullptr;
            for (auto p = data_ + pos; p <= data_ + size_ - s.size_; ++p) {
                if (traits_type::compare(p, s.data_, s.size_) == 0) {
                    result = p;
                    break;
                }
            }
            return result ? result - data_ : npos;
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of the given character starting at pos.
         *
         * @param c The character to search for
         * @param pos The position at which to start the search
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找给定字符首次出现的位置。
         *
         * @param c 待搜索的字符
         * @param pos 搜索的起始位置
         * @return 首次出现的下标，未找到时返回 npos
         */
        constexpr size_type find(CharType c, size_type pos = 0) const noexcept {
            if (pos >= size_) {
                return npos;
            }
            // NOLINTBEGIN
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto p = data_ + pos; p < data_ + size_; ++p) {
                    if (traits_type::eq(*p, c)) {
                        return static_cast<size_type>(p - data_);
                    }
                }
                return npos;
            }
#endif
            // NOLINTEND
            const CharType *result = traits_type::find(data_ + pos, size_ - pos, c);
            return result ? result - data_ : npos;
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of a character sequence of explicit length starting at pos.
         *
         * @param s The character sequence to search for
         * @param pos The position at which to start the search
         * @param n The length of the character sequence
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找指定长度的字符序列首次出现的位置。
         *
         * @param s 待搜索的字符序列
         * @param pos 搜索的起始位置
         * @param n 字符序列的长度
         * @return 首次出现的下标，未找到时返回 npos
         */
        constexpr size_type find(const CharType *s, size_type pos, size_type n) const {
            return find(basic_string_view(s, n), pos);
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of a null-terminated character string starting at pos.
         *
         * @param s The null-terminated string to search for
         * @param pos The position at which to start the search
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找以空字符结尾的字符串首次出现的位置。
         *
         * @param s 待搜索的以空字符结尾的字符串
         * @param pos 搜索的起始位置
         * @return 首次出现的下标，未找到时返回 npos
         */
        constexpr size_type find(const CharType *s, size_type pos = 0) const {
            return find(basic_string_view(s), pos);
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of the given substring, searching backward from pos.
         *
         * @param s The substring to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找给定子串最后一次出现的位置。
         *
         * @param s 待搜索的子串
         * @param pos 反向搜索的起始位置
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        constexpr size_type rfind(basic_string_view s, size_type pos = npos) const noexcept {
            if (s.size_ == 0) {
                return (core::min) (pos, size_);
            }
            if (s.size_ > size_) {
                return npos;
            }
            pos = (core::min) (pos, size_ - s.size_);
            // NOLINTBEGIN
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto p = data_ + pos;; --p) {
                    bool match = true;
                    for (size_type i = 0; i < s.size_; ++i) {
                        if (!traits_type::eq(p[i], s.data_[i])) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return static_cast<size_type>(p - data_);
                    }
                    if (p == data_) {
                        break;
                    }
                }
                return npos;
            }
#endif
            // NOLINTEND
            for (auto p = data_ + pos;; --p) {
                if (traits_type::compare(p, s.data_, s.size_) == 0) {
                    return p - data_;
                }
                if (p == data_) {
                    break;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of the given character, searching backward from pos.
         *
         * @param c The character to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找给定字符最后一次出现的位置。
         *
         * @param c 待搜索的字符
         * @param pos 反向搜索的起始位置
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        constexpr size_type rfind(CharType c, size_type pos = npos) const noexcept {
            if (size_ == 0) {
                return npos;
            }
            pos = (core::min) (pos, size_ - 1);
            for (auto p = data_ + pos;; --p) {
                if (traits_type::eq(*p, c)) {
                    return p - data_;
                }
                if (p == data_) {
                    break;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of a character sequence of explicit length, searching backward from pos.
         *
         * @param s The character sequence to search for
         * @param pos The position at which to start the backward search
         * @param n The length of the character sequence
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找指定长度的字符序列最后一次出现的位置。
         *
         * @param s 待搜索的字符序列
         * @param pos 反向搜索的起始位置
         * @param n 字符序列的长度
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        constexpr size_type rfind(const CharType *s, size_type pos, size_type n) const {
            return rfind(basic_string_view(s, n), pos);
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of a null-terminated character string, searching backward from pos.
         *
         * @param s The null-terminated string to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找以空字符结尾的字符串最后一次出现的位置。
         *
         * @param s 待搜索的以空字符结尾的字符串
         * @param pos 反向搜索的起始位置
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        constexpr size_type rfind(const CharType *s, size_type pos = npos) const {
            return rfind(basic_string_view(s), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character equal to any character of the given set, starting at pos.
         *
         * @param s The set of characters to search for
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符集合中任意字符相等的首个字符。
         *
         * @param s 待搜索的字符集合
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_of(basic_string_view s, size_type pos = 0) const noexcept {
            for (auto p = data_ + pos; p < data_ + size_; ++p) {
                if (s.find(*p) != npos) {
                    return p - data_;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of the given character starting at pos.
         *
         * @param c The character to search for
         * @param pos The position at which to start the search
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找给定字符首次出现的位置。
         *
         * @param c 待搜索的字符
         * @param pos 搜索的起始位置
         * @return 首次出现的下标，未找到时返回 npos
         */
        constexpr size_type find_first_of(CharType c, size_type pos = 0) const noexcept {
            return find(c, pos);
        }

        /**
         * \lang english
         * @brief Finds the first character equal to any character of the given sequence, starting at pos.
         *
         * @param s The character sequence to search for
         * @param pos The position at which to start the search
         * @param n The length of the character sequence
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符序列中任意字符相等的首个字符。
         *
         * @param s 待搜索的字符序列
         * @param pos 搜索的起始位置
         * @param n 字符序列的长度
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_of(const CharType *s, size_type pos, size_type n) const {
            return find_first_of(basic_string_view(s, n), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character equal to any character of the given null-terminated string, starting at pos.
         *
         * @param s The null-terminated string to search for
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定以空字符结尾的字符串中任意字符相等的首个字符。
         *
         * @param s 待搜索的以空字符结尾的字符串
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_of(const CharType *s, size_type pos = 0) const {
            return find_first_of(basic_string_view(s), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character equal to any character of the given set, searching backward from pos.
         *
         * @param s The set of characters to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符集合中任意字符相等的末个字符。
         *
         * @param s 待搜索的字符集合
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_of(basic_string_view s, size_type pos = npos) const noexcept {
            if (size_ == 0) {
                return npos;
            }
            pos = (core::min) (pos, size_ - 1);
            for (auto p = data_ + pos;; --p) {
                if (s.find(*p) != npos) {
                    return p - data_;
                }
                if (p == data_) {
                    break;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of the given character, searching backward from pos.
         *
         * @param c The character to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找给定字符最后一次出现的位置。
         *
         * @param c 待搜索的字符
         * @param pos 反向搜索的起始位置
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        constexpr size_type find_last_of(CharType c, size_type pos = npos) const noexcept {
            return rfind(c, pos);
        }

        /**
         * \lang english
         * @brief Finds the last character equal to any character of the given sequence, searching backward from pos.
         *
         * @param s The character sequence to search for
         * @param pos The position at which to start the backward search
         * @param n The length of the character sequence
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符序列中任意字符相等的末个字符。
         *
         * @param s 待搜索的字符序列
         * @param pos 反向搜索的起始位置
         * @param n 字符序列的长度
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_of(const CharType *s, size_type pos, size_type n) const {
            return find_last_of(basic_string_view(s, n), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character equal to any character of the given null-terminated string, searching backward from pos.
         *
         * @param s The null-terminated string to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定以空字符结尾的字符串中任意字符相等的末个字符。
         *
         * @param s 待搜索的以空字符结尾的字符串
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_of(const CharType *s, size_type pos = npos) const {
            return find_last_of(basic_string_view(s), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character not equal to any character of the given set, starting at pos.
         *
         * @param s The set of characters to exclude
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符集合中任意字符都不相等的首个字符。
         *
         * @param s 需要排除的字符集合
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_not_of(basic_string_view s, size_type pos = 0) const noexcept {
            for (auto p = data_ + pos; p < data_ + size_; ++p) {
                if (s.find(*p) == npos) {
                    return p - data_;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the first character different from the given character, starting at pos.
         *
         * @param c The character to exclude
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符不相等的首个字符。
         *
         * @param c 需要排除的字符
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_not_of(CharType c, size_type pos = 0) const noexcept {
            for (auto p = data_ + pos; p < data_ + size_; ++p) {
                if (!traits_type::eq(*p, c)) {
                    return p - data_;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the first character not equal to any character of the given sequence, starting at pos.
         *
         * @param s The character sequence to exclude
         * @param pos The position at which to start the search
         * @param n The length of the character sequence
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符序列中任意字符都不相等的首个字符。
         *
         * @param s 需要排除的字符序列
         * @param pos 搜索的起始位置
         * @param n 字符序列的长度
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_not_of(const CharType *s, size_type pos, size_type n) const {
            return find_first_not_of(basic_string_view(s, n), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character not equal to any character of the given null-terminated string, starting at pos.
         *
         * @param s The null-terminated string to exclude
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定以空字符结尾的字符串中任意字符都不相等的首个字符。
         *
         * @param s 需要排除的以空字符结尾的字符串
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_first_not_of(const CharType *s, size_type pos = 0) const {
            return find_first_not_of(basic_string_view(s), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character not equal to any character of the given set, searching backward from pos.
         *
         * @param s The set of characters to exclude
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符集合中任意字符都不相等的末个字符。
         *
         * @param s 需要排除的字符集合
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_not_of(basic_string_view s, size_type pos = npos) const noexcept {
            if (size_ == 0) {
                return npos;
            }
            pos = (core::min) (pos, size_ - 1);
            for (auto p = data_ + pos;; --p) {
                if (s.find(*p) == npos) {
                    return p - data_;
                }
                if (p == data_) {
                    break;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last character different from the given character, searching backward from pos.
         *
         * @param c The character to exclude
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符不相等的末个字符。
         *
         * @param c 需要排除的字符
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_not_of(CharType c, size_type pos = npos) const noexcept {
            if (size_ == 0) {
                return npos;
            }
            if (pos >= size_) {
                pos = size_ - 1;
            }
            for (size_type i = pos + 1; i > 0; --i) {
                if (!traits_type::eq(data_[i - 1], c)) {
                    return i - 1;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last character not equal to any character of the given sequence, searching backward from pos.
         *
         * @param s The character sequence to exclude
         * @param pos The position at which to start the backward search
         * @param n The length of the character sequence
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符序列中任意字符都不相等的末个字符。
         *
         * @param s 需要排除的字符序列
         * @param pos 反向搜索的起始位置
         * @param n 字符序列的长度
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_not_of(const CharType *s, size_type pos, size_type n) const {
            return find_last_not_of(basic_string_view(s, n), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character not equal to any character of the given null-terminated string, searching backward from pos.
         *
         * @param s The null-terminated string to exclude
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定以空字符结尾的字符串中任意字符都不相等的末个字符。
         *
         * @param s 需要排除的以空字符结尾的字符串
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        constexpr size_type find_last_not_of(const CharType *s, size_type pos = npos) const {
            return find_last_not_of(basic_string_view(s), pos);
        }

        /**
         * \lang english
         * @brief Formats the given arguments according to the format string and appends the result to a temporary string.
         *
         * @tparam Args The argument types
         * @param fmt The format string
         * @param args The arguments to format
         * @return A reference to the resulting string
         *
         * \lang simp-chinese
         * @brief 按照格式字符串格式化给定参数，并将结果追加到临时字符串中。
         *
         * @tparam Args 参数类型
         * @param fmt 格式字符串
         * @param args 待格式化的参数
         * @return 结果字符串的引用
         */
        template <typename... Args>
        basic_string &format(basic_string_view<value_type> fmt, Args const &...args); // implement in format.hpp

        /**
         * \lang english
         * @brief Formats the given arguments according to the format string and returns a new string.
         *
         * @tparam Args The argument types
         * @param fmt The format string
         * @param args The arguments to format
         * @return The formatted string
         *
         * \lang simp-chinese
         * @brief 按照格式字符串格式化给定参数，并返回新字符串。
         *
         * @tparam Args 参数类型
         * @param fmt 格式字符串
         * @param args 待格式化的参数
         * @return 格式化后的字符串
         */
        template <typename... Args>
        basic_string format_copy(basic_string_view<value_type> fmt, Args const &...args) const; // implement in format.hpp

        /**
         * \lang english
         * @brief Checks whether the left string view is equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否与右侧字符串视图相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator==(basic_string_view left, basic_string_view right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
            if (lsize != rsize) {
                return false;
            }
            return Traits::compare(left.data(), right.data(), lsize) == 0;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is not equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否与右侧字符串视图不相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator!=(basic_string_view left, basic_string_view right) noexcept {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically less than the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序小于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator<(basic_string_view left, basic_string_view right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
            auto result = Traits::compare(left.data(), right.data(), (core::min) (lsize, rsize));
            if (result < 0) {
                return true;
            }
            if (result > 0) {
                return false;
            }
            return lsize < rsize;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically greater than the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序大于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator>(basic_string_view left, basic_string_view right) noexcept {
            return right < left;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically less than or equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序小于或等于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator<=(basic_string_view left, basic_string_view right) noexcept {
            return !(right < left);
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically greater than or equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序大于或等于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator>=(basic_string_view left, basic_string_view right) noexcept {
            return !(left < right);
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否与字符指针相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator==(basic_string_view left, const CharType *right) noexcept {
            return left == basic_string_view(right);
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否与右侧字符串视图相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator==(const CharType *left, basic_string_view right) noexcept {
            return basic_string_view(left) == right;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is not equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否与字符指针不相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator!=(basic_string_view left, const CharType *right) noexcept {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is not equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否与右侧字符串视图不相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator!=(const CharType *left, basic_string_view right) noexcept {
            return !(basic_string_view(left) == right);
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically less than the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序小于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator<(basic_string_view left, const CharType *right) noexcept {
            return left < basic_string_view(right);
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is lexicographically less than the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否按字典序小于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator<(const CharType *left, basic_string_view right) noexcept {
            return basic_string_view(left) < right;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically greater than the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序大于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator>(basic_string_view left, const CharType *right) noexcept {
            return left > basic_string_view(right);
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is lexicographically greater than the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否按字典序大于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator>(const CharType *left, basic_string_view right) noexcept {
            return basic_string_view(left) > right;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically less than or equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序小于或等于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator<=(basic_string_view left, const CharType *right) noexcept {
            return left <= basic_string_view(right);
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is lexicographically less than or equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否按字典序小于或等于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator<=(const CharType *left, basic_string_view right) noexcept {
            return basic_string_view(left) <= right;
        }

        /**
         * \lang english
         * @brief Checks whether the left string view is lexicographically greater than or equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串视图是否按字典序大于或等于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator>=(basic_string_view left, const CharType *right) noexcept {
            return left >= basic_string_view(right);
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is lexicographically greater than or equal to the right string view.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否按字典序大于或等于右侧字符串视图。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend constexpr bool operator>=(const CharType *left, basic_string_view right) noexcept {
            return basic_string_view(left) >= right;
        }

    private:
        static constexpr bool equal_(CharType const *begin, CharType const *end, CharType const *first,
                                     CharType const *last) noexcept {
            if (last - first != end - begin) {
                return false;
            }
            for (; begin != end; ++begin, ++first) {
                if (*first != *begin) {
                    return false;
                }
            }
            return true;
        }

        constexpr void range_check_access(size_type pos) const {
            if (pos >= size_) {
                throw std::out_of_range("basic_string_view: index out of range");
            }
        }

        constexpr void range_check_substr(size_type pos) const {
            if (pos > size_) {
                throw std::out_of_range("basic_string_view: position out of range");
            }
        }

        const_pointer data_{nullptr};
        size_type size_{0};
    };

    /**
     * \lang english
     * @brief Deduction guide that deduces the character type from an iterator pair.
     *
     * \lang simp-chinese
     * @brief 从迭代器对推导字符类型的推导指引。
     */
    template <typename It, class End>
    basic_string_view(It, End) -> basic_string_view<type_traits::extras::iterators::iter_value_t<It>>;

    /**
     * \lang english
     * @brief Type alias for basic_string_view of char.
     *
     * \lang simp-chinese
     * @brief char 类型的 basic_string_view 别名。
     */
    using string_view = basic_string_view<char>;
    /**
     * \lang english
     * @brief Type alias for basic_string_view of wchar_t.
     *
     * \lang simp-chinese
     * @brief wchar_t 类型的 basic_string_view 别名。
     */
    using wstring_view = basic_string_view<wchar_t>;
    /**
     * \lang english
     * @brief Type alias for basic_string_view of char16_t.
     *
     * \lang simp-chinese
     * @brief char16_t 类型的 basic_string_view 别名。
     */
    using u16string_view = basic_string_view<char16_t>;
    /**
     * \lang english
     * @brief Type alias for basic_string_view of char32_t.
     *
     * \lang simp-chinese
     * @brief char32_t 类型的 basic_string_view 别名。
     */
    using u32string_view = basic_string_view<char32_t>;
#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Type alias for basic_string_view of char8_t.
     *
     * \lang simp-chinese
     * @brief char8_t 类型的 basic_string_view 别名。
     */
    using u8string_view = basic_string_view<char8_t>;
#endif

    template <typename Elem, typename UTy>
    /**
     * \lang english
     * @brief Writes the string view to the output stream.
     *
     * @param left The output stream
     * @param right The string view to write
     * @return A reference to the output stream
     *
     * \lang simp-chinese
     * @brief 将字符串视图写入输出流。
     *
     * @param left 输出流
     * @param right 待写入的字符串视图
     * @return 输出流的引用
     */
    RAINY_INLINE std::basic_ostream<Elem> &operator<<(std::basic_ostream<Elem> &left, basic_string_view<UTy> right) {
        return left.write(right.data(), right.size());
    }
}

namespace rainy::text {
    using core::text::basic_string_view;
    using core::text::string_view;
    using core::text::wstring_view;
    using core::text::u16string_view;
    using core::text::u32string_view;
#if RAINY_HAS_CXX20
    using core::text::u8string_view;
#endif
}

#endif
