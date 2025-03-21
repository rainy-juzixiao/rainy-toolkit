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
#include <unordered_set>
#include <rainy/meta/reflection/type_context.hpp>
#include <rainy/meta/reflection/refl_impl/invoker.hpp>

namespace rainy::meta::reflection::internals {
    struct fake_class {};

    template <typename T>
    struct is_arguments_impl : type_traits::helper::false_type {};

    template <typename... X>
    struct is_arguments_impl<arguments<X...>> : type_traits::helper::true_type {};

    template <typename Ty>
    constexpr bool is_arguments_v = is_arguments_impl<type_traits::other_trans::decay_t<Ty>>::value;

    template <typename... Args>
    constexpr bool contains_arguments_v = (is_arguments_v<Args> || ...);

    static const auto static_type = foundation::rtti::typeinfo::create_typeinfo_by_name("static[no-definite-class-type]");
}

#define RAINY_DECLARE_FUNCTION_CONSTRUCTOR(QUAL)                                                                                      \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    function(ReturnType (Class::*function)(Args...) QUAL) {                                                                           \
        invoker_ = utility::construct_at(                                                                                             \
            reinterpret_cast<remote_invoker_impl<ReturnType (Class::*)(Args...) QUAL, Class, ReturnType, Args...> *>(invoker_storage), \
                                         function);    \
    }

namespace rainy::meta::reflection {
    class argument {
    };

    enum class invoke_strategy {
        as_static,
        as_memfn,
        as_operator
    };

    class RAINY_TOOLKIT_API function {
    public:
        /* 因为invoker始终是无分配的，因此，不考虑额外的逻辑 */
        function() noexcept;

        function(function &&right) noexcept {
            move_from_other(utility::move(right));
        }

        function(const function &right) noexcept {
            copy_from_other(right);
        }

        function(std::nullptr_t) : invoker_storage{}, invoker_{nullptr} {
        }

        template <typename LambdaOrFnObj,
                  type_traits::other_trans::enable_if_t<!type_traits::primary_types::function_traits<LambdaOrFnObj>::valid, int> = 0>
        function(LambdaOrFnObj) = delete; // Lambda表达式或函数对象默认不进行支持

