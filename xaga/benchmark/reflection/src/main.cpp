#include <benchmark/benchmark.h>
#include <UDRefl/UDRefl.hpp>

int main(int argc, char **argv) {
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
