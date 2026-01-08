# tuple_like.hpp

## 使用

### 头文件导入指令

```cpp
#include <rainy/meta/moon/tuple_like.hpp>
```

### 受影响的 namespace

```cpp
namespace rainy::meta::moon;
namespace rainy::meta::moon::implements;
namespace rainy::utility;
namespace rainy::type_traits::primary_types;
```

## 成员

### 公共 traits

| 名称                                                             | 描述                 |
| ---------------------------------------------------------------- | -------------------- |
| [member_count](/md/meta/moon/tuple_like/pub_traits#member_count) | 获取聚合类的成员个数 |
| tuple_size                                                       | 与 member_count 等价 |

### 公共函数

| 名称                                                                | 描述                                   |
| ------------------------------------------------------------------- | -------------------------------------- |
| [get_member_names](/md/meta/moon/tuple_like/pubfn#get_member_names) | 尝试获取指定类型中所有成员的名称       |
| for_each                                                            | 对指定类型的所有成员进行遍历           |
| get                                                                 | 获取指定类型中对应索引的成员           |
| get_member_offset_arr                                               | 获取指定类型中所有成员的偏移量数组     |
| visit_members                                                       | 对指定类型的对象中的成员进行访问       |
| name_of                                                             | 获取指定类型中指定索引对应的成员名称   |
| index_of                                                            | 获取指定类型中指定名称对应的索引       |
| tuple_to_variant                                                    | 将类型转换为 variant，以用于运行时操作 |
