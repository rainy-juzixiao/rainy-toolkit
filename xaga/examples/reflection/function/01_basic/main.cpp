#include <iostream>
#include <optional>
#include <algorithm>
#include <rainy/utility/any.hpp>
#include <rainy/collections/inplace_vector.hpp>
#include <rainy/collections/string.hpp>
#include <rainy/collections/views/views_interface.hpp>
#include <rainy/collections/views/transform_view.hpp>
#include <rainy/foundation/container/optional.hpp>
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

constexpr auto test1() {
    foundation::container::optional<std::string_view> v = "hello world";
    foundation::container::optional<std::string_view> v2;
    return v2.value_or("SB");
}

int funccc(int count, ...) {
    va_list args{};
    va_start(args, count);
    int tmp = va_arg(args, int);
    va_end(args);
    return 42 + tmp;
}

int main() {
    foundation::functional::function_pointer fp = &funccc;
    std::cout << fp.invoke_variadic(1, 100) << '\n';
    fp.cast<int(int)>();
    {
        std::optional<std::string_view> v = "hello world";
        std::optional<std::string_view> v2(v);
    }
    constexpr auto vec = test1();
    
    std::vector<int> cont{1,2,3,4,5,6,7};
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
