#include <iostream>
#include <optional>
#include <rainy/utility/any.hpp>

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

#include <rainy/meta/reflection/function.hpp>
#include <rainy/utility/ioc.hpp>
#include <rainy/annotations/auto_wired.hpp>
#include <rainy/annotations/anno.hpp>

struct datasource {
    virtual void print_data_sources() noexcept = 0;
};

struct datasource_impl : datasource {
    datasource_impl(std::string username, std::string password) : username{username}, password{password} {
    }

    void print_data_sources() noexcept {
        std::cout << "this is a datasource\n";
        std::cout << "username is " << username << "\n";
        std::cout << "password is " << password << "\n";
    }

    std::string username;
    std::string password;
};
namespace rainy::utility::ioc {
    template <>
    struct factory_inject<datasource> {
        void inject(factory &fact) {
            inject_to<datasource, std::string, std::string>(fact)(std::in_place_type<datasource_impl>);
        }
    };
}

class user {
public:
    user() = default;

    void show_the_database() {
        datasources->print_data_sources();
    }

private:
    annotations::auto_wired<datasource> datasources{std::string{"root"}, std::string{"123456"}};
};

constexpr auto test() {
    collections::array<int, 5> v1{1, 2, 3, 4, 5};
    collections::array<int, 5> v2{1, 2, 3, 4, 5};
    collections::array<int, 0> v3{};
    return v1.concat(v2) == collections::array<int, 10>{1, 2, 3, 4, 5, 1, 2, 3, 4, 5};
}

int main() {
    constexpr auto vv = test();
    {
        any a{collections::array<int, 4>{10, 20, 30, 40}};
        a.destructure([](int v1, int v2, int v3, int v4) { std::cout << v1 << ',' << v2 << ',' << v3 << ',' << v4 << '\n'; });
    }
    user u;
    u.show_the_database();
    any a = 10;
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
    a.emplace<std::string>("");
    a.emplace<std::string>("Cello World");
    a[0].as<char>() = 'H';
    std::cout << (a + std::string{", Again!"}) << '\n';
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
                [](any::default_match, const any &v /* 允许额外引用一个any的常引用。引用自调用它的对象 */) {
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
    a.emplace<std::vector<int>>(); // 将无法匹配处理
    f();
    // destructure() 可以从一个类型中进行解构，类似于结构化绑定
    a = structure{10, 'c', "Hello World"};
    a.destructure([](int a, char b,const std::string& c) {
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
    auto var1 = a.match_for(
        auto_deduce, [](std::string_view str) { return str.size(); }, [](float x) { return static_cast<int>(x); });
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
    a["2"].as<int>() = 2;
    std::cout << a["2"] << '\n';
    a["3"].as<int>() = 3;
    std::cout << a["3"] << '\n';
    a["4"].as<int>() = 4;
    std::cout << a["4"] << '\n';
    for (auto iter = a.begin(); iter != a.end(); ++iter) {
        auto [first, second] = (*iter).destructure<std::string_view, int>();
        std::cout << first << ' ' << second << '\n';
    }
    a.emplace<std::tuple<const char *, int, float>>("Hello World", 42, 3.14f);
    std::cout << a[0] << '\n';
    std::cout << a[1] << '\n';
    std::cout << a[2] << '\n';
    return 0;
}
