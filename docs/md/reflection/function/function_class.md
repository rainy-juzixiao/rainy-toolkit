# function 类

<!--@include: ./function_class/abstract.md-->
<!--@include: ./function_class/members.md-->
<!--@include: ./function_class/ctor.md-->
<!--@include: ./function_class/static_invoke.md--> 
<!--@include: ./function_class/invoke.md-->
<!--@include: ./function_class/return_type.md-->
<!--@include: ./function_class/paramlist.md-->
<!--@include: ./function_class/arity.md-->
<!--@include: ./function_class/arg.md-->
<!--@include: ./function_class/function_signature.md-->
<!--@include: ./function_class/type.md-->
<!--@include: ./function_class/has.md-->
<!--@include: ./function_class/which_belongs.md-->
<!--@include: ./function_class/empty.md-->
<!--@include: ./function_class/is_xxx.md-->
<!--@include: ./function_class/copy_or_move_from_other.md-->
<!--@include: ./function_class/swap.md -->
<!--@include: ./function_class/reset.md -->
<!--@include: ./function_class/rebind.md -->
<!--@include: ./function_class/target.md -->

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

<!-- @include: ./function_class/mem_strategy.md -->