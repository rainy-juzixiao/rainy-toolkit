#define NOMINMAX
//#include <UDRefl/UDRefl.hpp>
#include <benchmark/benchmark.h>
//#include <meta/factory.hpp>
//#include <meta/meta.hpp>
#include <rainy/meta/reflection/registration.hpp>
#include <rttr/registration.h>

class test {
public:
    float fun() { // NOLINT
        static volatile float c;
        c += 3.14f;
        return c;
    }

    float fun1(float n) { // NOLINT
        rainy::utility::expects(n == 10.0f);
        static volatile float c;
        c += n;
        return c;
    }

    float fun2(float n, float m) { // NOLINT
        rainy::utility::expects(n == 10.0f && m == 20.0f);
        static volatile float c;
        c += n + m;
        return c;
    }

    float fun3(float n, float m, float o) { // NOLINT
        rainy::utility::expects(n == 10.0f && m == 20.0f && o == 30.0f);
        static volatile float c;
        c += n + m + o;
        return c;
    }

    float fun4(float n, float m, float o, float p) { // NOLINT
        rainy::utility::expects(n == 10.0f && m == 20.0f && o == 30.0f && p == 40.0f);
        static volatile float c;
        c += n + m + o + p;
        return c;
    }

    float fun5(float n, float m, float o, float p, float q) { // NOLINT
        rainy::utility::expects(n == 10.0f && m == 20.0f && o == 30.0f && p == 40.0f && q == 50.0f);
        static volatile float c;
        c += n + m + o + p + q;
        return c;
    }

    float fun6(float n, float m, float o, float p, float q, float a1) { // NOLINT
        rainy::utility::expects(n == 10.0f && m == 20.0f && o == 30.0f && p == 40.0f && q == 50.0f && a1 == 60.0f);
        static volatile float c;
        c += n + m + o + p + q + a1;
        return c;
    }
};

using namespace rttr;
//using namespace Ubpa;
//using namespace Ubpa::UDRefl;

class mybase1 {
public:
    void print_mybase1() {
        std::cout << "mybase1\n";
    }

    virtual void virtual_fun() {
    }
};

class mybase2 : virtual public mybase1 {
public:
    void print_mybase2() {
        std::cout << "mybase1\n";
    }

    void virtual_fun() override {
    }
};

class myclass : virtual public mybase2 {
public:
    myclass() {
    }

    myclass(int value) {
        this->field = value;
    }

    void print_field(int value) {
        std::cout << field + value << '\n';
    }

    void virtual_fun() override {
    }

private:
    int field{0};
};

struct auto_regsiter {
    auto_regsiter() {
        registration::class_<test>("test")
            .constructor()
            .method("fun", &test::fun)
            .method("fun1", &test::fun1)
            .method("fun2", &test::fun2)
            .method("fun3", &test::fun3)
            .method("fun4", &test::fun4)
            .method("fun5", &test::fun5)
            .method("fun6", &test::fun6);
       /*Mngr.RegisterType<test>();
        Mngr.AddMethod<&test::fun>("fun");
        Mngr.AddMethod<&test::fun1>("fun1");
        Mngr.AddMethod<&test::fun2>("fun2");
        Mngr.AddMethod<&test::fun3>("fun3");
        Mngr.AddMethod<&test::fun4>("fun4");
        Mngr.AddMethod<&test::fun5>("fun5");
        Mngr.AddMethod<&test::fun6>("fun6");
        constexpr std::hash<std::string_view> hash{};
        auto factory = meta::reflect<test>(hash("reflected_type"));
        factory.func<&test::fun>(hash("fun"));
        factory.func<&test::fun1>(hash("fun1"));
        factory.func<&test::fun2>(hash("fun2"));
        factory.func<&test::fun3>(hash("fun3"));
        factory.func<&test::fun4>(hash("fun4"));
        factory.func<&test::fun5>(hash("fun5"));
        factory.func<&test::fun6>(hash("fun6"));*/
        rainy::meta::reflection::registration::class_<test>("test")
            .constructor()
            .method("fun", &test::fun)
            .method("fun1", &test::fun1)
            .method("fun2", &test::fun2)
            .method("fun3", &test::fun3)
            .method("fun4", &test::fun4)
            .method("fun5", &test::fun5)
            .method("fun6", &test::fun6);
        {
            using namespace rainy;
            // clang-format off
            meta::reflection::registration::class_<myclass>("myclass")
                .constructor<>()
                .constructor<int>()
                (
                    meta::reflection::metadata("prop", "ctor")
                )
                .method("print_field", &myclass::print_field)
                (
                    meta::reflection::metadata("prop", "print"),
                    meta::reflection::default_arguments(50)
                )
                .method("virtual_fun", &myclass::virtual_fun)
                .base<mybase1>("mybase1")
                .base<mybase2>("mybase2");

            meta::reflection::registration::class_<mybase1>("mybase1")
                .method("print_mybase1", &mybase1::print_mybase1)
                .method("virtual_fun", &mybase1::virtual_fun);

            meta::reflection::registration::class_<mybase2>("mybase2")
                .method("print_mybase2", &mybase2::print_mybase2)
                .method("virtual_fun", &mybase1::virtual_fun);
            // clang-format on
        }
    }
};

auto_regsiter registers;

static void benchmark_rainytoolkit_reflection_invoke_method(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x));
        benchmark::ClobberMemory();
        (void) _;
    }
}


