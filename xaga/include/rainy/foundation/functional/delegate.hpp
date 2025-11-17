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
#ifndef RAINY_FOUNDATION_FUNCTIONAL_DELEGATE_HPP
#define RAINY_FOUNDATION_FUNCTIONAL_DELEGATE_HPP
#include <functional>
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::foundation::exceptions::runtime {
    class invalid_delegate : public runtime_error {
    public:
        using base = runtime_error;

        invalid_delegate(source loc = source::current()) : base("bad delegation call", loc) {
        }
    };

    RAINY_INLINE void throw_invalid_delegate(utility::source_location loc = utility::source_location::current()) {
        throw_exception(invalid_delegate(loc));
    }
}

namespace rainy::foundation::functional {
    template <typename Fx>
    class delegate;

    using core::method_flags;
}

namespace rainy::foundation::functional::implements {
    template <typename Rx, typename Class, typename... Args>
    struct invoker_accessor {
        virtual Rx invoke(Class *object, Args &&...args) = 0;
        RAINY_NODISCARD virtual std::uintptr_t target(const foundation::ctti::typeinfo &fx_sign) const noexcept = 0;
        RAINY_NODISCARD virtual const foundation::ctti::typeinfo &target_type() const noexcept = 0;
        virtual void destruct(bool local) noexcept = 0;
        virtual invoker_accessor *move(core::byte_t *soo_buffer) noexcept = 0;
        virtual invoker_accessor *copy(core::byte_t *soo_buffer) const = 0;
        virtual method_flags type() const noexcept = 0;
    };

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct invoker_accessor_impl : invoker_accessor<Rx, Class, Args...> {
    public:
        using fx_traits = type_traits::primary_types::function_traits<Fx>;
        using instance_t = Class;
        using base = invoker_accessor<Rx, Class, Args...>;

        template <typename UFx>
        invoker_accessor_impl(UFx &&fn) : fn(utility::forward<UFx>(fn)) {
        }

        Rx invoke(Class *object, Args &&...args) override {
            // 调用始终是无线程安全的，始终考虑执行的函数是否为线程安全
            if constexpr (type_traits::type_relations::is_void_v<Class>) {
                if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                    utility::invoke(utility::forward<Fx>(this->fn), utility::forward<Args>(args)...);
                } else {
                    return utility::invoke(utility::forward<Fx>(this->fn), utility::forward<Args>(args)...);
                }
            } else {
                if constexpr (fx_traits::is_invoke_for_lvalue) {
                    if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                        utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                        utility::forward<Args>(args)...);
                    } else {
                        return utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &>(*static_cast<instance_t *>(object)),
                                               utility::forward<Args>(args)...);
                    }
                } else if constexpr (fx_traits::is_invoke_for_rvalue) {
                    if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                        utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                        utility::forward<Args>(args)...);
                    } else {
                        return utility::invoke(utility::forward<Fx>(fn),
                                               static_cast<instance_t &&>(*static_cast<instance_t *>(object)),
                                               utility::forward<Args>(args)...);
                    }
                } else {
                    if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                        utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t *>(object), utility::forward<Args>(args)...);
                    } else {
                        return utility::invoke(utility::forward<Fx>(fn), static_cast<instance_t *>(object),
                                               utility::forward<Args>(args)...);
                    }
                }
            }
        }

        void destruct(bool local) noexcept override {
            this->~invoker_accessor_impl();
            if (!local) {
                delete this;
            }
        }

        method_flags type() const noexcept override {
            static constexpr method_flags flags = core::deduction_invoker_type<Fx, Args...>();
            return flags;
        }

        std::uintptr_t target(const foundation::ctti::typeinfo &fx_sign) const noexcept override {
            if (fx_sign == rainy_typeid(Fx)) {
                return reinterpret_cast<std::uintptr_t>(utility::addressof(fn));
            }
            return 0;
        }

        base *move(core::byte_t *soo_buffer) noexcept override {
            if constexpr (sizeof(invoker_accessor_impl) >= core::fn_obj_soo_buffer_size) {
                return nullptr;
            } else {
                return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), utility::move(fn));
            }
        }

        base *copy(core::byte_t *soo_buffer) const override {
            if constexpr (!type_traits::type_properties::is_copy_constructible_v<Fx>) {
                std::terminate();
            }
            if constexpr (sizeof(invoker_accessor_impl) >= core::fn_obj_soo_buffer_size) {
                return ::new invoker_accessor_impl(fn);
            } else {
                return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), fn);
            }
        }

        const foundation::ctti::typeinfo &target_type() const noexcept {
            return rainy_typeid(Fx);
        }

    private:
        Fx fn;
    };

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct get_ia_implement_type {
        using traits = type_traits::primary_types::function_traits<type_traits::other_trans::decay_t<Fx>>;

        using type = invoker_accessor_impl<type_traits::other_trans::decay_t<Fx>, Rx, Class, Args...>;
    };

    template <typename Fx, typename Rx, typename TypeList>
    class delegate_impl {};

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
            !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<UFx>, Delegate> &&
                type_traits::other_trans::conditional_t<
                    type_traits::type_relations::is_void_v<class_t>,
                    type_traits::type_properties::is_invocable_r<Rx, type_traits::other_trans::decay_t<UFx> &, Args...>,
                    type_traits::type_properties::is_invocable_r<Rx, type_traits::other_trans::decay_t<UFx> &, class_t,
                                                                 Args...>>::value,
            int>;

        template <typename UFx>
        friend class functional::delegate;

        delegate_impl() noexcept = default;

        ~delegate_impl() {
            reset();
        }

        template <typename Class, typename UFx = Fx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<result_type, UFx, Class, Args...>, int> = 0>
        Rx invoke(Class &&object, Args... args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_invalid_delegate();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            }
        }

        template <
            typename Class, typename UFx = Fx,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<result_type, UFx, Class, Args...> &&
                                                      type_traits::primary_types::is_member_function_pointer_v<Fx>,
                                                  int> = 0>
        Rx operator()(Class &&object, Args... args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_invalid_delegate();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(const_cast<class_t *>(utility::addressof(object)), utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = Fx, type_traits::other_trans::enable_if_t<
                                         type_traits::type_properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        Rx invoke(Args... args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_invalid_delegate();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = Fx, type_traits::other_trans::enable_if_t<
                                         type_traits::type_properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        Rx operator()(Args... args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_invalid_delegate();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            } else {
                return invoker_accessor()->invoke(nullptr, utility::forward<Args>(args)...);
            }
        }

        bool empty() const noexcept {
            return static_cast<bool>(!invoker_accessor_);
        }

        void reset() noexcept {
            if (!empty()) {
                invoker_accessor()->destruct(is_local());
                invoker_accessor_ = nullptr;
            }
        }

        void swap(delegate_impl &right) noexcept {
            if (!is_local() && !right.is_local()) {
                utility::swap(invoker_accessor_, right.invoker_accessor_);
            } else {
                delegate_impl temp;
                temp.move_from_other(utility::move(*this));
                reset_move(utility::move(right));
                right.move_from_other(utility::move(temp));
            }
        }

        template <typename UFx, enable_if_callable_t<UFx, delegate<Fx>> = 0>
        void rebind(UFx &&func) {
            if (!empty()) {
                invoker_accessor_->destruct(is_local());
            }
            using implemented_type = typename get_ia_implement_type<UFx, Rx, class_t, Args...>::type;
            if constexpr (sizeof(implemented_type) > core::fn_obj_soo_buffer_size) {
                invoker_accessor_ = ::new implemented_type(utility::forward<UFx>(func));
            } else {
                invoker_accessor_ =
                    utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage), utility::forward<UFx>(func));
            }
        }

        void rebind(delegate_impl &&right) {
            reset();
            move_from_other(right);
            right.reset();
        }

        void rebind(const delegate_impl &right) {
            reset();
            copy_from_other(right);
        }

        void move_from_other(delegate_impl &&right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            if (!right.empty()) {
                if (right.is_local()) {
                    invoker_accessor_ = right.invoker_accessor()->move(invoker_storage);
                    right.reset();
                } else {
                    invoker_accessor_ = right.invoker_accessor_;
                    right.invoker_accessor_ = nullptr;
                }
            }
        }

        void copy_from_other(const delegate_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            if (!right.empty()) {
                invoker_accessor_ = right.invoker_accessor()->copy(invoker_storage);
            }
        }

        const foundation::ctti::typeinfo &function_signature() const noexcept {
            return rainy_typeid(Fx);
        }

        const foundation::ctti::typeinfo &return_type() const noexcept {
            return rainy_typeid(Rx);
        }

        const foundation::ctti::typeinfo &which_belongs() const noexcept {
            return rainy_typeid(class_t);
        }

        const foundation::ctti::typeinfo &target_type() const noexcept {
            return invoker_accessor()->target_type();
        }

        method_flags type() const noexcept {
            if (empty()) {
                return method_flags::none;
            }
            return invoker_accessor()->type();
        }

        bool has(method_flags flag) const noexcept {
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
            auto ptr = reinterpret_cast<UFx *>(invoker_accessor()->target(rainy_typeid(UFx)));
            if (ptr) {
                return ptr;
            }
            return nullptr;
        }

        template <typename UFx>
        UFx *target() noexcept {
            if (empty()) {
                return nullptr;
            }
            auto ptr = reinterpret_cast<UFx *>(invoker_accessor()->target(rainy_typeid(UFx)));
            if (ptr) {
                return ptr;
            }
            return nullptr;
        }

        explicit operator bool() const noexcept {
            return !empty();
        }

    private:
        bool is_local() const noexcept {
            return static_cast<const void *>(invoker_accessor_) == reinterpret_cast<const void *>(invoker_storage);
        }

        implements::invoker_accessor<Rx, class_t, Args...> *invoker_accessor() const {
            return invoker_accessor_;
        }

        alignas(std::max_align_t) core::byte_t invoker_storage[core::fn_obj_soo_buffer_size]{};
        implements::invoker_accessor<Rx, class_t, Args...> *invoker_accessor_{nullptr};
    };

    template <typename Fx>
    struct get_delegate_impl {
        using type = implements::delegate_impl<Fx, typename type_traits::primary_types::function_traits<Fx>::return_type,
                                               typename type_traits::other_trans::tuple_like_to_type_list<
                                                   typename type_traits::primary_types::function_traits<Fx>::tuple_like_type>::type>;
    };
}

namespace rainy::foundation::functional {
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
            this->move_from_other(utility::move(right));
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

#endif