#include <algorithm>
#include <iostream>
#include <optional>
#include <rainy/collections/inplace_vector.hpp>
#include <rainy/collections/string.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/metadata.hpp>
#include <rainy/meta/reflection/registration.hpp>
#include <rainy/meta/reflection/shared_object.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <rainy/utility/any.hpp>
#include <ranges>

using namespace rainy;
using namespace rainy::utility;

struct structure {
    int a{};
    char b{};
    std::string c{};
};

struct mypair {
    int data1{};
    std::string_view data2{};
};

int funccc(std::string_view count, ...) {
    va_list args{};
    va_start(args, count);
    int tmp = va_arg(args, int);
    va_end(args);
    return 42 + tmp;
}

class mybase1 {
public:
    void print_mybase1() {
        std::cout << "mybase1\n";
    }

    virtual void virtual_fun() {
        std::cout << "this is mybase1\n";
    }
};

class mybase2 : virtual public mybase1 {
public:
    void print_mybase2() {
        std::cout << "mybase1\n";
    }

    void virtual_fun() override {
        std::cout << "this is mybase2\n";
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

    void print_field(myclass object) {
        std::cout << field + object.field << '\n';
    }

    void print_field(std::string value) const {
        std::cout << value << '\n';
    }

    void virtual_fun() override {
        std::cout << "this is myclass\n";
    }

    int field{0};
};

struct MyStruct {
    MyStruct() {};
    void func(double) {};
    int data;
};

enum class color {
    red,
    green,
    blue
};

RAINY_REFLECTION_REGISTRATION {
    // clang-format off
    using namespace rainy::meta::reflection;
	registration::class_<MyStruct>("MyStruct")
		.constructor<>()
		.property("data", &MyStruct::data)
		.method("func", &MyStruct::func);
    meta::reflection::registration::class_<myclass>("myclass")
        (
            meta::reflection::metadata("prop","virtual public")
        )
        .constructor<>()
        .copy_constructor()
        .move_constructor()
        .property("field", &myclass::field)
        .enumeration<color>("color")
        .constructor<int>()
        (
            meta::reflection::metadata("prop", "ctor")
        )
        .method("print_field",
            utility::get_overloaded_func<myclass,void(std::string) const>(&myclass::print_field))
        (
            meta::reflection::metadata("prop","const print")
        )
        .method("print_field", 
            utility::get_overloaded_func<myclass,void(int)>(&myclass::print_field))
        (
            meta::reflection::metadata("prop", "print"),
            meta::reflection::default_arguments(50)
        )
        .method("virtual_fun", &mybase1::virtual_fun)
        .method("print_field", 
            utility::get_overloaded_func<myclass,void(myclass)>(&myclass::print_field))
        .method("virtual_fun", &myclass::virtual_fun)
        .base<mybase1>("mybase1")
        .base<mybase2>("mybase2");
    meta::reflection::registration::fundamental<int>("int")
        (
            metadata("name", "11111")
        );
    meta::reflection::registration::class_<mybase1>()
        .method("print_mybase1", &mybase1::print_mybase1)
        .method("virtual_fun", &mybase1::virtual_fun);

    meta::reflection::registration::class_<mybase2>()
        .method("print_mybase2", &mybase2::print_mybase2)
        .method("virtual_fun", &mybase2::virtual_fun);
    // clang-format on
}

RAINY_REFLECT_TUPLE_LIKE(structure, a, b, c)

static_assert(member_count_v<mypair> == 2);

struct object {
    constexpr object() = default;
    constexpr object(int v) : obj(v), obj2(v + 1) {
    }

    int value() {
        return obj;
    }

    int value2() {
        return obj2;
    }

private:
    int obj{};
    int obj2{};
};

RAINY_PRIVATE_REFLECT_TUPLE_LIKE(object, obj, obj2)

#include <map>
#include <rainy/collections/dense_map.hpp>
#include <rainy/collections/unordered_map.hpp>
#include <rainy/foundation/functional.hpp>

void f(int n1, int n2, int n3, const int &n4, int n5) noexcept {
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}

int g(int n1) {
    return n1;
}

struct Foo {
    void print_sum(int n1, int n2) {
        std::cout << n1 + n2 << '\n';
    }

