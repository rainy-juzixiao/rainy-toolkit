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
                constexpr size_t block_size = 8;
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
#if RAINY_USING_AVX2
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
                    for (size_t idx = 0; idx != count; ++idx) {
                        to[idx] = from[idx];
                    }
                } else {
                    for (size_t idx = count; idx != 0; --idx) {
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
                for (size_t idx = 0; idx != count; ++idx) {
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

    template <typename Elem, typename Traits = char_traits<Elem>>
    class basic_string_view {
    public:
        using value_type = Elem;
        using pointer = const value_type *;
        using const_pointer = const value_type *;
        using size_type = std::size_t;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using reference = const value_type &;
        using const_reference = const value_type &;
        using traits_type = Traits;

        static inline constexpr size_type npos = static_cast<size_type>(-1);

        constexpr basic_string_view() noexcept : view_data(nullptr) {};

        RAINY_CONSTEXPR20 basic_string_view(const value_type *string) noexcept :
            view_data(string), view_size(traits_type::length(string)) {
        }

        RAINY_CONSTEXPR20 basic_string_view(const std::basic_string<Elem> &stdstring) noexcept :
            view_data(stdstring.c_str()), view_size(stdstring.size()) {
        }

        constexpr basic_string_view(const value_type *string, const size_type size) noexcept : view_data(string), view_size(size) {
        }

        template <std::size_t N>
        constexpr basic_string_view(const value_type (&string)[N]) noexcept {
        }

        RAINY_NODISCARD constexpr const_reference operator[](const std::size_t idx) const {
#if RAINY_ENABLE_DEBUG
            utility::expects(
                !empty(), "can not call " RAINY_STRINGIZE(basic_string_view<value_type, traits_type>::front) " on empty string_view");
            range_check(idx);
#endif
            return view_data[idx];
        }

        // 元素访问
        RAINY_NODISCARD constexpr const_reference at(const std::size_t idx) const {
#if RAINY_ENABLE_DEBUG
            utility::expects(!empty(),
                             "can not call " RAINY_STRINGIZE(basic_string_view<value_type, traits_type>::at) " on empty string_view");
#endif
            range_check(idx);
            return view_data[idx];
        }

        RAINY_NODISCARD constexpr const_reference back() const noexcept {
#if RAINY_ENABLE_DEBUG
            utility::expects(
                !empty(), "can not call " RAINY_STRINGIZE(basic_string_view<value_type, traits_type>::back) " on empty string_view");
#endif
            return view_data[view_size - 1];
        }

        RAINY_NODISCARD constexpr const_reference front() const noexcept {
#if RAINY_ENABLE_DEBUG
            utility::expects(
                !empty(), "can not call " RAINY_STRINGIZE(basic_string_view<value_type, traits_type>::front) " on empty string_view");
#endif
            return view_data[0];
        }

        RAINY_NODISCARD constexpr pointer data() const noexcept {
            return view_data;
        }

        RAINY_NODISCARD constexpr pointer c_str() const noexcept {
            return view_data;
        }

        // 容量
        RAINY_NODISCARD constexpr size_type size() const noexcept {
            return view_size;
        }

        RAINY_NODISCARD constexpr size_type length() const noexcept {
            return view_size;
        }

        RAINY_NODISCARD constexpr size_type max_size() const noexcept {
            return (std::min) (static_cast<size_t>(PTRDIFF_MAX), static_cast<size_t>(-1) / sizeof(value_type));
        }

        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return view_size == 0;
        }

        // 迭代器
        RAINY_NODISCARD constexpr const_iterator begin() const noexcept {
            return const_iterator(view_data);
        }

        RAINY_NODISCARD constexpr const_iterator end() const noexcept {
            return const_iterator(view_data + view_size);
        }

        RAINY_NODISCARD constexpr const_iterator cbegin() const noexcept {
            return const_iterator(view_data);
        }

        RAINY_NODISCARD constexpr const_iterator cend() const noexcept {
            return const_iterator(view_data + view_size);
        }

        RAINY_NODISCARD constexpr basic_string_view substr(const size_type offset = 0, size_type count = npos) const {
            range_check(offset);
            count = clamp_suffix_size(offset, count);
            return basic_string_view(view_data + offset, count);
        }


        template <typename... Args>
        std::basic_string<value_type> make_format(Args... fmt_args) const {
            std::basic_string<value_type> buffer;
#if RAINY_HAS_CXX20
            utility::basic_format(buffer, std::basic_string_view{view_data, view_size}, fmt_args...);
#else
            utility::cstyle_format(buffer, view_data, fmt_args...);
#endif
            return buffer;
        }

        template <typename Elem_, typename Uty>
        friend std::basic_ostream<Elem_> &operator<<(std::basic_ostream<Elem_> &ostream, const basic_string_view<Uty> &right) {
            ostream.write(right.view_data, right.view_size);
            return ostream;
        }

    private:
        RAINY_NODISCARD constexpr size_type clamp_suffix_size(const size_type offset, const size_type count) const noexcept {
            return (std::min) (count, view_size - offset);
        }


        constexpr void range_check(const std::size_t idx) const {
            if (view_size <= idx) {
                foundation::exceptions::logic::throw_out_of_range("Invalid subscript");
            }
        }

        pointer view_data{nullptr};
        size_type view_size{0};
    };

    using string_view = basic_string_view<char>;
    using wstring_view = basic_string_view<wchar_t>;
}

#endif
