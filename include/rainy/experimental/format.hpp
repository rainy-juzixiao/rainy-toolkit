#ifndef RAINY_FORMAT_HPP
#define RAINY_FORMAT_HPP

#include <rainy/core/core_format.hpp>
#if RAINY_HAS_CXX20
#include <array>
#include <charconv>
#include <concepts>
#include <locale>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <rainy/functional/function_pointer.hpp>

namespace rainy::experimental::format_internals {
    template <typename CharType>
    struct decode_result {
        const CharType *next_ptr;
        bool is_unicode_scalarvalue; // Also is_usv below, see https://www.unicode.org/glossary/#unicode_scalarvalue
        // is_unicode_scalarvalue is also used for non-Unicode encodings, to indicate that the input can be converted to
        // Unicode.
    };

    // decode_utf decodes UTF-8 or UTF-16 encoded unsigned char or wchar_t strings respectively
    RAINY_NODISCARD constexpr decode_result<wchar_t> decode_utf(const wchar_t *first, const wchar_t *last, char32_t &val) noexcept {
        val = static_cast<char32_t>(*first);
        if (val < 0xD800) {
            return {first + 1, true};
        } else if (val <= 0xDBFF) {
            // 0xD800 <= val <= 0xDBFF: High surrogate
            if (first + 1 == last) {
                val = 0xFFFD;
                return {last, false};
            }

            if (first[1] < 0xDC00 || first[1] > 0xDFFF) {
                // unpaired high surrogate
                val = 0xFFFD;
                return {first + 1, false};
            }

            val = (val - 0xD800) << 10;
            val += first[1] - 0xDC00;
            val += 0x10000;
            return {first + 2, true};
        } else if (val <= 0xDFFF) {
            // unpaired low surrogate
            val = 0xFFFD;
            return {first + 1, false};
        }

        return {first + 1, true};
    }

