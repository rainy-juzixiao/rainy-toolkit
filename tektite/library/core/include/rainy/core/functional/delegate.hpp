/*
 * Copyright 2026 rainy-juzixiao
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
#ifndef RAINY_CORE_FUNCTIONAL_DELEGATE_HPP
#define RAINY_CORE_FUNCTIONAL_DELEGATE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/typeinfo.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>

namespace rainy::functional::exceptions {
    /**
     * \lang english
     * @brief Exception thrown when a delegate is invoked while it is empty.
     *
     * \lang simp-chinese
     * @brief 当委托为空时被调用而抛出的异常。
     */
    class invalid_delegate : public core::exceptions::runtime::runtime_error {
    public:
        using base = runtime_error;

        /**
         * \lang english
         * @brief Constructs an invalid_delegate at the given source location.
         * @param loc The source location where the invalid call occurred.
         *
         * \lang simp-chinese
         * @brief 在给定的源码位置构造invalid_delegate。
         * @param loc 无效调用发生处的源码位置。
         */
        invalid_delegate(source loc = source::current()) : base("bad delegation call", loc) {
        }
    };

    /**
     * \lang english
     * @brief Throws an invalid_delegate exception.
     * @param loc The source location where the invalid call occurred.
     *
     * \lang simp-chinese
     * @brief 抛出invalid_delegate异常。
     * @param loc 无效调用发生处的源码位置。
     */
    RAINY_INLINE void throw_invalid_delegate(utility::source_location loc = utility::source_location::current()) {
        throw_exception(invalid_delegate(loc));
    }
}

namespace rainy::functional {
    template <typename Fx>
    class delegate;

    template <typename Fx>
    class move_only_delegate;

