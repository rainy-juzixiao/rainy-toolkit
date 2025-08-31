## invoke

向function对象中存储的函数实例，发起调用，以对象上下文函数或静态域又或对象实例函数方式

```cpp
template <typename... Args>
rainy::utility::any invoke(
    rainy::meta::reflection::object_view instance,
    Args &&...args
) const;
```

### 参数

`instance`: 一个对象实例，可能是`non_exists_instance`（一个对象占位符，表示不存在的对象实例，用于调用静态域函数）也或者是一个对象实例的引用。
`args...`: 任意数量的参数，用于提供给目标。另注：`rainy::utility::any`支持被`拆箱（自动处理内部类型）`，`数组`会被退化为指针，不保留原类型

### 返回值

如果调用目标的返回类型不为void，则此部分返回的any将包含实际的返回值内容。如，目标调用返回`int`类型，数据为`42`。则`any`将包含`int`类型的ctti信息和数据`42`。

### 备注

若当前对象存储的是一个对象实例函数，需要提供与`which_belongs()`返回的类型信息兼容的对象。否则会调用失败。在DEBUG模式下会检查是否为`non_exists_instance`。若是，则会触发契约错误。契约将会在DEBUG模式下触发一个断点。且无法直接捕获。在RELEASE模式不会进行检查。因此，若调用目标是个对象实例函数，则产生未定义行为。如空指针访问成员函数等。

另外，在调用的时候，会检查参数的可转换性，通常会计算参数是否适合且参数是否完全一致。若参数数量不一致，则`errno`会被设置为`EINVAL`，则尝试检查类型兼容性并在可能的情况下尝试不同类型的转换（`可能会导致std::terminate()`的调用，如果异常被禁用。启用异常则会抛出bad_any_cast，由库定义而非标准库bad_any_cast）。但是，如果调用所需类型未被用于特化，则`errno`也会被设置为`EACCES`。表示无法找到适当的类型转换。（默认情况下提供对拷贝/移动构造、同类型引用转换、算术类型转换、`std::string_view`转换的支持。详见`any`文档的`any_converter`描述）。

如果在绑定函数的时候，若提供了默认参数，则允许不提供参数，若对应默认参数可用。例如，若为int(int,char)提供char类型参数，则仅需要提供int的，或是全部提供，则无需提供参数调用。且，允许覆盖。

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

struct class_a {
    void change() noexcept {
        std::cout << "change()\n";
        a = 10;
        b = 20;
    }

    void print() const noexcept {
        std::cout << "print()\n";
        std::cout << a << '\n';
        std::cout << b << '\n';
    }

    int a;
    int b;
};

void foo() noexcept {
    std::cout << "foo\n";
}

void foo1() {
    std::cout << "foo1\n";
}

int main() {
    using namespace rainy::meta::reflection;
    function f1 = &foo;
    function f2 = &foo1;
    f1.invoke(non_exists_instance); // invoke f1 , f1 --> foo()
    f2.invoke(non_exists_instance); // invoke f2, f2 --> foo1()
    function a_change = &class_a::change;
    function a_print = &class_a::print;
    class_a object{};
    a_change.invoke(object); // invoke a_change, a_change --> a::change()
    a_print.invoke(object); // invoke a_print, a_print --> a::print()
    auto lmb = [](int a, const int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
    };
    function lambda_without_capture = [](int a, const int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
    }; // bind a non-capture lambda expression
    std::cout << "lambda_without_capture's signature: " << lambda_without_capture << '\n';
    lambda_without_capture.invoke(non_exists_instance, 10, 30);
    // invoke lambda_without_capture, lambda_without_capture --> lambda
    auto lambda = [&](int a, int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
        std::cout << f1 << '\n';
    }; // bind a capture lambda expression
    function lambda_with_capture = lambda;
    lambda_with_capture.invoke(non_exists_instance, 10, 30);
    std::cout << "lambda_with_capture's signature: " << lambda_with_capture << '\n';
    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
foo
foo1
change()
print()
10
20
lambda_without_capture's signature: void(__cdecl main::<lambda_2>::* )(int,const int) const -> 000000EC348FF8A0
This Is Lambda
a = 10
b = 30
void(__cdecl *)(void) noexcept -> 000000EC348FF6E0
lambda_with_capture's signature: void(__cdecl main::<lambda_3>::* )(int,int) const -> 000000EC348FF920
```
