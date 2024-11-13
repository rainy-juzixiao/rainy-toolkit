#ifndef RAINY_REF_WRAP_HPP
#define RAINY_REF_WRAP_HPP

namespace rainy::utility {
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(type_traits::internals::_is_object_v<Ty> || type_traits::internals::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        using type = Ty;

        template <typename Uty>
        RAINY_CONSTEXPR20 reference_wrapper(Uty &&val) noexcept {
            Ty &ref = static_cast<Uty &&>(val);
            reference_data = rainy::utility::addressof(ref);
        }

        reference_wrapper(const reference_wrapper &) = delete;
        reference_wrapper(reference_wrapper &&) = delete;

        RAINY_CONSTEXPR20 operator Ty &() const noexcept {
            return *reference_data;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get() const noexcept {
            return *reference_data;
        }

        template <typename Elem, typename Uty>
        friend std::basic_ostream<Elem> &operator<<(std::basic_ostream<Elem> &ostream, const reference_wrapper<Uty> &ref_wrap) {
            ostream << ref_wrap.get();
            return ostream;
        }

    private:
        Ty *reference_data{};
    };
}

#if RAINY_HAS_CXX20
template <typename Ty>
class std::formatter<rainy::utility::reference_wrapper<Ty>, char> // NOLINT
{
public:
    explicit formatter() noexcept = default;

    auto parse(format_parse_context &ctx) const noexcept {
        return ctx.begin();
    }

    auto format(const rainy::utility::reference_wrapper<Ty> &value, std::format_context fc) const noexcept {
        return std::format_to(fc.out(), "{}", value.get());
    }
};
#endif

#endif