        template <typename ReturnType, typename... Args>
        function(ReturnType (*function)(Args...)) {
            using implemented_type = remote_invoker_impl<ReturnType (*)(Args...), void, ReturnType, Args...>;
            invoker_ = utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage), function);
        }

        template <typename Class, typename ReturnType, typename... Args>
        function(ReturnType (Class::*function)(Args...)) {
            using implemented_type = remote_invoker_impl<ReturnType (Class::*)(Args...), Class, ReturnType, Args...>;
            invoker_ = utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage), function);
        }

        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(&);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(&&);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const &);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const &&);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(& noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(&& noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const & noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const && noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(volatile);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(volatile &);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(volatile &&);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(volatile noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(volatile & noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(volatile && noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const volatile);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const volatile &);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const volatile &&);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const volatile noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const volatile & noexcept);
        RAINY_DECLARE_FUNCTION_CONSTRUCTOR(const volatile && noexcept);

        containers::any invoke_static(arguments_visitor &args = empty_param) const;
        containers::any invoke_static(const arguments_visitor &args = empty_param) const;

        template <typename... Args, type_traits::other_trans::enable_if_t<!internals::contains_arguments_v<Args...>, int> = 0>
        containers::any invoke_static(Args &&...args) const {
            return invoke(invoke_strategy::as_static, non_exists_instance,make_arguments(utility::forward<Args>(args)...));
        }

        containers::any invoke(instance object,arguments_visitor &args = empty_param) const;
        containers::any invoke(instance object,const arguments_visitor &args = empty_param) const;
        containers::any invoke(invoke_strategy strategy, instance object, arguments_visitor &args) const;
        containers::any invoke(invoke_strategy strategy, instance object, const arguments_visitor &args) const;

        containers::any oinvoke(instance object, containers::any ax1) {
            return invoker_->oinvoke(object, ax1);
        }

        containers::any oinvoke(instance object, containers::any ax1, containers::any ax2) {
            return invoker_->oinvoke(object, ax1, ax2);
        }

        containers::any oinvoke(instance object, containers::any ax1, containers::any ax2,containers::any ax3) {
            return invoker_->oinvoke(object, ax1, ax2, ax3);
        }

        containers::any oinvoke(instance object, containers::any ax1, containers::any ax2, containers::any ax3,containers::any ax4) {
            return invoker_->oinvoke(object, ax1, ax2, ax3, ax4);
        }

        containers::any oinvoke(instance object, containers::any ax1, containers::any ax2, containers::any ax3, containers::any ax4,
                                containers::any ax5) {
            return invoker_->oinvoke(object, ax1, ax2, ax3, ax4, ax5);
        }

        containers::any oinvoke(instance object, containers::any ax1, containers::any ax2, containers::any ax3, containers::any ax4,
                                containers::any ax5,containers::any ax6) {
            return invoker_->oinvoke(object, ax1, ax2, ax3, ax4, ax5, ax6);
        }

        containers::any oinvoke(instance object, containers::any ax1, containers::any ax2, containers::any ax3, containers::any ax4,
                                containers::any ax5, containers::any ax6, containers::any ax7) {
            return invoker_->oinvoke(object, ax1, ax2, ax3, ax4, ax5, ax6, ax7);
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<!internals::contains_arguments_v<Args...>,int> = 0>
        containers::any invoke_paramlist(instance instance, Args &&...args) const {
            utility::expects(!empty(), "You're trying to invoke an empty object!");
            auto params = make_arguments<Args...>(utility::forward<Args>(args)...);
            return invoker_->invoke(instance, params);
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<!internals::contains_arguments_v<Args...>,int> = 0>
        containers::any operator()(instance instance, Args &&...args) const {
            return invoke_paramlist(instance, utility::forward<Args>(args)...);
        }

        const foundation::rtti::typeinfo &return_type() const noexcept;

        const containers::array_view<foundation::rtti::typeinfo> param_lists() const noexcept;

        containers::any invoke_variadic(instance instance, std::vector<containers::any> &any_args) const;
        containers::any invoke_variadic(instance instance, const std::vector<containers::any> &any_args) const;

        bool empty() const noexcept;

        void copy_from_other(const function &right) noexcept;

        void move_from_other(function &&right) noexcept;

        void swap(function &right) noexcept;

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

        const foundation::rtti::typeinfo &which_belongs() const noexcept;

        bool is_static() const noexcept;

        bool is_const() const noexcept;

        bool is_volatile() const noexcept;

        template <typename Fx>
        Fx target() const noexcept {
            auto ptr = reinterpret_cast<Fx*>(invoker_->target(rainy_typeid(Fx)));
            return *ptr;
        }

        friend bool operator==(const function &left, const function &right) noexcept {
            return left.equal_with(right);
        }

        friend bool operator!=(const function &left, const function &right) noexcept {
            return left.not_equal_with(right);
        }

    private:
        template <typename ReturnType>
        static const foundation::rtti::typeinfo &return_type_res() noexcept {
            return rainy_typeid(ReturnType);
        }

        template <typename... Args>
        static const containers::array<foundation::rtti::typeinfo, sizeof...(Args)> &param_types_res() noexcept {
            static const containers::array<foundation::rtti::typeinfo, sizeof...(Args)> param_types = {
                foundation::rtti::typeinfo::create<Args>()...};
            return param_types;
        }

        template <typename FunctionSignature>
        static const foundation::rtti::typeinfo &function_signature_res() noexcept {
            static const foundation::rtti::typeinfo signature = foundation::rtti::typeinfo::create<FunctionSignature>();
            return signature;
        }

        template <typename Class>
        static const foundation::rtti::typeinfo &which_belongs_res() noexcept {
            if constexpr (type_traits::type_relations::is_void_v<Class>) {
                return internals::static_type;
            } else {
                return rainy_typeid(Class);
            }
        }

        struct remote_invoker {
            virtual ~remote_invoker() = default;
            virtual std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept = 0;
            virtual method_type type() const noexcept = 0;
            /* oinvoke将通过以下接口调用 */
            virtual containers::any oinvoke(instance object, containers::any &ax1) const = 0;
            virtual containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2) const = 0;
            virtual containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2,
                                           containers::any &ax3) const = 0;
            virtual containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                           containers::any &ax4) const = 0;
            virtual containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                           containers::any &ax4, containers::any &ax5) const = 0;
            virtual containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                           containers::any &ax4, containers::any &ax5, containers::any &ax6) const = 0;
            virtual containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                           containers::any &ax4, containers::any &ax5, containers::any &ax6,
                                           containers::any &ax7) const = 0;
            /* 普通invoke接口通过如下接口调用 */
            virtual containers::any invoke(instance obj, arguments_visitor &args) const = 0;
            virtual containers::any invoke_variadic(instance obj, std::vector<containers::any> &any_args) const = 0;
            virtual const foundation::rtti::typeinfo &which_belongs() const noexcept = 0;
            virtual const foundation::rtti::typeinfo &return_type() const noexcept = 0;
            virtual const foundation::rtti::typeinfo &function_signature() const noexcept = 0;
            virtual remote_invoker *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;
            virtual const containers::array_view<foundation::rtti::typeinfo> param_types() const noexcept = 0;
            virtual bool equal_with(remote_invoker *impl) const noexcept = 0;
        };

        template <typename Fx, typename Class, typename ReturnType, typename... Args>
        struct remote_invoker_impl : remote_invoker {
            using function_signature_t = Fx;
            using storage_t = internals::invoker<Fx, Args...>;

            template <typename Method>
            remote_invoker_impl(Method method) {
                utility::construct_at(&this->storage, method);
            }

            containers::any invoke_variadic(instance object, std::vector<containers::any> &params) const override {
#if RAINY_ENABLE_DEBUG
                utility::expects(
                    object.rtti().is_compatible(rainy_typeid(Class)),
                    "We can't invoke this function because we found the ClassType is not same with your passed instance!");
#else
                if (!object.rtti().is_compatible(rainy_typeid(Class))) {
                    return {};
                }
#endif
                return storage.invoke_variadic(object.get_pointer(), containers::make_array_view(params));
            }

            containers::any invoke(instance object, arguments_visitor &args) const override {
#if RAINY_ENABLE_DEBUG
                utility::expects(
                    object.rtti().is_compatible(rainy_typeid(Class)),
                    "We can't invoke this function because we found the ClassType is not same with your passed instance!");
                // utility::expects(storage.is_invocable(args.typeinfo()), "Cannot invoke this function!");
#else
                if (!object.rtti().is_compatible(rainy_typeid(Class))) {
                    return {};
                }
#endif
                return storage.invoke(object.get_pointer(), args);
            }

            const foundation::rtti::typeinfo &return_type() const noexcept override {
                return return_type_res<ReturnType>();
            }

            const containers::array_view<foundation::rtti::typeinfo> param_types() const noexcept override {
                return param_types_res<Args...>();
            }

            const foundation::rtti::typeinfo &function_signature() const noexcept override {
                return function_signature_res<function_signature_t>();
            }

            const foundation::rtti::typeinfo &which_belongs() const noexcept override {
                return which_belongs_res<Class>();
            }

            remote_invoker *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
                return utility::construct_at(reinterpret_cast<remote_invoker_impl *>(soo_buffer), storage.fn);
            }

            method_type type() const noexcept override {
                return storage.type;
            }

            bool equal_with(remote_invoker *impl) const noexcept override {
                if (function_signature() != impl->function_signature()) {
                    return false;
                }
                if (impl->type() != type()) {
                    return false;
                }
                auto cast_impl = static_cast<remote_invoker_impl *>(impl);
                return storage.fn == cast_impl->storage.fn;
            }

            std::uintptr_t target(const foundation::rtti::typeinfo &fx_sign) const noexcept override {
                if (fx_sign != function_signature()) {
                    return 0;
                }
                return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
            }

            containers::any oinvoke(instance object, containers::any &ax1) const override {
                return storage.oinvoke(object.get_pointer(), ax1);
            }

            containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2) const override {
                return storage.oinvoke(object.get_pointer(), ax1, ax2);
            }

            containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3) const override {
                return storage.oinvoke(object.get_pointer(), ax1, ax2, ax3);
            }

            containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                    containers::any &ax4) const override {
                return storage.oinvoke(object.get_pointer(), ax1, ax2, ax3, ax4);
            }

            containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                    containers::any &ax4, containers::any &ax5) const override {
                return storage.oinvoke(object.get_pointer(), ax1, ax2, ax3, ax4, ax5);
            }

            containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                    containers::any &ax4, containers::any &ax5, containers::any &ax6) const override {
                return storage.oinvoke(object.get_pointer(), ax1, ax2, ax3, ax4, ax5, ax6);
            }

            containers::any oinvoke(instance object, containers::any &ax1, containers::any &ax2, containers::any &ax3,
                                    containers::any &ax4, containers::any &ax5, containers::any &ax6, containers::any &ax7) const override {
                return storage.oinvoke(object.get_pointer(), ax1, ax2, ax3, ax4, ax5, ax6, ax7);
            }

            storage_t storage;
        };

        static constexpr inline std::size_t soo_buffer_size =
            sizeof(remote_invoker_impl<void (internals::fake_class::*)(int), internals::fake_class, void, int>);

        alignas(std::max_align_t) core::byte_t invoker_storage[soo_buffer_size]{};
        remote_invoker *invoker_{nullptr};
    };
}

