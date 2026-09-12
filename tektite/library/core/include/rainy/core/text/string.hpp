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
#ifndef RAINY_CORE_YESOD_TEXT_STRING_HPP
#define RAINY_CORE_YESOD_TEXT_STRING_HPP

// NOLINTBEGIN
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/char_traits.hpp>
#include <rainy/core/text/fwd.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/utility/hash.hpp>
// NOLINTEND

#if RAINY_USING_GCC && !RAINY_USING_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

#if !RAINY_USING_MACOS
#pragma GCC diagnostic ignored "-Wstringop-overflow" // Ok...
#endif

#endif

namespace rainy::core::text {
    /**
     * \lang english
     * @brief An owning, mutable string with small-string optimization (SSO).
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Allocator The allocator type
     *
     * \lang simp-chinese
     * @brief 具有小字符串优化（SSO）的拥有型可变字符串。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Allocator 分配器类型
     */
    template <typename CharType, typename Traits, typename Allocator>
    class basic_string {
    public:
        // NOLINTBEGIN

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
         * @brief The allocator type.
         *
         * \lang simp-chinese
         * @brief 分配器类型。
         */
        using allocator_type = Allocator;
        /**
         * \lang english
         * @brief The unsigned size type.
         *
         * \lang simp-chinese
         * @brief 无符号大小类型。
         */
        using size_type = typename memory::allocator_traits<Allocator>::size_type;
        /**
         * \lang english
         * @brief The signed difference type.
         *
         * \lang simp-chinese
         * @brief 有符号差值类型。
         */
        using difference_type = typename memory::allocator_traits<Allocator>::difference_type;
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
        using const_reference = value_type const &;
        /**
         * \lang english
         * @brief The pointer type.
         *
         * \lang simp-chinese
         * @brief 指针类型。
         */
        using pointer = typename memory::allocator_traits<Allocator>::pointer;
        /**
         * \lang english
         * @brief The constant pointer type.
         *
         * \lang simp-chinese
         * @brief 常量指针类型。
         */
        using const_pointer = typename memory::allocator_traits<Allocator>::const_pointer;
        /**
         * \lang english
         * @brief The allocator traits type.
         *
         * \lang simp-chinese
         * @brief 分配器 traits 类型。
         */
        using allocator_traits = memory::allocator_traits<allocator_type>;
        /**
         * \lang english
         * @brief The mutable iterator type.
         *
         * \lang simp-chinese
         * @brief 可变迭代器类型。
         */
        using iterator = value_type *;
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
         * @brief The reverse iterator type.
         *
         * \lang simp-chinese
         * @brief 反向迭代器类型。
         */
        using reverse_iterator = utility::reverse_iterator<iterator>;
        /**
         * \lang english
         * @brief The constant reverse iterator type.
         *
         * \lang simp-chinese
         * @brief 常量反向迭代器类型。
         */
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        // NOLINTEND

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
         * @brief The default capacity used for short-string storage.
         *
         * \lang simp-chinese
         * @brief 短字符串存储使用的默认容量。
         */
        static constexpr std::size_t default_capacity{sizeof(CharType *) * 4 / sizeof(CharType) - 2};

        /**
         * \lang english
         * @brief Constructs an empty string.
         *
         * \lang simp-chinese
         * @brief 构造一个空字符串。
         */
        RAINY_CONSTEXPR20 basic_string() noexcept : pair_{} {
            get_storage().start = get_storage().storage_union_.localbuffer;
        }

        /**
         * \lang english
         * @brief Constructs an empty string with the given allocator.
         *
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用给定的分配器构造一个空字符串。
         *
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(allocator_type const &allocator) noexcept : pair_{allocator, {}} { // NOLINT
            get_storage().start = get_storage().storage_union_.localbuffer;
        }

        /**
         * \lang english
         * @brief Constructs a string with count copies of the character ch.
         *
         * @param count The number of characters
         * @param ch The character to fill with
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 构造一个包含 count 个字符 ch 副本的字符串。
         *
         * @param count 字符数量
         * @param ch 用于填充的字符
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(size_type count, value_type ch, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            if (count > 0) {
                allocate_plus_one_(count);
                auto begin = begin_();
                std::fill(begin, begin + count, ch);
            }
            resize_(count);
        }

        /**
         * \lang english
         * @brief Constructs a string from the substring [pos, pos + count) of right.
         *
         * @param right The source string
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由 right 的子串 [pos, pos + count) 构造字符串。
         *
         * @param right 源字符串
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, size_type count,
                                       allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            auto other_size = right.size();
            if (pos > other_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (other_size - pos, count);
            allocate_plus_one_(count);
            auto start = right.begin_() + pos;
            fill_(start, start + count);
            resize_(count);
        }

        /**
         * \lang english
         * @brief Constructs a string from the substring of right starting at pos.
         *
         * @param right The source string
         * @param pos The starting position of the substring
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由 right 从 pos 开始的子串构造字符串。
         *
         * @param right 源字符串
         * @param pos 子串的起始位置
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(const basic_string &right, size_type pos, allocator_type const &allocator = allocator_type()) :
            basic_string(right, pos, right.size() - pos, allocator) {
        }

        /**
         * \lang english
         * @brief Constructs a string from the first count characters of the given character array.
         *
         * @param ptr The character array
         * @param count The number of characters
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由给定字符数组的前 count 个字符构造字符串。
         *
         * @param ptr 字符数组
         * @param count 字符数量
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(const value_type *ptr, size_type count, allocator_type const &allocator = allocator_type()) :
            pair_{allocator, {}} {
            allocate_plus_one_(count);
            fill_(ptr, ptr + count);
            resize_(count);
        }

        /**
         * \lang english
         * @brief Constructs a string from a null-terminated character string.
         *
         * @param ptr The null-terminated character string
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由以空字符结尾的字符串构造字符串。
         *
         * @param ptr 以空字符结尾的字符串
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(const value_type *ptr, allocator_type const &allocator = allocator_type()) : // NOLINT
            basic_string(ptr, traits_type::length(ptr), allocator) {
        }

        /**
         * \lang english
         * @brief Constructs a string from the range [first, last).
         *
         * @tparam InputIt The iterator type
         * @param first Iterator to the first character
         * @param last Iterator one past the last character
         *
         * \lang simp-chinese
         * @brief 由区间 [first, last) 构造字符串。
         *
         * @tparam InputIt 迭代器类型
         * @param first 指向首字符的迭代器
         * @param last 指向末字符之后位置的迭代器
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string(InputIt first, InputIt last) {
            init_empty_();
            if constexpr (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto length = utility::distance(first, last);
                allocate_plus_one_(length);
                core::algorithm::copy(first, last, begin_());
                resize_(length);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
        }

        /**
         * \lang english
         * @brief Copy constructor.
         *
         * @param right The string to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         *
         * @param right 拷贝来源字符串
         */
        RAINY_CONSTEXPR20 basic_string(const basic_string &right) :
            pair_{allocator_traits::select_on_container_copy_construction(right.get_al()), {}} {
            auto other_size = right.size();
            allocate_plus_one_(other_size);
            fill_(right.begin_(), right.end_());
            resize_(other_size);
        }

        /**
         * \lang english
         * @brief Copy constructor with an explicit allocator.
         *
         * @param right The string to copy from
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用显式分配器的拷贝构造函数。
         *
         * @param right 拷贝来源字符串
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(const basic_string &right, allocator_type const &allocator) : pair_{allocator, {}} {
            auto other_size = right.size();
            allocate_plus_one_(other_size);
            fill_(right.begin_(), right.end_());
            resize_(other_size);
        }

        /**
         * \lang english
         * @brief Constructs a string from a substring of a right-value string, stealing its storage when possible.
         *
         * @param right The right-value source string
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由右值字符串的子串构造字符串，尽可能窃取其存储。
         *
         * @param right 右值源字符串
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(basic_string &&right, size_type pos, size_type count,
                                       allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            init_empty_();
            auto other_size = right.size();
            if (pos > other_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (other_size - pos, count);
            if (pos != 0) {
                auto other_begin = right.begin_();
                auto start = other_begin + pos;
                auto last = start + count;
                // NOLINTBEGIN
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    algorithm::copy(start, last, other_begin);
                } else
#endif
                {
                    std::memmove(other_begin, start, (last - start) * sizeof(CharType));
                }
                // NOLINTEND
            }
            right.resize_(count);
            right.swap(*this);
        }

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param right The string to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param right 移动来源字符串
         */
        RAINY_CONSTEXPR20 basic_string(basic_string &&right) noexcept : pair_{{}, {}} {
            init_empty_();
            if constexpr (allocator_traits::propagate_on_container_swap::value) {
                using std::swap;
                swap(this->get_al(), right.get_al());
            } else {
                assert(right.get_al() == this->get_al());
            }
            right.swap_without_ator(*this);
        }

        /**
         * \lang english
         * @brief Move constructor with an explicit allocator.
         *
         * @param right The string to move from
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 使用显式分配器的移动构造函数。
         *
         * @param right 移动来源字符串
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(basic_string &&right, allocator_type const &allocator) : pair_{allocator, {}} {
            init_empty_();
            if (right.get_al() == allocator) {
                right.swap_without_ator(*this);
            } else {
                basic_string temp{right.data(), right.size(), allocator};
                temp.swap(*this);
                right.swap(temp);
            }
        }

        /**
         * \lang english
         * @brief Constructs a string from an initializer list.
         *
         * @param ilist The initializer list
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由初始化列表构造字符串。
         *
         * @param ilist 初始化列表
         * @param allocator 使用的分配器
         */
        RAINY_CONSTEXPR20 basic_string(std::initializer_list<CharType> ilist, allocator_type const &allocator = allocator_type()) :
            basic_string(std::data(ilist), ilist.size(), allocator) {
        }

        /**
         * \lang english
         * @brief Constructs a string from a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由类似字符串视图的类型构造字符串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @param allocator 使用的分配器
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &sv, allocator_type const &allocator = allocator_type()) : // NOLINT
            basic_string(sv.data(), sv.size(), allocator) {
        }

        /**
         * \lang english
         * @brief Constructs a string from a substring of a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @param allocator The allocator to use
         *
         * \lang simp-chinese
         * @brief 由类似字符串视图类型的子串构造字符串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @param allocator 使用的分配器
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string(const StringViewLike &sv, size_type pos, size_type count,
                                       allocator_type const &allocator = allocator_type()) : pair_{allocator, {}} {
            auto data = sv.data();
            auto sv_size = sv.size();
            if (pos > sv_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (sv_size - pos, count);
            allocate_plus_one_(count);
            fill_(data + pos, data + pos + count);
            resize_(count);
        }

        /**
         * \lang english
         * @brief Destroys the string and releases its storage.
         *
         * \lang simp-chinese
         * @brief 销毁字符串并释放其存储。
         */
        RAINY_CONSTEXPR20 ~basic_string() {
            if (is_long_()) {
                dealloc_(get_storage().start, get_storage().storage_union_.allocated_capacity);
            }
        }

        /**
         * \lang english
         * @brief Checks whether the string is empty.
         *
         * @return true if the string is empty, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否为空。
         *
         * @return 若字符串为空返回 true，否则返回 false
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 bool empty() const noexcept {
            return size() == 0;
        }

        /**
         * \lang english
         * @brief Returns the number of characters in the string.
         *
         * @return The number of characters
         *
         * \lang simp-chinese
         * @brief 返回字符串中的字符数量。
         *
         * @return 字符数量
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 std::size_t size() const noexcept {
            return get_storage().mylength;
        }

        /**
         * \lang english
         * @brief Returns the number of characters in the string.
         *
         * @return The number of characters
         *
         * \lang simp-chinese
         * @brief 返回字符串中的字符数量。
         *
         * @return 字符数量
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 std::size_t length() const noexcept {
            return size();
        }

        /**
         * \lang english
         * @brief Returns the maximum number of characters the string can hold.
         *
         * @return The maximum size
         *
         * \lang simp-chinese
         * @brief 返回字符串可容纳的最大字符数。
         *
         * @return 最大大小
         */
        RAINY_NODISCARD RAINY_CONSTEXPR20 static size_type max_size() noexcept {
            return static_cast<size_type>(-1) / sizeof(CharType) / 2;
        }

