## function

### 1. `function()`/`function(std::nullptr_t)`

用于构造一个空的function对象

```cpp
function() noexcept;
function(std::nullptr_t) noexcept;
```

### 2. `function(function &&right)`/`function(const function &right)`

用于拷贝或移动function对象

```cpp
function(function &&right) noexcept;
function(const function &right) noexcept;
```

#### 参数

`right`: 待移动或拷贝的function对象

### 3. `function(Fx, Args...)`

通过传入可调用实体和可选的默认参数来构造function对象

```cpp
template <typename Fx>
function(Fx function, Args&&... default_arguments) noexcept;
```

#### 参数

`function`: 可支持构造的可调用函数，但是必须是*静态域函数、对象实例函数、仿函数*这类的。对于仿函数，仅支持无重载版本的`operator()`：即所属类中有且仅有一个，因此，bind表达式这类无法使用

`default_arguments...`：这部分是可选的，表示默认的追加参数。对于默认参数，function对象会在内部将参数类型处理为值。会将引用性、const/volatile属性全部抹除。另外，数组直接退化为指针。以及，参数是必须能转换到对应参数的。

#### 备注

对于仿函数，该构造函数将会进行类型推导，一般来说，若传入lambda表达式、函数对象。情况会稍稍不同，例如，带捕获的lambda表达式可能会触发内存分配操作来用于存储当前函数对象。不过，一般情况下，构造函数会尽可能使用function对象为其保留的一段SOO内存空间，这一内存空间大小是编译期决定的，通常大小为：`(rainy::core::small_object_num_ptrs * sizeof(void *)) + alignof(std::max_align_t)`。具体的值由编译平台决定。

另外，如果有默认参数的传入，则分配大小也会受到影响。（可能触发分配）

详细细节，请查阅本页中的内存策略页。