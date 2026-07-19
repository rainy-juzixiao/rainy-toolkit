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
    template <typename Elem>
    struct char_traits {
        using char_type = Elem;
        using int_type = int;
        using off_type = std::streamoff;
        using pos_type = std::streampos;
        using state_type = std::mbstate_t;
        using size_type = std::size_t;
#if RAINY_HAS_CXX20
        using comparison_category = std::strong_ordering;
#endif

        static constexpr void assign(char_type &char_to, const char_type &char_from) noexcept {
            char_to = char_from;
        }

        static RAINY_CONSTEXPR20 void assign(char_type *char_to, const size_type num, const char_type &char_from) {
            for (int i = 0; i < num; ++i) {
                char_to[i] = char_from;
            }
        }

        static RAINY_CONSTEXPR20 bool eq(const char_type left, const char_type right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 bool eq_int_type(const int_type &left, const int_type &right) noexcept {
            return left == right;
        }

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

        static RAINY_CONSTEXPR20 bool lt(const char_type left, const char_type right) {
            return left < right;
        }

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

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(std::array<char_type, N> &to, const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(collections::array<char_type, N> &to, const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(rainy::collections::views::array_view<char_type> &to, const char_type *from,
                                                 const size_type count) {
            if (to.size() < count || to.empty()) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(std::vector<char_type> &to, const char_type *from, const size_type count) {
            if (to.size() < count || to.empty()) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move_s(Elem (&to)[N], const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to, from, N);
        }

        static RAINY_CONSTEXPR20 char_type *move_s(char_type *dest, const size_type dest_size, const char_type *from,
                                                   const size_type count) {
            if (dest_size < count) {
                return nullptr;
            }
            return move(dest, from, count);
        }

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

        static constexpr int_type to_int_type(const int_type &ch) {
            return ch;
        }

        static constexpr char_type to_char_type(const int_type &ch) {
            return ch;
        }

        static constexpr int_type eof() {
            return EOF;
        }

        static constexpr int_type not_eof(const int_type &ch) {
            return ch != eof();
        }
    };
}

#endif
