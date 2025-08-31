## paramlists

获取当前function对象的调用参数的类型标识列表

```cpp
const rainy::collections::views::array_view<
    rainy::foundation::ctti::typeinfo
>& paramlists() const noexcept;
```

### 返回值

返回当前function对象中持有的函数指针对应的参数列表信息

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言。另外，对paramlists进行const_cast修改，将导致未定义行为

以及，`paramlists()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断


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

int static_foo(int, std::string_view, std::vector<int>&) {
    return 0;
}

std::vector<std::int64_t> use_array(int[100]) {
    return {};
}

void cv_ref_fn(const int&, volatile double&&) {}

void test_fnptr(fp) {}

void print(std::string_view fn_name, const array_view<typeinfo>& array_view) {
    std::cout << fn_name << "(";
    for (std::size_t i = 0; i < array_view.size(); ++i) {
        std::cout << array_view[i].name();
        if (i != array_view.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ")\n";
    std::cout << fn_name << " paramlist's details\n{\n";
    for (auto&& type : array_view) {
        std::cout << '\t' << type.name() 
            << ", is_ref: " << (type.has_traits(traits::is_lref) || type.has_traits(traits::is_rref)) 
            << ", is_cv: " << (type.has_traits(traits::is_const) && type.has_traits(traits::is_volatile)) << std::endl;
    }
    std::cout << "}\n";
    std::cout.put('\n');
}

int main() {
    std::cout << std::boolalpha;
    function fn = &example::foo;
    print("example::foo", fn.paramlists()); // paramlists() returns a view reference
    fn.rebind(&static_foo);
    print("static_foo", fn.paramlists());
    fn.rebind(&use_array);
    print("use_array", fn.paramlists());
    fn.rebind(&cv_ref_fn);
    print("cv_ref_fn", fn.paramlists());
    fn.rebind(&test_fnptr);
    print("test_fnptr", fn.paramlists());
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
example::foo(__int64, int, class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >)
example::foo paramlist's details
{
        __int64, is_ref: false, is_cv: false
        int, is_ref: false, is_cv: false
        class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >, is_ref: false, is_cv: false
}

static_foo(int, class std::basic_string_view<char,struct std::char_traits<char> >, class std::vector<int,class std::allocator<int> >&)
static_foo paramlist's details
{
        int, is_ref: false, is_cv: false
        class std::basic_string_view<char,struct std::char_traits<char> >, is_ref: false, is_cv: false
        class std::vector<int,class std::allocator<int> >&, is_ref: true, is_cv: false
}

use_array(int*)
use_array paramlist's details
{
        int*, is_ref: false, is_cv: false
}

cv_ref_fn(const int&, volatile double&&)
cv_ref_fn paramlist's details
{
        const int&, is_ref: true, is_cv: false
        volatile double&&, is_ref: true, is_cv: false
}

test_fnptr(void(__cdecl *)(float))
test_fnptr paramlist's details
{
        void(__cdecl *)(float), is_ref: false, is_cv: false
}

```
