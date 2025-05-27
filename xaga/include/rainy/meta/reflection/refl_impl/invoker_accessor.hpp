#ifndef RAINY_META_REFL_IMPL_INVOKER_ACCESSOR_HPP
#define RAINY_META_REFL_IMPL_INVOKER_ACCESSOR_HPP
#include <rainy/meta/reflection/refl_impl/invoker.hpp>

namespace rainy::meta::reflection::implements {
    /**
     * @brief function使用的接口抽象层。用于统一访问函数对象，非库开发者不需要直接引用
     */
    struct invoker_accessor {
        virtual ~invoker_accessor() = default;
        RAINY_NODISCARD virtual std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept = 0;
        RAINY_NODISCARD virtual method_flags type() const noexcept = 0;
        virtual utility::any invoke(object_view &object) const = 0;
        virtual utility::any invoke(object_view &object, arg_view arg_view) const = 0;
        RAINY_NODISCARD virtual const foundation::rtti::typeinfo &which_belongs() const noexcept = 0;
        RAINY_NODISCARD virtual const foundation::rtti::typeinfo &return_type() const noexcept = 0;
        RAINY_NODISCARD virtual const foundation::rtti::typeinfo &function_signature() const noexcept = 0;
        virtual invoker_accessor *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;
        RAINY_NODISCARD virtual const rainy::collections::views::array_view<foundation::rtti::typeinfo> &paramlists()
            const noexcept = 0;
        virtual bool equal_with(const invoker_accessor *impl) const noexcept = 0;
        RAINY_NODISCARD virtual bool is_invocable(
            rainy::collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept = 0;
        virtual void destruct(bool local) noexcept = 0;
    };

    template <typename Fx, typename Class, typename ReturnType, typename... Args>
    struct invoker_accessor_impl final : invoker_accessor {
        using function_signature_t = Fx;
        using storage_t = invoker<Fx, Args...>;
        using typelist = type_traits::other_trans::type_list<Args...>;

        template <typename Functor>
        explicit invoker_accessor_impl(Functor &&method) noexcept {
            utility::construct_at(&this->storage, utility::forward<Functor>(method));
        }

        RAINY_NODISCARD const foundation::rtti::typeinfo &return_type() const noexcept override {
            return implements::return_type_res<ReturnType>();
        }

        RAINY_NODISCARD const collections::views::array_view<foundation::rtti::typeinfo> &paramlists() const noexcept override {
            static const collections::views::array_view<foundation::rtti::typeinfo> paramlist = implements::param_types_res<Args...>();
            return paramlist;
        }

        RAINY_NODISCARD const foundation::rtti::typeinfo &function_signature() const noexcept override {
            return implements::function_signature_res<function_signature_t>();
        }

        RAINY_NODISCARD const foundation::rtti::typeinfo &which_belongs() const noexcept override {
            return implements::which_belongs_res<Class>();
        }

