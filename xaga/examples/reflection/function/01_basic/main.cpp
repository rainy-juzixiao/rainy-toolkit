#include <iostream>
#include <rainy/meta/reflection/function.hpp>

void test_str(std::string_view str) {
    std::cout << str << '\n';
}

int main() {
    using namespace rainy::meta::reflection;
    function ts_fn = &test_str;
    ts_fn.invoke_static(std::string{"Hello World"});
    ts_fn = [](std::string_view s) { std::cout << s << '\n'; };
    ts_fn.invoke_static("This Is A CStyle String");
    return 0;
}
