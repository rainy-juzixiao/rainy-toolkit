# Hello World样例

## 编写Hello World程序

首先，让我们按照常规流程，编写要反射的函数。并包含需要的头文件。

```cpp
#include <rainy/meta/reflection/registration.hpp>

static void f() { 
	std::cout << "Hello World" << std::endl;
}
```

然后，我们还需要使用`RAINY_REFLECTION_REGSITRATION`进行反射的注册。并在其中，直接使用`registration::method`作为对全局的注册。

```cpp
RAINY_REFLECTION_REGSITRATION {
    registration::method("f", &f);
}
```

这将注册函数`f`并绑定名称`f`。并且如果你在`RAINY_REFLECTION_REGSITRATION`注册，这个宏会确保在你正式进入`main`函数的时候，类型信息是立即可用的。

:::warning
RAINY_REFLECTION_REGSITRATION每个cpp编译单元应当仅定义一次。否则，这将导致编译错误。并且，在`RAINY_REFLECTION_REGSITRATION`进行任何对除注册外的反射API操作，都将可能导致未定义行为的发生。
:::

之后，在使用中，你可以直接使用`type::invoke_global`方法对`f`发起目标调用。

```cpp
type::invoke_global("f");
```

其中，最简形式你只需要指定目标名称即可，如果目标需要参数，请直接传入需要的参数。例如，如果`f`的参数签名为`int(int, char)`，那么调用代码为：

```cpp
type::invoke_global("f", 42, 'c');
```

其中，如果调用无效，则会通过特定方式进行报告，相关部分参见[此处](/md/reflection/function/function_class#invoke)文档。

最后，如果没有问题的情况下，那么，你可以直接看到`Hello World`的输出。

```cpp
#include <rainy/meta/reflection/registration.hpp>

static void f() { 
	std::cout << "Hello World" << std::endl;
}

using namespace rainy::meta::reflection;

RAINY_REFLECTION_REGSITRATION {
    registration::method("f", &f);
}

int main() {
    type::invoke_global("f");
}
```

至此，你已经完成了基本的入门。