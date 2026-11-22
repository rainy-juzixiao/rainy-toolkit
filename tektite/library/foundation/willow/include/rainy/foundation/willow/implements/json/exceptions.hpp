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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::exceptions::willow::json {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "json exception", throw_json_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json_type_error, json_exception, "json type error exception", throw_json_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json_invalid_key, json_exception, "json invalid key exception", throw_json_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json_invalid_iterator, json_exception, "json invalid iterator exception", throw_json_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json_parse_error, json_exception, "json parse error exception", throw_json_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(json_serialize_error, json_exception, "json serialize error exception", throw_json_serialize_error);
}


#endif
