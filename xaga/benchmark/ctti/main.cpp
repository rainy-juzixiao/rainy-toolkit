#include <benchmark/benchmark.h>
#include <rainy/foundation/typeinfo.hpp>
#include <string>
#include <typeinfo>

struct simple_type {
    int x;
    double y;
};

static void rtti_on_type(benchmark::State &state) {
    for (auto _: state) {
        const std::type_info &info = typeid(simple_type);
        benchmark::DoNotOptimize(info);
    }
}
BENCHMARK(rtti_on_type);

static void rtti_on_instance(benchmark::State &state) {
    simple_type obj{42, 3.14};
    for (auto _: state) {
        const std::type_info &info = typeid(obj);
        benchmark::DoNotOptimize(info);
    }
}
BENCHMARK(rtti_on_instance);

static void rtti_name_lookup(benchmark::State &state) {
    simple_type obj{42, 3.14};
    for (auto _: state) {
        const char *name = typeid(obj).name();
        benchmark::DoNotOptimize(name);
    }
}
BENCHMARK(rtti_name_lookup);

static void rtti_hash_lookup(benchmark::State &state) {
    simple_type obj{42, 3.14};
    for (auto _: state) {
        std::size_t hash = typeid(obj).hash_code();
        benchmark::DoNotOptimize(hash);
    }
}
BENCHMARK(rtti_hash_lookup);


static void ctti_on_type(benchmark::State &state) {
    for (auto _: state) {
        const rainy::foundation::ctti::typeinfo &info = rainy_typeid(simple_type);
        benchmark::DoNotOptimize(info);
    }
}
BENCHMARK(ctti_on_type);

static void ctti_on_instance(benchmark::State &state) {
    simple_type obj{42, 3.14};
    for (auto _: state) {
        const rainy::foundation::ctti::typeinfo &info = rainy_typeid(decltype(obj));
        benchmark::DoNotOptimize(info);
    }
}
BENCHMARK(ctti_on_instance);

static void ctti_name_lookup(benchmark::State &state) {
    simple_type obj{42, 3.14};
    for (auto _: state) {
        auto name = rainy_typeid(decltype(obj)).name();
        benchmark::DoNotOptimize(name);
    }
}
BENCHMARK(ctti_name_lookup);

static void ctti_hash_lookup(benchmark::State &state) {
    simple_type obj{42, 3.14};
    for (auto _: state) {
        std::size_t hash = rainy_typeid(decltype(obj)).hash_code();
        benchmark::DoNotOptimize(hash);
    }
}
BENCHMARK(ctti_hash_lookup);

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