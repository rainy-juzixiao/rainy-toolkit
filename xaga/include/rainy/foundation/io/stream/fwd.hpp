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
#ifndef RAINY_FOUNDATION_IO_STREAM_FWD_HPP
#define RAINY_FOUNDATION_IO_STREAM_FWD_HPP

namespace rainy::foundation::io {
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
    using native_handle_type = int;
#endif
}

#endif
