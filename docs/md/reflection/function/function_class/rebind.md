## rebind

为function对象重新绑定一个函数实例。

### 1. `void rebind(function &&function)`

重新绑定一个右值引用的function对象进行转移。

```cpp
void rebind(function &&function) noexcept;
```

#### 参数

`function`: 待绑定的右值引用function对象。

### 2. `void rebind(const function &function)`

重新绑定一个左值引用的function对象进行转移。

```cpp
void rebind(const function &function) noexcept;
```

#### 参数

`function`: 待绑定的左值引用function对象。


### 3. `void rebind(std::nullptr_t)`

将当前function对象置空。

```cpp
void rebind(std::nullptr_t) noexcept;
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
    function f1;
    f1.rebind(f); // f as const-lref to rebind
    assert(!f1.empty()); // it should be passed
    f1.rebind(std::move(f)); // f as rref to rebind
    assert(f.empty()); // f has been moved
    f1.rebind(nullptr); // rebind a nullpointer
    assert(f1.empty()); // f1 is null
    return 0;
}
```

<!--@include: @/static/example_assertion.md-->