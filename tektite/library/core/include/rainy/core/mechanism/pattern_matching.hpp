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
#ifndef RAINY_CORE_MECHANISM_PATTERN_MATCHING_HPP
#define RAINY_CORE_MECHANISM_PATTERN_MATCHING_HPP
#include <rainy/core/mechanism/pattern_matching/expression.hpp>
#include <rainy/core/mechanism/pattern_matching/patterns.hpp>
#include <rainy/core/mechanism/pattern_matching/pre.hpp>

namespace rainy::core::mechanism::pattern_matching {
    using implements::_;
    using implements::and_;
    using implements::app;
    using implements::ds;
    using implements::id;
    using implements::meet;
    using implements::not_;
    using implements::or_;
    using implements::ooo;
    using implements::ooo_t;
    using implements::overload;
    using implements::pattern;
    using implements::pattern_helper;
    using implements::when;
    using implements::wildcard;
}

namespace rainy::mechanism {
    using core::mechanism::pattern_matching::_;
    using core::mechanism::pattern_matching::and_;
    using core::mechanism::pattern_matching::app;
    using core::mechanism::pattern_matching::ds;
    using core::mechanism::pattern_matching::id;
    using core::mechanism::pattern_matching::meet;
    using core::mechanism::pattern_matching::not_;
    using core::mechanism::pattern_matching::or_;
    using core::mechanism::pattern_matching::ooo;
    using core::mechanism::pattern_matching::ooo_t;
    using core::mechanism::pattern_matching::overload;
    using core::mechanism::pattern_matching::pattern;
    using core::mechanism::pattern_matching::pattern_helper;
    using core::mechanism::pattern_matching::when;
    using core::mechanism::pattern_matching::wildcard;
}

#endif
