#ifndef RAINY_CORE_LIMITS_HPP
#define RAINY_CORE_LIMITS_HPP
#include <rainy/core/platform.hpp>
#include <climits>
#include <cfloat>

namespace rainy::utility {
    enum float_denorm_style {
        denorm_indeterminate = -1,
        denorm_absent = 0,
        denorm_present = 1
    };

    enum float_round_style {
        round_indeterminate = -1,
        round_toward_zero = 0,
        round_to_nearest = 1,
        round_toward_infinity = 2,
        round_toward_neg_infinity = 3
    };
}

namespace rainy::utility::implements {
    struct number_base {
        static constexpr float_denorm_style has_denorm = denorm_absent;
        static constexpr bool has_denorm_loss = false;

        static constexpr bool has_infinity = false;
        static constexpr bool has_quiet_NaN = false;
        static constexpr bool has_signaling_NaN = false;
        static constexpr bool is_bounded = false;
        static constexpr bool is_exact = false;
        static constexpr bool is_iec559 = false;
        static constexpr bool is_integer = false;
        static constexpr bool is_modulo = false;
        static constexpr bool is_signed = false;
        static constexpr bool is_specialized = false;
        static constexpr bool tinyness_before = false;
        static constexpr bool traps = false;
        static constexpr float_round_style round_style = round_toward_zero;
        static constexpr int digits = 0;
        static constexpr int digits10 = 0;
        static constexpr int max_digits10 = 0;
        static constexpr int max_exponent = 0;
        static constexpr int max_exponent10 = 0;
        static constexpr int min_exponent = 0;
        static constexpr int min_exponent10 = 0;
        static constexpr int radix = 0;
    };

    struct number_int_base : number_base {
        static constexpr bool is_bounded = true;
        static constexpr bool is_exact = true;
        static constexpr bool is_integer = true;
        static constexpr bool is_specialized = true;
        static constexpr int radix = 2;
    };

    struct number_float_base : number_base {
        static constexpr float_denorm_style has_denorm = denorm_present;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = true;
        static constexpr bool has_signaling_NaN = true;
        static constexpr bool is_bounded = true;
        static constexpr bool is_iec559 = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_specialized = true;
        static constexpr float_round_style round_style = round_to_nearest;
        static constexpr int radix = FLT_RADIX;
    };
}

namespace rainy::utility {
    template <typename Ty>
    class numeric_limits : public implements::number_base {
    public:
        RAINY_NODISCARD static constexpr Ty(min)() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty(max)() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty lowest() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty epsilon() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty round_error() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty denorm_min() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty infinity() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty quiet_NaN() noexcept {
            return Ty();
        }

        RAINY_NODISCARD static constexpr Ty signaling_NaN() noexcept {
            return Ty();
        }
    };

    template <typename Ty>
    class numeric_limits<const Ty> : public numeric_limits<Ty> {};

    template <typename Ty>
    class numeric_limits<volatile Ty> : public numeric_limits<Ty> {};

    template <typename Ty>
    class numeric_limits<const volatile Ty> : public numeric_limits<Ty> {};

    template <>
    class numeric_limits<bool> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr bool(min)() noexcept {
            return false;
        }

        RAINY_NODISCARD static constexpr bool(max)() noexcept {
            return true;
        }

        RAINY_NODISCARD static constexpr bool lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr bool epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr bool round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr bool denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr bool infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr bool quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr bool signaling_NaN() noexcept {
            return 0;
        }

