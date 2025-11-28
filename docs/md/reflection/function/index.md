# function.hpp

此头文件提供对*静态域函数、对象实例函数、仿函数*三类的反射信息支持，包括可对函数返回值类型、参数列表以及可调用功能就行检查，可随时用指定类型取出其存储的函数指针或重新绑定函数指针，支持`const`、`volatile`、`noexcept`修饰并提供其检查能力。且允许用户检查function对象的可调用性，以及允许通过引用这它们函数的*函数指针*并进行动态调用（类型安全难保证，若需转换，可能会出现异常，使用rainy::utility::any作为参数传递）。

具体示例请参见function类页面。

## 相关术语

### 静态域函数

在C++标准中术语中，静态域函数通常是指普通函数或 static 成员函数。他们可能会改变状态，也可能不改变，但是引用他们始终是静态的。且不具备指向对象的this指针。即无绑定上下文的可调用实体。

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
```

### 对象实例函数

对象实例函数是指，如同下列的非静态成员函数。是绑定了对象上下文的成员可调用实体。

```cpp
class examples {
    int fun(char a, std::string b); // <-- 可适用，签名为 int(examples::*)(char,std::string)
    int fun1(char a, std::string b, std::vector<int>) noexcept; // <-- 可适用，签名为int(examples::*)(char,std::string,std::vector<int>)

    static void no(); // 不适用，no是个平凡函数
};
```

:::tip
对象实例函数与成员函数不是同一概念
:::

### 仿函数

本库对于仿函数的支持，有需注意的事项。仿函数的反射极为特殊。通常，对于仿函数的处理会检查传入的函数对象类型。进行类型推导。在推导后，有可能是*静态域函数*也可能是*对象实例函数*。

```cpp
auto lmb = [/* 带捕获 */] {}; // <-- 可引用，作为仿函数存在

auto lmb_as_ptr = [/* 无捕获 */]() {}; // <-- 可引用，作为仿函数存在

struct plus {
    int operator()(int a,int b) {
        return a + b;
    }
};

plus plus_object; // <-- 可引用，要求所属类可复制构造以用于构造对象

struct plus_not_empty {
    int operator()(int a, int b) {
        sum += a + b;
        return sum;
    }

    int sum;
};

plus_not_empty plus_not_empty_object; // <-- 可引用，要求所属类可复制构造以用于构造对象，将具有独立的状态

struct not_good {
    int operator(int a, int b) {
        return a + b;
    }

    void operator(int a, int b, int c) {
        std::cout << "a + b + c = " <<a + b + c<<"\n";
    }
};
not_good not_good_object; // <-- 不可引用，因为无法直接解析重载决议，受限制
```

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
|function 类|提供对*静态域函数、对象实例函数、仿函数*三类的反射信息。支持动态调用、动态检索和重绑定|

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