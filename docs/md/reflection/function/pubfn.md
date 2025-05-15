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

在调用的时候，`function`内部会检查参数的可转换性，通常会计算参数是否适合且参数是否完全一致。若参数数量不一致，则`errno`会被设置为`EINVAL`，则尝试检查类型兼容性并在可能的情况下尝试不同类型的转换（`可能会导致std::terminate()`的调用）。但是，如果调用所需类型未被用于特化，则`errno`也会被设置为`EACCES`。表示无法找到适当的类型转换。（默认情况下提供对拷贝/移动构造、同类型引用转换、算术类型转换、`std::string_view`转换的支持。详见`any`文档的`any_converter`描述）。
