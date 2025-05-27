# function 类

## 成员
|成员函数|说明|
|-|-|
|[function](#function)|用于构造function对象|
|[invoke_static](#invoke_static)|向function对象中存储的函数指针，发起调用，以静态域函数方式|
|[invoke](#invoke)|向function对象中存储的函数指针，发起调用|
|[return_type](#return_type)|获取当前function对象指向的返回值类型的rtti类型标识|
|[paramlists](#paramlists)|获取当前function对象的调用参数的类型标识列表|
|[arg_count](#arg_count)|获取调用当前function的所需参数数量|
|[arg](#arg)|获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息|
|[function_signature](#function_signature)|获取当前function对象的函数签名的rtti类型标识|
|[type](#type)|获取当前function对象具体属于哪种调用类型|
|[which_belongs](#which_belongs)|获取当前function对象中，函数指针具体所属的类类型信息|
|[empty](#empty)|检查当前function对象是否为空|
|[is_static](#is_static)|检查当前function对象是否为静态域函数|
|[is_memfn](#is_memfn)|检查当前function对象是否为成员实例函数|
|[is_const](#is_const)|检查当前function对象是否为成员实例函数，且检查是否具有const属性|
|[is_volatile](#is_volatile)|检查当前function对象是否为成员实例函数，且检查是否具有volatile属性|
|[is_invocable](#is_invocable)|检查当前function对象是否可通过其提供的参数进行动态调用|
|[is_noexcept](#is_noexcept)|检查当前function对象是否具有noexcept属性|
|[is_invoke_for_lvalue](#is_invoke_for_lvalue)|检查当前function对象是否是适用于左值引用的调用|
|[is_invoke_for_rvalue](#is_invoke_for_rvalue)|检查当前function对象是否是适用于右值引用的调用|
|[copy_from_other](#copy_from_other)|拷贝一个function对象存储的函数指针到此function对象中|
|[move_from_other](#move_from_other)|将function对象存储的函数指针转移到此function对象中|
|[swap](#swap)|将function对象的函数指针与当前function对象的函数指针进行交换|
|[clear](#clear)|将function对象进行置空|
|[rebind](#rebind)|为function对象重新绑定一个函数指针|
|[target](#target)|安全提取function对象中存储的函数指针|
|[equal_with](#equal_with)|检查一个function对象与当前的function对象是否一致|
|[not_equal_with](#not_equal_with)|检查一个function对象与当前的function对象是否为不同的|

|运算符|说明|
|-|-|
|[function::operator bool()](#function-operator-bool)|检查当前对象是否有效|
|[function::operator=](#function-assign-operator)|拷贝function对象或修改function对象|

## function

### 1. `function()`/`function(std::nullptr_t)`

用于构造一个空的function对象

```cpp
function() noexcept;
function(std::nullptr_t) noexcept;
```

### 2. `function(function &&right)`/`function(const function &right)`

用于拷贝或移动function对象

```cpp
function(function &&right) noexcept;
function(const function &right) noexcept;
```

#### 参数

`right`: 待移动或拷贝的function对象

### 3. `function(...)`

通过函数指针，构造function对象

```cpp
template <typename Fx>
function(Fx function) noexcept;
```

#### 参数

`function`: 可支持构造的函数指针，但是必须是*静态域函数、对象实例函数、仿函数*这类的。

#### 备注

对于仿函数，该构造函数将会进行类型推导，首先获取operator()。
随后检查仿函数的类型是否可被用于直接构造并调用，如果可能，则存储一个代理函数对象，以*静态域*函数的形式存在。否则作为成员函数进行存储，以*对象实例函数*形式存在。

### 示例

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

struct a {
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
    function f3 = f1; // <-- f3 copied f1's resources
    f3.invoke(non_exists_instance); // invoke f3 , f3 --> foo()
    function f4 = std::move(f3); // f4 move f1's resources
    f4.invoke(non_exists_instance); // invoke f4 , f4 --> foo()
    f2.invoke(non_exists_instance); // invoke f2, f2 --> foo1()
    function a_change = &a::change;
    function a_print = &a::print;
    a object{};
    a_change.invoke(object); // invoke a_change, a_change --> a::change()
    a_print.invoke(object); // invoke a_print, a_print --> a::print()
    function lambda_without_capture = [](int a, const int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
    }; // bind a non-capture lambda expression
    std::cout << "lambda_without_capture's signature: " << lambda_without_capture << '\n';
    lambda_without_capture.invoke_static(10, 30);
    // invoke lambda_without_capture, lambda_without_capture --> lambda
    auto lambda = [&](int a,int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
        std::cout << f1 << '\n';
    }; // bind a capture lambda expression
    function lambda_with_capture = lambda;
    lambda_with_capture.invoke(lambda, 10, 30);
    std::cout << "lambda_without_capture's signature: " << lambda_with_capture << '\n';
    return 0;
}
/*Output
foo
foo
foo1
change()
print()
10
20
lambda_without_capture's signature: void(__cdecl *)(int,int) noexcept(false) -> 00007FF6496FA7A0
This Is Lambda
a = 10
b = 30
This Is Lambda
a = 10
b = 30
void(__cdecl *)(void) noexcept -> 00007FF6496F1974
lambda_without_capture's signature: void(__cdecl main::<lambda_2>::* )(int,int) const -> 00007FF6496F8DE0
*/
```

## invoke_static {#invoke_static}

向function对象中存储的函数指针，发起调用，以静态域函数方式

```cpp
template <typename... Args>
rainy::utility::any invoke_static(Args &&...args);
```

### 参数

`args...`: 任意数量的参数，用于提供给目标。另注：`rainy::utility::any`支持被`拆箱（自动处理内部类型）`，`数组`会被退化为指针，不保留原类型

### 返回值

如果调用目标的返回类型不为void，则此部分返回的any将包含实际的返回值内容。如，目标调用返回`int`类型，数据为`42`。则`any`将包含`int`类型的rtti信息和数据`42`。

### 备注

该接口是对[invoke](#invoke)函数的封装。其效果与下列调用等效。详见示例。

```cpp
fn.invoke(non_exists_instance, args...);
```

此函数是为了语义进行封装的，使用`invoke_static`即明确表示当前调用的是一个静态域函数，而非对象实例函数。倘若当前对象存储的是一个对象实例函数，则会调用失败。在DEBUG模式下，会检查是否为`non_exists_instance`。若不是，则会触发契约错误。契约将会在DEBUG模式下触发一个断点。且无法直接捕获。在RELEASE模式不会进行检查。因此，若调用目标是个对象实例函数，则产生未定义行为。如空指针访问成员函数等

另外，在调用的时候，会检查参数的可转换性，通常会计算参数是否适合且参数是否完全一致。若参数数量不一致，则`errno`会被设置为`EINVAL`，则尝试检查类型兼容性并在可能的情况下尝试不同类型的转换（`可能会导致std::terminate()`的调用）。但是，如果调用所需类型未被用于特化，则`errno`也会被设置为`EACCES`。表示无法找到适当的类型转换。（默认情况下提供对拷贝/移动构造、同类型引用转换、算术类型转换、`std::string_view`转换的支持。详见`any`文档的`any_converter`描述）。

### 示例

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
    f1.invoke_static(); // No need to pass non_exists_instance.
    f1.invoke(non_exists_instance); // Same effect with f1.invoke_static()
    function f2 = &foo1;
    f2.invoke_static(10, 20);
    f2.invoke(non_exists_instance, 10, 20); // Same effect with f2.invoke_static()

    rainy::utility::any result_of_foo1 = f2.invoke_static(10, 20);
    std::cout << "f2 returns: " << result_of_foo1.as<int>() << '\n';
    return 0;
}
/*Output
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
*/
```

## invoke

用于调用当前function对象存有的函数。

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

如果调用目标的返回类型不为void，则此部分返回的any将包含实际的返回值内容。如，目标调用返回`int`类型，数据为`42`。则`any`将包含`int`类型的rtti信息和数据`42`。

### 备注

若当前对象存储的是一个对象实例函数，需要提供与`which_belongs()`返回的类型信息兼容的对象。否则会调用失败。在DEBUG模式下会检查是否为`non_exists_instance`。若是，则会触发契约错误。契约将会在DEBUG模式下触发一个断点。且无法直接捕获。在RELEASE模式不会进行检查。因此，若调用目标是个对象实例函数，则产生未定义行为。如空指针访问成员函数等。

另外，在调用的时候，会检查参数的可转换性，通常会计算参数是否适合且参数是否完全一致。若参数数量不一致，则`errno`会被设置为`EINVAL`，则尝试检查类型兼容性并在可能的情况下尝试不同类型的转换（`可能会导致std::terminate()`的调用）。但是，如果调用所需类型未被用于特化，则`errno`也会被设置为`EACCES`。表示无法找到适当的类型转换。（默认情况下提供对拷贝/移动构造、同类型引用转换、算术类型转换、`std::string_view`转换的支持。详见`any`文档的`any_converter`描述）。

### 示例

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
    using t = decltype(&decltype(lmb)::operator());
    function lambda_without_capture = [](int a, const int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
    }; // bind a non-capture lambda expression
    std::cout << "lambda_without_capture's signature: " << lambda_without_capture << '\n';
    lambda_without_capture.invoke(non_exists_instance,10, 30);
    // invoke lambda_without_capture, lambda_without_capture --> lambda
    auto lambda = [&](int a,int b) {
        std::cout << "This Is Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
        std::cout << f1 << '\n';
    }; // bind a capture lambda expression
    function lambda_with_capture = lambda;
    lambda_with_capture.invoke(lambda, 10, 30);
    std::cout << "lambda_without_capture's signature: " << lambda_with_capture << '\n';
    return 0;
}
/*Output
foo
foo1
change()
print()
10
20
lambda_without_capture's signature: void(__cdecl *)(int,int) noexcept(false) -> 00007FF79F004530
This Is Lambda
a = 10
b = 30
This Is Lambda
a = 10
b = 30
void(__cdecl *)(void) noexcept -> 00007FF79F0019DD
lambda_without_capture's signature: void(__cdecl main::<lambda_2>::* )(int,int) const -> 00007FF79F004420
*/
```

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

## return_type {#return_type}

获取当前function对象指向的返回值类型的rtti类型标识

### 返回值

返回一个类型为`rainy::foundation::rtti::typeinfo`的对象，其中包含该对象对应类型的类型特征表示以及其哈希码和字符串表示名称。

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

以及，`return_type()`返回的rtti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

### 备注

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
/*Output
example::foo ret type => int
rainy_typeid(int) == fn.return_type() ? => true
static_foo ret type => int
rainy_typeid(int) == fn.return_type() ? => true
use_array ret type => class std::vector<__int64,class std::allocator<__int64> >
rainy_typeid(int) == fn.return_type() ? => false
*/
```

## paramlists

获取当前function对象的调用参数的类型标识列表

```cpp
const rainy::collections::views::array_view<
    rainy::foundation::rtti::typeinfo
>& paramlists() const noexcept;
```

### 返回值

返回当前function对象中持有的函数指针对应的参数列表信息

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言。另外，对paramlists进行const_cast修改，将导致未定义行为

以及，`paramlists()`返回的rtti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断


### 示例

```cpp
#include <iostream>
#include <string_view>
#include <vector>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::rtti;
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
/*Output
example::foo(__int64, int, class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >)
example::foo paramlist's details
{       __int64, is_ref: false, is_cv: false
        int, is_ref: false, is_cv: false
        class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >, is_ref: false, is_cv: false
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

test_fnptr(void(__cdecl *)(float))
test_fnptr paramlist's details
{
        void(__cdecl *)(float), is_ref: false, is_cv: false
}
*/
```

## arg_count {#arg_count}

获取调用当前function的所需参数数量

```cpp
std::size_t arg_count() const noexcept;
```

### 返回值

返回当前function对象中持有的函数指针对应的参数列表的参数总数量

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

### 示例

```cpp
#include <iostream>
#include <string_view>
#include <vector>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::rtti;
using namespace rainy::meta::reflection;

using fp = void(*)(float);

struct example {
    // int(example::*)(std::int64_t, int, std::string);
    int foo(std::int64_t, int , std::string) {
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

int main() {
    function fn = &example::foo;
    std::cout << "example::foo 's arg count -> " << fn.arg_count() << '\n';
    fn.rebind(&static_foo);
    std::cout << "static_foo 's arg count -> " << fn.arg_count() << '\n';
    fn.rebind(&use_array);
    std::cout << "use_array 's arg count -> " << fn.arg_count() << '\n';
    fn.rebind(&cv_ref_fn);
    std::cout << "cv_ref_fn 's arg count -> " << fn.arg_count() << '\n';
    fn.rebind(&test_fnptr);
    std::cout << "test_fnptr 's arg count -> " << fn.arg_count() << '\n';
    return 0;
}
/*Output
example::foo 's arg count -> 3
static_foo 's arg count -> 3  
use_array 's arg count -> 1   
cv_ref_fn 's arg count -> 2   
test_fnptr 's arg count -> 1  
*/
```

## arg

获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息

```cpp
const rainy::foundation::rtti::typeinfo &arg(
    std::size_t idx
) const;
```

### 参数

`idx`: 指定的索引

### 返回值

当前function对象存储的参数列表对应的类型信息

### 异常

产生`out_of_range`异常，当`idx >= arg_count()`

### 备注

`arg()`返回的rtti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

### 示例

```cpp
#include <iostream>
#include <string_view>
#include <vector>
#include <rainy/meta/reflection/function.hpp>

using namespace rainy;
using namespace rainy::collections::views;
using namespace rainy::foundation::rtti;
using namespace rainy::meta::reflection;

using fp = void(*)(float);

struct example {
    // int(example::*)(std::int64_t, int, std::string);
    int foo(std::int64_t,int , std::string) {
        return 0;
    }
};

void void_arg_fn() {
}

void int_int_fn(int ,int) {
}

int main() {
    function fn = &example::foo;
    std::cout << "example::foo: \n";
    for (int i = 0; i < fn.arg_count(); ++i) {
        std::cout << fn.arg(i).name() << '\n';
    }
    // fn.arg(4) <-- it will fail
    fn.rebind(&void_arg_fn);
    // fn.arg(0) <-- it will fail;
    fn.rebind(&int_int_fn);
    std::cout << '\n';
    std::cout << fn.arg(0).name() << '\n';
    std::cout << fn.arg(1).name() << '\n';
    return 0;
}
/*Output
example::foo: 
__int64
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >

int
int
*/
```

## function_signature {#function_signature}

获取当前function对象的函数签名的rtti类型标识

```cpp
const rainy::foundation::rtti::typeinfo & function_signature() const noexcept;
```

### 备注

`function_signature()`返回的rtti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

## function::operator bool

检查当前对象是否有效

### 示例
```cpp
#include <rainy/meta/reflection/function.hpp>

void foo() {}

int main() {
    using namespace rainy::meta::reflection;
    function fn = &foo;
    if (fn) {
        std::cout << "fn is valid!\n";
    }
    fn.rebind(nullptr);
    if (!fn) {
        std::cout << "Now, fn is invalid\n";
    }
    fn = &foo;
    if (fn) {
        std::cout << "fn is valid again\n";
    }
    fn.clear();
    if (!fn) {
        std::cout << "Uh, oh. We lost foo again! I shall be quit.\n";
    }
    return 0;
}

/*Output
fn is valid!
Now, fn is invalid
fn is valid again
Uh, oh. We lost foo again! I shall be quit.
*/
```

## function::operator= {#function-assign-operator}

拷贝function对象或修改function对象

### 1. `operator=(const function &)`

拷贝一个function对象存储的指针到当前function对象中

```cpp
function &operator=(const function &right) noexcept;
```

#### 参数
`right`: 待拷贝的function对象
#### 返回值
当前function对象的this引用

### 2. `operator=(function &&)`

将一个function对象存储的指针移动到当前function对象中

```cpp
function &operator=(function &&right) noexcept;
```

#### 参数
`right`: 待移动的function对象
#### 返回值
当前function对象的this引用
 
### 3. `operator=(std::nullptr_t)`

将一个function对象置空

```cpp
function &operator=(std::nullptr_t) noexcept;
```

#### 返回值
当前function对象的this引用

### 备注
- `operator=(const function&)`与`copy_from_other`成员函数具有等效行为
- `operator=(function&&)`与`move_from_other`成员函数具有等效行为
- `operator=(std::nullptr_t)`与`clear`成员函数具有等效行为

### 异常

上述函数均不产生异常（强异常noexcept保证）

### 异常

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

void foo() {}

void foo1() {}

int main() {
    using namespace rainy::meta::reflection;
    function f1 = &foo;
    function f2 = &foo1;
    f1 = std::move(f2); // <-- 移动f2
    if (!f2) {
        std::cout << "Now, f2 does not hold any resources because it was moved or cleared.\n";
    }
    f2 = std::move(f1);
    if (f2) {
        std::cout << "Now, f2 get his resources again.\n";
        if (!f1) {
            std::cout << "Also, f1 is invalid now!\n";
        }
    }
    f1 = f2 = nullptr;
    if (!f1 && !f2) {
        std::cout << "Now, f1 and f2 is empty!\n";
    }
    return 0;
}
/* Output
Now, f2 does not hold any resources because it was moved or cleared.
Now, f2 get his resources again.
Also, f1 is invalid now!
Now, f1 and f2 is empty!
*/
```