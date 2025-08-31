#define NOMINMAX
#include <benchmark/benchmark.h>
#include <rainy/component/willow/json.hpp>
#include <rapidjson.h>
#include <prettywriter.h>
#include <document.h>
#include "include/configor/json.hpp"
#include "json.hpp"

std::string_view benchmark_json = R"json(
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

static void benchmark_rainytoolkit_parse_json(benchmark::State &state) {
    for (auto _: state) {
        auto parsed = rainy::component::willow::json::parse(benchmark_json);
        benchmark::DoNotOptimize(parsed);
    }
}

static void benchmark_rainytoolkit_read_metadata_version(benchmark::State &state) {
    auto parsed = rainy::component::willow::json::parse(benchmark_json);
    for (auto _: state) {
        auto version = parsed["metadata"]["version"].as_string();
        benchmark::DoNotOptimize(version);
    }
}

static void benchmark_rainytoolkit_read_first_event_type(benchmark::State &state) {
    auto parsed = rainy::component::willow::json::parse(benchmark_json);
    for (auto _: state) {
        auto type = parsed["payload"][0]["type"].as_string();
        benchmark::DoNotOptimize(type);
    }
}

static void benchmark_rainytoolkit_read_temperature_array(benchmark::State &state) {
    auto parsed = rainy::component::willow::json::parse(benchmark_json);
    for (auto _: state) {
        auto temps = parsed["payload"][1]["attributes"]["readings"]["temperature"];
        benchmark::DoNotOptimize(temps);
    }
}

static void benchmark_rainytoolkit_read_stack_trace_strings(benchmark::State &state) {
    auto parsed = rainy::component::willow::json::parse(benchmark_json);
    for (auto _: state) {
        for (const auto &frame: parsed["payload"][2]["attributes"]["stackTrace"]) {
            benchmark::DoNotOptimize(frame.value());
        }
    }
}

static void benchmark_rainytoolkit_read_feature_flags(benchmark::State &state) {
    auto parsed = rainy::component::willow::json::parse(benchmark_json);
    for (auto _: state) {
        auto flags = parsed["metadata"]["config"]["featureFlags"];
        for (const auto &flag: flags) {
            benchmark::DoNotOptimize(flag.value());
        }
    }
}

static void benchmark_rainytoolkit_write_json_string(benchmark::State &state) {
    auto parsed = rainy::component::willow::json::parse(benchmark_json);
    for (auto _: state) {
        benchmark::DoNotOptimize(parsed.dump(4));
    }
}

BENCHMARK(benchmark_rainytoolkit_parse_json);
BENCHMARK(benchmark_rainytoolkit_read_metadata_version);
BENCHMARK(benchmark_rainytoolkit_read_first_event_type);
BENCHMARK(benchmark_rainytoolkit_read_temperature_array);
BENCHMARK(benchmark_rainytoolkit_read_stack_trace_strings);
BENCHMARK(benchmark_rainytoolkit_read_feature_flags);
BENCHMARK(benchmark_rainytoolkit_write_json_string);

static void benchmark_nlohmann_parse_json(benchmark::State &state) {
    for (auto _: state) {
        auto parsed = nlohmann::json::parse(benchmark_json);
        benchmark::DoNotOptimize(parsed);
    }
}

static void benchmark_nlohmann_read_metadata_version(benchmark::State &state) {
    auto parsed = nlohmann::json::parse(benchmark_json);
    for (auto _: state) {
        auto version = parsed["metadata"]["version"].get<std::string>();
        benchmark::DoNotOptimize(version);
    }
}

static void benchmark_nlohmann_read_first_event_type(benchmark::State &state) {
    auto parsed = nlohmann::json::parse(benchmark_json);
    for (auto _: state) {
        auto type = parsed["payload"][0]["type"].get<std::string>();
        benchmark::DoNotOptimize(type);
    }
}

static void benchmark_nlohmann_read_temperature_array(benchmark::State &state) {
    auto parsed = nlohmann::json::parse(benchmark_json);
    for (auto _: state) {
        auto temps = parsed["payload"][1]["attributes"]["readings"]["temperature"];
        benchmark::DoNotOptimize(temps);
    }
}

static void benchmark_nlohmann_read_stack_trace_strings(benchmark::State &state) {
    auto parsed = nlohmann::json::parse(benchmark_json);
    for (auto _: state) {
        for (const auto &frame: parsed["payload"][2]["attributes"]["stackTrace"]) {
            benchmark::DoNotOptimize(frame.get<std::string>());
        }
    }
}

static void benchmark_nlohmann_read_feature_flags(benchmark::State &state) {
    auto parsed = nlohmann::json::parse(benchmark_json);
    for (auto _: state) {
        auto flags = parsed["metadata"]["config"]["featureFlags"];
        for (const auto &flag: flags) {
            benchmark::DoNotOptimize(flag.get<std::string>());
        }
    }
}

static void benchmark_nlohmann_write_json_string(benchmark::State &state) {
    auto parsed = nlohmann::json::parse(benchmark_json.data());
    for (auto _: state) {
        benchmark::DoNotOptimize(parsed.dump(4));
    }
}
BENCHMARK(benchmark_nlohmann_parse_json);
BENCHMARK(benchmark_nlohmann_read_metadata_version);
BENCHMARK(benchmark_nlohmann_read_first_event_type);
BENCHMARK(benchmark_nlohmann_read_temperature_array);
BENCHMARK(benchmark_nlohmann_read_stack_trace_strings);
BENCHMARK(benchmark_nlohmann_read_feature_flags);
BENCHMARK(benchmark_nlohmann_write_json_string);

