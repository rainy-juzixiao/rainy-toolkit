#define NOMINMAX
#include <benchmark/benchmark.h>
#include <rainy/meta/reflection/function.hpp>
#include <rttr/registration.h>
#include <UDRefl/UDRefl.hpp>
#include <meta/factory.hpp>
#include <meta/meta.hpp>

class test {
public:
    void fun() {
        static float c;
        c += 3.14f;  
    }

    void fun1(float n) {
        static float c;
        c += n;
    }

    void fun2(float n, float m) {
        static float c;
        c += n + m;
    }

    void fun3(float n, float m, float o) {
        static float c;
        c += n + m + o;
    }

    void fun4(float n, float m, float o, float p) {
        static float c;
        c += n + m + o + p;
    }

    void fun5(float n, float m, float o, float p, float q) {
        static float c;
        c += n + m + o + p + q;
    }

    void fun6(float n, float m, float o, float p, float q, float a1) {
        static float c;
        c += n + m + o + p + q +a1;
    }
};

using namespace rttr;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

RTTR_REGISTRATION {
    registration::class_<test>("test")
        .constructor()
        .method("fun1", &test::fun1)
        .method("fun2", &test::fun2)
        .method("fun3", &test::fun3)
        .method("fun4", &test::fun4)
        .method("fun5", &test::fun5)
        .method("fun6", &test::fun6);
    Mngr.RegisterType<test>();
    Mngr.AddMethod<&test::fun>("fun");
    Mngr.AddMethod<&test::fun1>("fun1");
    Mngr.AddMethod<&test::fun2>("fun2");
    Mngr.AddMethod<&test::fun3>("fun3");
    Mngr.AddMethod<&test::fun4>("fun4");
    Mngr.AddMethod<&test::fun5>("fun5");
    Mngr.AddMethod<&test::fun6>("fun6");
    std::hash<std::string_view> hash{};
    auto factory = meta::reflect<test>(hash("reflected_type"));
    factory.func<&test::fun>(hash("fun"));
    factory.func<&test::fun1>(hash("fun1"));
    factory.func<&test::fun2>(hash("fun2"));
    factory.func<&test::fun3>(hash("fun3"));
    factory.func<&test::fun4>(hash("fun4"));
    factory.func<&test::fun5>(hash("fun5"));
    factory.func<&test::fun6>(hash("fun6"));
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun;
        static test object_x;
        fun_rf.invoke_paramlist(object_x);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method_1(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun1;
        static test object_x;
        fun_rf.invoke_paramlist(object_x, 10.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method_2(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun2;
        static test object_x;
        fun_rf.invoke_paramlist(object_x, 10.0f, 20.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method_3(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun3;
        static test object_x;
        fun_rf.invoke_paramlist(object_x, 10.0f, 20.0f, 30.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method_4(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun4;
        static test object_x;
        fun_rf.invoke_paramlist(object_x, 10.0f, 20.0f, 30.0f, 40.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method_5(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun5;
        static test object_x;
        fun_rf.invoke_paramlist(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_variadic_method_6(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun6;
        static test object_x;
        fun_rf.invoke_paramlist(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method_1);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method_2);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method_3);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method_4);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method_5);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_variadic_method_6);

static void benchmark_rttr_reflection_invoke_variadic_method(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun1 = type.get_method("fun1");
        test object_x;
        fun1.invoke_variadic(object_x, {});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_variadic_method_1(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun1 = type.get_method("fun1");
        test object_x;
        fun1.invoke_variadic(object_x, {10.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_variadic_method_2(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun2 = type.get_method("fun2");
        test object_x;
        fun2.invoke_variadic(object_x, {10.0f, 20.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_variadic_method_3(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun3 = type.get_method("fun3");
        test object_x;
        fun3.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_variadic_method_4(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun4 = type.get_method("fun4");
        test object_x;
        fun4.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_variadic_method_5(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun5 = type.get_method("fun5");
        test object_x;
        fun5.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f, 50.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_variadic_method_6(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun6 = type.get_method("fun6");
        test object_x;
        fun6.invoke_variadic(object_x, {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method);
BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method_1);
BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method_2);
BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method_3);
BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method_4);
BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method_5);
BENCHMARK(benchmark_rttr_reflection_invoke_variadic_method_6);

static void benchmark_skypjack_meta_reflection_invoke_variadic_method(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun"));
        test object;
        f.invoke(object);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_variadic_method_1(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun1"));
        test object;
        std::array<float, 1> array{10.0f};
        f.invoke(object, array[0]);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_variadic_method_2(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun2"));
        test object;
        std::array<float, 2> array{10.0f, 20.0f};
        f.invoke(object, array[0], array[1]);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_variadic_method_3(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun3"));
        test object;
        std::array<float, 3> array{10.0f, 20.0f, 30.0f};
        f.invoke(object, array[0], array[1], array[2]);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_variadic_method_4(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun4"));
        test object;
        std::array<float, 4> array{10.0f, 20.0f, 30.0f, 40.0f};
        f.invoke(object, array[0], array[1], array[2], array[3]);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_variadic_method_5(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun5"));
        test object;
        std::array<float, 5> array{10.0f, 20.0f, 30.0f, 40.0f, 50.0f};
        f.invoke(object, array[0], array[1], array[2], array[3], array[4]);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_variadic_method_6(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun6"));
        test object;
        std::array<float, 6> array{10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.f};
        f.invoke(object, array[0], array[1], array[2], array[3], array[4], array[5]);
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method_1);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method_2);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method_3);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method_4);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method_5);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_variadic_method_6);

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun");
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method_1(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun1", TempArgsView{10.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method_2(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun2", TempArgsView{10.0f, 20.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method_3(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun3", TempArgsView{10.0f, 20.0f, 30.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method_4(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun4", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method_5(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun5", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f, 50.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_variadic_method_6(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun6", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method_1);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method_2);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method_3);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method_4);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method_5);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_variadic_method_6);

static void benchmark_rainytoolkit_reflection_invoke_method(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun;
        static test object_x;
        fun_rf.invoke(object_x);
        benchmark::DoNotOptimize(state.iterations());
    }
}


static void benchmark_rainytoolkit_reflection_invoke_method_1(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun1;
        static test object_x;
        fun_rf.invoke(object_x, 10.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_2(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun2;
        static test object_x;
        fun_rf.invoke(object_x, 10.0f, 20.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_3(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun3;
        static test object_x;
        fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_4(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun4;
        static test object_x;
        fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_5(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun5;
        static test object_x;
        fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_6(benchmark::State &state) {
    using namespace rainy::meta::reflection;
    for (auto _: state) {
        static function fun_rf = &test::fun6;
        static test object_x;
        fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_1);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_2);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_3);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_4);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_5);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_6);

static void benchmark_rttr_reflection_invoke_method(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun1 = type.get_method("fun");
        test object_x;
        fun1.invoke(object_x);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_method_1(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun1 = type.get_method("fun1");
        test object_x;
        fun1.invoke(object_x, 10.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_method_2(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun2 = type.get_method("fun2");
        test object_x;
        fun2.invoke(object_x, 10.0f, 20.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_method_3(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun3 = type.get_method("fun3");
        test object_x;
        fun3.invoke(object_x, 10.0f, 20.0f, 30.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_method_4(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun4 = type.get_method("fun4");
        test object_x;
        fun4.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_method_5(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun5 = type.get_method("fun5");
        test object_x;
        fun5.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_rttr_reflection_invoke_method_6(benchmark::State &state) {
    for (auto _: state) {
        static auto type = rttr::type::get_by_name("test");
        static auto fun6 = type.get_method("fun6");
        test object_x;
        fun6.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_rttr_reflection_invoke_method);
BENCHMARK(benchmark_rttr_reflection_invoke_method_1);
BENCHMARK(benchmark_rttr_reflection_invoke_method_2);
BENCHMARK(benchmark_rttr_reflection_invoke_method_3);
BENCHMARK(benchmark_rttr_reflection_invoke_method_4);
BENCHMARK(benchmark_rttr_reflection_invoke_method_5);
BENCHMARK(benchmark_rttr_reflection_invoke_method_6);

static void benchmark_skypjack_meta_reflection_invoke_method(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun"));
        test object;
        f.invoke(object);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_method_1(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun1"));
        test object;
        f.invoke(object, 10.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_method_2(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun2"));
        test object;
        f.invoke(object, 10.0f, 20.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_method_3(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun3"));
        test object;
        f.invoke(object, 10.0f, 20.0f, 30.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_method_4(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun4"));
        test object;
        f.invoke(object, 10.0f, 20.0f, 30.0f, 40.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_method_5(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun5"));
        test object;
        f.invoke(object, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_skypjack_meta_reflection_invoke_method_6(benchmark::State &state) {
    for (auto _: state) {
        static std::hash<std::string_view> hasher;
        auto f = meta::resolve<test>().func(hasher("fun6"));
        test object;
        f.invoke(object, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f);
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_1);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_2);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_3);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_4);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_5);
BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_6);

static void benchmark_ubpa_udrefl_reflection_invoke_method(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun");
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_method_1(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun1", TempArgsView{10.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_method_2(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun2", TempArgsView{10.0f, 20.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_method_3(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun3", TempArgsView{10.0f, 20.0f, 30.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_method_4(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun4", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_method_5(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun5", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f, 50.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

static void benchmark_ubpa_udrefl_reflection_invoke_method_6(benchmark::State &state) {
    for (auto _: state) {
        static auto object = Mngr.MakeShared(Type_of<test>);
        object.Invoke("fun6", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f});
        benchmark::DoNotOptimize(state.iterations());
    }
}

BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_1);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_2);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_3);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_4);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_5);
BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_6);