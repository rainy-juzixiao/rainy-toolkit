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
#include <rainy/foundation/concurrency/executor.hpp>

namespace rainy::foundation::concurrency {
    executor &get_global_pooled_executor() {
        static executor obj = make_pooled_executor();
        return obj;
    }

    executor &get_global_dedicated_executor() {
        static executor obj = make_dedicated_executor();
        return obj;
    }
}
