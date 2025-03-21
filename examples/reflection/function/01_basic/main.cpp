#include <vector>
#include <iostream>
#include <string>
#include <rainy/meta/reflection/function.hpp>
#include "test.h"

class MyClass {
public:
    int add(int a, int b) {
        ++field;
        return a + b;
    }

    int multiply(int a, int b) const {
        ++field;
        return a * b;
    }

    static int static_add(int a, int b) {
        return a + b;
    }

    static int static_multiply(int a, int b) {
        return a * b;
    }

    int subtract(int a, int b) volatile {
        ++field;
        return a - b;
    }

    static void print_hello(std::string_view name) {
        std::cout << "Hello, " << name << "!" << std::endl;
    }

    static void print_info(const std::string& info, int count) {
        std::cout << info << " " << count << " times!" << std::endl;
    }

    volatile mutable int field{0};
};

int multiply(int a, int b) {
    return a * b;
}

struct param {
    int field{};
    std::string s{};
};

int main() {
    using namespace rainy::meta::reflection;

    // 示例 1: 调用静态函数
    function static_func_obj(&MyClass::static_add);
    auto static_add_result = static_func_obj.invoke_static(10, 20);
    std::cout << "Result of static_add: " << static_add_result << std::endl; // 输出：30

    // 示例 2: 调用成员函数
    MyClass obj;
    function member_func_obj(&MyClass::add);
    auto member_result = member_func_obj.invoke_paramlist(obj, 3, 4);
    std::cout << "Result of add (member): " << member_result << std::endl; // 输出：7

    // 示例 3: 调用带有多个参数的静态和成员函数
    function static_multiply_func(&MyClass::static_multiply);
    auto static_multiply_result = static_multiply_func.invoke_static(5, 6);
    std::cout << "Result of static_multiply: " << static_multiply_result << std::endl; // 输出：30

    function member_multiply_func(&MyClass::multiply);
    auto member_multiply_result = member_multiply_func.invoke_paramlist(obj, 7, 8);
    std::cout << "Result of multiply (member): " << member_multiply_result << std::endl; // 输出：56

    // 示例 4: 调用可变参数函数
    function print_info_func(&MyClass::print_info);
    auto args = make_arguments(std::string("Printing info"), 5);
    auto print_info_result = print_info_func.invoke_static(args);
    // 输出：Printing info 5 times!

    // 示例 5: 检查函数的常量性、易变性
    function const_member_func(&MyClass::multiply);
    if (const_member_func.is_const()) {
        std::cout << "multiply is a const member function!" << std::endl; // 输出：multiply is a const member function!
    }

    function volatile_member_func(&MyClass::subtract);
    if (volatile_member_func.is_volatile()) {
        std::cout << "subtract is a volatile member function!" << std::endl; // 输出：subtract is a volatile member function!
    }

    // 示例 6: 获取目标函数
    auto target_function = static_func_obj.target<decltype(&MyClass::static_add)>();
    std::cout << "Target function address: " << &target_function << std::endl;

    // 示例 7: 异常处理
    try {
        function print_hello_func(&MyClass::print_hello);
        print_hello_func.invoke_static(std::string_view{"John"}.data());
        // 输出：Hello, John!
    } catch (const std::exception &e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    // 示例 8: 获取函数所属类类型、返回类型、参数类型
    // 获取静态函数的返回类型和参数类型
    const auto &static_return_type = static_func_obj.return_type();
    const auto &static_param_types = static_func_obj.param_lists();
    std::cout << "Static function return type: " << static_return_type.name() << std::endl;
    std::cout << "Static function parameter types: ";
    for (const auto &param_type: static_param_types) {
        std::cout << param_type.name() << " ";
    }
    std::cout << std::endl;
    // 获取成员函数的返回类型和参数类型
    const auto &member_return_type = member_func_obj.return_type();
    const auto &member_param_types = member_func_obj.param_lists();

    std::cout << "Member function return type: " << member_return_type.name() << std::endl;
    std::cout << "Member function parameter types: ";
    for (const auto &param_type: member_param_types) {
        std::cout << param_type.name() << " ";
    }
    std::cout << std::endl;
    // 获取函数所属类类型
    const auto &static_class_type = static_func_obj.which_belongs();
    std::cout << "Static function belongs to class: " << static_class_type.name() << std::endl;
    const auto &member_class_type = member_func_obj.which_belongs();
    std::cout << "Member function belongs to class: " << member_class_type.name() << std::endl;
    auto object = get_obj();
    auto test_f = get_fn();
    std::cout << object.rtti().name() << '\n';
    std::cout << test_f.function_signature().name() << '\n';
    test_f.invoke_paramlist(object);
    auto param = get_param();
    auto test_f1 = get_fn1();

    test_f1.oinvoke(object, param);
    rainy::containers::any a = param;
    std::cout << a.as<instance>().rtti().name() << '\n';
    std::cout << a.type().name() << '\n';
    return 0;
}
