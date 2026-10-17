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
#ifndef RAINY_FOUNDATION_WILLOW_HJSON_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_HJSON_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::exceptions::willow::hjson {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(hjson_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "hjson exception", throw_hjson_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(hjson_type_error, hjson_exception, "hjson type error exception",
                                      throw_hjson_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(hjson_invalid_key, hjson_exception, "hjson invalid key exception",
                                      throw_hjson_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(hjson_invalid_iterator, hjson_exception, "hjson invalid iterator exception",
                                      throw_hjson_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(hjson_parse_error, hjson_exception, "hjson parse error exception",
                                      throw_hjson_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(hjson_serialize_error, hjson_exception, "hjson serialize error exception",
                                      throw_hjson_serialize_error);
}

#endif
