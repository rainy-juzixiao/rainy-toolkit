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
#ifndef RAINY_FOUNDATION_WILLOW_YAML_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_YAML_EXCEPTIONS_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::exceptions::willow::yaml {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_exception, rainy::foundation::exceptions::willow::willow_exception, "yaml exception",
                                      throw_yaml_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_type_error, yaml_exception, "yaml type error exception", throw_yaml_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_invalid_key, yaml_exception, "yaml invalid key exception", throw_yaml_invalid_key);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_invalid_iterator, yaml_exception, "yaml invalid iterator exception",
                                      throw_yaml_invalid_iterator);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_parse_error, yaml_exception, "yaml parse error exception", throw_yaml_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_serialize_error, yaml_exception, "yaml serialize error exception",
                                      throw_yaml_serialize_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_indentation_error, yaml_exception, "yaml indentation error exception",
                                      throw_yaml_indentation_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_anchor_error, yaml_exception, "yaml anchor error exception", throw_yaml_anchor_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_alias_error, yaml_exception, "yaml alias error exception", throw_yaml_alias_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_tag_error, yaml_exception, "yaml tag error exception", throw_yaml_tag_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_directive_error, yaml_exception, "yaml directive error exception",
                                      throw_yaml_directive_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_scalar_error, yaml_exception, "yaml scalar error exception", throw_yaml_scalar_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(yaml_document_error, yaml_exception, "yaml document error exception", throw_yaml_document_error);
}

#endif
