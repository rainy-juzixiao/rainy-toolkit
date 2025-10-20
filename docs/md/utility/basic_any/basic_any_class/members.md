## 成员

basic_any类定义了如下成员

| 成员函数                                      | 说明                                           |
| ----------------------------------------- | -------------------------------------------- |
| [basic_any](#basic_any)                   | 用于构造basic_any对象                              |
| [emplace](#emplace)                       | 就地对basic_any构造一个新的对象                         |
| [reset](#reset)                           | 重置basic_any所有权                               |
| [swap](#swap)                             | 将basic_any对象与另一个basic_any对象的内容进行交换           |
| [has_value](#has_value)                   | 检查basic_any是否有值或引用                           |
| [has_ownership](#has_ownership)           | 检查basic_any是否持有一个对象的所有权，而非引用                 |
| [type](#type)                             | 获取basic_any持有内容的类型标识实例                       |
| [as](#as)                                 | 按照类型，返回basic_any持有的原类型的引用形式                  |
| [cast_to_pointer](#cast_to_pointer)       | 按照类型，返回basic_any持有的原类型的指针形式                  |
| [transform](#transform)                   | 将basic_any内部的内容进行变形                          |
| [convert](#transform)                     | 对basic_any执行类型转换                             |
| [is_convertible](#is_convertible)         | 检查any是否可以进行类型转换                              |
| [is](#is)                                 | 检查basic_any类型是否与目标类型一致                       |
| [is_one_of](#is_one_of)                   | 检查basic_any类型是否与给定的目标类型集合中的某个类型一致            |
| [target_as_void_ptr](#target_as_void_ptr) | 允许跳过类型检查，直接获取basic_any存储对象的地址或引用的二级指针地址      |
| [hash_code](#hash_code)                   | 获取basic_any内部对象的哈希值                          |
| [match](#match)                           | 对basic_any内部对象类型执行类型匹配                       |
| [match_for](#match_for)                   | 在match的基础上，对match返回的对象进行类型还原，以std::variant形式 |
| [destructure](#destructure)               | 尝试对basic_any内部类型进行解构                         |

|运算符|说明|
|-|-|
|[basic_any::operator[]](#basic-any-index-operator)|尝试对basic_any执行索引访问|
|[basic_any::operator++](#basic-any-self-incr-operator)|尝试对basic_any对象进行自增|
|[basic_any::operator--](#basic-any-self-decr-operator)|尝试对basic_any对象进行自减|
|[basic_any::operator+=](#basic-any-add-assign-operator)|尝试对basic_any对象执行加等于操作|
|[basic_any::operator-=](#basic-any-sub-assign-operator)|尝试对basic_any对象执行减等于操作|
|[basic_any::operator*=](#basic-any-mul-assign-operator)|尝试对basic_any对象执行乘等于操作|
|[basic_any::operator/=](#basic-any-div-assign-operator)|尝试对basic_any对象执行除等于操作|
|[basic_any::operator=](#basic-any-assign-operator)|拷贝basic_any对象或修改basic_any对象|