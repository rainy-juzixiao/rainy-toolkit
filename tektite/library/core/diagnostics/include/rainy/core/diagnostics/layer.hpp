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
#ifndef RAINY_CORE_DEBUGGING_PAL_DEBUG_HPP
#define RAINY_CORE_DEBUGGING_PAL_DEBUG_HPP

#include <rainy/core/platform.hpp>

namespace rainy::core::layer {
    /**
     * @brief Maximum number of stack frames to dump.
     *        要转储的最大堆栈帧数。
     */
    constexpr int max_frames_dump = 128;

    /**
     * @brief Triggers a debug breakpoint.
     *        触发调试断点。
     */
    RAINY_TOOLKIT_API void breakpoint() noexcept;

    /**
     * @brief Triggers a debug breakpoint only if a debugger is present.
     *        仅在调试器存在时触发调试断点。
     */
    RAINY_TOOLKIT_API void breakpoint_if_debugging() noexcept;

    /**
     * @brief Checks whether a debugger is currently attached to the process.
     *        检查当前是否有调试器附加到进程。
     */
    RAINY_TOOLKIT_API bool is_debugger_present() noexcept;

    /**
     * @brief Triggers a debug breakpoint (always).
     *        触发调试断点（始终）。
     */
    RAINY_TOOLKIT_API rain_fn debug_break() -> void;

    /**
     * @brief Collects stack frames for debugging.
     *        收集堆栈帧用于调试。
     */
    RAINY_TOOLKIT_API rain_fn collect_stack_frame(native_frame_ptr_t *out_frames, std::size_t max_frames_count,
                                                  std::size_t skip) noexcept -> std::size_t;

    /**
     * @brief Safely dumps stack frames to memory.
     *        安全地将堆栈帧转储到内存。
     */
    RAINY_TOOLKIT_API rain_fn safe_dump_to(void *memory, std::size_t size, std::size_t skip) noexcept -> std::size_t;

    /**
     * @brief Resolves a stack frame to a human-readable string.
     *        将堆栈帧解析为人类可读的字符串。
     */
    RAINY_TOOLKIT_API rain_fn resolve_stack_frame(native_frame_ptr_t frame, cstring buf, std::size_t buf_size) noexcept -> bool;

    /**
     * @brief Demangles a C++ name to a human-readable form.
     *        将C++名称解修饰为人类可读的形式。
     */
    RAINY_TOOLKIT_API rain_fn demangle(czstring name, cstring buf, std::size_t buffer_length) -> void;
}

#endif
