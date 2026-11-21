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
#include <benchmark/benchmark.h>
#include <rainy/foundation/willow/json5.hpp>

using namespace rainy::foundation::willow;

namespace {
    const rainy::text::string &json5_payload() {
        static const rainy::text::string text = R"({
            // configuration
            host: 'localhost',
            port: 0x1F90,
            ratios: [1, .5, -2.,],
            floor: -Infinity,
            placeholder: NaN,
            "quoted-key": true,
            nested: {a: {b: [1, {c: 'd'},],},},
        })";
        return text;
    }
}

static void benchmark_json5_parse(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = json5::parse(json5_payload());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_json5_dump(benchmark::State &state) {
    auto doc = json5::parse(json5_payload());
    for (auto _ : state) {
        auto text = json5::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

BENCHMARK(benchmark_json5_parse);
BENCHMARK(benchmark_json5_dump);
