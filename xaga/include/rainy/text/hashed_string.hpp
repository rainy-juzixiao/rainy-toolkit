#ifndef RAINY_TEXT_HASHED_STRING_HPP
#define RAINY_TEXT_HASHED_STRING_HPP
#include <rainy/core/core.hpp>
#include <rainy/text/char_traits.hpp>

namespace rainy::text {
    template <typename CharType, typename Traits = char_traits<CharType>>
    class basic_hashed_string {
    public:
        using value_type = CharType;
        using pointer = const value_type *;
        using const_pointer = const value_type *;
        using traits_type = Traits;
        using size_type = std::size_t;

        constexpr basic_hashed_string() = default;

        constexpr basic_hashed_string(const_pointer ptr) noexcept :
            hash_val{fnv1a_hash(ptr)}, str{ptr}, size_{traits_type::length(ptr)} {
        }

        constexpr basic_hashed_string(const basic_hashed_string &right) noexcept :
            hash_val{right.hash_val}, str{right.str}, size_{right.size_} {
        }

        constexpr basic_hashed_string(basic_hashed_string &&right) noexcept :
            hash_val{utility::exchange(right.hash_val, 0)}, str{utility::exchange(right.str, nullptr)},
            size_{utility::exchange(right.size_, 0)} {
        }

        template <typename StringViewLike,
                  type_traits::other_trans::enable_if_t<type_traits::extras::meta_method::has_begin_v<StringViewLike> &&
                                                            type_traits::extras::meta_method::has_end_v<StringViewLike> &&
                                                            type_traits::extras::meta_method::has_size_v<StringViewLike> &&
                                                            type_traits::extras::meta_method::has_data_v<StringViewLike>,
                                                        int> = 0>
        constexpr basic_hashed_string(const StringViewLike &svlike) :
            hash_val{fnv1a_hash(svlike)}, str{svlike.data()}, size_{svlike.size()} {
        }

        constexpr basic_hashed_string &operator=(const basic_hashed_string &) noexcept = default;
        constexpr basic_hashed_string &operator=(basic_hashed_string &&) noexcept = default;

        constexpr bool empty() const noexcept {
            return hash_val == 0;
        }

        constexpr const_pointer data() const noexcept {
            return str;
        }

        constexpr size_type size() const noexcept {
            return size_;
        }

        constexpr size_type length() const noexcept {
            return size_;
        }

        static constexpr size_type max_size() noexcept {
            return (core::min)(static_cast<std::size_t>((utility::numeric_limits<std::ptrdiff_t>::max)()),
                               static_cast<std::size_t>(-1) / sizeof(value_type));
        }

        constexpr operator const_pointer() const noexcept {
            return str;
        }

        constexpr operator std::basic_string_view<value_type>() const noexcept {
            return {str, size_};
        }

        constexpr std::size_t hash_code() const noexcept {
            return hash_val;
        }

        friend constexpr bool operator==(const basic_hashed_string &left, const basic_hashed_string &right) noexcept {
            return left.hash_val == right.hash_val;
        }

        friend constexpr bool operator!=(const basic_hashed_string &left, const basic_hashed_string &right) noexcept {
            return left.hash_val != right.hash_val;
        }

        static constexpr basic_hashed_string make_hashed_string(const_pointer ptr) noexcept {
            return basic_hashed_string{ptr};
        }

        static constexpr std::size_t eval_hash(const basic_hashed_string &right) noexcept {
            return right.hash_val;
        }

    private:
        template <typename Input>
        static constexpr std::size_t fnv1a_hash(const Input &val,
                                                std::size_t offset_basis = rainy::utility::implements::fnv_offset_basis) noexcept {

            std::size_t hash = offset_basis;
            if constexpr (type_traits::extras::meta_method::has_begin_v<Input> && type_traits::extras::meta_method::has_end_v<Input>) {
                for (value_type ch: val) {
                    hash ^= static_cast<std::size_t>(static_cast<type_traits::helper::make_unsigned_t<value_type>>(ch));
                    hash *= rainy::utility::implements::fnv_prime;
                }
            } else {
                const_pointer cur{val};
                while (*cur != '\0') {
                    hash ^= static_cast<std::size_t>(static_cast<type_traits::helper::make_unsigned_t<value_type>>(*cur));
                    hash *= rainy::utility::implements::fnv_prime;
                    ++cur;
                }
            }
            return hash;
        }

        std::size_t hash_val{0};
        const_pointer str{nullptr};
        std::size_t size_{0};
    };

    using hashed_string = basic_hashed_string<char>;
    using whashed_string = basic_hashed_string<wchar_t>;
    using u16hashed_string = basic_hashed_string<char16_t>;
    using u32hashed_string = basic_hashed_string<char32_t>;
#if RAINY_HAS_CXX20
    using u8hashed_string = basic_hashed_string<char8_t>;
#endif
}

#define rainy_switch_hashed_string(x) switch (::rainy::text::hashed_string::eval_hash(x))
#define rainy_switch_whashed_string(x) switch (::rainy::text::whashed_string::eval_hash(x))
#define rainy_switch_u16hashed_string(x) switch (::rainy::text::u16hashed_string::eval_hash(x))
#define rainy_switch_u32hashed_string(x) switch (::rainy::text::u32hashed_string::eval_hash(x))
#if RAINY_HAS_CXX20
#define rainy_switch_u8hashed_string(x) switch (::rainy::text::u8hashed_string::eval_hash(x))
#endif

#define rainy_match_hashed_string(x) ::rainy::text::hashed_string::eval_hash(x)
#define rainy_match_whashed_string(x) ::rainy::text::whashed_string::eval_hash(x)
#define rainy_match_u16hashed_string(x) ::rainy::text::u16hashed_string::eval_hash(x)
#define rainy_match_u32hashed_string(x) ::rainy::text::u32hashed_string::eval_hash(x)
#if RAINY_HAS_CXX20
#define rainy_match_u8hashed_string(x) ::rainy::text::u8hashed_string::eval_hash(x)
#endif

namespace rainy::utility {
    template <typename CharType>
    struct hash<text::basic_hashed_string<CharType>> {
        using argument_type = text::basic_hashed_string<CharType>;
        using result_type = std::size_t;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return val.hash_code();
        }

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return hash_this_val(val);
        }
    };
}

#endif