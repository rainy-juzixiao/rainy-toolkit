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
#include <rainy/foundation/os/process.hpp>

#include <dirent.h>
#include <sys/resource.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

extern char **environ; // for extern

namespace rainy::foundation::os::implements {
    rain_fn current_process_id() noexcept -> std::uint64_t {
        return static_cast<std::uint64_t>(::getpid());
    }

    rain_fn create_process(const core::text::string &exec, const core::collections::vector<core::text::string> &args)
        noexcept -> native_process {
        native_process result{};
        core::collections::vector<char *> argv;
        argv.reserve(args.size() + 2);
        argv.push_back(const_cast<char *>(exec.c_str()));
        for (const auto &argument : args) {
            argv.push_back(const_cast<char *>(argument.c_str()));
        }
        argv.push_back(nullptr);

        pid_t child = -1;
        if (::posix_spawnp(&child, exec.c_str(), nullptr, nullptr, argv.data(), environ) != 0) {
            return result;
        }
        result.id = static_cast<std::uint64_t>(child);
        return result;
    }

    rain_fn wait_process(native_process &process) noexcept -> int {
        if (process.id == 0) {
            return -1;
        }
        int status = 0;
        if (::waitpid(static_cast<pid_t>(process.id), &status, 0) < 0) {
            return -1;
        }
        process.id = 0;
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return -1;
    }

    rain_fn terminate_process(native_process &process) noexcept -> bool {
        if (process.id == 0) {
            return false;
        }
        return ::kill(static_cast<pid_t>(process.id), SIGKILL) == 0;
    }

    rain_fn release_process(native_process &process) noexcept -> void {
        process.id = 0;
    }

    rain_fn query_process_list() noexcept -> core::collections::vector<process_entry> {
        core::collections::vector<process_entry> entries;
        DIR *directory = ::opendir("/proc");
        if (!directory) {
            return entries;
        }
        while (const dirent *item = ::readdir(directory)) {
            const char *name = item->d_name;
            const std::size_t length = std::strlen(name);
            for (std::size_t i = 0; i < length; ++i) {
                if (name[i] < '0' || name[i] > '9') {
                    name = nullptr;
                    break;
                }
            }
            if (!name) {
                continue;
            }
            process_entry entry{};
            entry.id = std::strtoull(name, nullptr, 10);
            char path[64];
            std::snprintf(path, sizeof(path), "/proc/%s/comm", name);
            if (FILE *file = ::fopen(path, "r")) {
                char buffer[256]{};
                if (::fgets(buffer, sizeof(buffer), file)) {
                    const std::size_t value_length = std::strlen(buffer);
                    if (value_length > 0 && buffer[value_length - 1] == '\n') {
                        buffer[value_length - 1] = '\0';
                    }
                    entry.name = core::text::string{buffer};
                }
                ::fclose(file);
            }
            entries.push_back(utility::move(entry));
        }
        ::closedir(directory);
        return entries;
    }

    rain_fn current_priority() noexcept -> int {
        // nice 值范围 [-20, 19]，errno 读取前先归零以区分失败。
        errno = 0;
        const int nice_value = ::getpriority(PRIO_PROCESS, 0);
        return errno == 0 ? nice_value : 0;
    }

    rain_fn set_current_priority(int priority) noexcept -> bool {
        return ::setpriority(PRIO_PROCESS, 0, priority) == 0;
    }
}
