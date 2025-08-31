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

### 3. `function(Fx, Args...)`

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

<!--@include: @/static/example_code_header_cxx17.md-->

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

<!--@include: @/static/example_output_header_cxx17.md-->

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