## which_belongs {#which_belongs}

获取当前function对象中持有实例具体所属的类类型信息。

```cpp
const rainy::foundation::ctti::typeinfo & which_belongs() const noexcept;
```

### 返回值

当前function对象持有实例具体所属的类类型信息。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

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

int main() {
    function f = &example::foo;
    std::cout << f.which_belongs().name() << '\n';
    f.rebind(&void_arg_fn);
    std::cout << f.which_belongs().name() << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
struct example
static[no-definite-class-type]
```