    using core::method_flags;
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::functional::implements {
    static constexpr std::size_t soo_threshold = core::fn_obj_soo_buffer_size + 8;

    template <typename Rx, typename Class, typename... Args>
    struct invoker_accessor {
        virtual ~invoker_accessor() = default;
        virtual Rx invoke(Class *object, Args &&...args) = 0;
        RAINY_NODISCARD virtual std::uintptr_t target(const core::typeinfo &fx_sign, bool no_check) const noexcept = 0;
        RAINY_NODISCARD virtual const core::typeinfo &target_type() const noexcept = 0;
        virtual void destruct(bool local) noexcept = 0;
        virtual invoker_accessor *move(core::byte_t *soo_buffer) noexcept = 0;
        virtual invoker_accessor *copy(core::byte_t *soo_buffer) const = 0;
        RAINY_NODISCARD virtual method_flags type() const noexcept = 0;
    };

    template <typename Impl>
    RAINY_CONSTEXPR_BOOL fits_soo = sizeof(Impl) < soo_threshold;

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct invoker_accessor_impl : invoker_accessor<Rx, Class, Args...> {
        using fx_traits = type_traits::primary_types::function_traits<Fx>;
        using instance_t = Class;
        using base = invoker_accessor<Rx, Class, Args...>;

        template <typename UFx>
        invoker_accessor_impl(UFx &&fn) : fn(utility::forward<UFx>(fn)) {
        }

        Rx invoke(Class *object, Args &&...args) override {
            if constexpr (type_traits::type_relations::is_void_v<Class>) {
                if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                    utility::invoke(this->fn, utility::forward<Args>(args)...);
                } else {
                    return utility::invoke(this->fn, utility::forward<Args>(args)...);
                }
            } else {
                if constexpr (fx_traits::is_invoke_for_lvalue) {
                    if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                        utility::invoke(this->fn, static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                        utility::forward<Args>(args)...);
                    } else {
                        return utility::invoke(this->fn, static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                               utility::forward<Args>(args)...);
                    }
                } else if constexpr (fx_traits::is_invoke_for_rvalue) {
                    if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                        utility::invoke(this->fn, static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                        utility::forward<Args>(args)...);
                    } else {
                        return utility::invoke(this->fn, static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                               utility::forward<Args>(args)...);
                    }
                } else {
                    if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                        utility::invoke(this->fn, static_cast<instance_t *>(object), utility::forward<Args>(args)...);
                    } else {
                        return utility::invoke(this->fn, static_cast<instance_t *>(object), utility::forward<Args>(args)...);
                    }
                }
            }
        }

        void destruct(bool local) noexcept override {
            if (local) {
                this->~invoker_accessor_impl();
            } else {
                delete this;
            }
        }

        base *move(core::byte_t *soo_buffer) noexcept override {
            if constexpr (fits_soo<invoker_accessor_impl>) {
                return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), utility::move(fn));
            } else {
                return ::new (std::nothrow) invoker_accessor_impl(utility::move(fn));
            }
        }

        base *copy(core::byte_t *soo_buffer) const override {
            if constexpr (type_traits::properties::is_copy_constructible_v<Fx>) {
                if constexpr (fits_soo<invoker_accessor_impl>) {
                    return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), fn);
                } else {
                    return ::new invoker_accessor_impl(fn);
                }
            } else {
                std::terminate();
            }
        }

        RAINY_NODISCARD method_flags type() const noexcept override {
            static constexpr method_flags flags = core::deduction_invoker_type<Fx, Args...>();
            return flags;
        }

        std::uintptr_t target(const core::typeinfo &fx_sign, bool no_check) const noexcept override {
            if (fx_sign == rainy_typeid(Fx) || no_check) {
                return reinterpret_cast<std::uintptr_t>(utility::addressof(fn));
            }
            return 0;
        }

        const core::typeinfo &target_type() const noexcept override {
            return rainy_typeid(Fx);
        }

    private:
        Fx fn;
    };

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct get_ia_implement_type {
        using type = invoker_accessor_impl<type_traits::other_trans::decay_t<Fx>, Rx, Class, Args...>;
    };

    template <typename Fx, typename Rx, typename TypeList>
    class delegate_impl {};

    template <typename Rx, typename Callable, typename Class, typename... Args>
    struct is_invocable_object
        : type_traits::logical_traits::disjunction<
              type_traits::properties::is_invocable_r<Rx, Callable, Class &, Args...>,
              type_traits::properties::is_invocable_r<Rx, Callable, Class, Args...>> {};

    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_object<Rx, Callable, void, Args...>
        : type_traits::properties::is_invocable_r<Rx, Callable, Args...> {};

    template <typename Fx, typename Rx, typename... Args>
    class delegate_impl<Fx, Rx, type_traits::other_trans::type_list<Args...>> {
    public:
        using paramlist = type_traits::other_trans::type_list<Args...>;
        using result_type = Rx;
        using function_type = Fx;
        using class_t = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;

        static constexpr std::size_t arity = sizeof...(Args);

        template <typename UFx, typename Delegate>
        using enable_if_callable_t = type_traits::other_trans::enable_if_t<
            !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<UFx>, Delegate> &&
                implements::is_invocable_object<Rx, type_traits::other_trans::decay_t<UFx> &, class_t, Args...>::value,
            int>;

        template <typename UFx>
        friend class functional::delegate;

        template <typename UFx>
        friend class functional::move_only_delegate;

        delegate_impl() noexcept = default;

        ~delegate_impl() {
            reset();
        }

        template <typename Class, typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Class, Args...>,
                                                        int> = 0>
        Rx invoke(Class &&object, Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            }
        }

        template <typename Class, typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Class, Args...> &&
                                                            type_traits::primary_types::is_member_function_pointer_v<Fx>,
                                                        int> = 0>
        Rx operator()(Class &&object, Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        Rx invoke(Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        Rx operator()(Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            }
        }

        bool empty() const noexcept {
            return invoker_accessor_ == nullptr;
        }

        void reset() noexcept {
            if (!empty()) {
                invoker_accessor()->destruct(is_local());
                invoker_accessor_ = nullptr;
            }
        }

        void swap(delegate_impl &right) noexcept {
            if (is_local() || right.is_local()) {
                delegate_impl temp;
                temp.move_from_other(utility::move(*this));
                move_from_other(utility::move(right));
                right.move_from_other(utility::move(temp));
            } else {
                std::swap(invoker_accessor_, right.invoker_accessor_);
            }
        }

        template <typename UFx, enable_if_callable_t<UFx, delegate<Fx>> = 0>
        void rebind(UFx &&func) {
            reset();
            using implemented_type = typename get_ia_implement_type<UFx, Rx, class_t, Args...>::type;
            if constexpr (implements::fits_soo<implemented_type>) {
                invoker_accessor_ =
                    utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage), utility::forward<UFx>(func));
            } else {
                invoker_accessor_ = ::new implemented_type(utility::forward<UFx>(func));
            }
        }

        void rebind(delegate_impl &&right) noexcept {
            reset();
            move_from_other(utility::move(right));
        }

        void rebind(const delegate_impl &right) {
            reset();
            copy_from_other(right);
        }

        void move_from_other(delegate_impl &&right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            reset();
            if (!right.empty()) {
                invoker_accessor_ = right.invoker_accessor()->move(invoker_storage);
                right.invoker_accessor()->destruct(right.is_local());
                right.invoker_accessor_ = nullptr;
            }
        }

        void copy_from_other(const delegate_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            reset();
            if (!right.empty()) {
                invoker_accessor_ = right.invoker_accessor()->copy(invoker_storage);
            }
        }

        RAINY_NODISCARD const core::typeinfo &function_signature() const noexcept {
            return rainy_typeid(Fx);
        }

        RAINY_NODISCARD const core::typeinfo &return_type() const noexcept {
            return rainy_typeid(Rx);
        }

        RAINY_NODISCARD const core::typeinfo &which_belongs() const noexcept {
            return rainy_typeid(class_t);
        }

        RAINY_NODISCARD const core::typeinfo &target_type() const noexcept {
            return invoker_accessor()->target_type();
        }

       RAINY_NODISCARD  method_flags type() const noexcept {
            if (empty()) {
                return method_flags::none;
            }
            return invoker_accessor()->type();
        }

        RAINY_NODISCARD bool has(method_flags flag) const noexcept {
            return static_cast<bool>(type() & flag);
        }

        RAINY_NODISCARD bool is_const() const noexcept {
            return has(method_flags::const_qualified);
        }

        RAINY_NODISCARD bool is_noexcept() const noexcept {
            return has(method_flags::noexcept_specified);
        }

        RAINY_NODISCARD bool is_invoke_for_lvalue() const noexcept {
            return has(method_flags::lvalue_qualified);
        }

        RAINY_NODISCARD bool is_invoke_for_rvalue() const noexcept {
            return has(method_flags::rvalue_qualified);
        }

        RAINY_NODISCARD bool is_static() const noexcept {
            return has(method_flags::static_specified);
        }

        RAINY_NODISCARD bool is_volatile() const noexcept {
            return has(method_flags::volatile_qualified);
        }

        RAINY_NODISCARD bool is_memfn() const noexcept {
            return !is_static();
        }

        template <typename UFx>
        const UFx *target() const noexcept {
            if (empty()) {
                return nullptr;
            }
            if constexpr (type_traits::primary_types::is_member_function_pointer_v<UFx>) {
                if constexpr (type_traits::type_relations::is_convertible_v<Fx, UFx>) {
                    return reinterpret_cast<const UFx *>(invoker_accessor()->target(rainy_typeid(Fx), true));
                }
            } else {
                auto ptr = reinterpret_cast<const UFx *>(invoker_accessor()->target(rainy_typeid(UFx), false));
                return ptr ? ptr : nullptr;
            }
            return nullptr;
        }

        template <typename UFx>
        UFx *target() noexcept {
            if (empty()) {
                return nullptr;
            }
            if constexpr (type_traits::primary_types::is_member_function_pointer_v<UFx>) {
                if constexpr (type_traits::type_relations::is_convertible_v<Fx, UFx>) {
                    return reinterpret_cast<UFx *>(invoker_accessor()->target(rainy_typeid(Fx), true));
                }
            } else {
                auto ptr = reinterpret_cast<UFx *>(invoker_accessor()->target(rainy_typeid(UFx), false));
                return ptr ? ptr : nullptr;
            }
            return nullptr;
        }

        explicit operator bool() const noexcept {
            return !empty();
        }

    private:
        bool is_local() const noexcept {
            return static_cast<const void *>(invoker_accessor_) == static_cast<const void *>(invoker_storage);
        }

        implements::invoker_accessor<Rx, class_t, Args...> *invoker_accessor() const {
            return invoker_accessor_;
        }

        alignas(std::max_align_t) core::byte_t invoker_storage[core::fn_obj_soo_buffer_size]{};
        implements::invoker_accessor<Rx, class_t, Args...> *invoker_accessor_{nullptr};
    };

    template <typename Fx>
    struct get_delegate_impl {
        using type = delegate_impl<Fx, typename type_traits::primary_types::function_traits<Fx>::return_type,
                                   typename type_traits::primary_types::function_traits<Fx>::argument_list>;
    };
}

namespace rainy::functional {
    template <typename Fx>
    class delegate final : public implements::get_delegate_impl<Fx>::type {
    public:
        using base = typename implements::get_delegate_impl<Fx>::type;

        delegate() noexcept = default;

        template <typename UFx, typename base::template enable_if_callable_t<UFx, delegate> = 0>
        delegate(UFx &&fn) {
            this->rebind(utility::forward<UFx>(fn));
        }

