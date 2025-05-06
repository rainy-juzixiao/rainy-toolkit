/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_META_RELF_IMPL_FUNCTION_HPP
#define RAINY_META_RELF_IMPL_FUNCTION_HPP
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/meta/reflection/refl_impl/invoker.hpp>
#include <rainy/meta/reflection/refl_impl/object_view.hpp>

namespace rainy::meta::reflection::implements {
    struct remote_invoker {
        virtual ~remote_invoker() = default;
        virtual std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept = 0;
        virtual method_type type() const noexcept = 0;
        virtual utility::any invoke(object_view object) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3,
                                    utility::any &ax4) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3, utility::any &ax4,
                                    utility::any &ax5) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3, utility::any &ax4,
                                    utility::any &ax5, utility::any &ax6) const = 0;
        virtual utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3, utility::any &ax4,
                                    utility::any &ax5, utility::any &ax6, utility::any &ax7) const = 0;
        virtual utility::any invoke_variadic(object_view obj, rainy::collections::views::array_view<utility::any> &any_args) const = 0;
        virtual const foundation::rtti::typeinfo &which_belongs() const noexcept = 0;
        virtual const foundation::rtti::typeinfo &return_type() const noexcept = 0;
        virtual const foundation::rtti::typeinfo &function_signature() const noexcept = 0;
        virtual remote_invoker *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;
        virtual const rainy::collections::views::array_view<foundation::rtti::typeinfo> param_types() const noexcept = 0;
        virtual bool equal_with(const remote_invoker *impl) const noexcept = 0;
        virtual bool is_invocable(rainy::collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept = 0;
    };

    template <typename Fx, typename Class, typename ReturnType, typename... Args>
    struct remote_invoker_impl : implements::remote_invoker {
        using function_signature_t = Fx;
        using storage_t = implements::invoker<Fx, Args...>;
        using typelist = type_traits::other_trans::type_list<Args...>;

        template <typename Method>
        remote_invoker_impl(Method method) noexcept {
            utility::construct_at(&this->storage, method);
        }

        utility::any invoke_variadic(object_view object, rainy::collections::views::array_view<utility::any> &params) const override {
#if RAINY_ENABLE_DEBUG
            utility::expects(object.rtti().is_compatible(rainy_typeid(Class)),
                             "We can't invoke this function because we found the ClassType is not same with your passed instance!");
#endif
            if (storage.arity == params.size()) {
                return invoke_variadic_helper(object.get_pointer(), params.data(),
                                              type_traits::helper::make_index_sequence<storage.arity>{});
            }
            return {};
        }

        const foundation::rtti::typeinfo &return_type() const noexcept override {
            return implements::return_type_res<ReturnType>();
        }

        const rainy::collections::views::array_view<foundation::rtti::typeinfo> param_types() const noexcept override {
            return implements::param_types_res<Args...>();
        }

        const foundation::rtti::typeinfo &function_signature() const noexcept override {
            return implements::function_signature_res<function_signature_t>();
        }

        const foundation::rtti::typeinfo &which_belongs() const noexcept override {
            return implements::which_belongs_res<Class>();
        }

        remote_invoker *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            return utility::construct_at(reinterpret_cast<remote_invoker_impl *>(soo_buffer), storage.fn);
        }

        method_type type() const noexcept override {
            return storage.type;
        }

        bool equal_with(const remote_invoker *impl) const noexcept override {
            if (function_signature() != impl->function_signature()) {
                return false;
            }
            if (impl->type() != type()) {
                return false;
            }
            auto cast_impl = static_cast<const remote_invoker_impl *>(impl);
            return storage.fn == cast_impl->storage.fn;
        }

        std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept override {
            if (fx_sign != function_signature()) {
                if (fx_sign == rainy_typeid(function *)) {
                    return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
                }
                return 0;
            }
            return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
        }

        utility::any invoke(object_view object) const override {
            return storage.invoke(object.get_pointer());
        }

        utility::any invoke(object_view object, utility::any &ax1) const override {
            return storage.invoke(object.get_pointer(), ax1);
        }

        utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2) const override {
            return storage.invoke(object.get_pointer(), ax1, ax2);
        }

        utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3) const override {
            return storage.invoke(object.get_pointer(), ax1, ax2, ax3);
        }

        utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3,
                            utility::any &ax4) const override {
            return storage.invoke(object.get_pointer(), ax1, ax2, ax3, ax4);
        }

        utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3, utility::any &ax4,
                            utility::any &ax5) const override {
            return storage.invoke(object.get_pointer(), ax1, ax2, ax3, ax4, ax5);
        }

        utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3, utility::any &ax4,
                            utility::any &ax5, utility::any &ax6) const override {
            return storage.invoke(object.get_pointer(), ax1, ax2, ax3, ax4, ax5, ax6);
        }

        utility::any invoke(object_view object, utility::any &ax1, utility::any &ax2, utility::any &ax3, utility::any &ax4,
                            utility::any &ax5, utility::any &ax6, utility::any &ax7) const override {
            return storage.invoke(object.get_pointer(), ax1, ax2, ax3, ax4, ax5, ax6, ax7);
        }

        bool is_invocable(collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept override {
            if (storage.arity != paramlist.size()) {
                return false;
            }
            std::size_t paramhash =
                core::accumulate(paramlist.begin(), paramlist.end(), std::size_t{0},
                                 [&paramhash](std::size_t acc, const utility::any &item) { return acc + item.type().hash_code(); });
            if (paramhash == storage.param_hash) {
                return true;
            }
            return is_invocable_helper(paramlist, type_traits::helper::make_index_sequence<storage.arity>{});
        }

        template <std::size_t... I>
        bool is_invocable_helper(collections::views::array_view<foundation::rtti::typeinfo> paramlist, type_traits::helper::index_sequence<I...>) const noexcept {
            return (... && utility::any_converter<typename type_traits::other_trans::type_at<I, typelist>::type>::is_convertible(
                               paramlist[I]));
        }

        template <std::size_t... I>
        utility::any invoke_variadic_helper(void *object, utility::any *data, type_traits::helper::index_sequence<I...>) const {
            return storage.invoke(object, (static_cast<void>(I), *data++)...);
        }

        storage_t storage;
    };    

    template <typename Fx,typename Traits,typename TypeList = typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
    struct get_ri_implement_type {};

    template <typename Fx, typename Traits, typename... Args>
    struct get_ri_implement_type<Fx, Traits, type_traits::other_trans::type_list<Args...>> {
        using memptr_traits = type_traits::primary_types::member_pointer_traits<Fx>;

        template <typename Traits, bool IsMemptr = Traits::valid>
        struct decl_class {
            using type = void;
        };

        template <typename Traits>
        struct decl_class<Traits, true> {
            using type = typename Traits::class_type;
        };

        using type = remote_invoker_impl<Fx, typename decl_class<memptr_traits>::type, typename Traits::return_type, Args...>;
    };
}

