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
#ifndef RAINY_FOUNDATION_FUNCTION_POINTER_HPP
#define RAINY_FOUNDATION_FUNCTION_POINTER_HPP
#include <functional>
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::foundation::functional {
    template <typename Fx>
    struct function_pointer;
}

namespace rainy::foundation::functional::internals {
    template <typename Fx,typename ProtoType>
    struct as_helper {};

    template <typename ProtoType,typename Rx,typename...Args>
    struct as_helper<Rx(Args...), ProtoType> {
        static constexpr bool is_invocable_v = type_traits::type_properties::is_invocable_r_v<Rx, ProtoType, Args...>;
    };

    template <typename ProtoType, typename Rx, typename... Args>
    struct as_helper<Rx(Args...,...), ProtoType> {
        static constexpr bool is_invocable_v = type_traits::type_properties::is_invocable_r_v<Rx, ProtoType, Args...>;
    };
}

namespace rainy::foundation::functional::internals {
    template <typename PointerType>
    class function_pointer_common {
    public:
        using pointer = PointerType;

        static constexpr std::size_t arity = type_traits::primary_types::function_traits<PointerType>::arity;

        constexpr function_pointer_common() : invoker(nullptr) {
        }

        constexpr explicit function_pointer_common(pointer function_address) : invoker(function_address) {
        }

        constexpr function_pointer_common(const function_pointer_common &) noexcept = default;
        constexpr function_pointer_common(function_pointer_common &&) noexcept = default;
        RAINY_CONSTEXPR20 ~function_pointer_common() = default;

        RAINY_NODISCARD constexpr pointer get() const noexcept {
            return invoker;
        }
        RAINY_NODISCARD explicit operator pointer() const noexcept {
            return invoker;
        }

        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return !static_cast<bool>(invoker);
        }

        explicit operator bool() const noexcept {
            return empty();
        }

        constexpr void clear() noexcept {
            invoker = nullptr;
        }
        constexpr void swap(function_pointer_common &other) noexcept {
            std::swap(invoker, other.invoker);
        }

        RAINY_NODISCARD constexpr bool equal(const function_pointer_common &other) const noexcept {
            return invoker == other.invoker;
        }
        RAINY_NODISCARD constexpr bool not_equal(const function_pointer_common &other) const noexcept {
            return invoker != other.invoker;
        }

        friend constexpr bool operator==(const function_pointer_common &left, const function_pointer_common &right) noexcept {
            return left.equal(right);
        }
        friend constexpr bool operator!=(const function_pointer_common &left, const function_pointer_common &right) noexcept {
            return left.not_equal(right);
        }

        RAINY_NODISCARD constexpr rtti::typeinfo typeinfo() const noexcept {
            return rainy_typeid(pointer);
        }

        friend std::ostream &operator<<(std::ostream &ostream, const function_pointer_common &right) {
            ostream << right.typeinfo().name() << ' ' << right.invoker;
            return ostream;
        }

        constexpr function_pointer_common &assign(pointer function_address) noexcept {
            this->invoker = function_address;
            return *this;
        }

