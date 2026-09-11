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
/**
 * \lang english
 * @file function_pointer.hpp
 * @author rainy-juzixiao
 *
 * \lang simp-chinese
 * @brief 此模块允许你创建一个具有类型安全的函数指针对象。
 */
#ifndef RAINY_CORE_FUNCTIONAL_FUNCTION_POINTER_HPP
#define RAINY_CORE_FUNCTIONAL_FUNCTION_POINTER_HPP
#include <rainy/core/annotations/lifetime_annotation.hpp>
#include <rainy/core/annotations/smf_control.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/typeinfo.hpp>

namespace rainy::functional {
    template <typename Fx>
    class function_pointer;

    using core::method_flags;
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON

namespace rainy::functional::implements {
    template <typename Class, typename Pointer, typename... UArgs>
    RAINY_INLINE constexpr rain_fn fp_invoke_impl(Pointer pointer, Class *object, UArgs &&...args) -> decltype(auto) {
        using fx_traits = type_traits::primary_types::function_traits<Pointer>;
        using class_t = Class;
        using return_type = typename fx_traits::return_type;
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            if constexpr (type_traits::type_relations::is_void_v<return_type>) {
                utility::invoke(pointer, utility::forward<UArgs>(args)...);
            } else {
                return utility::invoke(pointer, utility::forward<UArgs>(args)...);
            }
        } else {
            if constexpr (fx_traits::is_invoke_for_lvalue) {
                if constexpr (type_traits::type_relations::is_void_v<return_type>) {
                    utility::invoke(pointer, static_cast<class_t &>(*static_cast<class_t *>(object)),
                                    utility::forward<UArgs>(args)...);
                } else {
                    return utility::invoke(pointer, static_cast<class_t &>(*static_cast<class_t *>(object)),
                                           utility::forward<UArgs>(args)...);
                }
            } else if constexpr (fx_traits::is_invoke_for_rvalue) {
                if constexpr (type_traits::type_relations::is_void_v<return_type>) {
                    utility::invoke(pointer, static_cast<class_t &&>(*static_cast<class_t *>(object)),
                                    utility::forward<UArgs>(args)...);
                } else {
                    return utility::invoke(pointer, static_cast<class_t &&>(*static_cast<class_t *>(object)),
                                           utility::forward<UArgs>(args)...);
                }
            } else {
                if constexpr (type_traits::type_relations::is_void_v<return_type>) {
                    utility::invoke(pointer, static_cast<class_t *>(object), utility::forward<UArgs>(args)...);
                } else {
                    return utility::invoke(pointer, static_cast<class_t *>(object), utility::forward<UArgs>(args)...);
                }
            }
        }
    }