    RAINY_NODISCARD constexpr decode_result<char> decode_utf(const char *first, const char *last, char32_t &val) noexcept {
        // Decode a UTF-8 encoded codepoint starting at first and not exceeding last, returning
        // one past the end of the character decoded. Any invalid codepoints will result in
        // val == U+FFFD and decode_utf will return one past the
        // maximal subpart of the ill-formed subsequence. So, most invalid UTF-8 will result in
        // one U+FFFD for each byte of invalid data. Truncated but otherwise valid UTF-8 may
        // result in one U+FFFD for more than one input byte.
        val = static_cast<char32_t>(static_cast<unsigned char>(*first));

        // All UTF-8 text is at least one byte.
        // The zero extended values of the "prefix" bytes for
        // a multi-byte sequence are the lowest numeric value (in two's complement)
        // that any leading byte could have for a code unit of that size, so
        // we just sum the comparisons to get the number of trailing bytes.
        int num_bytes;
        if (val <= 0x7F) {
            return {first + 1, true};
        } else if (val >= 0xC2 && val <= 0xDF) {
            num_bytes = 2;
        } else if (val >= 0xE0 && val <= 0xEF) {
            num_bytes = 3;
        } else if (val >= 0xF0 && val <= 0xF4) {
            num_bytes = 4;
        } else {
            // definitely not valid
            val = 0xFFFD;
            return {first + 1, false};
        }

        if (first + 1 == last) {
            // We got a multibyte sequence and the next byte is off the end, we need
            // to check just the next byte here since we need to look for overlong sequences.
            // We want to return one past the end of a truncated sequence if everything is
            // otherwise valid, so we can't check if first + num_bytes is off the end.
            val = 0xFFFD;
            return {last, false};
        }

        switch (val) {
            case 0xE0:
                // we know first[1] is in range because we just checked above,
                // and a leader of 0xE0 implies num_bytes == 3
                if (static_cast<unsigned char>(first[1]) < 0xA0) {
                    // note, we just increment forward one-byte,
                    // even though num_bytes would imply the next
                    // codepoint starts at first + 2, this is because
                    // we don't consume trailing bytes of ill-formed subsequences
                    val = 0xFFFD;
                    return {first + 1, false};
                }
                break;
            case 0xED:
                if (static_cast<unsigned char>(first[1]) > 0x9F) {
                    val = 0xFFFD;
                    return {first + 1, false};
                }
                break;
            case 0xF0:
                if (static_cast<unsigned char>(first[1]) < 0x90) {
                    val = 0xFFFD;
                    return {first + 1, false};
                }
                break;
            case 0xF4:
                if (static_cast<unsigned char>(first[1]) > 0x8F) {
                    val = 0xFFFD;
                    return {first + 1, false};
                }
                break;
            default:
                break;
        }

        // mask out the "value bits" in the leading byte,
        // for one-byte codepoints there is no leader,
        // two-byte codepoints have the same number of value
        // bits as trailing bytes (including the leading zero)
        switch (num_bytes) {
            case 2:
                val &= 0b1'1111u;
                break;
            case 3:
                val &= 0b1111u;
                break;
            case 4:
                val &= 0b111u;
                break;
            default:
                break;
        }

        for (int idx = 1; idx < num_bytes; ++idx) {
            if (first + idx >= last || static_cast<unsigned char>(first[idx]) < 0x80 || static_cast<unsigned char>(first[idx]) > 0xBF) {
                // truncated sequence
                val = 0xFFFD;
                return {first + idx, false};
            }
            // we know we're always in range due to the above check.
            val = (val << 6) | (static_cast<unsigned char>(first[idx]) & 0b11'1111u);
        }
        return {first + num_bytes, true};
    }

    RAINY_NODISCARD constexpr decode_result<char32_t> decode_utf(const char32_t *first, const char32_t *last, char32_t &val) noexcept {
        (void) last;
        val = *first;
        const bool is_usv = val < 0xD800 || (val > 0xDFFF && val <= 0x10FFFF);
        return {first + 1, is_usv};
    }

    template <typename CharType>
    class unicode_codepoint_iterator {
    public:
        using value_type = char32_t;
        using difference_type = ptrdiff_t;
        using const_pointer = const CharType *;

        constexpr unicode_codepoint_iterator(const CharType *firstval, const CharType *lastval) noexcept :
            first_(firstval), last_(lastval) {
            if (first_ != last_) {
                next_ = decode_utf(first_, last_, value_).next_ptr;
            }
        }

        constexpr unicode_codepoint_iterator() = default;

        constexpr unicode_codepoint_iterator &operator++() noexcept {
            first_ = next_;
            if (first_ != last_) {
                next_ = decode_utf(first_, last_, value_).next_ptr;
            }

            return *this;
        }

        constexpr unicode_codepoint_iterator operator++(int) noexcept {
            auto old = *this;
            ++*this;
            return old;
        }

        RAINY_NODISCARD constexpr value_type operator*() const noexcept {
            return value_;
        }

        RAINY_NODISCARD constexpr const_pointer position() const noexcept {
            return first_;
        }

        RAINY_NODISCARD constexpr bool operator==(std::default_sentinel_t) const noexcept {
            return first_ == last_;
        }

        RAINY_NODISCARD constexpr bool operator==(const unicode_codepoint_iterator &other) const noexcept {
            core::internals::stl_internal_check(last_ == other.last_);
            return first_ == other.first_ && last_ == other.last_;
        }

    private:
        const_pointer first_ = nullptr;
        const_pointer last_ = nullptr;
        const_pointer next_ = nullptr;
        char32_t value_ = 0;
    };

    // Implements a DFA matching the regex on the left side of rule GB11. The DFA is:
    //
    // +---+   ExtPic      +---+    ZWJ        +---+
    // | 1 +---------------> 2 +---------------> 3 |
    // +---+               ++-^+               +---+
    //                      | |
    //                      +-+
    //                      Extend
    //
    // Note state 3 is never explicitly transitioned to, since it's the "accept" state, we just
    // transition back to state 1 and return true.
    class gb11_left_hand_regex {
    public:
        RAINY_NODISCARD constexpr bool operator==(const gb11_left_hand_regex &) const noexcept = default;

        RAINY_NODISCARD constexpr bool match(const grapheme_break_propertyvalues left_gbp,
                                                const extended_pictographic_propertyvalues left_ext_pic) noexcept {
            switch (state_) {
                case start:
                    if (left_ext_pic == extended_pictographic_propertyvalues::extended_pictographicvalue) {
                        state_ = ext_pic;
                    }
                    return false;
                case ext_pic:
                    if (left_gbp == grapheme_break_propertyvalues::zwjvalue) {
                        state_ = start;
                        return true;
                    }
                    if (left_gbp != grapheme_break_propertyvalues::extendvalue) {
                        state_ = start;
                        return false;
                    }
                    return false;
                default:
                    return false;
            }
        }

    private:
        enum state_t : bool {
            start,
            ext_pic
        };

        state_t state_{start};
    };

    template <typename CharType>
    class grapheme_break_property_iterator {
    public:
        using wrapped_iter_type = unicode_codepoint_iterator<CharType>;

        RAINY_NODISCARD constexpr bool operator==(std::default_sentinel_t) const noexcept {
            return wrapped_iter_ == std::default_sentinel;
        }

        RAINY_NODISCARD constexpr bool operator==(const grapheme_break_property_iterator &) const noexcept = default;

        using difference_type = ptrdiff_t;
        using value_type = typename wrapped_iter_type::value_type;

        constexpr grapheme_break_property_iterator(const CharType *first, const CharType *last) : wrapped_iter_(first, last), num_ris_(0) {
            update_current_properties();
        }

        constexpr grapheme_break_property_iterator() = default;

        constexpr grapheme_break_property_iterator &operator++() noexcept {
            do {
                ++wrapped_iter_;
                auto next_gbp = grapheme_break_property_data.get_property_for_codepoint(*wrapped_iter_);
                auto next_ext_pic = extended_pictographic_property_data.get_property_for_codepoint(*wrapped_iter_);

                if (current_gbp_ == grapheme_break_propertyvalues::regional_indicatorvalue) {
                    ++num_ris_;
                } else {
                    num_ris_ = 0;
                }

                if (should_break(next_gbp, next_ext_pic)) {
                    update_current_properties();
                    return *this;
                }

                current_gbp_ = next_gbp;
                current_ext_pic_ = next_ext_pic;
            } while (true);
        }

        constexpr grapheme_break_property_iterator operator++(int) noexcept {
            auto old = *this;
            ++*this;
            return old;
        }

        RAINY_NODISCARD constexpr const CharType *position() const noexcept {
            return wrapped_iter_.position();
        }

        RAINY_NODISCARD constexpr value_type operator*() const noexcept {
            return *wrapped_iter_;
        }

    private:
        void update_current_properties() {
            if (wrapped_iter_ == std::default_sentinel) {
                current_gbp_ = grapheme_break_propertyvalues::novalue;
                current_ext_pic_ = extended_pictographic_propertyvalues::novalue;
            } else {
                current_gbp_ = grapheme_break_property_data.get_property_for_codepoint(*wrapped_iter_);
                current_ext_pic_ = extended_pictographic_property_data.get_property_for_codepoint(*wrapped_iter_);
            }
        }

        RAINY_NODISCARD bool should_break(const grapheme_break_propertyvalues next_gbp,const extended_pictographic_propertyvalues next_ext_pic) {
            // GB1 sot ÷ Any
            // 这个规则不需要在这里实现，因为它只适用于字符串的开始

            // GB2 Any ÷ eot
            if (wrapped_iter_ == std::default_sentinel)
                return true;

            // GB3 CR × LF
            if (current_gbp_ == grapheme_break_propertyvalues::crvalue && next_gbp == grapheme_break_propertyvalues::lfvalue)
                return false;

            // GB4 (Control | CR | LF) ÷
            if (current_gbp_ == grapheme_break_propertyvalues::controlvalue || current_gbp_ == grapheme_break_propertyvalues::crvalue ||
                current_gbp_ == grapheme_break_propertyvalues::lfvalue)
                return true;

            // GB5 ÷ (Control | CR | LF)
            if (next_gbp == grapheme_break_propertyvalues::controlvalue || next_gbp == grapheme_break_propertyvalues::crvalue ||
                next_gbp == grapheme_break_propertyvalues::lfvalue)
                return true;

            // GB6 L × (L | V | LV | LVT)
            if (current_gbp_ == grapheme_break_propertyvalues::lvalue &&
                (next_gbp == grapheme_break_propertyvalues::lvalue || next_gbp == grapheme_break_propertyvalues::vvalue ||
                 next_gbp == grapheme_break_propertyvalues::lvvalue || next_gbp == grapheme_break_propertyvalues::lvtvalue))
                return false;

            // GB7 (LV | V) × (V | T)
            if ((current_gbp_ == grapheme_break_propertyvalues::lvvalue || current_gbp_ == grapheme_break_propertyvalues::vvalue) &&
                (next_gbp == grapheme_break_propertyvalues::vvalue || next_gbp == grapheme_break_propertyvalues::tvalue))
                return false;

            // GB8 (LVT | T) × T
            if ((current_gbp_ == grapheme_break_propertyvalues::lvtvalue || current_gbp_ == grapheme_break_propertyvalues::tvalue) &&
                next_gbp == grapheme_break_propertyvalues::tvalue)
                return false;

            // GB9 × (Extend | ZWJ)
            if (next_gbp == grapheme_break_propertyvalues::extendvalue || next_gbp == grapheme_break_propertyvalues::zwjvalue)
                return false;

            // GB9a × SpacingMark
            if (next_gbp == grapheme_break_propertyvalues::spacing_markvalue)
                return false;

            // GB9b Prepend ×
            if (current_gbp_ == grapheme_break_propertyvalues::prependvalue)
                return false;

            // GB11 ExtPict Extend* ZWJ × ExtPict
            if (gb11_rx_.match(current_gbp_, current_ext_pic_) &&
                next_ext_pic == extended_pictographic_propertyvalues::extended_pictographicvalue)
                return false;

            // GB12 and GB13 ^(RI RI)* RI × RI
            if (current_gbp_ == grapheme_break_propertyvalues::regional_indicatorvalue &&
                next_gbp == grapheme_break_propertyvalues::regional_indicatorvalue && num_ris_ % 2 != 0)
                return false;

            // GB999 Any ÷ Any
            return true;
        }

        wrapped_iter_type wrapped_iter_;
        gb11_left_hand_regex gb11_rx_;
        grapheme_break_propertyvalues current_gbp_{};
        extended_pictographic_propertyvalues current_ext_pic_{};
        size_t num_ris_{};
    };
}

namespace rainy::experimental {
    template <typename CharType>
    class basic_format_parse_context {
    public:
        using char_type_alias = CharType;
        using const_iterator = typename std::basic_string_view<CharType>::const_iterator;
        using iterator = const_iterator;

