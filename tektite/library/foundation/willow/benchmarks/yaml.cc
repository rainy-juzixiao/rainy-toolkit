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
#include <rainy/foundation/willow/yaml.hpp>

#include <string>

using namespace rainy::foundation::willow;

namespace {
    const rainy::text::string &small_yaml() {
        static const rainy::text::string text =
            "name: rainy\n"
            "version: 3\n"
            "enabled: true\n"
            "score: 1.25\n"
            "tags: [alpha, beta]\n"
            "meta: null\n";
        return text;
    }

    const rainy::text::string &realistic_yaml() {
        static const rainy::text::string text = R"(metadata:
  version: "1.0.9-beta"
  generatedAt: "2025-07-11T10:45:00Z"
  source: benchmark-suite-X
  config:
    threads: 16
    locale: zh-CN
    featureFlags: [experimental, deep-nesting, high-entropy]
    thresholds:
      memoryLimitMB: 2048
      timeoutMs: 30000
payload:
  - id: "00001"
    type: user-event
    attributes:
      timestamp: 1720691100
      location:
        lat: 39.9042
        lon: 116.4074
        geoHash: wx4g0ec1
      actions:
        - actionType: click
          target: "#submit-button"
          delayMs: 120
        - actionType: input
          target: input-email
          value: "test@example.com"
          valid: true
    checksum: e3b0c44298fc1c149afbf4c8996
  - id: "00002"
    type: sensor-data
    attributes:
      readings:
        temperature: [23.4, 23.6, 23.5, 23.7]
        humidity:
          min: 41.2
          max: 48.5
          avg: 44.9
      status: nominal
statistics:
  totalEvents: 2
  errorCount: 0
  averageProcessingTimeMs: 27.4
)";
        return text;
    }

    const rainy::text::string &large_sequence_yaml() {
        static const rainy::text::string text = [] {
            rainy::text::string source;
            for (int i = 0; i < 2000; ++i) {
                source += "- ";
                source += std::to_string(i);
                source += '\n';
            }
            return source;
        }();
        return text;
    }
}

static void benchmark_yaml_parse_small(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = yaml::parse(small_yaml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_yaml_parse_realistic(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = yaml::parse(realistic_yaml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_yaml_parse_large_sequence(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = yaml::parse(large_sequence_yaml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_yaml_dump_small(benchmark::State &state) {
    auto doc = yaml::parse(small_yaml());
    for (auto _ : state) {
        auto text = yaml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_yaml_dump_realistic(benchmark::State &state) {
    auto doc = yaml::parse(realistic_yaml());
    for (auto _ : state) {
        auto text = yaml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_yaml_dump_large_sequence(benchmark::State &state) {
    auto doc = yaml::parse(large_sequence_yaml());
    for (auto _ : state) {
        auto text = yaml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

BENCHMARK(benchmark_yaml_parse_small);
BENCHMARK(benchmark_yaml_parse_realistic);
BENCHMARK(benchmark_yaml_parse_large_sequence);
BENCHMARK(benchmark_yaml_dump_small);
BENCHMARK(benchmark_yaml_dump_realistic);
BENCHMARK(benchmark_yaml_dump_large_sequence);
