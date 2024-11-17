#ifndef RAINY_INVOKE_HPP
#define RAINY_INVOKE_HPP
#include <rainy/core.hpp>
#include <rainy/utility/ref_wrap.hpp>

namespace rainy::utility {
    enum class invoker_category {
        decl_failed, // 推导失败
        functor, // 仿函数或函数类型
        pmf_object, // 类成员函数——对象调用
        pmf_refwrap, // 类成员函数——对象引用包装器调用
        pmf_pointer, // 类成员函数——对象指针调用
        pmd_object, // 类成员变量——对象调用
        pmd_refwrap, // 类成员变量——对象引用包装器调用
        pmd_pointer // 类成员变量——对象指针调用
    };
}

namespace rainy::utility::internals {
    template <invoker_category>
    struct invoker_impl {
        static constexpr auto category = invoker_category::decl_failed;
    };

    template <>
    struct invoker_impl<invoker_category::functor> {
        static constexpr auto category = invoker_category::functor;

        template <typename Callable, typename... Args>
        static auto invoke(Callable &&callable,
                           Args &&...args) noexcept(noexcept(static_cast<Callable &&>(callable)(static_cast<Args &&>(args)...)))
            -> decltype(rainy::utility::forward<Callable>(callable)(rainy::utility::forward<Args>(args)...)) {
            return static_cast<Callable &&>(callable)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_object> {
        static constexpr auto category = invoker_category::pmf_object;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept((static_cast<Ty &&>(args1).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)) {
            return (static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_refwrap> {
        static constexpr auto category = invoker_category::pmf_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, RefWrap<Ty> ref_wrap,
                                     Args &&...args) noexcept(noexcept((ref_wrap.get().*pmf)(static_cast<Args &&>(args)...))) {
            return (ref_wrap.get().*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_pointer> {
        static constexpr auto category = invoker_category::pmf_pointer;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto invoke(Decayed pmf, Ty &&args1, Args &&...args) noexcept(noexcept(((*static_cast<Ty &&>(args1)).*
                                                                                                 pmf)(static_cast<Args &&>(args)...)))
            -> decltype(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)) {
            return ((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_object> {
        static constexpr auto category = invoker_category::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept -> decltype(static_cast<Ty &&>(args).*pmd) {
            return static_cast<Ty &&>(args).*pmd;
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_refwrap> {
        static constexpr auto category = invoker_category::pmd_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty>
        static constexpr auto invoke(Decayed pmd, RefWrap<Ty> ref_wrap) noexcept -> decltype(ref_wrap.get().*pmd) {
            return ref_wrap.get().*pmd;
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_pointer> {
        static constexpr auto category = invoker_category::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto invoke(Decayed pmd, Ty &&args) noexcept(noexcept((*static_cast<Ty &&>(args)).*pmd))
            -> decltype((*static_cast<Ty &&>(args)).*pmd) {
            return (*static_cast<Ty &&>(args)).*pmd;
        }
    };

    template <typename Callable, typename Ty1, typename RemovedCvref = type_traits::cv_modify::remove_cvref_t<Callable>,
              bool is_pmf = type_traits::primary_types::is_member_function_pointer_v<RemovedCvref>,
              bool is_pmd = type_traits::primary_types::is_member_object_pointer_v<RemovedCvref>>
    struct select_invoker {};

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, true, false> {
        using __class_type = typename type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type;
        using __ty1_type = type_traits::cv_modify::remove_cvref_t<Ty1>;

        template <bool Test, invoker_category IfTrue, invoker_category IfFalse>
        static constexpr auto cond_v =
            type_traits::other_transformations::conditional_value_v<invoker_category, Test, IfTrue, IfFalse>;

        static constexpr auto value = cond_v < type_traits::type_relations::is_same_v<__class_type, __ty1_type> ||
                                      type_traits::type_relations::is_base_of_v<__class_type, __ty1_type>,
                              invoker_category::pmf_object,
                              cond_v < type_traits::primary_types::is_specialization_v<__ty1_type, std::reference_wrapper> ||
                                  type_traits::primary_types::is_specialization_v<__ty1_type, utility::reference_wrapper>,
                              invoker_category::pmf_refwrap, invoker_category::pmf_pointer >>
            ;
    };

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, false, true> {
        using __class_type = typename type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type;
        using __ty1_type = type_traits::cv_modify::remove_cvref_t<Ty1>;

        template <bool Test, invoker_category IfTrue, invoker_category IfFalse>
        static constexpr auto cond_v =
            type_traits::other_transformations::conditional_value_v<invoker_category, Test, IfTrue, IfFalse>;

        static constexpr auto value = cond_v < type_traits::type_relations::is_same_v<__class_type, __ty1_type> ||
                                      type_traits::type_relations::is_base_of_v<__class_type, __ty1_type>,
                              invoker_category::pmd_object,
                              cond_v < type_traits::primary_types::is_specialization_v<__ty1_type, std::reference_wrapper> ||
                                  type_traits::primary_types::is_specialization_v<__ty1_type, utility::reference_wrapper>,
                              invoker_category::pmd_refwrap, invoker_category::pmd_pointer >>
            ;
    };

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, false, false> {
        static constexpr auto value =
            type_traits::other_transformations::conditional_value_v<invoker_category,
                                                                    type_traits::primary_types::function_traits<RemovedCvref>::valid,
                                                                    invoker_category::functor, invoker_category::decl_failed>;
        /*
        rainy's toolkit 会自动推导Callable模板；如果推导成功，则可以认为是一个functor，否则推导失败，因为pmf和pmd均做过检测。
        此外，关于推导的细节。function_traits具有61个特化类型，尽可能用于精准的识别Callable是否符合要求
        */
    };
}

namespace rainy::utility {
    template <typename Callable, typename Ty1>
    struct invoker : internals::invoker_impl<internals::select_invoker<Callable, Ty1>::value> {};

    template <typename callable>
    constexpr auto invoke(callable &&object) noexcept(noexcept(static_cast<callable &&>(object)()))
        -> decltype(static_cast<callable &&>(object)()) {
        return static_cast<callable &&>(object)();
    }

    template <typename Callable, typename Ty, typename... Args>
    constexpr auto invoke(Callable &&object, Ty &&args1, Args &&...args2) noexcept(noexcept(
        invoker<Callable, Ty>::invoke(static_cast<Callable &&>(object), static_cast<Ty &&>(args1), static_cast<Args &&>(args2)...)))
        -> decltype(invoker<Callable, Ty>::invoke(static_cast<Callable &&>(object), static_cast<Ty &&>(args1),
                                                  static_cast<Args &&>(args2)...)) {
        using invoker_t = invoker<Callable, Ty>;
        static_assert(invoker_t::category != invoker_category::decl_failed);
        // 我们可以直接使用invoker提供的invoke模板函数完成调用
        return invoker_t::invoke(utility::forward<Callable>(object), utility::forward<Ty>(args1), utility::forward<Args>(args2)...);
    }
}

#endif
