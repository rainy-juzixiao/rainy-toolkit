## accumulate

计算给定范围内所有元素与初始值的累计结果。

**语法**:

```cpp
template <typename InputIt, typename Ty>
RAINY_INLINE constexpr rain_fn accumulate(
    InputIt first,
    InputIt last,
    Ty init
) -> Ty; // (1)

template <typename InputIt, typename Ty, typename BinaryOperation>
RAINY_INLINE constexpr rain_fn accumulate(
    InputIt first,
    InputIt last,
    Ty init,
    BinaryOperation op
) -> Ty; // (2)
```

**(1)**

计算指定范围 `[first, last)` 内所有元素的累加值，从初始值 `init` 开始。默认使用加法运算符 (`+`) 进行累加。

**类型参数**
- `InputIt`: 输入迭代器类型，必须满足 InputIterator 的要求。
- `Ty`: 累加结果的类型，也是初始值 `init` 的类型。

**参数**
- `first`: 范围起始的输入迭代器。
- `last`: 范围结束的输入迭代器（不包含）。
- `init`: 累加的初始值。

**备注**
- 此函数执行的操作等价于 `std::accumulate`。
- 保证为 `constexpr`。

**(2)**

计算指定范围 `[first, last)` 内所有元素的累加值，从初始值 `init` 开始。使用自定义的二元操作 `op` 代替默认的加法运算符 (`+`) 进行累加。

**类型参数**
- `InputIt`: 输入迭代器类型，必须满足 InputIterator 的要求。
- `Ty`: 累加结果的类型，也是初始值 `init` 的类型。
- `BinaryOperation`: 用于累加的二元操作类型。它必须可使用两个参数调用：第一个参数为累加值（类型为 `Ty`），第二个参数为输入范围中的元素值（类型为 `InputIt` 的解引用类型）。

**参数**
- `first`: 范围起始的输入迭代器。
- `last`: 范围结束的输入迭代器（不包含）。
- `init`: 累加的初始值。
- `op`: 用于累加的二元操作函数对象。

**备注**
- 此函数执行的操作等价于 `std::accumulate`。
- 保证为 `constexpr`。