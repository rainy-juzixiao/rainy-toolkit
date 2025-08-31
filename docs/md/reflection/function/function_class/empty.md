## empty

检查当前function对象是否为空

```cpp
bool empty() const noexcept;
```

### 返回值

如果为空返回true，否则false

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

struct example {
    // int(example::*)(std::int64_t, int, std::string);
    int foo(std::int64_t, int, std::string) {
        return 0;
    }
};

int main() {
    function f = &example::foo;
    if (!f.empty()) {
        std::cout << "f is not empty.\n";
    }
    f = nullptr;
    if (f.empty()) {
        std::cout << "now f is empty.\n";
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
f is not empty.
now f is empty.
```