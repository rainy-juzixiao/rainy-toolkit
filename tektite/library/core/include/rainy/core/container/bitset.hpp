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
    template <std::size_t N>
    class bitset {
    public:
        static constexpr std::size_t BITS_PER_WORD = sizeof(unsigned long long) * CHAR_BIT;
        static constexpr std::size_t NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;

        class reference {
        public:
            friend class bitset;

            constexpr reference(const reference &) = default;
            constexpr ~reference() = default;

            constexpr reference &operator=(bool x) noexcept {
                if (x) {
                    bs->data[word_index(pos)] |= bit_mask(pos);
                } else {
                    bs->data[word_index(pos)] &= ~bit_mask(pos);
                }
                return *this;
            }

            constexpr reference &operator=(const reference &ref) noexcept {
                return *this = bool(ref);
            }

            constexpr bool operator~() const noexcept {
                return !bool(*this);
            }

            constexpr operator bool() const noexcept {
                return (bs->data[word_index(pos)] & bit_mask(pos)) != 0;
            }

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

        constexpr bitset() noexcept : data{} {
        }

        constexpr bitset(unsigned long long val) noexcept : data{} {
            data[0] = val;
            sanitize();
        }

        template <typename CharT, typename Traits, typename Allocator>
        constexpr explicit bitset(const core::text::basic_string<CharT, Traits, Allocator> &str,
                                  typename core::text::basic_string<CharT, Traits, Allocator>::size_type pos = 0,
                                  typename core::text::basic_string<CharT, Traits, Allocator>::size_type n =
                                      core::text::basic_string<CharT, Traits, Allocator>::npos,
                                  CharT zero = CharT('0'), CharT one = CharT('1')) :
            bitset(core::text::basic_string_view<CharT, Traits>(str), pos, n, zero, one) {
        }

        template <typename CharT, typename Traits>
        constexpr explicit bitset(
            core::text::basic_string_view<CharT, Traits> str, typename core::text::basic_string_view<CharT, Traits>::size_type pos = 0,
            typename core::text::basic_string_view<CharT, Traits>::size_type n = core::text::basic_string_view<CharT, Traits>::npos,
            CharT zero = CharT('0'), CharT one = CharT('1')) : data{} {
            if (pos > str.size()) {
                throw std::out_of_range("bitset::bitset");
            }

            std::size_t rlen = std::min(n, str.size() - pos);
            std::size_t bits_to_copy = std::min(rlen, N);

            for (std::size_t i = 0; i < bits_to_copy; ++i) {
                CharT ch = str[pos + rlen - 1 - i];
                if (Traits::eq(ch, one)) {
                    set(i);
                } else if (!Traits::eq(ch, zero)) {
                    throw std::invalid_argument("bitset::bitset");
                }
            }
        }

        template <typename CharT>
        constexpr explicit bitset(
            const CharT *str, typename core::text::basic_string_view<CharT>::size_type n = core::text::basic_string_view<CharT>::npos,
            CharT zero = CharT('0'), CharT one = CharT('1')) :
            bitset(n == core::text::basic_string_view<CharT>::npos ? core::text::basic_string_view<CharT>(str)
                                                                   : core::text::basic_string_view<CharT>(str, n),
                   0, n, zero, one) {
        }

        constexpr bitset &operator&=(const bitset &rhs) noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] &= rhs.data[i];
            }
            return *this;
        }

        constexpr bitset &operator|=(const bitset &rhs) noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] |= rhs.data[i];
            }
            return *this;
        }

        constexpr bitset &operator^=(const bitset &rhs) noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] ^= rhs.data[i];
            }
            return *this;
        }

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

        constexpr bitset operator<<(std::size_t pos) const noexcept {
            bitset result(*this);
            result <<= pos;
            return result;
        }

        constexpr bitset operator>>(std::size_t pos) const noexcept {
            bitset result(*this);
            result >>= pos;
            return result;
        }

        constexpr bitset &set() noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] = ~0ULL;
            }
            sanitize();
            return *this;
        }

        constexpr bitset &set(std::size_t pos, bool val = true) {
            check_position(pos);
            if (val) {
                data[word_index(pos)] |= bit_mask(pos);
            } else {
                data[word_index(pos)] &= ~bit_mask(pos);
            }
            return *this;
        }

        constexpr bitset &reset() noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] = 0;
            }
            return *this;
        }

        constexpr bitset &reset(std::size_t pos) {
            check_position(pos);
            data[word_index(pos)] &= ~bit_mask(pos);
            return *this;
        }

        constexpr bitset operator~() const noexcept {
            bitset result;
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                result.data[i] = ~data[i];
            }
            result.sanitize();
            return result;
        }

        constexpr bitset &flip() noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                data[i] = ~data[i];
            }
            sanitize();
            return *this;
        }

        constexpr bitset &flip(std::size_t pos) {
            check_position(pos);
            data[word_index(pos)] ^= bit_mask(pos);
            return *this;
        }

        constexpr bool operator[](std::size_t pos) const {
            return (data[word_index(pos)] & bit_mask(pos)) != 0;
        }

        constexpr reference operator[](std::size_t pos) {
            return reference(this, pos);
        }

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

        template <typename CharT = char, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
        constexpr core::text::basic_string<CharT, Traits, Allocator> to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const {
            core::text::basic_string<CharT, Traits, Allocator> result(N, zero);
            for (std::size_t i = 0; i < N; ++i) {
                if (test(i)) {
                    result[N - 1 - i] = one;
                }
            }
            return result;
        }

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

        constexpr std::size_t size() const noexcept {
            return N;
        }

        constexpr bool operator==(const bitset &rhs) const noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                if (data[i] != rhs.data[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool test(std::size_t pos) const {
            check_position(pos);
            return (data[word_index(pos)] & bit_mask(pos)) != 0;
        }

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

        constexpr bool any() const noexcept {
            for (std::size_t i = 0; i < NUM_WORDS; ++i) {
                if (data[i] != 0) {
                    return true;
                }
            }
            return false;
        }

        constexpr bool none() const noexcept {
            return !any();
        }

        static constexpr std::size_t word_index(std::size_t pos) noexcept {
            return pos / BITS_PER_WORD;
        }

        static constexpr std::size_t bit_index(std::size_t pos) noexcept {
            return pos % BITS_PER_WORD;
        }

        static constexpr unsigned long long bit_mask(std::size_t pos) noexcept {
            return 1ULL << bit_index(pos);
        }

        constexpr void sanitize() noexcept {
            if constexpr (N % BITS_PER_WORD != 0) {
                data[NUM_WORDS - 1] &= (1ULL << (N % BITS_PER_WORD)) - 1;
            }
        }

        constexpr void check_position(std::size_t pos) const {
            if (pos >= N) {
                throw std::out_of_range("bitset::check_position");
            }
        }

        unsigned long long data[NUM_WORDS > 0 ? NUM_WORDS : 1] = {0};
    };


    template <std::size_t N>
    constexpr bitset<N> operator&(const bitset<N> &lhs, const bitset<N> &rhs) noexcept {
        bitset<N> result(lhs);
        result &= rhs;
        return result;
    }

    template <std::size_t N>
    constexpr bitset<N> operator|(const bitset<N> &lhs, const bitset<N> &rhs) noexcept {
        bitset<N> result(lhs);
        result |= rhs;
        return result;
    }

    template <std::size_t N>
    constexpr bitset<N> operator^(const bitset<N> &lhs, const bitset<N> &rhs) noexcept {
        bitset<N> result(lhs);
        result ^= rhs;
        return result;
    }
}


namespace rainy::utility {
    template <std::size_t N>
    struct hash<rainy::core::container::bitset<N>> {
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
    template <std::size_t N>
    struct hash<rainy::core::container::bitset<N>> {
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