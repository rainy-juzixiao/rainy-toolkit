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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::foundation::exceptions::willow {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(willow_exception, core::exceptions::runtime::runtime_error, "willow exception",
                                      throw_willow_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(willow_type_error, willow_exception, "willow type error exception",
                                      throw_willow_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(willow_invalid_key, willow_exception, "willow invalid key exception",
                                      throw_willow_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(willow_invalid_iterator, willow_exception, "willow invalid iterator exception",
                                      throw_willow_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(willow_parse_error, willow_exception, "willow parse error exception",
                                      throw_willow_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(willow_serialize_error, willow_exception, "willow serialize error exception",
                                      throw_willow_serialize_error);
}

#endif
