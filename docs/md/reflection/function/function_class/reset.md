## reset

将function对象进行状态重置。

```cpp
void reset() noexcept;
```

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

void arg_fn(int, std::string) noexcept {
}

int main() {
    function f = &arg_fn;
    if (!f.empty()) {
        std::cout << "This message will output\n";
    }
    f.reset();
    if (f.empty()) { 
        std::cout << "This message will output after f.reset() called\n";
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
This message will output
This message will output after f.reset() called
```