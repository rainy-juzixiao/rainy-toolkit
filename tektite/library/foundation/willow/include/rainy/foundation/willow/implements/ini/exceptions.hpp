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
#ifndef RAINY_FOUNDATION_WILLOW_INI_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_INI_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::exceptions::willow::ini {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(ini_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "ini exception", throw_ini_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(ini_type_error, ini_exception, "ini type error exception", throw_ini_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(ini_invalid_key, ini_exception, "ini invalid key exception", throw_ini_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(ini_invalid_iterator, ini_exception, "ini invalid iterator exception",
                                      throw_ini_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(ini_parse_error, ini_exception, "ini parse error exception", throw_ini_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(ini_serialize_error, ini_exception, "ini serialize error exception",
                                      throw_ini_serialize_error);
}

#endif
