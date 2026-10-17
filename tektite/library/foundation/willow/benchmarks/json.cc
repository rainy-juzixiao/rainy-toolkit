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
#include <rainy/foundation/willow/json.hpp>

#include <string>

using namespace rainy::foundation::willow;

namespace {
    const rainy::text::string &small_json() {
        static const rainy::text::string text =
            R"({"name":"rainy","version":3,"enabled":true,"score":1.25,"tags":["alpha","beta"],"meta":null})";
        return text;
    }

    const rainy::text::string &realistic_json() {
        static const rainy::text::string text = R"json(
{
  "metadata": {
    "version": "1.0.9-beta",
    "generatedAt": "2025-07-11T10:45:00Z",
    "source": "benchmark-suite-X",
    "config": {
      "threads": 16,
      "locale": "zh-CN",
      "featureFlags": ["experimental", "deep-nesting", "high-entropy"],
      "thresholds": {
        "memoryLimitMB": 2048,
        "timeoutMs": 30000
      }
    }
  },
  "payload": [
    {
      "id": "00001",
      "type": "user-event",
      "attributes": {
        "timestamp": 1720691100,
        "location": {
          "lat": 39.9042,
          "lon": 116.4074,
          "geoHash": "wx4g0ec1"
        },
        "actions": [
          {
            "actionType": "click",
            "target": "#submit-button",
            "meta": {
              "domPath": ["body", "div.main", "form", "button#submit-button"],
              "delayMs": 120
            }
          },
          {
            "actionType": "input",
            "target": "input[name='email']",
            "value": "test@example.com",
            "meta": {
              "valid": true,
              "charCount": 17
            }
          }
        ]
      },
      "checksum": "e3b0c44298fc1c149afbf4c8996"
    },
    {
      "id": "00002",
      "type": "sensor-data",
      "attributes": {
        "readings": {
          "temperature": [23.4, 23.6, 23.5, 23.7],
          "humidity": {
            "min": 41.2,
            "max": 48.5,
            "avg": 44.9
          },
          "vibration": {
            "x": [0.003, 0.002, 0.004],
            "y": [0.001, 0.002, 0.001],
            "z": [0.005, 0.006, 0.004]
          }
        },
        "timestamp": "2025-07-11T10:45:10.134Z",
        "status": "nominal"
      },
      "checksum": "c5f73a89c0df8ef243b9ab1e64b"
    },
    {
      "id": "00003",
      "type": "log-entry",
      "attributes": {
        "severity": "ERROR",
        "message": "Unhandled exception occurred",
        "stackTrace": [
          "at Module.run (core.js:151)",
          "at processTicksAndRejections (internal/process/task_queues.js:93:5)",
          "at async handleRequest (server.js:42:12)"
        ],
        "tags": ["backend", "critical", "retry"]
      },
      "checksum": "ffcc33ee991283aa3299f0912eb"
    }
  ],
  "statistics": {
    "totalEvents": 3,
    "errorCount": 1,
    "averageProcessingTimeMs": 27.4,
    "byType": {
      "user-event": {
        "count": 1,
        "avgSizeBytes": 512
      },
      "sensor-data": {
        "count": 1,
        "avgSizeBytes": 623
      },
      "log-entry": {
        "count": 1,
        "avgSizeBytes": 812
      }
    }
  }
}
)json";
        return text;
    }

    const rainy::text::string &large_array_json() {
        static const rainy::text::string text = [] {
            rainy::text::string source = "[";
            for (int i = 0; i < 2000; ++i) {
                if (i) {
                    source += ',';
                }
                source += std::to_string(i);
            }
            source += ']';
            return source;
        }();
        return text;
    }
}

static void benchmark_willow_parse_small(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = json::parse(small_json());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_willow_parse_realistic(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = json::parse(realistic_json());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_willow_parse_large_array(benchmark::State &state) {
    for (auto _ : state) {
        auto doc = json::parse(large_array_json());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_willow_dump_small(benchmark::State &state) {
    auto doc = json::parse(small_json());
    for (auto _ : state) {
        auto text = json::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_willow_dump_realistic(benchmark::State &state) {
    auto doc = json::parse(realistic_json());
    for (auto _ : state) {
        auto text = json::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_willow_dump_large_array(benchmark::State &state) {
    auto doc = json::parse(large_array_json());
    for (auto _ : state) {
        auto text = json::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

BENCHMARK(benchmark_willow_parse_small);
BENCHMARK(benchmark_willow_parse_realistic);
BENCHMARK(benchmark_willow_parse_large_array);
BENCHMARK(benchmark_willow_dump_small);
BENCHMARK(benchmark_willow_dump_realistic);
BENCHMARK(benchmark_willow_dump_large_array);
