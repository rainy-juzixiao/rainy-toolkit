## function_signature {#function_signature}

获取当前function对象的函数签名的ctti类型标识

```cpp
const rainy::foundation::ctti::typeinfo & function_signature() const noexcept;
```

### 备注

`function_signature()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

另外，对于函数对象，即具有`operator()`的，将会返回一个被自动伪造的函数签名。该函数签名与一致。但不会影响`which_belongs()`函数的返回。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>
#include <string_view>
#include <vector>

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

void int_int_fn(int, int) {
}

int main() {
    std::cout << std::boolalpha;
    function fn = &example::foo;

    std::cout << fn.function_signature().name() << '\n';
    const auto &example_foo_type = rainy_typeid(decltype(&example::foo));
    std::cout << "Is fn.function_signature equals with example_foo_type ? " << (fn.function_signature() == example_foo_type) << '\n';

    fn.rebind(&void_arg_fn);

    std::cout << fn.function_signature().name() << '\n';
    const auto &void_arg_fn_type = rainy_typeid(decltype(&void_arg_fn));
    std::cout << "Is fn.function_signature equals with void_arg_type ? " << (fn.function_signature() == void_arg_fn_type) << '\n';

    fn.rebind(&int_int_fn);

    std::cout << fn.function_signature().name() << '\n';
    const auto &int_int_fn_type = rainy_typeid(decltype(&int_int_fn));
    std::cout << "Is fn.function_signature equals with int_int_fn_type ? " << (fn.function_signature() == int_int_fn_type) << '\n';

    auto no_capture = [](std::string_view) mutable noexcept -> int { return 0; };

    fn.rebind(no_capture);

    std::cout << fn.function_signature().name() << '\n';
    const auto &no_capture_type = rainy_typeid(decltype(no_capture));
    std::cout << "Is fn.function_signature equals with no_capture_type ? " << (fn.function_signature() == no_capture_type) << '\n';
    const auto &no_capture_fx_type = rainy_typeid(int(std::string_view) noexcept);
    std::cout << "Is fn.function_signature equals with no_capture_fx_type ? " << (fn.function_signature() == no_capture_fx_type)
              << '\n';
    std::cout << fn.function_signature().name() << '\n';
    int unused{};
    auto capture = [i = std::size_t{0}, unused](std::string_view) mutable noexcept -> int { return 0; };
    fn.rebind(capture);
    const auto &capture_type = rainy_typeid(decltype(capture));
    std::cout << "Is fn.function_signature equals with no_capture_type ? " << (fn.function_signature() == capture_type) << '\n';
    const auto &capture_fx_type = rainy_typeid(int(std::string_view) noexcept);
    std::cout << "Is fn.function_signature equals with no_capture_fx_type ? " << (fn.function_signature() == capture_fx_type) << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
int(__cdecl example::* )(__int64,int,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >)
Is fn.function_signature equals with example_foo_type ? true
void(__cdecl *)(void)
Is fn.function_signature equals with void_arg_type ? true
void(__cdecl *)(int,int)
Is fn.function_signature equals with int_int_fn_type ? true
int(class std::basic_string_view<char,struct std::char_traits<char> >) noexcept
Is fn.function_signature equals with no_capture_type ? false
Is fn.function_signature equals with no_capture_fx_type ? true
int(class std::basic_string_view<char,struct std::char_traits<char> >) noexcept
Is fn.function_signature equals with no_capture_type ? false
Is fn.function_signature equals with no_capture_fx_type ? true
```
