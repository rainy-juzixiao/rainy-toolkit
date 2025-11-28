# rainy-toolkit 反射

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

由此可见，这3个术语在其复杂性上并不会带来过大负担。另外，rainy-toolkit不会像某些库中去使用复杂的术语体系，对于函数，我们是仅界定了三个术语作为补充。而我们在类型中，仅会使用如下术语去描述反射：

- moon —— 静态反射
- lunar —— 动态反射
- 方法属性元
- 字段属性元
- 类型元
- shared_object/object_view 具有引用计数的生命周期动态对象/对象视图

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