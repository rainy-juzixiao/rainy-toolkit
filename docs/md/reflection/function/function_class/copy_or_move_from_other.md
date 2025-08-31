## copy_from_other {#copy_from_other}

拷贝一个function对象存储的函数指针到此function对象中。

```cpp
void copy_from_other(const function &right) noexcept;
```

### 参数

`right`: 待拷贝的对象

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
        std::cout << "Now f is not empty.\n";
    }
    function f_copy{};
    f_copy.copy_from_other(f);
    if (!f_copy.empty()){
        std::cout << "Now f_copy is not empty by copy from f.\n";
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
Now f is not empty.
Now f_copy is not empty by copy from f.
```

## move_from_other {#move_from_other}

将function对象存储的函数指针转移到此function对象中。

```cpp
void move_from_other(function &&right) noexcept;
```

### 参数

`right`: 待移动的对象

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
        std::cout << "Now f is not empty.\n";
    }
    function f_move{};
    f_move.move_from_other(std::move(f));
    if (!f_move.empty() && f.empty()){
        std::cout << "Now f_move is not empty. also f have been moved.\n";
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
Now f is not empty.
Now f_move is not empty. also f have been moved.
```