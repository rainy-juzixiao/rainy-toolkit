## arg

获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息

```cpp
const rainy::foundation::ctti::typeinfo &arg(
    std::size_t idx
) const;
```

### 参数

`idx`: 指定的索引

### 返回值

当前function对象存储的参数列表对应的类型信息

### 异常

产生`out_of_range`异常，当`idx >= arity()`

### 备注

`arg()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

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
    int foo(std::int64_t,int , std::string) {
        return 0;
    }
};

void void_arg_fn() {
}

void int_int_fn(int ,int) {
}

int main() {
    function fn = &example::foo;
    std::cout << "example::foo: \n";
    for (int i = 0; i < fn.arity(); ++i) {
        std::cout << fn.arg(i).name() << '\n';
    }
    // fn.arg(4) <-- it will fail
    fn.rebind(&void_arg_fn);
    // fn.arg(0) <-- it will fail;
    fn.rebind(&int_int_fn);
    std::cout << '\n';
    std::cout << fn.arg(0).name() << '\n';
    std::cout << fn.arg(1).name() << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
example::foo:
__int64
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
int_int_fn:
int
int
```