        constexpr explicit basic_format_parse_context(const std::basic_string_view<CharType> fmt, const size_t num_args = 0) noexcept :
            format_string(fmt), num_args(num_args) {
        }

        basic_format_parse_context(const basic_format_parse_context &) = delete;
        basic_format_parse_context &operator=(const basic_format_parse_context &) = delete;

        RAINY_NODISCARD constexpr const_iterator begin() const noexcept {
            return format_string.begin();
        }

        RAINY_NODISCARD constexpr const_iterator end() const noexcept {
            return format_string.end();
        }

        RAINY_NODISCARD constexpr const CharType *unchecked_begin() const noexcept {
            return format_string._Unchecked_begin();
        }

        RAINY_NODISCARD constexpr const CharType *unchecked_end() const noexcept {
            rainy_let end_offset = format_string.size();
            return format_string.data() + end_offset;
        }

        constexpr void advance_to(const const_iterator it) {
            utility::internals::adl_verify_range(it, format_string.end());
            utility::internals::adl_verify_range(format_string.begin(), it);
            const auto diff = static_cast<size_t>(it._Unwrapped() - format_string._Unchecked_begin());
            format_string.remove_prefix(diff);
        }

        // While the standard presents an exposition-only enum value for
        // the indexing mode (manual, automatic, or unknown) we use next_arg_id to indicate it.
        // next_arg_id > 0 means automatic
        // next_arg_id == 0 means unknown
        // next_arg_id < 0 means manual
        RAINY_NODISCARD constexpr size_t next_arg_id() {
            if (next_arg_id_ < 0) {
                foundation::system::exceptions::runtime::throw_format_error("Cannot switch from manual to automatic indexing");
            }
            if (std::is_constant_evaluated()) {
                if (static_cast<size_t>(next_arg_id_) >= num_args) {
                    format_internals::found_error_cause_by_arg_id_is_out_of_range();
                }
            }
            return static_cast<size_t>(next_arg_id_++);
        }

