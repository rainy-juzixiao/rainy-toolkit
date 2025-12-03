# rainy-toolkit 反射

## 导言

rainy-toolkit提供了一套反射机制用于进行反射性编程。这是一套混合的反射体系，即动态和静态均被提供的模式。

在使用方式上，基本上有大部分的API，是类似于RTTR的API。

但是其不同在于，在具体使用中，会存在一些细微的差异。例如部分API被进行了重命名，并且添加了一部分新的接口。

以下是一些部分的细节差异，不代表全部。

例如对于method类中，RTTR在获取类型的时候，是通过`get_return_type()`方法获取的，但是在`rainy-toolkit`中直接使用`return_type()`返回。并且不会返回`type`类，而是直接的rainy-toolkit中foundation模块的`ctti`类型信息对象。这通常考虑到组件的低耦合性设计，因此，时刻注意rainy-toolkit反射API中的细节部分。

以及，对于method类中的invoke方法，invoke直接废弃了下面的这类RTTR的API。

```cpp
variant invoke(instance object);
variant invoke(instance object, argument arg1) const;
...
variant invoke(
	instance object,
	argument arg1,
	argument arg2,
	argument arg3,
	argument arg4,
	argument arg5,
	argument arg6
) const;
```

并替换为：

```cpp
template <typename... Args>
any invoke(
    object_view instance,
    Args &&...args
) const;
```

`invoke_varadic`则保持原样，作为动态API的使用或作为迁移的临时解决方案。并且，引入了回退的转换参数尝试机制。默认情况下，提供了`std::basic_string`、`std::basic_string_view`以及部分算术类型的有限转换支持。如有需要，则按照约定，对要转换的目标类型提供转换器或是直接通过动态API，注入参数转换逻辑。

除此之外，注册方式上rainy-toolkit很类似于RTTR，但是其宏名为`RAINY_REFLECTION_REGISTRATION`。而不是`RTTR_REGISTRATION`。并且RTTR的`variant`和rainy-toolkit的`any`是不同的，并且也有一部分RTTR的API在`rainy-toolkit`中是不存在的。

另外，对于基础部分，rainy-toolkit进行了一些水平的界定以帮助评估rainy-toolkit的反射库是否应当使用。rainy-toolkit通常会使用一小部分专用化的术语对一些函数的情况下进行界定。当然这是理论层面，在实践层面中，您无需评估。

例如，在理论层面中，我们对函数反射仅界定了三个术语用于区分其情形。

- 仿函数
- 对象实例函数
- 静态域函数

另外，rainy-toolkit不会像某些库中去使用复杂的术语体系，对于函数，我们是仅界定了三个术语作为补充。而我们在类型中，仅会使用如下术语去描述反射：

- moon —— 静态反射
- lunar —— 动态反射
- 方法属性元
- 字段属性元
- 类型元
- shared_object/object_view 具有引用计数的生命周期动态对象/对象视图
- any 指代rainy-toolkit的动态容器，作为shared_object的底层容器

某些库中使用的类似域的概念，也就是类似如下的术语，这类术语对于rainy-toolkit来说，并非最佳实践，对于使用者来说，也不应该强制要求使用者理解术语：

- 域：类型的成员对象
- 基本域 `FieldFlag::Basic` ：类型的非静态成员对象
- 虚拟域 `FieldFlag::Virtual`：含虚基类的类型的非静态成员对象或类型的引用成员对象
- 静态域 `FieldFlag::Static`：类型的静态成员对象
- 动态共享域 `FieldFlag::DynamicShared`：类型的动态成员对象，可共享
- 动态缓冲域 `FieldFlag::DynamicBuffer`：类型的动态成员对象，非共享，只能存一些简单类型
- 自有域 `FieldFlag::Owned`：基础域，虚拟域
- 无主域 `FieldFlag::Unowned`：静态域，动态共享域，动态缓冲域

rainy-toolkit仅关注类型的联系，并且尽可能基于标准术语进行拓展补充。并且，尽可能避免不必要的术语堆砌，也同样，lunar和moon均不具备access_level抽象。因为access_leval使得抽象变得更加复杂。

## 快速上手

### 注册

在本例中，我们将演示基于对类中属性、方法以及构造器的使用。众所周知，目前C++无法做到完美的解析式的反射注册。因为反射本身就不是语言原生特性。因此，为了实现反射，你必须像其它库一样，执行注册操作。在注册之前，请确保你已经包含了注册头文件，注册头文件包含了你所需的一切接口。

```cpp
#include <rainy/meta/reflection/registration.hpp>
```