        delegate(const delegate &right) {
            this->copy_from_other(right);
        }

        delegate(delegate &&right) noexcept {
            this->move_from_other(utility::move(right));
        }

        delegate(std::nullptr_t) noexcept {
        }

        ~delegate() {
            this->reset();
        }

        template <typename UFx, typename base::template enable_if_callable_t<UFx, delegate> = 0>
        delegate &operator=(UFx &&fn) {
            this->rebind(utility::forward<UFx>(fn));
            return *this;
        }

        delegate &operator=(const delegate &right) {
            this->copy_from_other(right);
            return *this;
        }

        delegate &operator=(delegate &&right) noexcept {
            this->rebind(utility::move(right));
            return *this;
        }

        delegate &operator=(std::nullptr_t) noexcept {
            this->reset();
            return *this;
        }
    };

    template <typename Fx>
    delegate(Fx &&) -> delegate<Fx>;
}

namespace rainy::functional {
    template <typename Fx>
    class move_only_delegate final : public implements::get_delegate_impl<Fx>::type {
    public:
        using base = typename implements::get_delegate_impl<Fx>::type;

        move_only_delegate() noexcept = default;

        move_only_delegate(const move_only_delegate &) = delete;
        move_only_delegate &operator=(const move_only_delegate &) = delete;

        move_only_delegate(move_only_delegate &&right) noexcept {
            this->move_from_other(utility::move(right));
        }

        ~move_only_delegate() {
            this->reset();
        }

        move_only_delegate &operator=(move_only_delegate &&right) noexcept {
            this->rebind(utility::move(right));
            return *this;
        }

        template <typename UFx, typename base::template enable_if_callable_t<UFx, move_only_delegate> = 0>
        move_only_delegate(UFx &&fn) {
            this->rebind(utility::forward<UFx>(fn));
        }

        template <typename UFx, typename base::template enable_if_callable_t<UFx, move_only_delegate> = 0>
        move_only_delegate &operator=(UFx &&fn) {
            this->rebind(utility::forward<UFx>(fn));
            return *this;
        }

        move_only_delegate(std::nullptr_t) noexcept {
        }

        move_only_delegate &operator=(std::nullptr_t) noexcept {
            this->reset();
            return *this;
        }
    };

    template <typename Fx>
    move_only_delegate(Fx &&) -> move_only_delegate<Fx>;
}

#else

namespace rainy::functional::implements {
    /**
     * \lang english
     * @brief Small-object optimization threshold used by delegate implementations.
     *
     *  Implementations whose size is below this threshold are stored inline in the
     *  delegate's internal buffer; larger implementations are allocated on the heap.
     *
     * \lang simp-chinese
     * @brief delegate 实现所使用的小对象优化阈值。
     *
     *  大小低于该阈值的实现被内联存储在 delegate 的内部缓冲区中；更大的实现
     *  则在堆上分配。
     */
    static constexpr std::size_t soo_threshold = core::fn_obj_soo_buffer_size + 8;

    /**
     * \lang english
     * @brief Type-erased invoker interface for a callable of a given signature.
     *
     *  Provides the polymorphic operations needed by delegate: invocation, target
     *  querying, destruction, moving and copying. Implementations derive from this
     *  and are stored either inline (small-object optimization) or on the heap.
     *
     * @tparam Rx The return type of the invoker.
     * @tparam Class The class type on which the callable is invoked, or void for
     *               free callables.
     * @tparam Args The argument types of the callable.
     *
     * \lang simp-chinese
     * @brief 针对给定签名可调用对象的类型擦除调用器接口。
     *
     *  提供 delegate 所需的多态操作：调用、目标查询、析构、移动与拷贝。实现类
     *  派生于它，并被内联存储（小对象优化）或在堆上存储。
     *
     * @tparam Rx 调用器的返回类型。
     * @tparam Class 调用可调用对象时使用的类类型，对于自由可调用对象为 void。
     * @tparam Args 可调用对象的参数类型。
     */
    template <typename Rx, typename Class, typename... Args>
    struct invoker_accessor {
        virtual ~invoker_accessor() = default;
        virtual Rx invoke(Class *object, Args &&...args) = 0;
        RAINY_NODISCARD virtual std::uintptr_t target(const core::typeinfo &fx_sign, bool no_check) const noexcept = 0;
        RAINY_NODISCARD virtual const core::typeinfo &target_type() const noexcept = 0;
        virtual void destruct(bool local) noexcept = 0;
        virtual invoker_accessor *move(core::byte_t *soo_buffer) noexcept = 0;
        virtual invoker_accessor *copy(core::byte_t *soo_buffer) const = 0;
        RAINY_NODISCARD virtual method_flags type() const noexcept = 0;
    };

    /**
     * \lang english
     * @brief Trait indicating whether an implementation fits in the SOO buffer.
     *
     * @tparam Impl The implementation type.
     *
     * \lang simp-chinese
     * @brief 指示某实现是否可放入 SOO 缓冲区的特征。
     *
     * @tparam Impl 实现类型。
     */
    template <typename Impl>
    RAINY_CONSTEXPR_BOOL fits_soo = sizeof(Impl) < soo_threshold;

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct invoker_accessor_impl;

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct get_ia_implement_type {
        using type = invoker_accessor_impl<type_traits::other_trans::decay_t<Fx>, Rx, Class, Args...>;
    };

    template <typename Fx, typename Rx, typename TypeList>
    class delegate_impl {};

    template <typename Fx, typename Rx, typename... Args>
    class delegate_impl<Fx, Rx, type_traits::other_trans::type_list<Args...>> {
    public:
        /**
         * \lang english
         * @brief The list of argument types of the delegate.
         *
         * \lang simp-chinese
         * @brief delegate 的参数类型列表。
         */
        using paramlist = type_traits::other_trans::type_list<Args...>;

        /**
         * \lang english
         * @brief The return type of the delegate.
         *
         * \lang simp-chinese
         * @brief delegate 的返回类型。
         */
        using result_type = Rx;

        /**
         * \lang english
         * @brief The function signature type of the delegate.
         *
         * \lang simp-chinese
         * @brief delegate 的函数签名类型。
         */
        using function_type = Fx;

        /**
         * \lang english
         * @brief The class type on which the stored callable is invoked.
         *
         * \lang simp-chinese
         * @brief 调用所存储可调用对象时使用的类类型。
         */
        using class_t = typename type_traits::primary_types::member_pointer_traits<Fx>::class_type;

        /**
         * \lang english
         * @brief The number of arguments of the delegate.
         *
         * \lang simp-chinese
         * @brief delegate 的参数个数。
         */
        static constexpr std::size_t arity = sizeof...(Args);

