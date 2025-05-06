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
#include <rainy/linux_api/core.h>
#if RAINY_USING_LINUX

namespace rainy::linux_api::filesystem {
    enum class linux_file_attributes {
        read_only = 0x1,
        hidden = 0x2,
        system = 0x4,
        directory = 0x8,
        archive = 0x10,
        device = 0x20,
        normal = 0x40,
        temporary = 0x80,
        sparse_file = 0x100,
        reparse_point = 0x200,
        invalid = -1,
    };

    enum class linux_reparse_tag {
        none,
        mount_point,
        symlink,
    };

    struct linux_file_status {
        std::int64_t last_write_time;
        std::uint64_t file_size;
        linux_file_attributes attributes;
        linux_reparse_tag reparse_tag;
        std::uint32_t link_count;
    };

    enum class linux_file_open_mode : unsigned int {
        no_state = 0x00,
        read = 0x01,
        write = 0x02,
        append = 0x04,
        binary = 0x08,
        truncate = 0x10,
        nocreate = 0x20,
        update = 0x40
    };

    enum class linux_file_type {
        none,
        not_found,
        regular,
        directory,
        symlink,
        block,
        character,
        fifo,
        socket,
        unknown,
        junction
    };

    using linux_filehandle = std::uintptr_t;
}

namespace rainy::linux_api::filesystem {
    RAINY_TOOLKIT_LINUXAPI linux_file_status get_file_status(core::czstring file_path) noexcept;
    RAINY_TOOLKIT_LINUXAPI linux_filehandle open_file(core::czstring filepath, linux_file_open_mode mode);
    RAINY_TOOLKIT_LINUXAPI bool close_file(linux_filehandle handle);
    RAINY_TOOLKIT_LINUXAPI bool file_exists(core::czstring file_path) noexcept;
    RAINY_TOOLKIT_LINUXAPI linux_file_type get_file_type(core::czstring file_path) noexcept;
}

#endif