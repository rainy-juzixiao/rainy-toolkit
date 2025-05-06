#include <iostream>
#include <rainy/meta/reflection/function.hpp>

int add(int, int) {
    return 10 + 20;
}

std::string make_str() {
    return "Hello World";
}

struct impl {
    void test(const float&) volatile & noexcept {
    
    }
};

int main() {
    using namespace rainy::meta::reflection;
    using namespace rainy::foundation::rtti;
    function f = &impl::test;
    std::cout << f.is_invocable({rainy_typeid(float)}) << '\n';
    std::cout << f.is_noexcept() << '\n';
    std::cout << f.is_const() << '\n';
    std::cout << f.is_volatile() << '\n';
    std::cout << f.is_invoke_for_lvalue() << '\n';
}
