# typeinfo.hpp

## Classes

| Name | Description |
|------|-------------|
| [`hash`](#hash) |  |
| [`hash`](#hash) | 为std::hash定义的特化模板 |
| [`typeinfo`](#typeinfo) |  支持编译期计算的类型信息抽象 可用于查询类型信息  |
| [`conversion_key`](#conversion_key) |  |
| [`conversion_key_hash`](#conversion_key_hash) |  |
| [`any_converter`](#any_converter) |  |
| [`enable_for_type_convert`](#enable_for_type_convert) | ! |

## Functions

| Name | Description |
|------|-------------|
| [`type_name`](#type_name) | 通过类型获取一个名称 |
| [`variable_name`](#variable_name) | 获取一个可在编译时确定的变量名 |
| [`register_base`](#register_base) |  |
| [`apply_offset`](#apply_offset) |  |
| [`dynamic_convert`](#dynamic_convert) |  |
| [`is_convertible_to`](#is_convertible_to) |  |
| [`get_conversion_map`](#get_conversion_map) |  |
| [`register_conversion`](#register_conversion) |  |
| [`find_converter`](#find_converter) |  |

## Marcos

| Name | Description |
|------|-------------|
| [`rainy_typeid`](#rainy_typeid) | 用于获取类型信息的宏，考虑到使用传统rtti的使用习惯  |
| [`rainy_typehash`](#rainy_typehash) | 用于类型switch匹配 |
| [`RAINY_REGISTER_BASE`](#RAINY_REGISTER_BASE) | 用于快捷注册一个类的继承关系 |

---

## struct `hash`

> **Specialization** `<rainy::foundation::ctti::typeinfo>`

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#hash-operator())  |  |
| [hash_this_val](#hash-hash_this_val)  |  |

---

### `operator()` {#hash-operator()}

```cpp
std::size_t operator()(const argument_type & right) const;
```

`const`

### `hash_this_val` {#hash-hash_this_val}

```cpp
std::size_t hash_this_val(const argument_type & val) noexcept;
```

`static` `noexcept`

## struct `hash`

> **Specialization** `<rainy::foundation::ctti::typeinfo>`

为std::hash定义的特化模板

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#hash-operator())  |  |

---

### `operator()` {#hash-operator()}

```cpp
std::size_t operator()(const rainy::foundation::ctti::typeinfo & val) const noexcept;
```

`const` `noexcept`

## class `typeinfo`

 支持编译期计算的类型信息抽象 可用于查询类型信息 

### Member Functions

| Function | Description |
|----------|-------------|
| [typeinfo](#typeinfo-typeinfo) *(3 overloads)*  |  |
| [operator=](#typeinfo-operator=) *(2 overloads)*  |  |
| [name](#typeinfo-name)  | 获取类型信息中对应的名称  |
| [hash_code](#typeinfo-hash_code)  | 获取类型信息中对应的哈希值  |
| [is_same](#typeinfo-is_same)  | 检查两个类型信息是否相同 |
| [is_void](#typeinfo-is_void)  | 检查两个类型信息是否为void |
| [remove_const](#typeinfo-remove_const)  | 对当前类型执行移除const属性操作. |
| [remove_volatile](#typeinfo-remove_volatile)  | 对当前类型执行移除volatile属性操作. |
| [remove_cv](#typeinfo-remove_cv)  | 对当前类型执行移除const和volatile属性操作. |
| [remove_reference](#typeinfo-remove_reference)  | 对当前类型执行移除reference属性操作. |
| [remove_cvref](#typeinfo-remove_cvref)  | 对当前类型执行移除const、volatile以及reference属性操作. |
| [remove_pointer](#typeinfo-remove_pointer)  | 对当前类型执行移除指针属性操作. |
| [decay](#typeinfo-decay)  | 对当前类型执行decay操作. |
| [is_compatible](#typeinfo-is_compatible)  |  |
| [has_traits](#typeinfo-has_traits)  | 检查此类型信息是否具有特定的traits |
| [sizeof_the_type](#typeinfo-sizeof_the_type)  | 获取当前类型的sizeof大小 |
| [template_arguments](#typeinfo-template_arguments)  | 尝试获取当前类型的模板实例化参数 |
| [operator unsigned long](#typeinfo-operator unsigned long)  | 用于适配到switch(x)中，实现编译时类型匹配  |
| [is_arithmetic](#typeinfo-is_arithmetic)  | 检查当前是否为算数类型. |
| [is_floating_point](#typeinfo-is_floating_point)  | 检查当前是否为浮点类型. |
| [is_integer](#typeinfo-is_integer)  | 检查当前是否为整型类型. |
| [is_nullptr](#typeinfo-is_nullptr)  | 检查当前是否为空指针类型. |
| [is_reference](#typeinfo-is_reference)  | 检查当前是否为引用类型（包括左或右）. |
| [is_pointer](#typeinfo-is_pointer)  | 检查当前是否为指针类型. |
| [is_lvalue_reference](#typeinfo-is_lvalue_reference)  | 检查当前是否为左引用类型. |
| [is_rvalue_reference](#typeinfo-is_rvalue_reference)  | 检查当前是否为右引用类型. |
| [is_const](#typeinfo-is_const)  | 检查当前是否为const类型. |
| [is_volatile](#typeinfo-is_volatile)  | 检查当前是否为volatile类型. |
| [is_const_volatile](#typeinfo-is_const_volatile)  | 检查当前是否为const volatile混合的类型. |
| [is_associative_container](#typeinfo-is_associative_container)  | 检查当前是否为关联容器类型. |
| [is_sequential_container](#typeinfo-is_sequential_container)  | 检查当前是否为序列容器类型. |
| [is_class](#typeinfo-is_class)  | 检查当前是否为类类型. |

---

### `typeinfo` {#typeinfo-typeinfo}

#### Overload 1

```cpp
void typeinfo() noexcept = default;
```

`noexcept` `= default`

#### Overload 2

```cpp
void typeinfo(annotations::lifetime::in<typeinfo>) noexcept = default;
```

`noexcept` `= default`

#### Overload 3

```cpp
void typeinfo(annotations::lifetime::move_from<typeinfo>) noexcept = default;
```

`noexcept` `= default`

### `operator=` {#typeinfo-operator=}

#### Overload 1

```cpp
typeinfo & operator=(annotations::lifetime::in<typeinfo>) noexcept = default;
```

`noexcept` `= default`

#### Overload 2

```cpp
typeinfo & operator=(annotations::lifetime::move_from<typeinfo>) noexcept = default;
```

`noexcept` `= default`

### `name` {#typeinfo-name}

```cpp
std::string_view name() const noexcept;
```

`const` `noexcept`

获取类型信息中对应的名称 

### `hash_code` {#typeinfo-hash_code}

```cpp
std::size_t hash_code() const noexcept;
```

`const` `noexcept`

获取类型信息中对应的哈希值 

### `is_same` {#typeinfo-is_same}

```cpp
bool is_same(annotations::lifetime::in<typeinfo> right) const noexcept;
```

`const` `noexcept`

检查两个类型信息是否相同

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `right` | `annotations::lifetime::in<typeinfo>` | in | 另一个类型信息对象 |

**Returns**

如果类型信息相同，返回true，否则返回false 

### `is_void` {#typeinfo-is_void}

```cpp
bool is_void() const noexcept;
```

`const` `noexcept`

检查两个类型信息是否为void

**Returns**

如果类型信息是void，返回true，否则返回false 

### `remove_const` {#typeinfo-remove_const}

```cpp
typeinfo remove_const() const noexcept;
```

`const` `noexcept`

对当前类型执行移除const属性操作.

**Returns**

返回被处理后的类型信息 

### `remove_volatile` {#typeinfo-remove_volatile}

```cpp
typeinfo remove_volatile() const noexcept;
```

`const` `noexcept`

对当前类型执行移除volatile属性操作.

**Returns**

返回被处理后的类型信息 

### `remove_cv` {#typeinfo-remove_cv}

```cpp
typeinfo remove_cv() const noexcept;
```

`const` `noexcept`

对当前类型执行移除const和volatile属性操作.

**Returns**

返回被处理后的类型信息 

### `remove_reference` {#typeinfo-remove_reference}

```cpp
typeinfo remove_reference() const noexcept;
```

`const` `noexcept`

对当前类型执行移除reference属性操作.

**Returns**

返回被处理后的类型信息 

### `remove_cvref` {#typeinfo-remove_cvref}

```cpp
typeinfo remove_cvref() const noexcept;
```

`const` `noexcept`

对当前类型执行移除const、volatile以及reference属性操作.

**Returns**

返回被处理后的类型信息 

### `remove_pointer` {#typeinfo-remove_pointer}

```cpp
typeinfo remove_pointer() const noexcept;
```

`const` `noexcept`

对当前类型执行移除指针属性操作.

**Returns**

返回被处理后的类型信息 

### `decay` {#typeinfo-decay}

```cpp
typeinfo decay() const noexcept;
```

`const` `noexcept`

对当前类型执行decay操作.

**Returns**

返回被处理后的类型信息 

### `is_compatible` {#typeinfo-is_compatible}

```cpp
bool is_compatible(annotations::lifetime::in<typeinfo> right) const noexcept;
```

`const` `noexcept`

### `has_traits` {#typeinfo-has_traits}

```cpp
bool has_traits(annotations::lifetime::in<traits> traits) const noexcept;
```

`const` `noexcept`

检查此类型信息是否具有特定的traits

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `traits` | `annotations::lifetime::in<traits>` | in | 要检查的traits，从traits枚举中获取 |

**Returns**

如果类型信息具有traits，返回true，否则返回false 

### `sizeof_the_type` {#typeinfo-sizeof_the_type}

```cpp
std::size_t sizeof_the_type() const noexcept;
```

`const` `noexcept`

获取当前类型的sizeof大小

**Returns**

返回当前类型的sizeof大小 

### `template_arguments` {#typeinfo-template_arguments}

```cpp
collections::views::array_view<typeinfo> template_arguments() const noexcept;
```

`const` `noexcept`

尝试获取当前类型的模板实例化参数

> **Attention:** 对于 template <std::size_t> 这类带有NTTP参数的模板，则无法获取，仅支持纯类型的模板

**Returns**

返回模板实例化参数类型的列表视图 

### `operator unsigned long` {#typeinfo-operator unsigned long}

```cpp
std::size_t operator unsigned long() const noexcept;
```

`const` `noexcept`

用于适配到switch(x)中，实现编译时类型匹配 

### `is_arithmetic` {#typeinfo-is_arithmetic}

```cpp
bool is_arithmetic() const noexcept;
```

`const` `noexcept`

检查当前是否为算数类型.

**Returns**

如果是返回true，否则false 

### `is_floating_point` {#typeinfo-is_floating_point}

```cpp
bool is_floating_point() const noexcept;
```

`const` `noexcept`

检查当前是否为浮点类型.

**Returns**

如果是返回true，否则false 

### `is_integer` {#typeinfo-is_integer}

```cpp
bool is_integer() const noexcept;
```

`const` `noexcept`

检查当前是否为整型类型.

**Returns**

如果是返回true，否则false 

### `is_nullptr` {#typeinfo-is_nullptr}

```cpp
bool is_nullptr() const noexcept;
```

`const` `noexcept`

检查当前是否为空指针类型.

**Returns**

如果是返回true，否则false 

### `is_reference` {#typeinfo-is_reference}

```cpp
bool is_reference() const noexcept;
```

`const` `noexcept`

检查当前是否为引用类型（包括左或右）.

**Returns**

如果是返回true，否则false 

### `is_pointer` {#typeinfo-is_pointer}

```cpp
bool is_pointer() const noexcept;
```

`const` `noexcept`

检查当前是否为指针类型.

**Returns**

如果是返回true，否则false 

### `is_lvalue_reference` {#typeinfo-is_lvalue_reference}

```cpp
bool is_lvalue_reference() const noexcept;
```

`const` `noexcept`

检查当前是否为左引用类型.

**Returns**

如果是返回true，否则false 

### `is_rvalue_reference` {#typeinfo-is_rvalue_reference}

```cpp
bool is_rvalue_reference() const noexcept;
```

`const` `noexcept`

检查当前是否为右引用类型.

**Returns**

如果是返回true，否则false 

### `is_const` {#typeinfo-is_const}

```cpp
bool is_const() const noexcept;
```

`const` `noexcept`

检查当前是否为const类型.

**Returns**

如果是返回true，否则false 

### `is_volatile` {#typeinfo-is_volatile}

```cpp
bool is_volatile() const noexcept;
```

`const` `noexcept`

检查当前是否为volatile类型.

**Returns**

如果是返回true，否则false 

### `is_const_volatile` {#typeinfo-is_const_volatile}

```cpp
bool is_const_volatile() const noexcept;
```

`const` `noexcept`

检查当前是否为const volatile混合的类型.

**Returns**

如果是返回true，否则false 

### `is_associative_container` {#typeinfo-is_associative_container}

```cpp
bool is_associative_container() const noexcept;
```

`const` `noexcept`

检查当前是否为关联容器类型.

**Returns**

如果是返回true，否则false 

### `is_sequential_container` {#typeinfo-is_sequential_container}

```cpp
bool is_sequential_container() const noexcept;
```

`const` `noexcept`

检查当前是否为序列容器类型.

**Returns**

如果是返回true，否则false 

### `is_class` {#typeinfo-is_class}

```cpp
bool is_class() const noexcept;
```

`const` `noexcept`

检查当前是否为类类型.

**Returns**

如果是返回true，否则false 

## struct `conversion_key`

### Member Functions

| Function | Description |
|----------|-------------|
| [operator==](#conversion_key-operator==)  |  |

### Member Variables

| Name | Type | Description |
|------|------|-------------|
| [source_hash](#conversion_key-source_hash) | `std::size_t` |  |
| [target_hash](#conversion_key-target_hash) | `std::size_t` |  |

---

### `operator==` {#conversion_key-operator==}

```cpp
bool operator==(const conversion_key & other) const;
```

`const`

### `source_hash` {#conversion_key-source_hash}

```cpp
std::size_t source_hash;
```

### `target_hash` {#conversion_key-target_hash}

```cpp
std::size_t target_hash;
```

## struct `conversion_key_hash`

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#conversion_key_hash-operator())  |  |

---

### `operator()` {#conversion_key_hash-operator()}

```cpp
std::size_t operator()(const conversion_key & k) const;
```

`const`

## class `any_converter`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `TargetType` |  |
| `` |  |

### Member Functions

| Function | Description |
|----------|-------------|
| [is_convertible](#any_converter-is_convertible)  |  |

---

### `is_convertible` {#any_converter-is_convertible}

```cpp
bool is_convertible(const foundation::ctti::typeinfo &);
```

`static`

## class `enable_for_type_convert`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `ConverterClass` | 静态类型描述类 |

!

### Member Functions

| Function | Description |
|----------|-------------|
| [~enable_for_type_convert<ConverterClass>](#enable_for_type_convert-~enable_for_type_convert<converterclass>)  |  |
| [enable_for_type_convert<ConverterClass>](#enable_for_type_convert-enable_for_type_convert<converterclass>)  |  |

### Nested Classes

| Name | Description |
|------|-------------|
| [inject](#inject) |  |

---

### `~enable_for_type_convert<ConverterClass>` {#enable_for_type_convert-~enable_for_type_convert<converterclass>}

```cpp
void ~enable_for_type_convert<ConverterClass>() = default;
```

`virtual` `= default`

### `enable_for_type_convert<ConverterClass>` {#enable_for_type_convert-enable_for_type_convert<converterclass>}

```cpp
void enable_for_type_convert<ConverterClass>() = default;
```

`= default`

### struct `inject`

### Member Functions

| Function | Description |
|----------|-------------|
| [inject](#inject-inject)  |  |

---

#### `inject` {#inject-inject}

```cpp
void inject();
```

## `type_name` {#type_name}

通过类型获取一个名称

**Returns:** s 一个字符串视图，表示对编译时变量的名称 

## `variable_name` {#variable_name}

`noexcept`

获取一个可在编译时确定的变量名

**Returns:** s 一个字符串视图，表示对编译时变量的名称 

## `register_base` {#register_base}

## `apply_offset` {#apply_offset}

`inline`

## `dynamic_convert` {#dynamic_convert}

## `is_convertible_to` {#is_convertible_to}

`inline` `noexcept`

## `get_conversion_map` {#get_conversion_map}

`inline`

## `register_conversion` {#register_conversion}

`inline`

## `find_converter` {#find_converter}

`inline`

