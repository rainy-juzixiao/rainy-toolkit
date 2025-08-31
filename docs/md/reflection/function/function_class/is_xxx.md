## is简化查询接口

function类提供了一套用于简化查询用的接口，这些接口被声明如下，用于针对某些

```cpp
bool is_static() const noexcept;
bool is_memfn() const noexcept;
bool is_const() const noexcept;
bool is_volatile() const noexcept;

bool is_invocable(
    rainy::collections::views::array_view<
        rainy::foundation::ctti::typeinfo
    > paramlist) const noexcept; // 1
template <typename... Args>
bool is_invocable() const noexcept; // 2

template <typename... Args>
bool is_invocable_with(Args &&...args) const noexcept

bool is_noexcept() const noexcept;
bool is_invoke_for_lvalue() const noexcept;
bool is_invoke_for_rvalue() const noexcept;
```

### is_static {#is_static}

检查当前function对象是否为静态域函数。

### is_memfn {#is_memfn}

检查当前function对象是否为成员实例函数。

### is_const {#is_const}

检查当前function对象是否为成员实例函数，且检查是否具有const属性。

### is_volatile {#is_volatile}

检查当前function对象是否为成员实例函数，且检查是否具有volatile属性。

### is_invocable {#is_invocable}

检查当前function对象是否可通过其提供的参数类型进行动态调用。

### 参数

对于重载1，以下参数适用。

`paramlist`: 一个可转换到`rainy::collections::array_view`的视图。其中，需存储`rainy::collections::ctti::typeinfo`类型。

对于重载2，需提供一系列模板调用参数类型作为检查。

### 备注

不包含对成员函数的检查。例如，`int(foo::*)(int,int)`仅需传入两个可转换到int表示的CTTI标识作为列表即返回true。

### is_invocable_with {#is_invocable_with}

检查当前function对象是否可通过其提供的参数进行动态调用。

### 备注

此函数是针对于具备any、object_view这一特定类型的。通过检测底层类型识别。但是效率往往不如`is_invocable()`函数。另外，`is_invocable_with()`的备注也同样适用此部分。

### is_noexcept {#is_noexcept}

检查当前function对象是否具有noexcept属性

### is_invoke_for_lvalue {#is_invoke_for_lvalue}

检查当前function对象是否是适用于左值引用的调用。

### is_invoke_for_rvalue {#is_invoke_for_rvalue}

检查当前function对象是否是适用于右值引用的调用

<!--@include: @/static/example_code_header_cxx17.md-->

#### 另注

该示例适用于`is_static()`、`is_memfn()`、`is_const()`、`is_volatile()`、`is_noexcept()`、`is_invoke_for_lvalue()`、`is_invoke_for_rvalue()`等函数的使用。

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

struct example {
    int foo(std::int64_t, int, std::string) const volatile {
        return 0;
    }

    int left(std::int64_t, int, std::string) & {
        return 0;
    }

    int right(std::int64_t, int, std::string) && {
        return 0;
    }
};

void void_arg_fn() noexcept {
}

void print_is_xxx(std::string_view name, const function &f) noexcept {
    std::cout << "for " << name << '\n';
    std::cout << std::boolalpha;
    std::cout << "f is_static ? " << f.is_static() << "\n";
    std::cout << "f is_memfn ? " << f.is_memfn() << "\n";
    std::cout << "f is_const ? " << f.is_const() << "\n";
    std::cout << "f is_volatile ? " << f.is_volatile() << "\n";
    std::cout << "f is_noexcept ? " << f.is_noexcept() << "\n";
    std::cout << "f is_invoke_for_lvalue ? " << f.is_invoke_for_lvalue() << "\n";
    std::cout << "f is_invoke_for_rvalue ? " << f.is_invoke_for_rvalue() << "\n";
}

int main() {
    /**
     * is_static
     * is_memfn
     * is_const
     * is_volatile
     * is_noexcept
     * is_invoke_for_lvalue
     * is_invoke_for_rvalue
     */
    function f = &void_arg_fn;
    print_is_xxx("void_arg_fn", f);
    f.rebind(&example::foo);
    print_is_xxx("example::foo", f);
    f.rebind(&example::left);
    print_is_xxx("example::left", f);
    f.rebind(&example::right);
    print_is_xxx("example::right", f);
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
for void_arg_fn   
f is_static ? true
f is_memfn ? false
f is_const ? false
f is_volatile ? false
f is_noexcept ? true
f is_invoke_for_lvalue ? false
f is_invoke_for_rvalue ? false
for example::foo
f is_static ? false
f is_memfn ? true
f is_const ? true
f is_volatile ? true
f is_noexcept ? false
f is_invoke_for_lvalue ? false
f is_invoke_for_rvalue ? false
for example::left
f is_static ? false
f is_memfn ? true
f is_const ? false
f is_volatile ? false
f is_noexcept ? false
f is_invoke_for_lvalue ? true
f is_invoke_for_rvalue ? false
for example::right
f is_static ? false
f is_memfn ? true
f is_const ? false
f is_volatile ? false
f is_noexcept ? false
f is_invoke_for_lvalue ? false
f is_invoke_for_rvalue ? true
```

<!--@include: @/static/example_code_header_cxx17.md-->

#### 另注

该示例适用于`is_invocable()`、`is_invocable_with()`等函数的使用。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections;
using namespace rainy::collections::views;
using namespace rainy::foundation::ctti;
using namespace rainy::meta::reflection;

struct example {
    int foo(std::int64_t, int, std::string_view) const volatile {
        return 0;
    }
};

void arg_fn(int, std::string) noexcept {
}

int main() {
    function f = &arg_fn;
    // is_invocable
    if (f.is_invocable<int, std::string>()) {
        std::cout << "arg_fn is invocable with (int, std::string)\n";
    }
    f.rebind(&example::foo);
    if (f.is_invocable<int, int , std::string>()) {
        std::cout << "example::foo is invocable with (int, int, std::string)\n";
    }
    utility::any a = "C-str";
    if (f.is_invocable_with(10,10, a)) {
        std::cout << "example::foo is invocable with (int, int, rainy::utility::any[const char*])\n";
    }
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
arg_fn is invocable with (int, std::string)
example::foo is invocable with (int, int, std::string)
example::foo is invocable with (int, int, rainy::utility::any[const char*])
```