        /**
         * \lang english
         * @brief Returns the number of characters the string can currently hold without reallocation.
         *
         * @return The current capacity
         *
         * \lang simp-chinese
         * @brief 返回字符串在不重新分配的情况下当前可容纳的字符数。
         *
         * @return 当前容量
         */
        RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            if (is_long_()) {
                return get_storage().storage_union_.allocated_capacity - 1;
            }
            return short_string_max_;
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
        RAINY_CONSTEXPR20 value_type const *data() const noexcept {
            return begin_();
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
        RAINY_CONSTEXPR20 CharType *data() noexcept {
            return begin_();
        }

        /**
         * \lang english
         * @brief Returns a pointer to a null-terminated character array.
         *
         * @return A pointer to a null-terminated character array
         *
         * \lang simp-chinese
         * @brief 返回指向以空字符结尾的字符数组的指针。
         *
         * @return 指向以空字符结尾的字符数组的指针
         */
        RAINY_CONSTEXPR20 value_type const *c_str() const noexcept {
            return begin_();
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
        RAINY_CONSTEXPR20 const_reference at(size_type pos) const {
            if (pos >= size()) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            return *(begin_() + pos);
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
        RAINY_CONSTEXPR20 reference at(size_type pos) {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).at(pos));
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
        RAINY_CONSTEXPR20 reference operator[](size_type pos) noexcept {
#if RAINY_HAS_CXX20
            if (!std::is_constant_evaluated()) {
                assert(("pos >= size, please check the arg" && pos < size()));
            }
#else
            assert(("pos >= size, please check the arg" && pos < size()));
#endif
            return const_cast<CharType &>(const_cast<basic_string const &>(*this)[pos]);
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
        RAINY_CONSTEXPR20 const_reference operator[](size_type pos) const noexcept {
#if RAINY_HAS_CXX20
            if (!std::is_constant_evaluated()) {
                assert(("pos >= size, please check the arg" && pos < size()));
            }
#else
            assert(("pos >= size, please check the arg" && pos < size()));
#endif
            return *(begin_() + pos);
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
        RAINY_CONSTEXPR20 const CharType &front() const noexcept {
            assert(("string is empty" && !empty()));
            return *begin_();
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
        RAINY_CONSTEXPR20 CharType &front() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).front());
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
        RAINY_CONSTEXPR20 const CharType &back() const noexcept {
            assert(("string is empty" && !empty()));
            return *(end_() - 1);
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
        RAINY_CONSTEXPR20 CharType &back() {
            return const_cast<CharType &>(const_cast<basic_string const &>(*this).back());
        }

        /**
         * \lang english
         * @brief Converts the string to a basic_string_view.
         *
         * @return A string view over the string contents
         *
         * \lang simp-chinese
         * @brief 将字符串转换为 basic_string_view。
         *
         * @return 覆盖字符串内容的字符串视图
         */
        RAINY_CONSTEXPR20 operator basic_string_view<value_type>() const noexcept { // NOLINT
            return basic_string_view<value_type>(begin_(), end_());
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
        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return {this->begin_()};
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
        RAINY_CONSTEXPR20 iterator end() noexcept {
            return {this->end_()};
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
        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return const_iterator{this->begin_()};
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
        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return const_iterator{this->end_()};
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
        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return begin();
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
        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return end();
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
        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
            return reverse_iterator{this->end_()};
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
        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
            return reverse_iterator{this->begin_()};
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
        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator{this->end_()};
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
        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator{this->begin_()};
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
        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() const noexcept {
            return rbegin();
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
        RAINY_CONSTEXPR20 const_reverse_iterator crend() const noexcept {
            return rend();
        }

        /**
         * \lang english
         * @brief Requests the removal of unused capacity.
         *
         * \lang simp-chinese
         * @brief 请求移除未使用的容量。
         */
        RAINY_CONSTEXPR20 void shrink_to_fit() noexcept { // NOLINT
            auto current_size = size();
            // Case 1: long -> short
            if (current_size <= short_string_max_ && is_long_()) {
                auto old_start = get_storage().start;
                auto old_allocated = get_storage().storage_union_.allocated_capacity;
                // NOLINTBEGIN
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    for (size_type i = 0; i < current_size; ++i) {
                        get_storage().storage_union_.localbuffer[i] = old_start[i];
                    }
                } else
#endif
                {
                    traits_type::copy(get_storage().storage_union_.localbuffer, old_start, current_size);
                }
                // NOLINTEND
                get_storage().storage_union_.localbuffer[current_size] = CharType{};
                get_storage().start = get_storage().storage_union_.localbuffer;
                get_storage().mylength = current_size;
                dealloc_(old_start, old_allocated);
                return;
            }
            // Case 2: long -> long with smaller capacity
            if (is_long_()) {
                auto current_capacity = capacity();
                if (current_capacity <= current_size + current_size / 2) {
                    return;
                }
                const auto new_capacity = current_size;
                auto new_ptr = allocator_traits::allocate(get_al(), new_capacity + 1);
                // NOLINTBEGIN
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    for (size_type i = 0; i < current_size; ++i) {
                        new_ptr[i] = get_storage().start[i];
                    }
                } else
#endif
                {
                    traits_type::copy(new_ptr, get_storage().start, current_size);
                }
                // NOLINTEND
                new_ptr[current_size] = CharType{};
                dealloc_(get_storage().start, get_storage().storage_union_.allocated_capacity);
                get_storage().start = new_ptr;
                get_storage().storage_union_.allocated_capacity = new_capacity + 1;
                get_storage().mylength = current_size;
            }
            // Case 3: short, nothing to do
        }

        /**
         * \lang english
         * @brief Ensures the string can hold at least new_cap characters without reallocation.
         *
         * @param new_cap The minimum capacity to ensure
         *
         * \lang simp-chinese
         * @brief 确保字符串在不重新分配的情况下至少可容纳 new_cap 个字符。
         *
         * @param new_cap 需要确保的最小容量
         */
        RAINY_CONSTEXPR20 void reserve(size_type new_cap) {
            if (capacity() >= new_cap) {
                return;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                basic_string temp{};
                temp.allocate_plus_one_(new_cap);
                for (const auto ch: *this) {
                    temp.push_back(ch);
                }
                temp.swap_without_ator(*this);
            } else
#endif
            {
                if (is_long_()) {
                    auto old_start = get_storage().start;
                    auto old_allocated = get_storage().storage_union_.allocated_capacity;
                    auto old_mylength = get_storage().mylength;
                    allocate_plus_one_(new_cap);
                    traits_type::copy(get_storage().start, old_start, old_mylength);
                    dealloc_(old_start, old_allocated);
                    get_storage().mylength = old_mylength;
                } else {
                    // Save local buffer data BEFORE allocate_plus_one_, because it
                    // writes to storage_union_.allocated_capacity which shares the
                    // same memory as localbuffer via the union, corrupting the data.
                    CharType local_copy[short_string_max_ + 1];
                    auto old_size = get_storage().mylength;
                    traits_type::copy(local_copy, get_storage().storage_union_.localbuffer, old_size);
                    allocate_plus_one_(new_cap);
                    traits_type::copy(get_storage().start, local_copy, old_size);
                    get_storage().mylength = old_size;
                }
            }
        }

        /**
         * \lang english
         * @brief Resizes the string to contain count characters, filling new elements with ch.
         *
         * @param count The new size
         * @param ch The character to fill new elements with
         *
         * \lang simp-chinese
         * @brief 将字符串大小调整为 count 个字符，用 ch 填充新增元素。
         *
         * @param count 新的大小
         * @param ch 用于填充新增元素的字符
         */
        RAINY_CONSTEXPR20 void resize(size_type count, value_type ch) {
            auto size = this->size();
            if (count > size) {
                reserve(count);
                auto begin = begin_();
                std::fill(begin + size, begin + count, ch);
            }
            resize_(count);
        }

        /**
         * \lang english
         * @brief Resizes the string to contain count characters.
         *
         * @param count The new size
         *
         * \lang simp-chinese
         * @brief 将字符串大小调整为 count 个字符。
         *
         * @param count 新的大小
         */
        RAINY_CONSTEXPR20 void resize(size_type count) {
            resize(count, CharType{});
        }

        /**
         * \lang english
         * @brief Resizes the string and lets the caller overwrite the new elements.
         *
         * @tparam Operation The overwrite operation type
         * @param count The new size
         * @param op The operation invoked with the writable buffer
         *
         * \lang simp-chinese
         * @brief 调整字符串大小并允许调用方覆写新元素。
         *
         * @tparam Operation 覆写操作类型
         * @param count 新的大小
         * @param op 以可写缓冲区为参数调用的操作
         */
        template <typename Operation>
        RAINY_CONSTEXPR20 void resize_and_overwrite(size_type count, Operation op) {
            if (count > max_size()) {
                core::implements::throw_exception_length_error("resize_and_overwrite: count exceeds max_size");
            }
            if (count > capacity()) {
                reserve(count);
            }
            CharType *ptr = data();
            size_type new_len = utility::move(op)(ptr, count);
            assert(new_len <= count && "resize_and_overwrite: operation returned length exceeding count");
            this->resize_(new_len);
        }

        /**
         * \lang english
         * @brief Removes all characters.
         *
         * \lang simp-chinese
         * @brief 移除所有字符。
         */
        RAINY_CONSTEXPR20 void clear() noexcept {
            resize_(0);
        }

        /**
         * \lang english
         * @brief Appends a character to the end of the string.
         *
         * @param ch The character to append
         *
         * \lang simp-chinese
         * @brief 将字符追加到字符串末尾。
         *
         * @param ch 待追加的字符
         */
        RAINY_CONSTEXPR20 void push_back(value_type ch) {
            auto size = this->size();
            if (capacity() == size) {
                reserve(size * 2 - size / 2);
            }
            *end_() = ch;
            resize_(size + 1);
        }

        /**
         * \lang english
         * @brief Exchanges the contents of the two strings.
         *
         * @param right The string to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个字符串的内容。
         *
         * @param right 与之交换的字符串
         */
        RAINY_CONSTEXPR20 void swap(basic_string &right) noexcept {
            if constexpr (memory::allocator_traits<Allocator>::propagate_on_container_swap::value) {
                std::swap(right.get_al(), right.get_al());
            } else {
                assert(right.get_al() == this->get_al());
            }
            right.swap_without_ator(*this);
        }

        /**
         * \lang english
         * @brief Exchanges the contents of the two strings.
         *
         * @param self The first string
         * @param right The second string
         *
         * \lang simp-chinese
         * @brief 交换两个字符串的内容。
         *
         * @param self 第一个字符串
         * @param right 第二个字符串
         */
        friend void swap(basic_string &self, basic_string &right) noexcept {
            self.swap(right);
        }

        // NOLINTBEGIN
        /**
         * \lang english
         * @brief Deleted: a string cannot be constructed from a null pointer.
         *
         * \lang simp-chinese
         * @brief 已删除：禁止使用空指针构造字符串。
         */
        RAINY_CONSTEXPR20 basic_string(std::nullptr_t) = delete;
        /**
         * \lang english
         * @brief Deleted: a string cannot be assigned from a null pointer.
         *
         * \lang simp-chinese
         * @brief 已删除：禁止使用空指针赋值给字符串。
         */
        RAINY_CONSTEXPR20 basic_string &operator=(std::nullptr_t) = delete;
        // NOLINTEND

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right The string to move from
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 移动来源字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator=(basic_string &&right) noexcept(
            memory::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            memory::allocator_traits<Allocator>::is_always_equal::value) {
            assign(utility::move(right));
            return *this;
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         *
         * @param right The string to copy from
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         *
         * @param right 拷贝来源字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator=(const basic_string &right) {
            assign(right);
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns a null-terminated character string.
         *
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 赋值为以空字符结尾的字符串。
         *
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator=(const value_type *ptr) {
            assign(ptr);
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns a single character.
         *
         * @param ch The character to assign
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 赋值为单个字符。
         *
         * @param ch 待赋值的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator=(CharType ch) {
            resize_(1);
            *begin_() = ch;
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns the characters from the initializer list.
         *
         * @param ilist The initializer list
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 赋值为初始化列表中的字符。
         *
         * @param ilist 初始化列表
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator=(std::initializer_list<CharType> ilist) {
            assign(ilist);
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns the contents of a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 赋值为类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator=(const StringViewLike &sv) {
            assign(sv);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with count copies of the character ch.
         *
         * @param count The number of characters
         * @param ch The character to fill with
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为 count 个字符 ch 的副本。
         *
         * @param count 字符数量
         * @param ch 用于填充的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(size_type count, value_type ch) {
            reserve(count);
            auto begin = begin_();
            auto end = begin + count;
            std::fill(begin, end, ch);
            resize_(count);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with a copy of str.
         *
         * @param str The string to copy from
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为 str 的副本。
         *
         * @param str 拷贝来源字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(const basic_string &str) {
            if (utility::addressof(str) == this) {
                return *this;
            }
            if RAINY_CONSTEXPR20 (memory::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
                if (this->get_al() != str.get_al()) {
                    basic_string temp{this->get_al()};
                    temp.swap(*this);
                    this->get_al() = str.get_al();
                }
            }
            assign_(str.begin_(), str.end_());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with a substring of str.
         *
         * @param str The source string
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为 str 的子串。
         *
         * @param str 源字符串
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size();
            if (pos > str_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (str_size - pos, count));
            auto str_begin = str.begin_();
            assign_(str_begin + pos, str_begin + pos + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with those of right, stealing its storage when possible.
         *
         * @param right The string to move from
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为 right 的内容，尽可能窃取其存储。
         *
         * @param right 移动来源字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(basic_string &&right) noexcept { // NOLINT
            if RAINY_CONSTEXPR20 (memory::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
                right.swap(*this);
            } else {
                if (this->get_al() == right.get_al()) {
                    right.swap_without_ator(*this);
                } else {
                    assign_(right.begin_(), right.end_());
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with the first count characters of the given character array.
         *
         * @param ptr The character array
         * @param count The number of characters
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为给定字符数组的前 count 个字符。
         *
         * @param ptr 字符数组
         * @param count 字符数量
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(const value_type *ptr, size_type count) {
            assign_(ptr, ptr + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with a null-terminated character string.
         *
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为以空字符结尾的字符串。
         *
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(const value_type *ptr) {
            assign_(ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with the characters in the range [first, last).
         *
         * @tparam InputIt The iterator type
         * @param first Iterator to the first character
         * @param last Iterator one past the last character
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为区间 [first, last) 内的字符。
         *
         * @tparam InputIt 迭代器类型
         * @param first 指向首字符的迭代器
         * @param last 指向末字符之后位置的迭代器
         * @return 对此字符串的引用
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string &assign(InputIt first, InputIt last) {
            resize_(0);
            return append(first, last);
        }

        /**
         * \lang english
         * @brief Replaces the contents with the characters of the initializer list.
         *
         * @param ilist The initializer list
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为初始化列表中的字符。
         *
         * @param ilist 初始化列表
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &assign(std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            assign_(data, data + ilist.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        basic_string &assign(const StringViewLike &sv) {
            auto data = sv.data();
            assign_(data, data + sv.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the contents with a substring of a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将内容替换为类似字符串视图类型的子串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &assign(const StringViewLike &sv, size_type pos, size_type count = npos) {
            auto sv_size = sv.size();
            if (pos > sv_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (sv_size - pos, count));
            auto data = sv.data();
            assign_(data + pos, data + pos + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Appends the characters in the range [first, last).
         *
         * @tparam InputIt The iterator type
         * @param first Iterator to the first character
         * @param last Iterator one past the last character
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加区间 [first, last) 内的字符。
         *
         * @tparam InputIt 迭代器类型
         * @param first 指向首字符的迭代器
         * @param last 指向末字符之后位置的迭代器
         * @return 对此字符串的引用
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(InputIt first, InputIt last) {
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto size = this->size();
                auto length = utility::distance(first, last);
                auto new_size = size + length;
                reserve((core::max) (size * 2, new_size));
                core::algorithm::copy(first, last, begin_() + size);
                resize_(new_size);
            } else {
                for (; first != last; ++first) {
                    push_back(*first);
                }
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Appends count copies of the character ch.
         *
         * @param count The number of characters
         * @param ch The character to append
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加 count 个字符 ch 的副本。
         *
         * @param count 字符数量
         * @param ch 待追加的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &append(size_type count, value_type ch) {
            auto size = this->size();
            reserve(size + count);
            auto end = begin_() + size;
            std::fill(end, end + count, ch);
            resize_(size + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Appends the first count characters of the given character array.
         *
         * @param ptr The character array
         * @param count The number of characters
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加给定字符数组的前 count 个字符。
         *
         * @param ptr 字符数组
         * @param count 字符数量
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &append(const value_type *ptr, size_type count) {
            append_(ptr, ptr + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Appends a copy of str.
         *
         * @param str The string to append
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加 str 的副本。
         *
         * @param str 待追加的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &append(const basic_string &str) {
            auto begin = str.begin_();
            append_(begin, begin + str.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Appends a substring of str.
         *
         * @param str The source string
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加 str 的子串。
         *
         * @param str 源字符串
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &append(const basic_string &str, size_type pos, size_type count = npos) {
            auto str_size = str.size();
            if (pos > str_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (str_size - pos, count));
            return append(str.begin_() + pos, count);
        }

        /**
         * \lang english
         * @brief Appends a null-terminated character string.
         *
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加以空字符结尾的字符串。
         *
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &append(const value_type *ptr) {
            append_(ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        /**
         * \lang english
         * @brief Appends the characters of the initializer list.
         *
         * @param ilist The initializer list
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加初始化列表中的字符。
         *
         * @param ilist 初始化列表
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &append(std::initializer_list<CharType> ilist) {
            append_(ilist.begin(), ilist.end());
            return *this;
        }

        /**
         * \lang english
         * @brief Appends a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &sv) {
            auto data = sv.data();
            append_(data, data + sv.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Appends a substring of a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @param pos The starting position of the substring
         * @param count The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加类似字符串视图类型的子串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @param pos 子串的起始位置
         * @param count 子串的长度
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &append(const StringViewLike &sv, size_type pos, size_type count = npos) {
            auto sv_size = sv.size();
            if (pos > sv_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (sv_size - count, count));
            return append(sv.data() + pos, count);
        }

        /**
         * \lang english
         * @brief Appends a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The source view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 源视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &operator+=(const StringViewLike &sv) {
            return append(sv);
        }

        /**
         * \lang english
         * @brief Appends a copy of str.
         *
         * @param str The string to append
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加 str 的副本。
         *
         * @param str 待追加的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator+=(const basic_string &str) {
            return append(str);
        }

        /**
         * \lang english
         * @brief Appends a character.
         *
         * @param ch The character to append
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加一个字符。
         *
         * @param ch 待追加的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator+=(value_type ch) {
            push_back(ch);
            return *this;
        }

        /**
         * \lang english
         * @brief Appends a null-terminated character string.
         *
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加以空字符结尾的字符串。
         *
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator+=(const value_type *ptr) {
            return append(ptr);
        }

        /**
         * \lang english
         * @brief Appends the characters of the initializer list.
         *
         * @param ilist The initializer list
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 追加初始化列表中的字符。
         *
         * @param ilist 初始化列表
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &operator+=(std::initializer_list<CharType> ilist) {
            return append(ilist);
        }

        // NOLINTBEGIN
        /**
         * \lang english
         * @brief Finds the first occurrence of the given substring starting at pos.
         *
         * @param sv The substring to search for
         * @param pos The position at which to start the search
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找给定子串首次出现的位置。
         *
         * @param sv 待搜索的子串
         * @param pos 搜索的起始位置
         * @return 首次出现的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find(basic_string_view<value_type> sv, size_type pos = 0) const noexcept {
            const size_type self_size = size();
            const size_type sv_size = sv.size();
            if (pos > self_size || sv_size > self_size - pos) {
                return npos;
            }
            if (sv_size == 0) {
                return pos;
            }
            const value_type *self_begin = this->begin_();
            const size_type limit = self_size - sv_size;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = pos; i <= limit; ++i) {
                    bool match = true;
                    for (size_type j = 0; j < sv_size; ++j) {
                        if (!traits_type::eq(self_begin[i + j], sv[j])) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return i;
                    }
                }
                return npos;
            }
#endif
            for (size_type i = pos; i <= limit; ++i) {
                if (traits_type::compare(self_begin + i, sv.data(), sv_size) == 0) {
                    return i;
                }
            }
            return npos;
        }
        // NOLINTEND

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
        RAINY_CONSTEXPR20 size_type find(value_type c, size_type pos = 0) const noexcept {
            if (pos >= size()) {
                return npos;
            }
            // NOLINTBEGIN
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (auto p = this->begin_() + pos, end = this->end_(); p != end; ++p) {
                    if (traits_type::eq(*p, c)) {
                        return static_cast<size_type>(p - this->begin_());
                    }
                }
                return npos;
            }
#endif
            // NOLINTEND
            const value_type *result = traits_type::find(this->begin_() + pos, size() - pos, c);
            return result ? result - this->begin_() : npos;
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of a character sequence of explicit length starting at pos.
         *
         * @param ptr The character sequence to search for
         * @param pos The position at which to start the search
         * @param count The length of the character sequence
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找指定长度的字符序列首次出现的位置。
         *
         * @param ptr 待搜索的字符序列
         * @param pos 搜索的起始位置
         * @param count 字符序列的长度
         * @return 首次出现的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find(const value_type *ptr, size_type pos, size_type count) const {
            return find(basic_string_view<value_type>(ptr, count), pos);
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of a null-terminated character string starting at pos.
         *
         * @param ptr The null-terminated string to search for
         * @param pos The position at which to start the search
         * @return The index of the first occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找以空字符结尾的字符串首次出现的位置。
         *
         * @param ptr 待搜索的以空字符结尾的字符串
         * @param pos 搜索的起始位置
         * @return 首次出现的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find(const value_type *ptr, size_type pos = 0) const {
            return find(basic_string_view<value_type>(ptr), pos);
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of the given substring, searching backward from pos.
         *
         * @param sv The substring to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找给定子串最后一次出现的位置。
         *
         * @param sv 待搜索的子串
         * @param pos 反向搜索的起始位置
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type rfind(basic_string_view<value_type> sv, size_type pos = npos) const noexcept {
            const size_type self_size = size();
            const size_type sv_size = sv.size();
            if (sv_size == 0) {
                return (core::min) (pos, self_size);
            }
            if (sv_size > self_size) {
                return npos;
            }
            const value_type *self_begin = this->begin_();
            const size_type limit = (core::min) (pos, self_size - sv_size);
#if RAINY_HAS_CXX20
            // NOLINTBEGIN
            if (std::is_constant_evaluated()) { // NOLINT
                for (size_type i = limit + 1; i-- > 0;) {
                    bool match = true;
                    for (size_type j = 0; j < sv_size; ++j) {
                        if (!traits_type::eq(self_begin[i + j], sv[j])) { // NOLINT
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return i;
                    }
                }
                return npos;
            }
            // NOLINTEND
#endif
            // NOLINTBEGIN
            for (size_type i = limit + 1; i-- > 0;) {
                if (traits_type::compare(self_begin + i, sv.data(), sv_size) == 0) {
                    return i;
                }
            }
            // NOLINTEND
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
        RAINY_CONSTEXPR20 size_type rfind(value_type c, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            pos = (core::min) (pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (traits_type::eq(*p, c)) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of a character sequence of explicit length, searching backward from pos.
         *
         * @param ptr The character sequence to search for
         * @param pos The position at which to start the backward search
         * @param count The length of the character sequence
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找指定长度的字符序列最后一次出现的位置。
         *
         * @param ptr 待搜索的字符序列
         * @param pos 反向搜索的起始位置
         * @param count 字符序列的长度
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type rfind(const value_type *ptr, size_type pos, size_type count) const {
            return rfind(basic_string_view<value_type>(ptr, count), pos);
        }

        /**
         * \lang english
         * @brief Finds the last occurrence of a null-terminated character string, searching backward from pos.
         *
         * @param ptr The null-terminated string to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last occurrence, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找以空字符结尾的字符串最后一次出现的位置。
         *
         * @param ptr 待搜索的以空字符结尾的字符串
         * @param pos 反向搜索的起始位置
         * @return 最后一次出现的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type rfind(const value_type *ptr, size_type pos = npos) const {
            return rfind(basic_string_view<value_type>(ptr), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character equal to any character of the given set, starting at pos.
         *
         * @param ptr The set of characters to search for
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符集合中任意字符相等的首个字符。
         *
         * @param ptr 待搜索的字符集合
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_first_of(basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
            for (auto p = this->begin_() + pos; p < this->begin_() + size(); ++p) {
                if (ptr.find(*p) != npos) {
                    return p - this->begin_();
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
        RAINY_CONSTEXPR20 size_type find_first_of(value_type c, size_type pos = 0) const noexcept {
            return find(c, pos);
        }

        /**
         * \lang english
         * @brief Finds the first character equal to any character of the given sequence, starting at pos.
         *
         * @param ptr The character sequence to search for
         * @param pos The position at which to start the search
         * @param count The length of the character sequence
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符序列中任意字符相等的首个字符。
         *
         * @param ptr 待搜索的字符序列
         * @param pos 搜索的起始位置
         * @param count 字符序列的长度
         * @return 首个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_first_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_first_of(basic_string_view<value_type>(ptr, count), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character equal to any character of the given null-terminated string, starting at pos.
         *
         * @param ptr The null-terminated string to search for
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定以空字符结尾的字符串中任意字符相等的首个字符。
         *
         * @param ptr 待搜索的以空字符结尾的字符串
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_first_of(const value_type *ptr, size_type pos = 0) const {
            return find_first_of(basic_string_view<value_type>(ptr), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character equal to any character of the given set, searching backward from pos.
         *
         * @param ptr The set of characters to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符集合中任意字符相等的末个字符。
         *
         * @param ptr 待搜索的字符集合
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_last_of(basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            pos = (core::min) (pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (ptr.find(*p) != npos) {
                    return p - this->begin_();
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
        RAINY_CONSTEXPR20 size_type find_last_of(value_type c, size_type pos = npos) const noexcept {
            return rfind(c, pos);
        }

        /**
         * \lang english
         * @brief Finds the last character equal to any character of the given sequence, searching backward from pos.
         *
         * @param ptr The character sequence to search for
         * @param pos The position at which to start the backward search
         * @param count The length of the character sequence
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符序列中任意字符相等的末个字符。
         *
         * @param ptr 待搜索的字符序列
         * @param pos 反向搜索的起始位置
         * @param count 字符序列的长度
         * @return 末个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_last_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_last_of(basic_string_view<value_type>(ptr, count), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character equal to any character of the given null-terminated string, searching backward from pos.
         *
         * @param ptr The null-terminated string to search for
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定以空字符结尾的字符串中任意字符相等的末个字符。
         *
         * @param ptr 待搜索的以空字符结尾的字符串
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_last_of(const value_type *ptr, size_type pos = npos) const {
            return find_last_of(basic_string_view<value_type>(ptr), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character not equal to any character of the given set, starting at pos.
         *
         * @param ptr The set of characters to exclude
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符集合中任意字符都不相等的首个字符。
         *
         * @param ptr 需要排除的字符集合
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_first_not_of(basic_string_view<value_type> ptr, size_type pos = 0) const noexcept {
            for (auto p = this->begin_() + pos; p < this->begin_() + size(); ++p) {
                if (ptr.find(*p) == npos) {
                    return p - this->begin_();
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
        RAINY_CONSTEXPR20 size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept {
            for (auto p = this->begin_() + pos; p < this->begin_() + size(); ++p) {
                if (!traits_type::eq(*p, c)) {
                    return p - this->begin_();
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the first character not equal to any character of the given sequence, starting at pos.
         *
         * @param ptr The character sequence to exclude
         * @param pos The position at which to start the search
         * @param count The length of the character sequence
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定字符序列中任意字符都不相等的首个字符。
         *
         * @param ptr 需要排除的字符序列
         * @param pos 搜索的起始位置
         * @param count 字符序列的长度
         * @return 首个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_first_not_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_first_not_of(basic_string_view<value_type>(ptr, count), pos);
        }

        /**
         * \lang english
         * @brief Finds the first character not equal to any character of the given null-terminated string, starting at pos.
         *
         * @param ptr The null-terminated string to exclude
         * @param pos The position at which to start the search
         * @return The index of the first match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 开始查找与给定以空字符结尾的字符串中任意字符都不相等的首个字符。
         *
         * @param ptr 需要排除的以空字符结尾的字符串
         * @param pos 搜索的起始位置
         * @return 首个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_first_not_of(const value_type *ptr, size_type pos = 0) const {
            return find_first_not_of(basic_string_view<value_type>(ptr), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character not equal to any character of the given set, searching backward from pos.
         *
         * @param ptr The set of characters to exclude
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符集合中任意字符都不相等的末个字符。
         *
         * @param ptr 需要排除的字符集合
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_last_not_of(basic_string_view<value_type> ptr, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            pos = (core::min) (pos, size() - 1);
            for (auto p = this->begin_() + pos; p >= this->begin_(); --p) {
                if (ptr.find(*p) == npos) {
                    return p - this->begin_();
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
        RAINY_CONSTEXPR20 size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept {
            if (empty()) {
                return npos;
            }
            if (pos >= size()) {
                pos = size() - 1;
            }
            for (size_type i = pos + 1; i > 0; --i) {
                if (!traits_type::eq(this->begin_()[i - 1], c)) {
                    return i - 1;
                }
            }
            return npos;
        }

        /**
         * \lang english
         * @brief Finds the last character not equal to any character of the given sequence, searching backward from pos.
         *
         * @param ptr The character sequence to exclude
         * @param pos The position at which to start the backward search
         * @param count The length of the character sequence
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定字符序列中任意字符都不相等的末个字符。
         *
         * @param ptr 需要排除的字符序列
         * @param pos 反向搜索的起始位置
         * @param count 字符序列的长度
         * @return 末个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_last_not_of(const value_type *ptr, size_type pos, size_type count) const {
            return find_last_not_of(basic_string_view<value_type>(ptr, count), pos);
        }

        /**
         * \lang english
         * @brief Finds the last character not equal to any character of the given null-terminated string, searching backward from pos.
         *
         * @param ptr The null-terminated string to exclude
         * @param pos The position at which to start the backward search
         * @return The index of the last match, or npos if not found
         *
         * \lang simp-chinese
         * @brief 从 pos 起向前查找与给定以空字符结尾的字符串中任意字符都不相等的末个字符。
         *
         * @param ptr 需要排除的以空字符结尾的字符串
         * @param pos 反向搜索的起始位置
         * @return 末个匹配的下标，未找到时返回 npos
         */
        RAINY_CONSTEXPR20 size_type find_last_not_of(const value_type *ptr, size_type pos = npos) const {
            return find_last_not_of(basic_string_view<value_type>(ptr), pos);
        }

        /**
         * \lang english
         * @brief Checks whether the string starts with the given string view.
         *
         * @param sv The prefix to check for
         * @return true if the string starts with sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否以给定的字符串视图开头。
         *
         * @param sv 待检查的前缀
         * @return 若字符串以 sv 开头返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool starts_with(basic_string_view<value_type> sv) const noexcept {
            auto sv_size = sv.size();
            auto data = sv.data();
            auto begin = begin_();
            if (sv_size > size()) {
                return false;
            }
            return equal_(data, data + sv_size, begin, begin + sv_size);
        }

        /**
         * \lang english
         * @brief Checks whether the string starts with the given character.
         *
         * @param ch The character to check for
         * @return true if the string starts with ch, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否以给定的字符开头。
         *
         * @param ch 待检查的字符
         * @return 若字符串以 ch 开头返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool starts_with(CharType ch) const noexcept {
            return *begin_() == ch;
        }

        /**
         * \lang english
         * @brief Checks whether the string starts with the given null-terminated character string.
         *
         * @param ptr The null-terminated string to check for
         * @return true if the string starts with ptr, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否以给定的以空字符结尾的字符串开头。
         *
         * @param ptr 待检查的以空字符结尾的字符串
         * @return 若字符串以 ptr 开头返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool starts_with(const value_type *ptr) const {
            if (empty()) {
                return false;
            }
            auto length = traits_type::length(ptr);
            auto begin = begin_();
            if (length > size()) {
                return false;
            }
            return equal_(ptr, ptr + length, begin, begin + length);
        }

        /**
         * \lang english
         * @brief Checks whether the string ends with the given string view.
         *
         * @param sv The suffix to check for
         * @return true if the string ends with sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否以给定的字符串视图结尾。
         *
         * @param sv 待检查的后缀
         * @return 若字符串以 sv 结尾返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool ends_with(basic_string_view<value_type> sv) const noexcept {
            if (empty()) {
                return false;
            }
            auto sv_size = sv.size();
            auto sv_data = sv.data();
            auto end = end_();
            if (sv_size > size()) {
                return false;
            }
            return equal_(sv_data, sv_data + sv_size, end - sv_size, end);
        }

        /**
         * \lang english
         * @brief Checks whether the string ends with the given character.
         *
         * @param ch The character to check for
         * @return true if the string ends with ch, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否以给定的字符结尾。
         *
         * @param ch 待检查的字符
         * @return 若字符串以 ch 结尾返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool ends_with(CharType ch) const noexcept {
            if (empty()) {
                return false;
            }
            return *(end_() - 1) == ch;
        }

        /**
         * \lang english
         * @brief Checks whether the string ends with the given null-terminated character string.
         *
         * @param ptr The null-terminated string to check for
         * @return true if the string ends with ptr, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否以给定的以空字符结尾的字符串结尾。
         *
         * @param ptr 待检查的以空字符结尾的字符串
         * @return 若字符串以 ptr 结尾返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool ends_with(value_type const *ptr) const {
            if (empty()) {
                return false;
            }
            auto length = traits_type::length(ptr);
            if (length > size()) {
                return false;
            }
            auto end = end_();
            return equal_(ptr, ptr + length, end - length, end);
        }

        /**
         * \lang english
         * @brief Checks whether the string contains the given substring.
         *
         * @tparam StringViewLike The string-view-like type
         * @param sv The substring to look for
         * @return true if the string contains sv, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否包含给定的子串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param sv 待查找的子串
         * @return 若字符串包含 sv 返回 true，否则返回 false
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 bool contains(StringViewLike sv) const noexcept {
            auto size = sv.size();
            auto begin = begin_();
            auto data = sv.data();
            if (this->size() < size) {
                return false;
            }
            if (equal_(begin, begin + size, data, data + size)) {
                return true;
            }
            return find(sv) != npos;
        }

        /**
         * \lang english
         * @brief Checks whether the string contains the given character.
         *
         * @param ch The character to look for
         * @return true if the string contains ch, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否包含给定的字符。
         *
         * @param ch 待查找的字符
         * @return 若字符串包含 ch 返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool contains(CharType ch) const noexcept {
            for (auto begin = begin_(), end = end_(); begin != end; ++begin) {
                if (*begin == ch) {
                    return true;
                }
            }
            return false;
        }

        /**
         * \lang english
         * @brief Checks whether the string contains the given null-terminated character string.
         *
         * @param ptr The null-terminated string to look for
         * @return true if the string contains ptr, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查字符串是否包含给定的以空字符结尾的字符串。
         *
         * @param ptr 待查找的以空字符结尾的字符串
         * @return 若字符串包含 ptr 返回 true，否则返回 false
         */
        RAINY_CONSTEXPR20 bool contains(const value_type *ptr) const noexcept {
            return basic_string_view<value_type>{begin_(), end_()}.contains(
                basic_string_view<value_type>{ptr, ptr + traits_type::length(ptr)});
        }

        /**
         * \lang english
         * @brief Inserts count copies of the character ch at the given index.
         *
         * @param index The position at which to insert
         * @param count The number of characters
         * @param ch The character to insert
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定下标处插入 count 个字符 ch 的副本。
         *
         * @param index 插入位置
         * @param count 字符数量
         * @param ch 待插入的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &insert(size_type index, size_type count, CharType ch) {
            auto size = this->size();
            if (index > size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            reserve(size + count);
            auto start = begin_() + index;
            auto end = begin_() + size;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                core::algorithm::copy_backward(start, end, end + count);
                std::fill(start, start + count, ch);
            } else
#endif
            // NOLINTBEGIN
            {
                std::memmove(start + count, start, (end - start) * sizeof(CharType));
                std::fill(start, start + count, ch);
            }
            // NOLINTEND
            resize_(size + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Inserts the first count characters of the given character array at the given index.
         *
         * @param index The position at which to insert
         * @param ptr The character array
         * @param count The number of characters
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定下标处插入给定字符数组的前 count 个字符。
         *
         * @param index 插入位置
         * @param ptr 字符数组
         * @param count 字符数量
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const value_type *ptr, size_type count) {
            insert_(index, ptr, ptr + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Inserts a null-terminated character string at the given index.
         *
         * @param index The position at which to insert
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定下标处插入以空字符结尾的字符串。
         *
         * @param index 插入位置
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const value_type *ptr) {
            insert_(index, ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        /**
         * \lang english
         * @brief Inserts a copy of str at the given index.
         *
         * @param index The position at which to insert
         * @param str The string to insert
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定下标处插入 str 的副本。
         *
         * @param index 插入位置
         * @param str 待插入的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str) {
            insert_(index, str.begin_(), str.end_());
            return *this;
        }

        /**
         * \lang english
         * @brief Inserts a substring of str at the given index.
         *
         * @param index The position at which to insert
         * @param str The source string
         * @param s_index The starting position of the substring
         * @param count The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定下标处插入 str 的子串。
         *
         * @param index 插入位置
         * @param str 源字符串
         * @param s_index 子串的起始位置
         * @param count 子串的长度
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos) {
            auto s_size = str.size();
            if (s_index > s_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (s_size - s_index, count));
            auto s_start = str.begin_() + s_index;
            insert_(index, s_start, s_start + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Inserts a character before the element pointed to by pos.
         *
         * @param pos The position before which to insert
         * @param ch The character to insert
         * @return An iterator to the inserted character
         *
         * \lang simp-chinese
         * @brief 在 pos 指向的元素之前插入一个字符。
         *
         * @param pos 插入位置（位于该元素之前）
         * @param ch 待插入的字符
         * @return 指向已插入字符的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, CharType ch) {
            auto size = this->size();
            auto start = const_cast<iterator>(pos);
            auto index = start - begin_();
            reserve(size + 1);
            auto begin = begin_();
            auto end = begin + size;
            start = begin + index;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                core::algorithm::copy_backward(start, end, end + 1);
            } else
#endif
            // NOLINTBEGIN
            {
                traits_type::move(start + 1, start, end - start);
            }
            // NOLINTEND
            *start = ch;
            resize_(size + 1);
            return {start};
        }

        /**
         * \lang english
         * @brief Inserts count copies of the character ch before the element pointed to by pos.
         *
         * @param pos The position before which to insert
         * @param count The number of characters
         * @param ch The character to insert
         * @return An iterator to the first inserted character
         *
         * \lang simp-chinese
         * @brief 在 pos 指向的元素之前插入 count 个字符 ch 的副本。
         *
         * @param pos 插入位置（位于该元素之前）
         * @param count 字符数量
         * @param ch 待插入的字符
         * @return 指向首个已插入字符的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, size_type count, CharType ch) { // NOLINT
            auto start = const_cast<iterator>(pos);
            auto index = start - begin_();
            insert(index, count, ch);
            return {start};
        }

        /**
         * \lang english
         * @brief Inserts the characters in the range [first, last) before the element pointed to by pos.
         *
         * @tparam InputIt The iterator type
         * @param pos The position before which to insert
         * @param first Iterator to the first character
         * @param last Iterator one past the last character
         * @return An iterator to the first inserted character
         *
         * \lang simp-chinese
         * @brief 在 pos 指向的元素之前插入区间 [first, last) 内的字符。
         *
         * @tparam InputIt 迭代器类型
         * @param pos 插入位置（位于该元素之前）
         * @param first 指向首字符的迭代器
         * @param last 指向末字符之后位置的迭代器
         * @return 指向首个已插入字符的迭代器
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, InputIt first, InputIt last) {
            assert(("pos isn'sv in this string" && pos >= begin_() && pos <= end_()));
            auto size = this->size();
            auto start = pos;
            auto end = end_();
            auto index = start - begin_();
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto length = utility::distance(first, last);
                reserve(size + length);
                auto begin = begin_();
                core::algorithm::copy_backward(begin + index, begin + size, begin + size + length);
                core::algorithm::copy(first, last, begin + index);
                resize_(size + length);
            } else {
                basic_string temp{start, end};
                resize_(pos - begin_());
                for (; first != last; ++first) {
                    push_back(*first);
                }
                append_(temp.begin_(), temp.end_());
            }
            return {start};
        }

        /**
         * \lang english
         * @brief Inserts the characters of the initializer list before the element pointed to by pos.
         *
         * @param pos The position before which to insert
         * @param ilist The initializer list
         * @return An iterator to the first inserted character
         *
         * \lang simp-chinese
         * @brief 在 pos 指向的元素之前插入初始化列表中的字符。
         *
         * @param pos 插入位置（位于该元素之前）
         * @param ilist 初始化列表
         * @return 指向首个已插入字符的迭代器
         */
        RAINY_CONSTEXPR20 iterator insert(const_iterator pos, std::initializer_list<CharType> ilist) { // NOLINT
            auto i_data = std::data(ilist);
            auto start = const_cast<iterator>(pos);
            insert_(start - begin_(), i_data, i_data + ilist.size());
            return {start};
        }

        /**
         * \lang english
         * @brief Inserts a string-view-like type at the given position.
         *
         * @tparam StringViewLike The string-view-like type
         * @param pos The position at which to insert
         * @param sv The source view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定位置插入类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param pos 插入位置
         * @param sv 源视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &sv) {
            auto sv_data = sv.data();
            insert_(pos, sv_data, sv_data + sv.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Inserts a substring of a string-view-like type at the given position.
         *
         * @tparam StringViewLike The string-view-like type
         * @param pos The position at which to insert
         * @param sv The source view
         * @param t_index The starting position of the substring
         * @param count The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 在指定位置插入类似字符串视图类型的子串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param pos 插入位置
         * @param sv 源视图
         * @param t_index 子串的起始位置
         * @param count 子串的长度
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &insert(size_type pos, const StringViewLike &sv, size_type t_index, size_type count = npos) {
            auto sv_size = sv.size();
            auto size = this->size();
            if (t_index > sv_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (sv_size - t_index, count));
            auto sv_data = sv.data();
            insert_(pos, sv_data + t_index, sv_data + t_index + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Compares this string with a string-view-like type lexicographically.
         *
         * @tparam StringViewLike The string-view-like type
         * @param right The value to compare with
         * @return A negative value if less, zero if equal, a positive value if greater
         *
         * \lang simp-chinese
         * @brief 按字典序将此字符串与类似字符串视图的类型进行比较。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param right 与之比较的值
         * @return 小于返回负值，等于返回零，大于返回正值
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 int compare(StringViewLike right) const noexcept {
            const size_type rlen = (core::min) (size(), right.size());
            // NOLINTBEGIN
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = 0; i < rlen; ++i) {
                    if (begin_()[i] < right.data()[i]) {
                        return -1;
                    }
                    if (begin_()[i] > right.data()[i]) {
                        return 1;
                    }
                }
                return size() == right.size() ? 0 : (size() < right.size() ? -1 : 1);
            }
#endif
            // NOLINTEND
            if (const int result = traits_type::compare(this->begin_(), right.data(), rlen); result != 0) {
                return result;
            }
            return size() == right.size() ? 0 : (size() < right.size() ? -1 : 1);
        }
        /**
         * \lang english
         * @brief Compares the substring [pos1, pos1 + n1) with a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param pos1 The starting position of the substring
         * @param n1 The length of the substring
         * @param s The value to compare with
         * @return A negative value if less, zero if equal, a positive value if greater
         *
         * \lang simp-chinese
         * @brief 将此字符串的子串 [pos1, pos1 + n1) 与类似字符串视图的类型进行比较。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param pos1 子串的起始位置
         * @param n1 子串的长度
         * @param s 与之比较的值
         * @return 小于返回负值，等于返回零，大于返回正值
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 int compare(size_type pos1, size_type n1, StringViewLike s) const {
            return substr(pos1, n1).compare(s);
        }

        /**
         * \lang english
         * @brief Compares the substrings of this string and a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param pos1 The starting position of this substring
         * @param n1 The length of this substring
         * @param s The value to compare with
         * @param pos2 The starting position of the other substring
         * @param n2 The length of the other substring
         * @return A negative value if less, zero if equal, a positive value if greater
         *
         * \lang simp-chinese
         * @brief 比较此字符串与类似字符串视图类型的子串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param pos1 本子串的起始位置
         * @param n1 本子串的长度
         * @param s 与之比较的值
         * @param pos2 另一个子串的起始位置
         * @param n2 另一个子串的长度
         * @return 小于返回负值，等于返回零，大于返回正值
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 int compare(size_type pos1, size_type n1, StringViewLike s, size_type pos2, size_type n2) const {
            return substr(pos1, n1).compare(s.substr(pos2, n2));
        }

        /**
         * \lang english
         * @brief Compares this string with a null-terminated character string lexicographically.
         *
         * @param s The null-terminated character string to compare with
         * @return A negative value if less, zero if equal, a positive value if greater
         *
         * \lang simp-chinese
         * @brief 按字典序将此字符串与以空字符结尾的字符串进行比较。
         *
         * @param s 与之比较的以空字符结尾的字符串
         * @return 小于返回负值，等于返回零，大于返回正值
         */
        RAINY_CONSTEXPR20 int compare(const CharType *s) const {
            return compare(basic_string_view(s));
        }

        /**
         * \lang english
         * @brief Compares the substring [pos1, pos1 + n1) with a null-terminated character string.
         *
         * @param pos1 The starting position of the substring
         * @param n1 The length of the substring
         * @param s The null-terminated character string to compare with
         * @return A negative value if less, zero if equal, a positive value if greater
         *
         * \lang simp-chinese
         * @brief 将此字符串的子串 [pos1, pos1 + n1) 与以空字符结尾的字符串进行比较。
         *
         * @param pos1 子串的起始位置
         * @param n1 子串的长度
         * @param s 与之比较的以空字符结尾的字符串
         * @return 小于返回负值，等于返回零，大于返回正值
         */
        RAINY_CONSTEXPR20 int compare(size_type pos1, size_type n1, const CharType *s) const {
            return substr(pos1, n1).compare(basic_string_view(s));
        }

        /**
         * \lang english
         * @brief Compares the substring of this string with a character sequence of explicit length.
         *
         * @param pos1 The starting position of the substring
         * @param n1 The length of the substring
         * @param s The character sequence to compare with
         * @param n2 The length of the character sequence
         * @return A negative value if less, zero if equal, a positive value if greater
         *
         * \lang simp-chinese
         * @brief 将此字符串的子串与指定长度的字符序列进行比较。
         *
         * @param pos1 子串的起始位置
         * @param n1 子串的长度
         * @param s 与之比较的字符序列
         * @param n2 字符序列的长度
         * @return 小于返回负值，等于返回零，大于返回正值
         */
        RAINY_CONSTEXPR20 int compare(size_type pos1, size_type n1, const CharType *s, size_type n2) const {
            return substr(pos1, n1).compare(basic_string_view(s, n2));
        }

        /**
         * \lang english
         * @brief Removes count characters starting at the given index.
         *
         * @param index The position at which to start erasing
         * @param count The number of characters to remove
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 从指定下标开始移除 count 个字符。
         *
         * @param index 开始删除的位置
         * @param count 待移除的字符数量
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &erase(size_type index = 0, size_type count = npos) {
            auto size = this->size();
            if (index > size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count = (core::min) (npos, (core::min) (size - index, count));
            auto start = begin_() + index;
            erase_(start, start + count);
            return *this;
        }

        /**
         * \lang english
         * @brief Removes the character at the given position.
         *
         * @param position The position of the character to remove
         * @return An iterator to the character following the removed one
         *
         * \lang simp-chinese
         * @brief 移除指定位置的字符。
         *
         * @param position 待移除字符的位置
         * @return 指向被移除字符之后字符的迭代器
         */
        RAINY_CONSTEXPR20 iterator erase(const_iterator position) noexcept {
            auto start = const_cast<iterator>(position);
            erase_(start, start + 1);
            return {start};
        }

        /**
         * \lang english
         * @brief Removes the characters in the range [first, last).
         *
         * @param first Iterator to the first character to remove
         * @param last Iterator one past the last character to remove
         * @return An iterator to the character following the removed range
         *
         * \lang simp-chinese
         * @brief 移除区间 [first, last) 内的字符。
         *
         * @param first 指向首个待移除字符的迭代器
         * @param last 指向最后一个待移除字符之后位置的迭代器
         * @return 指向被移除区间之后字符的迭代器
         */
        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) noexcept {
            auto start = const_cast<iterator>(first);
            erase_(start, last);
            return {start};
        }

        /**
         * \lang english
         * @brief Removes the last character.
         *
         * \lang simp-chinese
         * @brief 移除最后一个字符。
         */
        RAINY_CONSTEXPR20 void pop_back() noexcept {
            assert(("string is empty" && !empty()));
            resize_(size() - 1);
        }

        /**
         * \lang english
         * @brief Replaces the range [pos, pos + count) with a copy of str.
         *
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param str The replacement string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [pos, pos + count) 替换为 str 的副本。
         *
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param str 替换字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str) {
            replace_(pos, count, str.begin_(), str.end_());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with a copy of str.
         *
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param str The replacement string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为 str 的副本。
         *
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param str 替换字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const basic_string &str) {
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, str.begin_(), str.end_());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [pos, pos + count) with a substring of str.
         *
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param str The source string
         * @param pos2 The starting position of the substring
         * @param count2 The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [pos, pos + count) 替换为 str 的子串。
         *
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param str 源字符串
         * @param pos2 子串的起始位置
         * @param count2 子串的长度
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const basic_string &str, size_type pos2,
                                                size_type count2 = npos) {
            auto str_size = str.size();
            if (pos2 > str_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count2 = (core::min) (npos, (core::min) (count2, str_size - pos2));
            auto begin = str.begin_();
            replace_(pos, count, begin + count2, begin + count2 + pos2);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range with the first count2 characters of the given character array.
         *
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param ptr The character array
         * @param count2 The number of characters to insert
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间替换为给定字符数组的前 count2 个字符。
         *
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param ptr 字符数组
         * @param count2 待插入的字符数量
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const value_type *ptr, size_type count2) {
            replace_(pos, count, ptr, ptr + count2);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with the first count2 characters of the given character array.
         *
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param ptr The character array
         * @param count2 The number of characters to insert
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为给定字符数组的前 count2 个字符。
         *
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param ptr 字符数组
         * @param count2 待插入的字符数量
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const value_type *ptr, size_type count2) {
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, ptr, ptr + count2);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range with a null-terminated character string.
         *
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间替换为以空字符结尾的字符串。
         *
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const value_type *ptr) {
            replace_(pos, count, ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with a null-terminated character string.
         *
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param ptr The null-terminated character string
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为以空字符结尾的字符串。
         *
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param ptr 以空字符结尾的字符串
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const value_type *ptr) {
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, ptr, ptr + traits_type::length(ptr));
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with the characters of the initializer list.
         *
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param ilist The initializer list
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为初始化列表中的字符。
         *
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param ilist 初始化列表
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, std::initializer_list<CharType> ilist) {
            auto data = std::data(ilist);
            auto start = const_cast<iterator>(first);
            replace_(start - begin_(), last - first, data, data + ilist.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [pos, pos + count) with a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param sv The replacement view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [pos, pos + count) 替换为类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param sv 替换视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const StringViewLike &sv) {
            auto data = sv.data();
            replace_(pos, count, data, data + sv.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param sv The replacement view
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为类似字符串视图类型的内容。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param sv 替换视图
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      (type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> ||
                       type_traits::type_relations::is_convertible_v<const StringViewLike &, basic_string_view<value_type>>) &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, const StringViewLike &sv) {
            auto sv_data = sv.data();
            auto start = first;
            replace_(start - begin_(), last - first, sv_data, sv_data + sv.size());
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [pos, pos + count) with a substring of a string-view-like type.
         *
         * @tparam StringViewLike The string-view-like type
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param sv The source view
         * @param pos2 The starting position of the substring
         * @param count2 The length of the substring
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [pos, pos + count) 替换为类似字符串视图类型的子串。
         *
         * @tparam StringViewLike 类似字符串视图的类型
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param sv 源视图
         * @param pos2 子串的起始位置
         * @param count2 子串的长度
         * @return 对此字符串的引用
         */
        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<const StringViewLike &, std::basic_string_view<value_type>> &&
                          (!type_traits::type_relations::is_convertible_v<const StringViewLike &, const value_type *>),
                      int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, const StringViewLike &sv, size_type pos2,
                                                size_type count2 = npos) {
            auto sv_size = sv.size();
            if (pos2 > sv_size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            count2 = (core::min) (npos, (core::min) (sv_size - pos2, count2));
            auto data = sv.data();
            replace_(pos, count, data + pos2, data + pos2 + count2);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range with count2 copies of the character ch.
         *
         * @param pos The starting position of the range to replace
         * @param count The length of the range to replace
         * @param count2 The number of characters to insert
         * @param ch The character to insert
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间替换为 count2 个字符 ch 的副本。
         *
         * @param pos 待替换区间的起始位置
         * @param count 待替换区间的长度
         * @param count2 待插入的字符数量
         * @param ch 待插入的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(size_type pos, size_type count, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = begin_();
            replace_(pos, count, begin, begin + count2);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with count2 copies of the character ch.
         *
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param count2 The number of characters to insert
         * @param ch The character to insert
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为 count2 个字符 ch 的副本。
         *
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param count2 待插入的字符数量
         * @param ch 待插入的字符
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, size_type count2, CharType ch) {
            basic_string temp{count2, ch};
            auto begin = begin_();
            auto start = first;
            replace_(start - begin_(), last - first, begin, begin + count2);
            return *this;
        }

        /**
         * \lang english
         * @brief Replaces the range [first, last) with the characters in the range [first2, last2).
         *
         * @tparam InputIt The iterator type
         * @param first Iterator to the first character of the range
         * @param last Iterator one past the last character of the range
         * @param first2 Iterator to the first replacement character
         * @param last2 Iterator one past the last replacement character
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将区间 [first, last) 替换为区间 [first2, last2) 内的字符。
         *
         * @tparam InputIt 迭代器类型
         * @param first 指向区间首字符的迭代器
         * @param last 指向区间末字符之后位置的迭代器
         * @param first2 指向首个替换字符的迭代器
         * @param last2 指向最后一个替换字符之后位置的迭代器
         * @return 对此字符串的引用
         */
        template <typename InputIt,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIt>, int> = 0>
        RAINY_CONSTEXPR20 basic_string &replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) {
            auto start = first;
            if RAINY_CONSTEXPR20 (type_traits::extras::iterators::is_random_access_iterator_v<InputIt>) {
                auto data = utility::addressof(*first2);
                auto length2 = utility::distance(first2, last2);
                replace_(start - begin_(), last - first, data, data + length2);
            } else {
                basic_string temp{first2, last2};
                auto begin = temp.begin_();
                auto size = temp.size();
                replace_(start - begin_(), last - first, begin, begin + size);
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Repeats the string content repeat_count times in place.
         *
         * @param repeat_count The number of repetitions
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将字符串内容原地重复 repeat_count 次。
         *
         * @param repeat_count 重复次数
         * @return 对此字符串的引用
         */
        RAINY_CONSTEXPR20 basic_string &repeat(const std::size_t repeat_count) {
            if (repeat_count == 0) {
                clear();
                return *this;
            }
            if (repeat_count == 1) {
                return *this;
            }
            auto original_size = size();
            reserve(original_size * repeat_count);
            basic_string_view<value_type> view(data(), original_size);
            for (std::size_t i = 1; i < repeat_count; ++i) {
                append(view.data(), view.size());
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Returns a new string with the content repeated repeat_count times.
         *
         * @param repeat_count The number of repetitions
         * @return The repeated string
         *
         * \lang simp-chinese
         * @brief 返回内容重复 repeat_count 次的新字符串。
         *
         * @param repeat_count 重复次数
         * @return 重复后的字符串
         */
        RAINY_CONSTEXPR20 basic_string repeat_copy(const std::size_t repeat_count) const {
            basic_string result;
            if (repeat_count == 0) {
                return result;
            }
            result.reserve(size() * repeat_count);
            auto original_size = size();
            basic_string_view<value_type> view(data(), original_size);
            for (std::size_t i = 0; i < repeat_count; ++i) {
                result.append(view.data(), view.size());
            }
            return result;
        }

        /**
         * \lang english
         * @brief Converts all characters to uppercase in place.
         *
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将所有字符原地转换为大写。
         *
         * @return 对此字符串的引用
         */
        basic_string &to_upper() {
            core::algorithm::transform(this->begin(), this->end(), this->begin(),
                                       [](value_type ch) { return std::toupper(static_cast<int>(ch)); });
            return *this;
        }

        /**
         * \lang english
         * @brief Returns a copy with all characters converted to uppercase.
         *
         * @return The converted string
         *
         * \lang simp-chinese
         * @brief 返回所有字符已转换为大写的副本。
         *
         * @return 转换后的字符串
         */
        basic_string to_upper_copy() const {
            basic_string str = *this;
            core::algorithm::transform(this->begin(), this->end(), str.begin(),
                                       [](value_type ch) { return std::toupper(static_cast<int>(ch)); });
            return str;
        }

        /**
         * \lang english
         * @brief Converts all characters to lowercase in place.
         *
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 将所有字符原地转换为小写。
         *
         * @return 对此字符串的引用
         */
        basic_string &to_lower() {
            core::algorithm::transform(this->begin(), this->end(), this->begin(),
                                       [](value_type ch) { return std::tolower(static_cast<int>(ch)); });
            return *this;
        }

        /**
         * \lang english
         * @brief Returns a copy with all characters converted to lowercase.
         *
         * @return The converted string
         *
         * \lang simp-chinese
         * @brief 返回所有字符已转换为小写的副本。
         *
         * @return 转换后的字符串
         */
        basic_string to_lower_copy() const {
            basic_string str = *this;
            core::algorithm::transform(this->begin(), this->end(), str.begin(),
                                       [](value_type ch) { return std::tolower(static_cast<int>(ch)); });
            return str;
        }

        /**
         * \lang english
         * @brief Removes leading and trailing whitespace in place.
         *
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 原地移除首尾空白字符。
         *
         * @return 对此字符串的引用
         */
        basic_string &trim() {
            static constexpr value_type whitespace[] = {' ', '\t', '\n', '\r', '\f', '\v', '\0'};
            size_type pos = this->find_first_not_of(whitespace);
            if (pos == npos) {
                clear();
                return *this;
            }
            size_type pos2 = this->find_last_not_of(whitespace);
            if (pos2 != npos) {
                *this = substr(pos, pos2 - pos + 1);
            } else {
                *this = substr(pos);
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Returns a copy with leading and trailing whitespace removed.
         *
         * @return The trimmed string
         *
         * \lang simp-chinese
         * @brief 返回移除首尾空白字符后的副本。
         *
         * @return 修剪后的字符串
         */
        basic_string trimmed() const {
            static constexpr value_type whitespace[] = {' ', '\t', '\n', '\r', '\f', '\v', '\0'};
            size_type pos = this->find_first_not_of(whitespace);
            if (pos == npos) {
                return basic_string();
            }
            size_type pos2 = this->find_last_not_of(whitespace);
            if (pos2 != npos) {
                return substr(pos, pos2 - pos + 1);
            }
            return substr(pos);
        }

        /**
         * \lang english
         * @brief Splits the string by the given delimiter and returns a vector of substrings.
         *
         * @tparam Alloc The allocator type of the result vector
         * @param delim The delimiter character
         * @return A vector of the resulting substrings
         *
         * \lang simp-chinese
         * @brief 按给定分隔符拆分字符串并返回子串向量。
         *
         * @tparam Alloc 结果向量的分配器类型
         * @param delim 分隔符字符
         * @return 拆分得到的子串向量
         */
        template <typename Alloc = memory::allocator<basic_string>>
        RAINY_CONSTEXPR20 collections::vector<basic_string, Alloc> split(value_type delim) const {
            if (empty()) {
                return {};
            }
            size_type previous = 0;
            size_type current = find_first_of(delim);
            basic_string_view<value_type> str_view = (*this);
            collections::vector<basic_string, Alloc> res{};
            while (current != npos) {
                res.push_back(basic_string(str_view.substr(previous, current - previous)));
                previous = current + 1;
                current = find_first_of(delim, previous);
            }
            res.push_back(basic_string(str_view.substr(previous)));
            return res;
        }

        // NOLINTBEGIN

        /**
         * \lang english
         * @brief Formats the given arguments according to the format string and stores the result in this string.
         *
         * @tparam Args The argument types
         * @param fmt The format string
         * @param args The arguments to format
         * @return A reference to this string
         *
         * \lang simp-chinese
         * @brief 按照格式字符串格式化给定参数，并将结果存储到此字符串中。
         *
         * @tparam Args 参数类型
         * @param fmt 格式字符串
         * @param args 待格式化的参数
         * @return 对此字符串的引用
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

        // NOLINTEND

        /**
         * \lang english
         * @brief Returns a substring starting at pos with the given length.
         *
         * @param pos The starting position
         * @param count The length of the substring
         * @return The requested substring
         *
         * \lang simp-chinese
         * @brief 返回从 pos 开始、长度为 count 的子串。
         *
         * @param pos 起始位置
         * @param count 子串的长度
         * @return 所请求的子串
         */
        RAINY_CONSTEXPR20 basic_string substr(size_type pos = 0, size_type count = npos) const {
            return basic_string_view<value_type>{data(), size()}.substr(pos, count);
        }

        /**
         * \lang english
         * @brief Returns a copy of the allocator.
         *
         * @return A copy of the allocator
         *
         * \lang simp-chinese
         * @brief 返回分配器的副本。
         *
         * @return 分配器的副本
         */
        RAINY_CONSTEXPR20 allocator_type get_allocator() const {
            return this->get_al();
        }

        /**
         * \lang english
         * @brief Checks whether the left string is equal to the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否与右侧字符串相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator==(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                if (lsize != rsize) {
                    return false;
                }
                for (auto l = left.begin_(), r = right.begin_(), end = l + lsize; l != end; ++l, ++r) {
                    if (*l != *r) {
                        return false;
                    }
                }
                return true;
            }
#endif
            return traits_type::compare(left.begin_(), right.begin_(), (core::min) (lsize, rsize)) == 0 && lsize == rsize;
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically less than the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序小于右侧字符串。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, basic_string const &right) noexcept {
            auto lsize = left.size();
            auto rsize = right.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto l = left.begin_(), r = right.begin_(), end = l + (core::min) (lsize, rsize); l != end; ++l, ++r) {
                    if (*l < *r) {
                        return true;
                    }
                    if (*l > *r) {
                        return false;
                    }
                }
                return lsize < rsize;
            }
#endif
            auto res = traits_type::compare(left.begin_(), right.begin_(), (core::min) (rsize, lsize));
            if (res < 0) {
                return true;
            }
            if (res > 0) {
                return false;
            }
            return lsize < rsize;
        }

        /**
         * \lang english
         * @brief Checks whether the left string is not equal to the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否与右侧字符串不相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator!=(basic_string const &left, basic_string const &right) noexcept {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically greater than the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序大于右侧字符串。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator>(basic_string const &left, basic_string const &right) noexcept {
            return right < left;
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically less than or equal to the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序小于或等于右侧字符串。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator<=(basic_string const &left, basic_string const &right) noexcept {
            return !(right < left);
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically greater than or equal to the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序大于或等于右侧字符串。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator>=(basic_string const &left, basic_string const &right) noexcept {
            return !(left < right);
        }

        /**
         * \lang english
         * @brief Checks whether the left string is equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否与字符指针相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator==(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = traits_type::length(start);
            auto lsize = left.size();
            if (lsize != rsize) {
                return false;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto l = left.begin_(), end = l + lsize; l != end; ++l, ++start) {
                    if (*l != *start) {
                        return false;
                    }
                }
                return true;
            }
#endif
            {
                return traits_type::compare(left.begin_(), start, lsize) == 0;
            }
        }

        /**
         * \lang english
         * @brief Checks whether the character pointer is equal to the right string.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查字符指针是否与右侧字符串相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator==(CharType const *left, basic_string const &right) noexcept {
            auto start = left;
            auto lsize = traits_type::length(start);
            auto rsize = right.size();
            if (lsize != rsize) {
                return false;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto r = right.begin_(), end = r + rsize; r != end; ++r, ++start) {
                    if (*start != *r) {
                        return false;
                    }
                }
                return true;
            }
#endif
            {
                return traits_type::compare(start, right.begin_(), lsize) == 0;
            }
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically less than the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序小于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator<(basic_string const &left, CharType const *right) noexcept {
            auto start = right;
            auto rsize = traits_type::length(start);
            auto lsize = left.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) { // NOLINT
                for (auto l = left.begin_(), end = l + (core::min) (lsize, rsize); l != end; ++l, ++start) {
                    if (*l < *start) {
                        return true;
                    }
                    if (*l > *start) {
                        return false;
                    }
                }
                return lsize < rsize;
            }
#endif
            {
                auto res = traits_type::compare(left.begin_(), start, (core::min) (rsize, lsize));
                if (res < 0) {
                    return true;
                }
                if (res > 0) {
                    return false;
                }
                return lsize < rsize;
            }
        }

        /**
         * \lang english
         * @brief Checks whether the left string is not equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否与字符指针不相等。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator!=(basic_string const &left, CharType const *right) noexcept {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically greater than the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序大于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator>(basic_string const &left, CharType const *right) noexcept {
            return right < left;
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically less than or equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序小于或等于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator<=(basic_string const &left, CharType const *right) noexcept {
            return !(right < left);
        }

        /**
         * \lang english
         * @brief Checks whether the left string is lexicographically greater than or equal to the character pointer.
         *
         * @param left The left operand
         * @param right The right operand
         * @return The comparison result
         *
         * \lang simp-chinese
         * @brief 检查左侧字符串是否按字典序大于或等于字符指针。
         *
         * @param left 左操作数
         * @param right 右操作数
         * @return 比较结果
         */
        friend RAINY_CONSTEXPR20 bool operator>=(basic_string const &left, CharType const *right) noexcept {
            return !(left < right);
        }

    private:
        static constexpr std::size_t short_string_max_ = default_capacity;

        struct storage_type_ {
            pointer start{};
            size_type mylength{};
            union {
                CharType localbuffer[short_string_max_ + 1]{};
                size_type allocated_capacity;
            } storage_union_;
        };

        static inline char exception_string_[] = "parameter is out of range, please check it.";

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool is_long_() const noexcept {
            return get_storage().start != get_storage().storage_union_.localbuffer;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool is_short_() const noexcept {
            return get_storage().start == get_storage().storage_union_.localbuffer;
        }

        RAINY_CONSTEXPR20 void init_empty_() noexcept {
            auto &s = get_storage();
            s.start = s.storage_union_.localbuffer;
            s.mylength = 0;
            s.storage_union_.localbuffer[0] = CharType{};
        }

        static RAINY_CONSTEXPR20 void begin_lifetime(pointer begin, size_type n) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = 0; i != n; ++i) {
                    std::construct_at(utility::addressof(begin[i]));
                }
            }
#else
            utility::ignore = begin;
            utility::ignore = n;
#endif
        }

        RAINY_CONSTEXPR20 void fill_(value_type const *begin, value_type const *end) noexcept {
            auto count = static_cast<size_type>(end - begin);
            assert(count <= capacity());
            if (count == 0) {
                return;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (size_type i = 0; i < count; ++i) {
                    std::construct_at(begin_() + i, begin[i]);
                }
            } else
#endif
            {
                traits_type::copy(begin_(), begin, count);
            }
        }

        // NOLINTBEGIN
        RAINY_CONSTEXPR20 void append_(value_type const *first, value_type const *last) {
            auto length = last - first;
            auto size = this->size();
            auto new_size = size + length;
            // clang/gcc对这种可能更敏感，需要进行这种处理
#if (RAINY_USING_CLANG || RAINY_USING_GCC) && RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                if (is_short_()) { // 嗯……需要强制转化成长字符串，先保存，然后我再进行分配，不然constexpr的检查会进行不合理的报错
                    value_type old_data[short_string_max_ + 1];
                    auto old_size = size;
                    for (size_type i = 0; i < old_size; ++i) {
                        old_data[i] = get_storage().storage_union_.localbuffer[i]; // NOLINT
                    }
                    // 这个workaround并不优雅，不过，能跑就行的标准，不求太多了
                    allocate_plus_one_(new_size); // 应该可以分配了
                    auto new_begin = begin_();
                    for (size_type i = 0; i < old_size; ++i) {
                        utility::construct_at(new_begin + i, old_data[i]); // NOLINT
                    }
                    size = old_size;
                    // 应该不会爆炸了
                }
                if (capacity() < new_size) {
                    reserve(new_size);
                }
                // 追加新数据
                auto begin = begin_();
                for (size_type i = 0; i < length; ++i) {
                    utility::construct_at(begin + size + i, first[i]);
                }
                resize_(new_size);
            } else
#endif
            {
                reserve(new_size);
                auto begin = begin_();
                traits_type::copy(begin + size, first, length);
                resize_(new_size);
            }
        }
        // NOLINTEND

        RAINY_CONSTEXPR20 void erase_(CharType *first, value_type const *last) noexcept {
            assert(("first or last is not in this string" && first >= begin_() && last <= end_()));
            // NOLINTBEGIN
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                core::algorithm::copy(last, const_cast<basic_string const &>(*this).end_(), first);
            } else
#endif
            {
                traits_type::move(first, last, (const_cast<basic_string const &>(*this).end_() - last));
            }
            // NOLINTEND
            resize_(size() - (last - first));
        }

        RAINY_CONSTEXPR20 bool static equal_(value_type const *begin, value_type const *end, value_type const *first,
                                             value_type const *last) noexcept {
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

        RAINY_CONSTEXPR20 void allocate_plus_one_(size_type count) {
            if (count <= short_string_max_) {
                get_storage().start = get_storage().storage_union_.localbuffer;
                return;
            }
            if (count > max_size()) {
                core::implements::throw_exception_length_error("basic_string: requested size exceeds max_size()");
            }
            ++count;
            auto ptr = allocator_traits::allocate(this->get_al(), count);
            begin_lifetime(ptr, count);
            get_storage().start = ptr;
            get_storage().storage_union_.allocated_capacity = count;
        }

        RAINY_CONSTEXPR20 void swap_without_ator(basic_string &right) noexcept {
            auto &s = get_storage();
            auto &r = right.get_storage();

            const bool self_long = is_long_();

            if (const bool right_long = right.is_long_(); self_long && right_long) {
                // Both long: swap pointers, lengths, and capacities
                std::swap(s.start, r.start);
                std::swap(s.mylength, r.mylength);
                std::swap(s.storage_union_.allocated_capacity, r.storage_union_.allocated_capacity);
            } else if (!self_long && !right_long) {
                // Both short: swap lengths and buffer contents, but keep start pointing to own localbuffer
                std::swap(s.mylength, r.mylength);
                // NOLINTBEGIN
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    // In constexpr, only swap initialized elements to avoid reading indeterminate values
                    auto max_len = (core::max) (s.mylength, r.mylength);
                    for (size_type i = 0; i <= max_len; ++i) {
                        std::swap(s.storage_union_.localbuffer[i], r.storage_union_.localbuffer[i]);
                    }
                } else
#endif
                {
                    for (size_type i = 0; i <= short_string_max_; ++i) {
                        std::swap(s.storage_union_.localbuffer[i], r.storage_union_.localbuffer[i]);
                    }
                }
                // NOLINTEND
            } else if (self_long && !right_long) {
                // *this is long, right is short
                auto self_start = s.start;
                auto self_cap = s.storage_union_.allocated_capacity;
                auto self_len = s.mylength;
                auto right_len = r.mylength;

                // Copy right's short data into *this (activates localbuffer)
                s.mylength = right_len;
                for (size_type i = 0; i <= right_len; ++i) {
                    s.storage_union_.localbuffer[i] = r.storage_union_.localbuffer[i];
                }
                s.start = s.storage_union_.localbuffer;

                // right becomes long (activates allocated_capacity)
                r.start = self_start;
                r.storage_union_.allocated_capacity = self_cap;
                r.mylength = self_len;
            } else {
                // *this is short, right is long
                auto self_len = s.mylength;
                auto right_start = r.start;
                auto right_cap = r.storage_union_.allocated_capacity;
                auto right_len = r.mylength;

                // Copy *this's short data into right (activates localbuffer)
                r.mylength = self_len;
                for (size_type i = 0; i <= self_len; ++i) {
                    r.storage_union_.localbuffer[i] = s.storage_union_.localbuffer[i];
                }
                r.start = r.storage_union_.localbuffer;

                // *this becomes long (activates allocated_capacity)
                s.start = right_start;
                s.storage_union_.allocated_capacity = right_cap;
                s.mylength = right_len;
            }
        }

        RAINY_CONSTEXPR20 void resize_(size_type count) noexcept { // NOLINT
#if RAINY_HAS_CXX20
            if (!std::is_constant_evaluated()) {
                assert(("count > capacity()" && count <= capacity()));
            }
#else
            assert(("count > capacity()" && count <= capacity()));
#endif
            get_storage().mylength = count;
            get_storage().start[count] = CharType{};
        }

        RAINY_CONSTEXPR20 void assign_(value_type const *first, value_type const *last) {
            auto size = last - first;
            if (capacity() < static_cast<size_type>(size)) {
                if (is_long_()) {
                    auto old_start = get_storage().start;
                    auto old_allocated = get_storage().storage_union_.allocated_capacity;
                    allocate_plus_one_(size);
                    dealloc_(old_start, old_allocated);
                } else {
                    allocate_plus_one_(size);
                }
            }
            fill_(first, last);
            resize_(size);
        }

        RAINY_CONSTEXPR20 void insert_(size_type index, value_type const *first, value_type const *last) {
            auto size = this->size();
            if (index > size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            auto length = last - first;
            auto new_size = size + length;
            auto begin = begin_();
            auto end = begin + size;
            auto start = begin + index;
            if (capacity() >= new_size) {
#if RAINY_HAS_CXX20
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) { // NOLINT
                    core::algorithm::copy_backward(start, end, end + length);
                    core::algorithm::copy(first, last, start);
                } else
#endif
                {
                    std::memmove(start + length, start, (end - start) * sizeof(CharType)); // NOLINT
                    std::memcpy(start, first, length * sizeof(CharType)); // NOLINT
                }
                // NOLINTEND
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + index;
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) { // NOLINT
                    core::algorithm::copy(begin, start, temp_begin);
                    core::algorithm::copy(first, last, temp_start);
                    core::algorithm::copy(start, end, temp_start + length);
                } else
#endif
                {
                    // NOLINTBEGIN
                    std::memcpy(temp_begin, begin, index * sizeof(CharType));
                    std::memcpy(temp_start, first, length * sizeof(CharType));
                    std::memcpy(temp_start + length, start, (end - start) * sizeof(CharType));
                    // NOLINTEND
                }
                temp.swap(*this);
            }
            resize_(new_size);
        }

        RAINY_CONSTEXPR20 void replace_(size_type pos, size_type count, value_type const *first2, value_type const *last2) {
            auto size = this->size();
            if (pos > size) {
                core::implements::throw_exception_out_of_range(exception_string_);
            }
            auto begin = begin_();
            auto first1 = begin + pos;
            auto last1 = begin + (core::min) (pos + count, size);
            auto length1 = last1 - first1;
            auto length2 = last2 - first2;
            auto new_size = size - length1 + length2; // Fix 修复：符号对调
            auto end = begin + size;
#if RAINY_HAS_CXX20
            // In constexpr, pointer comparison across different allocations (e.g. string literal vs local buffer)
            // is not allowed. Always use the temp-string approach to avoid this UB.
            if (std::is_constant_evaluated()) {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + (first1 - begin);
                core::algorithm::copy(begin, first1, temp_begin);
                core::algorithm::copy(first2, last2, temp_start);
                core::algorithm::copy(last1, end, temp_start + length2);
                temp.swap(*this);
            } else
#endif
                if (!(last1 < first2 || last2 < first1) && new_size <= capacity()) {
                auto diff = length1 - length2;
                // NOLINTBEGIN
                if (std::is_constant_evaluated()) {
                    if (diff > 0) {
                        core::algorithm::copy(last1, end, last1 - diff);
                    } else if (diff < 0) {
                        core::algorithm::copy_backward(last1, end, end - diff);
                    }
                } else {
                    if (diff > 0) {
                        std::memmove(last1 + diff, last1, diff * sizeof(CharType));
                    } else if (diff < 0) {
                        std::memmove(last1 - diff, last1, -diff * sizeof(CharType));
                    }
                }
                // NOLINTEND
                if (first2 >= last1 && last2 <= end) {
                    first2 += diff;
                    last2 += diff;
                }
                // NOLINTBEGIN
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    core::algorithm::copy(first2, last2, first1);
                } else
#endif
                {
                    std::memmove(first1, first2, length2 * sizeof(CharType));
                }
                // NOLINTEND
            } else {
                basic_string temp{};
                temp.allocate_plus_one_(new_size);
                auto temp_begin = temp.begin_();
                auto temp_start = temp_begin + (first1 - begin);
                // NOLINTBEGIN
#if RAINY_HAS_CXX20
                if (std::is_constant_evaluated()) {
                    core::algorithm::copy(begin, first1, temp_begin);
                    core::algorithm::copy(first2, last2, temp_start);
                    core::algorithm::copy(last1, end, temp_start + length2);
                } else
#endif
                {
                    // Fix 修复：dst/src 对调，括号保证优先级正确
                    std::memcpy(temp_begin, begin, (first1 - begin) * sizeof(CharType));
                    std::memcpy(temp_start, first2, length2 * sizeof(CharType));
                    std::memcpy(temp_start + length2, last1, (end - last1) * sizeof(CharType));
                }
                // NOLINTEND
                temp.swap(*this);
            }
            resize_(new_size); // NOLINT
        }

        RAINY_CONSTEXPR20 void dealloc_(pointer ptr, size_type allocated_size) noexcept {
            allocator_traits::deallocate(this->get_al(), ptr, allocated_size);
        }

        RAINY_CONSTEXPR20 value_type const *begin_() const noexcept {
            return get_storage().start;
        }

        RAINY_CONSTEXPR20 CharType *begin_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).begin_());
        }

        RAINY_CONSTEXPR20 value_type const *end_() const noexcept {
            return get_storage().start + get_storage().mylength;
        }

        RAINY_CONSTEXPR20 CharType *end_() noexcept {
            return const_cast<CharType *>(const_cast<basic_string const &>(*this).end_());
        }

        RAINY_CONSTEXPR20 Allocator &get_al() noexcept {
            return pair_.get_first();
        }

        RAINY_CONSTEXPR20 const Allocator &get_al() const noexcept {
            return pair_.get_first();
        }

        RAINY_CONSTEXPR20 storage_type_ &get_storage() noexcept {
            return pair_.get_second();
        }

        RAINY_CONSTEXPR20 const storage_type_ &get_storage() const noexcept {
            return pair_.get_second();
        }

        container::compressed_pair<Allocator, storage_type_> pair_;
    };

    /**
     * \lang english
     * @brief Type alias for basic_string of char.
     *
     * \lang simp-chinese
     * @brief char 类型的 basic_string 别名。
     */
    using string = basic_string<char>;
    /**
     * \lang english
     * @brief Type alias for basic_string of wchar_t.
     *
     * \lang simp-chinese
     * @brief wchar_t 类型的 basic_string 别名。
     */
    using wstring = basic_string<wchar_t>;
    /**
     * \lang english
     * @brief Type alias for basic_string of char16_t.
     *
     * \lang simp-chinese
     * @brief char16_t 类型的 basic_string 别名。
     */
    using u16string = basic_string<char16_t>;
    /**
     * \lang english
     * @brief Type alias for basic_string of char32_t.
     *
     * \lang simp-chinese
     * @brief char32_t 类型的 basic_string 别名。
     */
    using u32string = basic_string<char32_t>;

#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Type alias for basic_string of char8_t.
     *
     * \lang simp-chinese
     * @brief char8_t 类型的 basic_string 别名。
     */
    using u8string = basic_string<char8_t>;
#endif

    /**
     * \lang english
     * @brief Swaps the contents of two strings.
     *
     * @tparam CharType The character type
     * @tparam Traits The character traits type
     * @tparam Allocator The allocator type
     * @param left The first string
     * @param right The second string
     *
     * \lang simp-chinese
     * @brief 交换两个字符串的内容。
     *
     * @tparam CharType 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Allocator 分配器类型
     * @param left 第一个字符串
     * @param right 第二个字符串
     */
    template <typename CharType, typename Traits, typename Allocator>
    RAINY_CONSTEXPR20 void swap(basic_string<CharType, Traits, Allocator> &left,
                                basic_string<CharType, Traits, Allocator> &right) noexcept {
        left.swap(right);
    }

    /**
     * \lang english
     * @brief Writes the string to the output stream.
     *
     * @tparam StandardTraits The stream character traits
     * @tparam value_type The character type
     * @tparam Traits The string character traits
     * @tparam Alloc The allocator type
     * @param ostream The output stream
     * @param str The string to write
     * @return A reference to the output stream
     *
     * \lang simp-chinese
     * @brief 将字符串写入输出流。
     *
     * @tparam StandardTraits 流的字符 traits
     * @tparam value_type 字符类型
     * @tparam Traits 字符串的字符 traits
     * @tparam Alloc 分配器类型
     * @param ostream 输出流
     * @param str 待写入的字符串
     * @return 输出流的引用
     */
    template <typename StandardTraits, typename value_type, typename Traits, typename Alloc>
    std::basic_ostream<value_type, StandardTraits> &operator<<(std::basic_ostream<value_type, StandardTraits> &ostream,
                                                               const basic_string<value_type, Traits, Alloc> &str) {
        ostream.write(str.data(), str.size());
        return ostream;
    }
}

// NOLINTBEGIN
    /**
     * \lang english
     * @brief Hash specialization for rainy basic_string used by std containers.
     *
     * @tparam value_type The character type
     * @tparam Traits The character traits type
     * @tparam Alloc The allocator type
     *
     * \lang simp-chinese
     * @brief 供 std 容器使用的 rainy basic_string 哈希特化。
     *
     * @tparam value_type 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Alloc 分配器类型
     */
namespace std {
    template <typename value_type, typename Traits, typename Alloc>
    struct hash<rainy::core::text::basic_string<value_type, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::core::text::basic_string<value_type, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return rainy::utility::implements::hash_array_representation(val.data(), val.size());
        }
    };
}

    /**
     * \lang english
     * @brief Hash specialization for rainy basic_string.
     *
     * @tparam value_type The character type
     * @tparam Traits The character traits type
     * @tparam Alloc The allocator type
     *
     * \lang simp-chinese
     * @brief rainy basic_string 的哈希特化。
     *
     * @tparam value_type 字符类型
     * @tparam Traits 字符 traits 类型
     * @tparam Alloc 分配器类型
     */
namespace rainy::utility {
    template <typename value_type, typename Traits, typename Alloc>
    struct hash<rainy::core::text::basic_string<value_type, Traits, Alloc>> {
        using result_type = std::size_t;
        using argument_type = rainy::core::text::basic_string<value_type, Traits, Alloc>;

        result_type operator()(const argument_type &val) const {
            return implements::hash_array_representation(val.data(), val.size());
        }
    };
}
// NOLINTEND

namespace rainy::core::text {
    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(const basic_string<CharType, Traits, Alloc> &left,
                                                                      const basic_string<CharType, Traits, Alloc> &right) {
        basic_string<CharType, Traits, Alloc> result(left);
        result.append(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(basic_string<CharType, Traits, Alloc> &&left,
                                                                      const basic_string<CharType, Traits, Alloc> &right) {
        left.append(right);
        return utility::move(left);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(const basic_string<CharType, Traits, Alloc> &left,
                                                                      basic_string<CharType, Traits, Alloc> &&right) {
        right.insert(0, left);
        return utility::move(right);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(basic_string<CharType, Traits, Alloc> &&left,
                                                                      basic_string<CharType, Traits, Alloc> &&right) {
        left.append(right);
        return utility::move(left);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(const CharType *left,
                                                                      const basic_string<CharType, Traits, Alloc> &right) {
        basic_string<CharType, Traits, Alloc> result(left);
        result.append(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(const CharType *left,
                                                                      basic_string<CharType, Traits, Alloc> &&right) {
        right.insert(0, left);
        return utility::move(right);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(CharType left,
                                                                      const basic_string<CharType, Traits, Alloc> &right) {
        basic_string<CharType, Traits, Alloc> result(1, left);
        result.append(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(CharType left, basic_string<CharType, Traits, Alloc> &&right) {
        right.insert(0, 1, left);
        return utility::move(right);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(const basic_string<CharType, Traits, Alloc> &left,
                                                                      const CharType *right) {
        basic_string<CharType, Traits, Alloc> result(left);
        result.append(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(basic_string<CharType, Traits, Alloc> &&left,
                                                                      const CharType *right) {
        left.append(right);
        return utility::move(left);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(const basic_string<CharType, Traits, Alloc> &left,
                                                                      CharType right) {
        basic_string<CharType, Traits, Alloc> result(left);
        result.push_back(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(basic_string<CharType, Traits, Alloc> &&left, CharType right) {
        left.push_back(right);
        return utility::move(left);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(
        const basic_string<CharType, Traits, Alloc> &left,
        type_traits::primary_types::type_identity_t<basic_string_view<CharType, Traits>> right) {
        basic_string<CharType, Traits, Alloc> result(left);
        result.append(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(
        basic_string<CharType, Traits, Alloc> &&left,
        type_traits::primary_types::type_identity_t<basic_string_view<CharType, Traits>> right) {
        left.append(right);
        return utility::move(left);
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(
        type_traits::primary_types::type_identity_t<basic_string_view<CharType, Traits>> left,
        const basic_string<CharType, Traits, Alloc> &right) {
        basic_string<CharType, Traits, Alloc> result(left);
        result.append(right);
        return result;
    }

    /**
     * \lang english
     * @brief Concatenates the two operands and returns the resulting string.
     *
     * @param left The left operand
     * @param right The right operand
     * @return The concatenated string
     *
     * \lang simp-chinese
     * @brief 拼接两个操作数并返回结果字符串。
     *
     * @param left 左操作数
     * @param right 右操作数
     * @return 拼接后的字符串
     */
    template <typename CharType, typename Traits, typename Alloc>
    RAINY_CONSTEXPR20 basic_string<CharType, Traits, Alloc> operator+(
        type_traits::primary_types::type_identity_t<basic_string_view<CharType, Traits>> left,
        basic_string<CharType, Traits, Alloc> &&right) {
        right.insert(0, left);
        return utility::move(right);
    }
}

#if RAINY_USING_GCC && !RAINY_USING_CLANG
#pragma GCC diagnostic pop
#endif

namespace rainy::text {
    using core::text::basic_string;
    using core::text::string;
    using core::text::u16string;
    using core::text::u32string;
    using core::text::wstring;
#if RAINY_HAS_CXX20
    using core::text::u8string;
#endif
}

#endif
