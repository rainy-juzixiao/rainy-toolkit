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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_BSON_EXCEPTIONS_HPP
#include <rainy/foundation/willow/implements/bson/version.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

#if RAINY_WILLOW_BSON_AVAILABLE

namespace rainy::foundation::exceptions::willow::bson {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bson_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "bson exception", throw_bson_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bson_type_error, bson_exception, "bson type error exception", throw_bson_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bson_parse_error, bson_exception, "bson parse error exception", throw_bson_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(bson_serialize_error, bson_exception, "bson serialize error exception",
                                      throw_bson_serialize_error);
}

#endif

#endif
