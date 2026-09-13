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
#ifndef RAINY_CORE_CONTAINER_BITSET_HPP
#define RAINY_CORE_CONTAINER_BITSET_HPP

#include <rainy/core/text/string.hpp>

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A fixed-size sequence of N bits.
     *         Provides bitwise operations, element access and conversions to integer/string forms.
     *
     * @tparam N The number of bits
     *
     * \lang simp-chinese
     * @brief 固定大小为 N 的位序列。
     *         提供位运算、元素访问以及到整数/字符串形式的转换。
     *
     * @tparam N 位的数量
     */
    template <std::size_t N>
    class bitset {
    public:
        /**
         * \lang english
         * @brief The number of bits in a single storage word.
         *
         * \lang simp-chinese
         * @brief 单个存储字中的位数。
         */
        static constexpr std::size_t BITS_PER_WORD = sizeof(unsigned long long) * CHAR_BIT;

        /**
         * \lang english
         * @brief The number of storage words required to hold N bits.
         *
         * \lang simp-chinese
         * @brief 容纳 N 位所需的存储字数。
         */
        static constexpr std::size_t NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;

        /**
         * \lang english
         * @brief A proxy reference that allows modifying a single bit of the bitset.
         *
         * \lang simp-chinese
         * @brief 允许修改 bitset 中单个位的代理引用。
         */
        class reference {
        public:
            friend class bitset;

            /**
             * \lang english
             * @brief Copy constructor.
             *
             * \lang simp-chinese
             * @brief 拷贝构造函数。
             */
            constexpr reference(const reference &) = default;

            /**
             * \lang english
             * @brief Destructor.
             *
             * \lang simp-chinese
             * @brief 析构函数。
             */
            constexpr ~reference() = default;

            /**
             * \lang english
             * @brief Assigns a boolean value to the referenced bit.
             *
             * @param x The value to assign
             * @return Reference to this proxy
             *
             * \lang simp-chinese
             * @brief 向被引用的位赋一个布尔值。
             *
             * @param x 要赋的值
             * @return 此代理的引用
             */
            constexpr reference &operator=(bool x) noexcept {
                if (x) {
                    bs->data[word_index(pos)] |= bit_mask(pos);
                } else {
                    bs->data[word_index(pos)] &= ~bit_mask(pos);
                }
                return *this;
            }

            /**
             * \lang english
             * @brief Assigns the value of another reference to the referenced bit.
             *
             * @param ref The reference to copy from
             * @return Reference to this proxy
             *
             * \lang simp-chinese
             * @brief 将另一个引用的值赋给被引用的位。
             *
             * @param ref 要拷贝的引用
             * @return 此代理的引用
             */
            constexpr reference &operator=(const reference &ref) noexcept {
                return *this = bool(ref);
            }

            /**
             * \lang english
             * @brief Returns the inverted value of the referenced bit.
             *
             * @return true if the referenced bit is 0, false otherwise
             *
             * \lang simp-chinese
             * @brief 返回被引用位的取反值。
             *
             * @return 若被引用位为 0 则返回 true，否则返回 false
             */
            constexpr bool operator~() const noexcept {
                return !bool(*this);
            }

            /**
             * \lang english
             * @brief Converts the referenced bit to bool.
             *
             * @return true if the referenced bit is 1, false otherwise
             *
             * \lang simp-chinese
             * @brief 将被引用位转换为 bool。
             *
             * @return 若被引用位为 1 则返回 true，否则返回 false
             */
            constexpr operator bool() const noexcept {
                return (bs->data[word_index(pos)] & bit_mask(pos)) != 0;
            }

            /**
             * \lang english
             * @brief Flips the referenced bit.
             *
             * @return Reference to this proxy
             *
             * \lang simp-chinese
             * @brief 翻转被引用的位。
             *
             * @return 此代理的引用
             */
            constexpr reference &flip() noexcept {
                bs->data[word_index(pos)] ^= bit_mask(pos);
                return *this;
            }

        private:
            bitset *bs;
            std::size_t pos;

            constexpr reference(bitset *b, std::size_t p) noexcept : bs(b), pos(p) {
            }
        };

        /**
         * \lang english
         * @brief Default constructor. Initializes all bits to 0.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。将所有位初始化为 0。
         */
        constexpr bitset() noexcept : data{} {
        }

        /**
         * \lang english
         * @brief Constructs the bitset from the low bits of an unsigned long long value.
         *
         * @param val The value used to initialize the bits
         *
         * \lang simp-chinese
         * @brief 使用 unsigned long long 值的低位构造 bitset。
         *
         * @param val 用于初始化位的值
         */
        constexpr bitset(unsigned long long val) noexcept : data{} {
            data[0] = val;
            sanitize();
        }

        /**
         * \lang english
         * @brief Constructs the bitset from a string.
         *
         * @tparam CharType The character type of the string
         * @tparam Traits The character traits of the string
         * @tparam Allocator The allocator type of the string
         * @param str The string to construct from
         * @param pos The starting position in the string
         * @param n The number of characters to use
         * @param zero The character representing 0
         * @param one The character representing 1
         *
         * @throws std::out_of_range If pos is greater than str.size()
         * @throws std::invalid_argument If a character other than zero/one is encountered
         *
         * \lang simp-chinese
         * @brief 从字符串构造 bitset。
         *
         * @tparam CharType 字符串的字符类型
         * @tparam Traits 字符串的字符特征
         * @tparam Allocator 字符串的分配器类型
         * @param str 用于构造的字符串
         * @param pos 字符串中的起始位置
         * @param n 要使用的字符数量
         * @param zero 表示 0 的字符
         * @param one 表示 1 的字符
         *
         * @throws std::out_of_range 若 pos 大于 str.size()
         * @throws std::invalid_argument 若遇到除 zero/one 之外的字符
         */
        template <typename CharType, typename Traits, typename Allocator>
        constexpr explicit bitset(const core::text::basic_string<CharType, Traits, Allocator> &str,
                                  typename core::text::basic_string<CharType, Traits, Allocator>::size_type pos = 0,
                                  typename core::text::basic_string<CharType, Traits, Allocator>::size_type n =
                                      core::text::basic_string<CharType, Traits, Allocator>::npos,
                                  CharType zero = CharType('0'), CharType one = CharType('1')) :
            bitset(core::text::basic_string_view<CharType, Traits>(str), pos, n, zero, one) {
        }

        /**
         * \lang english
         * @brief Constructs the bitset from a string view.
         *
         * @tparam CharType The character type of the string view
         * @tparam Traits The character traits of the string view
         * @param str The string view to construct from
         * @param pos The starting position in the string view
         * @param n The number of characters to use
         * @param zero The character representing 0
         * @param one The character representing 1
         *
         * @throws std::out_of_range If pos is greater than str.size()
         * @throws std::invalid_argument If a character other than zero/one is encountered
         *
         * \lang simp-chinese
         * @brief 从字符串视图构造 bitset。
         *
         * @tparam CharType 字符串视图的字符类型
         * @tparam Traits 字符串视图的字符特征
         * @param str 用于构造的字符串视图
         * @param pos 字符串视图中的起始位置
         * @param n 要使用的字符数量
         * @param zero 表示 0 的字符
         * @param one 表示 1 的字符
         *
         * @throws std::out_of_range 若 pos 大于 str.size()
         * @throws std::invalid_argument 若遇到除 zero/one 之外的字符
         */
        template <typename CharType, typename Traits>
        constexpr explicit bitset(
            core::text::basic_string_view<CharType, Traits> str, typename core::text::basic_string_view<CharType, Traits>::size_type pos = 0,
            typename core::text::basic_string_view<CharType, Traits>::size_type n = core::text::basic_string_view<CharType, Traits>::npos,
            CharType zero = CharType('0'), CharType one = CharType('1')) : data{} {
            if (pos > str.size()) {
                throw std::out_of_range("bitset::bitset");
            }

            std::size_t rlen = std::min(n, str.size() - pos);
            std::size_t bits_to_copy = std::min(rlen, N);

            for (std::size_t i = 0; i < bits_to_copy; ++i) {
                CharType ch = str[pos + rlen - 1 - i];
                if (Traits::eq(ch, one)) {
                    set(i);
                } else if (!Traits::eq(ch, zero)) {
                    throw std::invalid_argument("bitset::bitset");
                }
            }
        }

        /**
         * \lang english
         * @brief Constructs the bitset from a null-terminated character string.
         *
         * @tparam CharType The character type of the string
         * @param str The null-terminated string to construct from
         * @param n The number of characters to use
         * @param zero The character representing 0
         * @param one The character representing 1
         *
         * @throws std::invalid_argument If a character other than zero/one is encountered
         *
         * \lang simp-chinese
         * @brief 从以空字符结尾的字符串构造 bitset。
         *
         * @tparam CharType 字符串的字符类型
         * @param str 用于构造的以空字符结尾的字符串
         * @param n 要使用的字符数量
         * @param zero 表示 0 的字符
         * @param one 表示 1 的字符
         *
         * @throws std::invalid_argument 若遇到除 zero/one 之外的字符
         */
        template <typename CharType>
        constexpr explicit bitset(
            const CharType *str, typename core::text::basic_string_view<CharType>::size_type n = core::text::basic_string_view<CharType>::npos,
            CharType zero = CharType('0'), CharType one = CharType('1')) :
            bitset(n == core::text::basic_string_view<CharType>::npos ? core::text::basic_string_view<CharType>(str)
                                                                   : core::text::basic_string_view<CharType>(str, n),
                   0, n, zero, one) {
        }

        /**
         * \lang english
         * @brief Performs bitwise AND assignment with another bitset.
         *
         * @param rhs The other bitset
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 与另一个 bitset 执行按位与赋值。
         *
         * @param rhs 另一个 bitset
         * @return 此 bitset 的引用
         */
        constexpr bitset &operator&=(const bitset &rhs) noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] &= rhs.data[i];
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Performs bitwise OR assignment with another bitset.
         *
         * @param rhs The other bitset
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 与另一个 bitset 执行按位或赋值。
         *
         * @param rhs 另一个 bitset
         * @return 此 bitset 的引用
         */
        constexpr bitset &operator|=(const bitset &rhs) noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] |= rhs.data[i];
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Performs bitwise XOR assignment with another bitset.
         *
         * @param rhs The other bitset
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 与另一个 bitset 执行按位异或赋值。
         *
         * @param rhs 另一个 bitset
         * @return 此 bitset 的引用
         */
        constexpr bitset &operator^=(const bitset &rhs) noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] ^= rhs.data[i];
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Performs a left shift assignment by pos bits.
         *
         * @param pos The number of positions to shift
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 向左移动 pos 位。
         *
         * @param pos 要移动的位数
         * @return 此 bitset 的引用
         */
        constexpr bitset &operator<<=(std::size_t pos) noexcept {
            if (pos >= N) {
                for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                    data[i] = 0;
                }
                return *this;
            }

            if (pos == 0) {
                return *this;
            }

            std::size_t word_shift = pos / BITS_PER_WORD;
            std::size_t bit_shift = pos % BITS_PER_WORD;

            if (bit_shift == 0) {
                for (std::size_t i = NUM_WORDS - 1; i >= word_shift; --i) {
                    data[i] = data[i - word_shift];
                    if (i == word_shift) {
                        break;
                    }
                }
            } else {
                for (std::size_t i = NUM_WORDS - 1; i > word_shift; --i) {
                    data[i] = (data[i - word_shift] << bit_shift) | (data[i - word_shift - 1] >> (BITS_PER_WORD - bit_shift));
                }
                data[word_shift] = data[0] << bit_shift;
            }

            for (std::size_t i = 0; i < word_shift; ++i) {
                data[i] = 0;
            }

            sanitize();
            return *this;
        }

        /**
         * \lang english
         * @brief Performs a right shift assignment by pos bits.
         *
         * @param pos The number of positions to shift
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 向右移动 pos 位。
         *
         * @param pos 要移动的位数
         * @return 此 bitset 的引用
         */
        constexpr bitset &operator>>=(std::size_t pos) noexcept {
            if (pos >= N) {
                for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                    data[i] = 0;
                }
                return *this;
            }

            if (pos == 0) {
                return *this;
            }

            std::size_t word_shift = pos / BITS_PER_WORD;
            std::size_t bit_shift = pos % BITS_PER_WORD;

            if (bit_shift == 0) {
                for (std::size_t i = 0; i < NUM_WORDS - word_shift; ++i) {
                    data[i] = data[i + word_shift];
                }
            } else {
                for (std::size_t i = 0; i < NUM_WORDS - word_shift - 1; ++i) {
                    data[i] = (data[i + word_shift] >> bit_shift) | (data[i + word_shift + 1] << (BITS_PER_WORD - bit_shift));
                }
                data[NUM_WORDS - word_shift - 1] = data[NUM_WORDS - 1] >> bit_shift;
            }

            for (std::size_t i = NUM_WORDS - word_shift; i < NUM_WORDS; ++i) {
                data[i] = 0;
            }

            return *this;
        }

        /**
         * \lang english
         * @brief Returns a bitset shifted left by pos bits.
         *
         * @param pos The number of positions to shift
         * @return A new bitset with the bits shifted left
         *
         * \lang simp-chinese
         * @brief 返回左移 pos 位后的 bitset。
         *
         * @param pos 要移动的位数
         * @return 左移后的新 bitset
         */
        constexpr bitset operator<<(std::size_t pos) const noexcept {
            bitset result(*this);
            result <<= pos;
            return result;
        }

        /**
         * \lang english
         * @brief Returns a bitset shifted right by pos bits.
         *
         * @param pos The number of positions to shift
         * @return A new bitset with the bits shifted right
         *
         * \lang simp-chinese
         * @brief 返回右移 pos 位后的 bitset。
         *
         * @param pos 要移动的位数
         * @return 右移后的新 bitset
         */
        constexpr bitset operator>>(std::size_t pos) const noexcept {
            bitset result(*this);
            result >>= pos;
            return result;
        }

        /**
         * \lang english
         * @brief Sets all bits to 1.
         *
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 将所有位设置为 1。
         *
         * @return 此 bitset 的引用
         */
        constexpr bitset &set() noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] = ~0ULL;
            }
            sanitize();
            return *this;
        }

        /**
         * \lang english
         * @brief Sets the bit at the given position to the given value.
         *
         * @param pos The position of the bit
         * @param val The value to set (default true)
         * @return Reference to this bitset
         *
         * @throws std::out_of_range If pos is greater than or equal to N
         *
         * \lang simp-chinese
         * @brief 将指定位置的位设置为给定值。
         *
         * @param pos 位的位置
         * @param val 要设置的值（默认为 true）
         * @return 此 bitset 的引用
         *
         * @throws std::out_of_range 若 pos 大于或等于 N
         */
        constexpr bitset &set(std::size_t pos, bool val = true) {
            check_position(pos);
            if (val) {
                data[word_index(pos)] |= bit_mask(pos);
            } else {
                data[word_index(pos)] &= ~bit_mask(pos);
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Resets all bits to 0.
         *
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 将所有位重置为 0。
         *
         * @return 此 bitset 的引用
         */
        constexpr bitset &reset() noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] = 0;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Resets the bit at the given position to 0.
         *
         * @param pos The position of the bit
         * @return Reference to this bitset
         *
         * @throws std::out_of_range If pos is greater than or equal to N
         *
         * \lang simp-chinese
         * @brief 将指定位置的位重置为 0。
         *
         * @param pos 位的位置
         * @return 此 bitset 的引用
         *
         * @throws std::out_of_range 若 pos 大于或等于 N
         */
        constexpr bitset &reset(std::size_t pos) {
            check_position(pos);
            data[word_index(pos)] &= ~bit_mask(pos);
            return *this;
        }

        /**
         * \lang english
         * @brief Returns a bitset with all bits inverted.
         *
         * @return A new bitset with every bit flipped
         *
         * \lang simp-chinese
         * @brief 返回所有位取反后的 bitset。
         *
         * @return 每一位都被翻转的新 bitset
         */
        constexpr bitset operator~() const noexcept {
            bitset result;
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                result.data[i] = ~data[i];
            }
            result.sanitize();
            return result;
        }

        /**
         * \lang english
         * @brief Flips all bits.
         *
         * @return Reference to this bitset
         *
         * \lang simp-chinese
         * @brief 翻转所有位。
         *
         * @return 此 bitset 的引用
         */
        constexpr bitset &flip() noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] = ~data[i];
            }
            sanitize();
            return *this;
        }

        /**
         * \lang english
         * @brief Flips the bit at the given position.
         *
         * @param pos The position of the bit
         * @return Reference to this bitset
         *
         * @throws std::out_of_range If pos is greater than or equal to N
         *
         * \lang simp-chinese
         * @brief 翻转指定位置的位。
         *
         * @param pos 位的位置
         * @return 此 bitset 的引用
         *
         * @throws std::out_of_range 若 pos 大于或等于 N
         */
        constexpr bitset &flip(std::size_t pos) {
            check_position(pos);
            data[word_index(pos)] ^= bit_mask(pos);
            return *this;
        }

        /**
         * \lang english
         * @brief Accesses the bit at the given position.
         *
         * @param pos The position of the bit
         * @return true if the bit is set, false otherwise
         *
         * \lang simp-chinese
         * @brief 访问指定位置的位。
         *
         * @param pos 位的位置
         * @return 若该位已设置则返回 true，否则返回 false
         */
        constexpr bool operator[](std::size_t pos) const {
            return (data[word_index(pos)] & bit_mask(pos)) != 0;
        }

        /**
         * \lang english
         * @brief Accesses the bit at the given position, returning a proxy reference.
         *
         * @param pos The position of the bit
         * @return A proxy reference that can read or modify the bit
         *
         * \lang simp-chinese
         * @brief 访问指定位置的位，返回代理引用。
         *
         * @param pos 位的位置
         * @return 可读或修改该位的代理引用
         */
        constexpr reference operator[](std::size_t pos) {
            return reference(this, pos);
        }

        /**
         * \lang english
         * @brief Converts the bitset to an unsigned long value.
         *
         * @return The unsigned long value represented by the bitset
         *
         * @throws std::overflow_error If the value cannot be represented in unsigned long
         *
         * \lang simp-chinese
         * @brief 将 bitset 转换为 unsigned long 值。
         *
         * @return bitset 所表示的 unsigned long 值
         *
         * @throws std::overflow_error 若该值无法用 unsigned long 表示
         */
        constexpr unsigned long to_ulong() const {
            if constexpr (N == 0) {
                return 0;
            }

            if constexpr (N > sizeof(unsigned long) * CHAR_BIT) {
                for (std::size_t i = sizeof(unsigned long) * CHAR_BIT / BITS_PER_WORD; i < NUM_WORDS; ++i) {
                    if (data[i] != 0) {
                        throw std::overflow_error("bitset::to_ulong");
                    }
                }
            }
            // NOLINTBEGIN
            unsigned long result = static_cast<unsigned long>(data[0]);
            if constexpr (sizeof(unsigned long) > sizeof(unsigned long long)) {
                return result;
            } else if constexpr (sizeof(unsigned long) < sizeof(unsigned long long)) {
                if (data[0] > static_cast<unsigned long long>(~0UL)) {
                    throw std::overflow_error("bitset::to_ulong");
                }
            }
            // NOLINTEND
            return result;
        }

        /**
         * \lang english
         * @brief Converts the bitset to an unsigned long long value.
         *
         * @return The unsigned long long value represented by the bitset
         *
         * @throws std::overflow_error If the value cannot be represented in unsigned long long
         *
         * \lang simp-chinese
         * @brief 将 bitset 转换为 unsigned long long 值。
         *
         * @return bitset 所表示的 unsigned long long 值
         *
         * @throws std::overflow_error 若该值无法用 unsigned long long 表示
         */
        constexpr unsigned long long to_ullong() const {
            if constexpr (N == 0) {
                return 0;
            }

            if constexpr (N > sizeof(unsigned long long) * CHAR_BIT) {
                for (std::size_t i = 1; i < NUM_WORDS; ++i) {
                    if (data[i] != 0) {
                        throw std::overflow_error("bitset::to_ullong");
                    }
                }
            }

            return data[0];
        }

        /**
         * \lang english
         * @brief Converts the bitset to a string representation.
         *
         * @tparam CharType The character type of the resulting string
         * @tparam Traits The character traits of the resulting string
         * @tparam Allocator The allocator type of the resulting string
         * @param zero The character representing 0
         * @param one The character representing 1
         * @return A string where each bit is represented by zero or one
         *
         * \lang simp-chinese
         * @brief 将 bitset 转换为字符串表示。
         *
         * @tparam CharType 结果字符串的字符类型
         * @tparam Traits 结果字符串的字符特征
         * @tparam Allocator 结果字符串的分配器类型
         * @param zero 表示 0 的字符
         * @param one 表示 1 的字符
         * @return 每个位由 zero 或 one 表示的字符串
         */
        template <typename CharType = char, typename Traits = std::char_traits<CharType>, typename Allocator = memory::allocator<CharType>>
        constexpr core::text::basic_string<CharType, Traits, Allocator> to_string(CharType zero = CharType('0'), CharType one = CharType('1')) const {
            core::text::basic_string<CharType, Traits, Allocator> result(N, zero);
            for (std::size_t i = 0; i < N; ++i) {
                if (test(i)) {
                    result[N - 1 - i] = one;
                }
            }
            return result;
        }

        /**
         * \lang english
         * @brief Counts the number of bits that are set to 1.
         *
         * @return The number of set bits
         *
         * \lang simp-chinese
         * @brief 统计被设置为 1 的位的数量。
         *
         * @return 置位数量
         */
        constexpr std::size_t count() const noexcept {
            std::size_t result = 0;
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                unsigned long long w = data[i];
                while (w) {
                    w &= w - 1;
                    ++result;
                }
            }
            return result;
        }

        /**
         * \lang english
         * @brief Returns the number of bits in the bitset.
         *
         * @return N
         *
         * \lang simp-chinese
         * @brief 返回 bitset 中的位数。
         *
         * @return N
         */
        constexpr std::size_t size() const noexcept {
            return N;
        }

        /**
         * \lang english
         * @brief Compares two bitsets for equality.
         *
         * @param rhs The other bitset
         * @return true if all bits are equal, false otherwise
         *
         * \lang simp-chinese
         * @brief 比较两个 bitset 是否相等。
         *
         * @param rhs 另一个 bitset
         * @return 若所有位均相等则返回 true，否则返回 false
         */
        constexpr bool operator==(const bitset &rhs) const noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                if (data[i] != rhs.data[i]) {
                    return false;
                }
            }
            return true;
        }

        /**
         * \lang english
         * @brief Tests the bit at the given position.
         *
         * @param pos The position of the bit
         * @return true if the bit is set, false otherwise
         *
         * @throws std::out_of_range If pos is greater than or equal to N
         *
         * \lang simp-chinese
         * @brief 测试指定位置的位。
         *
         * @param pos 位的位置
         * @return 若该位已设置则返回 true，否则返回 false
         *
         * @throws std::out_of_range 若 pos 大于或等于 N
         */
        constexpr bool test(std::size_t pos) const {
            check_position(pos);
            return (data[word_index(pos)] & bit_mask(pos)) != 0;
        }

        /**
         * \lang english
         * @brief Checks whether all bits are set to 1.
         *
         * @return true if all bits are set, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查是否所有位都为 1。
         *
         * @return 若所有位均已设置则返回 true，否则返回 false
         */
        constexpr bool all() const noexcept {
            if constexpr (N == 0) {
                return true;
            }

            for (std::size_t i = 0; i < NUM_WORDS - 1; ++i) {
                if (data[i] != ~0ULL) {
                    return false;
                }
            }

            if constexpr (N % BITS_PER_WORD == 0) {
                return data[NUM_WORDS - 1] == ~0ULL;
            } else {
                unsigned long long mask = (1ULL << (N % BITS_PER_WORD)) - 1;
                return data[NUM_WORDS - 1] == mask;
            }
        }

        /**
         * \lang english
         * @brief Checks whether any bit is set to 1.
         *
         * @return true if at least one bit is set, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查是否有任意一位为 1。
         *
         * @return 若至少有一位被设置则返回 true，否则返回 false
         */
        constexpr bool any() const noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                if (data[i] != 0) {
                    return true;
                }
            }
            return false;
        }

        /**
         * \lang english
         * @brief Checks whether no bit is set to 1.
         *
         * @return true if no bit is set, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查是否没有一位为 1。
         *
         * @return 若没有位被设置则返回 true，否则返回 false
         */
        constexpr bool none() const noexcept {
            return !any();
        }

        /**
         * \lang english
         * @brief Returns the index of the storage word containing the given bit position.
         *
         * @param pos The bit position
         * @return The word index
         *
         * \lang simp-chinese
         * @brief 返回包含指定位的存储字索引。
         *
         * @param pos 位的位置
         * @return 字索引
         */
        static constexpr std::size_t word_index(std::size_t pos) noexcept {
            return pos / BITS_PER_WORD;
        }

        /**
         * \lang english
         * @brief Returns the bit index within a storage word for the given bit position.
         *
         * @param pos The bit position
         * @return The bit index within the word
         *
         * \lang simp-chinese
         * @brief 返回指定位在存储字内的位索引。
         *
         * @param pos 位的位置
         * @return 字内的位索引
         */
        static constexpr std::size_t bit_index(std::size_t pos) noexcept {
            return pos % BITS_PER_WORD;
        }

        /**
         * \lang english
         * @brief Returns the bit mask for the given bit position.
         *
         * @param pos The bit position
         * @return A mask with a single bit set at the given position
         *
         * \lang simp-chinese
         * @brief 返回指定位的位掩码。
         *
         * @param pos 位的位置
         * @return 在指定位置设置单个位的掩码
         */
        static constexpr unsigned long long bit_mask(std::size_t pos) noexcept {
            return 1ULL << bit_index(pos);
        }

        /**
         * \lang english
         * @brief Clears the unused high bits in the last storage word.
         *
         * \lang simp-chinese
         * @brief 清除最后一个存储字中未使用的高位。
         */
        constexpr void sanitize() noexcept {
            if constexpr (N % BITS_PER_WORD != 0) {
                data[NUM_WORDS - 1] &= (1ULL << (N % BITS_PER_WORD)) - 1;
            }
        }

        /**
         * \lang english
         * @brief Validates that the given position is within range.
         *
         * @param pos The bit position
         *
         * @throws std::out_of_range If pos is greater than or equal to N
         *
         * \lang simp-chinese
         * @brief 校验给定的位置是否在范围内。
         *
         * @param pos 位的位置
         *
         * @throws std::out_of_range 若 pos 大于或等于 N
         */
        constexpr void check_position(std::size_t pos) const {
            if (pos >= N) {
                throw std::out_of_range("bitset::check_position");
            }
        }

        /**
         * \lang english
         * @brief The underlying storage words holding the bits.
         *
         * \lang simp-chinese
         * @brief 保存位的底层存储字。
         */
        unsigned long long data[NUM_WORDS > 0 ? NUM_WORDS : 1] = {0};
    };


    /**
     * \lang english
     * @brief Performs bitwise AND between two bitsets.
     *
     * @tparam N The number of bits
     * @param lhs The left bitset
     * @param rhs The right bitset
     * @return A new bitset with the bitwise AND of lhs and rhs
     *
     * \lang simp-chinese
     * @brief 对两个 bitset 执行按位与运算。
     *
     * @tparam N 位的数量
     * @param lhs 左 bitset
     * @param rhs 右 bitset
     * @return lhs 与 rhs 按位与的新 bitset
     */
    template <std::size_t N>
    constexpr bitset<N> operator&(const bitset<N> &lhs, const bitset<N> &rhs) noexcept {
        bitset<N> result(lhs);
        result &= rhs;
        return result;
    }

    /**
     * \lang english
     * @brief Performs bitwise OR between two bitsets.
     *
     * @tparam N The number of bits
     * @param lhs The left bitset
     * @param rhs The right bitset
     * @return A new bitset with the bitwise OR of lhs and rhs
     *
     * \lang simp-chinese
     * @brief 对两个 bitset 执行按位或运算。
     *
     * @tparam N 位的数量
     * @param lhs 左 bitset
     * @param rhs 右 bitset
     * @return lhs 与 rhs 按位或的新 bitset
     */
    template <std::size_t N>
    constexpr bitset<N> operator|(const bitset<N> &lhs, const bitset<N> &rhs) noexcept {
        bitset<N> result(lhs);
        result |= rhs;
        return result;
    }

    /**
     * \lang english
     * @brief Performs bitwise XOR between two bitsets.
     *
     * @tparam N The number of bits
     * @param lhs The left bitset
     * @param rhs The right bitset
     * @return A new bitset with the bitwise XOR of lhs and rhs
     *
     * \lang simp-chinese
     * @brief 对两个 bitset 执行按位异或运算。
     *
     * @tparam N 位的数量
     * @param lhs 左 bitset
     * @param rhs 右 bitset
     * @return lhs 与 rhs 按位异或的新 bitset
     */
    template <std::size_t N>
    constexpr bitset<N> operator^(const bitset<N> &lhs, const bitset<N> &rhs) noexcept {
        bitset<N> result(lhs);
        result ^= rhs;
        return result;
    }
}


