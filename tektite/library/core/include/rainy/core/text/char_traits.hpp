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
#ifndef RAINY_CORE_TEXT_CHAR_TRAITS_HPP
#define RAINY_CORE_TEXT_CHAR_TRAITS_HPP
#include <rainy/core/collections/array.hpp>
#include <rainy/core/collections/array_view.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::core::text::implements {
    template <typename CharType>
    static const CharType *find_impl_fallback(const CharType *string, std::size_t count, const CharType &target) {
        for (; 0 < count; --count, ++string) {
            if (*string == target) {
                return string;
            }
        }
        return nullptr;
    }

    template <>
    const char *find_impl_fallback(const char *string, std::size_t count, const char &target) {
        return static_cast<const char *>(std::memchr(string, target, count));
    }

    template<typename CharType>
    static const CharType* find_impl_char(const CharType* string, std::size_t count, const CharType& target) {
#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
        const auto* bytes = reinterpret_cast<const unsigned char*>(string);
        const __m256i target_vec = _mm256_set1_epi8(static_cast<unsigned char>(target));

        for (std::size_t i = 0; i + 32 <= count; i += 32) {
            const __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(bytes + i));
            const __m256i cmp = _mm256_cmpeq_epi8(chunk, target_vec);
            const int mask = _mm256_movemask_epi8(cmp);
            if (mask != 0) {
                return string + i + builtin::ctz_avx2(mask);
            }
        }

        std::size_t remaining_start = count & ~0x1F;
        for (std::size_t i = remaining_start; i + 16 <= count; i += 16) {
            const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bytes + i));
            const __m128i cmp = _mm_cmpeq_epi8(chunk, _mm_set1_epi8(static_cast<unsigned char>(target)));
            const int mask = _mm_movemask_epi8(cmp);
            if (mask != 0) {
                return string + i + builtin::ctz(mask);
            }
        }

        for (std::size_t i = count & ~0xF; i < count; ++i) {
            if (bytes[i] == target) {
                return string + i;
            }
        }
#else
        return implements::find_impl_fallback(string, count, target);
#endif
        return nullptr;
    }

    template<typename CharType>
    static const CharType* find_impl_wchar_16bit(const CharType* string, std::size_t count, const CharType& target) {
#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
        const __m256i target_vec = _mm256_set1_epi16(static_cast<uint16_t>(target));

        for (std::size_t i = 0; i + 16 <= count; i += 16) {
            const __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(string + i));
            const __m256i cmp = _mm256_cmpeq_epi16(chunk, target_vec);
            const int mask = _mm256_movemask_epi8(cmp);
            if (mask != 0) {
                int byte_offset = builtin::ctz_avx2(mask);
                int element_offset = byte_offset / 2;
                return string + i + element_offset;
            }
        }

        std::size_t remaining_start = count & ~0xF;
        for (std::size_t i = remaining_start; i + 8 <= count; i += 8) {
            const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(string + i));
            const __m128i cmp = _mm_cmpeq_epi16(chunk, _mm_set1_epi16(static_cast<uint16_t>(target)));
            const int mask = _mm_movemask_epi8(cmp);
            if (mask != 0) {
                int byte_offset = builtin::ctz(mask);
                int element_offset = byte_offset / 2;
                return string + i + element_offset;
            }
        }

        for (std::size_t i = count & ~0x7; i < count; ++i) {
            if (string[i] == target) {
                return string + i;
            }
        }
#else
        return implements::find_impl_fallback(string, count, target);
#endif
        return nullptr;
    }

    template<typename CharType>
    static const CharType* find_impl_wchar_32bit(const CharType* string, std::size_t count, const CharType& target) {
#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
        const __m256i target_vec = _mm256_set1_epi32(static_cast<uint32_t>(target));

        for (std::size_t i = 0; i + 8 <= count; i += 8) {
            const __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(string + i));
            const __m256i cmp = _mm256_cmpeq_epi32(chunk, target_vec);
            const int mask = _mm256_movemask_epi8(cmp);
            if (mask != 0) {
                int byte_offset = builtin::ctz_avx2(mask);
                int element_offset = byte_offset / 4;
                return string + i + element_offset;
            }
        }

        std::size_t remaining_start = count & ~0x7;
        for (std::size_t i = remaining_start; i + 4 <= count; i += 4) {
            const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(string + i));
            const __m128i cmp = _mm_cmpeq_epi32(chunk, _mm_set1_epi32(static_cast<uint32_t>(target)));
            const int mask = _mm_movemask_epi8(cmp);
            if (mask != 0) {
                int byte_offset = builtin::ctz(mask);
                int element_offset = byte_offset / 4;
                return string + i + element_offset;
            }
        }

        for (std::size_t i = count & ~0x3; i < count; ++i) {
            if (string[i] == target) {
                return string + i;
            }
        }
