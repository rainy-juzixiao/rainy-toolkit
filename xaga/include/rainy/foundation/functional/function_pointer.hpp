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
/**
* @file function_pointer.hpp
* @brief 此模块允许你创建一个具有类型安全的函数指针对象。
* @author rainy-juzixiao
*/
#ifndef RAINY_FOUNDATION_FUNCTIONAL_FUNCTION_POINTER_HPP
#define RAINY_FOUNDATION_FUNCTIONAL_FUNCTION_POINTER_HPP
#include <functional>
#include <ostream>
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/annotations/smf_control.hpp>

namespace rainy::foundation::functional {
    template <typename Fx>
    class function_pointer;

    using core::method_flags;
}

namespace rainy::foundation::functional::implements {
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
                    utility::invoke(this->pointer_, static_cast<class_t *>(object), utility::forward<UArgs>(args)...);
                } else {
                    return utility::invoke(this->pointer_, static_cast<class_t *>(object), utility::forward<UArgs>(args)...);
                }
            }
        }
    }

    template <typename Fx, typename Rx, typename Class, typename... Args>
    struct function_pointer_impl {
        using fx_traits = type_traits::primary_types::function_traits<Fx>;
        using paramlist = type_traits::other_trans::type_list<Args...>;
        using result_type = Rx;
        using function_type = Fx;
        using pointer = type_traits::other_trans::conditional_t<type_traits::primary_types::is_pointer_v<Fx>, Fx,
                                                                type_traits::pointer_modify::add_pointer_t<Fx>>;
        using class_t = Class;

        static constexpr bool is_variadic = type_traits::primary_types::is_variadic_function_v<Fx>;
        static constexpr bool is_member_function = type_traits::primary_types::is_member_function_pointer_v<Fx>;

        static constexpr std::size_t arity = sizeof...(Args);

        template <typename UFx, typename FunctionPtr>
        using enable_if_callable_t = type_traits::other_trans::enable_if_t<
            !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<UFx>, FunctionPtr> &&
                type_traits::type_relations::is_convertible_v<UFx, pointer>,
            int>;

        template <typename UTy>
        constexpr void assign_impl_(UTy &&right) noexcept {
            this->pointer_ = right;
        }

        template <typename Self>
        constexpr void construct_impl_(Self &&right) noexcept {
            this->pointer_ = right.pointer_;
        }

        constexpr void reset() noexcept {
            this->pointer_ = nullptr;
        }

        constexpr void rebind(function_pointer_impl &&right) {
            reset();
            move_from_other(right);
            right.reset();
        }

        constexpr void rebind(const function_pointer_impl &right) {
            reset();
            copy_from_other(right);
        }

        template <typename UFx, typename enable_if_callable_t<UFx, function_pointer<Fx>> = 0>
        constexpr void rebind(UFx &&pointer) {
            this->pointer_ = pointer;
        }

        constexpr void move_from_other(function_pointer_impl &&right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            this->pointer_ = right.pointer_;
        }

        constexpr void copy_from_other(const function_pointer_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            this->pointer_ = right.pointer_;
        }

        constexpr method_flags type() const noexcept {
            static constexpr method_flags flags = core::deduction_invoker_type<Fx, Args...>();
            return flags;
        }

        constexpr const foundation::ctti::typeinfo &target_type() const noexcept {
            return rainy_typeid(Fx);
        }

        /**
         * @brief 检查当前invoker是否为空指针
         * @return 如果指针为空，则为true，反之false
         */
        constexpr bool empty() const noexcept {
            return !static_cast<bool>(this->pointer_);
        }

        constexpr pointer get() const noexcept {
            return this->pointer_;
        }

        template <typename Class, typename UFx = pointer,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<result_type, UFx, Class, Args...> && is_member_function, int> = 0>
        constexpr result_type invoke(Class &&object, Args... args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                        utility::forward<Args>(args)...);
            } else {
                return invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                            utility::forward<Args>(args)...);
            }
        }

        template <
            typename Class, typename UFx = function_type, typename... UArgs,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<result_type, UFx, Class, Args...> &&
                                                      is_variadic && is_member_function,
                                                  int> = 0>
        constexpr result_type invoke_variadic(Class &&object, Args... args, UArgs &&...variadic_args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                        utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            } else {
                return fp_invoke_impl<class_t>(this->pointer_, const_cast<class_t *>(utility::addressof(object)),
                                               utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            }
        }

        template <typename Class, typename UFx = function_type,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<result_type, UFx, Class, Args...> && is_member_function, int> = 0>
        constexpr result_type operator()(Class &&object, Args... args) const {
            return invoke(utility::forward<Class>(object), utility::forward<Args>(args)...);
        }

        template <
            typename Class, typename UFx = function_type, typename... UArgs,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<result_type, UFx, Class, Args...> &&
                                                      is_member_function && is_variadic,
                                                  int> = 0>
        constexpr result_type operator()(Class &&object, Args... args, UArgs &&...variadic_args) const {
            return invoke_variadic(utility::forward<Class>(object), utility::forward<Args>(args)...,
                                   utility::forward<UArgs>(variadic_args)...);
        }

        template <
            typename UFx = function_type,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        constexpr result_type invoke(Args... args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...);
            } else {
                return fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<result_type, UFx, Args..., UArgs...> && is_variadic &&
                          !is_member_function,
                      int> = 0>
        constexpr result_type invoke_variadic(Args... args, UArgs &&...variadic_args) const {
            if (empty()) {
                foundation::exceptions::runtime::throw_nullpointer_exception();
            }
            if constexpr (rainy::type_traits::type_relations::is_void_v<Rx>) {
                fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...,
                                     utility::forward<UArgs>(variadic_args)...);
            } else {
                return fp_invoke_impl<void>(this->pointer_, nullptr, utility::forward<Args>(args)...,
                                            utility::forward<UArgs>(variadic_args)...);
            }
        }

        template <
            typename UFx = function_type,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<result_type, UFx, Args...>, int> = 0>
        constexpr result_type operator()(Args... args) const {
            if constexpr (type_traits::type_relations::is_void_v<result_type>) {
                invoke(utility::forward<Args>(args)...);
            } else {
                return invoke(utility::forward<Args>(args)...);
            }
        }

        template <typename UFx = function_type, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<result_type, UFx, Args...> && is_variadic, int> = 0>
        constexpr result_type operator()(Args... args, UArgs &&...variadic_args) const {
            if constexpr (type_traits::type_relations::is_void_v<result_type>) {
                invoke_variadic(utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            } else {
                return invoke_variadic(utility::forward<Args>(args)..., utility::forward<UArgs>(variadic_args)...);
            }
        }

        /**
         * @brief 将当前函数指针对象转换为另一种函数指针对象类型（确保类型安全）
         * @tparam Fx 目标函数指针签名类型（必须与当前函数指针签名类型保持兼容性）
         * @return 目标函数指针对象
         */
        template <typename UFx, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<Rx,UFx,Args...>, int> = 0>
        constexpr function_pointer<UFx> cast() noexcept {
            return function_pointer<UFx>(this->pointer_);
        }

        pointer pointer_;
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

        // 修改这里：使用 extract 来展开 type_list
        using type =
            typename extract<Fx, typename traits::return_type,
                             typename type_traits::primary_types::member_pointer_traits<Fx>::class_type,
                             typename type_traits::other_trans::tuple_like_to_type_list<typename traits::tuple_like_type>::type>::type;
    };
}

namespace rainy::foundation::functional {
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

        /**
         * @brief 检查当前invoker是否不为空指针
         * @return 如果指针为空，则为false，反之true
         */
        constexpr operator bool() const noexcept {
            return !this->empty();
        }

        constexpr operator pointer() const noexcept {
            return this->pointer_;
        }

        void swap(function_pointer& right) noexcept {
            std::swap(this->pointer_, right.pointer_);
        }

        friend void swap(function_pointer &left, function_pointer &right) noexcept {
            left.swap(right);
        }

        using base::empty;
        using base::get;
        using base::operator();
        using base::invoke;
        using base::invoke_variadic;
        using base::reset;
        using base::cast;
    };

    template <typename Fx>
    function_pointer(Fx &&) -> function_pointer<Fx>;
}

#endif
