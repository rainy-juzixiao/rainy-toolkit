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
#ifndef RAINY_FOUNDATION_WILLOW_JSON5_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_JSON5_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::exceptions::willow::json5 {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json5_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "json5 exception", throw_json5_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json5_type_error, json5_exception, "json5 type error exception", throw_json5_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json5_invalid_key, json5_exception, "json5 invalid key exception", throw_json5_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json5_invalid_iterator, json5_exception, "json5 invalid iterator exception",
                                      throw_json5_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json5_parse_error, json5_exception, "json5 parse error exception", throw_json5_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json5_serialize_error, json5_exception, "json5 serialize error exception",
                                      throw_json5_serialize_error);
}


#endif