    int data = 10;
};

int minus(int a, int b) {
    return a - b;
}

struct S {
    int val;
    int minus(int arg) const noexcept {
        return val - arg;
    }
};

int main() {
    {
        auto fifty_minus = foundation::functional::bind_front(minus, 50);
        assert(fifty_minus(3) == 47); // equivalent to: minus(50, 3) == 47

        auto member_minus = foundation::functional::bind_front(&S::minus, S{50});
        assert(member_minus(3) == 47); //: S tmp{50}; tmp.minus(3) == 47

        // Noexcept-specification is preserved:
        static_assert(!noexcept(fifty_minus(3)));
        static_assert(noexcept(member_minus(3)));

        // Binding of a lambda:
        auto plus = [](int a, int b) { return a + b; };
        auto forty_plus = foundation::functional::bind_front(plus, 40);
        assert(forty_plus(7) == 47);
    }
    {
        using namespace foundation::functional::placeholders; // for _1, _2, _3...

        std::cout << "1) argument reordering and pass-by-reference: ";
        int n = 7;
        // (_1 and _2 are from std::placeholders, and represent future
        // arguments that will be passed to f1)
        auto f1 = foundation::functional::bind(f, _2, 42, _1, std::cref(n), n);
        n = 10;
        const bool c = noexcept(f1(1, 2, 100));
        f1(1, 2, 1001); // 1 is bound by _1, 2 is bound by _2, 1001 is unused
                        // makes a call to f(2, 42, 1, n, 7)

        std::cout << "2) achieving the same effect using a lambda: ";
        n = 7;
        auto lambda = [&ncref = n, n](auto a, auto b, auto /*unused*/) { f(b, 42, a, ncref, n); };
        n = 10;
        lambda(1, 2, 1001); // same as a call to f1(1, 2, 1001)

        std::cout << "3) nested bind subexpressions share the placeholders: ";
        auto f2 = foundation::functional::bind(f, _3, foundation::functional::bind(g, _3), _3, 4, 5);
        f2(10, 11, 12); // makes a call to f(12, g(12), 12, 4, 5);

        std::cout << "4) bind a RNG with a distribution: ";
        std::default_random_engine e;
        std::uniform_int_distribution<> d(0, 10);
        auto rnd = bind(d, e); // a copy of e is stored in rnd
        for (int n = 0; n < 10; ++n)
            std::cout << rnd() << ' ';
        std::cout << '\n';

        std::cout << "5) bind to a pointer to member function: ";
        Foo foo;
        auto f3 = foundation::functional::bind(&Foo::print_sum, &foo, 95, _1);
        f3(5);

        std::cout << "6) bind to a mem_fn that is a pointer to member function: ";
        auto ptr_to_print_sum = &Foo::print_sum;
        auto f4 = foundation::functional::bind(ptr_to_print_sum, &foo, 95, _1);
        f4(5);

        std::cout << "7) bind to a pointer to data member: ";
        auto f5 = foundation::functional::bind(&Foo::data, _1);
        std::cout << f5(foo) << '\n';

        std::cout << "8) bind to a mem_fn that is a pointer to data member: ";
        auto ptr_to_data = &Foo::data;
        auto f6 = foundation::functional::bind(ptr_to_data, _1);
        std::cout << f6(foo) << '\n';

        std::cout << "9) use smart pointers to call members of the referenced objects: ";
        std::cout << f6(std::make_shared<Foo>(foo)) << ' ' << f6(std::make_unique<Foo>(foo)) << '\n';
    }
    foundation::functional::binder bind_func(
        [](int a, std::string b) {
            std::cout << "a = " << a << '\n';
            std::cout << "b = " << b << '\n';
        },
        foundation::functional::placeholders::_2, foundation::functional::placeholders::_1);
    bind_func("Hello World", 40);
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    collections::unordered_map<int, char> map = {{10, '1'}, {20, '2'}};
    auto node = map.extract(10);
    std::cout << node.mapped() << '\n';
    for (const auto &item: map) {
        std::cout << item.first << " : " << item.second << '\n';
    }
    using namespace rainy::meta::moon;
    for (const std::size_t offset: meta::moon::get_member_offset_arr<structure>()) {
        std::cout << offset << '\n';
    }
    object o{0};
    meta::moon::get<0>(o) = 10;
    ++meta::moon::get<1>(o);
    std::cout << o.value() << '\n';
    std::cout << o.value2() << '\n';
    /*meta::moon::visit_members(sosss, [](auto &&...args) {
        ((std::cout << args << " "), ...);
        std::cout << "\n";
    });*/
    constexpr object ooo = {10};
    {
        using namespace rainy::meta::reflection;
        type enum_t = type::get<color>();
        std::cout << enum_t.get_name() << '\n';
        {
            enumeration enum_ = enum_t.get_enumeration();
            for (const auto &item: enum_.get_names()) {
                std::cout << item << std::endl;
            }
            auto enum_var = enum_t.create(color::red);
            std::cout << enum_.contains("red") << '\n';
            std::cout << enum_var.type().name() << std::endl;
            enum_var = enum_t.create(0);
            std::cout << enum_var.type().name() << std::endl;
            std::cout << enum_.value_to_name(0) << std::endl;
            std::cout << enum_.value_to_name(color::red) << std::endl;
            std::cout << "name_to_value--type : " << enum_.name_to_value("red").type().name() << std::endl;
        }
    }
    constexpr collections::views::array_view<foundation::ctti::typeinfo> n =
        foundation::ctti::typeinfo::create<type_traits::other_trans::type_list<int, char, std::string>>().template_arguments();
    {
        using namespace rainy::meta::reflection;
        for (const auto &item: type::get_global_methods()) {
            std::cout << item.get_name() << '\n';
        }
        std::cout << type::get_global_methods().size() << '\n';
        type t = type::get<MyStruct>();
        for (auto &prop: t.get_properties()) {
            std::cout << "meth: " << prop.get_name() << std::endl;
        }
        for (auto &meth: t.get_methods()) {
            std::cout << "name: " << meth.get_name() << std::endl;
        }


        type ty = type::get<int>();
        std::cout << ty.get_metadata("name").value() << '\n';
        std::cout << ty.get_fundmental().is_valid() << '\n';
        std::cout << ty.get_fundmental().create(10).type().name() << '\n';
        std::cout << ty.get_fundmental().get_metadata("name").value() << '\n';
        shared_object var = t.create();
        constructor ctor = t.get_constructor();
        var = ctor.invoke();
        std::cout << var.type().name() << std::endl; // 打印 'MyStruct'
        {
            MyStruct obj;

            property prop = type::get<MyStruct>().get_property("data");
            prop(obj) = 23;

            auto var_prop = prop(obj);
            std::cout << var_prop << std::endl; // 打印 '23'
        }
        {
            MyStruct obj;

            method meth = type::get<MyStruct>().get_method("func");
            meth.invoke(obj, 42.0);
            meth.invoke(obj, 42.f);
            meth.invoke(obj, 42ull);

            rainy::utility::any dynamic_var = 42;
            meth.invoke(obj, dynamic_var);
        }
        auto type = meta::reflection::type::get_by_name("myclass");
        auto type1 = meta::reflection::type::get_by_name("mybase1");
        myclass object = 50;
        rainy::meta::reflection::object_view view{object};
        {
            auto so = view.create_shared();
            std::cout << "create shared_object typename : " << so.type().name() << '\n';
            view.invoke("print_mybase1");
            auto &field = type.get_property("field");
            field.set_value(so, 40);
            so.invoke("print_field", 20);
            so.invoke("print_field", so);
        }
        std::cout << type.get_method("print_mybase1") << '\n';
        std::cout << type.get_method("print_mybase2") << '\n';
        std::cout << "is_base_of " << type.is_base_of(type1) << '\n';
        std::cout << "is_derived_of " << type1.is_derived_from(type) << '\n';
        {
            const auto &vmeth = type.get_method("virtual_fun");
            vmeth.invoke(object);
        }
        {
            const auto &vmeth = type1.get_method("virtual_fun");
            vmeth.invoke(object);
        }
        {
            std::cout << type.get_metadata("prop").value() << '\n';
        }
    }
    any a = 10;
    std::cout << std::as_const(a).as_lvalue_reference().type().name() << '\n';
    std::cout << std::as_const(a).as_rvalue_reference().type().name() << '\n';
    a = std::array<int, 5>{5, 1, 2, 4, 3};
    for (auto i: a.as_lvalue_reference()) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    std::sort(a.begin(), a.end(), [](const auto &left, const auto &right) { return right > left; });
    for (auto i: a.as_lvalue_reference()) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    a = 10;
    std::cout << (a + 10000 - 10) << '\n';
    std::cout << (--a) << '\n';
    std::cout << (a) << '\n';
    std::cout << (++a) << '\n';
    std::cout << (a) << '\n';
    std::cout << (a++) << '\n';
    std::cout << (a) << '\n';
    std::cout << (a--) << '\n';
    std::cout << (a) << '\n';
    std::cout << (a % 4) << '\n';
    std::cout << (a / 4) << '\n';
    std::cout << (a * 10) << '\n';
    a *= 200;
    a -= 10;
    std::cout << a << '\n';
    int x = 20;
    a.emplace<int &>(x);
    a += 10;
    ++a;
    std::cout << a << '\n';
    std::cout << "x = " << x << '\n';
    a.as<int>() = 42;
    std::cout << a << '\n';
    std::cout << x << '\n';
    a.emplace<std::string>("Cello World");
    a[0] = 'H';
    std::cout << (a + std::string{", Again!"}) << '\n';
    std::cout << a.target_as_void_ptr() << '\n';
    a = 10;
    if (a.is<int>()) { // 检查a是否为int
        std::cout << "a is int\n";
    }
    if (a.is_convertible<char>()) { // 检查a是否可以转换到char
        std::cout << "a can convert to char\n";
    }
    auto f = [&a]() {
        a.match([](int a) { std::cout << "Got int! value = " << a << '\n'; },
                [](std::optional<char> a) { std::cout << "Got char! value = " << a.value() << '\n'; },
                [](std::string_view s) { std::cout << "Got string_view! value = " << s << '\n'; },
                [](any_default_match, const any &v /* 允许额外引用一个any的常引用。引用自调用它的对象 */) {
                    // 该handler会在找不到匹配的时候，返回对象
                    std::cout << "This Method will be default match." << '\n';
                    std::cout << "Cause by failed to match type -> " << v.type().name() << '\n';
                });
    };
    std::cout << "any = " << a << '\n'; // 输出10
    f();
    a = std::optional<char>{'c'};
    f();
    a = "Hello World"; // 这个会在match的std::string_view分支中自动转换
    f();
    a = std::vector<int>{}; // 将无法匹配处理
    f();
    // destructure() 可以从一个类型中进行解构，类似于结构化绑定
    a = structure{10, 'c', "Hello World"};
    a.destructure([](int a, char b, const std::string &c) {
        std::cout << "I got var-a here is the value : " << a << '\n';
        std::cout << "I got var-b here is the value : " << b << '\n';
        std::cout << "I got var-c here is the value : " << c << '\n';
    });
    std::tuple<int, char, std::string> tuple;
    a.destructure(tuple);
    std::cout << "tuple get<0> : " << std::get<0>(tuple) << '\n';
    std::cout << "tuple get<1> : " << std::get<1>(tuple) << '\n';
    std::cout << "tuple get<2> : " << std::get<2>(tuple) << '\n';
    a = std::make_pair(10, "hello world");
    std::pair<int, std::string_view> standard_pair;
    pair<int, std::string_view> utility_pair;
    a.destructure(standard_pair);
    std::cout << "Let's test for std::pair!\n";
    std::cout << "first = " << standard_pair.first << '\n';
    std::cout << "second = " << standard_pair.second << '\n';
    a.destructure(utility_pair);
    std::cout << "Let's test for pair!\n";
    std::cout << "first = " << utility_pair.first << '\n';
    std::cout << "second = " << utility_pair.second << '\n';
    mypair structure;
    a.destructure(structure);
    std::cout << structure.data1 << '\n';
    std::cout << structure.data2 << '\n';
    a = std::make_pair(42, "Hello World");
    auto [dvar1, dvar2] = a.destructure<int, std::string_view>();
    std::cout << "dvar1 = " << dvar1 << '\n';
    std::cout << "dvar2 = " << dvar2 << '\n';
    a = 10;
    // match_for允许指定一系列类型作为variant的实例化参数以表明处理handler可能返回的类型。
    auto var = a.match_for<std::string_view, int, double>([](std::string_view str) { return str.size(); },
                                                          [](float x) { return static_cast<int>(x); });
    std::visit([](auto &&value) { std::cout << "I got value! the value is " << value << '\n'; }, var);
    // auto_deduce将会使用每个handler的返回值类型作为variant实例化参数
    auto var1 = a.match_for(auto_deduce, [](std::string_view str) { return str.size(); }, [](float x) { return static_cast<int>(x); });
    std::visit([](auto &&value) { std::cout << "I got value! the value is " << value << '\n'; }, var1);
    // 也可以求出哈希值，并用于哈希相关的容器
    std::cout << a.hash_code() << '\n';
    std::cout << std::hash<int>{}.operator()(10) << '\n';
    std::unordered_map<any, std::string_view> any_map = {{10, "number:10"},
                                                         {3.14f, "float:3.14f"},
                                                         {'c', "char:c"},
                                                         {{std::in_place_type<std::string>, "hello_world_text"}, "Hello World"}};
    std::cout << any_map[10] << '\n';
    std::cout << any_map[3.14f] << '\n';
    std::cout << any_map['c'] << '\n';
    std::cout << any_map[{std::in_place_type<std::string>, "hello_world_text"}] << '\n';
    a = std::unordered_map<std::string_view, int>{{"1", 1}};
    std::cout << a["1"] << '\n';
    a["2"] = 2;
    std::cout << a["2"] << '\n';
    a["3"] = 3;
    std::cout << a["3"] << '\n';
    a["4"] = 4;
    std::cout << a["4"] << '\n';
    for (auto iter = a.begin(); iter != a.end(); ++iter) {
        auto [first, second] = (*iter).destructure<std::string_view, int>();
        std::cout << first << ' ' << second << '\n';
    }
    std::cout << "category = " << (int) a.begin().category() << '\n';
    a = std::make_tuple("Hello World", 42, 3.14f);
    std::cout << a[0] << '\n';
    std::cout << a[1] << '\n';
    std::cout << a[2] << '\n';
    a = std::vector<int>{1, 2, 3, 4, 5};
    for (const auto item: a) {
        std::cout << item << '\n';
    }
    return 0;
}
