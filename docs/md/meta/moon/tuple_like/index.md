# tuple_like.hpp


## 使用

### 头文件导入指令

```cpp
#include <rainy/meta/moon/tuple_like.hpp>
```

### 受影响的namespace

```cpp
namespace rainy::meta::moon;
namespace rainy::meta::moon::implements;
namespace rainy::utility;
namespace rainy::type_traits::primary_types;
```

## 成员

### 公共traits

| 名称         | 描述               |
| ------------ | ------------------ |
| member_count | 获取聚合           |
| tuple_size   | 与member_count等价 |

### 公共函数

| 名称                  | 描述                                  |
| --------------------- | ------------------------------------- |
| get_member_names      | 获取类型的所有成员的名称              |
| for_each              | 对指定类型的所有成员进行遍历          |
| get                   | 获取指定类型中对应索引的成员          |
| get_member_offset_arr | 获取指定类型中所有成员的偏移量数组    |
| visit_members         | 对指定类型的对象中的成员进行访问      |
| name_of               | 获取指定类型中指定索引对应的成员名称  |
| index_of              | 获取指定类型中指定名称对应的索引      |
| tuple_to_variant      | 将类型转换为variant，以用于运行时操作 |