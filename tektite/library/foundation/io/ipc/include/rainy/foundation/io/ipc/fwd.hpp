/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0

 * *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS
 * IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language
 * governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_FOUNDATION_IO_IPC_STREAM_FWD_HPP
#define RAINY_FOUNDATION_IO_IPC_STREAM_FWD_HPP

namespace rainy::foundation::io::ipc::stream {
    class stream_descriptor;
    class console_stream;
    class named_pipe_stream;
    class null_stream;

    enum class console_stream_kind {
        input,
        output,
        error
    };

    enum class pipe_direction {
        in,
        out,
        inout
    };

#if RAINY_USING_WINDOWS
    using native_handle_type = void *;
#else
    using native_handle_type = std::intptr_t;
#endif
}

namespace rainy::foundation::io::ipc::message_queue {
    class message_queue;

    enum class priority : std::uint16_t {
        idle = 0,
        background = 1,
        maintenance = 2,
        lowest = 3,
        very_low = 4,
        low = 5,
        fairly_low = 6,
        below_normal = 7,
        slightly_low = 8,
        normal = 9,
        slightly_high = 10,
        above_normal = 11,
        fairly_high = 12,
        high = 13,
        very_high = 14,
        extremely_high = 15,
        highest = 16,
        critical = 17,
        very_critical = 18,
        extremely_critical = 19,
        emergency = 20,
        urgent = 21,
        very_urgent = 22,
        critical_alert = 23,
        realtime = 24,
        realtime_high = 25,
        realtime_critical = 26,
        realtime_emergency = 27,
        system = 28,
        system_high = 29,
        system_critical = 30,
        maximum = 31
    };

    struct attributes {
        std::size_t max_messages = 0;
        std::size_t max_message_size = 0;
        bool non_blocking = false;
    };

    enum class direction {
        in,
        out,
        inout
    };
}

namespace rainy::foundation::io::ipc::shared_memory {
    class shared_memory;

    enum class access_mode {
        read_only,
        read_write
    };

    struct attributes {
        std::size_t size;
        access_mode mode;
    };
}

namespace rainy::foundation::io::ipc::semaphore {
    class semaphore;

    struct attributes {
        unsigned int initial_value = 1;
        unsigned int max_value = 1;
    };
}

#endif