namespace rainy::utility {
    /**
     * \lang english
     * @brief Hash support for rainy::core::container::bitset in rainy::utility.
     *
     * @tparam N The number of bits
     *
     * \lang simp-chinese
     * @brief rainy::utility 中针对 rainy::core::container::bitset 的哈希支持。
     *
     * @tparam N 位的数量
     */
    template <std::size_t N>
    struct hash<rainy::core::container::bitset<N>> {
        /**
         * \lang english
         * @brief Computes a hash value for the given bitset.
         *
         * @param bs The bitset to hash
         * @return The hash value
         *
         * \lang simp-chinese
         * @brief 计算给定 bitset 的哈希值。
         *
         * @param bs 要哈希的 bitset
         * @return 哈希值
         */
        std::size_t operator()(const rainy::core::container::bitset<N> &bs) const noexcept {
            std::size_t result = 0;
            for (std::size_t i = 0; i < rainy::core::container::bitset<N>::NUM_WORDS; ++i) {
                result ^= hash<unsigned long long>{}(bs.data[i]) + 0x9e3779b9 + (result << 6) + (result >> 2);
            }
            return result;
        }
    };
}

namespace std {
    /**
     * \lang english
     * @brief Hash support for rainy::core::container::bitset in namespace std.
     *
     * @tparam N The number of bits
     *
     * \lang simp-chinese
     * @brief std 命名空间中针对 rainy::core::container::bitset 的哈希支持。
     *
     * @tparam N 位的数量
     */
    template <std::size_t N>
    struct hash<rainy::core::container::bitset<N>> {
        /**
         * \lang english
         * @brief Computes a hash value for the given bitset.
         *
         * @param bs The bitset to hash
         * @return The hash value
         *
         * \lang simp-chinese
         * @brief 计算给定 bitset 的哈希值。
         *
         * @param bs 要哈希的 bitset
         * @return 哈希值
         */
        constexpr std::size_t operator()(const rainy::core::container::bitset<N> &bs) const noexcept {
            std::size_t result = 0;
            for (std::size_t i = 0; i < N; ++i) {
                if (bs.test(i)) {
                    result ^= hash<std::size_t>{}(i) + 0x9e3779b9 + (result << 6) + (result >> 2);
                }
            }
            return result;
        }
    };
}

#endif
