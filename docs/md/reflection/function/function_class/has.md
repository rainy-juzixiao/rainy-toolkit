## has

用于检索当前function对象是否具有特定的属性，封装自[type](#type)接口。

```cpp
bool has(rainy::meta::reflection::method_flags flag) const noexcept;
```

### 参数

`flag`：取自`rainy::meta::reflection`的枚举类：
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

关于method_flags的说明，请参见[method_flags枚举](/md/reflection/function/method_flags)页面。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>
#include <string_view>
#include <vector>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

using fp = void (*)(float);

struct example {
    // int(example::*)(std::int64_t, int, std::string);
    int foo(std::int64_t, int, std::string) {
        return 0;
    }
};

void void_arg_fn() {
}

void int_int_fn(int, int) {
}

int main() {
    std::cout << std::boolalpha;
    function f = []() noexcept {};
    std::cout << "f is noexcept ? " << f.has(method_flags::noexcept_specified) << '\n';
    std::cout << "f is static ? " << f.has(method_flags::static_specified) << '\n';
    std::cout << "f is memfn ? " << f.has(method_flags::memfn_specified) << '\n';
    f = &example::foo;
    std::cout << "f is noexcept ? " << f.has(method_flags::noexcept_specified) << '\n';
    std::cout << "f is static ? " << f.has(method_flags::static_specified) << '\n';
    std::cout << "f is memfn ? " << f.has(method_flags::memfn_specified) << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
f is noexcept ? true
f is static ? true
f is memfn ? false
f is noexcept ? false
f is static ? false
f is memfn ? true
```