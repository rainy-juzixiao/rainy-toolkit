#include <array>
#include <gtest/gtest.h>
#include <rainy/hybrid_jenova/containers/vector.hpp>
#include <rainy/utility.hpp>
#include <rainy/utility/pair.hpp>
#undef max
#undef min

constexpr int p() {
    rainy::containers::array<int, 4> a{1, 2, 3, 4};
    rainy::containers::array<int, 4> a2{};
    rainy::algorithm::container_operater::copy_n(a.begin(), 4, a2.data());
    return 0;
}


int shared_data;

void test_fun() {
    for (int i = 0; i < 1000; ++i) {
        rainy::core::pal::interlocked_exchange_add(reinterpret_cast<volatile long *>(&shared_data), 4);
    }
}

#include <rainy/containers/string.hpp>

#include <chrono>
#include <rainy/containers/array.hpp>
#include <rainy/foundation/pal/threading.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/text/char_traits.hpp>

RAINY_CONSTEXPR20 int pp() {
    using string = rainy::text::basic_string<char, rainy::text::char_traits<char>, std::allocator<char>>;
    string str;
    return 0;
}

template <typename Fn, typename... Args>
void benchmark(const char *const label, Fn &&func, Args &&...args) {
    const auto start = std::chrono::high_resolution_clock::now();
    (void) std::forward<Fn>(func)(std::forward<Args>(args)...);
    const auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << label << " took " << time.count() << " nanoseconds\n";
}


#include <rainy/meta/reflection/reflection.hpp>



class test_x {
public:
    test_x() = default;

    int fun(std::string_view n1) {
        static volatile std::size_t c = 0;
        c += n1.size();
        return 0;
    };

    int foo(const std::string &) {
        return 0;
    }

    int another(int a, int b, int c, std::string_view d, int e) {
        std::cout << a << '\n';
        std::cout << b << '\n';
        std::cout << c << '\n';
        std::cout << d << '\n';
        std::cout << e << '\n';
        return 0;
    }

    int copy_another(int a, int b, int c, std::string_view d, int e) {
        return 0;
    }

    int my() const volatile & {
        return 0;
    }

    void try_modify(int &val) {
        val = 666;
    }

    static void static_method() {
        std::cout << "hello my static!" << '\n';
    }

    int field;
};

#include <rttr/registration.h>

using namespace rttr;

class MyBase {
public:
    virtual ~MyBase() = default;

    virtual void sayHello() {
        std::cout << "Hello from Base!" << std::endl;
    }
};

class Derived : public MyBase {
public:
    void sayHello() override {
        std::cout << "Hello from Derived!" << std::endl;
    }

    void sayGoodbye() {
        std::cout << "Goodbye from Derived!" << std::endl;
    }
};

#include <rainy/meta/reflection/type_context.hpp>

void test_my_context() {
    auto &my_context = rainy::meta::reflection::instance_context::instance();
    my_context.create_blueprint("person")
        .add_member<std::string_view>("name", rainy::meta::reflection::acc_level::public_access)
        .add_member<int>("age", rainy::meta::reflection::acc_level::public_access)
        .add_member<int>("study_id", rainy::meta::reflection::acc_level::public_access)
        .add_member_by_typename("int", "extra_object", rainy::meta::reflection::acc_level::public_access)
        .add_member_by_rtti(rainy_typeid(std::string_view), "mysv", rainy::meta::reflection::acc_level::private_access);
    auto object = my_context.get_blueprint("person").create_dynamic_object(
        {{"name", std::string_view{"rainy-juzixiao"}}, {"age", 18}, {"study_id", 22313043}});
    rainy::utility::expects(object.get_member("name")->getter<std::string_view>() == "rainy-juzixiao");
    rainy::utility::expects(object.has_member("extra_object"), "object not exists");
    rainy::utility::expects(object.typeinfo("extra_object") == rainy_typeid(int));
    rainy::utility::expects(object.typeinfo("mysv") == rainy_typeid(std::string_view));
    my_context.remove_blueprint("person");
}

namespace rainy::meta::reflection {
    class registration {
    public:
        template <typename Class>
        class class_ {
        public:
            class_(std::string_view name) {
                
            }

        private:

        };


    private:
    };
}

//#include <UDRefl/UDRefl.hpp>
#include <filesystem>

#include <sys/stat.h>
#include <expected>

class A {
public:
    virtual void show() {
        std::cout << "A's show function\n";
    }

    virtual ~A() = default; // 虚析构函数
};

class B : virtual public A {
public:
    virtual void show() override {
        std::cout << "B's show function\n";
    }

    virtual ~B() = default;
};

class C : virtual public B {
public:
    void show() override {
        std::cout << "C's show function\n";
    }

    virtual ~C() = default;
};

RTTR_REGISTRATION {
    registration::class_<test_x>("test_x").constructor<>().method("fun", &test_x::fun).property("field", &test_x::field);
    registration::class_<A>("A").method("show", &A::show);
    registration::class_<C>("C").constructor().method("show", &C::show);
}