#else
        return find_impl_fallback(string, count, target);
#endif
        return nullptr;
    }

    template <typename CharType>
    RAINY_NODISCARD static RAINY_CONSTEXPR20 const CharType *find_impl(const CharType *string, std::size_t count, const CharType &target) {
        if constexpr (sizeof(CharType) == 1) {
            return find_impl_char(string, count, target);
        } else if constexpr (sizeof(CharType) == 2) {
            return find_impl_wchar_16bit(string, count, target);
        } else if constexpr (sizeof(CharType) == 4) {
            return find_impl_wchar_32bit(string, count, target);
        } else {
            return find_impl_fallback(string, count, target);
        }
    }
}

namespace rainy::core::text {
    /**
     * \lang english
     * @brief Character traits that define operations on character sequences for text classes.
     *
     * @tparam Elem The character element type
     *
     * \lang simp-chinese
     * @brief 定义文本类对字符序列操作所需的字符 traits。
     *
     * @tparam Elem 字符元素类型
     */
    template <typename Elem>
    struct char_traits {
        /**
         * \lang english
         * @brief The character element type.
         *
         * \lang simp-chinese
         * @brief 字符元素类型。
         */
        using char_type = Elem;
        /**
         * \lang english
         * @brief An integer type able to represent all characters plus the end-of-file value.
         *
         * \lang simp-chinese
         * @brief 能够表示所有字符以及文件结束值的整数类型。
         */
        using int_type = int;
        /**
         * \lang english
         * @brief The type used to represent stream offsets.
         *
         * \lang simp-chinese
         * @brief 用于表示流偏移量的类型。
         */
        using off_type = std::streamoff;
        /**
         * \lang english
         * @brief The type used to represent stream positions.
         *
         * \lang simp-chinese
         * @brief 用于表示流位置的类型。
         */
        using pos_type = std::streampos;
        /**
         * \lang english
         * @brief The type used to represent conversion state.
         *
         * \lang simp-chinese
         * @brief 用于表示转换状态的类型。
         */
        using state_type = std::mbstate_t;
        /**
         * \lang english
         * @brief The unsigned type used to represent sizes and counts.
         *
         * \lang simp-chinese
         * @brief 用于表示大小与数量的无符号类型。
         */
        using size_type = std::size_t;
#if RAINY_HAS_CXX20
        /**
         * \lang english
         * @brief The comparison category type used by three-way comparison.
         *
         * \lang simp-chinese
         * @brief 三路比较使用的比较类别类型。
         */
        using comparison_category = std::strong_ordering;
#endif

        /**
         * \lang english
         * @brief Assigns one character to another.
         *
         * @param char_to The target character
         * @param char_from The source character
         *
         * \lang simp-chinese
         * @brief 将一个字符赋值给另一个字符。
         *
         * @param char_to 目标字符
         * @param char_from 源字符
         */
        static constexpr void assign(char_type &char_to, const char_type &char_from) noexcept {
            char_to = char_from;
        }

        /**
         * \lang english
         * @brief Assigns the same character to each element of a character array.
         *
         * @param char_to Pointer to the destination array
         * @param num The number of characters to assign
         * @param char_from The character to assign
         *
         * \lang simp-chinese
         * @brief 将同一个字符赋值给字符数组中的每个元素。
         *
         * @param char_to 目标数组指针
         * @param num 待赋值的字符数量
         * @param char_from 待赋值的字符
         */
        static RAINY_CONSTEXPR20 void assign(char_type *char_to, const size_type num, const char_type &char_from) {
            for (int i = 0; i < num; ++i) {
                char_to[i] = char_from;
            }
        }

        /**
         * \lang english
         * @brief Compares two characters for equality.
         *
         * @param left The left character
         * @param right The right character
         * @return true if both characters are equal
         *
         * \lang simp-chinese
         * @brief 比较两个字符是否相等。
         *
         * @param left 左字符
         * @param right 右字符
         * @return 两个字符相等时返回 true
         */
        static RAINY_CONSTEXPR20 bool eq(const char_type left, const char_type right) noexcept {
            return left == right;
        }

        /**
         * \lang english
         * @brief Compares two int_type values for equality.
         *
         * @param left The left value
         * @param right The right value
         * @return true if both values are equal
         *
         * \lang simp-chinese
         * @brief 比较两个 int_type 值是否相等。
         *
         * @param left 左值
         * @param right 右值
         * @return 两个值相等时返回 true
         */
        static RAINY_CONSTEXPR20 bool eq_int_type(const int_type &left, const int_type &right) noexcept {
            return left == right;
        }

