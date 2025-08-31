## 内存策略

:::warning
此部分可能涉及抽象泄露，因此，此节被置于末节。用于描述function类的内存策略
:::

function类的内部采用了SOO内存空间与动态内存分配共存的内存策略。且，大量细节被封装。因此为了屏蔽不必要的抽象泄露，你无法直接操作function对象的内存，除非你在进行未定义行为或寻找漏洞。并且，你没有权限得知是否使用SOO或者动态分配的内存。这一行为，我们不推荐，且如果可能的情况下，永远不要做这种蠢事。（哪怕你可以通过预定义常量在编译时计算是否可能使用动态分配内存）

但是，本节会介绍function类的内存策略机制来帮助理解function何时使用SOO、何时使用动态分配内存。

function类的内存布局如下。

```cpp
alignas(std::max_align_t) rainy::core::byte_t invoker_storage[
    rainy::core::fn_obj_soo_buffer_size
]{};
invoker_accessor *invoke_accessor_{nullptr};
```

`invoker_storage`是function对象保留的SOO空间，而`invoke_accessor_`则为访问器。在构造function对象时，如果目标存储的所需大小是小于SOO空间大小的，则会将目标构造于`invoker_storage`以加快性能。其中，SOO空间大小被定义如下：

```cpp
static inline constexpr std::size_t small_object_num_ptrs = 4 + 16 / sizeof(void *);

static constexpr inline std::size_t fn_obj_soo_buffer_size = (core::small_object_num_ptrs * sizeof(void *)) + alignof(std::max_align_t);
```

若所需大小是大于SOO空间大小的，则会触发动态分配来容纳。另外，function类会自动管理内存。如果function对象持有的是动态分配的内存。则会在对象析构时自动释放。

另注，SOO空间是具体的值根据平台决定的。因此，请时刻参考编译器给出的实际大小，本文不进行展示。

关于对象大小，一般有如下因素构成。

- 函数目标
- 默认参数存储
- 内存对齐

因此，一个具有大对象且捕获了变量或常量的lambda表达式将容易导致function对象执行动态分配。例如

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
function f = {[vec](int iter, std::string mark) {
    std::cout << mark << " : ";
    for (int i = 0; i < iter; ++i)  {
        std::cout << vec[i] << ' ';
    }
    std::cout << '\n';
}, 6, std::string{"my_vector"}};
```

上述的定义中，f将会通过动态分配的内存构造对象。因为，接受的参数中拷贝了vec，且在默认参数中有int和std::string的传入。因此，总大小将会超过SOO大小。因此，若对象被频繁构造，尽可能使用引用捕获。在默认参数方面，若非必须，则应避免绑定过多默认参数，以防止突破SOO阈值。