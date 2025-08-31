#include <any>
#include <benchmark/benchmark.h>
#include <rainy/utility/any.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <entt/entt.hpp>

static void benchmark_standard_any_construct_int(benchmark::State &state) {
    for (auto _: state) {
        std::any a = 42;
        benchmark::DoNotOptimize(a);
    }
}

BENCHMARK(benchmark_standard_any_construct_int);

static void benchmark_standard_any_construct_string(benchmark::State &state) {
    for (auto _: state) {
        std::any a = std::string("hello world");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(benchmark_standard_any_construct_string);

static void benchmark_standard_any_cast_int(benchmark::State &state) {
    std::any a = 123;
    for (auto _: state) {
        int value = std::any_cast<int>(a);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(benchmark_standard_any_cast_int);

static void benchmark_standard_any_copy(benchmark::State &state) {
    std::any a = std::vector{1, 2, 3, 4, 5};
    for (auto _: state) {
        std::any b = a;
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(benchmark_standard_any_copy);

static void benchmark_standard_any_move(benchmark::State &state) {
    for (auto _: state) {
        std::any a = std::vector{1, 2, 3, 4, 5};
        std::any b = std::move(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(benchmark_standard_any_move);

static void benchmark_rainytoolkit_any_construct_int(benchmark::State &state) {
    for (auto _: state) {
        rainy::utility::any a = 42;
        benchmark::DoNotOptimize(a);
    }
}

BENCHMARK(benchmark_rainytoolkit_any_construct_int);

static void benchmark_rainytoolkit_any_construct_string(benchmark::State &state) {
    for (auto _: state) {
        rainy::utility::any a = std::string("hello world");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(benchmark_rainytoolkit_any_construct_string);

static void benchmark_rainytoolkit_any_cast_int(benchmark::State &state) {
    rainy::utility::any a = 123;
    for (auto _: state) {
        int value = a.as<int>();
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(benchmark_rainytoolkit_any_cast_int);

static void benchmark_rainytoolkit_any_copy(benchmark::State &state) {
    rainy::utility::any a = std::vector<int>{1, 2, 3, 4, 5};
    for (auto _: state) {
        rainy::utility::any b = a;
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(benchmark_rainytoolkit_any_copy);

static void benchmark_rainytoolkit_any_move(benchmark::State &state) {
    for (auto _: state) {
        rainy::utility::any a = std::vector<int>{1, 2, 3, 4, 5};
        rainy::utility::any b = std::move(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(benchmark_rainytoolkit_any_move);

static void benchmark_entt_any_construct_int(benchmark::State &state) {
    for (auto _: state) {
        entt::any a = 42;
        benchmark::DoNotOptimize(a);
    }
}

BENCHMARK(benchmark_entt_any_construct_int);

static void benchmark_entt_any_construct_string(benchmark::State &state) {
    for (auto _: state) {
        entt::any a = std::string("hello world");
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(benchmark_entt_any_construct_string);

static void benchmark_entt_any_cast_int(benchmark::State &state) {
    entt::any a = 123;
    for (auto _: state) {
        int value = entt::any_cast<int>(a);
        benchmark::DoNotOptimize(value);
    }
}
BENCHMARK(benchmark_entt_any_cast_int);

static void benchmark_entt_any_copy(benchmark::State &state) {
    entt::any a = std::vector<int>{1, 2, 3, 4, 5};
    for (auto _: state) {
        entt::any b = a;
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(benchmark_entt_any_copy);

static void benchmark_entt_any_move(benchmark::State &state) {
    for (auto _: state) {
        entt::any a = std::vector<int>{1, 2, 3, 4, 5};
        entt::any b = std::move(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(benchmark_entt_any_move);

static void base_line_int(benchmark::State &state) {
    for (auto _: state) {
        int a = 42;
        int b = a;
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(base_line_int);

int main(int argc, char **argv) {
    char arg0_default[] = "benchmark";
    char *args_default = arg0_default;
    if (!argv) {
        argc = 1;
        argv = &args_default;
    }
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