        /**
         * \lang english
         * @brief Lexicographically compares up to count characters of two sequences.
         *
         * @param string1 The first character sequence
         * @param string2 The second character sequence
         * @param count The maximum number of characters to compare
         * @return A negative value if the first sequence is less, zero if equal, a positive value otherwise
         *
         * \lang simp-chinese
         * @brief 按字典序比较两个序列至多 count 个字符。
         *
         * @param string1 第一个字符序列
         * @param string2 第二个字符序列
         * @param count 最多比较的字符数
         * @return 第一个序列较小时为负值，相等时为零，否则为正值
         */
        static constexpr int compare(const char_type *string1, const char_type *string2, const size_type count) noexcept {
            if constexpr (type_traits::helper::is_wchar_t<char_type>) {
                return core::builtin::compare_string(string1, string2, count);
            } else if constexpr (type_traits::type_relations::is_any_of_v<char_type, char16_t, char32_t>) {
                if (string1 == string2 || count == 0) {
                    return 0;
                }
                const char_type *s1 = string1;
                const char_type *s2 = string2;
                const char_type *end = s1 + count;
                constexpr std::size_t block_size = 8;
                const char_type *block_end = s1 + (count / block_size) * block_size;
                while (s1 < block_end) {
                    if (s1[0] != s2[0]) {
                        return (s1[0] < s2[0]) ? -1 : 1;
                    }
                    if (s1[1] != s2[1]) {
                        return (s1[1] < s2[1]) ? -1 : 1;
                    }
                    if (s1[2] != s2[2]) {
                        return (s1[2] < s2[2]) ? -1 : 1;
                    }
                    if (s1[3] != s2[3]) {
                        return (s1[3] < s2[3]) ? -1 : 1;
                    }
                    if (s1[4] != s2[4]) {
                        return (s1[4] < s2[4]) ? -1 : 1;
                    }
                    if (s1[5] != s2[5]) {
                        return (s1[5] < s2[5]) ? -1 : 1;
                    }
                    if (s1[6] != s2[6]) {
                        return (s1[6] < s2[6]) ? -1 : 1;
                    }
                    if (s1[7] != s2[7]) {
                        return (s1[7] < s2[7]) ? -1 : 1;
                    }
                    s1 += block_size;
                    s2 += block_size;
                }
                while (end - s1 >= 4) {
                    if (s1[0] != s2[0]) {
                        return (s1[0] < s2[0]) ? -1 : 1;
                    }
                    if (s1[1] != s2[1]) {
                        return (s1[1] < s2[1]) ? -1 : 1;
                    }
                    if (s1[2] != s2[2]) {
                        return (s1[2] < s2[2]) ? -1 : 1;
                    }
                    if (s1[3] != s2[3]) {
                        return (s1[3] < s2[3]) ? -1 : 1;
                    }
                    s1 += 4;
                    s2 += 4;
                }
                if (end - s1 >= 2) {
                    if (s1[0] != s2[0]) {
                        return (s1[0] < s2[0]) ? -1 : 1;
                    }
                    if (s1[1] != s2[1]) {
                        return (s1[1] < s2[1]) ? -1 : 1;
                    }
                    s1 += 2;
                    s2 += 2;
                }
                if (s1 < end) {
                    if (*s1 != *s2) {
                        return (*s1 < *s2) ? -1 : 1;
                    }
                }
                return 0;
            } else {
                return core::builtin::compare_string(string1, string2, count);
            }
        }

        /**
         * \lang english
         * @brief Returns the length of a null-terminated character sequence.
         *
         * @param string The null-terminated character sequence
         * @return The number of characters before the terminating null
         *
         * \lang simp-chinese
         * @brief 返回以空字符结尾的字符序列的长度。
         *
         * @param string 以空字符结尾的字符序列
         * @return 终止空字符之前的字符数量
         */
        static constexpr size_type length(const char_type *string) {
            if constexpr (type_traits::implements::is_same_v<char, char_type> ||
                          type_traits::implements::is_same_v<wchar_t, char_type>) {
                return core::builtin::string_length(string);
            } else {
                size_type length{};
                for (; *string != '\0'; ++string, ++length) {
                    ;
                }
                return length;
            }
        }

