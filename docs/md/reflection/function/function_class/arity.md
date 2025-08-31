## arity

获取调用当前function的所需参数数量

```cpp
std::size_t arity() const noexcept;
```

### 返回值

返回当前function对象中持有的函数指针对应的参数列表的参数总数量

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <string_view>
#include <vector>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

using fp = void(*)(float);

struct example {
    // int(example::*)(std::int64_t, int, std::string);
    int foo(std::int64_t, int , std::string) {
        return 0;
    }
};

int static_foo(int, std::string_view, std::vector<int>&) {
    return 0;
}

std::vector<std::int64_t> use_array(int[100]) {
    return {};
}

void cv_ref_fn(const int&, volatile double&&) {}

void test_fnptr(fp) {}

int main() {
    function fn = &example::foo;
    std::cout << "example::foo 's arity -> " << fn.arity() << '\n';
    fn.rebind(&static_foo);
    std::cout << "static_foo 's arity -> " << fn.arity() << '\n';
    fn.rebind(&use_array);
    std::cout << "use_array 's arity -> " << fn.arity() << '\n';
    fn.rebind(&cv_ref_fn);
    std::cout << "cv_ref_fn 's arity -> " << fn.arity() << '\n';
    fn.rebind(&test_fnptr);
    std::cout << "test_fnptr 's arity -> " << fn.arity() << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
example::foo 's arity -> 3
static_foo 's arity -> 3  
use_array 's arity -> 1   
cv_ref_fn 's arity -> 2   
test_fnptr 's arity -> 1  
```
