# function.hpp

此头文件提供对*静态域函数、对象实例函数*两类的反射信息支持，包括可对函数返回值类型、参数列表以及可调用功能就行检查，可随时用指定类型取出其存储的函数指针或重新绑定函数指针，支持`const`、`volatile`、`noexcept`修饰并提供其检查。且允许用户通过引用这两类函数的*函数指针*并进行动态调用（类型安全难保证，使用rainy::utility::any作为参数传递）。

## 相关术语

### 静态域函数

静态域函数是指如同下列的静态成员函数或任意的已实例化的模板函数以及常规函数。他们可能会改变状态，也可能不改变，但是引用他们始终是静态的。且不具备指向对象的this指针。即无绑定上下文的可调用实体。

```cpp
class examples {
    static int fun(char a, std::string b); // <-- 可适用，函数签名int(char, std::string)
};

template <typename T>
T fn(T, T); // <-- 不可引用，未实例化

template <>
int fn(int, int) { // <-- 可引用，函数签名int(int,int)
    return 0;
}

int foo(); // <-- 可引用，函数签名int()

auto lmb = [] {}; // <-- 不可引用，因为是函数对象

auto lmb_as_ptr = +[]() {}; // <-- 可引用，可隐式转换为函数指针，但并不好

struct plus {
    int operator()(int a,int b) {
        return a + b;
    }
}

plus plus_object; // <-- 不可引用，因为是函数对象
```

### 对象实例函数

对象实例函数是指，如同下列的非静态成员函数。是绑定了对象上下文的成员可调用实体。

```cpp
class examples {
    int fun(char a, std::string b); // <-- 可适用，签名为 int(examples::*)(char,std::string)
    int fun1(char a, std::string b, std::vector<int>) noexcept; // <-- 可适用，签名为int(char,std::string,std::vector<int>)

    static no(); // 不可适用，no是个平凡函数
}
```

:::tip
对象实例函数与成员函数不是同一概念
:::

## 使用

### 头文件导入指令

```cpp
#include <rainy/meta/reflection/function.hpp>
```

### 受影响的namespace

```cpp
namespace rainy::meta::reflection;
namespace rainy::meta::reflection::implements;
```

## 成员

### 公共类

|名称|描述|
|-|-|
|function 类|提供对*静态域函数、对象实例函数*两类的反射信息。支持动态调用、动态检索和重绑定|

### 公共函数

|名称|描述|
|-|-|
|[make_function](/md/reflection/function/pubfn#make_function)|工厂函数、通过函数指针创建function对象|
|[invoke](/md/reflection/function/pubfn#invoke)|类似于`std::invoke/rainy::utility::invoke`，支持直接调用function对象|

### 运算符

|名称|描述|
|-|-|
|function::operator==|检查两个function对象是否一致|
|function::operator!=|检查两个function对象是否不一致|
|function::operator<<|用于std::ostream重载，输出函数签名以及调用地址，方便日志调试|