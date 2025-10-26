#include <any>
#include <benchmark/benchmark.h>
#include <rainy/utility/any.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <entt/entt.hpp>

struct structure {
    int a{};
    char b{};
    std::string c{};
};

struct mypair {
    int data1{};
    std::string_view data2{};
};

using trivial = std::array<int, 2>;
static_assert(std::is_trivially_copyable_v<trivial>);

struct small {
    std::array<int, 4> c{};
    small() = default;
    small(const small &) = default;
    small &operator=(const small &) = default;
    small(small &&) noexcept = default;
    small &operator=(small &&) noexcept = default;
    ~small() {
    }
};
static_assert(!std::is_trivially_copyable_v<small>);
static_assert(std::is_nothrow_move_constructible_v<small>);

using large = std::array<int, 32>;

template <class T>
void benchmark_standard_any_swap(benchmark::State &state) {
    std::any a = T{};
    std::any b = T{};

    for (auto _: state) {
        a.swap(b);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}

template <class T>
void benchmark_rainytoolkit_any_swap(benchmark::State &state) {
    rainy::utility::any a = T{};
    rainy::utility::any b = T{};

    for (auto _: state) {
        a.swap(b);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}

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

BENCHMARK(benchmark_standard_any_swap<trivial>);
BENCHMARK(benchmark_standard_any_swap<small>);
BENCHMARK(benchmark_standard_any_swap<large>);

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

BENCHMARK(benchmark_rainytoolkit_any_swap<trivial>);
BENCHMARK(benchmark_rainytoolkit_any_swap<small>);
BENCHMARK(benchmark_rainytoolkit_any_swap<large>);

static void benchmark_rainytoolkit_any_match(benchmark::State &state) {
    using namespace rainy::utility;
    any a;
    static auto f = [&a]() {
        a.match([](int a) { benchmark::DoNotOptimize(a); }, [](std::optional<char> a) { benchmark::DoNotOptimize(a); },
                [](std::string_view s) { benchmark::DoNotOptimize(s); },
                [](any::default_match, const any &v) { benchmark::DoNotOptimize(v); });
    };
    for (auto _: state) {
        a = 10;
        f();
        a = std::optional<char>{'c'};
        f();
        a = "Hello World";
        f();
        a = std::vector<int>{};
        f();
    }
}

BENCHMARK(benchmark_rainytoolkit_any_match);

static void benchmark_rainytoolkit_any_destructure_handler(benchmark::State &state) {
    using namespace rainy::utility;
    any a = structure{10, 'c', "Hello World"};
    for (auto _: state) {
        a.destructure([](int a, char b, const std::string& c) {
            benchmark::DoNotOptimize(a);
            benchmark::DoNotOptimize(b);
            benchmark::DoNotOptimize(c);
        });
    }
}

BENCHMARK(benchmark_rainytoolkit_any_destructure_handler);

static void benchmark_rainytoolkit_any_destructure_tuple(benchmark::State &state) {
    using namespace rainy::utility;
    any a = structure{10, 'c', "Hello World"};
    for (auto _: state) {
        std::tuple<int, char, std::string> tuple;
        a.destructure(tuple);
        benchmark::DoNotOptimize(tuple);
    }
}

BENCHMARK(benchmark_rainytoolkit_any_destructure_tuple);

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
