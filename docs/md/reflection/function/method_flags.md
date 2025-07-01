# `method_flags` 枚举

## 介绍

`method_flags`是一个枚举类，它是对函数进行反射的对象的属性表示。这是最基本的元信息。通常，`method_flags`是被定义如下

```cpp
enum class method_flags : std::uint8_t {
    none,
    static_specified,
    memfn_specified,
    noexcept_specified,
    lvalue_qualified,
    rvalue_qualifie,
    const_qualified,
    volatile_qualified
};
```

其中，具体的值可能会有所变化，因此不进行展示。请参见源文件定义的`method_flags`。

以下是`method_flags`各值的表示形式。

- `none`: 占位值，表示一个无任何限定的值，它必定为`0`。不过，此值一般用于帮助其它函数寻找重载版本，因此，不需要在意此值
- `static_specified`: 一般，用于表示一种非`对象实例函数`的标志，一般，若该invoker内部的`method_flags`枚举值中存储该枚举，则意味着，当前函数不会要求传入对象。可使用静态调用方式。通常这会存储如函数对象（lambda表达式在内），也可能存储非成员函数指针（静态域函数）。
- `memfn_specified`: 一般，用于表示`对象实例函数`。若此枚举存在，即代表当前对象存储了一个成员函数指针
- `noexcept_specified`: 表示此对象存储的调用器调用类型被声明为`noexcept`，即不会抛出异常
- `lvalue_qualified`: 表示此对象存储的调用器是面向一个左值引用对象使用的，仅对象实例函数
- `rvalue_qualified`: 表示此对象存储的调用器是面向一个右值引用对象使用的，仅对象实例函数
- `const_qualified`: 表示此对象存储的调用目标是具有常量性的，仅对象实例函数
- `volatile_qualfied`: 表示此对象存储的调用目标是具有易变性的，，仅对象实例函数

## 用途

在介绍中已被展开。

很显然，`method_flags`被用于快速查询当前函数的属性。

每个`function`类及其派生类对象均被绑定了一个调用器。详情可参见`rainy-toolkit`文档的内部实现页中的`invoke_accessor`抽象机制（若需要）。此处简要提及，通常为了确定当前function对象具体调用的函数调用类型。通常，在编译期中会生成代码方便在运行时查询或修改。

其中，在调用器内部将会生成该`method_flags`的具体表示以方便查询。

使用者一般可通过`type()`方法访问这个值，不过，为了简化查询，建议使用在`function`类提供的如下函数，他们简化了查询，可带来良好的可读性

|函数名称|说明|
|-|-|
|is_noexcept|检查当前function对象是否具有noexcept属性|
|is_static|检查当前function对象是否为静态域函数|
|is_const|检查当前function对象是否为成员实例函数，且检查是否具有const属性|
|is_volatile|检查当前function对象是否为成员实例函数，且检查是否具有volatile属性|
|is_memfn|检查当前function对象是否为成员实例函数|
|is_invoke_for_lvalue|检查当前function对象是否是适用于左值引用的调用|
|is_invoke_for_rvalue|检查当前function对象是否是适用于右值引用的调用|

具体文档说明请参见[function 类](/md/reflection/function/function_class)的文档部分。