    protected:
        pointer invoker;
    };

    template <typename Signature>
    class function_pointer_base;

    template <typename Rx, typename... Args>
    class function_pointer_base<Rx(Args...)> : public function_pointer_common<Rx (*)(Args...)> {
    public:
        using base = function_pointer_common<Rx (*)(Args...)>;
        using pointer = typename base::pointer;
        using base::operator=;

        constexpr function_pointer_base() = default;
        constexpr function_pointer_base(std::nullptr_t) : base(nullptr) {
        }
        constexpr explicit function_pointer_base(pointer function_address) : base(function_address) {
        }

        template <typename URx, typename... UArgs,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>,
                                                        int> = 0>
        constexpr function_pointer_base(URx (*function_address)(UArgs...)) : base(reinterpret_cast<pointer>(function_address)) {
        }    

        constexpr Rx invoke(Args... args) const {
            if (this->empty())
                system::exceptions::runtime::throw_runtime_error("Current pointer is null!");
            if constexpr (std::is_void_v<Rx>)
                this->invoker(args...);
            else
                return this->invoker(args...);
        }

        constexpr Rx operator()(Args... args) const {
            return invoke(args...);
        }

        template <typename Fx, type_traits::other_trans::enable_if_t<as_helper<Fx, Rx(Args...)>::is_invocable_v, int> = 0>
        constexpr function_pointer_base<Fx> &as() noexcept {
            return static_cast<function_pointer<Fx> &>(*this);
        }

        RAINY_NODISCARD std::function<Rx(Args...)> make_function_object() const {
            return this->invoker;
        }

        constexpr function_pointer_base &assign(std::nullptr_t) noexcept {
            this->clear();
            return *this;
        }

        constexpr function_pointer_base &operator=(std::nullptr_t) noexcept {
            return assign(nullptr);
        }
    };

    template <typename Rx, typename... Args>
    class function_pointer_base<Rx(Args..., ...)> : public function_pointer_common<Rx (*)(Args..., ...)> {
    public:
        using base = function_pointer_common<Rx (*)(Args..., ...)>;
        using pointer = typename base::pointer;

        constexpr function_pointer_base() = default;
        constexpr function_pointer_base(std::nullptr_t) : base(nullptr) {
        }
        constexpr explicit function_pointer_base(pointer function_address) : base(function_address) {
        }

        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs..., ...), Args...>, int> = 0>
        constexpr function_pointer_base(URx (*function_address)(UArgs..., ...)) : base(function_address) {
        }

        template <typename... UArgs>
        constexpr Rx invoke(Args... args, UArgs... uargs) const {
            if (this->empty()) {
                system::exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (type_traits::primary_types::is_void_v<Rx>) {
                this->invoker(args..., uargs...);
            } else {
                return this->invoker(args..., uargs...);
            }
        }

        template <typename... UArgs>
        constexpr Rx operator()(Args... args, UArgs... uargs) const {
            if (this->empty()) {
                system::exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (type_traits::primary_types::is_void_v<Rx>) {
                this->invoker(args..., uargs...);
            } else {
                return this->invoker(args..., uargs...);
            }
        }

        template <typename Fx, type_traits::other_trans::enable_if_t<as_helper<Fx, Rx(Args..., ...)>::is_invocable_v, int> = 0>
        function_pointer_base<Fx> &as() noexcept {
            return reinterpret_cast<function_pointer<Fx> &>(*this);
        }

        constexpr function_pointer_base &assign(std::nullptr_t) noexcept {
            this->clear();
            return *this;
        }

        constexpr function_pointer_base &operator=(std::nullptr_t) noexcept {
            return assign(nullptr);
        }
    };
}

namespace rainy::foundation::functional {
    /**
     * @brief 此模板允许用户创建一个具有类型安全的函数指针对象
     * @tparam Fx 函数类型
     *
     * @example 以下代码展示了如何创建一个函数指针对象，并调用它：
     * @code
     * int add(int a, int b) {
     *  return a + b;
     * function_pointer<int(int, int)> fptr = &add;
     * int result = fptr(1, 2); // result == 3
     * @endcode
     */
    template <typename Rx, typename... Args>
    struct function_pointer<Rx(Args...)> : public internals::function_pointer_base<Rx(Args...)> {
        using base = internals::function_pointer_base<Rx(Args...)>;
        using base::base;
        using base::operator=;

        using pointer = base::pointer;
        
        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        constexpr function_pointer reset(pointer function_address = nullptr) noexcept {
            pointer old_ptr = this->invoker;
            this->invoker = function_address;
            return function_pointer{old_ptr};
        }

        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer reset(URx (*function_address)(UArgs...) = nullptr) noexcept {
            pointer old_ptr = this->invoker;
            this->invoker = function_address;
            return function_pointer{old_ptr};
        }

