## deduction_invoker_type

生成用于推导调用器类型的工具。

```cpp
template <typename Fx, typename... Args>
constexpr rain_fn deduction_invoker_type() noexcept -> method_flags;
```
#### 类型参数

- `Fx`: 需要推导的函数类型
- `Args`: 函数参数类型包
#### 备注

- 返回与给定函数类型和参数包匹配的调用器标志
- 用于编译期类型推导与特性判断