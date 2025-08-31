## 成员

function类定义了如下成员

|成员函数|说明|
|-|-|
|[function](#function)|用于构造function对象|
|[static_invoke](#static_invoke)|向function对象中存储的函数实例，发起调用，以对象上下文函数或静态域函数方式|
|[invoke](#invoke)|向function对象中存储的函数实例，发起调用，以对象上下文函数或静态域又或对象实例函数方式|
|[return_type](#return_type)|获取当前function对象指向的返回值类型的ctti类型标识|
|[paramlists](#paramlists)|获取当前function对象的调用参数的类型标识列表|
|[arity](#arity)|获取调用当前function的所需参数数量|
|[arg](#arg)|获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息|
|[function_signature](#function_signature)|获取当前function对象的函数签名的ctti类型标识|
|[type](#type)|获取当前function对象的调用类型|
|[has](#has)|用于检索当前function对象是否具有特定的属性|
|[which_belongs](#which_belongs)|获取当前function对象中持有实例具体所属的类类型信息|
|[empty](#empty)|检查当前function对象是否为空|
|[is_static](#is_static)|检查当前function对象是否为静态域函数|
|[is_memfn](#is_memfn)|检查当前function对象是否为成员实例函数|
|[is_const](#is_const)|检查当前function对象是否为成员实例函数，且检查是否具有const属性|
|[is_volatile](#is_volatile)|检查当前function对象是否为成员实例函数，且检查是否具有volatile属性|
|[is_invocable](#is_invocable)|检查当前function对象是否可通过其提供的参数类型进行动态调用|
|[is_invocable_with](#is_invocable_with)|检查当前function对象是否可以通过其提供的参数进行动态调用|
|[is_noexcept](#is_noexcept)|检查当前function对象是否具有noexcept属性|
|[is_invoke_for_lvalue](#is_invoke_for_lvalue)|检查当前function对象是否是适用于左值引用的调用|
|[is_invoke_for_rvalue](#is_invoke_for_rvalue)|检查当前function对象是否是适用于右值引用的调用|
|[copy_from_other](#copy_from_other)|拷贝一个function对象存储的函数指针到此function对象中|
|[move_from_other](#move_from_other)|将function对象存储的函数指针转移到此function对象中|
|[swap](#swap)|将function对象的函数指针与当前function对象的函数指针进行交换|
|[reset](#reset)|将function对象进行状态重置|
|[rebind](#rebind)|为function对象重新绑定一个函数实例|
|[target](#target)|安全提取function对象中存储的函数指针|
|[equal_with](#equal_with)|检查一个function对象与当前的function对象是否一致|
|[not_equal_with](#not_equal_with)|检查一个function对象与当前的function对象是否为不同的|

|运算符|说明|
|-|-|
|[function::operator()](#function-invoke-operator)|提供仿函数语义调用invoke函数|
|[function::operator bool](#function-operator-bool)|检查当前对象是否有效|
|[function::operator=](#function-assign-operator)|拷贝function对象或修改function对象|