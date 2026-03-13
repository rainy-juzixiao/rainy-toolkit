# core.hpp

## Classes

| Name | Description |
|------|-------------|
| [`temporary_buffer`](#temporary_buffer) | A temporary buffer for POD types with automatic management. Provides RAII-style management of temporary memory buffers.  用于POD类型的临时缓冲区，支持自动管理。 提供RAII风格的临时内存缓冲区管理。  |
| [`poly_vtable`](#poly_vtable) | Virtual table provider for polymorphic concepts. Generates and stores the vtable for a given concept.  多态概念的虚表提供者。 为给定概念生成并存储虚表。  |
| [`poly_base`](#poly_base) | Base class for polymorphic objects providing virtual call dispatching. 为多态对象提供虚调用分派的基类。  |
| [`basic_poly`](#basic_poly) | Basic polymorphic wrapper that stores a void pointer and vtable. 存储void指针和虚表的基本多态包装器。  |
| [`poly_inspector`](#poly_inspector) |  |
| [`make_vtable`](#make_vtable) |  |
| [`resource_guard`](#resource_guard) |  |
| [`scope_guard`](#scope_guard) |  |
| [`hash`](#hash) | Primary template for hash function object. Provides hash computation for various types.  哈希函数对象的主模板。 为各种类型提供哈希计算。  |
| [`hash`](#hash) | Specialization for float type. float 类型的特化。  |
| [`hash`](#hash) | Specialization for double type. double 类型的特化。  |
| [`hash`](#hash) | Specialization for long double type. long double 类型的特化。  |
| [`hash`](#hash) | Specialization for nullptr_t type. nullptr_t 类型的特化。  |
| [`hash`](#hash) | Specialization for basic_string_view types. basic_string_view 类型的特化。  |
| [`hash`](#hash) | Specialization for basic_string types. basic_string 类型的特化。  |
| [`is_support_standard_hasher_available`](#is_support_standard_hasher_available) | Type trait to check if standard hasher is available for a type. 检查类型的标准哈希器是否可用的类型特性。  |
| [`is_support_standard_hasher_available`](#is_support_standard_hasher_available) | Specialization that detects if std::hash<Ty> is callable. 检测 std::hash<Ty> 是否可调用的特化。  |
| [`is_support_rainytoolkit_hasher_available`](#is_support_rainytoolkit_hasher_available) | Type trait to check if Rainy Toolkit hasher is available for a type. 检查类型的 Rainy Toolkit 哈希器是否可用的类型特性。  |
| [`is_support_rainytoolkit_hasher_available`](#is_support_rainytoolkit_hasher_available) | Specialization that detects if rainy::utility::hash<Ty> is callable. 检测 rainy::utility::hash<Ty> 是否可调用的特化。  |
| [`monostate`](#monostate) | Empty monostate type for use in variants and similar contexts. 用于变体等场景的空monostate类型。  |
| [`hash_enable_if`](#hash_enable_if) |  |
| [`hash_enable_if`](#hash_enable_if) |  |
| [`default_deleter`](#default_deleter) | Default deleter for unique_ptr and similar smart pointers. Uses delete operator for single objects.  用于 unique_ptr 等智能指针的默认删除器。 对单个对象使用 delete 操作符。  |
| [`default_deleter`](#default_deleter) | Default deleter specialization for array types. Uses delete[] operator for arrays.  数组类型的默认删除器特化。 对数组使用 delete[] 操作符。  |
| [`no_delete`](#no_delete) | No-op deleter that does nothing. Useful for smart pointers that don't own the resource.  空操作删除器，什么都不做。 用于不拥有资源的智能指针。  |
| [`no_delete`](#no_delete) | No-op deleter specialization for array types. Does nothing with the pointer.  数组类型的空操作删除器特化。 对指针不进行任何操作。  |
| [`iterator_range`](#iterator_range) | A range view over an iterator pair. 基于迭代器对的区间视图。  This class provides a view over a range defined by a pair of iterators, inheriting all functionality from adapter_iterator_range.  此类提供由一对迭代器定义的区间的视图， 继承自 adapter_iterator_range 的所有功能。  |
| [`iterator_range_iterator`](#iterator_range_iterator) |  |
| [`adapter_iterator_range`](#adapter_iterator_range) |  |

## Functions

| Name | Description |
|------|-------------|
| [`get_temporary_buffer`](#get_temporary_buffer) | Obtains a temporary buffer for a given number of elements. 获取用于指定数量元素的临时缓冲区。  |
| [`return_temporary_buffer`](#return_temporary_buffer) | Explicitly returns a temporary buffer. 显式归还临时缓冲区。  |
| [`is_main_thread`](#is_main_thread) |  |
| [`poly_call`](#poly_call) | Helper function to make polymorphic calls. 进行多态调用的辅助函数。  |
| [`swap_ranges`](#swap_ranges) | Swaps elements between two ranges. 交换两个范围之间的元素。  |
| [`fill`](#fill) | Assigns the given value to all elements in a range. 将给定值赋值给范围内的所有元素。  |
| [`fill_n`](#fill_n) | Assigns the given value to the first count elements in a range. 将给定值赋值给范围内的前count个元素。  |
| [`all_of`](#all_of) | Checks if a predicate is true for all elements in a range. 检查谓词是否对范围内的所有元素都为真。  |
| [`any_of`](#any_of) | Checks if a predicate is true for any element in a range. 检查谓词是否对范围内的任意元素为真。  |
| [`none_of`](#none_of) | Checks if a predicate is true for no elements in a range. 检查谓词是否对范围内没有元素为真。  |
| [`find`](#find) | Finds the first occurrence of a value in a range. 查找范围内第一次出现的值。  |
| [`find_if`](#find_if) | Finds the first element satisfying a predicate. 查找第一个满足谓词的元素。  |
| [`find_if_not`](#find_if_not) | Finds the first element not satisfying a predicate. 查找第一个不满足谓词的元素。  |
| [`equal`](#equal) | Checks if two ranges are equal. 检查两个范围是否相等。  |
| [`lexicographical_compare`](#lexicographical_compare) | Lexicographically compares two ranges. 字典序比较两个范围。  |
| [`lower_bound`](#lower_bound) | Returns an iterator to the first element not less than the given value. 返回指向第一个不小于给定值的元素的迭代器。  |
| [`upper_bound`](#upper_bound) | Returns an iterator to the first element greater than the given value. 返回指向第一个大于给定值的元素的迭代器。  |
| [`move`](#move) | Moves elements from one range to another. 将元素从一个范围移动到另一个范围。  |
| [`move_backward`](#move_backward) | Moves elements from one range to another, starting from the end. 从末尾开始将元素从一个范围移动到另一个范围。  |
| [`copy_backward`](#copy_backward) | Copies elements from one range to another, starting from the end. 从末尾开始将元素从一个范围复制到另一个范围。  |
| [`uninitialized_move_backward`](#uninitialized_move_backward) | Moves elements from one range to an uninitialized range, starting from the end. 从末尾开始将元素从一个范围移动到未初始化的范围。  |
| [`binary_search`](#binary_search) | Checks if a value exists in a sorted range using binary search. 使用二分查找检查值是否存在于已排序的范围中。  |
| [`binary_search`](#binary_search) | Checks if a value exists in a sorted range using binary search (default comparison). 使用二分查找检查值是否存在于已排序的范围中（默认比较）。  |
| [`copy`](#copy) | Copies elements from a range to another range. 将元素从一个范围复制到另一个范围。  |
| [`copy_n`](#copy_n) | Copies exactly n elements from a range to another range. 从一个范围精确复制n个元素到另一个范围。  |
| [`transform`](#transform) | Applies a function to each element in a range and stores the results. 对范围内的每个元素应用函数并存储结果。  |
| [`transform`](#transform) | Applies a binary function to elements from two ranges and stores the results. 对两个范围的元素应用二元函数并存储结果。  |
| [`popcount`](#popcount) | Counts the number of 1 bits in an unsigned integer. 计算无符号整数中1的位数。  |
| [`has_single_bit`](#has_single_bit) | Checks if a value has exactly one 1 bit (is a power of two). 检查一个值是否恰好有一个1位（是否为2的幂）。  |
| [`next_power_of_two`](#next_power_of_two) | Returns the next power of two greater than or equal to the input. 返回大于或等于输入值的下一个2的幂。  |
| [`bit_width`](#bit_width) | Returns the number of bits needed to represent the value. 返回表示该值所需的位数。  |
| [`bit_floor`](#bit_floor) | Returns the largest power of two not greater than the value. 返回不大于该值的最大2的幂。  |
| [`countl_zero`](#countl_zero) | Counts the number of consecutive 0 bits starting from the most significant bit. 从最高有效位开始计算连续0位的数量。  |
| [`countr_zero`](#countr_zero) | Counts the number of consecutive 0 bits starting from the least significant bit. 从最低有效位开始计算连续0位的数量。  |
| [`mod`](#mod) | Computes value % mod when mod is a power of two. 当mod是2的幂时计算value % mod。  |
| [`rotate_left`](#rotate_left) | Rotates bits to the left. 向左旋转位。  |
| [`rotate_right`](#rotate_right) | Rotates bits to the right. 向右旋转位。  |
| [`is_pow_2`](#is_pow_2) |  |
| [`get_size_of_n`](#get_size_of_n) |  |
| [`in_range`](#in_range) |  |
| [`stl_internal_check`](#stl_internal_check) |  |
| [`make_scope_guard`](#make_scope_guard) |  |
| [`fnv1a_append_bytes`](#fnv1a_append_bytes) |  |
| [`fnv1a_append_value`](#fnv1a_append_value) |  |
| [`fnv1a_append_range`](#fnv1a_append_range) |  |
| [`hash_representation`](#hash_representation) |  |
| [`hash_array_representation`](#hash_array_representation) |  |

---

## class `temporary_buffer`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The element type, must be a plain old data type |

A temporary buffer for POD types with automatic management. Provides RAII-style management of temporary memory buffers.  用于POD类型的临时缓冲区，支持自动管理。 提供RAII风格的临时内存缓冲区管理。 

元素类型，必须是POD类型

### Member Functions

| Function | Description |
|----------|-------------|
| [temporary_buffer<Ty>](#temporary_buffer-temporary_buffer<ty>) *(3 overloads)*  | Constructs a temporary buffer from an existing buffer. 从现有缓冲区构造临时缓冲区。  |
| [~temporary_buffer<Ty>](#temporary_buffer-~temporary_buffer<ty>)  | Destructor that automatically returns the buffer. 析构函数，自动归还缓冲区。  |
| [operator=](#temporary_buffer-operator=) *(2 overloads)*  | Move assignment operator. 移动赋值运算符。  |
| [begin](#temporary_buffer-begin) *(2 overloads)*  | Returns an iterator to the beginning. 返回指向起始的迭代器。  |
| [end](#temporary_buffer-end) *(2 overloads)*  | Returns an iterator to the end. 返回指向末尾的迭代器。  |
| [return_buffer](#temporary_buffer-return_buffer)  | Returns the buffer to the system. 将缓冲区归还给系统。  |
| [reallocate](#temporary_buffer-reallocate)  | Reallocates the buffer to a new size. 重新分配缓冲区到新大小。  |
| [get_buffer](#temporary_buffer-get_buffer) *(2 overloads)*  | Gets the raw buffer pointer. 获取原始缓冲区指针。  |
| [operator bool](#temporary_buffer-operator bool)  | Checks if the buffer is valid. 检查缓冲区是否有效。  |
| [operator type-parameter-0-0 *](#temporary_buffer-operator type-parameter-0-0 *)  | Implicit conversion to pointer. 到指针的隐式转换。  |
| [operator const type-parameter-0-0 *](#temporary_buffer-operator const type-parameter-0-0 *)  | Implicit conversion to const pointer. 到常量指针的隐式转换。  |

---

### `temporary_buffer<Ty>` {#temporary_buffer-temporary_buffer<ty>}

#### Overload 1

```cpp
void temporary_buffer<Ty>(Ty * buffer, difference_type count, difference_type capacity);
```

Constructs a temporary buffer from an existing buffer. 从现有缓冲区构造临时缓冲区。 

指向内存缓冲区的指针 已初始化元素的数量 缓冲区的总容量（以元素为单位）

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `count` | `Ty *` | in | Number of initialized elements |
| `capacity` | `difference_type` | in | Total capacity of the buffer in elements |
| `buffer` | `difference_type` | in | Pointer to the memory buffer |

#### Overload 2

```cpp
void temporary_buffer<Ty>(temporary_buffer<Ty> && right) noexcept;
```

`noexcept`

Move constructor. 移动构造函数。 

要移动的 temporary_buffer

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `right` | `temporary_buffer<Ty> &&` | in | The temporary_buffer to move from |

#### Overload 3

```cpp
void temporary_buffer<Ty>(const temporary_buffer<Ty> &) = delete;
```

`= delete`

### `~temporary_buffer<Ty>` {#temporary_buffer-~temporary_buffer<ty>}

```cpp
void ~temporary_buffer<Ty>();
```

Destructor that automatically returns the buffer. 析构函数，自动归还缓冲区。 

### `operator=` {#temporary_buffer-operator=}

#### Overload 1

```cpp
temporary_buffer<Ty> & operator=(temporary_buffer<Ty> && right) noexcept;
```

`noexcept`

Move assignment operator. 移动赋值运算符。 

要移动的 temporary_buffer

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `right` | `temporary_buffer<Ty> &&` | in | The temporary_buffer to move from |

**Returns**

Reference to this object 此对象的引用 

#### Overload 2

```cpp
temporary_buffer<Ty> & operator=(const temporary_buffer<Ty> &) = delete;
```

`= delete`

### `begin` {#temporary_buffer-begin}

#### Overload 1

```cpp
iterator begin() noexcept;
```

`noexcept`

Returns an iterator to the beginning. 返回指向起始的迭代器。 

**Returns**

Iterator to the first element 指向第一个元素的迭代器 

#### Overload 2

```cpp
const_iterator begin() const noexcept;
```

`const` `noexcept`

Returns a const iterator to the beginning. 返回指向起始的常量迭代器。 

**Returns**

Const iterator to the first element 指向第一个元素的常量迭代器 

### `end` {#temporary_buffer-end}

#### Overload 1

```cpp
iterator end() noexcept;
```

`noexcept`

Returns an iterator to the end. 返回指向末尾的迭代器。 

**Returns**

Iterator to one past the last element 指向最后一个元素之后位置的迭代器 

#### Overload 2

```cpp
const_iterator end() const noexcept;
```

`const` `noexcept`

Returns a const iterator to the end. 返回指向末尾的常量迭代器。 

**Returns**

Const iterator to one past the last element 指向最后一个元素之后位置的常量迭代器 

### `return_buffer` {#temporary_buffer-return_buffer}

```cpp
void return_buffer();
```

Returns the buffer to the system. 将缓冲区归还给系统。 

### `reallocate` {#temporary_buffer-reallocate}

```cpp
void reallocate(const std::size_t realloc);
```

Reallocates the buffer to a new size. 重新分配缓冲区到新大小。 

请求的元素数量

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `realloc` | `const std::size_t` | in | The requested number of elements |

### `get_buffer` {#temporary_buffer-get_buffer}

#### Overload 1

```cpp
pointer get_buffer() noexcept;
```

`noexcept`

Gets the raw buffer pointer. 获取原始缓冲区指针。 

**Returns**

Pointer to the buffer 指向缓冲区的指针 

#### Overload 2

```cpp
const_pointer get_buffer() const noexcept;
```

`const` `noexcept`

Gets the raw buffer pointer (const version). 获取原始缓冲区指针（常量版本）。 

**Returns**

Const pointer to the buffer 指向缓冲区的常量指针 

### `operator bool` {#temporary_buffer-operator bool}

```cpp
bool operator bool() const noexcept;
```

`const` `noexcept`

Checks if the buffer is valid. 检查缓冲区是否有效。 

**Returns**

true if buffer is valid, false otherwise 如果缓冲区有效则为true，否则为false 

### `operator type-parameter-0-0 *` {#temporary_buffer-operator type-parameter-0-0 *}

```cpp
pointer operator type-parameter-0-0 *() noexcept;
```

`noexcept`

Implicit conversion to pointer. 到指针的隐式转换。 

**Returns**

Pointer to the buffer 指向缓冲区的指针 

### `operator const type-parameter-0-0 *` {#temporary_buffer-operator const type-parameter-0-0 *}

```cpp
const_pointer operator const type-parameter-0-0 *() const noexcept;
```

`const` `noexcept`

Implicit conversion to const pointer. 到常量指针的隐式转换。 

**Returns**

Const pointer to the buffer 指向缓冲区的常量指针 

## class `poly_vtable`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Concept` | The concept that defines the polymorphic interface |

Virtual table provider for polymorphic concepts. Generates and stores the vtable for a given concept.  多态概念的虚表提供者。 为给定概念生成并存储虚表。 

定义多态接口的概念

---

## class `poly_base`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Poly` | The derived polymorphic type (CRTP) |

Base class for polymorphic objects providing virtual call dispatching. 为多态对象提供虚调用分派的基类。 

派生的多态类型（CRTP）

---

## class `basic_poly`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `AbstractBody` | The abstract concept body that defines the interface |

Basic polymorphic wrapper that stores a void pointer and vtable. 存储void指针和虚表的基本多态包装器。 

定义接口的抽象概念主体

**Inherits:** `private` AbstractBody::template type<poly_base<basic_poly<AbstractBody> > >

### Member Functions

| Function | Description |
|----------|-------------|
| [basic_poly<AbstractBody>](#basic_poly-basic_poly<abstractbody>) *(4 overloads)*  | Default constructor 默认构造函数  |
| [operator=](#basic_poly-operator=) *(3 overloads)*  | Move assignment operator 移动赋值运算符  |
| [reset](#basic_poly-reset)  | Resets to null state. 重置为空状态。  |
| [empty](#basic_poly-empty)  | Checks if the object is empty. 检查对象是否为空。  |
| [operator bool](#basic_poly-operator bool)  | Checks if the object holds a non-null pointer. 检查对象是否持有非空指针。  |
| [operator->](#basic_poly-operator->) *(2 overloads)*  | Arrow operator to access the abstract interface. 箭头运算符，用于访问抽象接口。  |
| [target_as_void_ptr](#basic_poly-target_as_void_ptr)  | Gets the stored pointer as void*. 获取存储的指针作为void*。  |

---

### `basic_poly<AbstractBody>` {#basic_poly-basic_poly<abstractbody>}

#### Overload 1

```cpp
void basic_poly<AbstractBody>() noexcept = default;
```

`noexcept` `= default`

Default constructor 默认构造函数 

#### Overload 2

```cpp
void basic_poly<AbstractBody>(std::nullptr_t);
```

Constructs a null polymorphic object. 构造空多态对象。 

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `nullptr_t` | `std::nullptr_t` | in |  |

#### Overload 3

```cpp
void basic_poly<AbstractBody>(basic_poly<AbstractBody> && other) noexcept;
```

`noexcept`

Move constructor 移动构造函数 

要移动的另一个多态对象

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `other` | `basic_poly<AbstractBody> &&` | in | The other polymorphic object to move from |

#### Overload 4

```cpp
void basic_poly<AbstractBody>(const basic_poly<AbstractBody> &) = default;
```

`= default`

Copy constructor 拷贝构造函数 

### `operator=` {#basic_poly-operator=}

#### Overload 1

```cpp
basic_poly<AbstractBody> & operator=(basic_poly<AbstractBody> && other) noexcept;
```

`noexcept`

Move assignment operator 移动赋值运算符 

要移动的另一个多态对象

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `other` | `basic_poly<AbstractBody> &&` | in | The other polymorphic object to move from |

**Returns**

Reference to this object 此对象的引用 

#### Overload 2

```cpp
basic_poly<AbstractBody> & operator=(std::nullptr_t) noexcept;
```

`noexcept`

Assigns a null pointer. 赋值为空指针。 

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `nullptr_t` | `std::nullptr_t` | in |  |

**Returns**

Reference to this object 此对象的引用 

#### Overload 3

```cpp
basic_poly<AbstractBody> & operator=(const basic_poly<AbstractBody> &) = default;
```

`= default`

Copy assignment operator 拷贝赋值运算符 

### `reset` {#basic_poly-reset}

```cpp
void reset() noexcept;
```

`noexcept`

Resets to null state. 重置为空状态。 

### `empty` {#basic_poly-empty}

```cpp
bool empty() const noexcept;
```

`const` `noexcept`

Checks if the object is empty. 检查对象是否为空。 

**Returns**

true if empty, false otherwise 如果为空则为true，否则为false 

### `operator bool` {#basic_poly-operator bool}

```cpp
bool operator bool() const noexcept;
```

`const` `noexcept`

Checks if the object holds a non-null pointer. 检查对象是否持有非空指针。 

**Returns**

true if pointer is non-null, false otherwise 如果指针非空则为true，否则为false 

### `operator->` {#basic_poly-operator->}

#### Overload 1

```cpp
abstract_type * operator->() noexcept;
```

`noexcept`

Arrow operator to access the abstract interface. 箭头运算符，用于访问抽象接口。 

**Returns**

Pointer to the abstract interface 指向抽象接口的指针 

#### Overload 2

```cpp
const abstract_type * operator->() const noexcept;
```

`const` `noexcept`

Const arrow operator to access the abstract interface. 常量箭头运算符，用于访问抽象接口。 

**Returns**

Const pointer to the abstract interface 指向抽象接口的常量指针 

### `target_as_void_ptr` {#basic_poly-target_as_void_ptr}

```cpp
void * target_as_void_ptr() const noexcept;
```

`const` `noexcept`

Gets the stored pointer as void*. 获取存储的指针作为void*。 

**Returns**

The stored void pointer 存储的void指针 

## struct `poly_inspector`

---

## class `make_vtable`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Concept` |  |

---

## class `resource_guard`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` |  |
| `Dx` |  |

### Member Functions

| Function | Description |
|----------|-------------|
| [resource_guard<Ty, Dx>](#resource_guard-resource_guard<ty, dx>) *(6 overloads)*  |  |
| [operator=](#resource_guard-operator=) *(2 overloads)*  |  |
| [~resource_guard<Ty, Dx>](#resource_guard-~resource_guard<ty, dx>)  |  |
| [release](#resource_guard-release)  |  |
| [get](#resource_guard-get) *(2 overloads)*  |  |
| [get_deleter](#resource_guard-get_deleter) *(2 overloads)*  |  |

---

### `resource_guard<Ty, Dx>` {#resource_guard-resource_guard<ty, dx>}

#### Overload 1

```cpp
void resource_guard<Ty, Dx>() = default;
```

`= default`

#### Overload 2

```cpp
void resource_guard<Ty, Dx>(const resource_guard<Ty, Dx> &) = delete;
```

`= delete`

#### Overload 3

```cpp
void resource_guard<Ty, Dx>(resource_guard<Ty, Dx> &&) = delete;
```

`= delete`

#### Overload 4

```cpp
void resource_guard<Ty, Dx>(std::nullptr_t) = delete;
```

`= delete`

#### Overload 5

```cpp
void resource_guard<Ty, Dx>(pointer data);
```

#### Overload 6

```cpp
void resource_guard<Ty, Dx>(pointer data, Dx deleter);
```

### `operator=` {#resource_guard-operator=}

#### Overload 1

```cpp
resource_guard<Ty, Dx> & operator=(const resource_guard<Ty, Dx> &) = delete;
```

`= delete`

#### Overload 2

```cpp
resource_guard<Ty, Dx> & operator=(resource_guard<Ty, Dx> &&) = delete;
```

`= delete`

### `~resource_guard<Ty, Dx>` {#resource_guard-~resource_guard<ty, dx>}

```cpp
void ~resource_guard<Ty, Dx>();
```

### `release` {#resource_guard-release}

```cpp
pointer release();
```

### `get` {#resource_guard-get}

#### Overload 1

```cpp
pointer get();
```

#### Overload 2

```cpp
const_pointer get() const;
```

`const`

### `get_deleter` {#resource_guard-get_deleter}

#### Overload 1

```cpp
auto & get_deleter() noexcept;
```

`noexcept`

#### Overload 2

```cpp
const auto & get_deleter() const noexcept;
```

`const` `noexcept`

## class `scope_guard`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `F` |  |

### Member Functions

| Function | Description |
|----------|-------------|
| [scope_guard<F>](#scope_guard-scope_guard<f>) *(3 overloads)*  |  |
| [operator=](#scope_guard-operator=)  |  |
| [~scope_guard<F>](#scope_guard-~scope_guard<f>)  |  |
| [dismiss](#scope_guard-dismiss)  |  |

---

### `scope_guard<F>` {#scope_guard-scope_guard<f>}

#### Overload 1

```cpp
void scope_guard<F>(F && f);
```

#### Overload 2

```cpp
void scope_guard<F>(const scope_guard<F> &) = delete;
```

`= delete`

#### Overload 3

```cpp
void scope_guard<F>(scope_guard<F> && other) noexcept;
```

`noexcept`

### `operator=` {#scope_guard-operator=}

```cpp
scope_guard<F> & operator=(const scope_guard<F> &) = delete;
```

`= delete`

### `~scope_guard<F>` {#scope_guard-~scope_guard<f>}

```cpp
void ~scope_guard<F>();
```

### `dismiss` {#scope_guard-dismiss}

```cpp
void dismiss() noexcept;
```

`noexcept`

## class `hash`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `key` | The type to compute hash for |

Primary template for hash function object. Provides hash computation for various types.  哈希函数对象的主模板。 为各种类型提供哈希计算。 

要计算哈希的类型

**Inherits:** `public` implements::hash_enable_if<key, !type_traits::type_properties::is_const_v<key> && !type_traits::type_properties::is_volatile_v<key> && (type_traits::primary_types::is_enum_v<key> || type_traits::primary_types::is_integral_v<key> || type_traits::primary_types::is_pointer_v<key>)>

### Member Functions

| Function | Description |
|----------|-------------|
| [hash_this_val](#hash-hash_this_val)  | Computes hash value for the given key. 计算给定键的哈希值。  |

---

### `hash_this_val` {#hash-hash_this_val}

```cpp
std::size_t hash_this_val(const key & keyval) noexcept;
```

`static` `noexcept`

Computes hash value for the given key. 计算给定键的哈希值。 

要哈希的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `keyval` | `const key &` | in | The value to hash |

**Returns**

Hash value 哈希值 

## struct `hash`

Specialization for float type. float 类型的特化。 

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#hash-operator())  | Computes hash value for float, treating -0.0f as 0.0f. 计算 float 的哈希值，将 -0.0f 视为 0.0f。  |

---

### `operator()` {#hash-operator()}

```cpp
result_type operator()(argument_type val) const;
```

`const`

Computes hash value for float, treating -0.0f as 0.0f. 计算 float 的哈希值，将 -0.0f 视为 0.0f。 

要哈希的 float 值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `argument_type` | in | The float value to hash |

**Returns**

Hash value 哈希值 

## struct `hash`

Specialization for double type. double 类型的特化。 

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#hash-operator())  | Computes hash value for double, treating -0.0 as 0.0. 计算 double 的哈希值，将 -0.0 视为 0.0。  |

---

### `operator()` {#hash-operator()}

```cpp
result_type operator()(argument_type val) const;
```

`const`

Computes hash value for double, treating -0.0 as 0.0. 计算 double 的哈希值，将 -0.0 视为 0.0。 

要哈希的 double 值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `argument_type` | in | The double value to hash |

**Returns**

Hash value 哈希值 

## struct `hash`

Specialization for long double type. long double 类型的特化。 

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#hash-operator())  | Computes hash value for long double, treating -0.0L as 0.0L. 计算 long double 的哈希值，将 -0.0L 视为 0.0L。  |

---

### `operator()` {#hash-operator()}

```cpp
result_type operator()(argument_type val) const;
```

`const`

Computes hash value for long double, treating -0.0L as 0.0L. 计算 long double 的哈希值，将 -0.0L 视为 0.0L。 

要哈希的 long double 值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `argument_type` | in | The long double value to hash |

**Returns**

Hash value 哈希值 

## struct `hash`

Specialization for nullptr_t type. nullptr_t 类型的特化。 

### Member Functions

| Function | Description |
|----------|-------------|
| [hash_this_val](#hash-hash_this_val)  | Computes hash value for nullptr. 计算 nullptr 的哈希值。  |
| [operator()](#hash-operator())  | Function call operator for nullptr. nullptr 的函数调用运算符。  |

---

### `hash_this_val` {#hash-hash_this_val}

```cpp
std::size_t hash_this_val(std::nullptr_t) noexcept;
```

`static` `noexcept`

Computes hash value for nullptr. 计算 nullptr 的哈希值。 

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `nullptr_t` | `std::nullptr_t` | in |  |

**Returns**

Hash value (hash of null pointer) 哈希值（空指针的哈希） 

### `operator()` {#hash-operator()}

```cpp
result_type operator()(std::nullptr_t) const;
```

`const`

Function call operator for nullptr. nullptr 的函数调用运算符。 

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `nullptr_t` | `std::nullptr_t` | in |  |

**Returns**

Hash value (hash of null pointer) 哈希值（空指针的哈希） 

## class `hash`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `CharType` | Character type |
| `Traits` | String traits type |

Specialization for basic_string_view types. basic_string_view 类型的特化。 

字符类型 字符串特性类型

### Member Functions

| Function | Description |
|----------|-------------|
| [hash_this_val](#hash-hash_this_val)  | Computes hash value for a string view. 计算字符串视图的哈希值。  |
| [operator()](#hash-operator())  | Function call operator for string view. 字符串视图的函数调用运算符。  |

---

### `hash_this_val` {#hash-hash_this_val}

```cpp
std::size_t hash_this_val(const argument_type & val) noexcept;
```

`static` `noexcept`

Computes hash value for a string view. 计算字符串视图的哈希值。 

要哈希的字符串视图

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `const argument_type &` | in | The string view to hash |

**Returns**

Hash value based on the string's characters 基于字符串字符的哈希值 

### `operator()` {#hash-operator()}

```cpp
result_type operator()(argument_type val) const;
```

`const`

Function call operator for string view. 字符串视图的函数调用运算符。 

要哈希的字符串视图

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `argument_type` | in | The string view to hash |

**Returns**

Hash value 哈希值 

## class `hash`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `CharType` | Character type |
| `Traits` | String traits type |
| `Alloc` | Allocator type |

Specialization for basic_string types. basic_string 类型的特化。 

字符类型 字符串特性类型 分配器类型

### Member Functions

| Function | Description |
|----------|-------------|
| [hash_this_val](#hash-hash_this_val)  | Computes hash value for a string. 计算字符串的哈希值。  |
| [operator()](#hash-operator())  | Function call operator for string. 字符串的函数调用运算符。  |

---

### `hash_this_val` {#hash-hash_this_val}

```cpp
std::size_t hash_this_val(const argument_type & val) noexcept;
```

`static` `noexcept`

Computes hash value for a string. 计算字符串的哈希值。 

要哈希的字符串

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `const argument_type &` | in | The string to hash |

**Returns**

Hash value based on the string's characters 基于字符串字符的哈希值 

### `operator()` {#hash-operator()}

```cpp
result_type operator()(const argument_type & val) const;
```

`const`

Function call operator for string. 字符串的函数调用运算符。 

要哈希的字符串

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `val` | `const argument_type &` | in | The string to hash |

**Returns**

Hash value 哈希值 

## class `is_support_standard_hasher_available`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The type to check |
| `` |  |

Type trait to check if standard hasher is available for a type. 检查类型的标准哈希器是否可用的类型特性。 

要检查的类型

**Inherits:** `public` type_traits::helper::false_type

---

## class `is_support_standard_hasher_available`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The type to check |

Specialization that detects if std::hash<Ty> is callable. 检测 std::hash<Ty> 是否可调用的特化。 

要检查的类型

**Inherits:** `public` type_traits::helper::true_type

---

## class `is_support_rainytoolkit_hasher_available`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The type to check |
| `` |  |

Type trait to check if Rainy Toolkit hasher is available for a type. 检查类型的 Rainy Toolkit 哈希器是否可用的类型特性。 

要检查的类型

**Inherits:** `public` type_traits::helper::false_type

---

## class `is_support_rainytoolkit_hasher_available`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The type to check |

Specialization that detects if rainy::utility::hash<Ty> is callable. 检测 rainy::utility::hash<Ty> 是否可调用的特化。 

要检查的类型

**Inherits:** `public` type_traits::helper::true_type

---

## struct `monostate`

Empty monostate type for use in variants and similar contexts. 用于变体等场景的空monostate类型。 

---

## class `hash_enable_if`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `key` |  |
| `check` | 如果为真，此模板将启用 |

### Member Functions

| Function | Description |
|----------|-------------|
| [operator()](#hash_enable_if-operator())  |  |

---

### `operator()` {#hash_enable_if-operator()}

```cpp
result_type operator()(const argument_type & val) const;
```

`const`

## class `hash_enable_if`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `key` |  |

### Member Functions

| Function | Description |
|----------|-------------|
| [hash_enable_if<key, false>](#hash_enable_if-hash_enable_if<key, false>) *(3 overloads)*  |  |
| [operator=](#hash_enable_if-operator=) *(2 overloads)*  |  |

---

### `hash_enable_if<key, false>` {#hash_enable_if-hash_enable_if<key, false>}

#### Overload 1

```cpp
void hash_enable_if<key, false>() = delete;
```

`= delete`

#### Overload 2

```cpp
void hash_enable_if<key, false>(const hash_enable_if<key, false> &) = delete;
```

`= delete`

#### Overload 3

```cpp
void hash_enable_if<key, false>(hash_enable_if<key, false> &&) = delete;
```

`= delete`

### `operator=` {#hash_enable_if-operator=}

#### Overload 1

```cpp
hash_enable_if<key, false> & operator=(const hash_enable_if<key, false> &) = delete;
```

`= delete`

#### Overload 2

```cpp
hash_enable_if<key, false> & operator=(hash_enable_if<key, false> &&) = delete;
```

`= delete`

## class `default_deleter`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The type of object to delete (non-array version) |

Default deleter for unique_ptr and similar smart pointers. Uses delete operator for single objects.  用于 unique_ptr 等智能指针的默认删除器。 对单个对象使用 delete 操作符。 

要删除的对象类型（非数组版本）

### Member Functions

| Function | Description |
|----------|-------------|
| [default_deleter<Ty>](#default_deleter-default_deleter<ty>) *(2 overloads)*  |  |
| [operator()](#default_deleter-operator())  | Function call operator that deletes the pointer. 删除指针的函数调用运算符。  |

---

### `default_deleter<Ty>` {#default_deleter-default_deleter<ty>}

#### Overload 1

```cpp
void default_deleter<Ty>() noexcept = default;
```

`noexcept` `= default`

#### Overload 2

```cpp
void default_deleter<Ty>(utility::placeholder_t) noexcept;
```

`noexcept`

Constructs a default_deleter from a placeholder. 从占位符构造 default_deleter。 

占位符值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `placeholder_t` | `utility::placeholder_t` | in | placeholder value |

### `operator()` {#default_deleter-operator()}

```cpp
void operator()(const Ty * resource) const noexcept;
```

`const` `noexcept`

Function call operator that deletes the pointer. 删除指针的函数调用运算符。 

要删除的指针

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `resource` | `const Ty *` | in | Pointer to delete |

## class `default_deleter`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The element type of the array |

Default deleter specialization for array types. Uses delete[] operator for arrays.  数组类型的默认删除器特化。 对数组使用 delete[] 操作符。 

数组的元素类型

### Member Functions

| Function | Description |
|----------|-------------|
| [default_deleter<Ty[]>](#default_deleter-default_deleter<ty[]>) *(2 overloads)*  |  |

---

### `default_deleter<Ty[]>` {#default_deleter-default_deleter<ty[]>}

#### Overload 1

```cpp
void default_deleter<Ty[]>() = default;
```

`= default`

#### Overload 2

```cpp
void default_deleter<Ty[]>(utility::placeholder_t) noexcept;
```

`noexcept`

Constructs a default_deleter from a placeholder. 从占位符构造 default_deleter。 

占位符值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `placeholder_t` | `utility::placeholder_t` | in | placeholder value |

## class `no_delete`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The type of object (non-array version) |

No-op deleter that does nothing. Useful for smart pointers that don't own the resource.  空操作删除器，什么都不做。 用于不拥有资源的智能指针。 

对象类型（非数组版本）

### Member Functions

| Function | Description |
|----------|-------------|
| [no_delete<Ty>](#no_delete-no_delete<ty>) *(2 overloads)*  |  |
| [operator()](#no_delete-operator())  | Function call operator that does nothing. 什么都不做的函数调用运算符。  |

---

### `no_delete<Ty>` {#no_delete-no_delete<ty>}

#### Overload 1

```cpp
void no_delete<Ty>() noexcept = default;
```

`noexcept` `= default`

#### Overload 2

```cpp
void no_delete<Ty>(utility::placeholder_t) noexcept;
```

`noexcept`

Constructs a no_delete from a placeholder. 从占位符构造 no_delete。 

占位符值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `placeholder_t` | `utility::placeholder_t` | in | placeholder value |

### `operator()` {#no_delete-operator()}

```cpp
void operator()(const Ty *) const noexcept;
```

`const` `noexcept`

Function call operator that does nothing. 什么都不做的函数调用运算符。 

指针（忽略）

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `resource` | `const Ty *` | in | Pointer (ignored) |

## class `no_delete`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Ty` | The element type of the array |

No-op deleter specialization for array types. Does nothing with the pointer.  数组类型的空操作删除器特化。 对指针不进行任何操作。 

数组的元素类型

### Member Functions

| Function | Description |
|----------|-------------|
| [no_delete<Ty[]>](#no_delete-no_delete<ty[]>) *(2 overloads)*  |  |

---

### `no_delete<Ty[]>` {#no_delete-no_delete<ty[]>}

#### Overload 1

```cpp
void no_delete<Ty[]>() = default;
```

`= default`

#### Overload 2

```cpp
void no_delete<Ty[]>(utility::placeholder_t) noexcept;
```

`noexcept`

Constructs a no_delete from a placeholder. 从占位符构造 no_delete。 

占位符值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `placeholder_t` | `utility::placeholder_t` | in | placeholder value |

## class `iterator_range`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Iter` | The underlying iterator type |

A range view over an iterator pair. 基于迭代器对的区间视图。  This class provides a view over a range defined by a pair of iterators, inheriting all functionality from adapter_iterator_range.  此类提供由一对迭代器定义的区间的视图， 继承自 adapter_iterator_range 的所有功能。 

底层迭代器类型

**Inherits:** `public` implements::adapter_iterator_range<Iter>

### Member Functions

| Function | Description |
|----------|-------------|
| [iterator_range<Iter>](#iterator_range-iterator_range<iter>)  | Constructs an iterator_range from begin and end iterators. 从开始和结束迭代器构造 iterator_range。  |

---

### `iterator_range<Iter>` {#iterator_range-iterator_range<iter>}

```cpp
void iterator_range<Iter>(Iter begin, Iter end);
```

Constructs an iterator_range from begin and end iterators. 从开始和结束迭代器构造 iterator_range。 

指向范围起始的迭代器 指向范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `begin` | `Iter` | in | Iterator to the beginning of the range |
| `end` | `Iter` | in | Iterator to the end of the range |

## class `iterator_range_iterator`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Iter` |  |

### Member Functions

| Function | Description |
|----------|-------------|
| [iterator_range_iterator<Iter>](#iterator_range_iterator-iterator_range_iterator<iter>)  | Constructs an iterator_range_iterator from an underlying iterator. 从底层迭代器构造iterator_range_iterator。  |
| [operator*](#iterator_range_iterator-operator*)  | Dereference operator. 解引用运算符。  |
| [operator->](#iterator_range_iterator-operator->)  | Arrow operator. 箭头运算符。  |
| [operator++](#iterator_range_iterator-operator++) *(2 overloads)*  | Prefix increment operator. 前置自增运算符。  |
| [operator--](#iterator_range_iterator-operator--) *(2 overloads)*  | Prefix decrement operator. 前置自减运算符。  |
| [operator+](#iterator_range_iterator-operator+)  | Addition operator. 加法运算符。  |
| [operator-](#iterator_range_iterator-operator-) *(2 overloads)*  | Subtraction operator. 减法运算符。  |
| [operator+=](#iterator_range_iterator-operator+=)  | Addition assignment operator. 加法赋值运算符。  |
| [operator-=](#iterator_range_iterator-operator-=)  | Subtraction assignment operator. 减法赋值运算符。  |

---

### `iterator_range_iterator<Iter>` {#iterator_range_iterator-iterator_range_iterator<iter>}

```cpp
void iterator_range_iterator<Iter>(Iter iter);
```

Constructs an iterator_range_iterator from an underlying iterator. 从底层迭代器构造iterator_range_iterator。 

要包装的底层迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `iter` | `Iter` | in | The underlying iterator to wrap |

### `operator*` {#iterator_range_iterator-operator*}

```cpp
decltype(auto) operator*() const;
```

`const`

Dereference operator. 解引用运算符。 

**Returns**

Reference to the element pointed to by the underlying iterator 底层迭代器所指向元素的引用 

### `operator->` {#iterator_range_iterator-operator->}

```cpp
pointer operator->() const;
```

`const`

Arrow operator. 箭头运算符。 

**Returns**

Pointer to the element pointed to by the underlying iterator 指向底层迭代器所指向元素的指针 

### `operator++` {#iterator_range_iterator-operator++}

#### Overload 1

```cpp
iterator_range_iterator<Iter> & operator++();
```

Prefix increment operator. 前置自增运算符。 

**Returns**

Reference to this iterator after increment 自增后此迭代器的引用 

#### Overload 2

```cpp
auto operator++(int);
```

Postfix increment operator. 后置自增运算符。 

**Returns**

Copy of this iterator before increment 自增前此迭代器的副本 

### `operator--` {#iterator_range_iterator-operator--}

#### Overload 1

```cpp
auto operator--();
```

Prefix decrement operator. 前置自减运算符。 

**Returns**

Reference to this iterator after decrement 自减后此迭代器的引用 

#### Overload 2

```cpp
auto operator--(int);
```

Postfix decrement operator. 后置自减运算符。 

**Returns**

Copy of this iterator before decrement 自减前此迭代器的副本 

### `operator+` {#iterator_range_iterator-operator+}

```cpp
iterator_range_iterator<Iter> operator+(difference_type n) const;
```

`const`

Addition operator. 加法运算符。 

前进的位置数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `n` | `difference_type` | in | Number of positions to advance |

**Returns**

New iterator advanced by n positions 前进n个位置后的新迭代器 

### `operator-` {#iterator_range_iterator-operator-}

#### Overload 1

```cpp
iterator_range_iterator<Iter> operator-(difference_type n) const;
```

`const`

Subtraction operator. 减法运算符。 

后退的位置数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `n` | `difference_type` | in | Number of positions to move back |

**Returns**

New iterator moved back by n positions 后退n个位置后的新迭代器 

#### Overload 2

```cpp
difference_type operator-(const iterator_range_iterator<Iter> & other) const;
```

`const`

Difference operator. 差运算符。 

要比较的另一个迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `other` | `const iterator_range_iterator<Iter> &` | in | Another iterator to compare with |

**Returns**

Distance between this iterator and other 此迭代器与other之间的距离 

### `operator+=` {#iterator_range_iterator-operator+=}

```cpp
auto & operator+=(difference_type n);
```

Addition assignment operator. 加法赋值运算符。 

前进的位置数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `n` | `difference_type` | in | Number of positions to advance |

**Returns**

Reference to this iterator after advancement 前进后此迭代器的引用 

### `operator-=` {#iterator_range_iterator-operator-=}

```cpp
auto & operator-=(difference_type n);
```

Subtraction assignment operator. 减法赋值运算符。 

后退的位置数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `n` | `difference_type` | in | Number of positions to move back |

**Returns**

Reference to this iterator after moving back 后退后此迭代器的引用 

## class `adapter_iterator_range`

**Template parameters**

| Parameter | Description |
|-----------|-------------|
| `Iter` |  |

**Inherits:** `public` views::view_interface<adapter_iterator_range<Iter> >

### Member Functions

| Function | Description |
|----------|-------------|
| [adapter_iterator_range<Iter>](#adapter_iterator_range-adapter_iterator_range<iter>) *(2 overloads)*  | Default constructor. 默认构造函数。  |
| [base](#adapter_iterator_range-base) *(2 overloads)*  | Returns a const reference to the base range. 返回基范围的常量引用。  |
| [begin](#adapter_iterator_range-begin) *(2 overloads)*  | Returns an iterator to the beginning. 返回指向起始的迭代器。  |
| [end](#adapter_iterator_range-end) *(2 overloads)*  | Returns an iterator to the end. 返回指向末尾的迭代器。  |
| [cbegin](#adapter_iterator_range-cbegin)  | Returns a const iterator to the beginning (explicit). 返回指向起始的常量迭代器（显式）。  |
| [cend](#adapter_iterator_range-cend)  | Returns a const iterator to the end (explicit). 返回指向末尾的常量迭代器（显式）。  |

---

### `adapter_iterator_range<Iter>` {#adapter_iterator_range-adapter_iterator_range<iter>}

#### Overload 1

```cpp
void adapter_iterator_range<Iter>();
```

Default constructor. 默认构造函数。 

#### Overload 2

```cpp
void adapter_iterator_range<Iter>(Iter begin, Iter end);
```

Constructs an adapter_iterator_range from begin and end iterators. 从开始和结束迭代器构造adapter_iterator_range。 

指向范围起始的迭代器 指向范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `end` | `Iter` | in | Iterator to the end of the range |
| `begin` | `Iter` | in | Iterator to the beginning of the range |

### `base` {#adapter_iterator_range-base}

#### Overload 1

```cpp
const adapter_iterator_range<Iter> & base() const noexcept;
```

`const` `noexcept`

Returns a const reference to the base range. 返回基范围的常量引用。 

**Returns**

Const reference to this adapter_iterator_range 此adapter_iterator_range的常量引用 

#### Overload 2

```cpp
adapter_iterator_range<Iter> base() noexcept;
```

`noexcept`

Returns an rvalue reference to the base range. 返回基范围的右值引用。 

**Returns**

Rvalue reference to this adapter_iterator_range 此adapter_iterator_range的右值引用 

### `begin` {#adapter_iterator_range-begin}

#### Overload 1

```cpp
iterator begin() noexcept;
```

`noexcept`

Returns an iterator to the beginning. 返回指向起始的迭代器。 

**Returns**

Iterator to the first element 指向第一个元素的迭代器 

#### Overload 2

```cpp
const_iterator begin() const noexcept;
```

`const` `noexcept`

Returns a const iterator to the beginning. 返回指向起始的常量迭代器。 

**Returns**

Const iterator to the first element 指向第一个元素的常量迭代器 

### `end` {#adapter_iterator_range-end}

#### Overload 1

```cpp
iterator end() noexcept;
```

`noexcept`

Returns an iterator to the end. 返回指向末尾的迭代器。 

**Returns**

Iterator to one past the last element 指向最后一个元素之后位置的迭代器 

#### Overload 2

```cpp
const_iterator end() const noexcept;
```

`const` `noexcept`

Returns a const iterator to the end. 返回指向末尾的常量迭代器。 

**Returns**

Const iterator to one past the last element 指向最后一个元素之后位置的常量迭代器 

### `cbegin` {#adapter_iterator_range-cbegin}

```cpp
const_iterator cbegin() const noexcept;
```

`const` `noexcept`

Returns a const iterator to the beginning (explicit). 返回指向起始的常量迭代器（显式）。 

**Returns**

Const iterator to the first element 指向第一个元素的常量迭代器 

### `cend` {#adapter_iterator_range-cend}

```cpp
const_iterator cend() const noexcept;
```

`const` `noexcept`

Returns a const iterator to the end (explicit). 返回指向末尾的常量迭代器（显式）。 

**Returns**

Const iterator to one past the last element 指向最后一个元素之后位置的常量迭代器 

## `get_temporary_buffer` {#get_temporary_buffer}

`noexcept`

Obtains a temporary buffer for a given number of elements. 获取用于指定数量元素的临时缓冲区。 

元素类型（必须是POD） 请求的元素数量

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `count` | `std::ptrdiff_t` | in | The requested number of elements |

**Returns:** A temporary_buffer instance managing the allocated memory 管理已分配内存的 temporary_buffer 实例 

## `return_temporary_buffer` {#return_temporary_buffer}

Explicitly returns a temporary buffer. 显式归还临时缓冲区。 

元素类型 要归还的缓冲区

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `buffer` | `temporary_buffer<Ty> &` | in | The buffer to return |

## `is_main_thread` {#is_main_thread}

`inline` `noexcept`

## `poly_call` {#poly_call}

Helper function to make polymorphic calls. 进行多态调用的辅助函数。 

要调用的成员函数索引 多态类型 参数类型 多态对象 要转发的参数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `args` | `Poly &&` | in | The arguments to forward |
| `self` | `Args &&...` | in | The polymorphic object |

**Returns:** The result of the function call 函数调用的结果 

## `swap_ranges` {#swap_ranges}

Swaps elements between two ranges. 交换两个范围之间的元素。 

第一个范围的前向迭代器类型 第二个范围的前向迭代器类型 指向第一个范围起始的迭代器 指向第一个范围末尾的迭代器 指向第二个范围起始的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first2` | `ForwardIt1` | in | Iterator to the beginning of the second range |
| `last1` | `ForwardIt1` | in | Iterator to the end of the first range |
| `first1` | `ForwardIt2` | in | Iterator to the beginning of the first range |

**Returns:** Iterator to the element past the last swapped element in the second range 指向第二个范围中最后一个被交换元素之后位置的迭代器 

## `fill` {#fill}

Assigns the given value to all elements in a range. 将给定值赋值给范围内的所有元素。 

迭代器类型 值类型 指向范围起始的迭代器 指向范围末尾的迭代器 要赋值的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `end` | `Iter` | in | Iterator to the end of the range |
| `value` | `const Ty &` | in | The value to assign |

## `fill_n` {#fill_n}

Assigns the given value to the first count elements in a range. 将给定值赋值给范围内的前count个元素。 

迭代器类型 大小类型（整型） 值类型 指向范围起始的迭代器 要填充的元素数量 要赋值的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `count` | `Iter` | in | Number of elements to fill |
| `value` | `Size` | in | The value to assign |
| `first` | `const Ty &` | in | Iterator to the beginning of the range |

**Returns:** Iterator one past the last element filled 指向最后一个被填充元素之后位置的迭代器 

## `all_of` {#all_of}

Checks if a predicate is true for all elements in a range. 检查谓词是否对范围内的所有元素都为真。 

迭代器类型 谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要应用的谓词

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `pred` | `Iter` | in | Predicate to apply |
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `last` | `Pred` | in | Iterator to the end of the range |

**Returns:** true if pred is true for all elements, false otherwise 如果所有元素都满足谓词则为true，否则为false 

## `any_of` {#any_of}

Checks if a predicate is true for any element in a range. 检查谓词是否对范围内的任意元素为真。 

迭代器类型 谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要应用的谓词

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `last` | `Iter` | in | Iterator to the end of the range |
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `pred` | `Pred` | in | Predicate to apply |

**Returns:** true if pred is true for any element, false otherwise 如果任意元素满足谓词则为true，否则为false 

## `none_of` {#none_of}

Checks if a predicate is true for no elements in a range. 检查谓词是否对范围内没有元素为真。 

迭代器类型 谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要应用的谓词

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `last` | `Iter` | in | Iterator to the end of the range |
| `pred` | `Pred` | in | Predicate to apply |

**Returns:** true if pred is false for all elements, false otherwise 如果所有元素都不满足谓词则为true，否则为false 

## `find` {#find}

Finds the first occurrence of a value in a range. 查找范围内第一次出现的值。 

迭代器类型 值类型 指向范围起始的迭代器 指向范围末尾的迭代器 要查找的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `value` | `Iter` | in | The value to find |
| `last` | `const Ty &` | in | Iterator to the end of the range |

**Returns:** Iterator to the first element equal to value, or last if not found 指向第一个等于value的元素的迭代器，如果未找到则返回last 

## `find_if` {#find_if}

Finds the first element satisfying a predicate. 查找第一个满足谓词的元素。 

迭代器类型 谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要应用的谓词

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `last` | `Iter` | in | Iterator to the end of the range |
| `pred` | `Pred` | in | Predicate to apply |

**Returns:** Iterator to the first element satisfying pred, or last if not found 指向第一个满足pred的元素的迭代器，如果未找到则返回last 

## `find_if_not` {#find_if_not}

Finds the first element not satisfying a predicate. 查找第一个不满足谓词的元素。 

迭代器类型 谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要应用的谓词

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `Iter` | in | Iterator to the beginning of the range |
| `pred` | `Iter` | in | Predicate to apply |
| `last` | `Pred` | in | Iterator to the end of the range |

**Returns:** Iterator to the first element not satisfying pred, or last if not found 指向第一个不满足pred的元素的迭代器，如果未找到则返回last 

## `equal` {#equal}

Checks if two ranges are equal. 检查两个范围是否相等。 

第一个范围的迭代器类型 第二个范围的迭代器类型 指向第一个范围起始的迭代器 指向第一个范围末尾的迭代器 指向第二个范围起始的迭代器 指向第二个范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first2` | `Iter1` | in | Iterator to the beginning of the second range |
| `first1` | `Iter1` | in | Iterator to the beginning of the first range |
| `last2` | `Iter2` | in | Iterator to the end of the second range |
| `last1` | `Iter2` | in | Iterator to the end of the first range |

**Returns:** true if the ranges are equal, false otherwise 如果范围相等则为true，否则为false 

## `lexicographical_compare` {#lexicographical_compare}

Lexicographically compares two ranges. 字典序比较两个范围。 

第一个范围的迭代器类型 第二个范围的迭代器类型 指向第一个范围起始的迭代器 指向第一个范围末尾的迭代器 指向第二个范围起始的迭代器 指向第二个范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `last2` | `Iter1` | in | Iterator to the end of the second range |
| `first1` | `Iter1` | in | Iterator to the beginning of the first range |
| `last1` | `Iter2` | in | Iterator to the end of the first range |
| `first2` | `Iter2` | in | Iterator to the beginning of the second range |

**Returns:** true if the first range is lexicographically less than the second, false otherwise 如果第一个范围字典序小于第二个范围则为true，否则为false 

## `lower_bound` {#lower_bound}

Returns an iterator to the first element not less than the given value. 返回指向第一个不小于给定值的元素的迭代器。 

前向迭代器类型 值类型 比较谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要比较的值 比较谓词（如果第一个参数小于第二个参数则返回true）

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `pred` | `ForwardIt` | in | Comparison predicate (returns true if first argument is less than second) |
| `first` | `ForwardIt` | in | Iterator to the beginning of the range |
| `value` | `const Ty &` | in | The value to compare against |
| `last` | `Pred` | in | Iterator to the end of the range |

**Returns:** Iterator to the first element not less than value, or last if not found 指向第一个不小于value的元素的迭代器，如果未找到则返回last 

## `upper_bound` {#upper_bound}

Returns an iterator to the first element greater than the given value. 返回指向第一个大于给定值的元素的迭代器。 

前向迭代器类型 值类型 比较谓词类型 指向范围起始的迭代器 指向范围末尾的迭代器 要比较的值 比较谓词（如果第一个参数小于第二个参数则返回true）

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `ForwardIt` | in | Iterator to the beginning of the range |
| `value` | `ForwardIt` | in | The value to compare against |
| `pred` | `const Ty &` | in | Comparison predicate (returns true if first argument is less than second) |
| `last` | `Pred` | in | Iterator to the end of the range |

**Returns:** Iterator to the first element greater than value, or last if not found 指向第一个大于value的元素的迭代器，如果未找到则返回last 

## `move` {#move}

Moves elements from one range to another. 将元素从一个范围移动到另一个范围。 

输入迭代器类型 输出迭代器类型 指向源范围起始的迭代器 指向源范围末尾的迭代器 指向目标范围起始的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `d_first` | `InputIt` | in | Iterator to the beginning of the destination range |
| `first` | `InputIt` | in | Iterator to the beginning of the source range |
| `last` | `OutputIt` | in | Iterator to the end of the source range |

**Returns:** Iterator to the element past the last moved element in the destination range 指向目标范围中最后一个被移动元素之后位置的迭代器 

## `move_backward` {#move_backward}

Moves elements from one range to another, starting from the end. 从末尾开始将元素从一个范围移动到另一个范围。 

源范围的双向迭代器类型 目标范围的双向迭代器类型 指向源范围起始的迭代器 指向源范围末尾的迭代器 指向目标范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `last` | `BidirIt1` | in | Iterator to the end of the source range |
| `d_last` | `BidirIt1` | in | Iterator to the end of the destination range |
| `first` | `BidirIt2` | in | Iterator to the beginning of the source range |

**Returns:** Iterator to the first moved element in the destination range 指向目标范围中第一个被移动元素的迭代器 

## `copy_backward` {#copy_backward}

Copies elements from one range to another, starting from the end. 从末尾开始将元素从一个范围复制到另一个范围。 

源范围的双向迭代器类型 目标范围的双向迭代器类型 指向源范围起始的迭代器 指向源范围末尾的迭代器 指向目标范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `last` | `BidirIt1` | in | Iterator to the end of the source range |
| `d_last` | `BidirIt1` | in | Iterator to the end of the destination range |
| `first` | `BidirIt2` | in | Iterator to the beginning of the source range |

**Returns:** Iterator to the first copied element in the destination range 指向目标范围中第一个被复制元素的迭代器 

## `uninitialized_move_backward` {#uninitialized_move_backward}

Moves elements from one range to an uninitialized range, starting from the end. 从末尾开始将元素从一个范围移动到未初始化的范围。 

输入迭代器类型 未初始化内存的输出迭代器类型 指向源范围起始的迭代器 指向源范围末尾的迭代器 指向目标范围末尾的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `d_last` | `InputIt` | in | Iterator to the end of the destination range |
| `first` | `InputIt` | in | Iterator to the beginning of the source range |
| `last` | `OutputIt` | in | Iterator to the end of the source range |

**Returns:** Iterator to the first moved element in the destination range 指向目标范围中第一个被移动元素的迭代器 

## `binary_search` {#binary_search}

Checks if a value exists in a sorted range using binary search. 使用二分查找检查值是否存在于已排序的范围中。 

前向迭代器类型 值类型 比较函数类型 指向已排序范围起始的迭代器 指向已排序范围末尾的迭代器 要搜索的值 比较函数（如果第一个参数小于第二个参数则返回true）

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `last` | `ForwardIt` | in | Iterator to the end of the sorted range |
| `first` | `ForwardIt` | in | Iterator to the beginning of the sorted range |
| `value` | `const Ty &` | in | The value to search for |
| `comp` | `Compare` | in | Comparison function (returns true if first argument is less than second) |

**Returns:** true if the value is found, false otherwise 如果找到该值则为true，否则为false 

## `binary_search` {#binary_search}

Checks if a value exists in a sorted range using binary search (default comparison). 使用二分查找检查值是否存在于已排序的范围中（默认比较）。 

前向迭代器类型 值类型 指向已排序范围起始的迭代器 指向已排序范围末尾的迭代器 要搜索的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `first` | `ForwardIt` | in | Iterator to the beginning of the sorted range |
| `value` | `ForwardIt` | in | The value to search for |
| `last` | `const Ty &` | in | Iterator to the end of the sorted range |

**Returns:** true if the value is found, false otherwise 如果找到该值则为true，否则为false 

## `copy` {#copy}

Copies elements from a range to another range. 将元素从一个范围复制到另一个范围。 

输入迭代器类型 输出迭代器类型 指向源范围起始的迭代器 指向源范围末尾的迭代器 指向目标范围起始的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `dest` | `InputIter` | in | Iterator to the beginning of the destination range |
| `begin` | `InputIter` | in | Iterator to the beginning of the source range |
| `end` | `OutIter` | in | Iterator to the end of the source range |

**Returns:** Iterator to the end of the destination range 指向目标范围末尾的迭代器 

## `copy_n` {#copy_n}

Copies exactly n elements from a range to another range. 从一个范围精确复制n个元素到另一个范围。 

输入迭代器类型 输出迭代器类型 指向源范围起始的迭代器 要复制的元素数量 指向目标范围起始的迭代器

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `begin` | `InputIter` | in | Iterator to the beginning of the source range |
| `count` | `const std::size_t` | in | Number of elements to copy |
| `dest` | `OutIter` | in | Iterator to the beginning of the destination range |

**Returns:** Iterator to the end of the destination range 指向目标范围末尾的迭代器 

## `transform` {#transform}

Applies a function to each element in a range and stores the results. 对范围内的每个元素应用函数并存储结果。 

输入迭代器类型 输出迭代器类型 一元函数类型 指向源范围起始的迭代器 指向源范围末尾的迭代器 指向目标范围起始的迭代器 应用于每个元素的函数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `begin` | `InputIter` | in | Iterator to the beginning of the source range |
| `end` | `InputIter` | in | Iterator to the end of the source range |
| `dest` | `OutIter` | in | Iterator to the beginning of the destination range |
| `func` | `Fx` | in | Function to apply to each element |

**Returns:** Iterator to the end of the destination range 指向目标范围末尾的迭代器 

## `transform` {#transform}

Applies a binary function to elements from two ranges and stores the results. 对两个范围的元素应用二元函数并存储结果。 

输入迭代器类型 输出迭代器类型 二元函数类型 指向第一个源范围起始的迭代器 指向第一个源范围末尾的迭代器 指向第二个源范围起始的迭代器 指向目标范围起始的迭代器 应用于每对元素的二元函数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `begin2` | `InputIter` | in | Iterator to the beginning of the second source range |
| `end1` | `InputIter` | in | Iterator to the end of the first source range |
| `begin1` | `InputIter` | in | Iterator to the beginning of the first source range |
| `func` | `OutIter` | in | Binary function to apply to each pair of elements |
| `dest` | `Fx` | in | Iterator to the beginning of the destination range |

**Returns:** Iterator to the end of the destination range 指向目标范围末尾的迭代器 

## `popcount` {#popcount}

`noexcept`

Counts the number of 1 bits in an unsigned integer. 计算无符号整数中1的位数。 

无符号整数类型 要计算位数的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `const Type` | in | The value to count bits for |

**Returns:** Number of 1 bits 1的位数 

## `has_single_bit` {#has_single_bit}

`noexcept`

Checks if a value has exactly one 1 bit (is a power of two). 检查一个值是否恰好有一个1位（是否为2的幂）。 

无符号整数类型 要检查的值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `const Type` | in | The value to check |

**Returns:** true if the value is a power of two, false otherwise 如果值是2的幂则为true，否则为false 

## `next_power_of_two` {#next_power_of_two}

`noexcept`

Returns the next power of two greater than or equal to the input. 返回大于或等于输入值的下一个2的幂。 

无符号整数类型 输入值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `const Type` | in | The input value |

**Returns:** The smallest power of two that is >= value 大于等于value的最小2的幂 

## `bit_width` {#bit_width}

`noexcept`

Returns the number of bits needed to represent the value. 返回表示该值所需的位数。 

无符号整数类型 输入值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `const Type` | in | The input value |

**Returns:** The bit width (position of the highest 1 bit + 1, or 0 for 0) 位宽（最高1位的位置+1，0返回0） 

## `bit_floor` {#bit_floor}

`noexcept`

Returns the largest power of two not greater than the value. 返回不大于该值的最大2的幂。 

无符号整数类型 输入值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `const Type` | in | The input value |

**Returns:** The largest power of two <= value, or 0 if value is 0 不大于value的最大2的幂，如果value为0则返回0 

## `countl_zero` {#countl_zero}

`noexcept`

Counts the number of consecutive 0 bits starting from the most significant bit. 从最高有效位开始计算连续0位的数量。 

无符号整数类型 输入值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `Type` | in | The input value |

**Returns:** Number of leading zero bits 前导零位的数量 

## `countr_zero` {#countr_zero}

`noexcept`

Counts the number of consecutive 0 bits starting from the least significant bit. 从最低有效位开始计算连续0位的数量。 

无符号整数类型 输入值

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `Type` | in | The input value |

**Returns:** Number of trailing zero bits 尾随零位的数量 

## `mod` {#mod}

`noexcept`

Computes value % mod when mod is a power of two. 当mod是2的幂时计算value % mod。 

无符号整数类型 要计算模的值 模数（必须是2的幂）

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `const Type` | in | The value to compute modulo for |
| `mod` | `const std::size_t` | in | The modulus (must be a power of two) |

**Returns:** value % mod value % mod的结果 

## `rotate_left` {#rotate_left}

`noexcept`

Rotates bits to the left. 向左旋转位。 

无符号整数类型 要旋转的值 旋转的位置数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `value` | `Type` | in | The value to rotate |
| `shift` | `int` | in | Number of positions to rotate |

**Returns:** Rotated value 旋转后的值 

## `rotate_right` {#rotate_right}

`noexcept`

Rotates bits to the right. 向右旋转位。 

无符号整数类型 要旋转的值 旋转的位置数

**Parameters**

| Name | Type | Direction | Description |
|------|------|-----------|-------------|
| `shift` | `Type` | in | Number of positions to rotate |
| `value` | `int` | in | The value to rotate |

**Returns:** Rotated value 旋转后的值 

## `is_pow_2` {#is_pow_2}

`inline` `noexcept`

## `get_size_of_n` {#get_size_of_n}

`noexcept`

## `in_range` {#in_range}

`noexcept`

## `stl_internal_check` {#stl_internal_check}

## `make_scope_guard` {#make_scope_guard}

## `fnv1a_append_bytes` {#fnv1a_append_bytes}

`inline` `noexcept`

## `fnv1a_append_value` {#fnv1a_append_value}

`noexcept`

## `fnv1a_append_range` {#fnv1a_append_range}

## `hash_representation` {#hash_representation}

`noexcept`

## `hash_array_representation` {#hash_array_representation}

`noexcept`

