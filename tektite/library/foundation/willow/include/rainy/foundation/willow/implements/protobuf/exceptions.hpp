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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_EXCEPTIONS_HPP
#include <rainy/foundation/willow/implements/common/exceptions.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::exceptions::willow::protobuf {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(protobuf_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "protobuf exception", throw_protobuf_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(protobuf_type_error, protobuf_exception, "protobuf type error exception",
                                      throw_protobuf_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(protobuf_parse_error, protobuf_exception, "protobuf parse error exception",
                                      throw_protobuf_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(protobuf_serialize_error, protobuf_exception, "protobuf serialize error exception",
                                      throw_protobuf_serialize_error);
}

#endif

#endif
