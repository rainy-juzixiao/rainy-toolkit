#include "test.h"

class param {
public:
    int field{100};
    std::string s{"Hello Under World!"};
};

class internal_class {
public:
    void invoke() {
        std::cout << "invoke from test.dll\n";
    }

    void invoke1(const param& p) {
        std::cout << p.field << '\n';
        std::cout << p.s << '\n';
    }
};

rainy::meta::reflection::function get_fn() {
    static rainy::meta::reflection::function f = &internal_class::invoke;
    return f;
}

rainy::meta::reflection::function get_fn1() {
    static rainy::meta::reflection::function f = &internal_class::invoke1;
    return f;
}

rainy::meta::reflection::instance get_obj() {
    static internal_class object;
    return object;
}

rainy::meta::reflection::instance get_param() {
    static param p;
    return p;
}