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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_EXCEPTIONS_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_CBOR_EXCEPTIONS_HPP
#include <rainy/foundation/willow/implements/cbor/version.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

#if RAINY_WILLOW_CBOR_AVAILABLE

namespace rainy::foundation::exceptions::willow::cbor {
    RAINY_DEFINE_EXCEPTION_WITH_THROW(cbor_exception, rainy::foundation::exceptions::willow::willow_exception,
                                      "cbor exception", throw_cbor_exception);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(cbor_type_error, cbor_exception, "cbor type error exception", throw_cbor_type_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(cbor_parse_error, cbor_exception, "cbor parse error exception", throw_cbor_parse_error);
    RAINY_DEFINE_EXCEPTION_WITH_THROW(cbor_serialize_error, cbor_exception, "cbor serialize error exception",
                                      throw_cbor_serialize_error);
}

#endif

#endif