        RAINY_NODISCARD invoker_accessor *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (is_fnobj<Fx>::value) {
                return ::new invoker_accessor_impl(storage.fn);
            } else {
                return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), storage.fn);
            }
        }

        RAINY_NODISCARD method_flags type() const noexcept override {
            return storage.type;
        }

        RAINY_NODISCARD bool equal_with(const invoker_accessor *impl) const noexcept override {
            if (function_signature() != impl->function_signature()) {
                return false;
            }
            if (impl->type() != type()) {
                return false;
            }
            auto cast_impl = static_cast<const invoker_accessor_impl *>(impl);
            if constexpr (is_fnobj<Fx>::value) {
                return std::memcmp(reinterpret_cast<const void *>(&storage.fn), reinterpret_cast<const void *>(&cast_impl->storage.fn),
                                   sizeof(storage.fn)) == 0;
            } else {
                return storage.fn == cast_impl->storage.fn;
            }
        }

        RAINY_NODISCARD std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept override {
            if (fx_sign != function_signature()) {
                if (fx_sign == rainy_typeid(function *)) {
                    return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
                }
                return 0;
            }
            return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
        }

        RAINY_NODISCARD utility::any invoke(object_view &object) const override {
#if RAINY_ENABLE_DEBUG
            utility::expects(object.rtti().is_compatible(rainy_typeid(Class)));
#endif
            if constexpr (storage_t::arity == 0) {
                return storage.access_invoke(object.get_pointer());
            } else {
                return {};
            }
        }

        RAINY_NODISCARD utility::any invoke(object_view &object, arg_view arg_view) const override {
#if RAINY_ENABLE_DEBUG
            utility::expects(object.rtti().is_compatible(rainy_typeid(Class)));
#endif
            return storage.invoke(object.get_pointer(), &arg_view);
        }

        RAINY_NODISCARD bool is_invocable(
            collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept override {
            if (storage.arity != paramlist.size()) {
                return false;
            }
            std::size_t paramhash =
                core::accumulate(paramlist.begin(), paramlist.end(), std::size_t{0},
                                 [right = std::size_t{1}](const std::size_t acc, const foundation::rtti::typeinfo &item) mutable {
                                     return acc + (item.hash_code() * right++);
                                 });
            if (paramhash == storage.param_hash) {
                return true;
            }
            if (storage.is_compatible(paramlist)) {
                return true;
            }
            return is_invocable_helper(paramlist, type_traits::helper::make_index_sequence<storage_t::arity>{});
        }

        template <std::size_t... I>
        bool is_invocable_helper(collections::views::array_view<foundation::rtti::typeinfo> paramlist,
                                 type_traits::helper::index_sequence<I...>) const noexcept {
            return (... && utility::any_converter<typename type_traits::other_trans::type_at<I, typelist>::type>::is_convertible(
                               paramlist[I]));
        }

        void destruct(bool local) noexcept override {
            if (local) {
                this->storage.~storage();
            } else {
                delete this;
            }
        }

        union {
            storage_t storage;
            std::max_align_t dummy{};
        };
    };

    template <typename Fx, typename Traits,
              typename TypeList = typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
    struct get_ia_implement_type {
        using unused_type1 = Fx;
        using unused_type2 = TypeList;
    };

    template <typename Fx, typename Traits, typename... Args>
    struct get_ia_implement_type<Fx, Traits, type_traits::other_trans::type_list<Args...>> {
        using memptr_traits = type_traits::primary_types::member_pointer_traits<Fx>;

        template <typename FxTraits, bool IsMemptr = FxTraits::valid>
        struct decl_class {
            using type = void;
            static RAINY_CONSTEXPR_BOOL unused = IsMemptr;
        };

        template <typename FxTraits>
        struct decl_class<FxTraits, true> {
            using type = typename FxTraits::class_type;
        };

        using type = invoker_accessor_impl<Fx, typename decl_class<memptr_traits>::type, typename Traits::return_type, Args...>;
    };

    template <typename Fx, typename = void>
    struct try_to_get_invoke_operator : type_traits::helper::false_type {
        static RAINY_CONSTEXPR_BOOL is_lambda_without_capture = false;
    };

    template <typename Fx>
    struct try_to_get_invoke_operator<Fx, type_traits::other_trans::void_t<decltype(&Fx::operator())>>
        : type_traits::helper::true_type {
        template <typename Ty, typename = void>
        struct test_is_lambda_without_capture : type_traits::helper::false_type {};

        template <typename Ty>
        struct test_is_lambda_without_capture<
            Ty, type_traits::other_trans::void_t<decltype(+utility::declval<type_traits::cv_modify::remove_cv_t<Ty>>())>>
            : type_traits::helper::bool_constant<
                  type_traits::primary_types::is_pointer_v<decltype(+utility::declval<type_traits::cv_modify::remove_cv_t<Ty>>())>> {};

        template <typename Ty, typename Traits,
                  typename TypeList =
                      typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
        struct get_fn_obj_invoke_if_default_constructible {
            using unused_type1 = Ty;
            using unused_type2 = TypeList;
        };

        template <typename Ty, typename Traits, typename... Args>
        struct get_fn_obj_invoke_if_default_constructible<Ty, Traits, type_traits::other_trans::type_list<Args...>> {
            static decltype(auto) invoke(Args... args) noexcept(Traits::is_noexcept) {
                static Ty fn_obj{};
                return fn_obj(utility::forward<Args>(args)...);
            };
        };

        static RAINY_CONSTEXPR_BOOL is_lambda_without_capture = test_is_lambda_without_capture<Fx>::value;

        static constexpr auto method = &Fx::operator();
    };
}

#endif