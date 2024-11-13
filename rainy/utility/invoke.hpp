#ifndef RAINY_INVOKE_HPP
#define RAINY_INVOKE_HPP
#include <rainy/core.hpp>
#include <rainy/utility/ref_wrap.hpp>

namespace rainy::utility::internals {
    enum class invoker_category {
        functor,
        pmf_object,
        pmf_refwrap,
        pmf_pointer,
        pmd_object,
        pmd_refwrap,
        pmd_pointer
    };

    template <invoker_category>
    struct invoker_impl {};

    template <>
    struct invoker_impl<invoker_category::functor> {
        static constexpr auto category = invoker_category::functor;

        template <typename Callable, typename... Args>
        static auto call(Callable &&callable,
                         Args &&...args) noexcept(noexcept(static_cast<Callable &&>(callable)(static_cast<Args &&>(args)...)))
            -> decltype(rainy::utility::forward<Callable>(callable)(rainy::utility::forward<Args>(args)...)) {
            return static_cast<Callable &&>(callable)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_object> {
        static constexpr auto category = invoker_category::pmf_object;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto call(Decayed pmf, Ty &&args1,
                                   Args &&...args) noexcept(noexcept((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)))
            -> decltype((static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...)) {
            return (static_cast<Ty &&>(args1).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_refwrap> {
        static constexpr auto category = invoker_category::pmf_refwrap;

        template <typename Decayed,template<typename> typename RefWrap,typename Ty,typename... Args>
        static constexpr auto call(Decayed pmf, RefWrap<Ty> ref_wrap,
                                   Args &&...args) noexcept(noexcept((ref_wrap.get().*pmf)(static_cast<Args &&>(args)...))) {
            return (ref_wrap.get().*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmf_pointer> {
        static constexpr auto category = invoker_category::pmf_pointer;

        template <typename Decayed, typename Ty, typename... Args>
        static constexpr auto call(Decayed pmf, Ty &&args1,
                                   Args &&...args) noexcept(noexcept(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)))
            -> decltype(((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...)) {
            return ((*static_cast<Ty &&>(args1)).*pmf)(static_cast<Args &&>(args)...);
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_object> {
        static constexpr auto category = invoker_category::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto call(Decayed pmd, Ty &&args) noexcept -> decltype(static_cast<Ty &&>(args).*pmd) {
            return static_cast<Ty &&>(args).*pmd;
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_refwrap> {
        static constexpr auto category = invoker_category::pmd_refwrap;

        template <typename Decayed, template <typename> typename RefWrap, typename Ty>
        static constexpr auto call(Decayed pmd, RefWrap<Ty> ref_wrap) noexcept -> decltype(ref_wrap.get().*pmd) {
            return ref_wrap.get().*pmd;
        }
    };

    template <>
    struct invoker_impl<invoker_category::pmd_pointer> {
        static constexpr auto category = invoker_category::pmd_pointer;

        template <typename Decayed, typename Ty>
        static constexpr auto call(Decayed pmd, Ty &&args) noexcept(noexcept((*static_cast<Ty &&>(args)).*pmd))
            -> decltype((*static_cast<Ty &&>(args)).*pmd) {
            return (*static_cast<Ty &&>(args)).*pmd;
        }
    };

    template <typename Callable, typename Ty1, typename RemovedCvref = rainy::type_traits::cv_modify::remove_cvref_t<Callable>,
              bool is_pmf = rainy::type_traits::primary_types::is_member_function_pointer_v<RemovedCvref>,
              bool is_pmd = rainy::type_traits::primary_types::is_member_object_pointer_v<RemovedCvref>>
    struct select_invoker {};

    template <typename Callable, typename Ty1,typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, true, false> {
        static constexpr invoker_category
            value = rainy::type_traits::other_transformations::conditional_value_v < invoker_category,
            rainy::type_traits::type_relations::is_same_v<
                typename rainy::type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type,
                rainy::type_traits::cv_modify::remove_cvref_t<Ty1>> ||
                rainy::type_traits::type_relations::is_base_of_v<
                    typename rainy::type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type,
                    rainy::type_traits::cv_modify::remove_cvref_t<Ty1>>,
            invoker_category::pmf_object, rainy::type_traits::other_transformations::conditional_value_v < invoker_category,
            rainy::type_traits::primary_types::is_specialization_v<rainy::type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                   std::reference_wrapper> ||
                rainy::type_traits::primary_types::is_specialization_v<rainy::type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                       utility::reference_wrapper>,
            invoker_category::pmf_refwrap, invoker_category::pmf_pointer >>
            ;
    };

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, false, true> {
        static constexpr invoker_category
            value = rainy::type_traits::other_transformations::conditional_value_v < invoker_category,
            rainy::type_traits::type_relations::is_same_v<
                typename rainy::type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type,
                rainy::type_traits::cv_modify::remove_cvref_t<Ty1>> ||
                rainy::type_traits::type_relations::is_base_of_v<
                    typename rainy::type_traits::primary_types::member_pointer_traits<RemovedCvref>::class_type,
                    rainy::type_traits::cv_modify::remove_cvref_t<Ty1>>,
            invoker_category::pmd_object,

            rainy::type_traits::other_transformations::conditional_value_v < invoker_category,
            rainy::type_traits::primary_types::is_specialization_v<rainy::type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                   std::reference_wrapper> ||
                rainy::type_traits::primary_types::is_specialization_v<rainy::type_traits::cv_modify::remove_cvref_t<Ty1>,
                                                                       rainy::utility::reference_wrapper>,
            invoker_category::pmd_refwrap, invoker_category::pmd_pointer >>
            ;
    };

    template <typename Callable, typename Ty1, typename RemovedCvref>
    struct select_invoker<Callable, Ty1, RemovedCvref, false, false> {
        static constexpr auto value = invoker_category::functor;
    };

    template <typename Callable,typename Ty1>
    struct invoker : invoker_impl<select_invoker<Callable, Ty1>::value> {
    };
}

namespace rainy::utility {
    template <typename callable>
    constexpr auto invoke(callable &&object) noexcept(noexcept(static_cast<callable &&>(object)()))
        -> decltype(static_cast<callable &&>(object)()) {
        return static_cast<callable &&>(object)();
    }

    template <typename Callable, typename Ty, typename... Args>
    constexpr auto invoke(Callable &&object, Ty &&args1, Args &&...args2) noexcept(
        noexcept(internals::invoker<Callable, Ty>::call(static_cast<Callable &&>(object), static_cast<Ty &&>(args1), static_cast<Args &&>(args2)...)))
        -> decltype(internals::invoker<Callable, Ty>::call(static_cast<Callable &&>(object), static_cast<Ty &&>(args1),
                                                                        static_cast<Args &&>(args2)...)) {
        using internals::invoker;
        using internals::invoker_category;
        if constexpr (invoker<Callable, Ty>::category == invoker_category::functor) {
            /* functor（函数或仿函数调用） */
            return static_cast<Callable &&>(object)(static_cast<Ty &&>(args1), static_cast<Args &&>(args2)...);
        } else if constexpr (invoker<Callable, Ty>::category == invoker_category::pmf_object) {
            /* member_function（对象调用类成员函数指针） */
            return (static_cast<Ty &&>(args1).*object)(static_cast<Args &&>(args2)...);
        } else if constexpr (invoker<Callable, Ty>::category == invoker_category::pmf_refwrap) {
            /* member_function（引用包装器调用类成员函数指针） */
            return (args1.get().*object)(static_cast<Args &&>(args2)...);
        } else if constexpr (invoker<Callable, Ty>::category == invoker_category::pmf_pointer) {
            /* member_function（引用包装器调用类成员函数指针） */
            return (static_cast<const Ty &&>(args1)->*object)(static_cast<Args &&>(args2)...);
        } else if constexpr (invoker<Callable, Ty>::category == invoker_category::pmd_object) {
            return static_cast<Ty &&>(args1).*object;
        } else if constexpr (invoker<Callable, Ty>::category == invoker_category::pmd_refwrap) {
            return args1.get().*object;
        } else {
            static_assert(invoker<Callable, Ty>::category == invoker_category::pmd_pointer);
            return (*static_cast<Ty &&>(args1)).*object;
        }
    }
}

#endif