        constexpr void check_arg_id(const size_t id) {
            if (std::is_constant_evaluated()) {
                if (id >= num_args) {
                    format_internals::found_error_cause_by_arg_id_is_out_of_range();
                }
            }

            if (next_arg_id_ > 0) {
                foundation::system::exceptions::runtime::throw_format_error("Cannot switch from automatic to manual indexing");
            }
            next_arg_id_ = -1;
        }

        constexpr void check_dynamic_spec_integral(const size_t idx) noexcept {
            if (std::is_constant_evaluated()) {
                // This downcast might seem UB-prone, but since it only happens at compile-time,
                // the compiler will produce an error if it is invalid.
                auto &ctx = static_cast<compile_time_parse_context<CharType> &>(*this);
                core::internals::stl_internal_check(ctx.arg_type[idx] != format_internals::basic_format_arg_type::none);
                if (ctx.arg_type[idx] > format_internals::basic_format_arg_type::ulong_long_type) {
                    format_internals::invalid_arg_type_for_dynamic_width_or_precision();
                }
            }
        }

    private:
        std::basic_string_view<CharType> format_string;
        size_t num_args;
        // The standard says this is size_t, however we use ptrdiff_t to save some space
        // by not having to store the indexing mode. Above is a more detailed explanation
        // of how this works.
        ptrdiff_t next_arg_id_ = 0;
    };

