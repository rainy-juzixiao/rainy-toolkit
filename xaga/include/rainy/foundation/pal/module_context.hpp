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
#ifndef RAINY_FOUNDATION_PAL_MODULE_CONTEXT_HPP
#define RAINY_FOUNDATION_PAL_MODULE_CONTEXT_HPP // NOLINT
#include <functional>
#include <rainy/core/core.hpp>
#include <rainy/foundation/pal/implements/tgc_layer_module_context.hpp>

namespace rainy::foundation::pal::module_context {
    /**
     * @brief 模块的上下文，是对动态库的封装
     * @remark 模块是程序加载的库，而非进程本身
     */
    class RAINY_TOOLKIT_API context {
    public:
        /**
         *
         */
        context() noexcept : private_(nullptr) {}

        /**
         * @brief 在构造期间对上下文进行初始化
         * @param module_path 模块所在的路径，包括模块名
         * @param load 指定是否加载模块，如果为false，则仅尝试获取已加载的模块
         */
        context(std::string_view module_path, bool load = true) noexcept; // NOLINT

        context(const context& right) = delete; // 为了确保模块生命周期管理，禁用拷贝，若要实现类似效果，请让另一个context加载同样的模块路径以获得新的模块上下文

        /**
         * @brief 将上下文对象移动到当前模块
         */
        context(context&& right) noexcept;

        context& operator=(const context& right) = delete; // 同上

        /**
         * @brief 将上下文对象移动到当前模块，如果当前模块仍然持有一个已有的模块，则会优先调用unload再执行
         */
        context& operator=(context&& right) noexcept;

        /**
         * @brief 自动在模块到达其生命周期末尾时，释放对当前模块的引用计数
         */
        ~context();

        /**
         * @brief 释放对当前模块的引用计数
         * @attention 如果is_loaded()为false，则此函数返回false，另外，如果该上下文是直接尝试获取模块而不加载，则不会释放引用计数
         * @return 如果成功释放，则返回true，否则false
         */
        bool release() noexcept;

        /**
         * @brief 尝试加载模块
         * @param module_path 模块所在的路径，包括模块名
         * @attention 该方式加载的模块将会让该上下文获得所有权，并且会在结束时释放
         * @return 如果成功加载，则返回true，否则false
         */
        bool load(std::string_view module_path) noexcept;

        /**
         * @brief 尝试获取模块，不尝试加载
         * @attention 该方式加载的模块不会让该上下文获得所有权，该上下文生命周期的结束不会产生任何副作用
         * @return 如果成功获取，则返回true，否则false
         */
        bool try_get_module(std::string_view module_path) noexcept;

        /**
         * @brief 从指定的模块中获取其导出的符号
         * @param symbol_name 符号名称
         * @attention 如果is_loaded()为false，则必为空地址。另外，如果当前上下文处于观察状态，请务必确保你观察的模块生命周期长于持有模块的生命周期
         * @return 如果获取成功，返回该符号对应的地址，以farproc_fn类型的形式
         */
        RAINY_NODISCARD farproc_fn load_symbol(std::string_view symbol_name) const noexcept;

        /**
         * @brief 从指定的模块中获取其导出的符号，并将其返回地质转化为一个函数指针并赋给ptr
         * @tparam Fx 目标符号函数指针的签名
         * @param symbol_name 符号名称
         * @param fx 用于接收的指针，以引用形式传递
         * @attention 如果is_loaded()为false，则必为空地址
         * @return 如果成功，返回true，并且fx将带有加载的符号地址；失败返回false并将fx设置为空
         */
        template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_function_v<Fx>, int> = 0>
        bool load_symbol_to(const std::string_view symbol_name, Fx *&fx) noexcept {
            if (rainy_let ptr = load_symbol(symbol_name); ptr != nullptr) {
                fx = reinterpret_cast<Fx *>(ptr);
                return true;
            }
            fx = nullptr;
            return false;
        }

        /**
         * @return 检查是否已成功加载
         */
        RAINY_NODISCARD bool is_loaded() const noexcept;

        /**
         * @brief 检查当前上下文是否持有模块
         * @return 如果is_loaded()为false返回false，如果是通过加载获取的，返回false，否则为true
         */
        RAINY_NODISCARD bool is_observe() const noexcept;

        /**
         * @return 获取当前模块的句柄
         * @return 模块句柄
         */
        RAINY_NODISCARD void *native_handle() const noexcept;

    private:
        struct context_private;

        static context_private* create_ctx();
        static void destroy_ctx(context_private const* ctx);

        context_private* private_;
    };
}

#endif
