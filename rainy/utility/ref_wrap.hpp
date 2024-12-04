#ifndef RAINY_REF_WRAP_HPP
#define RAINY_REF_WRAP_HPP

#include <rainy/core/core.hpp>

#if RAINY_HAS_CXX20
#include <format>
#endif

namespace rainy::utility::internals {
    template <typename Ty>
    void refwrap_ctor_fun(type_traits::helper::identity_t<Ty&>) noexcept {
    }

    template <typename Ty>
    void refwrap_ctor_fun(type_traits::helper::identity_t<Ty &&>) = delete;

    template <typename Ty, typename Uty, typename = void>
    struct refwrap_has_ctor_from : type_traits::helper::false_type {};

    template <typename Ty, typename Uty>
    struct refwrap_has_ctor_from<Ty, Uty, type_traits::other_trans::void_t<decltype(refwrap_ctor_fun<Ty>(declval<Uty>()))>>
        : type_traits::helper::true_type {};

    template <typename Fx,typename... Args>
    struct test_refwrap_nothrow_invoke {
        static auto test() {
            if constexpr (type_traits::type_properties::is_invocable_v<Fx, Args...>) {
                if constexpr (type_traits::primary_types::function_traits<Fx>::is_nothrow_invocable) {
                    return type_traits::helper::true_type{};
                } else {
                    return type_traits::helper::false_type{};
                }
            } else {
                return type_traits::helper::false_type{};
            }
        }

        static RAINY_CONSTEXPR_BOOL value = decltype(test())::value;
    };
}

namespace rainy::utility {
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(type_traits::internals::_is_object_v<Ty> || type_traits::internals::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        using type = Ty;

        template <
            typename Uty,
            type_traits::other_trans::enable_if_t<
                type_traits::logical_traits::conjunction_v<type_traits::logical_traits::negation<type_traits::type_relations::is_same<
                                                               type_traits::cv_modify::remove_cvref_t<Uty>, reference_wrapper>>,
                                                           internals::refwrap_has_ctor_from<Ty, Uty>>,
                int> = 0>
        constexpr reference_wrapper(Uty &&val) noexcept(noexcept(internals::refwrap_ctor_fun<Ty>(declval<Uty>()))) {
            Ty &ref = static_cast<Uty &&>(val);
            this->reference_data = utility::addressof(ref);
        }

        reference_wrapper(const reference_wrapper &) = delete;
        reference_wrapper(reference_wrapper &&) = delete;

        constexpr operator Ty &() const noexcept {
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

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) try_to_invoke_as_function(Args &&...args) const
            noexcept(internals::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            using f_traits = type_traits::primary_types::function_traits<Ty>;
            if constexpr (f_traits::valid) {
                using return_type = f_traits::return_type;
                if (type_traits::type_properties::is_invocable_r_v<return_type, Ty, Args...>) {
                    // 经过实践证明的是，reference_wrapper不能引用成员函数，因此像不同函数一样调用也就够了
                    if constexpr (type_traits::primary_types::is_void_v<return_type>) {
                        get(utility::forward<Args>(args)...);
                    } else {
                        return get()(utility::forward<Args>(args)...);
                    }
                }
            }
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_v<Ty, Args...>, int> = 0>
        constexpr decltype(auto) operator()(Args &&...args) const
            noexcept(internals::test_refwrap_nothrow_invoke<Ty, Args...>::value) {
            return try_to_invoke_as_function(utility::forward<Args>(args)...);
        }

    private:
        Ty *reference_data{nullptr};
    };

    template <typename Uty>
    reference_wrapper(Uty &) -> reference_wrapper<type_traits::cv_modify::remove_cvref_t<Uty>>; // 使用Deduction Guide模板参数推导

    template <typename Ty>
    void ref(const Ty &&) = delete;

    template <typename Ty>
    void cref(const Ty &&) = delete;

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(Ty &val) noexcept {
        return reference_wrapper<Ty>(val);
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<Ty> ref(reference_wrapper<Ty> val) noexcept {
        return val;
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(const Ty &val) noexcept {
        return reference_wrapper<const Ty>(val);
    }

    template <typename Ty>
    RAINY_NODISCARD RAINY_CONSTEXPR20 reference_wrapper<const Ty> cref(reference_wrapper<Ty> val) noexcept {
        return val;
    }
}

namespace rainy::type_traits::cv_modify {
    template <typename Ty>
    struct unwrap_reference {
        using type = Ty;
    };

    template <typename Ty>
    struct unwrap_reference<utility::reference_wrapper<Ty>> {
        using type = Ty;
    };

    template <typename Ty>
    struct unwrap_reference<std::reference_wrapper<Ty>> {
        using type = Ty;
    };

    template <class _Ty>
    using unwrap_reference_t = typename unwrap_reference<_Ty>::type;

    template <typename Ty>
    using unwrap_ref_decay_t = unwrap_reference_t<other_trans::decay_t<Ty>>;

    template <typename Ty>
    struct unwrap_ref_decay {
        using type = unwrap_ref_decay_t<Ty>;
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