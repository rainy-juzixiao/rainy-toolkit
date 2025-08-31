## 函数名

blahblah

<!-- 函数签名 -->
```cpp
```

### 参数

### 返回值

### 异常

### 备注



<!--@include: @/static/example_code_header_cxx17.md-->

<!-- 示例代码 -->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

struct example {
    void hello() {
        std::cout << "Hello method\n";
    }
};

void arg_fn(int, std::string) noexcept {
    std::cout << "This is arg_fn\n";
}

int main() {
    function f = &arg_fn;
    if (auto ptr = f.target<decltype(&arg_fn)>(); *ptr == &arg_fn) {
        std::cout << "Successfully to get target from f[bind -> arg_fn]\n";
        std::invoke(*ptr, 10, "H");
    }
    f.rebind(&example::hello);
    if (auto mptr = f.target<decltype(&example::hello)>(); *mptr == &example::hello) {
        std::cout << "Successfully to get target from f[bind -> example::hello]\n";
        example tmp;
        std::invoke(*mptr, tmp);
    }
    int x{10};
    auto lmb = [&x]() {
        std::cout << "call lambda\n";
        std::cout << "x = " << x << "\n";
    };
    f.rebind(lmb);
    using lambda_t = decltype(lmb);
    if (auto lmb_ptr = f.target<lambda_t>(); lmb_ptr != &lmb) {
        std::cout << "Successfully to get target from f[bind -> lmb]\n";
        std::cout << "Try to call lmb\n";
        std::invoke(*lmb_ptr);
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
Successfully to get target from f[bind -> arg_fn]
This is arg_fn
Successfully to get target from f[bind -> example::hello]
Hello method
Successfully to get target from f[bind -> lmb]
Try to call lmb
call lambda
x = 10
```