        /**
         * \lang english
         * @brief Finds the first occurrence of a character within a sequence.
         *
         * @param string The character sequence to search
         * @param count The number of characters to search
         * @param target The character to find
         * @return A pointer to the first match, or nullptr if not found
         *
         * \lang simp-chinese
         * @brief 在序列中查找字符首次出现的位置。
         *
         * @param string 待搜索的字符序列
         * @param count 待搜索的字符数量
         * @param target 待查找的字符
         * @return 指向首次匹配位置的指针，未找到时返回 nullptr
         */
        RAINY_NODISCARD static RAINY_CONSTEXPR20 const char_type *find(const char_type *string, std::size_t count,
                                                                       const char_type &target) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (; 0 < count; --count, ++string) {
                    if (*string == target) {
                        return string;
                    }
                }
                return nullptr;
            } else
#endif
            {
                return implements::find_impl(string, count, target);
            }
        }

        /**
         * \lang english
         * @brief Compares two characters with the less-than operator.
         *
         * @param left The left character
         * @param right The right character
         * @return true if left is less than right
         *
         * \lang simp-chinese
         * @brief 使用小于运算符比较两个字符。
         *
         * @param left 左字符
         * @param right 右字符
         * @return left 小于 right 时返回 true
         */
        static RAINY_CONSTEXPR20 bool lt(const char_type left, const char_type right) {
            return left < right;
        }

        /**
         * \lang english
         * @brief Moves count characters from one sequence to another, allowing overlap.
         *
         * @param to Pointer to the destination sequence
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination sequence
         *
         * \lang simp-chinese
         * @brief 将 count 个字符从源序列移动到目标序列，允许重叠。
         *
         * @param to 目标序列指针
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标序列指针
         */
        static RAINY_CONSTEXPR20 char_type *move(char_type *to, const char_type *from, size_type count) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                bool loop_forward = true;
                for (const Elem *source = from; source != from + count; ++source) {
                    if (to == source) {
                        loop_forward = false;
                        break;
                    }
                }
                if (loop_forward) {
                    for (std::size_t idx = 0; idx != count; ++idx) {
                        to[idx] = from[idx];
                    }
                } else {
                    for (std::size_t idx = count; idx != 0; --idx) {
                        to[idx - 1] = from[idx - 1];
                    }
                }
                return to;
            }
#endif
#if RAINY_USING_MSVC
            errno_t error = memmove_s(to, count, from, count);
            if (error != 0) {
                std::abort();
            }
#elif RAINY_USING_CLANG
            __builtin_memmove(to, from, count);
#elif RAINY_USING_GCC
            memmove(to, from, count);
