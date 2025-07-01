#include <iostream>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/foundation/functional/functional.hpp>

class myfoo {
public:
    void foo() noexcept {
    }

    void foo_const() const noexcept {
    }

    void foo_volatile() volatile noexcept {
    }

    void foo_right() && noexcept {
        std::cout << "right\n";
    }
};

void foo() noexcept {
    std::cout << "foo\n";
}

int foo1(int a, int b) {
    std::cout << "foo1\n";
    std::cout << a << '\n';
    std::cout << b << '\n';
    return a + b;
}

int main() {
    using namespace rainy::meta::reflection;
    function f1 = &foo;
    if (static_cast<bool>(f1.type() & method_flags::noexcept_specified) ||
        static_cast<bool>(f1.type() & method_flags::static_specified)) {
        std::cout << "f1 is noexcept or static\n";
    } else {
        std::cout << "f1 is not noexcept or static\n";
    }
    function f2 = &foo1;
    if (static_cast<bool>(f1.type() & method_flags::static_specified)) {
        std::cout << "f2 is static\n";
    } else {
        std::cout << "f2 is not static\n";
    }
    function f3 = {[](int a, int b) { std::cout << "f3: a + b = " << (a + b) << '\n'; }};
    if (static_cast<bool>(f3.type() & method_flags::static_specified)) {
        std::cout << "f3 is static\n";
    } else {
        std::cout << "f3 is not static\n";
    }
    function f4 = &myfoo::foo;
    if (static_cast<bool>(f4.type() & method_flags::noexcept_specified)) {
        std::cout << "f4 is noexcept\n";
    } else {
        std::cout << "f4 is not noexcept\n";
    }
    f4.rebind(&myfoo::foo_const);
    if (static_cast<bool>(f4.type() & method_flags::const_qualified)) {
        std::cout << "f4 is const\n";
    } else {
        std::cout << "f4 is not const\n";
    }
    f4.rebind(&myfoo::foo_volatile);
    if (static_cast<bool>(f4.type() & method_flags::volatile_qualified)) {
        std::cout << "f4 is volatile\n";
    } else {
        std::cout << "f4 is not volatile\n";
    }
    return 0;
}
