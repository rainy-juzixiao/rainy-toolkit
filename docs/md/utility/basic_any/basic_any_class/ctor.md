## basic_any

### 1. `basic_any()`

用于构造一个空的basic_any对象

```cpp
basic_any() noexcept;
```

### 2. basic_any(basic_any&& right)/basic_any(const basic_any& right)

用于拷贝或移动function对象

```cpp
basic_any(basic_any &&right) noexcept;
basic_any(const basic_any &right) noexcept;
```

#### 参数

`right`: 待移动或拷贝的basic_any对象

### 3. `basic_any(ValueType&& value)`

用于构造一个使用值模式的basic_any对象，并使用value构造ValueType

```cpp
template <typename ValueType>
basic_any(ValueType&& value);
```

#### 参数

`value`: 用于构造ValueType的值。

#### 备注

该构造函数在满足以下条件时可用：

- 可复制，`is_copy_constructible_v<ValueType> == true`
- ValueType不为`reference`或`basic_any`类自己。
- ValueType不为`std::in_place_type`的特化，如`std::in_place_type<int>`
- 不为`basic_any`的任意实例化版本，`is_any_v<ValueType> == false`

### 4. `basic_any(std::in_place_type<Type>, Args&&... args)`

用于构造一个持有Type类型的basic_any对象，并使用`args`去构造Type。

```cpp
template <typename Type, typename... Args>
basic_any(std::in_place_type<Type>, Args&&... args);
```

#### 参数

`std::in_place_type`:  作为第一个参数传入，其中使用一个类型进行实例化，标识对具体构造的类型，例如如下的参数均为有效的参数：

- `std::in_place_type<int>`
- `std::in_place_type<int&>`
- `std::in_place_type<std::string>`

`args...`: 被用于转发给具体的构造函数的参数。

#### 备注

该构造函数在满足以下条件时可用：

- 给定的参数对应的类型可构造Type，`is_construtible_v<Type, Args...> == true`
- ValueType不为`reference`或`basic_any`类自己。
- 不为`basic_any`的任意实例化版本，`is_any_v<ValueType> == false`

### 5. `basic_any(std::in_place_type<Type>, std::initializer_list<Elem>, Args&&... args)`

用于构造一个持有Type类型的basic_any对象，并使用初始化列表和`args`去构造Type。

```cpp
template <typename Type, typename Elem, typename... Args>
basic_any(std::in_place_type<Type>, std::initializer_list<Elem>, Args&&... args);
```

#### 参数

`std::in_place_type`:  作为第一个参数传入，其中使用一个类型进行实例化，标识对具体构造的类型，例如如下的参数均为有效的参数：

- `std::in_place_type<int>`
- `std::in_place_type<int&>`
- `std::in_place_type<std::string>`

`ilist`: 用于构造Ty的初始化列表。
`args...`: 被用于转发给具体的构造函数的参数。

#### 备注

该构造函数在满足以下条件时可用：

- 给定的参数对应的类型可构造Type，`is_construtible_v<Type, std::initializer_list<Elem>, Args...> == true`
- ValueType不为`reference`或`basic_any`类自己。
- 不为`basic_any`的任意实例化版本，`is_any_v<ValueType> == false`

### 6. basic_any(reference right)

从reference中取出实例并构造新的basic_any对象，以值模式。

### 另注

`reference`是指`basic_any`的引用类包装器。`reference`不是C++的`原生引用`类型，即`int&`、`int&&`这类。因此，请严格区分。若你需要存储引用，请直接使用`std::in_place_type`指明引用类型即可。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <string>
#include <rainy/utility/any.hpp>

using namespace rainy::utility;

int main() {
    basic_any a1; // construct a empty basic_any
    assert(a1.has_value() == false);
    any a2 = 42; // use value to construct basic_any
    assert(a2.has_value() == true);
    assert(a2.as<int>() == 42);
    basic_any a3(std::in_place_type<std::string>, "Hello World!");
    assert(a3.has_value() == true);
    assert(a3.as<std::string>() == "Hello World!");
    basic_any a4(std::in_place_type<std::vector<int>>, {1, 2, 3, 4, 5});
    assert(a4.has_value() == true);
    auto vec = a4.as<std::vector<int>>();
    assert(vec.size() == 5 && vec[0] == 1 && vec[4] == 5);
    return 0;
}
```

<!--@include: @/static/example_assertion.md-->