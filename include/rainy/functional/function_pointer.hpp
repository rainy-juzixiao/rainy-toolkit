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
#ifndef RAINY_FUNCTION_POINTER_HPP
#define RAINY_FUNCTION_POINTER_HPP
#include <rainy/core/core.hpp>
#include <rainy/system/basic_exceptions.hpp>
#include <functional>

namespace rainy::foundation::functional {
    template <typename Fx>
    class function_pointer;

    /**
     * @brief 此模板允许用户创建一个具有类型安全的函数指针对象
     * @tparam Rx 函数指针期望的返回类型
     * @tparam Args 函数形参列表
     */
    template <typename Rx, typename... Args>
    class function_pointer<Rx(Args...)> {
    public:
        using pointer = Rx (*)(Args...);

        /**
         * @brief 仅创建一个基本的函数指针对象
         */
        constexpr function_pointer() : invoker(nullptr) {
        }

        /**
         * @brief 同默认构造函数
         */
        constexpr function_pointer(std::nullptr_t) : invoker(nullptr) {
        }

        /**
         * @brief 用函数地址初始化此函数对象
         */
        constexpr function_pointer(Rx (*function_address)(Args...)) : invoker(function_address) {
        }

        /**
         * @brief 用可能可以转换的函数地址初始化此函数对象
         * @brief 例如，如果模板形参返回值是int，而函数地址对应的签名为char，则产生转换
         */
        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<std::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer(URx (*function_address)(UArgs...)) : invoker(function_address) {
        }

        constexpr function_pointer(const function_pointer &) noexcept = default;
        constexpr function_pointer(function_pointer &&) noexcept = default;

        RAINY_CONSTEXPR20 ~function_pointer() = default;

        /**
         * @brief 获取当前函数指针对象保存的invoke地址
         */
        RAINY_NODISCARD constexpr pointer get() const noexcept {
            return invoker;
        }

        /**
         * @brief 获取当前函数指针对象保存的invoke地址
         */
        RAINY_NODISCARD explicit operator pointer() const noexcept {
            return invoker;
        }

        /**
         * @brief 获取当前模板中形参的个数
         */
        RAINY_NODISCARD static constexpr unsigned int arity() noexcept {
            return sizeof...(Args);
        }

        /**
         * @brief 检查当前函数指针对象是否为空
         */
        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return !static_cast<bool>(invoker);
        }

        /**
         * @brief 检查当前函数指针对象是否为空
         */
        constexpr explicit operator bool() const noexcept {
            return static_cast<bool>(invoker);
        }

        /**
         * @brief 调用当前函数指针对象，并传入参数列表
         * @return 如果函数返回值是void，则返回void，否则返回函数返回值
         */
        constexpr Rx invoke(Args... args) const {
            if (empty()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (std::is_void_v<Rx>) {
                invoker(args...);
            } else {
                return invoker(args...);
            }
        }

        /**
         * @brief 调用当前函数指针对象，并传入参数列表
         * @return 如果函数返回值是void，则返回void，否则返回函数返回值
         */
        constexpr Rx operator()(Args... args) const {
            if (empty()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("Current pointer is null!");
            }
            if constexpr (std::is_void_v<Rx>) {
                invoker(args...);
            } else {
                return invoker(args...);
            }
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer &operator=(pointer function_address) noexcept {
            return assign(function_address); // NOLINT
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer &operator=(const function_pointer &right) noexcept {
            return assign(right); // NOLINT
        }

        /**
         * @brief 将当前函数指针对象置空，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer &operator=(std::nullptr_t) noexcept {
            return assign(nullptr); // NOLINT
        }

        /**
         * @brief 移动赋值给当前函数指针对象，并返回引用
         * @attetion 使用默认实现
         */
        constexpr function_pointer &operator=(function_pointer &&) noexcept = default;

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<std::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer &operator=(URx (*function_address)(UArgs...)) noexcept {
            return assign(function_address); // NOLINT
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer &assign(pointer function_address) noexcept {
            invoker = function_address;
            return *this;
        }

        /**
         * @brief 赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer &assign(const function_pointer &right) noexcept {
            if (this != rainy::utility::addressof(right)) {
                invoker = right.invoker;
            }
            return *this;
        }

        /**
         * @brief 将当前函数指针对象置空，并返回引用
         * @return 当前函数指针对象引用
         */
        constexpr function_pointer &assign(std::nullptr_t) noexcept {
            invoker = nullptr;
            return *this;
        }

        /**
         * @brief 移动赋值给当前函数指针对象，并返回引用
         * @return 当前函数指针对象引用
 */
        template <typename URx, typename... UArgs,
                  rainy::type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<Rx, URx (*)(UArgs...), Args...>, int> = 0>
        constexpr function_pointer &assign(URx (*function_address)(UArgs...)) noexcept {
            invoker = function_address;
            return *this;
        }

        /**
         * @brief 使用新的函数地址替换当前函数指针对象，并返回旧的函数指针对象
         * @return
         * 旧的函数指针对象
         */
        constexpr function_pointer reset(pointer function_address = nullptr) noexcept {
            pointer old_ptr = invoker;
            invoker = function_address;
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
            pointer old_ptr = invoker;
            invoker = function_address;
            return function_pointer{old_ptr};
        }

        /**
         * @brief 置空当前函数指针对象
         */
        constexpr void clear() noexcept {
            invoker = nullptr;
        }

        /**
         * @brief 交换当前函数指针对象和另一个函数指针对象
         */
        constexpr void swap(function_pointer &right) noexcept {
            std::swap(invoker, right.invoker);
        }

        /**
         * @brief 将函数指针转换为std::function对象
         * @return std::function对象
         */
        RAINY_NODISCARD std::function<Rx(Args...)> make_function_object() const noexcept {
            return std::function<Rx(Args...)>(invoker);
        }

    private:
        pointer invoker;
    };

    /**
     * @brief 辅助模板，用于建立function_pointer别名
     */
    template <typename Rx, typename... Args>
    class function_pointer<Rx (*)(Args...)> : public function_pointer<Rx(Args...)> {};

    // 设置CTAD（将自动把Rx(*)(Args...)推导为function_pointer<Rx(Args...)>）
    template <typename Rx, typename... Args>
    function_pointer(Rx (*)(Args...)) -> function_pointer<Rx(Args...)>;

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

}

#endif
