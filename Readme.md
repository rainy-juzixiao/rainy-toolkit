# rainy's toolkit

## ���ܼ���

rainy's toolkit��һ������C++17�����ϼ���C++20�Լ����߰汾��C++����⡣����cmakeϵͳ��������Ϊ���ͳ��򹤳��ṩһ���Կ������顣
����߱�������Ҫ��ģ�鹦�ܣ�

1. type_traits��SFINAE�ķ�װ
2. ����ָ��
3. WindowsAPI��LinuxAPI�Ĳ���ϵͳ�㼶��װ
4. any��optional��span�Լ�variant�������ķ�װ
5. �����֧��
6. �㷨��֧��
7. �º�����֧��
8. ��־��֧��
9. Э��֧��
10. �첽֧�� 
11. �¼���֧��

ͬʱ��ͳһ��STL-like���������ģ�����Ʋ��ô��շ��������󲿷ֺ���������ȫ��д�����������������������ں��������������������ȣ�����snake_case������
���⣬�󲿷������ģ����ʽ������hpp��ʽ�ṩ��
�����ռ����Ƕ����ʽ�����Ұ��չ����ϸ񻮷֡�

### ��ʾDemo

#### �������ʾ

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
    y_field.set(&sample_object, 20); // ʹ��setter��������ֵ

    std::cout << x_field.get(&sample_object).as<int>() << "\n"; // ͨ����ȡany��ͨ�����ͻ�ԭ��ȡx
    std::cout << sample_object.x << "\n";

    auto &mult_method = instance->get_method<demo_class>(rainy::utility::variable_name<&demo_class::mult>());

    std::cout << mult_method.invoke_with_paramspack(&sample_object).as<int>() << "\n";
    // ����...
    std::cout << mult_method(&sample_object, {}).as<int>() << "\n";
    return 0;
}
```

�������:
```
&demo_class::x
&demo_class::y
10
10
200
200
```