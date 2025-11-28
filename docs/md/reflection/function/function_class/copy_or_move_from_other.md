## copy_from_other {#copy_from_other}

拷贝一个function对象存储的函数指针到此function对象中。

```cpp
void copy_from_other(const function &right) noexcept;
```

### 参数

`right`: 待拷贝的对象
## move_from_other {#move_from_other}

将function对象存储的函数指针转移到此function对象中。

```cpp
void move_from_other(function &&right) noexcept;
```

### 参数

`right`: 待移动的对象