    template <typename CharType>
    class compile_time_parse_context : public basic_format_parse_context<CharType> {
    public:
        friend basic_format_parse_context<CharType>;

        constexpr compile_time_parse_context(const std::basic_string_view<CharType> fmt, const size_t num_args,
                                             const format_internals::basic_format_arg_type *const arg_tupe) noexcept :
            basic_format_parse_context<CharType>(fmt, num_args), arg_type(arg_tupe) {
        }

    private:
        const format_internals::basic_format_arg_type *const arg_type;
    };
}

namespace rainy::experimental {
    template <typename Context>
    class basic_format_arg {
    public:
        using context_type = Context;
        using char_type = typename context_type::char_type;

        class handle {
        public:
            template <typename Ty>
            explicit handle(Ty &val) noexcept :
                ptr(utility::addressof(val)),
                format_fptr([](basic_format_parse_context<char_type> &parse_ctx, Context &format_ctx, const void *ptr) {
                    using Td = type_traits::cv_modify::remove_const_t<Ty>;
                    using Tq = type_traits::other_trans::conditional_t<type_traits::concepts::formattable_with<const Ty, context_type>,
                                                                       const Ty, Ty>;
                    static_assert(type_traits::concepts::formattable_with<Tq, Context>);
                    typename context_type::template formatter_type<Td> formatter;
                    parse_ctx.advance_to(formatter.parse(parse_ctx));
                    format_ctx.advance_to(formatter.format(*const_cast<Tq *>(static_cast<const Td *>(ptr)), format_ctx));
                }) {
            }

            void format(basic_format_parse_context<char_type> &parse_ctx, context_type &format_ctx) const {
                format_fptr(parse_ctx, format_ctx, ptr);
            }

            template <class _Ty>
            RAINY_NODISCARD static handle make_from(_Ty &val) noexcept {
                return handle{val};
            }

        private:
            const void *ptr;
            foundation::functional::function_pointer<void(basic_format_parse_context<char_type> & /* parse_ctx */,
                                                          _Context &_Format_ctx, const void *)>
                format_fptr; 
        };

        basic_format_arg() noexcept : active_state(_Basic_format_arg_type::_None), () {
        }

        explicit operator bool() const noexcept {
            return active_state != _Basic_format_arg_type::_None;
        }

        // Function template _Make_from mirrors the exposition-only single-argument constructor template of
        // basic_format_arg (N4950 [format.arg]).
        template <_Formattable_with<_Context> _Ty>
        _NODISCARD static basic_format_arg _Make_from(_Ty &val) noexcept {
            using _Erased_type = _Format_arg_traits<_Context>::template _Storage_type<_Ty>;
            if constexpr (is_same_v<remove_const_t<_Ty>, char> && is_same_v<_CharType, wchar_t>) {
                return basic_format_arg(static_cast<_Erased_type>(static_cast<unsigned char>(val)));
            }
#if !_HAS_CXX23
            else if constexpr (is_same_v<_Erased_type, basic_string_view<_CharType>>) {
                return basic_format_arg(_Erased_type{val.data(), val.size()});
            }
#endif // !_HAS_CXX23
            else {
                return basic_format_arg(static_cast<_Erased_type>(val));
            }
        }

