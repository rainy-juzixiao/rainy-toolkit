#include <iostream>
#include <rainy/text/format_wrapper.hpp>

using namespace rainy::utility;

int main() {
    // 基础格式化测试
    std::cout << "---[Testing Basic format]--------------------"
              << "\n";
    {
        std::string result;
        cstyle_format(result, "Hello, %s!", "world");
        std::cout << "Basic String Format: " << result << " (Expected: Hello, world!)"
                  << "\n";
    }
    // 数值格式化测试
    std::cout << "---[Testing Number format]-------------------"
              << "\n";
    {
        std::string result;
        cstyle_format(result, "The answer is %d.", 42);
        std::cout << "Number Format: " << result << " (Expected: The answer is 42.)"
                  << "\n";
    }
    // 混合格式化测试
    std::cout << "---[Testing Mixed format]--------------------"
              << "\n";
    {
        std::string result;
        cstyle_format(result, "Name: %s, Age: %d, Score: %.2f", "Alice", 25, 92.5);
        std::cout << "Mixed Format: " << result << " (Expected: Name: Alice, Age: 25, Score: 92.50)"
                  << "\n";
    }
    // 占位符检查测试
    std::cout << "---[Testing error on mismatch placeholder]---"
              << "\n";
    {
        std::string result;
        try {
            cstyle_format(result, "This has %d placeholders %d", 20, 10);
            std::cout << "Error on Mismatch Placeholder: Test success"
                      << "\n";
#if !RAINY_ENABLE_DEBUG
            cstyle_format(result, "This has %d placeholders %d", 20, 10, 40);
            std::cout << "Error on Mismatch Placeholder: Test failed"
                      << "\n";
#endif
        } catch (const std::runtime_error &e) {
            std::cout << "Error on Mismatch Placeholder: Caught exception: " << e.what() << "\n";
        }
    }
    // 测试长resize输出
    std::cout << "---[Testing resize for large output]---------"
              << "\n";
    {
        std::string result;
        std::string large_string(500, 'A');
        cstyle_format(result, "Large test: %s", large_string.c_str());
        std::cout << "Resize for Large Output: " << result << " (Expected: Large test: " << large_string << ")"
                  << "\n";
    }
    // 宽字符串测试
    std::cout << "---[Testing wide string basic format test]---"
              << "\n";
    {
        std::wstring result;
        cstyle_format(result, L"Hello, %s!", L"world");
        std::wcout << L"WString Basic Format: " << result << L" (Expected: Hello, world!)" << "\n";
    }
}
