# `<function.hpp>` 函数

## make_function

适用于function类的工厂函数，通过静态域函数、对象实例函数的指针构造一个function对象用于反射

```cpp
template <typename Fx>
rainy::meta::reflection::function make_function(Fx &&fx) noexcept;
```

### 参数

`fx`: fx的类型必须能够满足静态域函数、对象实例函数的任一一个的定义。或者说，此Fx类型的参数必须能够用于构造function对象。否则将引发编译错误！


### 返回值

返回绑定fx指向的调用实体的function对象

## invoke

允许用户以类`std::invoke/rainy::utility::invoke`的方式，调用function类函数对象。且自动推导值的类别

```cpp
template <typename... Args>
rainy::utility::any invoke(const function &fn,object_view instance, Args &&...args);
```

### 参数

`fn`: function对象，必须是有效的（存有可调用函数指针）
`instance`: 如果目标是个对象实例函数，则此处需提供对象实例函数的引用，否则请使用`non_exists_instance`对象
`args...`: 任意数量的参数，用于提供给目标

### 返回值

如果调用目标的返回类型不为void，则此部分返回的any将包含实际的返回值内容。如，目标调用返回`int`类型，数据为`42`。则`any`将包含`int`类型的rtti信息和数据`42`。

### 备注

若args的类型与对应的目标参数类型并不完全一致，则function可能会尝试从rainy::utility::any_converter中进行类型转换。例如，float可转换为int，int可转换为float这类。但是，部分类型可能会存在转换失败问题，或是潜在的未定义行为，详见warning部分

:::warning
invoke的调用中，Args的类型均以`原样`被提供给调用目标。因此，诸如下列的对象：
```cpp
rainy::utility::any any_a(std::in_place_type<rainy::utility::any>, 10);
```
在推导中，仍然会以rainy::utility::any进行提供。且，若类型不匹配，在转换的时候，可能会产生运行时强制终止的崩溃。
此外，对于any_converter，目前已知以下问题：

1. 无法对不同类型进行引用级转换，如int&无法转换为long&，仅支持诸如int&转换为const int&这样的同类型引用转换
2. any_converter无法直接将当前类型转换为另一个类型的引用，仅支持值的构造（有可能导致UB）
:::