    template <typename Rx, typename Callable, typename Class, typename... Args>
    struct is_invocable_fp_object
        : type_traits::logical_traits::disjunction<type_traits::properties::is_invocable_r<Rx, Callable, Class &, Args...>,
                                                   type_traits::properties::is_invocable_r<Rx, Callable, Class, Args...>> {};

    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_fp_object<Rx, Callable, void, Args...> : type_traits::properties::is_invocable_r<Rx, Callable, Args...> {};

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct function_pointer_impl {
        using fx_traits = type_traits::primary_types::function_traits<Fx>;
        using paramlist = type_traits::other_trans::type_list<Args...>;
        using result_type = Rx;
        using function_type = Fx;

        using pointer = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_pointer_v<Fx>, Fx,
            type_traits::other_trans::conditional_t<type_traits::primary_types::is_member_function_pointer_v<Fx>, Fx,
                                                    type_traits::modifers::add_pointer_t<Fx>>>;
        using class_t = Class;

        static constexpr bool is_variadic = type_traits::primary_types::is_variadic_function_v<Fx>;
        static constexpr bool is_member_function = type_traits::primary_types::is_member_function_pointer_v<Fx>;

        static constexpr std::size_t arity = sizeof...(Args);

        template <typename UFx, typename FunctionPtr>
        using enable_if_callable_t = type_traits::other_trans::enable_if_t<
            !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<UFx>, FunctionPtr> &&
                type_traits::type_relations::is_convertible_v<UFx, pointer>,
            int>;

        template <typename UTy>
        constexpr rain_fn assign_impl_(UTy &&right) noexcept -> void {
            this->pointer_ = right;
        }

        template <typename Self>
        constexpr rain_fn construct_impl_(Self &&right) noexcept -> void {
            this->pointer_ = right.pointer_;
        }

        constexpr rain_fn reset() noexcept -> void {
            this->pointer_ = nullptr;
        }

        constexpr rain_fn rebind(annotations::lifetime::move_from<function_pointer_impl> right) -> void {
            reset();
            move_from_other(right);
            right.reset();
        }

        constexpr rain_fn rebind(annotations::lifetime::read_only<function_pointer_impl> right) -> void {
            reset();
            copy_from_other(right);
        }

        template <typename UFx, enable_if_callable_t<UFx, function_pointer<Fx>> = 0>
        constexpr rain_fn rebind(UFx &&pointer) -> void {
            this->pointer_ = pointer;
        }

        constexpr rain_fn move_from_other(annotations::lifetime::move_from<function_pointer_impl> right) noexcept -> void {
            if (this == utility::addressof(right)) {
                return;
            }
            this->pointer_ = right.pointer_;
            right.pointer_ = nullptr;
        }

        constexpr rain_fn copy_from_other(annotations::lifetime::read_only<function_pointer_impl> right) -> void {
            if (this == utility::addressof(right)) {
                return;
            }
            this->pointer_ = right.pointer_;
        }

        constexpr rain_fn type() const noexcept -> method_flags {
            constexpr method_flags flags = core::deduction_invoker_type<Fx, Args...>();
            return flags;
        }

        constexpr rain_fn target_type() const noexcept -> annotations::lifetime::static_read_only<core::typeinfo> {
            return rainy_typeid(Fx);
        }

        constexpr rain_fn empty() const noexcept -> bool {
            return !static_cast<bool>(this->pointer_);
        }

        constexpr rain_fn get() const noexcept -> pointer {
            return this->pointer_;
        }

        template <typename Clazz, typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<
                      implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value && is_member_function, int> = 0>
        constexpr rain_fn invoke(Clazz &&object, Args... args) const -> result_type {
            if (empty()) {
                core::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                        utility::forward<Args>(args)...);
            } else {
                return fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                               utility::forward<Args>(args)...);
            }
        }

        template <typename Clazz, typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value &&
                                                            is_variadic && is_member_function,
                                                        int> = 0>
        constexpr rain_fn invoke_variadic(Clazz &&object, Args... args, UArgs &&...variadic_args) const -> result_type {
            if (empty()) {
                core::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                        utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            } else {
                return fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                               utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            }
        }

        template <typename Clazz, typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<
                      implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value && is_member_function, int> = 0>
        constexpr rain_fn operator()(Clazz &&object, Args... args) const->result_type {
            return invoke(utility::forward<Clazz>(object), utility::forward<Args>(args)...);
        }

        template <typename Clazz, typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value &&
                                                            is_member_function && is_variadic,
                                                        int> = 0>
        constexpr rain_fn operator()(Clazz &&object, Args... args, UArgs &&...variadic_args) const->result_type {
            return invoke_variadic(utility::forward<Clazz>(object), utility::forward<Args>(args)...,
                                   utility::forward<UArgs>(variadic_args)...);
        }

        template <typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        constexpr rain_fn invoke(Args... args) const -> result_type {
            if (empty()) {
                core::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...);
            } else {
                return fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...);
            }
        }

        template <
            typename UFx = function_type, typename... UArgs,
            type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args..., UArgs...> &&
                                                      is_variadic && !is_member_function,
                                                  int> = 0>
        constexpr rain_fn invoke_variadic(Args... args, UArgs &&...variadic_args) const -> result_type {
            if (empty()) {
                core::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...,
                                     utility::forward<UArgs>(variadic_args)...);
            } else {
                return fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...,
                                            utility::forward<UArgs>(variadic_args)...);
            }
        }

        template <typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        constexpr rain_fn operator()(Args... args) const->result_type {
            if constexpr (type_traits::type_relations::is_void_v<result_type>) {
                invoke(utility::forward<Args>(args)...);
            } else {
                return invoke(utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_invocable_r_v<result_type, UFx, Args...> && is_variadic, int> = 0>
        constexpr rain_fn operator()(Args... args, UArgs &&...variadic_args) const->result_type {
            if constexpr (type_traits::type_relations::is_void_v<result_type>) {
                invoke_variadic(utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            } else {
                return invoke_variadic(utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            }
        }

        pointer pointer_{};
    };

    template <typename Fx>
    struct get_fp_implement_type {
        using traits = type_traits::primary_types::function_traits<type_traits::other_trans::decay_t<Fx>>;

        template <typename UFx, typename Rx, typename Class, typename TypeList>
        struct extract {};

        template <typename UFx, typename Rx, typename Class, typename... Args>
        struct extract<UFx, Rx, Class, type_traits::other_trans::type_list<Args...>> {
            using type = function_pointer_impl<UFx, Rx, Class, Args...>;
        };

        using type = typename extract<Fx, typename traits::return_type,
                                      typename type_traits::primary_types::member_pointer_traits<Fx>::class_type,
                                      typename traits::argument_list>::type;
    };
}

namespace rainy::functional {
    template <typename Fx>
    class function_pointer : private annotations::smf_control::control<typename implements::get_fp_implement_type<Fx>::type> {
    public:
        using base = annotations::smf_control::control<typename implements::get_fp_implement_type<Fx>::type>;

        using result_type = typename base::result_type;
        using function_type = typename base::function_type;
        using pointer = typename base::pointer;

        constexpr function_pointer() noexcept = default;

        template <typename UFx, typename base::template enable_if_callable_t<UFx, function_pointer> = 0>
        constexpr function_pointer(UFx &&fn) {
            this->rebind(utility::forward<UFx>(fn));
        }

        constexpr operator bool() const noexcept {
            return !this->empty();
        }

        constexpr operator pointer() const noexcept {
            return this->pointer_;
        }

        void swap(function_pointer &right) noexcept {
            std::swap(this->pointer_, right.pointer_);
        }

        friend void swap(function_pointer &left, function_pointer &right) noexcept {
            left.swap(right);
        }

        using base::empty;
        using base::get;
        using base::target_type;
        using base::operator();
        using base::copy_from_other;
        using base::invoke;
        using base::invoke_variadic;
        using base::move_from_other;
        using base::reset;

        constexpr core::method_flags type() const noexcept {
            return base::type();
        }
    };

    template <typename Fx>
    function_pointer(Fx &&) -> function_pointer<Fx>;
}

#else

namespace rainy::functional::implements {
    /**
     * \lang english
     * @brief Invokes a stored function or member function pointer with the given
     *        object and arguments.
     *
     *  If Class is void the pointer is treated as a free function pointer and the
     *  object argument is ignored. Otherwise the object is cast according to the
     *  ref-qualifier of the member function (lvalue, rvalue, or pointer).
     *
     * @tparam Class The class type the member function belongs to, or void for a
     *               free function pointer.
     * @tparam Pointer The pointer type.
     * @tparam UArgs The argument types to forward.
     * @param pointer The stored pointer.
     * @param object The object to invoke on, or nullptr for a free function pointer.
     * @param args The arguments to forward.
     * @return The result of the invocation.
     *
     * \lang simp-chinese
     * @brief 使用给定的对象与参数调用所存储的函数指针或成员函数指针。
     *
     *  若 Class 为 void，则该指针被视为自由函数指针，对象参数被忽略。否则，
     *  对象将依据成员函数的引用限定（左值、右值或指针）进行转换。
     *
     * @tparam Class 成员函数所属的类类型；对于自由函数指针则为 void。
     * @tparam Pointer 指针类型。
     * @tparam UArgs 要转发的参数类型。
     * @param pointer 所存储的指针。
     * @param object 调用所作用的对象；对于自由函数指针为 nullptr。
     * @param args 要转发的参数。
     * @return 调用结果。
     */
    template <typename Class, typename Pointer, typename... UArgs>
    RAINY_INLINE constexpr rain_fn fp_invoke_impl(Pointer pointer, Class *object, UArgs &&...args) -> decltype(auto);

    template <typename Rx, typename Callable, typename Class, typename... Args>
    struct is_invocable_fp_object;

    template <typename Rx, typename Callable, typename... Args>
    struct is_invocable_fp_object<Rx, Callable, void, Args...>;

    /**
     * \lang english
     * @brief Implementation base for function_pointer.
     *
     *  Stores the raw pointer and provides the invocability helpers, the rebind /
     *  reset / move_from_other / copy_from_other operations, the type and
     *  target_type queries, the empty and get accessors, and the invoke / operator()
     *  overloads.
     *
     * @tparam Fx The function signature type.
     * @tparam Rx The return type.
     * @tparam Class The class type for member function pointers, or void.
     * @tparam Args The argument types.
     *
     * \lang simp-chinese
     * @brief function_pointer 的实现基类。
     *
     *  存储原始指针，并提供可调用性辅助、rebind / reset / move_from_other /
     *  copy_from_other 操作、type 与 target_type 查询、empty 与 get 访问器，以及
     *  invoke / operator() 重载。
     *
     * @tparam Fx 函数签名类型。
     * @tparam Rx 返回类型。
     * @tparam Class 成员函数指针的类类型，或 void。
     * @tparam Args 参数类型。
     */
    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct function_pointer_impl {
        using fx_traits = type_traits::primary_types::function_traits<Fx>;
        using paramlist = type_traits::other_trans::type_list<Args...>;
        using result_type = Rx;
        using function_type = Fx;

        using pointer = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_pointer_v<Fx>, Fx,
            type_traits::other_trans::conditional_t<type_traits::primary_types::is_member_function_pointer_v<Fx>, Fx,
                                                    type_traits::modifers::add_pointer_t<Fx>>>;
        using class_t = Class;

        static constexpr bool is_variadic = type_traits::primary_types::is_variadic_function_v<Fx>;
        static constexpr bool is_member_function = type_traits::primary_types::is_member_function_pointer_v<Fx>;

        static constexpr std::size_t arity = sizeof...(Args);

        template <typename UFx, typename FunctionPtr>
        using enable_if_callable_t = type_traits::other_trans::enable_if_t<
            !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<UFx>, FunctionPtr> &&
                type_traits::type_relations::is_convertible_v<UFx, pointer>,
            int>;

        template <typename UTy>
        constexpr rain_fn assign_impl_(UTy &&right) noexcept -> void;

        template <typename Self>
        constexpr rain_fn construct_impl_(Self &&right) noexcept -> void;

        /**
         * \lang english
         * @brief Resets the stored pointer to null.
         *
         * \lang simp-chinese
         * @brief 将所存储的指针置空。
         */
        constexpr rain_fn reset() noexcept -> void;

        constexpr rain_fn rebind(annotations::lifetime::move_from<function_pointer_impl> right) -> void;

        constexpr rain_fn rebind(annotations::lifetime::read_only<function_pointer_impl> right) -> void;

        template <typename UFx, enable_if_callable_t<UFx, function_pointer<Fx>> = 0>
        constexpr rain_fn rebind(UFx &&pointer) -> void;

        constexpr rain_fn move_from_other(annotations::lifetime::move_from<function_pointer_impl> right) noexcept -> void;

        constexpr rain_fn copy_from_other(annotations::lifetime::read_only<function_pointer_impl> right) -> void;

        /**
         * \lang english
         * @brief Returns the flags describing the stored function type.
         * @return The method flags of the stored pointer type.
         *
         * \lang simp-chinese
         * @brief 返回描述所存储函数类型的标志。
         * @return 所存储指针类型的方法标志。
         */
        constexpr rain_fn type() const noexcept -> method_flags;

        /**
         * \lang english
         * @brief Returns the type info of the stored pointer type.
         * @return The type info of Fx.
         *
         * \lang simp-chinese
         * @brief 返回所存储指针类型的类型信息。
         * @return Fx 的类型信息。
         */
        constexpr rain_fn target_type() const noexcept -> annotations::lifetime::static_read_only<core::typeinfo>;

        /**
         * \lang english
         * @brief Checks whether the current invoker is a null pointer.
         * @return true if the pointer is null, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查当前invoker是否为空指针
         * @return 如果指针为空，则为true，反之false
         */
        constexpr rain_fn empty() const noexcept -> bool;

        /**
         * \lang english
         * @brief Returns the stored pointer.
         * @return The stored pointer.
         *
         * \lang simp-chinese
         * @brief 返回所存储的指针。
         * @return 所存储的指针。
         */
        constexpr rain_fn get() const noexcept -> pointer;

        /**
         * \lang english
         * @brief Invokes the stored member function pointer on the given object.
         *
         * @tparam Clazz The object type.
         * @tparam UFx The function signature used for the invocability check.
         * @param object The object to invoke on.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: member function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的成员函数指针。
         *
         * @tparam Clazz 对象类型。
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param object 调用所作用的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅成员函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <typename Clazz, typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<
                      implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value && is_member_function, int> = 0>
        constexpr rain_fn invoke(Clazz &&object, Args... args) const -> result_type;

        /**
         * \lang english
         * @brief Invokes the stored variadic member function pointer on the given object.
         *
         * @tparam Clazz The object type.
         * @tparam UFx The function signature used for the invocability check.
         * @tparam UArgs The types of the variadic arguments.
         * @param object The object to invoke on.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic member function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的可变参数成员函数指针。
         *
         * @tparam Clazz 对象类型。
         * @tparam UFx 用于可调用性检查的函数签名。
         * @tparam UArgs 可变参数的类型。
         * @param object 调用所作用的对象。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数成员函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <typename Clazz, typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value &&
                                                            is_variadic && is_member_function,
                                                        int> = 0>
        constexpr rain_fn invoke_variadic(Clazz &&object, Args... args, UArgs &&...variadic_args) const -> result_type;

        /**
         * \lang english
         * @brief Invokes the stored member function pointer on the given object
         *        (operator form).
         *
         * @tparam Clazz The object type.
         * @tparam UFx The function signature used for the invocability check.
         * @param object The object to invoke on.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: member function pointers only.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的成员函数指针（运算符形式）。
         *
         * @tparam Clazz 对象类型。
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param object 调用所作用的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅成员函数指针。
         */
        template <typename Clazz, typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<
                      implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value && is_member_function, int> = 0>
        constexpr rain_fn operator()(Clazz &&object, Args... args) const->result_type;

        /**
         * \lang english
         * @brief Invokes the stored variadic member function pointer on the given
         *        object (operator form).
         *
         * @tparam Clazz The object type.
         * @tparam UFx The function signature used for the invocability check.
         * @tparam UArgs The types of the variadic arguments.
         * @param object The object to invoke on.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic member function pointers only.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的可变参数成员函数指针（运算符形式）。
         *
         * @tparam Clazz 对象类型。
         * @tparam UFx 用于可调用性检查的函数签名。
         * @tparam UArgs 可变参数的类型。
         * @param object 调用所作用的对象。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数成员函数指针。
         */
        template <typename Clazz, typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<implements::is_invocable_fp_object<result_type, UFx, Class, Args...>::value &&
                                                            is_member_function && is_variadic,
                                                        int> = 0>
        constexpr rain_fn operator()(Clazz &&object, Args... args, UArgs &&...variadic_args) const->result_type;

        /**
         * \lang english
         * @brief Invokes the stored free function pointer without an object.
         *
         * @tparam UFx The function signature used for the invocability check.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: free function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的自由函数指针。
         *
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅自由函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        constexpr rain_fn invoke(Args... args) const -> result_type;

        /**
         * \lang english
         * @brief Invokes the stored variadic free function pointer without an object.
         *
         * @tparam UFx The function signature used for the invocability check.
         * @tparam UArgs The types of the variadic arguments.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic free function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的可变参数自由函数指针。
         *
         * @tparam UFx 用于可调用性检查的函数签名。
         * @tparam UArgs 可变参数的类型。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数自由函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <
            typename UFx = function_type, typename... UArgs,
            type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args..., UArgs...> &&
                                                      is_variadic && !is_member_function,
                                                  int> = 0>
        constexpr rain_fn invoke_variadic(Args... args, UArgs &&...variadic_args) const -> result_type;

        /**
         * \lang english
         * @brief Invokes the stored free function pointer without an object
         *        (operator form).
         *
         * @tparam UFx The function signature used for the invocability check.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: free function pointers only.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的自由函数指针（运算符形式）。
         *
         * @tparam UFx 用于可调用性检查的函数签名。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅自由函数指针。
         */
        template <typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        constexpr rain_fn operator()(Args... args) const->result_type;

        /**
         * \lang english
         * @brief Invokes the stored variadic free function pointer without an object
         *        (operator form).
         *
         * @tparam UFx The function signature used for the invocability check.
         * @tparam UArgs The types of the variadic arguments.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic free function pointers only.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的可变参数自由函数指针（运算符形式）。
         *
         * @tparam UFx 用于可调用性检查的函数签名。
         * @tparam UArgs 可变参数的类型。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数自由函数指针。
         */
        template <typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::properties::is_invocable_r_v<result_type, UFx, Args...> && is_variadic, int> = 0>
        constexpr rain_fn operator()(Args... args, UArgs &&...variadic_args) const->result_type;

        /**
         * \lang english
         * @brief The stored pointer.
         *
         * \lang simp-chinese
         * @brief 所存储的指针。
         */
        pointer pointer_{};
    };

    /**
     * \lang english
     * @brief Maps a function signature Fx to the corresponding function_pointer_impl.
     *
     * @tparam Fx The function signature type.
     *
     * \lang simp-chinese
     * @brief 将函数签名 Fx 映射到对应的 function_pointer_impl。
     *
     * @tparam Fx 函数签名类型。
     */
    template <typename Fx>
    struct get_fp_implement_type {
        using traits = type_traits::primary_types::function_traits<type_traits::other_trans::decay_t<Fx>>;

        template <typename UFx, typename Rx, typename Class, typename TypeList>
        struct extract {};

        template <typename UFx, typename Rx, typename Class, typename... Args>
        struct extract<UFx, Rx, Class, type_traits::other_trans::type_list<Args...>> {
            using type = function_pointer_impl<UFx, Rx, Class, Args...>;
        };

        using type = typename extract<Fx, typename traits::return_type,
                                      typename type_traits::primary_types::member_pointer_traits<Fx>::class_type,
                                      typename traits::argument_list>::type;
    };
}

namespace rainy::functional {
    /**
     * \lang english
     * @brief A type-safe wrapper around a function pointer or member function pointer.
     *
     *  Stores a pointer whose signature matches Fx and exposes invocation through
     *  operator() and invoke; invoking an empty pointer throws nullpointer_exception.
     *
     * @tparam Fx The function signature of the pointer.
     *
     * \lang simp-chinese
     * @brief 函数指针或成员函数指针的类型安全包装器。
     *
     *  存储签名与Fx匹配的指针，并通过operator()和invoke暴露调用能力；
     *  调用空的指针会抛出nullpointer_exception。
     *
     * @tparam Fx 指针的函数签名。
     */
    template <typename Fx>
    class function_pointer {
    public:
        /**
         * \lang english
         * @brief The return type of the stored pointer.
         *
         * \lang simp-chinese
         * @brief 所存储指针的返回类型。
         */
        using result_type = typename type_traits::primary_types::function_traits<Fx>::return_type;

        /**
         * \lang english
         * @brief The function signature type of the stored pointer.
         *
         * \lang simp-chinese
         * @brief 所存储指针的函数签名类型。
         */
        using function_type = Fx;

        /**
         * \lang english
         * @brief The raw pointer type stored by this wrapper.
         *
         * \lang simp-chinese
         * @brief 此包装器所存储的原始指针类型。
         */
        using pointer = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_pointer_v<Fx>, Fx,
            type_traits::other_trans::conditional_t<type_traits::primary_types::is_member_function_pointer_v<Fx>, Fx,
                                                    type_traits::modifers::add_pointer_t<Fx>>>;

        /**
         * \lang english
         * @brief Constructs an empty function_pointer.
         *
         * \lang simp-chinese
         * @brief 构造一个空的function_pointer。
         */
        constexpr function_pointer() noexcept = default;

        /**
         * \lang english
         * @brief Constructs a function_pointer from any pointer convertible to the stored pointer type.
         * @tparam UFx The type of the pointer.
         * @param fn The pointer to store.
         *
         * \lang simp-chinese
         * @brief 从任何可转换为所存储指针类型的指针构造function_pointer。
         * @tparam UFx 指针的类型。
         * @param fn 要存储的指针。
         */
        template <typename UFx>
        constexpr function_pointer(UFx &&fn);

        /**
         * \lang english
         * @brief Checks whether the stored pointer is non-null.
         * @return true if the pointer is non-null, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查当前invoker是否不为空指针
         * @return 如果指针为空，则为false，反之true
         */
        constexpr operator bool() const noexcept;

        /**
         * \lang english
         * @brief Converts to the underlying pointer type.
         * @return The stored pointer.
         *
         * \lang simp-chinese
         * @brief 转换为底层指针类型。
         * @return 存储的指针。
         */
        constexpr operator pointer() const noexcept;

        /**
         * \lang english
         * @brief Exchanges the stored pointers of two function_pointer objects.
         * @param right The object to swap with.
         *
         * \lang simp-chinese
         * @brief 交换两个function_pointer对象存储的指针。
         * @param right 要与之交换的对象。
         */
        void swap(function_pointer &right) noexcept;

        /**
         * \lang english
         * @brief Exchanges the stored pointers of two function_pointer objects.
         * @param left The first object.
         * @param right The second object.
         *
         * \lang simp-chinese
         * @brief 交换两个function_pointer对象存储的指针。
         * @param left 第一个对象。
         * @param right 第二个对象。
         */
        friend void swap(function_pointer &left, function_pointer &right) noexcept;

        /**
         * \lang english
         * @brief Checks whether the current invoker is a null pointer.
         * @return true if the pointer is null, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查当前invoker是否为空指针
         * @return 如果指针为空，则为true，反之false
         */
        constexpr bool empty() const noexcept;

        /**
         * \lang english
         * @brief Returns the stored pointer.
         * @return The stored pointer.
         *
         * \lang simp-chinese
         * @brief 返回所存储的指针。
         * @return 所存储的指针。
         */
        constexpr pointer get() const noexcept;

        /**
         * \lang english
         * @brief Resets the stored pointer to null.
         *
         * \lang simp-chinese
         * @brief 将所存储的指针置空。
         */
        constexpr void reset() noexcept;

        /**
         * \lang english
         * @brief Returns the type info of the stored pointer type.
         * @return The type info of Fx.
         *
         * \lang simp-chinese
         * @brief 返回所存储指针类型的类型信息。
         * @return Fx 的类型信息。
         */
        constexpr annotations::lifetime::static_read_only<core::typeinfo> target_type() const noexcept;

        /**
         * \lang english
         * @brief Returns the flags describing the stored function type.
         * @return The method flags of the stored pointer type.
         *
         * \lang simp-chinese
         * @brief 返回描述所存储函数类型的标志。
         * @return 所存储指针类型的方法标志。
         */
        constexpr core::method_flags type() const noexcept;

        /**
         * \lang english
         * @brief Invokes the stored member function pointer on the given object.
         *
         * @tparam Clazz The object type.
         * @param object The object to invoke on.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: member function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的成员函数指针。
         *
         * @tparam Clazz 对象类型。
         * @param object 调用所作用的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅成员函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <typename Clazz>
        constexpr result_type invoke(Clazz &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const;

        /**
         * \lang english
         * @brief Invokes the stored variadic member function pointer on the given object.
         *
         * @tparam Clazz The object type.
         * @tparam UArgs The types of the variadic arguments.
         * @param object The object to invoke on.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic member function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的可变参数成员函数指针。
         *
         * @tparam Clazz 对象类型。
         * @tparam UArgs 可变参数的类型。
         * @param object 调用所作用的对象。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数成员函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <typename Clazz, typename... UArgs>
        constexpr result_type invoke_variadic(Clazz &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args,
                                              UArgs &&...variadic_args) const;

        /**
         * \lang english
         * @brief Invokes the stored member function pointer on the given object
         *        (operator form).
         *
         * @tparam Clazz The object type.
         * @param object The object to invoke on.
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: member function pointers only.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的成员函数指针（运算符形式）。
         *
         * @tparam Clazz 对象类型。
         * @param object 调用所作用的对象。
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅成员函数指针。
         */
        template <typename Clazz>
        constexpr result_type operator()(Clazz &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const;

        /**
         * \lang english
         * @brief Invokes the stored variadic member function pointer on the given
         *        object (operator form).
         *
         * @tparam Clazz The object type.
         * @tparam UArgs The types of the variadic arguments.
         * @param object The object to invoke on.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic member function pointers only.
         *
         * \lang simp-chinese
         * @brief 在给定对象上调用所存储的可变参数成员函数指针（运算符形式）。
         *
         * @tparam Clazz 对象类型。
         * @tparam UArgs 可变参数的类型。
         * @param object 调用所作用的对象。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数成员函数指针。
         */
        template <typename Clazz, typename... UArgs>
        constexpr result_type operator()(Clazz &&object, typename type_traits::primary_types::function_traits<Fx>::argument_types... args,
                                         UArgs &&...variadic_args) const;

        /**
         * \lang english
         * @brief Invokes the stored free function pointer without an object.
         *
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: free function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的自由函数指针。
         *
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅自由函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        constexpr result_type invoke(typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const;

        /**
         * \lang english
         * @brief Invokes the stored variadic free function pointer without an object.
         *
         * @tparam UArgs The types of the variadic arguments.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic free function pointers only.
         * @note Throws nullpointer_exception if the pointer is null.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的可变参数自由函数指针。
         *
         * @tparam UArgs 可变参数的类型。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数自由函数指针。
         * @note 若指针为空则抛出 nullpointer_exception。
         */
        template <typename... UArgs>
        constexpr result_type invoke_variadic(typename type_traits::primary_types::function_traits<Fx>::argument_types... args,
                                              UArgs &&...variadic_args) const;

        /**
         * \lang english
         * @brief Invokes the stored free function pointer without an object
         *        (operator form).
         *
         * @param args The arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: free function pointers only.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的自由函数指针（运算符形式）。
         *
         * @param args 要转发的参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅自由函数指针。
         */
        constexpr result_type operator()(typename type_traits::primary_types::function_traits<Fx>::argument_types... args) const;

        /**
         * \lang english
         * @brief Invokes the stored variadic free function pointer without an object
         *        (operator form).
         *
         * @tparam UArgs The types of the variadic arguments.
         * @param args The fixed arguments to forward.
         * @param variadic_args The variadic arguments to forward.
         * @return The result of the invocation.
         *
         * @note Applies to: variadic free function pointers only.
         *
         * \lang simp-chinese
         * @brief 在不提供对象的情况下调用所存储的可变参数自由函数指针（运算符形式）。
         *
         * @tparam UArgs 可变参数的类型。
         * @param args 要转发的固定参数。
         * @param variadic_args 要转发的可变参数。
         * @return 调用结果。
         *
         * @note 适用类型：仅可变参数自由函数指针。
         */
        template <typename... UArgs>
        constexpr result_type operator()(typename type_traits::primary_types::function_traits<Fx>::argument_types... args,
                                         UArgs &&...variadic_args) const;

        /**
         * \lang english
         * @brief The stored pointer.
         *
         * \lang simp-chinese
         * @brief 所存储的指针。
         */
        pointer pointer_{};
    };

    /**
     * \lang english
     * @brief Deduces the function_pointer type from a pointer.
     *
     * \lang simp-chinese
     * @brief 从指针推导function_pointer类型。
     */
    template <typename Fx>
    function_pointer(Fx &&) -> function_pointer<Fx>;
}

#endif

#endif