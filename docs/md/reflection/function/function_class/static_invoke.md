## static_invoke {#static_invoke}

向function对象中存储的函数实例，发起调用，以对象上下文函数或静态域函数方式

```cpp
template <typename... Args>
rainy::utility::any static_invoke(Args &&...args);
```

### 参数

`args...`: 任意数量的参数，用于提供给目标。另注：`rainy::utility::any`支持被`拆箱（自动处理内部类型）`，`数组`会被退化为指针，不保留原类型。

### 返回值

如果调用目标的返回类型不为void，则此部分返回的any将包含实际的返回值内容。如，目标调用返回`int`类型，数据为`42`。则`any`将包含`int`类型的ctti信息和数据`42`。

### 备注

该接口是对[invoke](#invoke)函数的封装。其效果与下列调用等效。详见示例。

```cpp
fn.invoke(non_exists_instance, args...);
```

此函数是为了语义进行封装的，使用`invoke_static`即明确表示当前调用的是一个静态域函数或对象上下文函数。更多详细细节请参见invoke函数的描述。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

void foo() noexcept {
    std::cout << "foo\n";
}

int foo1(int a, int b) {
    std::cout << "foo1\n";
    std::cout << a << '\n';
    std::cout << b << '\n';
    return a + b;
}

int main() {
    using namespace rainy::meta::reflection;
    function f1 = &foo;
    f1.static_invoke(); // No need to pass non_exists_instance.
    f1.invoke(non_exists_instance); // Same effect with f1.invoke_static()
    function f2 = &foo1;
    f2.static_invoke(10, 20);
    f2.invoke(non_exists_instance, 10, 20); // Same effect with f2.invoke_static()

    rainy::utility::any result_of_foo1 = f2.static_invoke(10, 20);
    std::cout << "f2 returns: " << result_of_foo1.as<int>() << '\n';
    
    function f3 = {[](int a, int b) {
        std::cout<< "f3: a + b = " << (a + b) << '\n';
    }, 70};
    f3.static_invoke(30);
    f3.static_invoke(30, 40);

    return 0;
}
```

<!--@include: @/static/example_output_header_cxx17.md-->

```text
foo
foo
foo1
10
20
foo1
10
20
foo1
10
20
f2 returns: 30
f3: a + b = 100
f3: a + b = 70
```
