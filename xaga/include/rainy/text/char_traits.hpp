#ifndef RAINY_FOUNDATION_TEXT_CHAR_TRAITS_HPP
#define RAINY_FOUNDATION_TEXT_CHAR_TRAITS_HPP
#include <rainy/text/format_wrapper.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/core/core.hpp>

namespace rainy::text {
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
            return core::builtin::string_length(string);
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
            } else
#endif
            {
#if RAINY_USING_AVX2 && RAINY_IS_X86_PLATFORM
                auto *bytes = reinterpret_cast<const unsigned char *>(string);
                const __m128i target_vector = _mm_set1_epi8(target);
                for (std::size_t i = 0; i + 16 <= count; i += 16) {
                    const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i *>(bytes + i));
                    const __m128i cmp = _mm_cmpeq_epi8(chunk, target_vector);
                    if (const int mask = _mm_movemask_epi8(cmp); mask != 0) {
                        return string + i + rainy::core::builtin::ctz_avx2(mask);
                    }
                }
                for (std::size_t i = count & ~0xF; i < count; ++i) {
                    if (bytes[i] == target) {
                        return string + i;
                    }
                }
#else
                for (; 0 < count; --count, ++string) {
                    if (*string == target) {
                        return string;
                    }
                }
#endif
            }
            return nullptr;
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

        static RAINY_CONSTEXPR20 char_type *move(rainy::collections::views::array_view<char_type> &to, const char_type *from, const size_type count) {
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
