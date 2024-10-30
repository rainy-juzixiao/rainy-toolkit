/*
* 本例中，我们测试source_location模块
* source_location仅包含核心文件，
 */
#include <rainy/diagnostics/source_location.hpp>
#include <iostream>

using namespace rainy::foundation::diagnostics;

int main() {
    /* 基础测试 */
    auto sl = current_location();
    std::cout << "---------------[Testing basic]---------------"
              << "\n";
    std::cout << "Filename: " << sl.file_name() << "\n";
    std::cout << "Column: " << sl.column() << "\n";
    std::cout << "Function_name: " << sl.function_name() << "\n";
    std::cout << "Line: " << sl.line() << "\n";
    std::cout << "Invoke method to_string() result -> " << sl.to_string() << "\n";
    /* 逻辑运算符测试 */
    std::cout << "----------[Testing logic operators]----------"
              << "\n";
    auto sl_copy = sl;
    auto sl2 = current_location();
    std::cout << "Is sl_copy == sl ? result -> " << (sl_copy == sl ? "yes" : "no") << "\n";
    std::cout << "Is sl2 == sl ? result -> " << (sl2 == sl ? "yes" : "no") << "\n";
    std::cout << "Is sl_copy != sl ? result -> " << (sl_copy != sl ? "yes" : "no") << "\n";
    std::cout << "Is sl2 != sl ? result -> " << (sl2 != sl ? "yes" : "no") << "\n";
    /* 编译时计算测试 */
    std::cout << "---------[Testing complie-time eval]---------"
              << "\n";
    constexpr auto sl_constexpr = current_location();
    std::cout << "Filename: " << sl_constexpr.file_name() << "\n";
    std::cout << "Column: " << sl_constexpr.column() << "\n";
    std::cout << "Function_name: " << sl_constexpr.function_name() << "\n";
    std::cout << "Line: " << sl_constexpr.line() << "\n";
#if RAINY_HAS_CXX20
    std::cout << "---[Testing C++20 source_location surrport]--"
              << "\n";
    auto std_sl = std::source_location::current();
    source_location rainy_sl = std_sl;
    std::cout << "Filename: " << rainy_sl.file_name() << "\n";
    std::cout << "Column: " << rainy_sl.column() << "\n";
    std::cout << "Function_name: " << rainy_sl.function_name() << "\n";
    std::cout << "Line: " << rainy_sl.line() << "\n";
#endif
    return 0;
}
