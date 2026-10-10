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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_JSONNET_EXCEPTIONS_HPP
#include <rainy/foundation/willow/implements/common/exceptions.hpp>
#include <rainy/foundation/willow/implements/jsonnet/version.hpp>

#if RAINY_WILLOW_JSONNET_AVAILABLE

namespace rainy::foundation::exceptions::willow::jsonnet {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(jsonnet_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "jsonnet exception", throw_jsonnet_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(jsonnet_parse_error, jsonnet_exception, "jsonnet parse error exception",
                                      throw_jsonnet_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(jsonnet_evaluate_error, jsonnet_exception, "jsonnet evaluate error exception",
                                      throw_jsonnet_evaluate_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(jsonnet_type_error, jsonnet_exception, "jsonnet type error exception",
                                      throw_jsonnet_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(jsonnet_import_error, jsonnet_exception, "jsonnet import error exception",
                                      throw_jsonnet_import_error);
}

#endif

#endif
