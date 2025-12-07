# basic_poly 类

一个基础的多态包装器，通过类型擦除（Type Erasure）实现多态性，并管理对底层对象的指针和虚表（VTable）。用于避免虚表的开销，同时为编译器提供更多的优化机会。

<DeclarationTable
    header="rainy/core/type_traits.hpp"
    :content="`
(1) (since C++17)
template <typename AbstractBody>
class basic_poly;
`
"
/>

**模板类型形参**：
- `AbstractBody`: 定义了多态接口和 VTable 结构的策略类。它必须包含一个 `template type` 成员模板。

## 成员

`basic_poly` 类定义了如下成员：

<MergeTable>
成员函数|说明;;
[basic_poly](#basic_poly) | 构造并初始化 `basic_poly` 实例。;;
[reset](#reset) | 重置或重新设置底层的目标指针和虚表。;;
[target_as_void_ptr](#target_as_void_ptr) | 返回底层目标指针的 `void*` 形式。;;
</MergeTable>

<MergeTable>
类内部类型|说明;;
[abstract_type](#abstract_type) | 用于继承和成员访问的基类型。;;
[vtable_info](#vtable_info) | 虚表信息策略的类型。;;
[vtable_type](#vtable_type) | 实际虚表数据的类型。;;
</MergeTable>


<MergeTable>
运算符|说明;;
[operator=](#operator=) | 赋值操作，包括移动赋值和空指针赋值。;;
[operator bool](#bp-operator-bool) | 显式布尔转换，检查是否持有有效目标。;;
[operator->](#operator->) | 箭头运算符重载，提供对基类型成员的访问。;;
</MergeTable>

## basic_poly {#basic_poly}

<DeclarationTable 
    :content="
  `
(1) (since C++17)
basic_poly() noexcept;
(2) (since C++17)
template <typename Type>
basic_poly(Type *ptr) noexcept;
(3) (since C++17)
basic_poly(std::nullptr_t);
(4) (since C++17)
basic_poly(basic_poly &&right) noexcept;
(5) (since C++17)
basic_poly(const basic_poly &right);
  `
"
/>

执行相关构造操作并生成一个basic_poly对象。

<ParameterSection>

`right`: 待拷贝或移动的 `basic_poly` 实例。;;
`ptr`: 指向目标对象的指针。

</ParameterSection>

<ParameterSection isTypeParameter="true">

`Type`: 目标指针所指向对象的类型。

</ParameterSection>

## reset

**语法**：

```cpp
void reset() noexcept; // (1)
template <typename Type>
void reset(Type *ptr) noexcept; // (2)
```

**(1)**

将 `basic_poly` 实例重置为空状态，释放其当前持有的目标指针和虚表。

**(2)**

将 `basic_poly` 实例重置为持有新目标对象 `ptr` 的状态，并重新初始化虚表。

**类型参数**
- `Type`: 新目标指针所指向对象的类型。

**参数**
- `ptr`: 指向新目标对象的指针。
## target_as_void_ptr

返回底层目标指针的 `void*` 形式。

**语法**:

```cpp
void *target_as_void_ptr() const noexcept;
```

**备注**
- 返回值是指向目标对象的原始 `void*` 指针，可能为 `nullptr`。
## operator=

**语法**: 

```cpp
basic_poly &operator=(basic_poly &&other) noexcept; // (1)
basic_poly &operator=(std::nullptr_t) noexcept; // (2)
basic_poly &operator=(const basic_poly &) = default; // (3)
```

**(1)**

移动赋值运算符。从另一个 `basic_poly` 实例中移动资源，并返回对当前实例的引用。移动后，`other` 实例将处于空状态。

**参数**
- `other`: 待移动的 `basic_poly` 实例。

**(2)**

空指针赋值运算符。将 `basic_poly` 实例重置为空状态。

**(3)**

拷贝赋值运算符。通过默认实现进行按位拷贝。
## operator bool {#bp-operator-bool}

显式布尔转换运算符。用于检查 `basic_poly` 是否持有有效的目标对象。

**语法**:

```cpp
RAINY_NODISCARD explicit operator bool() const noexcept;
```

**备注**
- 当且仅当内部指针不为 `nullptr` 时返回 `true`。
## operator-> {#operator->}

```cpp
RAINY_NODISCARD abstract_type *operator->() noexcept; // (1)
RAINY_NODISCARD const abstract_type *operator->() const noexcept; // (2)
```

**(1)**

非 `const` 版本的箭头运算符，用于访问 `basic_poly` 所继承的 `abstract_type` 类型的成员。

**(2)**

`const` 版本的箭头运算符，用于访问 `basic_poly` 所继承的 `abstract_type` 类型的 `const` 成员。