namespace rainy::meta::reflection {
    class metadata {
    public:
        metadata() = default;
        metadata(const metadata &) = default;
        metadata(metadata &&) = default;
        metadata &operator=(const metadata &) = default;
        metadata &operator=(metadata &&) = default;
        ~metadata() = default;

        metadata(std::string_view name) : name_{name} {
        }

        metadata(std::string_view name, containers::any &&data) : name_{name}, data_{utility::move(data)} {
        }

        metadata(std::string_view name, const containers::any &data) : name_{name}, data_{data} {
        }

        std::size_t hash_key() const noexcept {
            return std::hash<std::string_view>{}(name_);
        }

        std::string_view name() const noexcept {
            return name_;
        }

        containers::any* operator->() noexcept {
            return &data_;
        }

        const containers::any *operator->() const noexcept {
            return &data_;
        }

        containers::any *data() noexcept {
            return &data_;
        }

        const containers::any *data() const noexcept {
            return &data_;
        }

        friend bool operator==(const metadata &left, const metadata &right) noexcept {
            return left.name_ == right.name_;
        }

        friend bool operator!=(const metadata &left, const metadata &right) noexcept {
            return left.name_ != right.name_;
        }

    private:
        std::string_view name_;
        containers::any data_;
    };

    using metadata_set = std::vector<metadata>;

    class function_annotation : public function {
    public:
        using function::function;

        function_annotation(std::string_view name) noexcept : name_{name} {
        }

        function_annotation(std::string_view name, const metadata_set &metadatas) noexcept : name_{name}, metadatas_{metadatas} {
        }

        std::string_view name() const noexcept {
            return name_;
        }

        metadata_set &metadatas() noexcept {
            return metadatas_;
        }

        const metadata_set &metadatas() const noexcept {
            return metadatas_;
        }

        metadata& operator[](const std::size_t idx) noexcept {
            return metadatas_[idx];
        }

        const metadata &operator[](const std::size_t idx) const noexcept {
            return metadatas_[idx];
        }

    private:
        std::string_view name_{"unnamed"};
        metadata_set metadatas_{};
    };
}

#undef RAINY_DECLARE_FUNCTION_CONSTRUCTOR

#endif