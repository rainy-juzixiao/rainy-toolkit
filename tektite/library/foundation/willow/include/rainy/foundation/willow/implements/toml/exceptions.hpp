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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_TOML_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::exceptions::willow::toml {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(toml_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "toml exception", throw_toml_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(toml_type_error, toml_exception, "toml type error exception", throw_toml_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(toml_invalid_key, toml_exception, "toml invalid key exception", throw_toml_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(toml_invalid_iterator, toml_exception, "toml invalid iterator exception",
                                      throw_toml_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(toml_parse_error, toml_exception, "toml parse error exception", throw_toml_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(toml_serialize_error, toml_exception, "toml serialize error exception",
                                      throw_toml_serialize_error);
}

#endif