int main(int argc, char **argv) {
    test_x object_x;
    rainy::meta::reflection::function f(&test_x::fun);
    auto rainy_view = rainy::meta::reflection::make_arguments(std::string_view{"hello world"});
    f.invoke(object_x, rainy_view);
    f.invoke(object_x, rainy_view);
    f.invoke(object_x, rainy_view);
    f.invoke(object_x, rainy_view);
    type test_x_type = type::get_by_name("test_x");
    auto method = test_x_type.get_method("fun");
    auto dummy = test_x_type.create();
    /*auto& refl_mgr = Ubpa::UDRefl::ReflMngr::Instance();
    refl_mgr.RegisterType<test_x>();
    refl_mgr.AddMethod<&test_x::fun>("fun");
    Ubpa::UDRefl::SharedObject shared_object = refl_mgr.MakeShared(Ubpa::Type_of<test_x>);
    auto ubpa_arg_view = Ubpa::UDRefl::ArgsView(Ubpa::UDRefl::TempArgsView<1>{std::string_view{"Hello World"}});*/
    auto rttr_arg_view = std::vector<rttr::argument>{std::string_view{"hello world"}};
    auto native_method = &test_x::fun;
    std::function<int(std::string_view)> std_function = [&](std::string_view ax) { return object_x.fun(ax); };
    auto rainy_variadic_args = std::vector<rainy::containers::any>{static_cast<const char *>("Hello World")};
    for (std::size_t i = 1; i <= 20; ++i) {
        std::cout << "第" << i << "次测试\n";
        benchmark("native invoke", [&] {
            for (int i = 0; i < 100000; ++i) {
                rainy::utility::invoke(native_method, object_x, std::string_view{"Hello World"});
            }
        });
        benchmark("std::function invoke", [&] {
            for (int i = 0; i < 100000; ++i) {
                std_function(std::string_view{"Hello World"});
            }
        });
        benchmark("rttr invoke", [&] {
            for (int i = 0; i < 100000; ++i) {
                method.invoke_variadic(dummy, rttr_arg_view);
            }
        });
        benchmark("rainy's reflection", [&] {
            for (int i = 0; i < 100000; ++i) {
                f(object_x, static_cast<const char *>("Hello World"));
            }
        });
    }
    rainy::meta::reflection::function f2(&test_x::static_method);
    std::cout << f2.which_belongs().name() << '\n';
    std::cout << "f == f2 ? " << (f == f2) << '\n';
    f2 = &test_x::fun;
    std::cout << "f == f2 ? " << (f == f2) << '\n';
    std::cout << "----\n";
    f2.rebind(&test_x::another);
    f2(object_x, 10, 10, 10, static_cast<const char *>("Hello World"), 10);
    f2.rebind(&test_x::copy_another);
    benchmark("test signle call invoke times ", [&] { f.invoke(object_x, rainy_view); });
    //f.invoke(nullptr, rainy::meta::reflection::make_arguments("Hello World"));
    //
    //f = &test_x::my;
    //f.invoke(nullptr, rainy::meta::reflection::make_arguments());
    //
    //using namespace rainy;
    //
    //rainy::meta::reflection::function copy_f;
    //copy_f = f;
    //rainy::meta::reflection::arguments_visitor *argument{};
    //auto instance = rainy::meta::reflection::make_arguments(std::string_view{"Hello World"});
    //argument = &instance;
    //f.invoke(nullptr, *argument);
    //copy_f.invoke(nullptr, *argument);
    //benchmark("rainy reflection once call", [&] { f.invoke(nullptr, *argument); });
    rainy::foundation::pal::threading::thread t1(test_fun);
    rainy::foundation::pal::threading::thread t2(test_fun);
    std::cout << t1.joinable() << '\n';
    t1.join();
    t2.join();
    rainy::utility::tuple<int, int, std::string> tuple = {1, 2, "Hello World"};
    auto &string_typeinfo = rainy_typeid(const std::string &);
    std::cout << std::boolalpha;
    std::cout << "Is string_typeinfo traits is_lref ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_lref)
              << '\n';
    std::cout << "Is string_typeinfo traits is_rref ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_rref)
              << '\n';
    std::cout << "Is string_typeinfo traits is_const ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_const)
              << '\n';
    std::cout << "Is string_typeinfo traits is_volatile ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_volatile)
              << '\n';
    std::cout << "Is string_typeinfo traits is_void ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_void)
              << '\n';
    std::cout << "Is string_typeinfo traits is_pointer ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_pointer)
              << '\n';
    std::cout << "Is string_typeinfo traits is_template ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_template)
              << '\n';
    std::cout << "Is string_typeinfo traits is_class ? " << string_typeinfo.has_traits(rainy::foundation::rtti::traits::is_class)
              << '\n';
    std::cout << string_typeinfo.remove_reference().name() << '\n';
    std::cout << rainy_typeid(const std::string &&).remove_reference().remove_const().name() << '\n';
    RAINY_CONSTEXPR20 int i = p();
    constexpr bool A = false;
    static_assert((A & A) == A);
    static_assert((A | A) == A);
    RAINY_CONSTEXPR20 int get{pp()};
    rainy::containers::wstring sss = L"Hello World";
    std::allocator<char> alloc;
    rainy::utility::compressed_pair<rainy::foundation::system::memory::allocator<char>, int> pair;
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "We're in!!!\n";
    std::cout << "Welcome to my test cases!\n";
    std::cout << "rainy: let's get started!\n";
    int ret = RUN_ALL_TESTS();
    std::cout << "Now, we finally finish our test!\n";
    std::cout << "Thanks for you patience!\n";
    return ret;
}