随后，对你需要反射的类进行注册。在本文中，以`MyStruct`为例，以下是`MyStruct`的代码。

```cpp
struct MyStruct {
    MyStruct() {};
    void func(double) {};
    int data;
};
```

随后，你应当使用`RAINY_REFLECTION_REGISTRATION`进行注册操作。

```cpp
RAINY_REFLECTION_REGISTRATION {
	using namespace rainy::meta::reflection;
	registration::class_<MyStruct>("MyStruct")
		.constructor<>()
		.property("data", &MyStruct::data)
		.method("func", &MyStruct::func);
}
```

这将会注册一个默认构造函数，一个属性字段外加一个方法。

我们建议，在每个编译单元的末尾再使用`RAINY_REFLECTION_REGISTRATION`，这样会便于管理。当然，如果您有自己的团队规范，请根据团队规范执行更佳。
### 遍历成员

接着，如果想使用反射系统，在注册之后，你应该通过`type`去访问该接口。我们提供了三种方式供你访问。分别是：

- 字符串类型名访问
- 类型模板参数访问
- CTTI类型信息访问

以下的代码演示了三种方式的获取反射对象接口。

```cpp
using namespace rainy::meta::reflection;
{
	type mystruct_type = type::get_by_name("MyStruct"); // 以字符串类型访问
}
{
	type mystruct_type = type::get<MyStruct>(); // 以类型模板参数访问
}
{
	rainy::foundation::ctti::typeinfo info = fun(); // 从某处获取的ctti信息
	type mystruct_type = get_by_typeinfo(info); // 以CTTI类型信息访问
}
```

在获取之后，调用`is_valid()`即可验证当前类型是否有进行注册。

```cpp
if (mystruct_type.is_valid()) {
	// 在此编写你要操作的代码
}
```

在此，你可以遍历你已经注册的成员。

```cpp
type t = type::get<MyStruct>();
for (auto &prop: t.get_properties()) {
    std::cout << "name: " << prop.get_name() << std::endl;
}
for (auto &meth: t.get_methods()) {
    std::cout << "name: " << meth.get_name() << std::endl;        
}
```

>注意，与RTTR不同。rainy-toolkit倾向于使用哈希表作为存储容器。这方面是因为哈希表提供了更快的查找性能。而`get_properties()`/`get_methods()`的遍历接口则返回的是一个迭代器范围，来模拟这种遍历。因此，如果你对遍历性能极度敏感，则请在此停住！

### 生成对象

同样的，在RTTR中，你可以构造一个非具体类型的对象。同样的，在rainy-toolkit中，也使用类似的形式进行对象的生成。你只需要把`variant`替换为`shared_object`即可。

```cpp
shared_object var = t.create();
constructor ctor = t.get_constructor();
var = ctor.invoke();
std::cout << var.type().name(); // 打印 'MyStruct'
```

>另注：
>shared_object更像是一种类似于java引用类型的对象。它共享了所有权，其生命周期由引用计数决定。如果生命周期结束，则shared_object持有的对象将不可用。

## 获取/修改字段信息

同样的，你也可以获取字段信息，你可以直接以类RTTR的接口进行编写。

```cpp
MyStruct obj;

property prop = type::get<MyStruct>().get_property("data");
prop.set_value(obj, 23);

auto var_prop = prop.get_value(obj);
std::cout << var_prop.as<int>(); // 打印 '23'
```

当然，你也可以以更简洁的写法进行操作，rainy-toolkit重载了一个仿函数运算符，通过仿函数运算符实现更直观的写法：

```cpp
MyStruct obj;

property prop = type::get<MyStruct>().get_property("data");
prop(obj) = 23;

auto var_prop = prop(obj);
std::cout << var_prop; // 打印 '23'
```

## 调用方法

同样的，你也可以以类RTTR的接口进行方法的调用，这是完全合法的。

```cpp
MyStruct obj;

method meth = type::get<MyStruct>().get_method("func");
meth.invoke(obj, 42.0);

shared_object var = type::get<MyStruct>().create();
meth.invoke(var, 42.0);
```

当然，你也可以使用`42.f`、`42.0`、`42ull`这样的形式进行调用，`rainy-toolkit`提供了一套转换规则以应对这类需求。甚至可以直接使用any对象作为参数，这将会自动处理。

```cpp
MyStruct obj;

method meth = type::get<MyStruct>().get_method("func");
meth.invoke(obj, 42.0);
meth.invoke(obj, 42.f);
meth.invoke(obj, 42ull);

rainy::utility::any dynamic_var = 42;
meth.invoke(obj, dynamic_var);
```

更多API和示例代码请参见文档部分。