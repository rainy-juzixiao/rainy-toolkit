## 用法

basic_any类是一个类模板，声明如下。

```cpp
template <std::size_t Length, std::size_t Align>
class basic_any;
```

`Length`: 指定any小对象的缓冲区长度（一般留空，默认为rainy::core::small_object_space_size - (sizeof(void *) * 2)）

`Align`: 指定any小对象的对齐长度（一般留空，默认为alignof(std::max_align_t)）