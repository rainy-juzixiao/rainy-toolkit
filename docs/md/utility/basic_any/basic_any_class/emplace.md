## emplace

就地对basic_any构造一个新的对象。

### 1. `decltype(auto) emplace<Ty, Types...>(Types &&...args)`

通过0到N个参数，用于构造一个类型为Ty的对象。

#### 类型参数

`Ty`: 指定要构造的对象类型
`Types...`: 由参数类型自动推导，可省略

#### 参数

`args...`: 用于构造T使用的参数。

#### 返回值

返回对新的已构造对象的引用。

#### 备注

仅当`is_constructible<Ty, Types...>`结果为`true`时，该函数才被允许调用。如果Ty为一个引用类型，即`is_reference_v<Ty>`为`true`时。basic_any将激活借用所有权模式。

### 2.`decltype(auto) emplace<Ty ,Elem, Types...>(std::initializer_list<Elem> ilist, Types &&...args)`

通过一个初始化列表和0到N个参数来构造类型为Ty的对象，通常用于构造容器。

#### 类型参数

`Ty`: 指定要构造的对象类型
`Elem`: 初始化列表的类型，由初始化列表自动推导
`Types...`: 由参数类型自动推导，可省略

#### 参数

`ilist`: 用于构造Ty的初始化列表。
`args...`: 用于构造T使用的参数。

#### 返回值

返回对新的已构造对象的引用。

#### 备注

仅当`is_constructible<Ty, std::initializer_list<Elem> &, Types...>`结果为`true`时，该函数才被允许调用。如果Ty为一个引用类型，basic_any将激活借用所有权模式。

<!--@include: @/static/example_code_header_cxx17.md-->

```cpp
#include <iostream>
#include <rainy/utility/any.hpp>

int main() {
	any a;
	a.emplace<int>(42); // a is int
	std::cout << a.type().name() << '\n'; // int
	int value{ 42 };
	a.emplace<int&>(value); // now a is int&
	std::cout << a.type().name() << '\n'; // int&
	return 0;
}
```

<!--@include: @/static/example_assertion.md-->