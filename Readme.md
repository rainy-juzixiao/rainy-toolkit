# rainy's toolkit

《雨相集》
君曾见繁华盛世，与世之韶华共创新梦。
却曾见千人哀于悲愁思绪，疏于情之交集,
也曾见千人乐于造，与苦之人成对仗。
更曾见千人苦于常事，疏于天际。
或悲或哀，有时也或为乐。倘若雨天。

某刻，吾思美好年华，曾梦见倾盆之象。见豪情壮志之人，闯世间南北，唱浩瀚之梦，传光荣伟业。
也曾悟出，人生并非本性善，也非本性恶。中向自然，便无是非。
但上述之，吾更信、世间美好年华来于一丝空，一丝气。一丝声，一丝土，一丝墨，一丝雨。
但也罢，但曾有时，吾曾见古人云: “空山新雨后，天气晚来秋。”
致此言，吾信雨既喜悲之相遇，无雨寂寥。

可却何曾想浩雨乃壮志；愁雨乃思绪；细雨乃思情；相雨乃相遇。
方始，曾见未雨落下，无人问; 便寄思虚无，终想到，悲欢皆有其本意，世间也便有本意。于是在此云云：
“未雨落下，望世界之深远。信飘渺细雨如浩瀚之星。愿当下之愿，成春景于未来之新肆月。”

## 功能简述

rainy's toolkit也被称为雨相集库。雨相集库是一个基于C++17，向上兼容C++20以及更高版本的现代C++模板库。采用cmake系统管理。皆在为大型程序工程提供一次性开箱体验。
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
#include <rainy/meta/reflection.hpp>

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