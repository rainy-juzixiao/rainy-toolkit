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
#ifndef RAINY_FOUNDATION_OS_PROCESS_HPP
#define RAINY_FOUNDATION_OS_PROCESS_HPP

#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::foundation::os {
    /**
     * @brief A snapshot of one running process enumerated from the system.
     *        从系统枚举到的一个运行中进程的快照。
     */
    struct process_entry {
        std::uint64_t id;
        core::text::string name;
    };
}

namespace rainy::foundation::os::implements {
    /**
     * @brief Returns the id of the current process.
     *        返回当前进程的id。
     */
    RAINY_TOOLKIT_API rain_fn current_process_id() noexcept -> std::uint64_t;

    struct native_process {
        std::uint64_t id{0};
        void *handle{nullptr};
    };

    /**
     * @brief Spawns a new process for the given executable and arguments.
     *        为给定可执行文件与参数创建新进程。
     *
     * @return The native process; id == 0 on failure
     *         原生进程；失败时id为0
     */
    RAINY_TOOLKIT_API rain_fn create_process(const core::text::string &exec,
                                             const core::collections::vector<core::text::string> &args) noexcept -> native_process;

    /**
     * @brief Waits until the process exits and returns its exit code.
     *        等待进程退出并返回其退出码。
     *
     * @return The exit code; -1 on failure
     *         退出码；失败时返回-1
     */
    RAINY_TOOLKIT_API rain_fn wait_process(native_process &process) noexcept -> int;

    /**
     * @brief Forces the process to terminate.
     *        强制终止进程。
     *
     * @return true on success
     *         成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn terminate_process(native_process &process) noexcept -> bool;

    /**
     * @brief Releases the platform resources held by the native process.
     *        释放原生进程持有的平台资源。
     */
    RAINY_TOOLKIT_API rain_fn release_process(native_process &process) noexcept -> void;

    /**
     * @brief Enumerates every running process on the system.
     *        枚举系统上所有运行中的进程。
     */
    RAINY_TOOLKIT_API rain_fn query_process_list() noexcept -> core::collections::vector<process_entry>;

    /**
     * @brief Queries the scheduling priority of the current process.
     *        查询当前进程的调度优先级。
     *
     * @return The priority value; POSIX nice value range or Windows priority class
     *         优先级值；POSIX为nice值范围，Windows为优先级类别
     */
    RAINY_TOOLKIT_API rain_fn current_priority() noexcept -> int;

    /**
     * @brief Updates the scheduling priority of the current process.
     *        更新当前进程的调度优先级。
     *
     * @param priority The new priority value (POSIX nice value or Windows priority class)
     *                 新的优先级值（POSIX nice值或Windows优先级类别）
     * @return true on success
     *         成功时返回true
     */
    RAINY_TOOLKIT_API rain_fn set_current_priority(int priority) noexcept -> bool;
}

namespace rainy::foundation::os {
    /**
     * @brief Returns the id of the current process.
     *        返回当前进程的id。
     */
    RAINY_NODISCARD RAINY_TOOLKIT_API rain_fn current_process_id() noexcept -> std::uint64_t;

    /**
     * @brief Enumerates every running process on the system.
     *        枚举系统上所有运行中的进程。
     */
    RAINY_NODISCARD RAINY_TOOLKIT_API rain_fn process_list() noexcept -> core::collections::vector<process_entry>;

    /**
     * @brief Queries the scheduling priority of the current process.
     *        查询当前进程的调度优先级。
     */
    RAINY_NODISCARD RAINY_TOOLKIT_API rain_fn current_priority() noexcept -> int;

    /**
     * @brief Updates the scheduling priority of the current process.
     *        更新当前进程的调度优先级。
     */
    RAINY_TOOLKIT_API rain_fn set_current_priority(int priority) noexcept -> bool;

    /**
     * @brief An owned handle over a process spawned from this library.
     *        对由本库创建的进程的独占句柄。
     */
    class process {
    public:
        process() = default;

        /**
         * @brief Spawns a new process for the given executable with arguments.
         *        为给定可执行文件与参数创建新进程。
         *
         * @param exec The executable path
         *             可执行文件路径
         * @param args The argument list (without argv[0])
         *             参数列表（不含argv[0]）
         */
        explicit process(const core::text::string &exec, const core::collections::vector<core::text::string> &args = {}) {
            native_ = implements::create_process(exec, args);
        }

        process(const process &) = delete;
        process &operator=(const process &) = delete;

        process(process &&right) noexcept : native_(utility::exchange(right.native_, implements::native_process{})) {
        }

        process &operator=(process &&right) noexcept {
            if (this != utility::addressof(right)) {
                reset();
                native_ = utility::exchange(right.native_, implements::native_process{});
            }
            return *this;
        }

        ~process() {
            reset();
        }

        /**
         * @brief Returns the process id; 0 when the process is not valid.
         *        返回进程id；进程无效时为0。
         */
        RAINY_NODISCARD rain_fn id() const noexcept -> std::uint64_t {
            return native_.id;
        }

        /**
         * @brief Returns true when the spawn succeeded.
         *        创建成功时返回true。
         */
        RAINY_NODISCARD explicit operator bool() const noexcept {
            return native_.id != 0;
        }

        /**
         * @brief Waits until the process exits and returns its exit code.
         *        等待进程退出并返回其退出码。
         */
        rain_fn wait() noexcept -> int {
            return implements::wait_process(native_);
        }

        /**
         * @brief Forces the process to terminate.
         *        强制终止进程。
         */
        rain_fn terminate() noexcept -> bool {
            return implements::terminate_process(native_);
        }

        /**
         * @brief Releases the handle without waiting; the child keeps running.
         *        不等待直接释放句柄；子进程继续运行。
         */
        rain_fn release() noexcept -> void {
            implements::release_process(native_);
            native_ = implements::native_process{};
        }

        rain_fn reset() noexcept -> void {
            if (native_.id != 0) {
                static_cast<void>(wait());
            }
        }

    private:
        implements::native_process native_{};
    };
}

#endif