        static constexpr int digits = 1;
    };


    template <>
    class numeric_limits<char> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr char(min)() noexcept {
            return CHAR_MIN;
        }

        RAINY_NODISCARD static constexpr char(max)() noexcept {
            return CHAR_MAX;
        }

        RAINY_NODISCARD static constexpr char lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr char epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_signed = CHAR_MIN != 0;
        static constexpr bool is_modulo = CHAR_MIN == 0;
        static constexpr int digits = 8 - (CHAR_MIN != 0);
        static constexpr int digits10 = 2;
    };

    template <>
    class numeric_limits<signed char> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr signed char(min)() noexcept {
            return SCHAR_MIN;
        }

        RAINY_NODISCARD static constexpr signed char(max)() noexcept {
            return SCHAR_MAX;
        }

        RAINY_NODISCARD static constexpr signed char lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr signed char epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr signed char round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr signed char denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr signed char infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr signed char quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr signed char signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_signed = true;
        static constexpr int digits = 7;
        static constexpr int digits10 = 2;
    };

    template <>
    class numeric_limits<unsigned char> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr unsigned char(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned char(max)() noexcept {
            return UCHAR_MAX;
        }

        RAINY_NODISCARD static constexpr unsigned char lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr unsigned char epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned char round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned char denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned char infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned char quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned char signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 8;
        static constexpr int digits10 = 2;
    };

    template <>
    class numeric_limits<char16_t> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr char16_t(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char16_t(max)() noexcept {
            return USHRT_MAX;
        }

        RAINY_NODISCARD static constexpr char16_t lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr char16_t epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char16_t round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char16_t denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char16_t infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char16_t quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char16_t signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 16;
        static constexpr int digits10 = 4;
    };

    template <>
    class numeric_limits<char32_t> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr char32_t(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char32_t(max)() noexcept {
            return UINT_MAX;
        }

        RAINY_NODISCARD static constexpr char32_t lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr char32_t epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char32_t round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char32_t denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char32_t infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char32_t quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char32_t signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 32;
        static constexpr int digits10 = 9;
    };

    template <>
    class numeric_limits<wchar_t> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr wchar_t(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr wchar_t(max)() noexcept {
            return 0xffff;
        }

        RAINY_NODISCARD static constexpr wchar_t lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr wchar_t epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr wchar_t round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr wchar_t denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr wchar_t infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr wchar_t quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr wchar_t signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 16;
        static constexpr int digits10 = 4;
    };

    template <>
    class numeric_limits<short> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr short(min)() noexcept {
            return SHRT_MIN;
        }

        RAINY_NODISCARD static constexpr short(max)() noexcept {
            return SHRT_MAX;
        }

        RAINY_NODISCARD static constexpr short lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr short epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr short round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr short denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr short infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr short quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr short signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_signed = true;
        static constexpr int digits = 15;
        static constexpr int digits10 = 4;
    };

    template <>
    class numeric_limits<int> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr int(min)() noexcept {
            return INT_MIN;
        }

        RAINY_NODISCARD static constexpr int(max)() noexcept {
            return INT_MAX;
        }

        RAINY_NODISCARD static constexpr int lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr int epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr int round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr int denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr int infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr int quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr int signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_signed = true;
        static constexpr int digits = 31;
        static constexpr int digits10 = 9;
    };

    template <>
    class numeric_limits<long> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr long(min)() noexcept {
            return LONG_MIN;
        }

        RAINY_NODISCARD static constexpr long(max)() noexcept {
            return LONG_MAX;
        }

        RAINY_NODISCARD static constexpr long lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr long epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_signed = true;
        static constexpr int digits = 31;
        static constexpr int digits10 = 9;
    };

    template <>
    class numeric_limits<long long> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr long long(min)() noexcept {
            return LLONG_MIN;
        }

        RAINY_NODISCARD static constexpr long long(max)() noexcept {
            return LLONG_MAX;
        }

        RAINY_NODISCARD static constexpr long long lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr long long epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long long round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long long denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long long infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long long quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr long long signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_signed = true;
        static constexpr int digits = 63;
        static constexpr int digits10 = 18;
    };

    template <>
    class numeric_limits<unsigned short> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr unsigned short(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned short(max)() noexcept {
            return USHRT_MAX;
        }

        RAINY_NODISCARD static constexpr unsigned short lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr unsigned short epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned short round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned short denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned short infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned short quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned short signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 16;
        static constexpr int digits10 = 4;
    };

    template <>
    class numeric_limits<unsigned int> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr unsigned int(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned int(max)() noexcept {
            return UINT_MAX;
        }

        RAINY_NODISCARD static constexpr unsigned int lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr unsigned int epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned int round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned int denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned int infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned int quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned int signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 32;
        static constexpr int digits10 = 9;
    };

    template <>
    class numeric_limits<unsigned long> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr unsigned long(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long(max)() noexcept {
            return ULONG_MAX;
        }

        RAINY_NODISCARD static constexpr unsigned long lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr unsigned long epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 32;
        static constexpr int digits10 = 9;
    };

    template <>
    class numeric_limits<unsigned long long> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr unsigned long long(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long long(max)() noexcept {
            return ULLONG_MAX;
        }

        RAINY_NODISCARD static constexpr unsigned long long lowest() noexcept {
            return (min) ();
        }

        RAINY_NODISCARD static constexpr unsigned long long epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long long round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long long denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long long infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long long quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr unsigned long long signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 64;
        static constexpr int digits10 = 19;
    };

    template <>
    class numeric_limits<float> : public implements::number_float_base {
    public:
        RAINY_NODISCARD static constexpr float(min)() noexcept {
            return FLT_MIN;
        }

        RAINY_NODISCARD static constexpr float(max)() noexcept {
            return FLT_MAX;
        }

        RAINY_NODISCARD static constexpr float lowest() noexcept {
            return -(max) ();
        }

        RAINY_NODISCARD static constexpr float epsilon() noexcept {
            return FLT_EPSILON;
        }

        RAINY_NODISCARD static constexpr float round_error() noexcept {
            return 0.5F;
        }

        RAINY_NODISCARD static constexpr float denorm_min() noexcept {
            return FLT_TRUE_MIN;
        }

        RAINY_NODISCARD static constexpr float infinity() noexcept {
            return core::builtin::huge_valf();
        }

        RAINY_NODISCARD static constexpr float quiet_NaN() noexcept {
            return __builtin_nanf("1");
        }

        RAINY_NODISCARD static constexpr float signaling_NaN() noexcept {
            return __builtin_nansf("1");
        }

        static constexpr int digits = FLT_MANT_DIG;
        static constexpr int digits10 = FLT_DIG;
        static constexpr int max_digits10 = 9;
        static constexpr int max_exponent = FLT_MAX_EXP;
        static constexpr int max_exponent10 = FLT_MAX_10_EXP;
        static constexpr int min_exponent = FLT_MIN_EXP;
        static constexpr int min_exponent10 = FLT_MIN_10_EXP;
    };

    template <>
    class numeric_limits<double> : public implements::number_float_base {
    public:
        RAINY_NODISCARD static constexpr double(min)() noexcept {
            return DBL_MIN;
        }

        RAINY_NODISCARD static constexpr double(max)() noexcept {
            return DBL_MAX;
        }

        RAINY_NODISCARD static constexpr double lowest() noexcept {
            return -(max) ();
        }

        RAINY_NODISCARD static constexpr double epsilon() noexcept {
            return DBL_EPSILON;
        }

        RAINY_NODISCARD static constexpr double round_error() noexcept {
            return 0.5;
        }

        RAINY_NODISCARD static constexpr double denorm_min() noexcept {
            return DBL_TRUE_MIN;
        }

        RAINY_NODISCARD static constexpr double infinity() noexcept {
            return core::builtin::huge_val();
        }

        RAINY_NODISCARD static constexpr double quiet_NaN() noexcept {
            return __builtin_nan("0");
        }

        RAINY_NODISCARD static constexpr double signaling_NaN() noexcept {
            return __builtin_nans("1");
        }

        static constexpr int digits = DBL_MANT_DIG;
        static constexpr int digits10 = DBL_DIG;
        static constexpr int max_digits10 = 17;
        static constexpr int max_exponent = DBL_MAX_EXP;
        static constexpr int max_exponent10 = DBL_MAX_10_EXP;
        static constexpr int min_exponent = DBL_MIN_EXP;
        static constexpr int min_exponent10 = DBL_MIN_10_EXP;
    };

    template <>
    class numeric_limits<long double> : public implements::number_float_base {
    public:
        RAINY_NODISCARD static constexpr long double(min)() noexcept {
            return LDBL_MIN;
        }

        RAINY_NODISCARD static constexpr long double(max)() noexcept {
            return LDBL_MAX;
        }

        RAINY_NODISCARD static constexpr long double lowest() noexcept {
            return -(max) ();
        }

        RAINY_NODISCARD static constexpr long double epsilon() noexcept {
            return LDBL_EPSILON;
        }

        RAINY_NODISCARD static constexpr long double round_error() noexcept {
            return 0.5L;
        }

        RAINY_NODISCARD static constexpr long double denorm_min() noexcept {
            return LDBL_TRUE_MIN;
        }

        RAINY_NODISCARD static constexpr long double infinity() noexcept {
            return (core::builtin::huge_val)();
        }

        RAINY_NODISCARD static constexpr long double quiet_NaN() noexcept {
            return __builtin_nan("0");
        }

        RAINY_NODISCARD static constexpr long double signaling_NaN() noexcept {
            return __builtin_nans("1");
        }

        static constexpr int digits = LDBL_MANT_DIG;
        static constexpr int digits10 = LDBL_DIG;
        static constexpr int max_digits10 = 2 + LDBL_MANT_DIG * 301L / 1000;
        static constexpr int max_exponent = LDBL_MAX_EXP;
        static constexpr int max_exponent10 = LDBL_MAX_10_EXP;
        static constexpr int min_exponent = LDBL_MIN_EXP;
        static constexpr int min_exponent10 = LDBL_MIN_10_EXP;
    };

#ifdef __cpp_char8_t
    template <>
    class numeric_limits<char8_t> : public implements::number_int_base {
    public:
        RAINY_NODISCARD static constexpr char8_t(min)() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t(max)() noexcept {
            return UCHAR_MAX;
        }

        RAINY_NODISCARD static constexpr char8_t lowest() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t epsilon() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t round_error() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t denorm_min() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t infinity() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t quiet_NaN() noexcept {
            return 0;
        }

        RAINY_NODISCARD static constexpr char8_t signaling_NaN() noexcept {
            return 0;
        }

        static constexpr bool is_modulo = true;
        static constexpr int digits = 8;
        static constexpr int digits10 = 2;
    };
#endif

    template <typename Ty>
    constexpr bool is_in_valid_range(Ty value) {
        return value >= numeric_limits<Ty>::lowest() && value <= (numeric_limits<Ty>::max)();
    }
}

#endif