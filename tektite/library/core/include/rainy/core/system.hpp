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
#ifndef RAINY_CORE_SYSTEM_HPP
#define RAINY_CORE_SYSTEM_HPP

#include <rainy/core/container/optional.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/layer.hpp>

namespace rainy::core::system {
    /**
     * \lang english
     * @brief The processor architecture the library was compiled for.
     *
     * \lang simp-chinese
     * @brief 本库编译目标所处的处理器架构。
     */
    enum class cpu_architecture { unknown, x86, x86_64, arm, arm64 };

    /**
     * \lang english
     * @brief Returns the processor architecture compiled for.
     *
     * \lang simp-chinese
     * @brief 返回编译目标处理器架构。
     */
    RAINY_NODISCARD constexpr rain_fn current_architecture() noexcept -> cpu_architecture {
#if RAINY_IS_ARM64
        return cpu_architecture::arm64;
#elif RAINY_IS_X86_PLATFORM && RAINY_USING_32_BIT_PLATFORM
        return cpu_architecture::x86;
#elif RAINY_IS_X86_PLATFORM && RAINY_USING_64_BIT_PLATFORM
        return cpu_architecture::x86_64;
#else
        return cpu_architecture::unknown;
#endif
    }

#if RAINY_IS_ARM64
    /**
     * \lang english
     * @brief ARM64 hardware capabilities probed from AT_HWCAP.
     *
     * \lang simp-chinese
     * @brief 通过AT_HWCAP探测的ARM64硬件能力。
     */
    enum class arm64_feature : unsigned long {
        asimd = 1 << 0,
        aes = 1 << 3,
        pmull = 1 << 4,
        sha1 = 1 << 5,
        sha2 = 1 << 6,
        crc32 = 1 << 7,
    };

    /**
     * \lang english
     * @brief Checks whether the CPU exposes an ARM64 hardware capability.
     *
     * \lang simp-chinese
     * @brief 检查CPU是否暴露某个ARM64硬件能力。
     */
    RAINY_NODISCARD inline rain_fn has_arm64_feature(const arm64_feature feature) noexcept -> bool {
        return (layer::arm64_hwcap() & static_cast<unsigned long>(feature)) != 0;
    }
#endif

    /**
     * \lang english
     * @brief A snapshot of the system's physical memory capacity.
     *
     * \lang simp-chinese
     * @brief 系统物理内存容量的快照。
     */
    struct memory_capacity {
        std::size_t total;
        std::size_t available;
    };

    /**
     * \lang english
     * @brief Queries the total and available physical memory.
     *
     * \lang simp-chinese
     * @brief 查询物理内存总量与可用量。
     */
    RAINY_NODISCARD inline rain_fn query_memory_capacity() noexcept -> memory_capacity {
        memory_capacity capacity{0, 0};
        layer::query_memory_capacity(&capacity.total, &capacity.available);
        return capacity;
    }

    /**
     * \lang english
     * @brief Reads an environment variable; an empty optional when it does not exist.
     *
     * \lang simp-chinese
     * @brief 读取环境变量；不存在时返回空optional。
     */
    RAINY_NODISCARD inline rain_fn get_environment(const text::string_view name) -> container::optional<text::string> {
        char buffer[4096];
        if (!layer::query_environment(name.data(), buffer, sizeof(buffer))) {
            return container::nullopt;
        }
        return text::string{buffer};
    }

    /**
     * \lang english
     * @brief Creates or updates an environment variable.
     *
     * \lang simp-chinese
     * @brief 创建或更新环境变量。
     */
    inline rain_fn set_environment(const text::string_view name, const text::string_view value) noexcept -> bool {
        return layer::set_environment(name.data(), value.data());
    }

    /**
     * \lang english
     * @brief Removes an environment variable.
     *
     * \lang simp-chinese
     * @brief 移除环境变量。
     */
    inline rain_fn remove_environment(const text::string_view name) noexcept -> bool {
        return layer::remove_environment(name.data());
    }

    /**
     * \lang english
     * @brief Returns the login name of the current user; an empty string on failure.
     *
     * \lang simp-chinese
     * @brief 返回当前用户登录名；失败时返回空串。
     */
    RAINY_NODISCARD inline rain_fn user_name() -> text::string {
        char buffer[270];
        if (!layer::query_user_name(buffer, sizeof(buffer))) {
            return text::string{};
        }
        return text::string{buffer};
    }

    /**
     * \lang english
     * @brief Returns the group id of the current user.
     *
     * \lang simp-chinese
     * @brief 返回当前用户所属组的id。
     */
    RAINY_NODISCARD inline rain_fn current_group_id() noexcept -> std::uint32_t {
        return layer::current_group_id();
    }

    /**
     * \lang english
     * @brief Returns the group name for a group id; an empty optional when unknown.
     *
     * \lang simp-chinese
     * @brief 返回组id对应的组名；未知时返回空optional。
     */
    RAINY_NODISCARD inline rain_fn group_name(const std::uint32_t group_id) -> container::optional<text::string> {
        char buffer[256];
        if (!layer::query_group_name(group_id, buffer, sizeof(buffer))) {
            return container::nullopt;
        }
        return text::string{buffer};
    }

    /**
     * \lang english
     * @brief Fills a buffer with cryptographically secure random bytes from the system source.
     *
     * \lang simp-chinese
     * @brief 使用系统随机源以加密安全方式填充缓冲区。
     */
    inline rain_fn system_random(void *buffer, const std::size_t length) noexcept -> bool {
        return layer::system_random(buffer, length);
    }
}

#endif