#endif
            return to;
        }

        /**
         * \lang english
         * @brief Moves characters into a std::array destination, returning nullptr if the array is too small.
         *
         * @tparam N The size of the destination array
         * @param to The destination std::array
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination data, or nullptr if N is smaller than count
         *
         * \lang simp-chinese
         * @brief 将字符移动到 std::array 目标中，数组过小时返回 nullptr。
         *
         * @tparam N 目标数组的大小
         * @param to 目标 std::array
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标数据指针；N 小于 count 时返回 nullptr
         */
        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(std::array<char_type, N> &to, const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        /**
         * \lang english
         * @brief Moves characters into a rainy::collections::array destination, returning nullptr if the array is too small.
         *
         * @tparam N The size of the destination array
         * @param to The destination array
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination data, or nullptr if N is smaller than count
         *
         * \lang simp-chinese
         * @brief 将字符移动到 rainy::collections::array 目标中，数组过小时返回 nullptr。
         *
         * @tparam N 目标数组的大小
         * @param to 目标数组
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标数据指针；N 小于 count 时返回 nullptr
         */
        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(rainy::collections::array<char_type, N> &to, const char_type *from,
                                                  const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        /**
         * \lang english
         * @brief Moves characters into an array_view destination, returning nullptr if the view is too small or empty.
         *
         * @param to The destination array_view
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination data, or nullptr if the view cannot hold count characters
         *
         * \lang simp-chinese
         * @brief 将字符移动到 array_view 目标中，视图过小或为空时返回 nullptr。
         *
         * @param to 目标 array_view
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标数据指针；视图无法容纳 count 个字符时返回 nullptr
         */
        static RAINY_CONSTEXPR20 char_type *move(rainy::collections::views::array_view<char_type> &to, const char_type *from,
                                                 const size_type count) {
            if (to.size() < count || to.empty()) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        /**
         * \lang english
         * @brief Moves characters into a std::vector destination, returning nullptr if the vector is too small or empty.
         *
         * @param to The destination std::vector
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination data, or nullptr if the vector cannot hold count characters
         *
         * \lang simp-chinese
         * @brief 将字符移动到 std::vector 目标中，vector 过小或为空时返回 nullptr。
         *
         * @param to 目标 std::vector
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标数据指针；vector 无法容纳 count 个字符时返回 nullptr
         */
        static RAINY_CONSTEXPR20 char_type *move(std::vector<char_type> &to, const char_type *from, const size_type count) {
            if (to.size() < count || to.empty()) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        template <size_type N>
        /**
         * \lang english
         * @brief Moves characters into a raw C-array destination, returning nullptr if the array is too small.
         *
         * @tparam N The size of the destination array
         * @param to The destination C-array
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination array, or nullptr if N is smaller than count
         *
         * \lang simp-chinese
         * @brief 将字符移动到原生 C 数组目标中，数组过小时返回 nullptr。
         *
         * @tparam N 目标数组的大小
         * @param to 目标 C 数组
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标数组指针；N 小于 count 时返回 nullptr
         */
        static RAINY_CONSTEXPR20 char_type *move_s(Elem (&to)[N], const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to, from, N);
        }

        /**
         * \lang english
         * @brief Bounds-checked move into a destination with explicit size, returning nullptr on overflow.
         *
         * @param dest Pointer to the destination buffer
         * @param dest_size The size of the destination buffer
         * @param from Pointer to the source sequence
         * @param count The number of characters to move
         * @return Pointer to the destination buffer, or nullptr if dest_size is smaller than count
         *
         * \lang simp-chinese
         * @brief 带边界检查的移动，目标缓冲区显式给出大小，溢出时返回 nullptr。
         *
         * @param dest 目标缓冲区指针
         * @param dest_size 目标缓冲区的大小
         * @param from 源序列指针
         * @param count 待移动的字符数量
         * @return 目标缓冲区指针；dest_size 小于 count 时返回 nullptr
         */
        static RAINY_CONSTEXPR20 char_type *move_s(char_type *dest, const size_type dest_size, const char_type *from,
                                                   const size_type count) {
            if (dest_size < count) {
                return nullptr;
            }
            return move(dest, from, count);
        }

        /**
         * \lang english
         * @brief Copies count characters from one sequence to another; the ranges must not overlap.
         *
         * @param string1 Pointer to the destination sequence
         * @param string2 Pointer to the source sequence
         * @param count The number of characters to copy
         * @return Pointer to the destination sequence
         *
         * \lang simp-chinese
         * @brief 将 count 个字符从源序列复制到目标序列；两个范围不得重叠。
         *
         * @param string1 目标序列指针
         * @param string2 源序列指针
         * @param count 待复制的字符数量
         * @return 目标序列指针
         */
        static RAINY_CONSTEXPR20 char_type *copy(char_type *const string1, const char_type *const string2,
                                                 const size_type count) noexcept /* strengthened */ {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (std::size_t idx = 0; idx != count; ++idx) {
                    string1[idx] = string2[idx];
                }
                return string1;
            }
#endif
            core::builtin::copy_memory(string1, string2, count * sizeof(char_type));
            return string1;
        }

        /**
         * \lang english
         * @brief Converts a character to its int_type representation.
         *
         * @param ch The character to convert
         * @return The int_type representation
         *
         * \lang simp-chinese
         * @brief 将字符转换为其 int_type 表示。
         *
         * @param ch 待转换的字符
         * @return int_type 表示
         */
        static constexpr int_type to_int_type(const int_type &ch) {
            return ch;
        }

        /**
         * \lang english
         * @brief Converts an int_type value back to a character.
         *
         * @param ch The int_type value to convert
         * @return The character value
         *
         * \lang simp-chinese
         * @brief 将 int_type 值转换回字符。
         *
         * @param ch 待转换的 int_type 值
         * @return 字符值
         */
        static constexpr char_type to_char_type(const int_type &ch) {
            return ch;
        }

        /**
         * \lang english
         * @brief Returns the end-of-file int_type value.
         *
         * @return The EOF value
         *
         * \lang simp-chinese
         * @brief 返回文件结束的 int_type 值。
         *
         * @return EOF 值
         */
        static constexpr int_type eof() {
            return EOF;
        }

        /**
         * \lang english
         * @brief Returns a value that is not equal to EOF, or the value itself if it is not EOF.
         *
         * @param ch The int_type value to check
         * @return A non-EOF int_type value
         *
         * \lang simp-chinese
         * @brief 返回一个不等于 EOF 的值；若给定值本身不是 EOF 则返回该值。
         *
         * @param ch 待检查的 int_type 值
         * @return 非 EOF 的 int_type 值
         */
        static constexpr int_type not_eof(const int_type &ch) {
            return ch != eof();
        }
    };
}

namespace rainy::text {
    using core::text::char_traits;
}

#endif
