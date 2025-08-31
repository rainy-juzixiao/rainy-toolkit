#include <rainy/asio.hpp>
#include <rainy/foundation/pal/asio/io_context.hpp>
#include <benchmark/benchmark.h>
#include <thread>
#include <vector>

// 基准测试函数：post N 个任务并运行它们
static void benchmark_asio_io_context_post_run_signlethread(benchmark::State &state) {
    for (auto _: state) {
        asio::io_context io_ctx;
        // 启动 io_context 的 run() 线程
        std::thread runner([&]() { io_ctx.run(); });
        // 提交任务
        for (int i = 0; i < state.range(0); ++i) {
            asio::post(io_ctx, []() {
                // 模拟轻量级任务
                benchmark::DoNotOptimize(1 + 1);
            });
        }
        runner.join();
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(benchmark_asio_io_context_post_run_signlethread)->Arg(1000)->Arg(10000)->Arg(100000);

// 多线程 run() 版本
static void benchmark_asio_io_context_post_run_multithread(benchmark::State &state) {
    for (auto _: state) {
        asio::io_context io_ctx;
        // 多线程运行 io_context
        int thread_count = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() { io_ctx.run(); });
        }

        // 提交任务
        for (int i = 0; i < state.range(0); ++i) {
            asio::post(io_ctx, []() { benchmark::DoNotOptimize(1 + 1); });
        }

        // 等待任务完成
        for (auto &t: threads) {
            t.join();
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(benchmark_asio_io_context_post_run_multithread)->Arg(1000)->Arg(10000)->Arg(100000);

// 基准测试函数：post N 个任务并运行它们
static void benchmark_rainytoolkit_io_context_post_run_signlethread(benchmark::State &state) {
    for (auto _: state) {
        rainy::foundation::pal::asio::io_context io_ctx;
        std::thread runner([&]() { io_ctx.run(); });
        for (int i = 0; i < state.range(0); ++i) {
            io_ctx.post(
                [](void *) {
                    // 模拟轻量级任务
                    benchmark::DoNotOptimize(1 + 1);
                },
                nullptr);
        }
        runner.join();
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(benchmark_rainytoolkit_io_context_post_run_signlethread)->Arg(1000)->Arg(10000)->Arg(100000);

// 多线程 run() 版本
static void benchmark_rainytoolkit_io_context_post_run_multithread(benchmark::State &state) {
    for (auto _: state) {
        rainy::foundation::pal::asio::io_context io_ctx;
        int thread_count = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() { io_ctx.run(); });
        }
        for (int i = 0; i < state.range(0); ++i) {
            io_ctx.post([](void *) { benchmark::DoNotOptimize(1 + 1); }, nullptr);
        }
        for (auto &t: threads) {
            t.join();
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(benchmark_rainytoolkit_io_context_post_run_multithread)->Arg(1000)->Arg(10000)->Arg(100000);


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
    ::benchmark::SetDefaultTimeUnit(benchmark::TimeUnit::kMillisecond);
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}