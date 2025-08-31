## swap

将function对象的函数指针与当前function对象的函数指针进行交换

```cpp
void swap(function &right) noexcept;
```

### 参数

`right` 待交换的对象

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

void arg_fn(int, std::string) noexcept {
}

void arg_fn1() {}

int main() {
    function f_1 = &arg_fn;
    function f_2 = &arg_fn1;
    std::cout << "f_1 = " << f_1 << '\n';
    std::cout << "f_2 = " << f_2 << '\n';
    f_1.swap(f_2);
    std::cout << "f_1 = " << f_1 << '\n';
    std::cout << "f_2 = " << f_2 << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
f_1 = void(__cdecl *)(int,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >) noexcept -> 000000EC8599FC00
f_2 = void(__cdecl *)(void) -> 000000EC8599FC60
f_1 = void(__cdecl *)(void) -> 000000EC8599FC00
f_2 = void(__cdecl *)(int,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >) noexcept -> 000000EC8599FC60
```