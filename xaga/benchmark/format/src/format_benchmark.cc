#include <benchmark/benchmark.h>
#include <cstdio>
#include <format>
#include <iomanip>
#include <rainy/core/implements/text/format.hpp>
#include <sstream>
#include <string>
#include <vector>

// 测试简单字符串格式化
static void BM_StringFormat_Sprintf(benchmark::State &state) {
    char buffer[100];
    for (auto _: state) {
        sprintf(buffer, "Hello %s, number %d, float %.2f", "world", 42, 3.14159);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_StringFormat_Sprintf);

static void BM_StringFormat_StringStream(benchmark::State &state) {
    for (auto _: state) {
        std::stringstream ss;
        ss << "Hello " << "world" << ", number " << 42 << ", float " << std::fixed << std::setprecision(2) << 3.14159;
        std::string result = ss.str();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringFormat_StringStream);

static void BM_StringFormat_StdFormat(benchmark::State &state) {
    for (auto _: state) {
        auto result = std::format("Hello {}, number {}, float {:.2f}", "world", 42, 3.14159);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringFormat_StdFormat);

static void BM_StringFormat_RainyToolkitFormat(benchmark::State &state) {
    for (auto _: state) {
        auto result = rainy::foundation::text::format("Hello {}, number {}, float {:.2f}", "world", 42, 3.14159);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringFormat_RainyToolkitFormat);

// 测试复杂字符串格式化（更多参数）
static void BM_ComplexFormat_Sprintf(benchmark::State &state) {
    char buffer[200];
    const char *str1 = "first";
    const char *str2 = "second";
    int a = 123;
    int b = 456;
    double x = 3.14159;
    double y = 2.71828;

    for (auto _: state) {
        sprintf(buffer, "Strings: %s, %s; Ints: %d, %d; Floats: %.3f, %.3f", str1, str2, a, b, x, y);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_ComplexFormat_Sprintf);

static void BM_ComplexFormat_StdFormat(benchmark::State &state) {
    const char *str1 = "first";
    const char *str2 = "second";
    int a = 123;
    int b = 456;
    double x = 3.14159;
    double y = 2.71828;

    for (auto _: state) {
        auto result = std::format("Strings: {}, {}; Ints: {}, {}; Floats: {:.3f}, {:.3f}", str1, str2, a, b, x, y);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComplexFormat_StdFormat);

static void BM_ComplexFormat_RainyToolkitFormat(benchmark::State &state) {
    const char *str1 = "first";
    const char *str2 = "second";
    int a = 123;
    int b = 456;
    double x = 3.14159;
    double y = 2.71828;

    for (auto _: state) {
       auto result = rainy::foundation::text::format("Strings: {}, {}; Ints: {}, {}; Floats: {:.3f}, {:.3f}", str1, str2, a, b, x, y);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComplexFormat_RainyToolkitFormat);

// 测试整数格式化
static void BM_IntFormat_Sprintf(benchmark::State &state) {
    char buffer[50];
    int num = 123456;

    for (auto _: state) {
        sprintf(buffer, "%d", num);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_IntFormat_Sprintf);

static void BM_IntFormat_StdFormat(benchmark::State &state) {
    int num = 123456;

    for (auto _: state) {
        auto result = std::format("{}", num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_IntFormat_StdFormat);

static void BM_IntFormat_RainyToolkitFormat(benchmark::State &state) {
    int num = 123456;

    for (auto _: state) {
        auto result = rainy::foundation::text::format("{}", num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_IntFormat_RainyToolkitFormat);

// 测试浮点数格式化
static void BM_FloatFormat_Sprintf(benchmark::State &state) {
    char buffer[50];
    double pi = 3.14159265359;

    for (auto _: state) {
        sprintf(buffer, "%.6f", pi);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_FloatFormat_Sprintf);

static void BM_FloatFormat_StdFormat(benchmark::State &state) {
    double pi = 3.14159265359;

    for (auto _: state) {
        auto result = std::format("{:.6f}", pi);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FloatFormat_StdFormat);

static void BM_FloatFormat_RainyToolkitFormat(benchmark::State &state) {
    double pi = 3.14159265359;

    for (auto _: state) {
        auto result = rainy::foundation::text::format("{:.6f}", pi);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FloatFormat_RainyToolkitFormat);

// 测试带格式说明符的格式化
static void BM_FormattedInt_Sprintf(benchmark::State &state) {
    char buffer[50];
    int num = 42;

    for (auto _: state) {
        sprintf(buffer, "%08d", num); // 宽度8，前导零
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_FormattedInt_Sprintf);

static void BM_FormattedInt_StdFormat(benchmark::State &state) {
    int num = 42;

    for (auto _: state) {
        auto result = std::format("{:08}", num); // 宽度8，前导零
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FormattedInt_StdFormat);

static void BM_FormattedInt_RainyToolkitFormat(benchmark::State &state) {
    int num = 42;

    for (auto _: state) {
        auto result = rainy::foundation::text::format("{:08}", num); // 宽度8，前导零
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FormattedInt_RainyToolkitFormat);

// 测试十六进制格式化
static void BM_HexFormat_Sprintf(benchmark::State &state) {
    char buffer[50];
    int num = 0xABCD;

    for (auto _: state) {
        sprintf(buffer, "%X", num);
        benchmark::DoNotOptimize(buffer);
    }
}
BENCHMARK(BM_HexFormat_Sprintf);

static void BM_HexFormat_StdFormat(benchmark::State &state) {
    int num = 0xABCD;

    for (auto _: state) {
        auto result = std::format("{:X}", num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_HexFormat_StdFormat);

static void BM_HexFormat_RainyToolkitFormat(benchmark::State &state) {
    int num = 0xABCD;

    for (auto _: state) {
        auto result = rainy::foundation::text::format("{:X}", num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_HexFormat_RainyToolkitFormat);

// 批量测试不同大小的字符串
static void BM_StringSize_StdFormat(benchmark::State &state) {
    int size = state.range(0);
    std::string str(size, 'a');

    for (auto _: state) {
        auto result = std::format("Prefix: {} Suffix: {}", str, 123);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringSize_StdFormat)->RangeMultiplier(2)->Range(8, 1024);

static void BM_StringSize_RainyToolkitFormat(benchmark::State &state) {
    int size = state.range(0);
    std::string str(size, 'a');

    for (auto _: state) {
        auto result = std::format("Prefix: {} Suffix: {}", str, 123);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringSize_RainyToolkitFormat)->RangeMultiplier(2)->Range(8, 1024);

static void BM_StringSize_Sprintf(benchmark::State &state) {
    int size = state.range(0);
    std::string str(size, 'a');

    for (auto _: state) {
        char buffer[2048];
        int len = sprintf(buffer, "Prefix: %s Suffix: %d", str.c_str(), 123);
        std::string result(buffer, len); // 转换为 string，模拟其他方法的开销
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringSize_Sprintf)->RangeMultiplier(2)->Range(8, 1024);

// 测试重复格式化
static void BM_RepeatFormat_StdFormat(benchmark::State &state) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<std::string> results;
    results.reserve(numbers.size());

    for (auto _: state) {
        results.clear();
        for (int n: numbers) {
            results.push_back(std::format("Number: {:04}", n));
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_RepeatFormat_StdFormat);

static void BM_RepeatFormat_RainyToolkitFormat(benchmark::State &state) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<rainy::foundation::text::string> results;
    results.reserve(numbers.size());

    for (auto _: state) {
        results.clear();
        for (int n: numbers) {
            results.push_back(rainy::foundation::text::format("Number: {:04}", n));
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_RepeatFormat_RainyToolkitFormat);

static void BM_RepeatFormat_Sprintf(benchmark::State &state) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<std::string> results;
    results.reserve(numbers.size());
    char buffer[50];

    for (auto _: state) {
        results.clear();
        for (int n: numbers) {
            sprintf(buffer, "Number: %04d", n);
            results.push_back(buffer);
        }
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_RepeatFormat_Sprintf);
