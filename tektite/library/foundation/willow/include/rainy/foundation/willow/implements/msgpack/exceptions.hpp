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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_MSGPACK_EXCEPTIONS_HPP
#include <rainy/foundation/willow/implements/common/exceptions.hpp>
#include <rainy/foundation/willow/implements/msgpack/version.hpp>

#if RAINY_WILLOW_MSGPACK_AVAILABLE

namespace rainy::foundation::exceptions::willow::msgpack {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(msgpack_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "msgpack exception", throw_msgpack_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(msgpack_type_error, msgpack_exception, "msgpack type error exception",
                                      throw_msgpack_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(msgpack_parse_error, msgpack_exception, "msgpack parse error exception",
                                      throw_msgpack_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(msgpack_serialize_error, msgpack_exception, "msgpack serialize error exception",
                                      throw_msgpack_serialize_error);
}

#endif

#endif
