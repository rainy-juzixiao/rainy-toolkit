## type

获取当前function对象的调用类型。不过，该接口一般是用于拓展生成序列化代码。一般用户不建议使用。而是应当使用[has](#has)接口

```cpp
rainy::meta::reflection::method_flags type() const noexcept;
```

### 返回值

其返回值是以下的一个枚举位组合成的一个值，用于检索其属性。

```cpp
enum class method_flags : std::uint8_t {
    none = 0,
    static_qualified = 1, // static method
    memfn_specified = 2, // member method
    noexcept_specified = 4, // noexcept
    lvalue_qualified = 8, // left qualifier (e.g. &)
    rvalue_qualified = 16, // right qualifier (e.g. &&)
    const_qualified = 32, // const
    volatile_qualified = 64, // volatile
};
```

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

class myfoo {
public:
    void foo() noexcept {
    }

    void foo_const() const noexcept {
    }

    void foo_volatile() volatile noexcept {
    }
};

void foo() noexcept {
    std::cout << "foo\n";
}

int foo1(int a, int b) {
    std::cout << "foo1\n";
    std::cout << a << '\n';
    std::cout << b << '\n';
    return a + b;
}

int main() {
    using namespace rainy::meta::reflection;
    function f1 = &foo;
    if (static_cast<bool>(f1.type() & method_flags::noexcept_specified) || static_cast<bool>(f1.type() & method_flags::static_specified)) {
        std::cout << "f1 is noexcept or static\n";
    } else {
        std::cout << "f1 is not noexcept or static\n";
    }
    function f2 = &foo1;
   if (static_cast<bool>(f1.type() & method_flags::static_specified)) {
        std::cout << "f2 is static\n";
    } else {
        std::cout << "f2 is not static\n";
    }
    function f3 = {[](int a, int b) { std::cout << "f3: a + b = " << (a + b) << '\n'; }};
    if (static_cast<bool>(f3.type() & method_flags::static_specified)) {
        std::cout << "f3 is static\n";
    } else {
        std::cout << "f3 is not static\n";
    }
    function f4 = &myfoo::foo;
    if (static_cast<bool>(f4.type() & method_flags::noexcept_specified)) {
        std::cout << "f4 is noexcept\n";
    } else {
        std::cout << "f4 is not noexcept\n";
    }
    f4.rebind(&myfoo::foo_const);
    if (static_cast<bool>(f4.type() & method_flags::const_qualified)) {
        std::cout << "f4 is const\n";
    } else {
        std::cout << "f4 is not const\n";
    }
    f4.rebind(&myfoo::foo_volatile);
    if (static_cast<bool>(f4.type() & method_flags::volatile_qualified)) {
        std::cout << "f4 is volatile\n";
    } else {
        std::cout << "f4 is not volatile\n";
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->


```text
f1 is noexcept or static
f2 is static
f3 is static
f4 is noexcept
f4 is const
f4 is volatile
```
