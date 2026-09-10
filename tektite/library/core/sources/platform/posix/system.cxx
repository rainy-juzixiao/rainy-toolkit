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
#include <rainy/core/platform.hpp>
#include <rainy/core/layer.hpp>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace rainy::core::layer {
    rain_fn query_environment(const char *name, char *buffer, std::size_t length) noexcept -> bool {
        if (!name || !buffer || length == 0) {
            return false;
        }
        const char *value = ::getenv(name);
        if (!value) {
            return false;
        }
        const std::size_t value_length = std::strlen(value);
        if (value_length >= length) {
            return false;
        }
        std::memcpy(buffer, value, value_length + 1);
        return true;
    }

    rain_fn set_environment(const char *name, const char *value) noexcept -> bool {
        if (!name || !value) {
            return false;
        }
        return ::setenv(name, value, 1) == 0;
    }

    rain_fn remove_environment(const char *name) noexcept -> bool {
        if (!name) {
            return false;
        }
        return ::unsetenv(name) == 0;
    }

    rain_fn query_user_name(char *buffer, std::size_t length) noexcept -> bool {
        if (!buffer || length == 0) {
            return false;
        }
        if (::getlogin_r(buffer, length) == 0) {
            return true;
        }
        // 在无控制终端/登录会话的环境（例如 CI 容器）中，getlogin_r 可能失败，试试用这个法子
        passwd storage{};
        passwd *result = nullptr;
        char aux[1024];
        if (::getpwuid_r(::geteuid(), &storage, aux, sizeof(aux), &result) != 0 || !result || !result->pw_name) {
            return false;
        }
        const std::size_t name_length = std::strlen(result->pw_name);
        if (name_length >= length) {
            return false;
        }
        std::memcpy(buffer, result->pw_name, name_length + 1);
        return true;
    }

    rain_fn current_group_id() noexcept -> std::uint32_t {
        return static_cast<std::uint32_t>(::getgid());
    }

    rain_fn query_group_name(std::uint32_t group_id, char *buffer, std::size_t length) noexcept -> bool {
        if (!buffer || length == 0) {
            return false;
        }
        group storage{};
        group *result = nullptr;
        char aux[1024];
        if (::getgrgid_r(static_cast<gid_t>(group_id), &storage, aux, sizeof(aux), &result) != 0 || !result ||
            !result->gr_name) {
            return false;
        }
        const std::size_t name_length = std::strlen(result->gr_name);
        if (name_length >= length) {
            return false;
        }
        std::memcpy(buffer, result->gr_name, name_length + 1);
        return true;
    }
}
