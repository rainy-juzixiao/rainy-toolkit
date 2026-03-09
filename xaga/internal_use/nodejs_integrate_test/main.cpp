#include <rainy/meta/wtf/node_bridge.hpp>
#include <iostream>

class Foo {
public:
    Foo(int x) : x_(x) {
    }
    int bar() {
        return x_ * 2;
    }
    int x_;
};

struct Calculator {
    int value;

    explicit Calculator(int v) : value(v) {
    }

    int add(int a, int b) {
        return a + b;
    }
    double multiply(double a, double b) {
        return a * b;
    }
    std::string describe() {
        return "Calculator(value=" + std::to_string(value) + ")";
    }
    void set_value(int v) {
        value = v;
    }
    bool is_positive() const {
        return value > 0;
    }
};

inline std::string greet(const std::string &name) {
    return "Hello, " + name + "!";
}

inline int sum(int a, int b) {
    return a + b;
}

using namespace rainy::meta::wtf::javascript;

RAINY_NODE_MODULE(my_addon, exports, env) {
    environment::setup(env, exports);
    registration::class_<Calculator>("Calculator")
        .constructor<int>()
        .method("add", &Calculator::add)
        .method("multiply", &Calculator::multiply)
        .method("describe", &Calculator::describe)
        .method("isPositive", &Calculator::is_positive)
        .property("value", &Calculator::value);
    // ② JS → C++：暴露自由函数
    bind_function(exports, env, "sum", &sum);
    bind_function(exports, env, "greet", &greet);
    // ③ JS → C++：接收 JS 回调，C++ 同步调回
    bind_function(exports, env, "callWithCallback", [](int a, int b) { return a + b; });
    bind_function(exports, env, "invokeSyncCallback",
                       [](js_callable cb, int arg, js_callable cb1) -> std::string {
                           cb1.call();
                           return cb.call<std::string>(arg);
                       });
    bind_async_function(exports, env, "registerAsyncCallback", [](js_callable on_done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        on_done.call_async(std::string("ping from C++ thread"));
    });
    bind_async_function(exports, env, "invokeAsync", [](js_callable on_done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Invoke!\n";
        on_done.call_async();
    });

    Napi::Value maybe_fn = env.Global().Get("onAddonLoaded");
    if (maybe_fn.IsFunction()) {
        auto init_cb = js_callable::sync(maybe_fn.As<Napi::Function>());
        init_cb.call<void>(std::string("test_addon ready"));
    }
    return exports;
}