static void benchmark_rainytoolkit_reflection_invoke_method_1(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun1");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x, 10.0f));
        benchmark::ClobberMemory();
        (void) _;
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_2(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun2");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x, 10.0f, 20.0f));
        benchmark::ClobberMemory();
        (void) _;
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_3(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun3");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f));
        benchmark::ClobberMemory();
        (void) _;
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_4(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun4");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f));
        benchmark::ClobberMemory();
        (void) _;
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_5(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun5");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f));
        benchmark::ClobberMemory();
        (void) _;
    }
}

static void benchmark_rainytoolkit_reflection_invoke_method_6(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<test>();
        static const auto &fun_rf = type.get_method("fun6");
        static test object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f));
        benchmark::ClobberMemory();
        (void) _;
    }
}

BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_1);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_2);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_3);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_4);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_5);
BENCHMARK(benchmark_rainytoolkit_reflection_invoke_method_6);

static void benchmark_rainytoolkit_reflection_invoke_virtual_method(benchmark::State &state) {
    using namespace rainy::meta;
    for (const auto _: state) {
        static auto type = reflection::type::get<mybase2>();
        static const auto &fun_rf = type.get_method("virtual_fun");
        static myclass object_x;
        benchmark::DoNotOptimize(fun_rf.invoke(object_x));
        benchmark::ClobberMemory();
        (void) _;
    }
}

BENCHMARK(benchmark_rainytoolkit_reflection_invoke_virtual_method);

//static void benchmark_rttr_reflection_invoke_method(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun1 = type.get_method("fun");
//        static test object_x;
//        benchmark::DoNotOptimize(fun1.invoke(object_x));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_rttr_reflection_invoke_method_1(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun1 = type.get_method("fun1");
//        static test object_x;
//        benchmark::DoNotOptimize(fun1.invoke(object_x, 10.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_rttr_reflection_invoke_method_2(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun2 = type.get_method("fun2");
//        static test object_x;
//        benchmark::DoNotOptimize(fun2.invoke(object_x, 10.0f, 20.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_rttr_reflection_invoke_method_3(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun3 = type.get_method("fun3");
//        static test object_x;
//        benchmark::DoNotOptimize(fun3.invoke(object_x, 10.0f, 20.0f, 30.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_rttr_reflection_invoke_method_4(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun4 = type.get_method("fun4");
//        static test object_x;
//        benchmark::DoNotOptimize(fun4.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_rttr_reflection_invoke_method_5(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun5 = type.get_method("fun5");
//        static test object_x;
//        benchmark::DoNotOptimize(fun5.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_rttr_reflection_invoke_method_6(benchmark::State &state) {
//    for (const auto _: state) {
//        static auto type = rttr::type::get_by_name("test");
//        static auto fun6 = type.get_method("fun6");
//        static test object_x;
//        benchmark::DoNotOptimize(fun6.invoke(object_x, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//BENCHMARK(benchmark_rttr_reflection_invoke_method);
//BENCHMARK(benchmark_rttr_reflection_invoke_method_1);
//BENCHMARK(benchmark_rttr_reflection_invoke_method_2);
//BENCHMARK(benchmark_rttr_reflection_invoke_method_3);
//BENCHMARK(benchmark_rttr_reflection_invoke_method_4);
//BENCHMARK(benchmark_rttr_reflection_invoke_method_5);
//BENCHMARK(benchmark_rttr_reflection_invoke_method_6);
//
//static void benchmark_skypjack_meta_reflection_invoke_method(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_skypjack_meta_reflection_invoke_method_1(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun1"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object, 10.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_skypjack_meta_reflection_invoke_method_2(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun2"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object, 10.0f, 20.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_skypjack_meta_reflection_invoke_method_3(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun3"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object, 10.0f, 20.0f, 30.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_skypjack_meta_reflection_invoke_method_4(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun4"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object, 10.0f, 20.0f, 30.0f, 40.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_skypjack_meta_reflection_invoke_method_5(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun5"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_skypjack_meta_reflection_invoke_method_6(benchmark::State &state) {
//    for (const auto _: state) {
//        static std::hash<std::string_view> hasher;
//        static auto f = meta::resolve<test>().func(hasher("fun6"));
//        static test object;
//        benchmark::DoNotOptimize(f.invoke(object, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method);
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_1);
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_2);
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_3);
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_4);
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_5);
//BENCHMARK(benchmark_skypjack_meta_reflection_invoke_method_6);
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun"));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method_1(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun1", TempArgsView{10.0f}));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method_2(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun2", TempArgsView{10.0f, 20.0f}));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method_3(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun3", TempArgsView{10.0f, 20.0f, 30.0f}));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method_4(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun4", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f}));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method_5(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun5", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f, 50.0f}));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//static void benchmark_ubpa_udrefl_reflection_invoke_method_6(benchmark::State &state) {
//    static auto object = Mngr.MakeShared(Type_of<test>);
//    for (const auto _: state) {
//        benchmark::DoNotOptimize(object.Invoke("fun6", TempArgsView{10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f}));
//        benchmark::ClobberMemory();
//        (void) _;
//    }
//}
//
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method);
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_1);
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_2);
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_3);
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_4);
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_5);
//BENCHMARK(benchmark_ubpa_udrefl_reflection_invoke_method_6);