namespace rainy::meta::reflection {
    class RAINY_TOOLKIT_API function final {
    public:
        /* 因为invoker始终是无分配的，因此，不考虑额外的逻辑。***线程安全*** */
        /**
         * @brief 构造一个空的反射函数对象
         */
        function() noexcept;

        /**
         * @brief 从右值引用中进行移动构造
         * @param right 待移动的函数对象
         */
        function(function &&right) noexcept;

        /**
         * @brief 从左值常量引用中进行拷贝构造
         * @param right 待拷贝的函数对象
         */
        function(const function &right) noexcept;
        /**
         * @brief 与默认构造函数的行为相同
         */
        function(std::nullptr_t) noexcept;

        /**
         * @brief lambda表达式或函数对象默认不进行支持，对构造函数进行删除
         * @brief lambda和函数对象的复杂度较高，且在反射中，意义不大
         */
        template <typename LambdaOrFnObj,
                  type_traits::other_trans::enable_if_t<!type_traits::primary_types::function_traits<LambdaOrFnObj>::valid, int> = 0>
        function(LambdaOrFnObj) = delete;

        template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid,int> = 0>
        function(Fx fx) noexcept {
            using implemented_type = implements::get_ri_implement_type<Fx, type_traits::primary_types::function_traits<Fx>>::type;
            utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage), fx);
        }

        /**
         * @brief 调用函数，以静态方式，并返回结果
         * @tparam ...Args 任意数量的函数参数类型，但是其数量需要与目标调用的参数数量一致
         * @param ...args 任意数量的函数实参，需要与目标调用的参数数量一致
         * @return 函数调用结果，以any形式
         */
        template <typename... Args>
        utility::any invoke_static(Args &&...args) const {
            return invoke(non_exists_instance, utility::forward<Args>(args)...);
        }

        utility::any invoke(object_view object) const;
        utility::any invoke(object_view object, utility::any ax1) const;
        utility::any invoke(object_view object, utility::any ax1, utility::any ax2) const;
        utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3) const;
        utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4) const;
        utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                            utility::any ax5) const;
        utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                            utility::any ax5, utility::any ax6) const;
        utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                            utility::any ax5, utility::any ax6, utility::any ax7) const;

        template <typename... Args>
        utility::any invoke_paramlist(object_view instance, Args &&...args) const {
            utility::expects(!empty(), "You're trying to invoke an empty object!");
            if constexpr (sizeof...(Args) == 0) {
                return invoke(instance);
            } else if constexpr (sizeof...(Args) <= 7) {
                return invoke(instance, {std::in_place_type<Args>, utility::forward<Args>(args)}...);
            } else {
                return invoke_variadic(instance, {{std::in_place_type<Args>, utility::forward<Args>(args)}...});
            }
        }

        template <typename... Args>
        utility::any operator()(object_view instance, Args &&...args) const {
            return invoke_paramlist(instance, utility::forward<Args>(args)...);
        }

        /**
         * @brief 以数组容器作为参数，调用目标函数，非静态
         * @param instance 对象的实例引用
         * @param any_args 可以是std::vector，std::array，rainy::collections::array，初始化列表或者是任意可用于构造array_view的对象
         * @return 函数调用结果，以any形式
         */
        utility::any invoke_variadic(object_view instance, rainy::collections::views::array_view<utility::any> any_args) const;

        const foundation::rtti::typeinfo &return_type() const noexcept;

        rainy::collections::views::array_view<foundation::rtti::typeinfo> param_lists() const noexcept;

        /**
         * @brief 检查当前反射函数对象是否为空
         */
        bool empty() const noexcept;

        /**
         * @brief 从常量左值引用中拷贝函数对象
         * @brief 与默认拷贝构造函数的行为相同
         */
        void copy_from_other(const function &right) noexcept;

        /**
         * @brief 从右值引用中移动函数对象
         * @brief 与默认移动构造函数的行为相同
         */
        void move_from_other(function &&right) noexcept;

        /**
         * @brief 交换两个函数对象
         * @brief 与默认swap函数的行为相同
         */
        void swap(function &right) noexcept;

        /**
         * @brief 从常量左值引用中拷贝函数对象
         * @param right 从此对象拷贝函数对象
         * @return 返回对象
         */
        function &operator=(const function &right) noexcept;

        function &operator=(function &&right) noexcept;

        function &operator=(std::nullptr_t) noexcept;

        const foundation::rtti::typeinfo &function_signature() const noexcept;

        method_type type() const noexcept;

        explicit operator bool() const noexcept;

        bool equal_with(const function &right) const noexcept;

        bool not_equal_with(const function &right) const noexcept;

        void clear() noexcept;

        void rebind(function &&function) noexcept;

        void rebind(const function &function) noexcept;

        void rebind(std::nullptr_t) noexcept;

        const foundation::rtti::typeinfo &which_belongs() const noexcept;

        bool is_static() const noexcept;

        bool is_memfn() const noexcept;

        bool is_const() const noexcept;

        bool is_noexcept() const noexcept;

        bool is_volatile() const noexcept;

        bool is_invoke_for_lvalue() const noexcept;

        bool is_invoke_for_rvalue() const noexcept;

        bool is_invocable(rainy::collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept;

        template <typename Fx>
        Fx target() const noexcept {
            auto ptr = reinterpret_cast<Fx *>(
                reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->target(rainy_typeid(Fx)));
            return *ptr;
        }

        friend bool operator==(const function &left, const function &right) noexcept {
            return left.equal_with(right);
        }

        friend bool operator!=(const function &left, const function &right) noexcept {
            return left.not_equal_with(right);
        }

        friend std::ostream &operator<<(std::ostream &os, const rainy::meta::reflection::function &f) {
            if (f.empty()) {
                return os;
            }
            return os << f.function_signature().name() << " -> " << f.target<rainy::meta::reflection::function *>();
        }

    private:
        static constexpr inline std::size_t soo_buffer_size =
            sizeof(implements::remote_invoker_impl<void (implements::fake_class::*)(int), implements::fake_class, void, int>);

        alignas(std::max_align_t) core::byte_t invoker_storage[soo_buffer_size]{};
    };

    template <typename Fx,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<function, Fx>, int> = 0>
    function make_function(Fx &&fx) noexcept {
        return function{utility::forward<Fx>(fx)};
    }

    template <typename... Args>
    utility::any invoke(const function &fn, object_view instance, Args &&...args) {
        return fn.invoke_paramlist(instance, utility::forward<Args>(args)...);
    }
}

namespace rainy::utility {
    RAINY_INLINE void swap(meta::reflection::function &left, meta::reflection::function &right) noexcept {
        left.swap(right);
    }
}

namespace std {
    inline void swap(rainy::meta::reflection::function& left, rainy::meta::reflection::function& right) noexcept {
        left.swap(right);
    }
}

#undef RAINY_DECLARE_FUNCTION_CONSTRUCTOR

#endif