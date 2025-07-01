# function 类

## 概要

function类是类似于标准库的function类，但是存在差异。例如，该类是非模板类。function类会根据函数类型自动生成调用代码并进行绑定。在使用中，function类通常被用于采集类型识别信息、动态的调用某些可调用函数和序列化某些信息。另注，详细机制请参见内部机制页面。

该类具有如下特性。

- 支持默认参数绑定
- 支持lambda、函数对象、成员函数指针、函数指针的绑定
- 动态调用
- 重绑定不同签名函数
- 动态检索目标属性
- 支持检查可调用性
- 可有限的对参数进行转换
- 无rtti

另外，具有如下缺点。

- 存在类型擦除的性能开销
- 不支持bind表达式（无法直接解析）
- 二进制膨胀
- 不具备完整的类型安全机制

上述缺点中，例如function类内部会实例化内部模板代码用于解析或检索等操作，因此会导致二进制膨胀。另外，由于模板实例化的特性，类型擦除被大量应用。以及，bind表达式因为具有多个重载导致无法直接解析。另外，类型安全是通过内建规则实现的。因此，如果需要完整的、原生的C++类型安全机制，而非动态调用。可考虑使用rainy-toolkit提供的delegate模板类，而非function类。例如，delegate类具有如下特性。

- 类型安全
- 优化潜力较高（编译时信息已知）
- 检索目标属性
- 无rtti
- 重绑定同签名函数

因此，如果希望使用类型安全和检索类型信息而不希望使用动态调用，使用delegate类是相对较好的选择。但是也同样存在如下缺点。

- 无法动态调用
- 统一存储难
- 灵活性相比function类较低
- 某些函数类型需显式实例化
- 不支持默认参数调用

### 成员

function类定义了如下成员

