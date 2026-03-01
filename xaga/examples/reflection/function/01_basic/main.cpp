#include <rainy/core/implements/text/format/implements.hpp>
#include <rainy/core/implements/text/format.hpp>

#if RAINY_USING_WINDOWS
#include <windows.h>
#endif
#include <iostream>
using namespace rainy;

int main() {
#if RAINY_USING_WINDOWS
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        int number = 1234567;
        std::string str = "Standard String";
        std::cout << "en_US: "
                  << foundation::text::format(std::locale("en_US.UTF-8"), "Hello World {} {:L} {} {} {} {0}", 3.14, number,
                                              "This char array", str, &str, nullptr)
                  << std::endl; // 1,234,567 Hello World This char array Standard String 0xdaa12ff438 0x0
        std::cout << "de_DE: " << foundation::text::format(std::locale("de_DE.UTF-8"), "{:L}", number) << std::endl; // 1.234.567
        std::cout << "fr_FR: "
                  << foundation::text::format(std::locale("fr_FR.UTF-8"), "{:L} Hello World 诗人握持 {}", number, "诗人握持")
                  << std::endl; // 1 234 567
        std::cout << "swiss: " << foundation::text::format(std::locale("de_CH.UTF-8"), "{:L}", number) << std::endl; // 1'234'567
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
    }

    /*rainy::foundation::concurrency::atomic<float> atomics = 10;
    std::cout << atomics << '\n';
    std::cout << atomics + 3.14f << '\n';
    std::cout << atomics + 3.14 << '\n';
    std::cout << atomics.fetch_sub(10) << '\n';
    std::cout << atomics << '\n';
    std::cout << atomics.fetch_sub(10) << '\n';
    rainy::foundation::concurrency::atomic_fetch_add(&atomics, 100);
    std::cout << atomics << '\n';*/
    return 0;
}
