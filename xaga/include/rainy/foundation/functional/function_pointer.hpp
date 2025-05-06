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

namespace rainy::foundation::functional {
    template <typename Fx>
    struct function_pointer;
}

namespace rainy::foundation::functional::implements {
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

namespace rainy::foundation::functional::implements {
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

        /**
         * @brief 检查当前invoker是否为空指针
         * @return 如果指针为空，则为true，反之false
         */
        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return !static_cast<bool>(invoker);
        }

        /**
         * @brief 检查当前invoker是否不为空指针
         * @return 如果指针为空，则为false，反之true
         */
        explicit operator bool() const noexcept {
            return !empty();
        }

        /**
         * @brief 将当前invoker设置为空指针
         */
        constexpr void clear() noexcept {
            invoker = nullptr;
        }

        /**
         * @brief 将对象other的invoker进行交换给当前对象
         * @param other 待交换的对象
         */
        constexpr void swap(function_pointer_common &other) noexcept {
            std::swap(invoker, other.invoker);
        }

        /**
         * @brief 检查当前对象持有的invoker是否与另一个对象一致
         * @param other 待检查的对象
         * @return 如果一致，是true，反之false
         */
        RAINY_NODISCARD constexpr bool equal(const function_pointer_common &other) const noexcept {
            return invoker == other.invoker;
        }

        /**
         * @brief 检查当前对象持有的invoker是否与另一个对象不相同
         * @param other 待检查的对象
         * @return 如果一致，是true，反之false
         */
        RAINY_NODISCARD constexpr bool not_equal(const function_pointer_common &other) const noexcept {
            return invoker != other.invoker;
        }

        /**
         * @brief 检查当前对象持有的invoker是否与另一个对象一致
         * @param left 待检查的对象
         * @param right 待检查的对象
         * @return 如果一致，是true，反之false
         */
        friend constexpr bool operator==(const function_pointer_common &left, const function_pointer_common &right) noexcept {
            return left.equal(right);
        }

        /**
         * @brief 检查当前对象持有的invoker是否与另一个对象不相同
         * @param left 待检查的对象
         * @param right 待检查的对象
         * @return 如果一致，是true，反之false
         */
        friend constexpr bool operator!=(const function_pointer_common &left, const function_pointer_common &right) noexcept {
            return left.not_equal(right);
        }

        /**
         * @brief 获取invoker的函数签名的类型信息，以静态方式
         * @return invoker函数签名类型信息
         */
        RAINY_NODISCARD static constexpr rtti::typeinfo typeinfo() noexcept {
            return rainy_typeid(pointer);
        }

        /**
         * @brief 将类型信息与函数指针地址一同输出的ostream中
         * @param ostream 用于输出的ostream对象
         * @param right 用于输出的函数指针对象
         * @return 返回ostream自身引用
         */
        friend std::ostream &operator<<(std::ostream &ostream, const function_pointer_common &right) {
            ostream << right.typeinfo().name() << ' ' << right.invoker;
            return ostream;
        }

        /**
         * @brief 将函数指针地址赋值给当前函数指针对象
         * @param function_address 函数指针地址
         * @return 返回当前this对象引用
         */
        constexpr function_pointer_common &assign(pointer function_address) noexcept {
            this->invoker = function_address;
            return *this;
        }