        template <class _Visitor>
        decltype(auto) _Visit(_Visitor &&_Vis) {
            switch (active_state) {
                case _Basic_format_arg_type::_None:
                    return utility::forward<_Visitor>(_Vis)(_No_state);
                case _Basic_format_arg_type::_Int_type:
                    return utility::forward<_Visitor>(_Vis)(_Int_state);
                case _Basic_format_arg_type::_UInt_type:
                    return utility::forward<_Visitor>(_Vis)(_UInt_state);
                case _Basic_format_arg_type::_Long_long_type:
                    return utility::forward<_Visitor>(_Vis)(_Long_long_state);
                case _Basic_format_arg_type::_ULong_long_type:
                    return utility::forward<_Visitor>(_Vis)(_ULong_long_state);
                case _Basic_format_arg_type::_Bool_type:
                    return utility::forward<_Visitor>(_Vis)(_Bool_state);
                case _Basic_format_arg_type::_Char_type:
                    return utility::forward<_Visitor>(_Vis)(_Char_state);
                case _Basic_format_arg_type::_Float_type:
                    return utility::forward<_Visitor>(_Vis)(_Float_state);
                case _Basic_format_arg_type::_Double_type:
                    return utility::forward<_Visitor>(_Vis)(_Double_state);
                case _Basic_format_arg_type::_Long_double_type:
                    return utility::forward<_Visitor>(_Vis)(_Long_double_state);
                case _Basic_format_arg_type::_Pointer_type:
                    return utility::forward<_Visitor>(_Vis)(_Pointer_state);
                case _Basic_format_arg_type::_CString_type:
                    return utility::forward<_Visitor>(_Vis)(_CString_state);
                case _Basic_format_arg_type::_String_type:
                    return utility::forward<_Visitor>(_Vis)(_String_state);
                case _Basic_format_arg_type::_Custom_type:
                    return utility::forward<_Visitor>(_Vis)(_Custom_state);
                default:
                    utility::ensures(false, "basic_format_arg is in impossible state");
                    int dummy{};
                    return utility::forward<_Visitor>(_Vis)(dummy);
            }
        }

    private:
        friend basic_format_args<_Context>;
        friend _Format_handler<_CharType>;
        friend _Format_arg_traits<_Context>;

        explicit basic_format_arg(const int val) noexcept : active_state(_Basic_format_arg_type::_Int_type), _Int_state(val) {
        }
        explicit basic_format_arg(const unsigned int val) noexcept :
            active_state(_Basic_format_arg_type::_UInt_type), _UInt_state(val) {
        }
        explicit basic_format_arg(const long long val) noexcept :
            active_state(_Basic_format_arg_type::_Long_long_type), _Long_long_state(val) {
        }
        explicit basic_format_arg(const unsigned long long val) noexcept :
            active_state(_Basic_format_arg_type::_ULong_long_type), _ULong_long_state(val) {
        }
        explicit basic_format_arg(const bool val) noexcept : active_state(_Basic_format_arg_type::_Bool_type), _Bool_state(val) {
        }
        explicit basic_format_arg(const char_type val) noexcept :
            active_state(format_internals::basic_format_arg_type::char_type), char_state(val) {
        }
        explicit basic_format_arg(const float val) noexcept : active_state(_Basic_format_arg_type::_Float_type), _Float_state(val) {
        }
        explicit basic_format_arg(const double val) noexcept :
            active_state(format_internals::basic_format_arg_type::double_type), double_state(val) {
        }
        explicit basic_format_arg(const long double val) noexcept :
            active_state(format_internals::basic_format_arg_type::long_double_type), long_double_state(val) {
        }
        explicit basic_format_arg(const void *val) noexcept :
            active_state(format_internals::basic_format_arg_type::pointer_type), pointer_state(val) {
        }
        explicit basic_format_arg(const _CharType *val) noexcept :
            active_state(format_internals::basic_format_arg_type::cstring_type), cstyle_string_state(val) {
        }
        explicit basic_format_arg(const basic_string_view<_CharType> val) noexcept :
            active_state(format_internals::basic_format_arg_type::string_type), string_state(val) {
        }
        explicit basic_format_arg(const handle val) noexcept :
            active_state(format_internals::basic_format_arg_type::custom_type), custom_state(val) {
        }


        union {
            std::monostate no_state = std::monostate{};
            int int_state;
            unsigned int uint_state;
            long long long_long_state;
            unsigned long long uLong_long_state;
            bool bool_state;
            char_type char_state;
            float float_state;
            double double_state;
            long double long_double_state;
            const void *pointer_state;
            const char_type *cstyle_string_state;
            std::basic_string_view<char_type> string_state;
            handle custom_state;
        };

        format_internals::basic_format_arg_type active_state = format_internals::basic_format_arg_type::none;
    };
}

#endif

#endif
