#include <algorithm>
#include <iostream>
#include <optional>
#include <rainy/collections/inplace_vector.hpp>
#include <rainy/component/willow/json.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/metadata.hpp>
#include <rainy/meta/reflection/registration.hpp>
#include <rainy/meta/reflection/shared_object.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <rainy/text/string.hpp>
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

enum class color {
    red,
    green,
    blue
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

struct Address {
    std::string city;
    int zip;
};

struct User {
    int id;
    std::string name;
    std::vector<int> scores;
    Address addr;
};


void fun() {
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

#include "header.h"

struct S {
    int val;
    int minus(int arg) const noexcept {
        return arg - val;
    }
};

#include <rainy/foundation/concurrency/concurrency.hpp>

using namespace rainy::foundation::concurrency;

monad_future<bool> async_check_username(const std::string &username) {
    auto state = std::make_shared<shared_state<bool>>();
    std::thread([state, username]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 模拟网络延迟

        // 模拟检查逻辑：admin 已被占用
        bool available = (username != "admin");
        state->set_value(available);
    }).detach();

    return monad_future<bool>(state);
}

struct MyUser {
    int id;
    std::string name;
    std::string email;
};

monad_future<MyUser> async_create_user(const std::string &name, const std::string &email) {
    auto state = std::make_shared<shared_state<MyUser>>();

    std::thread([state, name, email]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));

        static int next_id = 1000;
        MyUser user{next_id++, name, email};
        state->set_value(user);
    }).detach();

    return monad_future<MyUser>(state);
}

// 模拟异步发送欢迎邮件
monad_future<void> async_send_welcome_email(const MyUser &user) {
    auto state = std::make_shared<shared_state<void>>();

    std::thread([state, user]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        std::cout << "📧 欢迎邮件已发送给: " << user.name << " (" << user.email << ")" << std::endl;
        state->set_value();
    }).detach();

    return monad_future<void>(state);
}

// 模拟异步记录日志
monad_future<void> async_log_registration(const MyUser &user) {
    auto state = std::make_shared<shared_state<void>>();

    std::thread([state, user]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "📝 注册日志已记录: User #" << user.id << std::endl;
        state->set_value();
    }).detach();

    return monad_future<void>(state);
}

// 模拟可能失败的异步操作
monad_future<int> async_calculate_score(const MyUser &user) {
    auto state = std::make_shared<shared_state<int>>();

    std::thread([state, user]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        // 50% 概率失败
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dist(0, 1);

        if (dist(gen) == 0) {
            // 成功
            int score = user.id * 10;
            state->set_value(score);
        } else {
            // 失败
            try {
                throw std::runtime_error("计算服务暂时不可用");
            } catch (...) {
                state->set_exception(std::current_exception());
            }
        }
    }).detach();

    return monad_future<int>(state);
}

class ThreadPoolExecutor {
public:
    void submit(std::function<void()> task) {
        std::thread([task]() {
            std::cout << "  [线程池] 执行任务..." << std::endl;
            task();
        }).detach();
    }
};

// UI线程执行器（模拟在主线程执行）
class UIThreadExecutor {
public:
    void submit(std::function<void()> task) {
        // 实际应用中会post到UI消息队列
        std::cout << "  [UI线程] 执行任务..." << std::endl;
        task(); // 这里简化为直接执行
    }
};

int main() {
    S m{};
    std::cout << m.val << '\n';
    // get_overloaded_func<void()>(&f);
    any a = 10;
    std::cout << std::as_const(a).as_lvalue_reference().type().name() << '\n';
    std::cout << std::as_const(a).as_rvalue_reference().type().name() << '\n';
    a = std::array<int, 5>{5, 1, 2, 4, 3};
    auto a_ref = a.as_lvalue_reference();
    for (const auto i: a_ref) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    std::sort(a.begin(), a.end(), [](const auto &left, const auto &right) { return right > left; });
    for (const auto i: a_ref) {
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
