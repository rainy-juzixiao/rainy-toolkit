## return_type {#return_type}

获取当前function对象指向的返回值类型的ctti类型标识

### 返回值

返回一个类型为`rainy::foundation::ctti::typeinfo`的对象，其中包含该对象对应类型的类型特征表示以及其哈希码和字符串表示名称。

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

以及，`return_type()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <string_view>
#include <vector>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::meta::reflection;

struct example {
    // int(example::*)(std::int64_t, int, std::string);
    int foo(std::int64_t,int , std::string) {
        return 0;
    }
};

int static_foo(int, std::string_view, std::vector<int>&) {
    return 0;
}

std::vector<std::int64_t> use_array(int[100]) {
    return {};
}

int main() {
    std::cout << std::boolalpha;
    function fn = &example::foo;
    std::cout << "example::foo ret type => " << fn.return_type().name() << "\n"; // int
    std::cout << "rainy_typeid(int) == fn.return_type() ? => "<< (rainy_typeid(int) == fn.return_type() )<<'\n'; // true
    fn.rebind(&static_foo);
    std::cout << "static_foo ret type => " << fn.return_type().name() << "\n"; // int
    std::cout << "rainy_typeid(int) == fn.return_type() ? => "<< (rainy_typeid(int) == fn.return_type() )<<'\n'; // true
    fn.rebind(&use_array);
    std::cout << "use_array ret type => " << fn.return_type().name() << '\n';
    std::cout << "rainy_typeid(int) == fn.return_type() ? => "<< (rainy_typeid(int) == fn.return_type() )<<'\n'; // false
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
example::foo ret type => int
rainy_typeid(int) == fn.return_type() ? => true
static_foo ret type => int
rainy_typeid(int) == fn.return_type() ? => true
use_array ret type => class std::vector<__int64,class std::allocator<__int64> >
rainy_typeid(int) == fn.return_type() ? => false
```
