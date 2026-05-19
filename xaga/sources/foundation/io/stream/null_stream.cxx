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
#include <rainy/foundation/io/stream/null_stream.hpp>

namespace rainy::foundation::io::stream {
    null_stream& null_stream::null(){
        static null_stream instance{default_context().get_executor()};
        return instance;
    }

    null_stream::executor_type null_stream::get_executor() const noexcept {
        return executor_;
    }

    bool null_stream::is_open() const noexcept {
        return impl_->is_open();
    }

    native_handle_type null_stream::native_handle() const noexcept {
        return impl_->native_handle();
    }

    null_stream::null_stream(executor_type ex) :
        executor_(ex), impl_(implements::create_null_impl(ex)) {
    }

    io_context &null_stream::default_context() {
        static io_context default_context;
        return default_context;
    }
}
