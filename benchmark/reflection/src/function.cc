#define NOMINMAX
#include <benchmark/benchmark.h>
#include <chrono>
#include <rainy/meta/reflection/function.hpp>
#include <rttr/registration.h>

class test {
public:
    void test_conv1(float n) {
        static float c;
        c += n;
    }

    void test_conv2(float n, float m) {
        static float c;
        c += n + m;
    }

    void test_conv3(float n, float m, float o) {
        static float c;
        c += n + m + o;
    }

    void test_conv4(float n, float m, float o, float p) {
        static float c;
        c += n + m + o + p;
    }

    void test_conv5(float n, float m, float o, float p, float q) {
        static float c;
        c += n + m + o + p + q;
    }

    void test_conv6(float n, float m, float o, float p, float q, float a1) {
        static float c;
        c += n + m + o + p + q +a1;
    }
};

RTTR_REGISTRATION {
    using namespace rttr;
    registration::class_<test>("test")
        .constructor()
        .method("test_conv1", &test::test_conv1)
        .method("test_conv2", &test::test_conv2)
        .method("test_conv3", &test::test_conv3)
        .method("test_conv4", &test::test_conv4)
        .method("test_conv5", &test::test_conv5)
        .method("test_conv6", &test::test_conv6);
}

static void BM_rainytoolkit_reflection_invoke_variadic_method(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv1;
        static test object_x;
        test_conv_rf.invoke_variadic(object_x, {10.0f});
    }
}

static void BM_rttr_reflection_invoke_variadic_method(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv1 = type.get_method("test_conv1");
        test object_x;
        test_conv1.invoke_variadic(object_x, {10.0f});
    }
}

static void BM_rainytoolkit_reflection_invoke_variadic_method_2(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv2;
        static test object_x;
        test_conv_rf.invoke_variadic(object_x, {10.0f, 20.0f});
    }
}

static void BM_rttr_reflection_invoke_variadic_method_2(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv2 = type.get_method("test_conv2");
        test object_x;
        test_conv2.invoke_variadic(object_x, {10.0f, 20.0f});
    }
}

static void BM_rainytoolkit_reflection_invoke_variadic_method_3(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv3;
        static test object_x;
        test_conv_rf.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f});
    }
}

static void BM_rttr_reflection_invoke_variadic_method_3(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv3 = type.get_method("test_conv3");
        test object_x;
        test_conv3.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f});
    }
}

static void BM_rainytoolkit_reflection_invoke_variadic_method_4(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv4;
        static test object_x;
        test_conv_rf.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f});
    }
}

static void BM_rttr_reflection_invoke_variadic_method_4(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv4 = type.get_method("test_conv4");
        test object_x;
        test_conv4.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f});
    }
}

static void BM_rainytoolkit_reflection_invoke_variadic_method_5(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv5;
        static test object_x;
        test_conv_rf.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f, 50.0f});
    }
}

static void BM_rttr_reflection_invoke_variadic_method_5(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv5 = type.get_method("test_conv5");
        test object_x;
        test_conv5.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f, 50.0f});
    }
}

static void BM_rainytoolkit_reflection_invoke_variadic_method_6(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv6;
        static test object_x;
        test_conv_rf.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});
    }
}

static void BM_rttr_reflection_invoke_variadic_method_6(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv6 = type.get_method("test_conv6");
        test object_x;
        test_conv6.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});
    }
}

BENCHMARK(BM_rainytoolkit_reflection_invoke_variadic_method)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_variadic_method)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_variadic_method_2)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_variadic_method_2)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_variadic_method_3)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_variadic_method_3)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_variadic_method_4)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_variadic_method_4)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_variadic_method_5)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_variadic_method_5)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_variadic_method_6)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_variadic_method_6)->Repetitions(2);

static void BM_rainytoolkit_reflection_invoke_method(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv1;
        static test object_x;
        test_conv_rf.oinvoke(object_x, 10.0f);
    }
}

static void BM_rttr_reflection_invoke_method(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv1 = type.get_method("test_conv1");
        test object_x;
        test_conv1.invoke(object_x, 10.0f);
    }
}

static void BM_rainytoolkit_reflection_invoke_method_2(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv2;
        static test object_x;
        test_conv_rf.oinvoke(object_x, 10.0f, 20.0f);
    }
}

static void BM_rttr_reflection_invoke_method_2(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv2 = type.get_method("test_conv2");
        test object_x;
        test_conv2.invoke(object_x, 10.0f, 20.0f);
    }
}

static void BM_rainytoolkit_reflection_invoke_method_3(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv3;
        static test object_x;
        test_conv_rf.oinvoke(object_x, 10.0f, 20.0f, 30.0f);
    }
}

static void BM_rttr_reflection_invoke_method_3(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv3 = type.get_method("test_conv3");
        test object_x;
        test_conv3.invoke(object_x, 10.0f, 20.0f, 30.0f);
    }
}

static void BM_rainytoolkit_reflection_invoke_method_4(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv4;
        static test object_x;
        test_conv_rf.oinvoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f);
    }
}

static void BM_rttr_reflection_invoke_method_4(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv4 = type.get_method("test_conv4");
        test object_x;
        test_conv4.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f);
    }
}

static void BM_rainytoolkit_reflection_invoke_method_5(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv5;
        static test object_x;
        test_conv_rf.oinvoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f);
    }
}

static void BM_rttr_reflection_invoke_method_5(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv5 = type.get_method("test_conv5");
        test object_x;
        test_conv5.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f);
    }
}

static void BM_rainytoolkit_reflection_invoke_method_6(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function test_conv_rf = &test::test_conv6;
        static test object_x;
        test_conv_rf.oinvoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f);
    }
}

static void BM_rttr_reflection_invoke_method_6(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto test_conv6 = type.get_method("test_conv6");
        test object_x;
        test_conv6.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f);
    }
}

BENCHMARK(BM_rainytoolkit_reflection_invoke_method)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_method)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_method_2)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_method_2)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_method_3)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_method_3)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_method_4)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_method_4)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_method_5)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_method_5)->Repetitions(2);
BENCHMARK(BM_rainytoolkit_reflection_invoke_method_6)->Repetitions(2);
BENCHMARK(BM_rttr_reflection_invoke_method_6)->Repetitions(2);
