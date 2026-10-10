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
#include <rainy/foundation/willow/jsonnet.hpp>

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::jsonnet;

namespace {
    const rainy::core::text::string &small_jsonnet() {
        static const rainy::core::text::string text =
            "local port = 8080;\n"
            "{ host: \"example.com\", port: port, secure: true }\n";
        return text;
    }

    const rainy::core::text::string &realistic_jsonnet() {
        static const rainy::core::text::string text =
            "local defaults = { replicas: 2, image: \"app:1.0\", env: \"dev\" };\n"
            "local production = defaults + { replicas: 5, env: \"production\" };\n"
            "{\n"
            "  deployment: production,\n"
            "  total: production.replicas * 3,\n"
            "  tags: [\"web\", \"api\", \"worker\"],\n"
            "  limits: { memoryMB: 2048, cpu: 2 },\n"
            "  names: [std.char(65 + i) for i in [0, 1, 2, 3]],\n"
            "  summary: \"%s has %d replicas\" % [production.image, production.replicas],\n"
            "}\n";
        return text;
    }

    const rainy::core::text::string &large_jsonnet() {
        static const rainy::core::text::string text = [] {
            rainy::core::text::string source = "local base = { enabled: true, factor: 2 };\n{\n";
            for (int s = 0; s < 200; ++s) {
                source += "  section";
                source += std::to_string(s);
                source += ": base + { index: ";
                source += std::to_string(s);
                source += ", scaled: self.index * base.factor },\n";
            }
            source += "}\n";
            return source;
        }();
        return text;
    }

    facade<document> parse_cached(const rainy::core::text::string &source) {
        return jsonnet::parse(source);
    }
}

static void benchmark_jsonnet_parse_small(benchmark::State &state) {
    for (auto _: state) {
        auto doc = jsonnet::parse(small_jsonnet());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_jsonnet_parse_realistic(benchmark::State &state) {
    for (auto _: state) {
        auto doc = jsonnet::parse(realistic_jsonnet());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_jsonnet_parse_large(benchmark::State &state) {
    for (auto _: state) {
        auto doc = jsonnet::parse(large_jsonnet());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_jsonnet_evaluate_small(benchmark::State &state) {
    const auto doc = parse_cached(small_jsonnet());
    for (auto _: state) {
        auto result = evaluate(doc);
        benchmark::DoNotOptimize(result);
    }
}

static void benchmark_jsonnet_evaluate_realistic(benchmark::State &state) {
    const auto doc = parse_cached(realistic_jsonnet());
    for (auto _: state) {
        auto result = evaluate(doc);
        benchmark::DoNotOptimize(result);
    }
}

static void benchmark_jsonnet_evaluate_large(benchmark::State &state) {
    const auto doc = parse_cached(large_jsonnet());
    for (auto _: state) {
        auto result = evaluate(doc);
        benchmark::DoNotOptimize(result);
    }
}

static void benchmark_jsonnet_dump_ast(benchmark::State &state) {
    const auto doc = parse_cached(realistic_jsonnet());
    for (auto _: state) {
        auto text = dump_ast(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_jsonnet_dump_document(benchmark::State &state) {
    const auto doc = parse_cached(realistic_jsonnet());
    for (auto _: state) {
        auto text = jsonnet::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

BENCHMARK(benchmark_jsonnet_parse_small);
BENCHMARK(benchmark_jsonnet_parse_realistic);
BENCHMARK(benchmark_jsonnet_parse_large);
BENCHMARK(benchmark_jsonnet_evaluate_small);
BENCHMARK(benchmark_jsonnet_evaluate_realistic);
BENCHMARK(benchmark_jsonnet_evaluate_large);
BENCHMARK(benchmark_jsonnet_dump_ast);
BENCHMARK(benchmark_jsonnet_dump_document);