        constexpr function_pointer &assign(pointer function_address) noexcept {
            this->invoker = function_address;
            return *this;
        }

        constexpr function_pointer &assign(std::nullptr_t) noexcept {
            this->clear();
            return *this;
        }

        constexpr function_pointer &operator=(pointer function_address) noexcept {
            return this->assign(function_address);
        }

        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer &operator=(URx (*function_address)(UArgs...)) noexcept {
            return this->assign(function_address);
        }
    };

    template <typename Rx, typename... Args>
    struct function_pointer<Rx(Args..., ...)> : public internals::function_pointer_base<Rx(Args..., ...)> {
        using base = internals::function_pointer_base<Rx(Args..., ...)>;
        using base::base;
        using base::operator=;
        
        using pointer = base::pointer;

        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        constexpr function_pointer reset(pointer function_address = nullptr) noexcept {
            pointer old_ptr = this->invoker;
            this->invoker = function_address;
            return function_pointer{old_ptr};
        }

        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs..., ...), Args...>, int> = 0>
        constexpr function_pointer reset(URx (*function_address)(UArgs..., ...) = nullptr) noexcept {
            pointer old_ptr = this->invoker;
            this->invoker = function_address;
            return function_pointer{old_ptr};
        }

        constexpr function_pointer &operator=(pointer function_address) noexcept {
            return this->assign(function_address);
        }

        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs..., ...), Args...>, int> = 0>
        constexpr function_pointer &operator=(URx (*function_address)(UArgs..., ...)) noexcept {
            return this->assign(function_address);
        }
    };

    /**
     * @brief 此模板允许用户创建一个具有类型安全的函数指针对象
     * @tparam Fx 函数指针类型
     *
     * @example 以下代码展示了如何创建一个函数指针对象，并调用它：
     * @code
     * int add(int a, int b) {
     *  return a + b;
     * function_pointer<int(*)(int, int)> fptr = &add;
     * int result = fptr(1, 2); // result == 3
     * @endcode
     */
    template <typename Rx, typename... Args>
    struct function_pointer<Rx (*)(Args...)> : public function_pointer<Rx(Args...)> {
        using base = function_pointer<Rx(Args...)>;
        using base::base;
        using base::operator=;
    };

    template <typename Rx, typename... Args>
    struct function_pointer<Rx (*)(Args..., ...)> : public function_pointer<Rx(Args..., ...)> {
        using base = function_pointer<Rx(Args..., ...)>;
        using base::base;
        using base::operator=;
    };

    // 在此设置CTAD引导
    template <typename Fx, type_traits::other_trans::enable_if_t<!type_traits::primary_types::is_variadic_function_v<Fx>, int> = 0>
    function_pointer(Fx) -> function_pointer<Fx>;

    template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_variadic_function_v<Fx>, int> = 0>
    function_pointer(Fx) -> function_pointer<type_traits::pointer_modify::remove_pointer_t<Fx>>;

    /**
     * @brief   创建一个函数指针对象
     * @tparam  Rx 函数指针期望的返回类型
     * @tparam  Args 函数形参列表
     * @param   ptr 函数地址
     * @return  函数指针对象
     */
    template <typename Rx, typename... Args>
    constexpr auto make_function_pointer(Rx (*ptr)(Args...)) noexcept -> function_pointer<Rx(Args...)> {
        return function_pointer<Rx(Args...)>(ptr);
    }

    /**
     * @brief   创建一个函数指针对象
     * @tparam  Rx 函数指针期望的返回类型
     * @tparam  Args 函数形参列表
     * @param   ptr 函数地址
     * @return  函数指针对象
     */
    template <typename Rx, typename... Args>
    constexpr auto make_function_pointer(Rx (*ptr)(Args..., ...)) noexcept -> function_pointer<Rx(Args..., ...)> {
        return function_pointer<Rx(Args..., ...)>(ptr);
    }
}

#endif
