# rainy's toolkit

## 功能简述

rainy's toolkit是一个基于C++17，向上兼容C++20以及更高版本的C++程序库。采用cmake系统管理。皆在为大型程序工程提供一次性开箱体验。
本库具备下列主要的模块功能：

1. type_traits和SFINAE的封装
2. 智能指针
3. WindowsAPI与LinuxAPI的部分系统层级封装
4. any、optional、span以及variant等容器的封装
5. 反射库支持
6. 算法库支持
7. 仿函数库支持
8. 日志库支持
9. 协程支持
10. 异步支持 
11. 事件库支持

同时，统一以STL-like风格命名。模板名称采用大驼峰命名，大部分宏命名采用全大写，其它命名（包括但不限于函数名、类名、变量名等）采用snake_case命名。
此外，大部分类采用模板形式，并以hpp形式提供。
命名空间采用嵌套形式。并且按照功能严格划分。

### 演示Demo

#### 反射库演示

```cpp
#include <rainy/meta_programming/reflection.hpp>

struct demo_class {
    int x, y;

    int mult() {
        return x * y;
    }
};

int main() {
    auto* instance = rainy::foundation::reflection::lunar::instance();
    instance->register_class<demo_class>();
    instance->add_field<&demo_class::x>();
    instance->add_field<&demo_class::y>();
    instance->add_method<&demo_class::mult>();

    auto& x_field = instance->get_field<demo_class>(rainy::utility::variable_name<&demo_class::x>());
    auto& y_field = instance->get_field<demo_class>(rainy::utility::variable_name<&demo_class::y>());

    std::cout << x_field.name() << "\n";
    std::cout << y_field.name() << "\n";

    demo_class sample_object{};
    
    x_field.set(&sample_object, 10);
    y_field.set(&sample_object, 20); // 使用setter功能设置值

    std::cout << x_field.get(&sample_object).as<int>() << "\n"; // 通过获取any并通过类型还原获取x
    std::cout << sample_object.x << "\n";

    auto &mult_method = instance->get_method<demo_class>(rainy::utility::variable_name<&demo_class::mult>());

    std::cout << mult_method.invoke_with_paramspack(&sample_object).as<int>() << "\n";
    // 或者...
    std::cout << mult_method(&sample_object, {}).as<int>() << "\n";
    return 0;
}
```

输出如下:
```
&demo_class::x
&demo_class::y
10
10
200
200
```