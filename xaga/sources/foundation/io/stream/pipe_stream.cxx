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
#include <rainy/foundation/io/stream/pipe_stream.hpp>

namespace rainy::foundation::io::stream {
    unnamed_pipe_stream unnamed_pipe_stream::create(executor_type executor) {
        std::error_code ec;
        auto result = create(executor, ec);
        if (ec) {
            throw exceptions::runtime::system_error(ec);
        }
        return result;
    }

    unnamed_pipe_stream unnamed_pipe_stream::create(executor_type executor, std::error_code &ec) {
        auto [read_impl, write_impl] = implements::create_pipe_impl(executor, ec);
        if (ec) {
            return unnamed_pipe_stream{read_end{executor, implements::create_null_impl(executor)},
                                       write_end{executor, implements::create_null_impl(executor)}};
        }
        return unnamed_pipe_stream{read_end{executor, utility::move(read_impl)}, write_end{executor, utility::move(write_impl)}};
    }
}

namespace rainy::foundation::io::stream {
    named_pipe_stream named_pipe_stream::open_server(executor_type executor, foundation::text::string_view name,
                                                     pipe_direction dir) {
        std::error_code ec;
        auto result = open_server(executor, name, dir, ec);
        if (ec) {
            throw exceptions::runtime::system_error(ec);
        }
        return result;
    }

    named_pipe_stream named_pipe_stream::open_server(executor_type executor, foundation::text::string_view name, pipe_direction dir,
                                                     std::error_code &ec) {
        auto impl = implements::create_named_pipe_server_impl(executor, name.data(), dir, ec);
        return named_pipe_stream{executor, utility::move(impl)};
    }

    named_pipe_stream named_pipe_stream::open_client(executor_type executor, foundation::text::string_view name, pipe_direction dir) {
        std::error_code ec;
        auto result = open_client(executor, name, dir, ec);
        if (ec) {
            throw exceptions::runtime::system_error(ec);
        }
        return result;
    }

    named_pipe_stream named_pipe_stream::open_client(executor_type executor, foundation::text::string_view name, pipe_direction dir,
                                                     std::error_code &ec) {
        auto impl = implements::create_named_pipe_client_impl(executor, name.data(), dir, ec);
        return named_pipe_stream{executor, utility::move(impl)};
    }
}