        /**
         * \lang english
         * @brief SFINAE helper enabling a constructor when the argument is callable
         *        and is not the delegate itself.
         *
         * @tparam UFx The candidate callable type.
         * @tparam Delegate The delegate type to exclude.
         *
         * \lang simp-chinese
         * @brief 当参数可调用且不是 delegate 自身时，启用某构造函数的 SFINAE 辅助。
         *
         * @tparam UFx 候选可调用类型。
         * @tparam Delegate 要排除的 delegate 类型。
         */
        template <typename UFx, typename Delegate>
        using enable_if_callable_t = type_traits::other_trans::enable_if_t<
            !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<UFx>, Delegate> &&
                implements::is_invocable_object<Rx, type_traits::other_trans::decay_t<UFx> &, class_t, Args...>::value,
            int>;

        template <typename UFx>
        friend class functional::delegate;

        template <typename UFx>
        friend class functional::move_only_delegate;

        /**
         * \lang english
         * @brief Constructs an empty delegate implementation.
         *
         * \lang simp-chinese
         * @brief 构造一个空的 delegate 实现。
         */
        delegate_impl() noexcept = default;

        /**
         * \lang english
         * @brief Destructor, releasing the stored callable.
         *
         * \lang simp-chinese
         * @brief 析构函数，释放存储的可调用对象。
         */
        ~delegate_impl() {
            reset();
        }

        /**
         * \lang english
         * @brief Invokes the stored callable bound to an object.
         *
         * @tparam Class The object type.
         * @tparam UFx The function signature used for the invocability check.
         * @param object The object to bind the callable to.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 调用绑定到某对象的所存储可调用对象。
         *
         * @tparam Class 对象类型。
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param object 要绑定可调用对象的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename Class, typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Class, Args...>,
                                                        int> = 0>
        Rx invoke(Class &&object, Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            }
        }

        /**
         * \lang english
         * @brief Invokes the stored callable bound to an object (member function form).
         *
         * @tparam Class The object type.
         * @tparam UFx The function signature used for the invocability check.
         * @param object The object to bind the callable to.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 调用绑定到某对象的所存储可调用对象（成员函数形式）。
         *
         * @tparam Class 对象类型。
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param object 要绑定可调用对象的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename Class, typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Class, Args...> &&
                                                            type_traits::primary_types::is_member_function_pointer_v<Fx>,
                                                        int> = 0>
        Rx operator()(Class &&object, Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            }
        }

        /**
         * \lang english
         * @brief Invokes the stored callable without binding an object.
         *
         * @tparam UFx The function signature used for the invocability check.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 在不绑定对象的情况下调用所存储可调用对象。
         *
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        Rx invoke(Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            }
        }

        /**
         * \lang english
         * @brief Invokes the stored callable without binding an object (operator form).
         *
         * @tparam UFx The function signature used for the invocability check.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 在不绑定对象的情况下调用所存储可调用对象（运算符形式）。
         *
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        Rx operator()(Args... args) const {
            if (empty()) {
                exceptions::throw_invalid_delegate();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            }
        }

        /**
         * \lang english
         * @brief Checks whether the delegate is empty.
         *
         * @return true if no callable is stored.
         *
         * \lang simp-chinese
         * @brief 检查 delegate 是否为空。
         *
         * @return 若未存储可调用对象则返回 true。
         */
        bool empty() const noexcept {
            return invoker_accessor_ == nullptr;
        }

        /**
         * \lang english
         * @brief Releases the stored callable, leaving the delegate empty.
         *
         * \lang simp-chinese
         * @brief 释放所存储的可调用对象，使 delegate 变为空。
         */
        void reset() noexcept {
            if (!empty()) {
                invoker_accessor()->destruct(is_local());
                invoker_accessor_ = nullptr;
            }
        }

        /**
         * \lang english
         * @brief Swaps the contents with another delegate implementation.
         *
         * @param right The other implementation to swap with.
         *
         * \lang simp-chinese
         * @brief 与另一个 delegate 实现交换内容。
         *
         * @param right 要交换的另一个实现。
         */
        void swap(delegate_impl &right) noexcept {
            if (is_local() || right.is_local()) {
                delegate_impl temp;
                temp.move_from_other(utility::move(*this));
                move_from_other(utility::move(right));
                right.move_from_other(utility::move(temp));
            } else {
                std::swap(invoker_accessor_, right.invoker_accessor_);
            }
        }

