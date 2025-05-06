# function 类

## 成员
|成员函数|说明|
|-|-|
|[function](#function)|用于构造function对象|
|[invoke_static](#invoke_static)|向function对象中存储的函数指针，发起调用，以静态域函数方式|
|[invoke](#invoke)|向function对象中存储的函数指针，发起调用|
|[invoke_paramlist](#invoke_paramlist)|类似invoke的调用，但是支持自动根据当前传递的参数的值类别向invoke提供辅助推导信息|
|[invoke_variadic](#invoke_variadic)|对invoke的拓展，支持大于7以上的参数传递|
|[return_type](#return_type)|获取当前function对象指向的返回值类型的rtti类型标识|
|[param_lists](#param_lists)|获取当前function对象的调用参数的类型标识列表|
|[function_signature](#function_signature)|获取当前function对象的函数签名的rtti类型标识|
|[type](#type)|获取当前function对象具体属于哪种调用类型|
|[which_belongs](#which_belongs)|获取当前function对象中，函数指针具体所属的类类型信息|
|[empty](#empty)|检查当前function对象是否为空|
|[is_static](#is_static)|检查当前function对象是否为静态域函数|
|[is_memfn](#is_memfn)|检查当前function对象是否为成员实例函数|
|[is_const](#is_const)|检查当前function对象是否为成员实例函数，且检查是否具有const属性|
|[is_volatile](#is_volatile)|检查当前function对象是否为成员实例函数，且检查是否具有volatile属性|
|[is_invocable](#is_invocable)|检查当前function对象是否可通过其提供的参数进行动态调用|
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
template <typename ReturnType, typename... Args>
function(ReturnType (*function)(Args...));

template <typename Class, typename ReturnType, typename... Args>
function(ReturnType (Class::*function)(Args...));

/* 此处为function为不同修饰符（noexcept、volatile、const）组合提供的重载，此部分省去 */
```

#### 参数

`function`: 可支持构造的函数指针

#### 备注

以下构造函数被默认删除。

```cpp
template <typename LambdaOrFnObj>
function(LambdaOrFnObj) = delete;
```

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
*/
```

## invoke_static {#invoke_static}

向function对象中存储的函数指针，发起调用，以静态域函数方式

```cpp
template <typename... Args>
rainy::utility::any invoke_static(Args &&...args);
```

### 参数

`args...`: 任意数量的参数，用于提供给目标

### 返回值

如果调用目标的返回类型不为void，则此部分返回的any将包含实际的返回值内容。如，目标调用返回`int`类型，数据为`42`。则`any`将包含`int`类型的rtti信息和数据`42`。

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
    f2.invoke(non_exists_instance, 10, 20);

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

```cpp
class function {
public:
    // ===== 构造函数 =====
    function() noexcept;
    function(std::nullptr_t);
    function(function &&right) noexcept;
    function(const function &right) noexcept;

    template <typename ReturnType, typename... Args>
    function(ReturnType (*function)(Args...));

    template <typename Class, typename ReturnType, typename... Args>
    function(ReturnType (Class::*function)(Args...));

    template <typename LambdaOrFnObj,
              type_traits::other_trans::enable_if_t<!type_traits::primary_types::function_traits<LambdaOrFnObj>::valid, int> = 0>
    function(LambdaOrFnObj) = delete;

    /* 此处为function为不同修饰符（noexcept、volatile、const）组合提供的重载，此部分省去 */

    // ===== 赋值操作符 =====
    function &operator=(const function &right) noexcept;
    function &operator=(function &&right) noexcept;
    function &operator=(std::nullptr_t) noexcept;

    // ===== 调用接口 =====
    template <typename... Args>
    utility::any invoke_static(Args &&...args) const;

    utility::any invoke(object_view object) const;
    utility::any invoke(object_view object, utility::any ax1) const;
    utility::any invoke(object_view object, utility::any ax1, utility::any ax2) const;
    utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3) const;
    utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4) const;
    utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                        utility::any ax5) const;
    utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                        utility::any ax5, utility::any ax6) const;
    utility::any invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                        utility::any ax5, utility::any ax6, utility::any ax7) const;

    template <typename... Args>
    utility::any invoke_paramlist(object_view instance, Args &&...args) const;

    utility::any invoke_variadic(object_view instance, rainy::collections::views::array_view<utility::any> any_args) const;

    template <typename... Args>
    utility::any operator()(object_view instance, Args &&...args) const;

    // ===== 元信息和状态 =====
    const foundation::rtti::typeinfo &return_type() const noexcept;
    rainy::collections::views::array_view<foundation::rtti::typeinfo> param_lists() const noexcept;
    const foundation::rtti::typeinfo &function_signature() const noexcept;
    method_type type() const noexcept;
    const foundation::rtti::typeinfo &which_belongs() const noexcept;

    bool empty() const noexcept;
    explicit operator bool() const noexcept;

    // ===== 特性判断 =====
    bool is_static() const noexcept;
    bool is_const() const noexcept;
    bool is_volatile() const noexcept;
    bool is_invocable(rainy::collections::views::array_view<utility::any> paramlist) const noexcept;

    // ===== 管理函数 =====
    void copy_from_other(const function &right) noexcept;
    void move_from_other(function &&right) noexcept;
    void swap(function &right) noexcept;
    void clear() noexcept;
    void rebind(function &&function) noexcept;
    void rebind(const function &function) noexcept;

    // ===== 获取目标函数指针 =====
    template <typename Fx>
    Fx target() const noexcept;

    // ===== 比较函数 =====
    bool equal_with(const function &right) const noexcept;
    bool not_equal_with(const function &right) const noexcept;

    friend bool operator==(const function &left, const function &right) noexcept;
    friend bool operator!=(const function &left, const function &right) noexcept;
};

```