|成员函数|说明|
|-|-|
|[function](#function)|用于构造function对象|
|[static_invoke](#static_invoke)|向function对象中存储的函数实例，发起调用，以对象上下文函数或静态域函数方式|
|[invoke](#invoke)|向function对象中存储的函数实例，发起调用，以对象上下文函数或静态域又或对象实例函数方式|
|[return_type](#return_type)|获取当前function对象指向的返回值类型的ctti类型标识|
|[paramlists](#paramlists)|获取当前function对象的调用参数的类型标识列表|
|[arity](#arity)|获取调用当前function的所需参数数量|
|[arg](#arg)|获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息|
|[function_signature](#function_signature)|获取当前function对象的函数签名的ctti类型标识|
|[type](#type)|获取当前function对象的调用类型|
|[has](#has)|用于检索当前function对象是否具有特定的属性|
|[which_belongs](#which_belongs)|获取当前function对象中，函数指针具体所属的类类型信息|
|[empty](#empty)|检查当前function对象是否为空|
|[is_static](#is_static)|检查当前function对象是否为静态域函数|
|[is_memfn](#is_memfn)|检查当前function对象是否为成员实例函数|
|[is_const](#is_const)|检查当前function对象是否为成员实例函数，且检查是否具有const属性|
|[is_volatile](#is_volatile)|检查当前function对象是否为成员实例函数，且检查是否具有volatile属性|
|[is_invocable](#is_invocable)|检查当前function对象是否可通过其提供的参数类型进行动态调用|
|[is_invocable_with](#is_invocable_with)|检查当前function对象是否可以通过其提供的参数进行动态调用|
|[is_noexcept](#is_noexcept)|检查当前function对象是否具有noexcept属性|
|[is_invoke_for_lvalue](#is_invoke_for_lvalue)|检查当前function对象是否是适用于左值引用的调用|
|[is_invoke_for_rvalue](#is_invoke_for_rvalue)|检查当前function对象是否是适用于右值引用的调用|
|[copy_from_other](#copy_from_other)|拷贝一个function对象存储的函数指针到此function对象中|
|[move_from_other](#move_from_other)|将function对象存储的函数指针转移到此function对象中|
|[swap](#swap)|将function对象的函数指针与当前function对象的函数指针进行交换|
|[clear](#clear)|将function对象进行置空|
|[rebind](#rebind)|为function对象重新绑定一个函数实例|
|[target](#target)|安全提取function对象中存储的函数指针|
|[equal_with](#equal_with)|检查一个function对象与当前的function对象是否一致|
|[not_equal_with](#not_equal_with)|检查一个function对象与当前的function对象是否为不同的|

|运算符|说明|
|-|-|
|[function::operator()](#function-invoke-operator)|提供仿函数语义调用invoke函数|
|[function::operator bool](#function-operator-bool)|检查当前对象是否有效|
|[function::operator=](#function-assign-operator)|拷贝function对象或修改function对象|

### 备注

function类对象是可重入的，不具备线程安全机制。因此，若目标代码涉及数据竞争条件，请手动编写同步代码。或使用pal模块中`threading.hpp`的`create_synchronized_task`简化同步代码。

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

### 3. `function(Fx,Args...)`

通过传入可调用实体和可选的默认参数来构造function对象

```cpp
template <typename Fx>
function(Fx function, Args&&... default_arguments) noexcept;
```

#### 参数

`function`: 可支持构造的可调用函数，但是必须是*静态域函数、对象实例函数、仿函数*这类的。对于仿函数，仅支持无重载版本的`operator()`：即所属类中有且仅有一个，因此，bind表达式这类无法使用

`default_arguments...`：这部分是可选的，表示默认的追加参数。对于默认参数，function对象会在内部将参数类型处理为值。会将引用性、const/volatile属性全部抹除。另外，数组直接退化为指针。以及，参数是必须能转换到对应参数的。

#### 备注

对于仿函数，该构造函数将会进行类型推导，一般来说，若传入lambda表达式、函数对象。情况会稍稍不同，例如，带捕获的lambda表达式可能会触发内存分配操作来用于存储当前函数对象。不过，一般情况下，构造函数会尽可能使用function对象为其保留的一段SOO内存空间，这一内存空间大小是编译期决定的，通常大小为：`(rainy::core::small_object_num_ptrs * sizeof(void *)) + alignof(std::max_align_t)`。具体的值由编译平台决定。

另外，如果有默认参数的传入，则分配大小也会受到影响。（可能触发分配）

详细细节，请查阅本页中的内存策略页。

#### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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
    lambda_without_capture.static_invoke(10, 30);
    // invoke lambda_without_capture, lambda_without_capture --> lambda
    auto lambda = [&](int a, int b) {
        std::cout << "This Is Capture Lambda\n";
        std::cout << "a = " << a << '\n';
        std::cout << "b = " << b << '\n';
        std::cout << f1 << '\n';
    }; // bind a capture lambda expression
    function lambda_with_capture = lambda;
    lambda_with_capture.static_invoke(10, 30);
    // invoke lambda_with_capture, lambda_with_capture --> lambda
    std::cout << "lambda_with_capture's signature: " << lambda_with_capture << '\n';
    function use_defaults = {[](int a, int b) {
        std::cout<< "a + b = " << (a + b) << "\n";
    }, 90};
    use_defaults.static_invoke(10, 20); // invoke use_defaults, use_defaults --> lambda
    use_defaults.static_invoke(10); // invoke use_defaults with default value for b, use_defaults --> lambda
    return 0;
}
```

#### 输出

若编译成功，此程序会产生类似如下的输出

```text
foo
foo
foo1
change()
print()
10
20
lambda_without_capture's signature: void(__cdecl main::<lambda_1>::* )(int,const int) const -> 0000003175CFF3C0
This Is Lambda
a = 10
b = 30
This Is Capture Lambda
a = 10
b = 30
void(__cdecl *)(void) noexcept -> 0000003175CFF160
lambda_with_capture's signature: void(__cdecl main::<lambda_2>::* )(int,int) const -> 0000003175CFF440
a + b = 30
a + b = 100
```

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

### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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

#### 输出

若编译成功，此程序会产生类似如下的输出

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

#### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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

#### 输出

若编译成功，此程序会产生类似如下的输出

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

## return_type {#return_type}

获取当前function对象指向的返回值类型的ctti类型标识

### 返回值

返回一个类型为`rainy::foundation::ctti::typeinfo`的对象，其中包含该对象对应类型的类型特征表示以及其哈希码和字符串表示名称。

### 备注

在调用时，无论是Debug还是Release模式。都会默认调用一个契约函数`expects`以检查`empty()`返回结果是否为假，若为假，则代表当前对象无效，将触发断言

以及，`return_type()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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

#### 输出

若编译成功，此程序会产生类似如下的输出

```text
example::foo ret type => int
rainy_typeid(int) == fn.return_type() ? => true
static_foo ret type => int
rainy_typeid(int) == fn.return_type() ? => true
use_array ret type => class std::vector<__int64,class std::allocator<__int64> >
rainy_typeid(int) == fn.return_type() ? => false
```

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


### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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

#### 输出

若编译成功，此程序会产生类似如下的输出

```text
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
```

## arity

获取调用当前function的所需参数数量

```cpp
std::size_t arity() const noexcept;
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
using namespace rainy::foundation::ctti;
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
    std::cout << "example::foo 's arity -> " << fn.arity() << '\n';
    fn.rebind(&static_foo);
    std::cout << "static_foo 's arity -> " << fn.arity() << '\n';
    fn.rebind(&use_array);
    std::cout << "use_array 's arity -> " << fn.arity() << '\n';
    fn.rebind(&cv_ref_fn);
    std::cout << "cv_ref_fn 's arity -> " << fn.arity() << '\n';
    fn.rebind(&test_fnptr);
    std::cout << "test_fnptr 's arity -> " << fn.arity() << '\n';
    return 0;
}
```

#### 示例

若编译成功，此程序会产生类似如下的输出

```text
example::foo 's arity -> 3
static_foo 's arity -> 3  
use_array 's arity -> 1   
cv_ref_fn 's arity -> 2   
test_fnptr 's arity -> 1  
```

## arg

获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息

```cpp
const rainy::foundation::ctti::typeinfo &arg(
    std::size_t idx
) const;
```

### 参数

`idx`: 指定的索引

### 返回值

当前function对象存储的参数列表对应的类型信息

### 异常

产生`out_of_range`异常，当`idx >= arity()`

### 备注

`arg()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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

void void_arg_fn() {
}

void int_int_fn(int ,int) {
}

int main() {
    function fn = &example::foo;
    std::cout << "example::foo: \n";
    for (int i = 0; i < fn.arity(); ++i) {
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
```

#### 输出

若编译成功，此程序会产生类似如下的输出

```text
example::foo:
__int64
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
int_int_fn:
int
int
```

## function_signature {#function_signature}

获取当前function对象的函数签名的ctti类型标识

```cpp
const rainy::foundation::ctti::typeinfo & function_signature() const noexcept;
```

### 备注

`function_signature()`返回的ctti类型名称在不同编译器中不具备相同名称。因此，通过硬编码字符串确定名称并不是最佳的实践，建议通过`hash_code()`、`is_compatible`等方法进行类型的比对判断

另外，对于函数对象，即具有`operator()`的，将会返回`operator()`的参数签名而非它对象本身

### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

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
    // It will output a signature from decltype(&decltype(no_capture)::operator())
    const auto &no_capture_type = rainy_typeid(decltype(&no_capture));
    std::cout << "Is fn.function_signature equals with no_capture_type ? " << (fn.function_signature() == no_capture_type) << '\n';
    const auto &no_capture_fx_type = rainy_typeid(decltype(&decltype(no_capture)::operator()));
    std::cout << "Is fn.function_signature equals with no_capture_fx_type ? " << (fn.function_signature() == no_capture_fx_type)
              << '\n';
    int unused{};
    auto capture = [i = std::size_t{0}, unused](std::string_view) mutable noexcept -> int { return 0; };
    fn.rebind(capture);
    std::cout << fn.function_signature().name() << '\n';
    // It will output a signature from decltype(&decltype(capture)::operator())
    const auto &capture_type = rainy_typeid(decltype(&capture));
    std::cout << "Is fn.function_signature equals with no_capture_type ? " << (fn.function_signature() == capture_type) << '\n';
    const auto &capture_fx_type = rainy_typeid(decltype(&decltype(capture)::operator()));
    std::cout << "Is fn.function_signature equals with no_capture_fx_type ? " << (fn.function_signature() == capture_fx_type) << '\n';
    return 0;
}
```

#### 输出

若编译成功，此程序会产生类似如下的输出

```text
int(__cdecl example::* )(__int64,int,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >)
Is fn.function_signature equals with example_foo_type ? true
void(__cdecl *)(void)
Is fn.function_signature equals with void_arg_type ? true
void(__cdecl *)(int,int)
Is fn.function_signature equals with int_int_fn_type ? true
int(__cdecl main::<lambda_1>::* )(class std::basic_string_view<char,struct std::char_traits<char> >) noexcept
Is fn.function_signature equals with no_capture_type ? false
Is fn.function_signature equals with no_capture_fx_type ? true
int(__cdecl main::<lambda_2>::* )(class std::basic_string_view<char,struct std::char_traits<char> >) noexcept
Is fn.function_signature equals with no_capture_type ? false
Is fn.function_signature equals with no_capture_fx_type ? true
```

## type

获取当前function对象的调用类型。不过，该接口一般是用于拓展生成序列化代码。一般用户不建议使用。而是应当使用[has](#has)接口

```cpp
rainy::meta::reflection::method_flags type() const noexcept;
```

### 返回值

其返回值是以下的一个枚举位组合成的一个值，用于检索其属性。

```cpp
enum class method_flags : std::uint8_t {
    none = 0,
    static_qualified = 1, // static method
    memfn_specified = 2, // member method
    noexcept_specified = 4, // noexcept
    lvalue_qualified = 8, // left qualifier (e.g. &)
    rvalue_qualified = 16, // right qualifier (e.g. &&)
    const_qualified = 32, // const
    volatile_qualified = 64, // volatile
};
```

### 示例

请确保此代码编译时最低标准被配置为C++17，否则无法通过编译。若出现运行时错误，请记录调用堆栈、当前上下文信息并提交issues。

```cpp
#include <iostream>
#include <rainy/meta/reflection/function.hpp>

class myfoo {
public:
    void foo() noexcept {
    }

    void foo_const() const noexcept {
    }

    void foo_volatile() volatile noexcept {
    }
};

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
    if (static_cast<bool>(f1.type() & method_flags::noexcept_specified) || static_cast<bool>(f1.type() & method_flags::static_specified)) {
        std::cout << "f1 is noexcept or static\n";
    } else {
        std::cout << "f1 is not noexcept or static\n";
    }
    function f2 = &foo1;
   if (static_cast<bool>(f1.type() & method_flags::static_specified)) {
        std::cout << "f2 is static\n";
    } else {
        std::cout << "f2 is not static\n";
    }
    function f3 = {[](int a, int b) { std::cout << "f3: a + b = " << (a + b) << '\n'; }};
    if (static_cast<bool>(f3.type() & method_flags::static_specified)) {
        std::cout << "f3 is static\n";
    } else {
        std::cout << "f3 is not static\n";
    }
    function f4 = &myfoo::foo;
    if (static_cast<bool>(f4.type() & method_flags::noexcept_specified)) {
        std::cout << "f4 is noexcept\n";
    } else {
        std::cout << "f4 is not noexcept\n";
    }
    f4.rebind(&myfoo::foo_const);
    if (static_cast<bool>(f4.type() & method_flags::const_qualified)) {
        std::cout << "f4 is const\n";
    } else {
        std::cout << "f4 is not const\n";
    }
    f4.rebind(&myfoo::foo_volatile);
    if (static_cast<bool>(f4.type() & method_flags::volatile_qualified)) {
        std::cout << "f4 is volatile\n";
    } else {
        std::cout << "f4 is not volatile\n";
    }
    return 0;
}
```

#### 输出

若编译成功，此程序会产生类似如下的输出

```text
f1 is noexcept or static
f2 is static
f3 is static
f4 is noexcept
f4 is const
f4 is volatile
```

## has

用于检索当前function对象是否具有特定的属性，封装自[type](#type)接口。

```cpp
bool has(rainy::meta::reflection::method_flags flag) const noexcept;
```

### 参数

`flag`：取自`rainy::meta::reflection`的枚举类：
```cpp
enum class method_flags : std::uint8_t {
    none = 0,
    static_qualified = 1, // static method
    memfn_specified = 2, // member method
    noexcept_specified = 4, // noexcept
    lvalue_qualified = 8, // left qualifier (e.g. &)
    rvalue_qualified = 16, // right qualifier (e.g. &&)
    const_qualified = 32, // const
    volatile_qualified = 64, // volatile
};
```

关于method_flags的说明，请参见[method_flags枚举](/md/reflection/function/method_flags)页面。

## which_belongs {#which_belongs}

## empty

## is_static {#is_static}

## is_memfn {#is_memfn}

## is_const {#is_const}

## is_volatile {#is_volatile}

## is_invocable {#is_invocable}

## is_invocable_with {#is_invocable_with}

## is_noexcept {#is_noexcept}

## is_invoke_for_lvalue	{#is_invoke_for_lvalue}

## is_invoke_for_rvalue	{#is_invoke_for_rvalue}

## copy_from_other {#copy_from_other}

## move_from_other {#move_from_other}

## swap

## clear

## rebind

## target

## equal_with {#equal_with}

## not_equal_with {#not_equal_with}

## function::operator() {#function-invoke-operator}

该重载是

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

### 示例

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

## 内存策略

:::warning
此部分可能涉及抽象泄露，因此，此节被置于末节。用于描述function类的内存策略
:::

function类的内部采用了SOO内存空间与动态内存分配共存的内存策略。且，大量细节被封装。因此为了屏蔽不必要的抽象泄露，你无法直接操作function对象的内存，除非你在进行未定义行为或寻找漏洞。并且，你没有权限得知是否使用SOO或者动态分配的内存。这一行为，我们不推荐，且如果可能的情况下，永远不要做这种蠢事。（哪怕你可以通过预定义常量在编译时计算是否可能使用动态分配内存）

但是，本节会介绍function类的内存策略机制来帮助理解function何时使用SOO、何时使用动态分配内存。

function类的内存布局如下。

```cpp
alignas(std::max_align_t) rainy::core::byte_t invoker_storage[
    rainy::core::fn_obj_soo_buffer_size
]{};
invoker_accessor *invoke_accessor_{nullptr};
```

`invoker_storage`是function对象保留的SOO空间，而`invoke_accessor_`则为访问器。在构造function对象时，如果目标存储的所需大小是小于SOO空间大小的，则会将目标构造于`invoker_storage`以加快性能。其中，SOO空间大小被定义如下：

```cpp
static inline constexpr std::size_t small_object_num_ptrs = 4 + 16 / sizeof(void *);

static constexpr inline std::size_t fn_obj_soo_buffer_size = (core::small_object_num_ptrs * sizeof(void *)) + alignof(std::max_align_t);
```

若所需大小是大于SOO空间大小的，则会触发动态分配来容纳。另外，function类会自动管理内存。如果function对象持有的是动态分配的内存。则会在对象析构时自动释放。

另注，SOO空间是具体的值根据平台决定的。因此，请时刻参考编译器给出的实际大小，本文不进行展示。

关于对象大小，一般有如下因素构成。

- 函数目标
- 默认参数存储
- 内存对齐

因此，一个具有大对象且捕获了变量或常量的lambda表达式将容易导致function对象执行动态分配。例如

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
function f = {[vec](int iter, std::string mark) {
    std::cout << mark << " : ";
    for (int i = 0; i < iter; ++i)  {
        std::cout << vec[i] << ' ';
    }
    std::cout << '\n';
}, 6, std::string{"my_vector"}};
```

上述的定义中，f将会通过动态分配的内存构造对象。因为，接受的参数中拷贝了vec，且在默认参数中有int和std::string的传入。因此，总大小将会超过SOO大小。因此，若对象被频繁构造，尽可能使用引用捕获。在默认参数方面，若非必须，则应避免绑定过多默认参数，以防止突破SOO阈值。