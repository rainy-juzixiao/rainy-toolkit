#include <iostream>
#include <optional>
#include <algorithm>
#include <rainy/utility/any.hpp>
#include <rainy/collections/inplace_vector.hpp>
#include <rainy/collections/string.hpp>
#include <rainy/meta/reflection/metadata.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <rainy/meta/reflection/registration.hpp>
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

    void print_field(std::string value) const {
        std::cout << value << '\n';
    }

    void virtual_fun() override {
        std::cout << "this is myclass\n";
    }

private:
    int field{0};
};

RAINY_REFLECTION_REGISTRATION {
    // clang-format off
    meta::reflection::registration::class_<myclass>("myclass")
        (
            meta::reflection::metadata("prop","virtual public")
        )
        .constructor<>()
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

int main() {
    {
        auto type = meta::reflection::type::get_by_name("myclass");
        auto type1 = meta::reflection::type::get_by_name("mybase1");
        myclass object = 50;
        auto &func = type.get_method("print_field");
        func.invoke(object);
        func.invoke(object, 50);
        std::cout << func.name() << '\n';
        std::cout << func << '\n';
        std::cout << func.get_metadata("prop").value() << '\n';
        auto &ctor = type.get_construtor({rainy_typeid(int)});
        std::cout << ctor.name() << '\n';
        std::cout << ctor.get_metadata("prop").value() << '\n';
        for (const auto &bases: type.get_base_classes()) {
            std::cout << "base : " << bases.get_name() << '\n';
            for (const auto &item: bases.get_methods()) {
                std::cout << '\t' << item << '\n';
            }
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
    for (auto i : a.as_lvalue_reference()) {
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