        /**
         * \lang english
         * @brief Stores a new callable, replacing any previously stored one.
         *
         * @tparam UFx The callable type.
         * @param func The callable to store.
         *
         * \lang simp-chinese
         * @brief 存储新的可调用对象，替换此前存储的对象。
         *
         * @tparam UFx 可调用类型。
         * @param func 要存储的可调用对象。
         */
        template <typename UFx, enable_if_callable_t<UFx, delegate<Fx>> = 0>
        void rebind(UFx &&func) {
            reset();
            using implemented_type = typename get_ia_implement_type<UFx, Rx, class_t, Args...>::type;
            if constexpr (implements::fits_soo<implemented_type>) {
                invoker_accessor_ =
                    utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage), utility::forward<UFx>(func));
            } else {
                invoker_accessor_ = ::new implemented_type(utility::forward<UFx>(func));
            }
        }

        /**
         * \lang english
         * @brief Replaces the stored callable by moving from another implementation.
         *
         * @param right The implementation to move from.
         *
         * \lang simp-chinese
         * @brief 通过从另一个实现移动来替换所存储的可调用对象。
         *
         * @param right 要移动的来源实现。
         */
        void rebind(delegate_impl &&right) noexcept {
            reset();
            move_from_other(utility::move(right));
        }

        /**
         * \lang english
         * @brief Replaces the stored callable by copying from another implementation.
         *
         * @param right The implementation to copy from.
         *
         * \lang simp-chinese
         * @brief 通过从另一个实现拷贝来替换所存储的可调用对象。
         *
         * @param right 要拷贝的来源实现。
         */
        void rebind(const delegate_impl &right) {
            reset();
            copy_from_other(right);
        }

        /**
         * \lang english
         * @brief Moves the stored callable from another implementation, leaving it empty.
         *
         * @param right The implementation to move from.
         *
         * \lang simp-chinese
         * @brief 从另一个实现移动所存储的可调用对象，并使后者变为空。
         *
         * @param right 要移动的来源实现。
         */
        void move_from_other(delegate_impl &&right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            reset();
            if (!right.empty()) {
                invoker_accessor_ = right.invoker_accessor()->move(invoker_storage);
                right.invoker_accessor()->destruct(right.is_local());
                right.invoker_accessor_ = nullptr;
            }
        }

        /**
         * \lang english
         * @brief Copies the stored callable from another implementation.
         *
         * @param right The implementation to copy from.
         *
         * \lang simp-chinese
         * @brief 从另一个实现拷贝所存储的可调用对象。
         *
         * @param right 要拷贝的来源实现。
         */
        void copy_from_other(const delegate_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            reset();
            if (!right.empty()) {
                invoker_accessor_ = right.invoker_accessor()->copy(invoker_storage);
            }
        }

        /**
         * \lang english
         * @brief Returns the type info of the delegate's function signature.
         *
         * @return The type info of Fx.
         *
         * \lang simp-chinese
         * @brief 返回 delegate 函数签名的类型信息。
         *
         * @return Fx 的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &function_signature() const noexcept {
            return rainy_typeid(Fx);
        }

        /**
         * \lang english
         * @brief Returns the type info of the delegate's return type.
         *
         * @return The type info of Rx.
         *
         * \lang simp-chinese
         * @brief 返回 delegate 返回类型的类型信息。
         *
         * @return Rx 的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &return_type() const noexcept {
            return rainy_typeid(Rx);
        }

        /**
         * \lang english
         * @brief Returns the type info of the class the callable is invoked on.
         *
         * @return The type info of class_t.
         *
         * \lang simp-chinese
         * @brief 返回调用可调用对象时所用类的类型信息。
         *
         * @return class_t 的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &which_belongs() const noexcept {
            return rainy_typeid(class_t);
        }

        /**
         * \lang english
         * @brief Returns the type info of the stored callable.
         *
         * @return The type info of the stored callable.
         *
         * \lang simp-chinese
         * @brief 返回所存储可调用对象的类型信息。
         *
         * @return 所存储可调用对象的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &target_type() const noexcept {
            return invoker_accessor()->target_type();
        }

        /**
         * \lang english
         * @brief Returns the method flags describing the stored callable.
         *
         * @return The method flags, or method_flags::none if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 返回描述所存储可调用对象的方法标志。
         *
         * @return 方法标志；若 delegate 为空则返回 method_flags::none。
         */
       RAINY_NODISCARD  method_flags type() const noexcept {
            if (empty()) {
                return method_flags::none;
            }
            return invoker_accessor()->type();
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable has the given method flag.
         *
         * @param flag The flag to test.
         * @return true if the flag is set.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否具有给定的方法标志。
         *
         * @param flag 要测试的标志。
         * @return 若已设置该标志则返回 true。
         */
        RAINY_NODISCARD bool has(method_flags flag) const noexcept {
            return static_cast<bool>(type() & flag);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is const-qualified.
         *
         * @return true if const-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为 const 限定。
         *
         * @return 若为 const 限定则返回 true。
         */
        RAINY_NODISCARD bool is_const() const noexcept {
            return has(method_flags::const_qualified);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is noexcept-specified.
         *
         * @return true if noexcept-specified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否指定了 noexcept。
         *
         * @return 若指定了 noexcept 则返回 true。
         */
        RAINY_NODISCARD bool is_noexcept() const noexcept {
            return has(method_flags::noexcept_specified);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is lvalue-qualified.
         *
         * @return true if lvalue-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为左值限定。
         *
         * @return 若为左值限定则返回 true。
         */
        RAINY_NODISCARD bool is_invoke_for_lvalue() const noexcept {
            return has(method_flags::lvalue_qualified);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is rvalue-qualified.
         *
         * @return true if rvalue-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为右值限定。
         *
         * @return 若为右值限定则返回 true。
         */
        RAINY_NODISCARD bool is_invoke_for_rvalue() const noexcept {
            return has(method_flags::rvalue_qualified);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is static.
         *
         * @return true if static.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为静态。
         *
         * @return 若为静态则返回 true。
         */
        RAINY_NODISCARD bool is_static() const noexcept {
            return has(method_flags::static_specified);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is volatile-qualified.
         *
         * @return true if volatile-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为 volatile 限定。
         *
         * @return 若为 volatile 限定则返回 true。
         */
        RAINY_NODISCARD bool is_volatile() const noexcept {
            return has(method_flags::volatile_qualified);
        }

        /**
         * \lang english
         * @brief Checks whether the stored callable is a member function.
         *
         * @return true if it is a member function.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为成员函数。
         *
         * @return 若为成员函数则返回 true。
         */
        RAINY_NODISCARD bool is_memfn() const noexcept {
            return !is_static();
        }

        /**
         * \lang english
         * @brief Retrieves the stored callable as a pointer of the given type.
         *
         * @tparam UFx The type to retrieve.
         * @return A pointer to the stored callable, or nullptr if not available.
         *
         * \lang simp-chinese
         * @brief 以给定类型的指针检索所存储的可调用对象。
         *
         * @tparam UFx 要检索的类型。
         * @return 指向所存储可调用对象的指针；若不可用则返回 nullptr。
         */
        template <typename UFx>
        const UFx *target() const noexcept {
            if (empty()) {
                return nullptr;
            }
            if constexpr (type_traits::primary_types::is_member_function_pointer_v<UFx>) {
                if constexpr (type_traits::type_relations::is_convertible_v<Fx, UFx>) {
                    return reinterpret_cast<const UFx *>(invoker_accessor()->target(rainy_typeid(Fx), true));
                }
            } else {
                auto ptr = reinterpret_cast<const UFx *>(invoker_accessor()->target(rainy_typeid(UFx), false));
                return ptr ? ptr : nullptr;
            }
            return nullptr;
        }

        /**
         * \lang english
         * @brief Retrieves the stored callable as a mutable pointer of the given type.
         *
         * @tparam UFx The type to retrieve.
         * @return A pointer to the stored callable, or nullptr if not available.
         *
         * \lang simp-chinese
         * @brief 以给定类型的可修改指针检索所存储的可调用对象。
         *
         * @tparam UFx 要检索的类型。
         * @return 指向所存储可调用对象的指针；若不可用则返回 nullptr。
         */
        template <typename UFx>
        UFx *target() noexcept {
            if (empty()) {
                return nullptr;
            }
            if constexpr (type_traits::primary_types::is_member_function_pointer_v<UFx>) {
                if constexpr (type_traits::type_relations::is_convertible_v<Fx, UFx>) {
                    return reinterpret_cast<UFx *>(invoker_accessor()->target(rainy_typeid(Fx), true));
                }
            } else {
                auto ptr = reinterpret_cast<UFx *>(invoker_accessor()->target(rainy_typeid(UFx), false));
                return ptr ? ptr : nullptr;
            }
            return nullptr;
        }

        /**
         * \lang english
         * @brief Checks whether the delegate is non-empty.
         *
         * @return true if a callable is stored.
         *
         * \lang simp-chinese
         * @brief 检查 delegate 是否非空。
         *
         * @return 若存储了可调用对象则返回 true。
         */
        explicit operator bool() const noexcept {
            return !empty();
        }

    private:
        bool is_local() const noexcept {
            return static_cast<const void *>(invoker_accessor_) == static_cast<const void *>(invoker_storage);
        }

        implements::invoker_accessor<Rx, class_t, Args...> *invoker_accessor() const {
            return invoker_accessor_;
        }

        alignas(std::max_align_t) core::byte_t invoker_storage[core::fn_obj_soo_buffer_size]{};
        implements::invoker_accessor<Rx, class_t, Args...> *invoker_accessor_{nullptr};
    };

    template <typename Fx>
    struct get_delegate_impl {
        using type = delegate_impl<Fx, typename type_traits::primary_types::function_traits<Fx>::return_type,
                                   typename type_traits::primary_types::function_traits<Fx>::argument_list>;
    };
}

namespace rainy::functional {
    /**
     * \lang english
     * @brief A type-erased callable wrapper that stores any compatible callable.
     *
     *  The delegate stores a function object, member function pointer, or lambda
     *  whose signature matches Fx, using small-object optimization when possible.
     *  Invoking an empty delegate throws invalid_delegate.
     *
     * @tparam Fx The function signature of the delegate.
     *
     * \lang simp-chinese
     * @brief 存储任意兼容可调用对象的类型擦除可调用包装器。
     *
     *  delegate存储函数对象、成员函数指针或lambda等与Fx签名匹配的可调用对象，
     * 并在可能时使用小对象优化。调用空的delegate会抛出invalid_delegate。
     *
     * @tparam Fx delegate的函数签名。
     */
    template <typename Fx>
    class delegate final {
    public:
        /**
         * \lang english
         * @brief Constructs an empty delegate.
         *
         * \lang simp-chinese
         * @brief 构造一个空的delegate。
         */
        delegate() noexcept = default;

        /**
         * \lang english
         * @brief Constructs a delegate from any callable compatible with Fx.
         * @tparam UFx The type of the callable.
         * @param fn The callable to store.
         *
         * \lang simp-chinese
         * @brief 从任何与Fx兼容的可调用对象构造delegate。
         * @tparam UFx 可调用对象的类型。
         * @param fn 要存储的可调用对象。
         */
        template <typename UFx>
        delegate(UFx &&fn);

        /**
         * \lang english
         * @brief Copy constructor.
         * @param right The delegate to copy.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         * @param right 要拷贝的delegate。
         */
        delegate(const delegate &right);

        /**
         * \lang english
         * @brief Move constructor.
         * @param right The delegate to move from.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         * @param right 要移动的delegate。
         */
        delegate(delegate &&right) noexcept;

        /**
         * \lang english
         * @brief Constructs an empty delegate from nullptr.
         *
         * \lang simp-chinese
         * @brief 从nullptr构造一个空的delegate。
         */
        delegate(std::nullptr_t) noexcept;

        /**
         * \lang english
         * @brief Destructor, releasing the stored callable.
         *
         * \lang simp-chinese
         * @brief 析构函数，释放存储的可调用对象。
         */
        ~delegate();

        /**
         * \lang english
         * @brief Assigns a callable compatible with Fx to this delegate.
         * @tparam UFx The type of the callable.
         * @param fn The callable to store.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 将与Fx兼容的可调用对象赋值给此delegate。
         * @tparam UFx 可调用对象的类型。
         * @param fn 要存储的可调用对象。
         * @return 对此delegate的引用。
         */
        template <typename UFx>
        delegate &operator=(UFx &&fn);

        /**
         * \lang english
         * @brief Copy assignment operator.
         * @param right The delegate to copy.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         * @param right 要拷贝的delegate。
         * @return 对此delegate的引用。
         */
        delegate &operator=(const delegate &right);

        /**
         * \lang english
         * @brief Move assignment operator.
         * @param right The delegate to move from.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         * @param right 要移动的delegate。
         * @return 对此delegate的引用。
         */
        delegate &operator=(delegate &&right) noexcept;

        /**
         * \lang english
         * @brief Clears this delegate.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 清空此delegate。
         * @return 对此delegate的引用。
         */
        delegate &operator=(std::nullptr_t) noexcept;

        /**
         * \lang english
         * @brief Invokes the stored callable bound to an object.
         * @tparam Class The object type.
         * @param object The object to bind the callable to.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 调用绑定到某对象的所存储可调用对象。
         * @tparam Class 对象类型。
         * @param object 要绑定可调用对象的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename Class>
        auto invoke(Class &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Invokes the stored callable without binding an object.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 在不绑定对象的情况下调用所存储可调用对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        auto invoke(typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Invokes the stored callable bound to an object (operator form).
         * @tparam Class The object type.
         * @param object The object to bind the callable to.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 调用绑定到某对象的所存储可调用对象（运算符形式）。
         * @tparam Class 对象类型。
         * @param object 要绑定可调用对象的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename Class>
        auto operator()(Class &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Invokes the stored callable without binding an object (operator form).
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 在不绑定对象的情况下调用所存储可调用对象（运算符形式）。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        auto operator()(typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Checks whether the delegate is empty.
         * @return true if no callable is stored.
         *
         * \lang simp-chinese
         * @brief 检查 delegate 是否为空。
         * @return 若未存储可调用对象则返回 true。
         */
        bool empty() const noexcept;

        /**
         * \lang english
         * @brief Releases the stored callable, leaving the delegate empty.
         *
         * \lang simp-chinese
         * @brief 释放所存储的可调用对象，使 delegate 变为空。
         */
        void reset() noexcept;

        /**
         * \lang english
         * @brief Swaps the contents with another delegate.
         * @param right The other delegate to swap with.
         *
         * \lang simp-chinese
         * @brief 与另一个 delegate 交换内容。
         * @param right 要交换的另一个 delegate。
         */
        void swap(delegate &right) noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the delegate's function signature.
         * @return The type info of Fx.
         *
         * \lang simp-chinese
         * @brief 返回 delegate 函数签名的类型信息。
         * @return Fx 的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &function_signature() const noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the delegate's return type.
         * @return The type info of the return type.
         *
         * \lang simp-chinese
         * @brief 返回 delegate 返回类型的类型信息。
         * @return 返回类型的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &return_type() const noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the class the callable is invoked on.
         * @return The type info of the class type.
         *
         * \lang simp-chinese
         * @brief 返回调用可调用对象时所用类的类型信息。
         * @return 类类型的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &which_belongs() const noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the stored callable.
         * @return The type info of the stored callable.
         *
         * \lang simp-chinese
         * @brief 返回所存储可调用对象的类型信息。
         * @return 所存储可调用对象的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &target_type() const noexcept;

        /**
         * \lang english
         * @brief Returns the method flags describing the stored callable.
         * @return The method flags, or method_flags::none if empty.
         *
         * \lang simp-chinese
         * @brief 返回描述所存储可调用对象的方法标志。
         * @return 方法标志；若为空则返回 method_flags::none。
         */
        RAINY_NODISCARD method_flags type() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable has the given method flag.
         * @param flag The flag to test.
         * @return true if the flag is set.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否具有给定的方法标志。
         * @param flag 要测试的标志。
         * @return 若已设置该标志则返回 true。
         */
        RAINY_NODISCARD bool has(method_flags flag) const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is const-qualified.
         * @return true if const-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为 const 限定。
         * @return 若为 const 限定则返回 true。
         */
        RAINY_NODISCARD bool is_const() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is noexcept-specified.
         * @return true if noexcept-specified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否指定了 noexcept。
         * @return 若指定了 noexcept 则返回 true。
         */
        RAINY_NODISCARD bool is_noexcept() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is lvalue-qualified.
         * @return true if lvalue-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为左值限定。
         * @return 若为左值限定则返回 true。
         */
        RAINY_NODISCARD bool is_invoke_for_lvalue() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is rvalue-qualified.
         * @return true if rvalue-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为右值限定。
         * @return 若为右值限定则返回 true。
         */
        RAINY_NODISCARD bool is_invoke_for_rvalue() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is static.
         * @return true if static.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为静态。
         * @return 若为静态则返回 true。
         */
        RAINY_NODISCARD bool is_static() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is volatile-qualified.
         * @return true if volatile-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为 volatile 限定。
         * @return 若为 volatile 限定则返回 true。
         */
        RAINY_NODISCARD bool is_volatile() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is a member function.
         * @return true if it is a member function.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为成员函数。
         * @return 若为成员函数则返回 true。
         */
        RAINY_NODISCARD bool is_memfn() const noexcept;

        /**
         * \lang english
         * @brief Retrieves the stored callable as a pointer of the given type.
         * @tparam UFx The type to retrieve.
         * @return A pointer to the stored callable, or nullptr if not available.
         *
         * \lang simp-chinese
         * @brief 以给定类型的指针检索所存储的可调用对象。
         * @tparam UFx 要检索的类型。
         * @return 指向所存储可调用对象的指针；若不可用则返回 nullptr。
         */
        template <typename UFx>
        const UFx *target() const noexcept;

        /**
         * \lang english
         * @brief Retrieves the stored callable as a mutable pointer of the given type.
         * @tparam UFx The type to retrieve.
         * @return A pointer to the stored callable, or nullptr if not available.
         *
         * \lang simp-chinese
         * @brief 以给定类型的可修改指针检索所存储的可调用对象。
         * @tparam UFx 要检索的类型。
         * @return 指向所存储可调用对象的指针；若不可用则返回 nullptr。
         */
        template <typename UFx>
        UFx *target() noexcept;

        /**
         * \lang english
         * @brief Checks whether the delegate is non-empty.
         * @return true if a callable is stored.
         *
         * \lang simp-chinese
         * @brief 检查 delegate 是否非空。
         * @return 若存储了可调用对象则返回 true。
         */
        explicit operator bool() const noexcept;
    };

    /**
     * \lang english
     * @brief Deduces the delegate type from a callable.
     *
     * \lang simp-chinese
     * @brief 从可调用对象推导delegate类型。
     */
    template <typename Fx>
    delegate(Fx &&) -> delegate<Fx>;
}

namespace rainy::functional {
    /**
     * \lang english
     * @brief A move-only type-erased callable wrapper.
     *
     *  Behaves like delegate but is not copyable; the stored callable can only
     *  be moved, making it suitable for storing non-copyable callables.
     *
     * @tparam Fx The function signature of the delegate.
     *
     * \lang simp-chinese
     * @brief 仅可移动的类型擦除可调用包装器。
     *
     *  行为与delegate类似，但不可拷贝；存储的可调用对象只能被移动，
     *  因此适合存储不可拷贝的可调用对象。
     *
     * @tparam Fx delegate的函数签名。
     */
    template <typename Fx>
    class move_only_delegate final {
    public:
        /**
         * \lang english
         * @brief Constructs an empty move_only_delegate.
         *
         * \lang simp-chinese
         * @brief 构造一个空的move_only_delegate。
         */
        move_only_delegate() noexcept = default;

        /**
         * \lang english
         * @brief Deleted copy constructor; instances are move-only.
         *
         * \lang simp-chinese
         * @brief 删除的拷贝构造函数；实例仅可移动。
         */
        move_only_delegate(const move_only_delegate &) = delete;

        /**
         * \lang english
         * @brief Deleted copy assignment operator; instances are move-only.
         *
         * \lang simp-chinese
         * @brief 删除的拷贝赋值运算符；实例仅可移动。
         */
        move_only_delegate &operator=(const move_only_delegate &) = delete;

        /**
         * \lang english
         * @brief Move constructor.
         * @param right The delegate to move from.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         * @param right 要移动的delegate。
         */
        move_only_delegate(move_only_delegate &&right) noexcept;

        /**
         * \lang english
         * @brief Destructor, releasing the stored callable.
         *
         * \lang simp-chinese
         * @brief 析构函数，释放存储的可调用对象。
         */
        ~move_only_delegate();

        /**
         * \lang english
         * @brief Move assignment operator.
         * @param right The delegate to move from.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         * @param right 要移动的delegate。
         * @return 对此delegate的引用。
         */
        move_only_delegate &operator=(move_only_delegate &&right) noexcept;

        /**
         * \lang english
         * @brief Constructs a move_only_delegate from any callable compatible with Fx.
         * @tparam UFx The type of the callable.
         * @param fn The callable to store.
         *
         * \lang simp-chinese
         * @brief 从任何与Fx兼容的可调用对象构造move_only_delegate。
         * @tparam UFx 可调用对象的类型。
         * @param fn 要存储的可调用对象。
         */
        template <typename UFx>
        move_only_delegate(UFx &&fn);

        /**
         * \lang english
         * @brief Assigns a callable compatible with Fx to this delegate.
         * @tparam UFx The type of the callable.
         * @param fn The callable to store.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 将与Fx兼容的可调用对象赋值给此delegate。
         * @tparam UFx 可调用对象的类型。
         * @param fn 要存储的可调用对象。
         * @return 对此delegate的引用。
         */
        template <typename UFx>
        move_only_delegate &operator=(UFx &&fn);

        /**
         * \lang english
         * @brief Constructs an empty move_only_delegate from nullptr.
         *
         * \lang simp-chinese
         * @brief 从nullptr构造一个空的move_only_delegate。
         */
        move_only_delegate(std::nullptr_t) noexcept;

        /**
         * \lang english
         * @brief Clears this delegate.
         * @return Reference to this delegate.
         *
         * \lang simp-chinese
         * @brief 清空此delegate。
         * @return 对此delegate的引用。
         */
        move_only_delegate &operator=(std::nullptr_t) noexcept;

        /**
         * \lang english
         * @brief Invokes the stored callable bound to an object.
         * @tparam Class The object type.
         * @param object The object to bind the callable to.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 调用绑定到某对象的所存储可调用对象。
         * @tparam Class 对象类型。
         * @param object 要绑定可调用对象的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename Class>
        auto invoke(Class &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Invokes the stored callable without binding an object.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 在不绑定对象的情况下调用所存储可调用对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        auto invoke(typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Invokes the stored callable bound to an object (operator form).
         * @tparam Class The object type.
         * @param object The object to bind the callable to.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 调用绑定到某对象的所存储可调用对象（运算符形式）。
         * @tparam Class 对象类型。
         * @param object 要绑定可调用对象的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        template <typename Class>
        auto operator()(Class &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Invokes the stored callable without binding an object (operator form).
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Throws invalid_delegate if the delegate is empty.
         *
         * \lang simp-chinese
         * @brief 在不绑定对象的情况下调用所存储可调用对象（运算符形式）。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 若 delegate 为空则抛出 invalid_delegate。
         */
        auto operator()(typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const
            -> typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief Checks whether the delegate is empty.
         * @return true if no callable is stored.
         *
         * \lang simp-chinese
         * @brief 检查 delegate 是否为空。
         * @return 若未存储可调用对象则返回 true。
         */
        bool empty() const noexcept;

        /**
         * \lang english
         * @brief Releases the stored callable, leaving the delegate empty.
         *
         * \lang simp-chinese
         * @brief 释放所存储的可调用对象，使 delegate 变为空。
         */
        void reset() noexcept;

        /**
         * \lang english
         * @brief Swaps the contents with another delegate.
         * @param right The other delegate to swap with.
         *
         * \lang simp-chinese
         * @brief 与另一个 delegate 交换内容。
         * @param right 要交换的另一个 delegate。
         */
        void swap(move_only_delegate &right) noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the delegate's function signature.
         * @return The type info of Fx.
         *
         * \lang simp-chinese
         * @brief 返回 delegate 函数签名的类型信息。
         * @return Fx 的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &function_signature() const noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the delegate's return type.
         * @return The type info of the return type.
         *
         * \lang simp-chinese
         * @brief 返回 delegate 返回类型的类型信息。
         * @return 返回类型的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &return_type() const noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the class the callable is invoked on.
         * @return The type info of the class type.
         *
         * \lang simp-chinese
         * @brief 返回调用可调用对象时所用类的类型信息。
         * @return 类类型的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &which_belongs() const noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the stored callable.
         * @return The type info of the stored callable.
         *
         * \lang simp-chinese
         * @brief 返回所存储可调用对象的类型信息。
         * @return 所存储可调用对象的类型信息。
         */
        RAINY_NODISCARD const core::typeinfo &target_type() const noexcept;

        /**
         * \lang english
         * @brief Returns the method flags describing the stored callable.
         * @return The method flags, or method_flags::none if empty.
         *
         * \lang simp-chinese
         * @brief 返回描述所存储可调用对象的方法标志。
         * @return 方法标志；若为空则返回 method_flags::none。
         */
        RAINY_NODISCARD method_flags type() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable has the given method flag.
         * @param flag The flag to test.
         * @return true if the flag is set.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否具有给定的方法标志。
         * @param flag 要测试的标志。
         * @return 若已设置该标志则返回 true。
         */
        RAINY_NODISCARD bool has(method_flags flag) const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is const-qualified.
         * @return true if const-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为 const 限定。
         * @return 若为 const 限定则返回 true。
         */
        RAINY_NODISCARD bool is_const() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is noexcept-specified.
         * @return true if noexcept-specified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否指定了 noexcept。
         * @return 若指定了 noexcept 则返回 true。
         */
        RAINY_NODISCARD bool is_noexcept() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is lvalue-qualified.
         * @return true if lvalue-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为左值限定。
         * @return 若为左值限定则返回 true。
         */
        RAINY_NODISCARD bool is_invoke_for_lvalue() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is rvalue-qualified.
         * @return true if rvalue-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为右值限定。
         * @return 若为右值限定则返回 true。
         */
        RAINY_NODISCARD bool is_invoke_for_rvalue() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is static.
         * @return true if static.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为静态。
         * @return 若为静态则返回 true。
         */
        RAINY_NODISCARD bool is_static() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is volatile-qualified.
         * @return true if volatile-qualified.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为 volatile 限定。
         * @return 若为 volatile 限定则返回 true。
         */
        RAINY_NODISCARD bool is_volatile() const noexcept;

        /**
         * \lang english
         * @brief Checks whether the stored callable is a member function.
         * @return true if it is a member function.
         *
         * \lang simp-chinese
         * @brief 检查所存储可调用对象是否为成员函数。
         * @return 若为成员函数则返回 true。
         */
        RAINY_NODISCARD bool is_memfn() const noexcept;

        /**
         * \lang english
         * @brief Retrieves the stored callable as a pointer of the given type.
         * @tparam UFx The type to retrieve.
         * @return A pointer to the stored callable, or nullptr if not available.
         *
         * \lang simp-chinese
         * @brief 以给定类型的指针检索所存储的可调用对象。
         * @tparam UFx 要检索的类型。
         * @return 指向所存储可调用对象的指针；若不可用则返回 nullptr。
         */
        template <typename UFx>
        const UFx *target() const noexcept;

        /**
         * \lang english
         * @brief Retrieves the stored callable as a mutable pointer of the given type.
         * @tparam UFx The type to retrieve.
         * @return A pointer to the stored callable, or nullptr if not available.
         *
         * \lang simp-chinese
         * @brief 以给定类型的可修改指针检索所存储的可调用对象。
         * @tparam UFx 要检索的类型。
         * @return 指向所存储可调用对象的指针；若不可用则返回 nullptr。
         */
        template <typename UFx>
        UFx *target() noexcept;

        /**
         * \lang english
         * @brief Checks whether the delegate is non-empty.
         * @return true if a callable is stored.
         *
         * \lang simp-chinese
         * @brief 检查 delegate 是否非空。
         * @return 若存储了可调用对象则返回 true。
         */
        explicit operator bool() const noexcept;
    };

    /**
     * \lang english
     * @brief Deduces the move_only_delegate type from a callable.
     *
     * \lang simp-chinese
     * @brief 从可调用对象推导move_only_delegate类型。
     */
    template <typename Fx>
    move_only_delegate(Fx &&) -> move_only_delegate<Fx>;
}

#endif

#endif