static void benchmark_configor_parse_json(benchmark::State &state) {
    for (auto _: state) {
        auto parsed = configor::json::parse(benchmark_json.data());
        benchmark::DoNotOptimize(parsed);
    }
}

static void benchmark_configor_read_metadata_version(benchmark::State &state) {
    auto parsed = configor::json::parse(benchmark_json.data());
    for (auto _: state) {
        auto version = parsed["metadata"]["version"].get<std::string>();
        benchmark::DoNotOptimize(version);
    }
}

static void benchmark_configor_read_first_event_type(benchmark::State &state) {
    auto parsed = configor::json::parse(benchmark_json.data());
    for (auto _: state) {
        auto type = parsed["payload"][0]["type"].get<std::string>();
        benchmark::DoNotOptimize(type);
    }
}

static void benchmark_configor_read_temperature_array(benchmark::State &state) {
    auto parsed = configor::json::parse(benchmark_json.data());
    for (auto _: state) {
        auto temps = parsed["payload"][1]["attributes"]["readings"]["temperature"];
        benchmark::DoNotOptimize(temps);
    }
}

static void benchmark_configor_read_stack_trace_strings(benchmark::State &state) {
    auto parsed = configor::json::parse(benchmark_json.data());
    for (auto _: state) {
        for (const auto &frame: parsed["payload"][2]["attributes"]["stackTrace"]) {
            benchmark::DoNotOptimize(frame.get<std::string>());
        }
    }
}

static void benchmark_configor_read_feature_flags(benchmark::State &state) {
    auto parsed = configor::json::parse(benchmark_json.data());
    for (auto _: state) {
        auto flags = parsed["metadata"]["config"]["featureFlags"];
        for (const auto &flag: flags) {
            benchmark::DoNotOptimize(flag.get<std::string>());
        }
    }
}

static void benchmark_configor_write_json_string(benchmark::State &state) {
    auto parsed = configor::json::parse(benchmark_json.data());
    for (auto _: state) {
        benchmark::DoNotOptimize(configor::json::dump(parsed));
    }
}
BENCHMARK(benchmark_configor_parse_json);
BENCHMARK(benchmark_configor_read_first_event_type);
BENCHMARK(benchmark_configor_read_temperature_array);
BENCHMARK(benchmark_configor_read_metadata_version);
BENCHMARK(benchmark_configor_read_stack_trace_strings);
BENCHMARK(benchmark_configor_read_feature_flags);
BENCHMARK(benchmark_configor_write_json_string);

/// rapidjson

static void benchmark_rapidjson_parse_json(benchmark::State &state) {
    for (auto _: state) {
        rapidjson::Document doc;
        doc.Parse(benchmark_json.data());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_rapidjson_read_metadata_version(benchmark::State &state) {
    rapidjson::Document doc;
    doc.Parse(benchmark_json.data());
    for (auto _: state) {
        auto version = doc["metadata"]["version"].GetString();
        benchmark::DoNotOptimize(version);
    }
}

static void benchmark_rapidjson_read_first_event_type(benchmark::State &state) {
    rapidjson::Document doc;
    doc.Parse(benchmark_json.data());
    for (auto _: state) {
        auto type = doc["payload"][0]["type"].GetString();
        benchmark::DoNotOptimize(type);
    }
}

static void benchmark_rapidjson_read_temperature_array(benchmark::State &state) {
    rapidjson::Document doc;
    doc.Parse(benchmark_json.data());
    for (auto _: state) {
        auto temps = doc["payload"][1]["attributes"]["readings"]["temperature"].GetArray();
        benchmark::DoNotOptimize(temps);
    }
}

static void benchmark_rapidjson_read_stack_trace_strings(benchmark::State &state) {
    rapidjson::Document doc;
    doc.Parse(benchmark_json.data());
    for (auto _: state) {
        for (const auto &frame: doc["payload"][2]["attributes"]["stackTrace"].GetArray()) {
            benchmark::DoNotOptimize(frame.GetString());
        }
    }
}

static void benchmark_rapidjson_read_feature_flags(benchmark::State &state) {
    rapidjson::Document doc;
    doc.Parse(benchmark_json.data());
    for (auto _: state) {
        auto flags = doc["metadata"]["config"]["featureFlags"].GetArray();
        for (const auto &flag: flags) {
            benchmark::DoNotOptimize(flag.GetString());
        }
    }
}

static void benchmark_rapidjson_write_json_string(benchmark::State &state) {
    rapidjson::Document doc;
    doc.Parse(benchmark_json.data());
    for (auto _: state) {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter writer(buffer);
        doc.Accept(writer);
        benchmark::DoNotOptimize(buffer);
    }
}

BENCHMARK(benchmark_rapidjson_parse_json);
BENCHMARK(benchmark_rapidjson_read_metadata_version);
BENCHMARK(benchmark_rapidjson_read_first_event_type);
BENCHMARK(benchmark_rapidjson_read_temperature_array);
BENCHMARK(benchmark_rapidjson_read_stack_trace_strings);
BENCHMARK(benchmark_rapidjson_read_feature_flags);
BENCHMARK(benchmark_rapidjson_write_json_string);

int main(int argc, char **argv) {
    char arg0_default[] = "benchmark";
    char *args_default = arg0_default;
    if (!argv) {
        argc = 1;
        argv = &args_default;
    }
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
