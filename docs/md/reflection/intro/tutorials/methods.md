# Hello World样例

## 注册全局方法

前文中的Hello World样例已经提供了一个基础入门，此节我们将了解`rainy-toolkit`在方法上面的细节部分。

在此之前，复习全局方法的注册流程。为了将全局方法注册为RTTR，可以使用函数`registration::method()`进行注册。其用法如下：

<DeclarationTable 
    :content="
  `
(1) (since C++17)
template <typename Fx>
static rain_fn method(std::string_view name, Fx &&fn) -> bind<reflection::method, void, Fx>;
  `
"
/>

其中

- `name`是函数的名称
- `Fx`是一个函数，它不能是对象实例函数，只能是仿函数或静态域函数。如函数指针，lambda表达式，std::function

例如，当你想注册字节串转整数的函数时：`int atoi (const char * str);`