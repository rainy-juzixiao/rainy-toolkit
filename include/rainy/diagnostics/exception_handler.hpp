/*
 * Copyright 2024 rainy-juzixiao
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
#ifndef RAINY_EXCEPTION_HANDLER
#define RAINY_EXCEPTION_HANDLER
#include <random>
#include <unordered_map>
#include <rainy/functional/function_pointer.hpp>
#include <rainy/diagnostics/source_location.hpp>

namespace rainy::information {
    class exception_handler {
    public:
        RAINY_DECLARE_SIGNLE_INSTANCE(exception_handler)

        static std::size_t add_process() {
            
        }



    private:
        exception_handler() = default;
        exception_handler(const exception_handler&) = default;
        exception_handler(exception_handler&&) = default;


        std::unordered_map<std::size_t,
                           foundation::functional::function_pointer<void(const std::string_view, const utility::source_location &)>>
            process_queue;
    };
}

#endif
