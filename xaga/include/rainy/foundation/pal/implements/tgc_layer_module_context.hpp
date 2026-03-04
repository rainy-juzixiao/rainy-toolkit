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
#ifndef RAINY_FOUNDATION_PAL_IMPLEMENTS_TGC_LAYER_MODULE_CONTEXT_HPP
#define RAINY_FOUNDATION_PAL_IMPLEMENTS_TGC_LAYER_MODULE_CONTEXT_HPP // NOLINT
#include  <rainy/core/core.hpp>

namespace rainy::foundation::pal::module_context {
    using farproc_fn = void (*)();
}

namespace rainy::foundation::pal::module_context::implements {
    /**
     * @brief 尝试根据指定的参数加载模块并获取该模块的句柄
     * @param module_path 模块所在的路径，包括模块名
     * @param crossplatform 允许不使用平台特定名称加载模块
     * @return 如果成功，返回该模块的句柄，失败则返回空句柄
     */
    RAINY_TOOLKIT_API core::handle load_module(std::string_view module_path, bool crossplatform) noexcept;

    /**
     * @brief 尝试根据指定的参数获取已加载的模块
     * @param module_path 模块所在的路径，包括模块名
     * @param crossplatform 允许不使用平台特定名称加载模块
     * @return 如果成功，返回该模块的句柄，失败则返回空句柄
     */
    RAINY_TOOLKIT_API core::handle try_to_get_module(std::string_view module_path, bool crossplatform) noexcept;

    /**
     * @brief 从指定的模块中获取其导出的符号
     * @param handle 模块的pal句柄
     * @param symbol_name 符号名称
     * @return 如果获取成功，返回该符号对应的地址，以farproc_fn类型的形式
     */
    RAINY_TOOLKIT_API farproc_fn load_symbol(core::handle handle, std::string_view symbol_name) noexcept;

    /**
     * @brief 释放给定的pal模块
     * @param handle 模块的pal句柄
     * @attention 释放模块并不会直接导致
     * @return 如果释放成功，则返回true，否则false，所有模块不会因为使用，如果成功，handle句柄还会自动被设为空
     */
    RAINY_TOOLKIT_API bool release_module(core::handle *handle) noexcept;
}

#endif