        constexpr function_pointer_common &assign(std::nullptr_t) noexcept {
            this->clear();
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
        
        /**
         * @brief 调用当前函数指针对象
         * @param args 用于传入的参数
         * @return 如果Rx为void，则无返回值。如果不是，则可以接收返回值
         */
        constexpr Rx invoke(Args... args) const {
            if (this->empty()) {
                exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (std::is_void_v<Rx>) {
                this->invoker(args...);
                return;
            } else {
                return this->invoker(args...);
            }
        }

        /**
         * @brief 调用当前函数指针对象
         * @param args 用于传入的参数
         * @return 如果Rx为void，则无返回值。如果不是，则可以接收返回值
         */
        constexpr Rx operator()(Args... args) const {
            return invoke(args...);
        }

        /**
         * @brief 将当前函数指针对象转换为另一种函数指针对象类型的引用（确保类型安全）
         * @tparam Fx 目标函数指针签名类型（必须与当前函数指针签名类型保持兼容性）
         * @return 目标函数指针对象的引用
         */
        template <typename Fx, type_traits::other_trans::enable_if_t<as_helper<Fx, Rx(Args...)>::is_invocable_v, int> = 0>
        constexpr function_pointer_base<Fx> &as() noexcept {
            return static_cast<function_pointer<Fx> &>(*this);
        }

        /**
         * @brief 将函数指针对象制作为一个std::function的包装器
         * @return 返回包含invoker的std::function包装器
         */
        RAINY_NODISCARD std::function<Rx(Args...)> make_function_object() const {
            return this->invoker;
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

        /**
         * @brief 初始化函数指针对象
         * @param function_address 函数指针的地址
         */
        constexpr function_pointer_base(pointer function_address) : base(function_address) {
        }

        /**
         * @brief 构造函数，用于从函数指针对象构造函数指针对象
         * @attention 函数指针是必须与原类型兼容的，否则，编译失败
         * @tparam URx 如果此模板函数指针返回类型为int，而URx与int兼容。则会通过编译
         * @tparam UArgs 同理，如果与原类型兼容。也会通过编译
         * @param function_address 函数指针的地址
         */
        template <typename URx, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs..., ...), Args...>, int> = 0>
        constexpr function_pointer_base(URx (*function_address)(UArgs..., ...)) : base(function_address) {
        }

        /**
         * @brief 调用函数指针对象，并传入参数
         * @attention 必须确保函数指针是有效的，否则，将会抛出运行时异常。编译时将会导致编译失败
         * @tparam UArgs 这是特定于C风格的可变参数支持，例如，如果函数指针对象签名为 int(*)(const char*,...) 。则此部分由UArgs提供
         * @param args 固定的参数
         * @param uargs 可选的额外参数
         * @return 如果返回类型为void，则不进行返回。如果返回类型不为void，则返回调用结果
         */
        template <typename... UArgs>
        constexpr Rx invoke(Args... args, UArgs... uargs) const {
            if (this->empty()) {
                exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (type_traits::primary_types::is_void_v<Rx>) {
                this->invoker(args..., uargs...);
            } else {
                return this->invoker(args..., uargs...);
            }
        }

        /**
         * @brief 调用函数指针对象，并传入参数
         * @attention 必须确保函数指针是有效的，否则，将会抛出运行时异常。编译时将会导致编译失败
         * @tparam UArgs 这是特定于C风格的可变参数支持，例如，如果函数指针对象签名为 int(*)(const char*,...) 。则此部分由UArgs提供
         * @param args 固定的参数
         * @param uargs 可选的额外参数
         * @return 如果返回类型为void，则不进行返回。如果返回类型不为void，则返回调用结果
         */
        template <typename... UArgs>
        constexpr Rx operator()(Args... args, UArgs... uargs) const {
            if (this->empty()) {
                exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (type_traits::primary_types::is_void_v<Rx>) {
                this->invoker(args..., uargs...);
            } else {
                return this->invoker(args..., uargs...);
            }
        }

        /**
         * @brief 将函数指针对象转换成一种函数指针对象的引用
         * @tparam Fx 目标类型。它必须兼容原本函数指针的类型
         * @return 以this引用的形式返回一个函数指针对象，它的类型将会是Fx
         */
        template <typename Fx, type_traits::other_trans::enable_if_t<as_helper<Fx, Rx(Args..., ...)>::is_invocable_v, int> = 0>
        function_pointer_base<Fx> &as() noexcept {
            return reinterpret_cast<function_pointer<Fx> &>(*this);
        }

        /**
         * @brief 此重载将使得当前函数指针成为一个空指针
         * @return 返回当前对象的this引用
         */
        constexpr function_pointer_base &operator=(std::nullptr_t) noexcept {
            this->assign(nullptr);
            return *this;
        }
    };
}

namespace rainy::foundation::functional {
    /**
     * @brief 此模板允许用户创建一个具有类型安全的函数指针对象
     * @tparam Rx 函数指针返回值类型
     * @tparam Args 函数指针参数类型
     *
     * @brief 以下代码展示了如何创建一个函数指针对象，并调用它：
     * @code
     * int add(int a, int b) {
     *  return a + b;
     * }
     * function_pointer<int(int, int)> fptr = &add;
     * int result = fptr(1, 2); // result == 3
     * @endcode
     */
    template <typename Rx, typename... Args>
    struct function_pointer<Rx(Args...)> : implements::function_pointer_base<Rx(Args...)> {
        using base = implements::function_pointer_base<Rx(Args...)>;
        using base::base;
        using base::operator=;

        using pointer = typename base::pointer;
        
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
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer reset(URx (*function_address)(UArgs...) = nullptr) noexcept {
            pointer old_ptr = this->invoker;
            this->invoker = function_address;
            return function_pointer{old_ptr};
        }

        constexpr function_pointer &operator=(pointer function_address) noexcept {
            this->assign(function_address);
            return *this;
        }

        template <typename URx, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>,
                      int> = 0>
        constexpr function_pointer &operator=(URx (*function_address)(UArgs...)) noexcept {
            this->assign(static_cast<pointer>(function_address));
            return *this;
        }
    };

    template <typename Rx, typename... Args>
    struct function_pointer<Rx(Args..., ...)> : implements::function_pointer_base<Rx(Args..., ...)> {
        using base = implements::function_pointer_base<Rx(Args..., ...)>;
        using base::base;
        using base::operator=;
        
        using pointer = typename base::pointer;

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
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs..., ...), Args...>, int> = 0>
        constexpr function_pointer reset(URx (*function_address)(UArgs..., ...) = nullptr) noexcept {
            pointer old_ptr = this->invoker;
            this->invoker = function_address;
            return function_pointer{old_ptr};
        }

        constexpr function_pointer &operator=(pointer function_address) noexcept {
            this->assign(function_address);
            return *this;
        }

        template <typename URx, typename... UArgs,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs..., ...), Args...>, int> = 0>
        constexpr function_pointer &operator=(URx (*function_address)(UArgs..., ...)) noexcept {
            this->assign(function_address);
            return *this;
        }

        constexpr function_pointer &operator=(std::nullptr_t) noexcept {
            this->assign(nullptr);
            return *this;
        }
    };

    /**
     * @brief 此模板允许用户创建一个具有类型安全的函数指针对象
     * @tparam Rx 函数指针返回值类型
     * @tparam Args 函数指针参数类型
     *
     * @brief 以下代码展示了如何创建一个函数指针对象，并调用它：
     * @code
     * int add(int a, int b) {
     *  return a + b;
     * }
     * function_pointer<int(*)(int, int)> fptr = &add;
     * int result = fptr(1, 2); // result == 3
     * @endcode
     */
    template <typename Rx, typename... Args>
    struct function_pointer<Rx (*)(Args...)> : function_pointer<Rx(Args...)> {
        using base = function_pointer<Rx(Args...)>;
        using base::base;
        using base::operator=;
    };

    template <typename Rx, typename... Args>
    struct function_pointer<Rx (*)(Args..., ...)> : function_pointer<Rx(Args..., ...)> {
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
